#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# This is a set_env script
# Set up the CMSSW environment

while getopts ":-cmssw-ver:" opt; do
  case $opt in
    a)
      CMSSW_VER=$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done


cd $SCRIPT_DIR/skimmer/$CMSSW_VER/src/PxlSkimmer
source /cvmfs/cms.cern.ch/cmsset_default.sh
cmsenv
source set_env.sh

# Set up the TAPAS environment

cd
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


while getopts ":-init-proxy:" opt; do
  case $opt in
    a)
      echo "Authenticate your grid certificate..."
      voms-proxy-init --voms cms:/cms --valid 192:0
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
    #   echo "Option -$OPTARG requires an argument." >&2
    #   exit 1
    #   ;;
  esac
done


