#!/bin/bash

if [ -z "$1" ]; then
      echo "ERROR: Please, set the CMSSW version, your architecture and your CERN username."
      echo "ERROR: Example: ./setup_music.sh CMSSW_10_6_29 slc7_amd64_gcc700 your_CERN_username"
      exit 1
fi

if [ -z "$2" ]; then
      echo "ERROR: Please, set the CMSSW version, your architecture and your CERN username."
      echo "ERROR: Example: ./setup_music.sh CMSSW_10_6_29 slc7_amd64_gcc700 your_CERN_username"
      exit 1
fi

if [ -z "$3" ]; then
      echo "ERROR: Please, set the CMSSW version, your architecture and your CERN username."
      echo "ERROR: Example: ./setup_music.sh CMSSW_10_6_29 slc7_amd64_gcc700 your_CERN_username"
      exit 1
fi

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# Set up the CMSSW environment
echo "INFO: Always check the latest recommendations for CMSSW version."
echo "INFO: https://twiki.cern.ch/twiki/bin/view/CMS/PdmVRun2LegacyAnalysis"
echo ""

# Set CMSSW version
CMSSW_VER=$1
MY_SCRAM_ARCH=$2

# Set up the skimmer
cd $SCRIPT_DIR
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /cvmfs/cms.cern.ch/$MY_SCRAM_ARCH/cms/cmssw/$CMSSW_VER/ 
cmsenv
CMSSW_RELEASE_BASE=/cvmfs/cms.cern.ch/$MY_SCRAM_ARCH/cms/cmssw/$CMSSW_VER/
cd $SCRIPT_DIR

# create links for libaries
mkdir -p lib/python
ln -s /usr/lib64/libdavix.so.0 lib/libdavix.so.0
ln -s /usr/lib64/python2.6/lib-dynload/_curses.so lib/python/_curses.so
ln -s /cvmfs/cms.cern.ch/slc6_amd64_gcc493/external/py2-pycurl/7.19.0-kpegke/lib/python2.7/site-packages/pycurl.so lib/python/pycurl.so

# setup table2latex
cd $SCRIPT_DIR/NanoMUSiC/MUSiC-RoIScanner
git clone git@github.com:tobias-pook/table2latex.git
cd table2latex
pip install --user -e .

# clone Condor_utils
cd $SCRIPT_DIR
git clone git@github.com:CMSMUSiC/Condor_utils.git

# clone and compile correctionlib
cd $SCRIPT_DIR
git clone --recursive git@github.com:cms-nanoAOD/correctionlib.git
cd correctionlib
make PYTHON=python
make install

# Create music_env.config
cd $SCRIPT_DIR
touch music_env.config
echo "" > music_env.config
echo "export CERNUSERNAME=$3" >> music_env.config
echo "export CMSSW_VER=$CMSSW_VER" >> music_env.config
echo "export MY_SCRAM_ARCH=$MY_SCRAM_ARCH" >> music_env.config
echo "export CMSSW_RELEASE_BASE=/cvmfs/cms.cern.ch/$MY_SCRAM_ARCH/cms/cmssw/$CMSSW_VER/" >> music_env.config
