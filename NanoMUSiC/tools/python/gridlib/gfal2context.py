from collections import namedtuple
from contextlib import contextmanager
import functools
import gc
import inspect
import logging
import os
import random
import stat
import sys
import time
import gfal2
import re

# Use this for debugging
# Note that this will also trigger a segfault within gfal at the end of execution
gfal2.set_verbose(gfal2.verbose_level.warning)

log = logging.getLogger("gfal2context")
log.setLevel(logging._levelNames[ 'WARNING' ])
# Code adapted from package common.analyses.common.luigi.target.remote
# by Aachen 3A, Group Prof. Erdmann, RWTH Aachen University

def retry(func):
    @functools.wraps(func)
    def wrapper(self, *args, **kwargs):
        retry = kwargs.pop("retry", None)
        if retry is None:
            retry = self.retry

        retry_delay = kwargs.pop("retry_delay", None)
        if retry_delay is None:
            retry_delay = self.retry_delay

        try_ = 0

        lastErr = None
        while try_ <= retry:
            try:
                return func(self, *args, **kwargs)
            except gfal2.GError as e:
                lastErr = e
                try_ += 1
                time.sleep(retry_delay)
                continue
        else:
            raise lastErr

    return wrapper

DirectoryEntry = namedtuple("DirectoryEntry", ("name", "stat"))

