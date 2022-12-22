#!/bin/bash
echo "================= [ BEGIN ] Input arguments =================="
echo $@
job_id=$1
# toml_config=$2 
echo "================= [ END ] Input arguments =================="

echo "================= [ BEGIN ] Unpacking input files  =================="
if [ $job_id -gt 0 ]
then
    tar -zxvf task.tar.gz
fi

echo "Current directory and its content:"
pwd
ls
echo "================= [ END ] Unpacking input files =================="

# get input files from PSet.py, modify the TOML config file and dump a dummy PSet.py
python3 NanoMUSiC/MUSiC-CRAB/config_builder.py $job_id 


# set MUSiC environment
echo "================= [ BEGIN ] MUSiC environment =================="
# This is a set_env script
SCRIPTDIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Set up the LCG environment
# List of available software:
# https://lcginfo.cern.ch/release_packages/x86_64-centos7-gcc11-opt/102B/
source  /cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/setup.sh

echo "ROOT Version"
root -b -q

export MUSIC_BASE=$SCRIPTDIR
export LD_LIBRARY_PATH=$SCRIPTDIR/lib:$LD_LIBRARY_PATH

# set PATH
export PATH=$PATH:$SCRIPTDIR/bin;
export PATH=$PATH:$SCRIPTDIR/scripts;
export PATH=$PATH:$SCRIPTDIR/NanoMUSiC/MUSiC-CRAB;

env
echo "================= [ END ] MUSiC environment =================="

# run nano_music and save its exit code
nano_music --batch --run-config config.toml
exit_code=$?

# copy outputs to current dir
cp -r output*/* .

# produce dummy FrameworkJobReport.xml
# cmsRun -j FrameworkJobReport.xml -p PSet.py
cp $SCRIPTDIR/NanoMUSiC/MUSiC-CRAB/dummy_frameworkjob_report.xml FrameworkJobReport.xml

# exit
exit $exit_code 