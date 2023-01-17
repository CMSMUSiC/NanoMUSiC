#!/bin/bash

# this is not mean to be run locally
# echo Check if TTY
if [ "`tty`" != "not a tty" ]; then
  echo "YOU SHOULD NOT RUN THIS IN AN INTERACTIVE TERMINAL! IT CAN MAKE A MESS IN YOUR LOCAL FILES!"
else
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
  echo "================= [ END ] Unpacking input files =================="

  echo "================= [ BEGIN ] Build config TOML =================="
  echo "----> Current directory path:"
  pwd

  echo "----> Current directory content:"
  ls

  # copy MUSiC-CRAB directory to working path
  cp -r NanoMUSiC/MUSiC-CRAB/* .

  # get input files from PSet.py and modify the TOML config file 
  python3 config_builder.py $job_id 

  echo "\n\n\n\n --> config.toml:"
  ls config.toml
  cat config.toml
  echo "================= [ END ] Build config TOML =================="

  # set env, run nano_music and save its exit code
  SCRIPTDIR=`pwd`
  bash -c "eval \`scram unsetenv -sh\` ; echo \"================= [ BEGIN ] MUSiC environment ==================\" ; source /cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/setup.sh ; echo \"ROOT Version\"; root -b -q; export MUSIC_BASE=$SCRIPTDIR; export LD_LIBRARY_PATH=$SCRIPTDIR/lib:\$LD_LIBRARY_PATH ; export PATH=$SCRIPTDIR/bin:\$PATH ; export PATH=$SCRIPTDIR/scripts:\$PATH ; export PATH=$SCRIPTDIR/NanoMUSiC/MUSiC-CRAB:\$PATH ; export ROOT_INCLUDE_PATH=$SCRIPTDIR/NanoMUSiC/MUSiC/include:$SCRIPTDIR/NanoMUSiC/MUSiC:\$ROOT_INCLUDE_PATH  ; env; echo \"================= [ END ] MUSiC environment ==================\" ; nano_music --batch --run-config config.toml"
  exit_code=$?

  # copy outputs to current dir
  echo "================= [ BEGIN ] Outputs directory =================="
  ls outputs/*
  cp -r outputs/* .
  echo "================= [ END ] Outputs directory =================="

  # produce dummy FrameworkJobReport.xml
  # cmsRun -j FrameworkJobReport.xml -p PSet.py
  cp dummy_frameworkjob_report.xml FrameworkJobReport.xml

  # exit
  exit $exit_code 
fi