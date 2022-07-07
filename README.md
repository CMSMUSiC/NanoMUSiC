# MUSiC - Pxl

Run2 UL     condensed analysis code.

# Setup the environment (only once)

```
git clone git@github.com:CMSMUSiC/MUSiCPxl.git
cd MUSiCPxl
./setup_music.sh CMSSW_10_6_29 <your_CERN_username>
```

**Don't forget the set the proper CMSSW version. Could be that the one above is not updated.**

https://twiki.cern.ch/twiki/bin/view/CMS/PdmVRun2LegacyAnalysis


# Configuring the environment (for every new session)

```
source setenv_music.sh
```

# Compile the code

```
make clean
make
```

Other options are:

- `make skimmer`  
- `make utils`  
- `make rio`  
- `make pxlanalyzer`  

and

- `make skimmer_clean`  
- `make utils_clean`  
- `make rio_clean`  
- `make pxlanalyzer_clean`  

