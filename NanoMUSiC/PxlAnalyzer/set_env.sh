#!/usr/bin/sh
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
export PXLANALYZER_BASE=$DIR
export PXLANA=$DIR
export PATH=$PATH:$PXLANALYZER_BASE/bin;
export PYTHONPATH=$PXLANALYZER_BASE/python:$PYTHONPATH

# if [ "$SCRAM_ARCH" = "slc7_amd64_gcc700" ]; then
#   export LHAPDF=/cvmfs/cms.cern.ch/slc7_amd64_gcc700/external/lhapdf/6.2.1-omkpbe2
# elif [ "$SCRAM_ARCH" = "slc6_amd64_gcc700" ]; then
#   export LHAPDF=/cvmfs/cms.cern.ch/slc6_amd64_gcc700/external/lhapdf/6.2.1-omkpbe2
# elif [ "$SCRAM_ARCH" = "slc7_amd64_gcc530" ]; then
#   export LHAPDF=/cvmfs/cms.cern.ch/slc7_amd64_gcc530/external/lhapdf/6.1.5-kpegke2
# elif [ "$SCRAM_ARCH" = "slc6_amd64_gcc530" ]; then
#   export LHAPDF=/cvmfs/cms.cern.ch/slc6_amd64_gcc530/external/lhapdf/6.1.5-kpegke2
# elif [ "$SCRAM_ARCH" = "slc7_amd64_gcc11" ]; then
#   export LHAPDF=/cvmfs/cms.cern.ch/slc7_amd64_gcc11/external/lhapdf/6.4.0-2ccbff2a6e913e31ee74744d079bab2f
  
# else
#   echo "WARNING: Unknown SCRAM_ARCH or no SCRAM_ARCH set. Defaulting to slc7_amd64_gcc530. This might not work for other SCRAM_ARCH!!!"
#   export LHAPDF=/cvmfs/cms.cern.ch/slc7_amd64_gcc530/external/lhapdf/6.1.5-kpegke2
# fi
# export LHAPATH=/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current/:$LHAPDF/share/LHAPDF/
# export LD_LIBRARY_PATH=$LHAPDF/lib:$LD_LIBRARY_PATH
