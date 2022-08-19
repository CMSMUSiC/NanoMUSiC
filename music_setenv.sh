#!/bin/bash

# This is a set_env script
SCRIPTDIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
# load user configs
source music_env.config

# Set up the CMSSW environment
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /cvmfs/cms.cern.ch/$MY_SCRAM_ARCH/cms/cmssw/$CMSSW_VER/ 
cmsenv

# This is a TAPAS set_env script. Source it before usage of TAPAS.
export LD_LIBRARY_PATH=$SCRIPTDIR/NanoMUSiC/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/lib/python:$PYTHONPATH
export MYPXLANA=EventClassFactory

# Set up CRAB3
source /cvmfs/cms.cern.ch/crab3/crab.sh

# Source all the needed set_env
cd $SCRIPTDIR/NanoMUSiC/tools/
source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/PxlAnalyzer/
source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/PlotLib/
source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/PxlAnalyzer/EventClassFactory/
source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/MUSiC-Utils/
source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/MUSiC-Configs/
source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/MUSiC-RoIScanner/
source set_env.sh

cd $SCRIPTDIR

export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/bin;


export CORRECTIONLIB=$SCRIPTDIR/correctionlib/correctionlib
export LD_LIBRARY_PATH=$CORRECTIONLIB/lib/:$LD_LIBRARY_PATH

# control whether MUSiC anvs are set
# to be used by Makefile
export MUSIC_IS_SET_ENV=1

# reload user configs
source music_env.config