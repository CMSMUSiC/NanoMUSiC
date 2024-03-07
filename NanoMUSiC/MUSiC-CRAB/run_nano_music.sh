#!/bin/bash

# this is not mean to be run locally
# echo Check if TTY
if [ "`tty`" != "not a tty" ]; then
  echo "YOU SHOULD NOT RUN THIS IN AN INTERACTIVE TERMINAL! IT CAN MAKE A MESS IN YOUR LOCAL FILES!"
  exit 42
else
  echo "================= [ BEGIN ] Input arguments =================="
  echo $@
  job_id=$1
  echo "================= [ END ] Input arguments =================="

  echo "================= [ BEGIN ] Build config TOML =================="
  echo "----> Current directory path:"
  pwd

  echo "----> Current directory content:"
  ls

  # get input files from PSet.py and modify the TOML config file 
  python3 config_builder.py $job_id 

  echo "--> config.toml:"
  ls config.toml
  cat config.toml
  echo ""
  echo "================= [ END ] Build config TOML =================="

  echo "================= [ BEGIN ] MUSiC environment ==================" 
  echo ROOT Version
  root -b -q
  env
  echo ================= [ END ] MUSiC environment ==================

  echo "================= [ BEGIN ] NanoAOD Skimmer ==================" 
  ./nanoaod_skimmer
  EXIT_CODE=$?
  echo "================= [ END ] NanoAOD Skimmer ==================" 

  if [[ "$EXIT_CODE" -eq 0 ]]; then
      echo "Yay! Task finished succesfully."
  else 
      echo "ERROR: Finished nanoaod_skimmer with exit code: $EXIT_CODE"
      exit 42
  fi

  # check the status of the outputs 
  echo "NanoMUSiC Tree ..."
  root -b nano_music.root -e  "_file0->Get<TTree>(\"Events\")->Print(\"all\")" -q
  if [[ "$?" -eq 0 ]]; then
      echo "Yay! Events tree is available."
  else 
      exit 42
  fi
  echo "================= [ END ] Outputs directory =================="

  # produce dummy FrameworkJobReport.xml
  cp dummy_frameworkjob_report.xml FrameworkJobReport.xml

  # exit
  exit 0 
fi
