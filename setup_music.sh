#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Set up the CMSSW environment
echo "!!! Always check the latest recommendations for CMSSW version. !!!" 
echo "!!! https://twiki.cern.ch/twiki/bin/view/CMS/PdmVRun2LegacyAnalysis !!!" 
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


echo "--> TODO: Fix the PxlSkimmer tag mechanism!!!"
# cd PxlSkimmer
# git checkout CMSSW_10_6_v5p9_2017
# cd ..

# Compiles
scram build -j 8

#Redo the cmsenv
source /cvmfs/cms.cern.ch/cmsset_default.sh
cmsenv

cd $SCRIPT_DIR

# Setup TAPAS and PxlAnalyzer

# load user configuration parameters
source music.config

# setup TAPAS

# create links for libaries
mkdir -p lib/python
ln -s /usr/lib64/libdavix.so.0 lib/libdavix.so.0
ln -s /usr/lib64/python2.6/lib-dynload/_curses.so lib/python/_curses.so
ln -s /cvmfs/cms.cern.ch/slc6_amd64_gcc493/external/py2-pycurl/7.19.0-kpegke/lib/python2.7/site-packages/pycurl.so lib/python/pycurl.so


# Create music_env.config
touch music_env.config
echo "" > music_env.config
echo "export CERNUSERNAME=$2" >> music_env.config
echo "export CMSSW_VER=$CMSSW_VER" >> music_env.config
