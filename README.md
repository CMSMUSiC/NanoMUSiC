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



## Setup

#### Clone

```bash
git clone git@github.com:CMSMUSiC/NanoMUSiC.git
cd NanoMUSiC
```

#### Bootstrap

Only once.

```bash
./bootstrap_music.sh
```

#### Setup environment

For every session.

```bash
source setenv.sh
```

## Compilation

Compilation is always done within the `build` directory.

Setup cmake compilation files (only once or if cmake files have changed).
```
cd $MUSIC_BASE/build
cmake ..
```

Compile libraries and executables.

```bash
ninja
```

Copy them to appropriate directories (`bin` and `lib`).

```bash
ninja install
```


Generate p-values LUT (only once).

```bash
ninja lut
```
## Classification 

### Preparation

#### GRID proxy

```bash
voms-proxy-init --voms cms  -valid 192:00
```

#### Parse
Preparing the inputs and running the code, ideally should by done from a local disk (`/disk1`). Create one folder for each classification iteration. Try to keep control of disk usage.

Create `parsed_sample_list.toml` using `sample.txt` (list of DAS names of all the Data and MC samples to be used) and `configs/xsections_and_lumi/xSections.toml` (list of xSections).


```bash
music classification parse samples.txt
```

Example of samples.txt. empty lines and lines starting with `#` are ignored.
```
/TTZToLL_5f_TuneCP5_13TeV-madgraphMLM-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM
/TTGG_TuneCP5_13TeV-amcatnlo-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM

/TTWW_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM

#/ZToMuMu_M-120To200_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM
#/ZToMuMu_M-200To400_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM
#/ZToMuMu_M-400To800_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM
#/ZToMuMu_M-800To1400_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM
#/ZToMuMu_M-1400To2300_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM
#/ZToMuMu_M-2300To3500_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM
#/ZToMuMu_M-3500To4500_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM
#/ZToMuMu_M-4500To6000_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM
#/ZToMuMu_M-6000ToInf_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM


/TTZZ_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM

/TTTT_TuneCP5_13TeV-amcatnlo-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v2/NANOAODSIM
```



#### Build

Will query all file's Logical File Name (LFN) from DAS, for each sample and add it to the analysis_config.toml.

```bash
music classification build parsed_sample_list.toml
```


#### Preprocess

Will scan all MC input files and collect the total MC weight per sample. Produces `sum_weights.json`.

```bash
music classification preprocess analysis_config.toml
```

Snippet of `sum_weights.json` for `DYToEE_M-1400To2300_13TeV_PH`.
```json
    "DYToEE_M-1400To2300_13TeV_PH": {
        "2016APV": {
            "sum_genWeight": 3.422499867156148,
            "sum_LHEWeight": 3.422499867156148,
            "raw_events": 2500,
            "has_genWeight": true,
            "has_LHEWeight_originalXWGTUP": true
        },
        "2018": {
            "sum_genWeight": 6.844999734312296,
            "sum_LHEWeight": 6.844999734312296,
            "raw_events": 5000,
            "has_genWeight": true,
            "has_LHEWeight_originalXWGTUP": true
        },
        "2016": {
            "sum_genWeight": 3.422499867156148,
            "sum_LHEWeight": 3.422499867156148,
            "raw_events": 2500,
            "has_genWeight": true,
            "has_LHEWeight_originalXWGTUP": true
        },
        "2017": {
            "sum_genWeight": 6.844999734312296,
            "sum_LHEWeight": 6.844999734312296,
            "raw_events": 5000,
            "has_genWeight": true,
            "has_LHEWeight_originalXWGTUP": true
        }
    },

```


#### Launch Classification

##### Development mode

Will launch a classification job, running over only one sample of a given year. Useful for debugging.

```bash
music classification launch --target dev \
                            --process TTToSemiLeptonic_13TeV_PH \
                            --year 2018 analysis_config.toml \
                            --max-files 1
```

The command above will classify events from the first file of `TTToSemiLeptonic_13TeV_PH` (year `2018`). The produced outputs are:

```
TTToSemiLeptonic_13TeV_PH_2018_0.root
validation_TTToSemiLeptonic_13TeV_PH_2018_0.root
```

They contain the validation and classification results.

##### Full classification

Will launch the classification over all samples and years, using GNU parallel.

```bash
music classification launch 
```


## License

All the credit goes, to the authors of the legacy repositories.

The LICENSING inherits from them, also. When not this is not applicable, MIT license applies.
