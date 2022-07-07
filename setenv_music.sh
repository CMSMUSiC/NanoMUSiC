#!/bin/bash

# This is a set_env script
DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
# load user configs
source music_env.config


# Set up the CMSSW environment
cd $DIR/$CMSSW_VER/src/PxlSkimmer
source /cvmfs/cms.cern.ch/cmsset_default.sh
cmsenv
source set_env.sh

# This is a TAPAS set_env script. Source it before usage of TAPAS.
export LD_LIBRARY_PATH=$DIR/tapas/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$DIR/tapas/lib/python:$PYTHONPATH
export MYPXLANA=EventClassFactory


# Set up CRAB3
source /cvmfs/cms.cern.ch/crab3/crab.sh

# Source all the needed set_env
cd $DIR/tapas/tools/
source set_env.sh

cd $DIR/tapas/PxlAnalyzer/
source set_env.sh

cd $DIR/tapas/PlotLib/
source set_env.sh

cd $DIR/tapas/PxlAnalyzer/EventClassFactory/
source set_env.sh

cd $DIR/tapas/MUSiC-Utils/
source set_env.sh

cd $DIR/tapas/MUSiC-Configs/
source set_env.sh

cd $DIR/tapas/MUSiC-RoIScanner/
source set_env.sh

cd $DIR

echo "Initialize your grid certificate..."
voms-proxy-init --voms cms:/cms --valid 192:0
