#!/usr/bin/env python
# Copyright [2015] <III Phys. Inst A, RWTH Aachen University>

# standard library imports
import sys
import os
import subprocess
import functools

##################################################
# Utility functions

## Return the CERN username
#
# If the environment variable $CERNUSERNAME is not set, crab is used as a fallback.
def get_username_cern():
    if 'CERNUSERNAME' in os.environ:
        return os.environ.get('CERNUSERNAME')
    else:
        try:
            from crabFunctions import CrabController
            crab = CrabController()
            return crab.checkusername()
        except:
            raise Exception('Failed to obtain CERN username. Specify your CERN username using the environment variable $CERNUSERNAME.')


##################################################
# VOMS functions

## Exception for the VOMS proxy
class ProxyError(Exception):
    pass



## Returns the path to the current proxy
#
# @return path parsed path vom voms-proxy-info
def voms_proxy_path():
    process = subprocess.Popen(['voms-proxy-info', '-path'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    stdout = process.communicate()[0]
    if process.returncode != 0:
        return False
    else:
        return stdout.strip("\n")

## Returns the remaining lifetime of the voms proxy
#
# @return int time left for the proxy
def voms_proxy_time_left():
    process = subprocess.Popen(['voms-proxy-info', '-timeleft'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    stdout = process.communicate()[0]
    if process.returncode != 0:
        return 0
    else:
        return int(stdout)


## Checks if the proxy is valid longer than time
#
# @type time: int
# @param time: reference time in seconds to check against
# @return boolean returns True if the proxy is valid longer than time, False otherwise.
def voms_proxy_check(time=86400):
    timeleft = voms_proxy_time_left()
    return timeleft > time


## Create a new voms proxy
#
# @type voms: string
# @param voms: the voms group used to set up the server [default:cms:/cms/dcms]
# @type passphrase: string
# @param passphrase: Passphrase for GRID certificate [default:none]. The password request is send to the prompt if no passphrase given
# @param noprompt: Do not show a password request on the pompt but raise an exeption
def voms_proxy_create(voms='cms:/cms/dcms', passphrase=None, noprompt=False):
    if passphrase:
        p = subprocess.Popen(['voms-proxy-init', '--voms', voms, '--valid', '192:00'],
                             stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
        stdout = p.communicate(input=passphrase+'\n')[0]
        retcode = p.returncode
        if not retcode == 0:
            raise ProxyError('Proxy initialization command failed: %s'%stdout)
    elif not noprompt:
        retcode = subprocess.call(['voms-proxy-init', '--voms', voms, '--valid', '192:00'])
    if not retcode == 0:
        raise ProxyError('Proxy initialization command failed.')


## Ensure the proxy is still valid for the given time
#
# If the time check fails, a new proxy is generated.
#
# @type time: int
# @param time: reference time in seconds to check against
# @param voms: the voms group used to set up the server [default:cms:/cms/dcms]
# @type passphrase: string
# @param passphrase: Passphrase for GRID certificate [default:none]. The password request is send to the prompt if no passphrase given
def voms_proxy_ensure(time=604800, voms='cms:/cms/dcms', passphrase=None):
    if not voms_proxy_check(time):
        voms_proxy_create(passphrase=passphrase)
        if not voms_proxy_check(time):
            raise ProxyError('Proxy still not valid long enough!')


## Get vo, voGroup and voRole for current proxy
#
# @ return string tuple (vo, voGroup, voRole)
#
def voms_proxy_fquan():
    p = subprocess.Popen("voms-proxy-info  --fqan",
                          stdout = subprocess.PIPE,
                          stderr = subprocess.PIPE,
                          shell=True)
    stdout, stderr = p.communicate()
    vo, voGroup, voRole = '', '' ,''
    if p.returncode == 0:
        lines = stdout.split("\n")
        splitline = lines[0].split("/")
        if len(splitline) < 4:
            splitline = lines[1].split("/")
        vo = splitline[1]
        voGroup = splitline[2]
        try:
            voRole = splitline[2].split("=")[1]
            if  "NULL" in self.role:
                voRole = ""
        except:
            voRole = ""
    return vo, voGroup, voRole


class VomsProxy:

    ## The object constructor
    #
    # @param self The object pointer
    # @param vo default vo for this proxy
    # @param voGroup default voGroup for this proxy
    # @param voRole default voRole for this proxy
    def __init__(self,vo='cms',voGroup='',voRole='',passphrase=None):
        if not (voGroup or voRole):
            self.fetch_vo_infos()
        else:
            self.vo = vo
            self.voGroup = voGroup
            self.voRole = voRole
        self._path = None
        self.passphrase = passphrase

    ## Property for path to proxy
    #
    # @param self The object pointer
    @property
    def path(self):
        if not self._path:
            self._path = voms_proxy_path()
        return self._path

    ## Property for voms string constructed from vo, voGroup and voRole
    #
    # @param self The object pointer
    @property
    def voms(self):
        voms = [self.vo+":",self.vo]
        if self.voGroup:
            voms.append(self.voGroup)
        if self.voRole:
            voms.append("Role=" + self.voRole)
        return "/".join(voms)

    ## Property for the time left for this proxy
    #
    # @param self The object pointer
    @property
    def timeleft(self):
        return voms_proxy_time_left()

    ## Get and cache vo, voGroup and voRoles for proxy
    def fetch_vo_infos(self):
        self.vo, self.voGroup, self.voRole = voms_proxy_fquan()
        if not self.vo:
            raise ValueError("Unable to retrieve vo,voGroup,voRole info from proxy")

    ## Check if proxy is valid for some time or renew it
    #
    # @param self The object pointer
    # @param time required minimal lifetime in seconds
    def ensure(self, time=604800):
        if self.timeleft < time:
            self.create()

    ## Create a new proxy
    #
    # @param self The object pointer
    def create(self):
        voms_proxy_create(voms=self.voms, passphrase=self.passphrase)
        self._path = None
        self.fetch_vo_infos()

## Decorator: Cache the result of a function
#
# This decorator an be used for instance caching, too.
# Caching is done on a per-process basis using the process id.
# Only works if args and kwargs.values() contain only hashable types.
#
# @type func: function
# @param func: function to decorate
_memoized = {}
def memoize(func):
    @functools.wraps(func)
    def memoized(*args, **kwargs):
        key = memoization_id( func, args, kwargs )
        if key not in _memoized:
            _memoized[key] = func(*args, **kwargs)
        return _memoized[key]
    return memoized

## Helper: generate an id for memoization (used in other contexts too)
def memoization_id(func, args=tuple(), kwargs=dict()):
    return (func.__name__, tuple(args), tuple(kwargs))

if __name__=='__main__':
    print 'This is a library'
    sys.exit(0)
