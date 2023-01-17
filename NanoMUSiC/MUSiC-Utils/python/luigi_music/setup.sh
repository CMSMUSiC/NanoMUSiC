#!/bin/bash

cd -P "$( dirname "${BASH_SOURCE[0]}" )"

if [ -z "$CMSSW_RELEASE_BASE" ]; then
    echo "CMSSW must be sourced!"
    exit 1
fi

PYTHON="$CMSSW_RELEASE_BASE/external/$SCRAM_ARCH/bin/python"
PYTHONVERSION=$($PYTHON -V 2>&1)
echo "Found $PYTHONVERSION at $PYTHON"

PIP="$PYTHON -m pip"
$PIP -V 2>&1 >/dev/null
if [ $? -ne 0 ]; then
    echo "Pip not found, installing..."
    curl https://bootstrap.pypa.io/pip/2.7/get-pip.py | python - --user
fi

echo "Using $($PIP -V)"

echo "Installing Luigi..."
#$PIP install --user luigi==2.7.1
~/.local/bin/pip install --user luigi==2.7.1