# Documentation:
# https://pythonhosted.org/gfal2-python/gfal2.Gfal2Context-class.html
# http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/gfal2/group__file__group.html
class Gfal2Context(object):
    def __init__(self, gfal_options=None, transfer_options=None,
                 reset_context=False, retry=0, retry_delay=0):
        super(Gfal2Context, self).__init__()

        # cache for context objects per pid for process/thread safety
        self._contexts = {}

        # prepare gfal_options
        # for full list of options see configuration files in /etc/gfal2.d/
        if gfal_options is None:
            gfal_options = {}
        self.gfal_options = gfal_options

        # prepare transfer_options
        if transfer_options is None:
            transfer_options = {}
        transfer_options.setdefault("checksum_check", False)
        transfer_options.setdefault("overwrite", True)
        transfer_options["nbstreams"] = 1
        self.transfer_options = transfer_options

        self.reset_context = reset_context
        self.retry = retry
        self.retry_delay = retry_delay

    def __del__(self):
        for ctx in self._contexts.values():
            try:
                del ctx
            except:
                pass

        self._contexts.clear()

    @contextmanager
    def context(self):
        pid = os.getpid()
        if pid not in self._contexts:
            # yeap, the official "gfal2" API has a typo there (creat_context)
            log.debug("Creating new GFAL2 context for pid %d", pid)
            self._contexts[pid] = gfal2.creat_context()
            # apply options
            for group_name_tuple, value in self.gfal_options.items():
                if isinstance( value, str ):
                    type_ = "string"
                elif isinstance( value, int ):
                    type_ = "integer"
                elif isinstance( value, bool ):
                    type_ = "boolean"
                elif isinstance( value, (list, tuple) ):
                    type_ = "string_list"
                option_group, option_name = group_name_tuple
                func_name = "set_opt_" + type_
                log.debug("Setting GFAL2 option: %s(%r, %r, %r)", func_name, option_group, option_name, value)
                getattr(self._contexts[pid], func_name)( option_group, option_name, value )
        else:
            log.debug("Reusing existing GFAL2 context for pid %d", pid)

        try:
            yield self._contexts[pid]
        finally:
            if self.reset_context and pid in self._contexts:
                log.debug("Deleting GFAL2 context for pid %d", pid)
                del self._contexts[pid]
            gc.collect()

    @retry
    def exists(self, rpath):
        with self.context() as ctx:
            try:
                stat = ctx.stat(rpath)
            except gfal2.GError:
                stat = None

        return stat is not None

    @retry
    def isdir(self, rpath):
        with self.context() as ctx:
            mode = ctx.stat(rpath).st_mode
            return stat.S_ISDIR(mode)

    @retry
    def stat(self, rpath):
        with self.context() as ctx:
            return ctx.stat(rpath)

    @retry
    def chmod(self, rpath, mode):
        with self.context() as ctx:
            return ctx.chmod(rpath, mode)

    @retry
    def unlink(self, rpath):
        with self.context() as ctx:
            return ctx.unlink(rpath)

    @retry
    def rmdir(self, rpath):
        with self.context() as ctx:
            return ctx.rmdir(rpath)

    @retry
    def mkdir(self, rpath, mode=0o0755):
        with self.context() as ctx:
            return ctx.mkdir(rpath, mode)

    @retry
    def mkdir_rec(self, rpath, mode):
        with self.context() as ctx:
            return ctx.mkdir_rec(rpath, mode)

    @retry
    def listdir(self, rpath):
        with self.context() as ctx:
            return ctx.listdir(rpath)

    @retry
    def readpp(self, rpath):
        with self.context() as ctx:
            dir = ctx.opendir(rpath)
            entries = []
            while True:
                entry, stat = dir.readpp()
                if entry is None:
                    break
                entries.append( DirectoryEntry(entry.d_name, stat) )
            return entries

    @retry
    def filecopy(self, source, target, transfer_options=None):
        with self.context() as ctx:
            # combine global and local options to common dictionary
            final_transfer_options = self.transfer_options.copy()
            if transfer_options:
                final_transfer_options.update(transfer_options)

            log.debug("GFAL2 copy transfer options: %r", final_transfer_options)

            # copy dict values into params object
            params = ctx.transfer_parameters()
            for key, value in final_transfer_options.items():
                setattr(params, key, value)

            return ctx.filecopy(params, source, target)

    @retry
    def rename(self, source, target):
        with self.context() as ctx:
            return ctx.rename(source, target)

    @retry
    def checksum(self, rpath, algorithm="ADLER32"):
        with self.context() as ctx:
            return ctx.checksum(rpath, algorithm)

    @retry
    def open(self, rpath, mode="r"):
        with self.context() as ctx:
            return ctx.open(rpath, mode)

    @retry
    def listxattr(self, rpath):
        with self.context() as ctx:
            return ctx.listxattr(rpath)

    @retry
    def getxattr(self, rpath, name):
        with self.context() as ctx:
            return ctx.getxattr(rpath, name)

    ## GFAL equivalent to the bash 'find' function
    #
    # Returns files or directories located under the given path matching a regular expression
    #
    # @param self Object pointer
    # @param rpaths Either single path or list of paths to search in
    # @param regex Regular expression that file/directory names should match
    # @param match_directories If true, directories matching regex are returned
    # @param match_files If true, files matching regex are returned
    # @param matches Pass through argument for recursion implementation
    @retry
    def find(self, rpaths, regex, match_directories=True, match_files=True, matches=None):
        # For convenience
        if type(rpaths) is str:
            rpaths = [rpaths]

        # This is needed to avoid the matches list being stored between calls
        if not matches:
            matches = []

        # If we find subdirectories, store them and pass them to next level of recursion
        paths_to_check = []

        with self.context() as ctx:
            for rpath in rpaths:
                # Loop over all directory entries
                for entry in self.readpp(rpath):
                    is_dir = stat.S_ISDIR(entry.stat.st_mode)
                    current_path = os.path.join(rpath, entry.name)

                    is_match = re.match(regex, entry.name)

                    if is_dir:
                        paths_to_check.append(current_path)

                    if is_match:
                        if (match_directories and is_dir) or (match_files and not is_dir):
                            matches.append(current_path)

        if len(paths_to_check):
            return self.find(paths_to_check,
                         regex,
                         match_directories=match_directories,
                         match_files=match_files,
                         matches=matches)
        else:
            return matches

