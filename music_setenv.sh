#!/bin/bash

# This is a set_env script
SCRIPTDIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )


# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/release_packages/x86_64-centos7-gcc11-opt/102/
source /cvmfs/sft.cern.ch/lcg/views/LCG_102/x86_64-centos7-gcc11-opt/setup.sh

# Set dummy SCRAM_ARCH. needed for LHAPDF.
export SCRAM_ARCH=slc7_amd64_gcc11

# This is a TAPAS set_env script. Source it before usage of TAPAS.
export LD_LIBRARY_PATH=$SCRIPTDIR/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/lib/python:$PYTHONPATH
export MYPXLANA=EventClassFactory

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


export PATH=$PATH:$SCRIPTDIR/bin;


export CORRECTIONLIB=$SCRIPTDIR/extern/correctionlib/correctionlib
export LD_LIBRARY_PATH=$CORRECTIONLIB/lib/:$LD_LIBRARY_PATH


# control whether MUSiC envs are set
# to be used by Makefile
export MUSIC_IS_SET_ENV=1