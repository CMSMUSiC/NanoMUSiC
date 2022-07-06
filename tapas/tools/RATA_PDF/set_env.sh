#!/usr/bin/env bash
export RATAPDF="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/cvmfs/cms.cern.ch/slc6_amd64_gcc491/external/lhapdf6/6.1.5/lib/
export LHAPDF_BASE=/cvmfs/cms.cern.ch/slc6_amd64_gcc491/external/lhapdf6/6.1.5/
#export LD_LIBRARY_PATH=${RATAPDF}/lib/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$RATAPDF
