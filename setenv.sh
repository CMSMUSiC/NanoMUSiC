#!/bin/bash

# fix the many locale warnings
unset LC_ALL

# This is a set_env script
SCRIPTDIR=$(pwd)

# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/release_packages/x86_64-centos7-gcc11-opt/104c/

echo "[ LCG View ] Using x86_64-el9-gcc13-opt ..."
export LCGVIEW_PATH="/cvmfs/sft.cern.ch/lcg/views/LCG_106/x86_64-el9-gcc13-opt"
source $LCGVIEW_PATH/setup.sh
echo "[ LCG View ] Done ..."


# Set ninja as default CMake generator
export CMAKE_GENERATOR=Ninja

# set base dir
export MUSIC_BASE=$SCRIPTDIR

# Set dummy SCRAM_ARCH. needed for LHAPDF.
export SCRAM_ARCH=el9_amd64_gcc13

export LD_LIBRARY_PATH=$SCRIPTDIR/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$SCRIPTDIR/lib/python:$LD_LIBRARY_PATH
export PYTHONPATH=$SCRIPTDIR/lib/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/configs/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/Classification:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/Classification/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/MUSiC/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/Plotter/python:$PYTHONPATH
export PYTHONPATH=$SCRIPTDIR/NanoMUSiC/RoIScanner/python:$PYTHONPATH


# add gfal2 to PATH
export PYTHONPATH="$PYTHONPATH:/usr/lib64/python3.11/site-packages:/usr/lib/python3.11/site-packages"

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
export pybind11_DIR="/cvmfs/sft.cern.ch/lcg/views/LCG_106/x86_64-el9-gcc13-opt/lib/python3.11/site-packages/pybind11/share/cmake/pybind11"


#parallel
export PATH=$PATH:$SCRIPTDIR/external/parallel/bin

