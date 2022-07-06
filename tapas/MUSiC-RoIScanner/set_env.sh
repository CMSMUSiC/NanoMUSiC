#!/usr/bin/sh
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
export MYTEventClass=$MUSIC_BASE/EventClassFactory/TEventClass.hh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MUSIC_BASE/lib
export SCAN_BASE=$DIR
export PYTHONPATH=$PYTHONPATH:$SCAN_BASE/python
#remove MUSiC bin from PATH
#PATH=$(echo $PATH | sed -e 's;:\?/home/home1/institut_3a/knutzen/MUSiC_software/Classifier_2011/MUSiC/bin;;' -e 's;/home/home1/institut_3a/knutzen/MUSiC_software/Classifier_2011/MUSiC/bin:\?;;')
#PATH=$(echo $PATH | sed -e "s;:\?$MUSIC_BASE/bin;;" -e "s;$MUSIC_BASE/bin:\?;;")
#add Scanner bin to PATH
export PATH=$PATH:$SCAN_BASE/bin;
