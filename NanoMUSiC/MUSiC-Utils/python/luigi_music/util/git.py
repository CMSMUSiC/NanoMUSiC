import datetime
import os, os.path
import re
import subprocess

from .helpers import cached_property

class GitRepository:
    GIT_CMD = "git"

    @classmethod
    def clone_url( cls, url, path ):
        repo = cls( path )
        repo.clone( url )
        return repo

    @classmethod
    def get_remote_commit_hash( cls, url, commit ):
        branch_name = commit.rpartition("/")[ -1 ]
        output = cls._git( 'ls-remote', url, branch_name )
        for line in output.splitlines():
            hash, ref = map( lambda p: p.strip(), line.split() )
            if commit == "HEAD" and ref == "HEAD":
                return hash
            elif ref.startswith( "refs/" ) and ref.endswith( "/" + commit ):
                # return the first match
                return hash
        raise ValueError( "Commit/branch/tag '%s' not found at '%s'." % ( commit, url ) )

    def __init__( self, path ):
        self.path = path
        self._cached_commits = {}

    def get_remote_url( self, remote="origin" ):
        config = "remote." + remote + ".url"
        return self.call_git( 'config', '--get', config ).strip()

    @property
    def head( self ):
        return self.get_commit( "HEAD" )

    @property
    def git_dir( self ):
        return os.path.join( self.path, ".git" )

    def get_commit( self, name ):
        hash = self.get_commit_hash( name )
        if hash not in self._cached_commits:
            commit = GitCommit( self, hash )
            self._cached_commits[ hash ] = commit
        return self._cached_commits[ hash ]

    def get_commit_hash( self, commit ):
        return self.call_git( 'rev-parse', '--verify', commit ).strip()

    def exists( self ):
        return os.path.isdir( self.git_dir )

    def is_clean( self, ignore_untracked=True ):
        untracked_option = "--untracked-files=" + ( "no" if ignore_untracked else "normal" )
        stdout = self.call_git( "status", untracked_option, "--porcelain" )
        return stdout.strip() == ""

    def clone( self, url ):
        return self._git( 'clone', url, self.path )

    def reset( self, commit, hard=False ):
        return self.call_git( 'reset', '--hard', commit )

    def checkout( self, commit ):
        return self.call_git( 'checkout', commit )

    def fetch( self ):
        return self.call_git( 'fetch', '--all' )

    def add_worktree( self, path, commit ):
        self.call_git( 'worktree', 'add', path, commit )
        return GitWorktree( path, parent=self )

    def call_git( self, *args ):
        command = ['--work-tree', self.path, '--git-dir', self.git_dir ]
        command += args
        return self._git( *command )

    @classmethod
    def _git( cls, *args ):
        args = list( args )

        args.insert( 0, cls.GIT_CMD )

        args = map( str, args )
        proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate()

        if proc.returncode != 0:
            command_string = " ".join( args )
            raise IOError( proc.returncode, "Command '%s' returned non-zero code %d.\nstderr: %s" % (command_string, proc.returncode, stderr) )

        return stdout

# Untested: Worktree support
class GitWorktree( GitRepository ):
    def __init__( self, path, parent ):
        super( GitWorktree, self ).__init__( path )
        self.parent = parent
        assert isinstance( self.parent, GitRepo )

    @property
    def git_dir( self ):
        basename = os.path.basename( self.path )
        parent_git_dir = self.parent.git_dir
        return os.path.join( parent_git_dir, "worktrees", basename )

class GitCommit:
    HASH_RE = re.compile( r"^[0-9a-f]{40}$" )

    def __init__( self, repository, hash ):
        hash = hash.lower()
        if not self.HASH_RE.match( hash ):
            raise ValueError( "Commit hash '%s' is invalid." % hash )

        self.hash = hash
        self.repository = repository

    def _commit_log( self, format ):
        return self.repository.call_git( "log", "--format=" + format, "-n", 1, self.hash )

    @cached_property
    def message( self ):
        return self._commit_log( "%B" ).strip()

    @cached_property
    def author( self ):
        return self._commit_log( "%an" ).strip()

    @cached_property
    def author_email( self ):
        return self._commit_log( "%ae" ).strip()

    @cached_property
    def date( self ):
        unix_timestamp = float( self._commit_log( "%at" ).strip() )
        return datetime.datetime.fromtimestamp( unix_timestamp )

    @cached_property
    def parents( self ):
        hashes = self._commit_log( "%P" ).split()
        parents = []
        for hash in hashes:
            hash = hash.strip()
            commit = self.__class__( self.repository, hash )
            parents.append( commit )
        return parents

    def __eq__( self, other ):
        return self.hash == other.hash

if __name__=="__main__":
    repo = GitRepository( "." )
    h = repo.head
    print( h.hash )
    print( h.author )
    print( h.author_email )
    print( h.date )
    print( h.message )
