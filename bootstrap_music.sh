#!/bin/bash


# set -e

# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/

echo "[ LCG View ] Using x86_64-el9-gcc13-opt ..."
source /cvmfs/sft.cern.ch/lcg/views/LCG_106/x86_64-el9-gcc13-opt/setup.sh
echo "[ LCG View ] Done ..."

SCRIPT_DIR=$(pwd)

# create links for libaries
mkdir -p lib/python

# create the build directory
mkdir -p build

cd $SCRIPT_DIR

# install atlasplot
pip install --user atlasplots

# install ray
# python3 -m pip install --user "ray[default]"


# install CLI helpers and tools 
python3 -m pip install --user typer
python3 -m pip install --user pydantic

cd $SCRIPT_DIR

# install htcondor
python3 -m pip install --user htcondor

# install dbs3-client
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/DBS3APIInstructions
# https://cmsweb.cern.ch/dbs/prod/global/DBSReader/
python3 -m pip install --user dbs3-client


cd $SCRIPT_DIR
