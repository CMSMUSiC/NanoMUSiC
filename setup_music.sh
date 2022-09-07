#!/bin/bash

# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/release_packages/x86_64-centos7-gcc11-opt/102/
source /cvmfs/sft.cern.ch/lcg/views/LCG_102/x86_64-centos7-gcc11-opt/setup.sh

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# create links for libaries
mkdir -p lib/python
ln -s /usr/lib64/libdavix.so.0 lib/libdavix.so.0
ln -s /usr/lib64/python2.6/lib-dynload/_curses.so lib/python/_curses.so
ln -s /cvmfs/cms.cern.ch/slc6_amd64_gcc493/external/py2-pycurl/7.19.0-kpegke/lib/python2.7/site-packages/pycurl.so lib/python/pycurl.so

# setup table2latex
cd $SCRIPT_DIR/NanoMUSiC/MUSiC-RoIScanner
cd table2latex
pip install --user -e .

cd $SCRIPT_DIR


