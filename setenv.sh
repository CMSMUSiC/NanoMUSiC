#!/bin/bash

# This is a set_env script
SCRIPTDIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/release_packages/x86_6

echo "[ LCG View ] Using x86_64-centos7-gcc12-opt ..."
source /cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/setup.sh
echo "[ LCG View ] Done ..."


# echo "[ LCG View ] Using x86_64-centos7-gcc11-opt ..."
# source /cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc11-opt/setup.sh
# echo "[ LCG View ] Done ..."


# Set ninja as default CMake generator
export CMAKE_GENERATOR=Ninja

# set base dir
export MUSIC_BASE=$SCRIPTDIR

# Set dummy SCRAM_ARCH. needed for LHAPDF.
# export SCRAM_ARCH=slc7_amd64_gcc12
export SCRAM_ARCH=slc7_amd64_gcc11

# This is a TAPAS set_env script. Source it before usage of TAPAS.
export LD_LIBRARY_PATH=$SCRIPTDIR/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/lib/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/MUSiC-CRAB:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/MUSiC-Classification:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/MUSiC-Classification/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/NanoEventClass/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/CondorScheduler:$PYTHONPATH

export MYPXLANA=EventClassFactory

# Source all the needed set_env

cd $SCRIPTDIR/NanoMUSiC/tools/
source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/PxlAnalyzer/
source set_env.sh

# cd $SCRIPTDIR/NanoMUSiC/PlotLib/
# source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/PxlAnalyzer/EventClassFactory/
source set_env.sh

# cd $SCRIPTDIR/NanoMUSiC/MUSiC-Utils/
# source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/MUSiC-Configs/
source set_env.sh

cd $SCRIPTDIR/NanoMUSiC/MUSiC-RoIScanner/
source set_env.sh

cd $SCRIPTDIR

# set PATH
export PATH=$PATH:$SCRIPTDIR/bin;
export PATH=$PATH:$SCRIPTDIR/scripts;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/MUSiC-Classification/scripts;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/MUSiC-Classification/scripts;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/MUSiC-ClassificationUtils/scripts;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/NanoEventClass/scripts;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/MUSiC-BTagEff;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/MUSiC-CRAB;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/special_studies/jet_sf;



#setup go env
export GOPATH="$SCRIPTDIR/cache/classification_cli:$GOPATH"



# rust config
export RUSTFLAGS="-C linker=$CC"
