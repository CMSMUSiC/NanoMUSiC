# setup CMSSW
# cd /cvmfs/cms.cern.ch/slc7_amd64_gcc11/cms/cmssw/CMSSW_12_3_4 ; cmsenv ; cd - > /dev/null

source /cvmfs/cms.cern.ch/common/crab-setup.sh

export SCRAM_ARCH="slc7_amd64_gcc10"

DIR="CMSSW_12_3_4"
if [ -d "$DIR" ]; then
  echo "CMSSW release area found ..."
else
  echo "CMSSW release area not found. Will be created ..."
  cmsrel CMSSW_12_3_4
fi
 
cd CMSSW_12_3_4/src
cmsenv
pwd
cd ../..

export PATH="$PWD:$PATH"
export CRAB_MUSIC_BASE=$PWD