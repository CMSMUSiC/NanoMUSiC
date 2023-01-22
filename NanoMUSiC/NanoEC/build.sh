#!/usr/bin/env bash

echo "Building ... "

g++ -g -pthread -std=c++17 -m64 -Iinclude -I/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/include -L/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/lib -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -L/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/lib64 -lfmt -Wl,-rpath,/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/lib -pthread -lm -ldl -rdynamic -O3 NanoEC.cpp -o nanoec

echo "Done ... "