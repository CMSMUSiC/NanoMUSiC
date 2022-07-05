# PxlSkimmer
No Taus available yet! Wait for next version!
Adjust Quick installation to 106X for Legacy Full Run 2.

## Quick Installation

```bash
# Setup and source CMSSW
source /cvmfs/cms.cern.ch/cmsset_default.sh

# Get architecture
if [[ $SCRAM_ARCH == *"slc6"* ]]; then
  echo "SLC6 Architecture. Using CMSSW_9_4_X."
  cmsrel CMSSW_9_4_14
  cd CMSSW_9_4_14/src
elif [[ $SCRAM_ARCH == *"slc7"* ]]; then
  echo "SLC7 Architecture. Using CMSSW_10_6_X."
  cmsrel CMSSW_10_6_20
  cd CMSSW_10_6_20/src
else
  echo "Neither sl6 or sl7. Confused..."
fi

cmsenv

if [[ $CMSSW_VERSION == "CMSSW_9_4_14" ]]; then
  git cms-merge-topic cms-met:METFixEE2017_949_v2
else
  git cms-merge-topic cms-met:METFixEE2017_949_v2_backport_to_102X
fi

# Add EGamma scale and smearing for 2018
# Some steps are commented out because no longer needed for UltraLegacy
git cms-merge-topic cms-egamma:EgammaPostRecoTools #just adds in an extra file to have a setup function to make things easier 
#git cms-merge-topic cms-egamma:PhotonIDValueMapSpeedup1029 #optional but speeds up the photon ID value module so things fun faster
#git cms-merge-topic cms-egamma:slava77-btvDictFix_10210 #fixes the Run2018D dictionary issue, see https://github.com/cms-sw/cmssw/issues/26182, may not be necessary for later releases, try it first and see if it works
#now to add the scale and smearing for 2018 (eventually this will not be necessary in later releases but is harmless to do regardless)
git cms-addpkg EgammaAnalysis/ElectronTools
rm EgammaAnalysis/ElectronTools/data -rf
git clone git@github.com:cms-data/EgammaAnalysis-ElectronTools.git EgammaAnalysis/ElectronTools/data

# Get high pT electron MET fix*#
#git cms-merge-topic Sam-Harper:HighPtEleMETPatch_10216



# Skimmer setup
git clone ssh://git@gitlab.cern.ch:7999/aachen-3a/PxlSkimmer.git PxlSkimmer/
cd PxlSkimmer
# Next step needs the correct tag for the year you want to skim!
# git checkout tags/CMSSW_10_6_v5p10_201x
./setup.sh
cd ..

# Compile the code
scram build -j 8
```

## Installation
Before installing the PxlSkimmer, one has to setup CMSSW.

When CMSSW is set up and sourced, one can either clone the PxlSkimmer repository
into `${CMSSW_BASE}/src` or add a CMS package to set up the CMSSW git
infrastructure. As a recommended package to install the CMSSW git
infrastructure, the `PatAlgos` can be added by calling `git cms-addpkg
PhysicsTools/PatAlgos` in the `${CMSSW_BASE}/src` directory. To clone the
PxlSkimmer one has to call `git clone ssh://git@gitlab.cern.ch:7999/aachen-3a/PxlSkimmer.git`

If one wants to skim samples, the specific tag has to be checked out via `git
checkout tags/<tag-name>`. Otherwise developing new features or applying fixes
can be done on the master branch. As the final step, the `setup.sh` script
inside the PxlSkimmer directory has to be executed, which will take care of the
remaining additions required to run the skimmer.

## Using the Skimmer on the GRID
The submission of Jobs to the GRID can be done using the script music_crab3, see https://gitlab.cern.ch/aachen-3a/tools/wikis/music_crab3/
# Coding conventions
TODO
