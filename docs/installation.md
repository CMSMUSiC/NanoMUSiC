
# Setup

The instructions here described are designed tested for the computing environment of RWTH-3A. In principle, it should be no problem to setup and run the analysis code at other sites, with CentOS7 and CVMFS mounted,e.g. `lxplus`.

## Dependencies

There are no dependencies on CMSSW. The needed softwares are provided by LCG views (view 102b - `/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/setup.sh`).

LCG list of available software: https://lcginfo.cern.ch/release_packages/x86_64-centos7-gcc11-opt/102b/

## Setup the environment (only once)

```
git clone git@github.com:CMSMUSiC/NanoMUSiC.git
cd NanoMUSiC
./setup_music.sh
```

## Configuring the environment (for every new session)

```
source music_setenv.sh
```

## Compilation

```
cd build
```

If it does not exists, just create it (`mkdir build`). You can play around with this directory at will.


To configure and build (and install):

```
cmake ..
ninja install
```

Other options are:

- `ninja lut` &rarr; Only creates the p-values LUTs. Usually takes some time to run, but, in principle, one should do it only right before start a classification.
- `make_docs` &rarr; Produces the documentation for github.io.
- `ninja clean` &rarr; Clear compilations. 

## Installations directories

#### Executables

`bin/`

#### Shared libraries

`libs/`

#### Other useful scripts

`scripts/`