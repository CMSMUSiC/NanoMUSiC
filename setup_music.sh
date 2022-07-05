#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Set up the CMSSW environment
echo "Always check the latest recommendations for CMSSW version." 
echo "https://twiki.cern.ch/twiki/bin/view/CMS/PdmVRun2LegacyAnalysis" 
echo ""

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

# Set up the skimmer
# TODO: how to properly setup the skimmer, without messing with the tag???
cd $SCRIPT_DIR/skimmer
source /cvmfs/cms.cern.ch/cmsset_default.sh
cmsenv
cmsrel $CMSSW_VER

cd $CMSSW_VER/src
cmsenv

# setup UL configs
git cms-merge-topic cms-egamma:EgammaPostRecoTools
git cms-addpkg EgammaAnalysis/ElectronTools
rm EgammaAnalysis/ElectronTools/data -rf
git clone git@github.com:cms-data/EgammaAnalysis-ElectronTools.git EgammaAnalysis/ElectronTools/data

# links to Pxl and PxlSkimmer
ln -s ../../Pxl Pxl
ln -s ../../PxlSkimmer PxlSkimmer

echo "Fix the PxlSkimmer tag mechanism!!!"
# cd PxlSkimmer
# git checkout CMSSW_10_6_v5p9_2017

scram build -j 8
#Redo the cmsenv
source /cvmfs/cms.cern.ch/cmsset_default.sh
cmsenv


cd $SCRIPT_DIR
