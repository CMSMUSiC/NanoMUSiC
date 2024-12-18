#  NanoMUSiC

![music](https://raw.githubusercontent.com/CMSMUSiC/NanoMUSiC/main/docs/images/music.svg)


**MUSiC - Model Unspecific Search in CMS**

Analysis framework for Run2-UL ( and Run3 ? ) using NanoAOD.

Last poublication (Run2016): [Eur. Phys. J. C 81 (2021) 629](http://dx.doi.org/10.1140/epjc/s10052-021-09236-z)


## About


This repository stands on the shoulders of the work done by many brilliant students and scientists of III. Physikalisches Institut A und B (RWTH Aachen University), that along many years touched this code. Their contributions are mainly hosted in the legacy repositories below:

- https://gitlab.cern.ch/aachen-3a/tapas
- https://gitlab.cern.ch/aachen-3a/pxlanalyzer
- https://gitlab.cern.ch/aachen-3a/PxlSkimmer
- https://gitlab.cern.ch/MUSiC/MUSiC-EventClassFactory
- https://gitlab.cern.ch/MUSiC/MUSiC-Configs
- https://gitlab.cern.ch/MUSiC/MUSiC-Utils
- https://gitlab.cern.ch/MUSiC/MUSiC-RoIScanner
- https://gitlab.cern.ch/aachen-3a/lstar-analyzer/-/tree/ghosh_202107_updates

This repo is just a convolution of the algorithms and tools mentioned above, including:
- modernized C++ and Python code;
- GPU implementations of computing demanding procedures;
- simpler workflows and environment setups;
- adaptations to the NanoAOD format, supported by the CMS experiment.

### License

All the credit goes, to the authors of the legacy repositories.

The LICENSING inherits from them, also. When not this is not applicable, MIT license applies.


## Setup

In order to setup de environment, just clone the repository and run the bootstrap script.

```
git clone git@github.com:CMSMUSiC/NanoMUSiC.git
cd NanoMUSiC
./bootstrap_music.sh
```

For every new session you should setup the environment.

```
source setenv.sh
```
st public result":