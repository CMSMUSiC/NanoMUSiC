# setup CMSSW

set -e

source /cvmfs/cms.cern.ch/common/crab-setup.sh

# export SCRAM_ARCH="slc7_amd64_gcc10"
export SCRAM_ARCH=el9_amd64_gcc12

DIR="CMSSW_13_3_1"
if [ -d "$DIR" ]; then
  echo "CMSSW release area found ..."
else
  echo "CMSSW release area not found. Will be created ..."
  cmsrel CMSSW_13_3_1
fi
 
cd CMSSW_13_3_1/src
cmsenv
pwd
cd ../..

export PATH="$PWD:$PATH"
export CRAB_MUSIC_BASE=$PWD
