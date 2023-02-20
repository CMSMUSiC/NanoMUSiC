#!/usr/bin/env bash

cd $MUSIC_BASE
cd docs

$MUSIC_BASE/docs/parseMD.sh raw_index.md > index.md

doxygen
cd -