#!/bin/bash
# Copyright [2015] <RWTH Aachen, III. Phys. Inst. A>

# Get a fixed PXL version
git clone ssh://git@gitlab.cern.ch:7999/aachen-3a-cms/cmssw-pxl.git Pxl
cd Pxl
git reset --hard 9cc840b342afb80a3e8d06945d784ef23df4784d
cd ..
