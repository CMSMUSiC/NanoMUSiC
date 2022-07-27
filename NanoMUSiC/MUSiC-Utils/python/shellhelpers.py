from contextlib import contextmanager
import os, os.path
import pipes
import re
import stat
import subprocess
import sys

# https://stackoverflow.com/questions/377017/test-if-executable-exists-in-python
def which( cmd, paths=None ):
    # check if a path has already been provided
    cmd_basename = os.path.basename( cmd )
    if cmd != cmd_basename:
        return cmd

    if not paths:
        # split os.environ, note that os.pathsep exists exactly for this purpose
        # (':' on linux), and is not to be confused with os.path.sep ('/' on linux)
        path_env = os.environ.get( "PATH", "" )
        paths = path_env.split( os.pathsep )

    # iterate through candidate results, this is the same order as bash uses
    for path in paths:
        path = path.strip('"')
        candidate = os.path.join( path, cmd_basename )

        # check if a candidate file exists and is executable by the current user
        if os.path.exists( candidate ) and os.access( candidate, os.X_OK ):
            # return on the very first match
            return os.path.abspath( candidate )

    raise ValueError( "No %s in (%s)" % ( cmd, path_env ) )


# https://stackoverflow.com/questions/3503719/emulating-bash-source-in-python
def source_environment( script, args=tuple(), update_environ=True, update_pythonpath=True, start_blank=True ):
    # Explanation of the command:
    # bash -c: "Start bash with the following command"
    # source script 1>/dev/null: "Execute script and ignore stdout output"
    # env -0: "Print environment variables, separate each line with a null byte"
    script_command = [ quote( script ) ] + list( args )
    command = ['bash', '-c', 'source ' + ' '.join( script_command ) + ' 1>/dev/null && env -0']

    # provide a blank environment (or a copy of this process ones)
    if start_blank:
        proc_env = {}
    else:
        proc_env = os.environ.copy()

    # execute process and get content of stdout (also checks return code)
    proc = subprocess.Popen( command, env=proc_env, stderr=subprocess.PIPE, stdout=subprocess.PIPE )
    stdout, stderr = proc.communicate()
    if proc.returncode != 0:
        command_string = " ".join( command )
        raise IOError("Command '%s' returned non-zero code %d.\nstderr: %s" % (command_string, proc.returncode, stderr))

    # split by null bytes
    lines = stdout.split( '\0' )

    # define a blacklist of environment variables to ignore
    # shlvl is just an "inception counter" on how many nested levels of bash are called
    # and "_" is the name of the current shell (useless)
    ignore = ('SHLVL', '_')

    # record changes to env dictionary
    env = {}
    for line in lines:
        line = line.strip( '\0' )
        if not line:
            continue
        ( key, _, value ) = line.partition( "=" )
        if key in ignore:
            continue
        env[ key ] = value

    # optionally: update os.environ "magic" dict
    if update_environ:
        os.environ.update( env )

    # optionally: add missing paths to python search path
    if update_pythonpath:
        new_path = []
        for path in os.environ[ 'PYTHONPATH' ].split( os.pathsep ):
            if not path in sys.path:
                new_path.append( path )
        sys.path = new_path + sys.path

    return env

# expand environment variables and home (e.g. ~) from path, check if everything worked
def expand_path( name ):
    expanded = os.path.expandvars( name )
    expanded = os.path.expanduser( expanded )
    if "$" in expanded:
        raise ValueError( "Unresolved environment variable in %r" % name )
    return expanded

# remove all chars from a filename that are not included in official POSIX
# "Portable Filename Character Set", see
# http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_282
# Note that some operating systems may allow more characters, but sticking to
# these mentioned here will guarantee portability among POSIX systems.
def clean_filename( name ):
    name = re.sub( r'[^A-Za-z0-9\._-]', '_', name.strip() )
    assert name not in ( "", ".", ".." )
    return name

# temporary change working directory
@contextmanager
def tmp_chdir( destination, create=False ):
    cwd = os.getcwd()
    env_pwd = os.environ["PWD"]

    if create and not os.path.exists( destination ):
        os.makedirs( destination )

    os.chdir( destination )
    os.environ["PWD"] = os.getcwd()

    try:
        yield
    finally:
        os.chdir( cwd )
        os.environ["PWD"] = os.getcwd()

def mark_executable( path, user=True, group=False, others=False ):
    # obtain current flags
    st = os.stat( path )
    flags = st.st_mode

    # clear all executable flags
    flags &= ~0o111

    # selectively set executable flags
    if user:
        flags |= stat.S_IXUSR
    if group:
        flags |= stat.S_IXGRP
    if others:
        flags |= stat.S_IXOTH

    # apply to file using chmod
    os.chmod( path, flags )
