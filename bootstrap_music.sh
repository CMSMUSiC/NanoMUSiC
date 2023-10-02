#!/bin/bash

# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/release_packages/x86_64-centos7-gcc11-opt/102b/

echo "[ LCG View ] Using x86_64-centos7-gcc12-opt ..."
source /cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/setup.sh
echo "[ LCG View ] Done ..."

# echo "[ LCG View ] Using x86_64-centos7-gcc11-opt ..."
# source /cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc11-opt/setup.sh
# echo "[ LCG View ] Done ..."

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# create links for libaries
mkdir -p lib/python
# ln -s /usr/lib64/libdavix.so.0 lib/libdavix.so.0
# ln -s /usr/lib64/python2.6/lib-dynload/_curses.so lib/python/_curses.so

# FIX ME: is it corerct???
# ln -s /cvmfs/cms.cern.ch/slc6_amd64_gcc493/external/py2-pycurl/7.19.0-kpegke/lib/python2.7/site-packages/pycurl.so lib/python/pycurl.so

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
