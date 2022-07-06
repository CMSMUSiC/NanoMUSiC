#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Set up the CMSSW environment
echo "Always check the latest recommendations for CMSSW version." 
echo "https://twiki.cern.ch/twiki/bin/view/CMS/PdmVRun2LegacyAnalysis" 
echo ""

# Set CMSSW version

CMSSW_VER=$1

# Set up the skimmer
# TODO: how to properly setup the skimmer, without messing with the tag???
cd $SCRIPT_DIR
source /cvmfs/cms.cern.ch/cmsset_default.sh
cmsrel $CMSSW_VER
cd $CMSSW_VER/src
cmsenv

# Clones the PxlSkimmer
git clone git@github.com:CMSMUSiC/PxlSkimmer.git .

# setup Run2UL configs
git cms-merge-topic cms-egamma:EgammaPostRecoTools
git cms-addpkg EgammaAnalysis/ElectronTools
rm EgammaAnalysis/ElectronTools/data -rf
git clone git@github.com:cms-data/EgammaAnalysis-ElectronTools.git EgammaAnalysis/ElectronTools/data


echo "--> Fix the PxlSkimmer tag mechanism!!!"
# cd PxlSkimmer
# git checkout CMSSW_10_6_v5p9_2017
# cd ..

# Compiles
scram build -j 8

#Redo the cmsenv
source /cvmfs/cms.cern.ch/cmsset_default.sh
cmsenv

cd $SCRIPT_DIR
