#!/usr/bin/env bash
export TOOLS3A="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# please use TOOLS3A, LIBS3A is deprecated
export LIBS3A=$TOOLS3A
export PATH=${TOOLS3A}/scripts:${PATH}
export PYTHONPATH=${TOOLS3A}/python:${PYTHONPATH}
for dir in `find $TOOLS3A -name 'lib'`
  do
  export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${dir}
done

# Source grid tools (gfal etc.) from cvmfs for sl6 environments
if [[ `uname -a` = *.el6.* ]] ; then
    echo "sourcing from cvmfs"
    source /cvmfs/grid.cern.ch/umd-sl6ui-latest/etc/profile.d/setup-ui-example.sh
fi
