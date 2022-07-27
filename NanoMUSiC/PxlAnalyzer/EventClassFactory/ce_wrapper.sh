#!/bin/env sh

#~ eval "$(cat ./set_env.sh)"
source ./set_env.sh
source ./EventClassFactory/set_env.sh
export MUSIC_CONFIG=$PWD
echo 'do we find PXLANA ?'
echo $PXLANA
echo 'do we find PWD'
echo $PWD
#ldd MUSiC/music
echo $@
MUSiC/music "$@" || { echo 'running music failed' ; exit 1; }

tar czf MusicOutDir.tar.gz AnalysisOutput
