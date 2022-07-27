#!/usr/bin/env python

import os
import sys


def checkEnvironment():
    major, minor, micro, releaselevel, serial = sys.version_info
    if major < 2 or minor < 4:
        raise EnvironmentError( 1, 'Requires Python >= 2.4! Activate a CMSSW release to get that.' )

    var = 'MUSIC_BASE'
    music_path = os.getenv( var )
    if music_path == None:
        raise EnvironmentError( 1, 'Environment variable not set', var )

    var = 'CMSSW_VERSION'
    cmssw_version = os.getenv( var )
    if cmssw_version == None:
        raise EnvironmentError( 1, 'Environment variable not set', var )

    var = 'CMSSW_BASE'
    cmssw_base = os.getenv( var )
    if cmssw_base == None:
        raise EnvironmentError( 1, 'Environment variable not set', var )

    var = 'SCRAM_ARCH'
    scram_arch = os.getenv( var )
    if scram_arch == None:
        raise EnvironmentError( 1, 'Environment variable not set', var )

    #return music_path, cmssw_version, cmssw_base, scram_arch
    return  cmssw_version, cmssw_base, scram_arch
