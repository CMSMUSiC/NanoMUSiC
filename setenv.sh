#!/bin/bash

# This is a set_env script
SCRIPTDIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/release_packages/x86_64-centos7-gcc11-opt/104c/

echo "[ LCG View ] Using x86_64-el9-gcc12-opt ..."
source /cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/setup.sh
echo "[ LCG View ] Done ..."

# Set ninja as default CMake generator
export CMAKE_GENERATOR=Ninja

# set base dir
export MUSIC_BASE=$SCRIPTDIR

# Set dummy SCRAM_ARCH. needed for LHAPDF.
export SCRAM_ARCH=el9_amd64_gcc12

export LD_LIBRARY_PATH=$SCRIPTDIR/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$SCRIPTDIR/lib/python:$LD_LIBRARY_PATH
export PYTHONPATH=$SCRIPTDIR/lib/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/Classification:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/Classification/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/MUSiC/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/Plotter/python:$PYTHONPATH


# add gfal2 to PATH
export PYTHONPATH="$PYTHONPATH:/usr/lib64/python3.9/site-packages:/usr/lib/python3.9/site-packages"

cd $SCRIPTDIR

# set PATH
export PATH=$PATH:$SCRIPTDIR/bin;
export PATH=$PATH:$SCRIPTDIR/scripts;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/MUSiC/scripts;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/Classification/scripts;

#setup go env
export GOPATH="$SCRIPTDIR/cache:$GOPATH"
export GOROOT="$SCRIPTDIR/opt/go"
export PATH="$SCRIPTDIR/opt/go/bin:$PATH" 
export GOBIN="$SCRIPTDIR/bin"

# rust config
export RUSTFLAGS="-C linker=$CC"

#pybind11 CMAKE files
export pybind11_DIR="/cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/lib/python3.9/site-packages/pybind11/share/cmake/pybind11"
