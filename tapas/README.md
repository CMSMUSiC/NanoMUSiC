# BE CAREFUL! OLD STUFF LIES HERE...
# BE CAREFUL! OLD STUFF LIES HERE...
# BE CAREFUL! OLD STUFF LIES HERE...

# TAPAS
Three A Physics Analysis Software

## Quick Setup
Assuming you already have setup your SSH keys with GitLab, you can simply execute the following commands:
```bash
git clone ssh://git@gitlab.cern.ch:7999/aachen-3a/tapas.git TAPAS
cd TAPAS
./bootstrap_tapas.sh
```

## Setup TAPAS
Before you can clone the TAPAS repository, you have to setup your SSH keys on GitLab. More information on that can be found at: https://gitlab.cern.ch/help/ssh/README.md

Afterwards, clone the TAPAS repository
```bash
git clone ssh://git@gitlab.cern.ch:7999/aachen-3a/tapas.git TAPAS
```
and run the bootstrap_tapas.sh script inside. Running this script will install a large chunk of TAPAS, namely the following parts:
* Documentation - DoxyGen generated documentation of the code
* PxlAnalyzer - Basic machinery to run analyses on skimmed files. You can implement your own analysis here.
* tools - Common tools and libraries necessary for the framework
* PlotLib - Ploting lib for custom plotting apps


### Disabling TAPAS features
If you do not wish to use the plotting library or have the analyzer installed somewhere else, you can veto these features using the following command line options:
* -p no PlotLib
* -a no PxlAnalyzer

## Using TAPAS
Each repository contains a set_env script which you should source before using it. 
To make your live easier TAPAS creates a script called setenv_tapas.sh when you run the inital setup script.
You simply need to source this script and set_env scripts in all installed repos will also be sourced. 
If something does not work please [create an issue](https://its.cern.ch/jira/secure/CreateIssue!default.jspa) in JIRA and use a matching component.
A component lead will be informed and will organize a fix.

## Contributing to TAPAS
Please follow the [contribution guide for TAPAS](https://gitlab.cern.ch/aachen-3a/tapas/blob/master/CONTRIBUTING.md). If you have questions
or experience issues when following the guide, please ask an experienced developer before you proceed ! 

## Godperson System
Last Jamboree we agreed to have the following godpersons

| Object    | Person |
|-----------|--------|
| Electrons | Erdweg |
| Muons     | Radziej, Millet |
| Photons   | Endres |
| Jets      | Albert|
| Tau       | Mukherjee |
| MET       | Bispinck |
| PDF       | Erdweg, Brodski |
| GenInfo   | Esch |
| JEC       | Millet |
| Aix3adb   | Kutzner |
| Crab      | Esch, Pook |
| Cesubmit  | Albert, Lieb |
| Validation| Lieb |
| Plotlib   | Keller |




