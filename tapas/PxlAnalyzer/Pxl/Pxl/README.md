# BE CAREFUL! OLD STUFF LIES HERE...
# BE CAREFUL! OLD STUFF LIES HERE...
# BE CAREFUL! OLD STUFF LIES HERE...
## How does ``scripts/bundlePxl`` work?

``bundlePxl`` copies all required pxl files to the interace/ and src/ directories. It also changes ``#include`` statements within each file in order to satisfy CMSSW standards concerning file inclusions. Simply pass the directory of the pxl repository as an argument:

> ./scripts/bundlePxl ~/path/to/pxl

If you want to cleanup all bundled data, type:

> ./scripts/cleanup
