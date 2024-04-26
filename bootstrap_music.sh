#!/bin/bash

# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/

echo "[ LCG View ] Using x86_64-el9-gcc12-opt ..."
source /cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/setup.sh
echo "[ LCG View ] Done ..."

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# create links for libaries
mkdir -p lib/python

# setup table2latex
cd $SCRIPT_DIR/NanoMUSiC/MUSiC-RoIScanner
cd table2latex
pip install --user -e .

cd $SCRIPT_DIR

# install poxy (documentation generation manager, based on doxygen)
pip install --user poxy

# install atlasplot
pip install --user atlasplots

# install ray
python3 -m pip install --user "ray[default]"


# install CLI helpers and tools 
python3 -m pip install --user typer
python3 -m pip install --user pydantic


#install parallel with lastest version
cd $SCRIPT_DIR/bin 
wget https://git.savannah.gnu.org/cgit/parallel.git/plain/src/parallel 
mv parallel parallel_music
chmod +x parallel_music

cd $SCRIPT_DIR

#install go
mkdir -p opt
cd $SCRIPT_DIR/opt 
rm -rf go 
wget https://go.dev/dl/go1.22.0.linux-amd64.tar.gz 
tar -zvxf go1.22.0.linux-amd64.tar.gz 
rm go1.22.0.linux-amd64.tar.gz 


# install redis-server

cd $SCRIPT_DIR
