### Electron Trigger Scale Factors

Run2 Electron Trigger Scale Factors were only provided in the form of a ROOT file. In order to avoid their inclusion in the git repo (it is a very bad pratice to add binary files to git repositories), they were dumped to HEX using `xxd`.

```
xxd -i root_file_name.root > foo.hpp
```

The `-i` option dumps the file content inthe form of a C/C++ library that can be included. The `CorrectionSets.hpp` have functions/methods to properly load those dumps into a `TMemFile`, which has the same interface of a regular file.

Documentation and references:

 - EGamma Run2 recommendations: https://twiki.cern.ch/twiki/bin/view/CMS/EgammaRunIIRecommendations#E_gamma_IDs
 - EGamma trigger SF: https://twiki.cern.ch/twiki/bin/view/CMS/EgHLTScaleFactorMeasurements
 - Low Pt SFs: https://indico.cern.ch/event/1146225/contributions/4835158/attachments/2429997/4160813/HLTSFsWprime%20.pdf
 - High Pt SFs: https://indico.cern.ch/event/787353/#1-hlt-efficiency-measurement-f
