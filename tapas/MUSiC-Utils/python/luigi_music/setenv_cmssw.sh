#!/bin/bash

# Usage: $0 [SCRAM_ARCH] [CMSSW_VERSION]

SCRAM_ARCH=$1
CMSSW_VERSION=$2

# If the parameters are not given on the command line, this script
# will use the defaults below.

export SCRAM_ARCH=${SCRAM_ARCH:-slc6_amd64_gcc493}
export CMSSW_VERSION=${CMSSW_VERSION:-CMSSW_7_6_3}

source /cvmfs/cms.cern.ch/cmsset_default.sh
export CVSROOT=:gserver:cmssw.cvs.cern.ch:/local/reps/CMSSW
export CVS_RSH=ssh

function cmsenv {
    eval `scramv1 runtime -sh`
}

pushd ~/CMSSW/ >/dev/null
    test -d $CMSSW_VERSION || cmsrel $CMSSW_VERSION
    cd $CMSSW_VERSION/src
    cmsenv
popd >/dev/null

