#!/bin/bash

# This is a set_env script
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Set up the CMSSW environment
cd $SCRIPT_DIR/CMSSW_*/src/PxlSkimmer
source /cvmfs/cms.cern.ch/cmsset_default.sh
cmsenv
source set_env.sh

# Set up the TAPAS environment
cd $SCRIPT_DIR/tapas

source setenv_tapas.sh

# Set up CRAB3
source /cvmfs/cms.cern.ch/crab3/crab.sh

# Source all the needed set_env
cd $SCRIPT_DIR/tapas/tools/
source set_env.sh

cd $SCRIPT_DIR/tapas/PxlAnalyzer/
source set_env.sh

cd $SCRIPT_DIR/tapas/PxlAnalyzer/EventClassFactory/
source set_env.sh

cd $SCRIPT_DIR/tapas/MUSiC-Utils/
source set_env.sh

cd $SCRIPT_DIR/tapas/MUSiC-Configs/
source set_env.sh

cd $SCRIPT_DIR/tapas/MUSiC-RoIScanner/
source set_env.sh


cd $SCRIPT_DIR


echo "Initialize your grid certificate..."
voms-proxy-init --voms cms:/cms --valid 192:0
