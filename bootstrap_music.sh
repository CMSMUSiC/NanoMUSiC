#!/bin/bash

# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/release_packages/x86_64-centos7-gcc11-opt/104c/

echo "[ LCG View ] Using x86_64-el9-gcc13-opt ..."
source /cvmfs/sft.cern.ch/lcg/views/LCG_104c/x86_64-el9-gcc13-opt/setup.sh
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


#install parallel with lastest version
cd $SCRIPT_DIR/bin 
wget https://git.savannah.gnu.org/cgit/parallel.git/plain/src/parallel 
mv parallel parallel_music
chmod +x parallel_music


cd $SCRIPT_DIR
