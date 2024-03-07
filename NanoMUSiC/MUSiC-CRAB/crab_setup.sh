# setup CMSSW

SCRIPTDIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

source /cvmfs/cms.cern.ch/common/crab-setup.sh

# export SCRAM_ARCH=el9_amd64_gcc12
export SCRAM_ARCH=el8_amd64_gcc12
CMSSW_VERSION="CMSSW_14_0_1"

if [ -d "$CMSSW_VERSION" ]; then
  echo "$CMSSW_VERSION area found exists."
else
  echo "$CMSSW_VERSION area does not exist. Creating one..."
  cmsrel $CMSSW_VERSION
  echo "... done."
fi

cd $CMSSW_VERSION/src
cmsenv

cd $SCRIPTDIR

export PATH="$SCRIPTDIR:$PATH"
export CRAB_MUSIC_BASE=$SCRIPTDIR

#setup go env
export GOPATH="$SCRIPTDIR/../../cache:$GOPATH"
export GOROOT="$SCRIPTDIR/../../opt/go"
export PATH="$SCRIPTDIR/../../opt/go/bin:$PATH" 
export GOBIN="$SCRIPTDIR/../../bin"
