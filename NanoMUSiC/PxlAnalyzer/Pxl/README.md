# BE CAREFUL! OLD STUFF LIES HERE...
# BE CAREFUL! OLD STUFF LIES HERE...
# BE CAREFUL! OLD STUFF LIES HERE...
## cmssw-pxl

PXL for CMSSW.


### Note

CMSSW forces modules to be placed in a so-called *subsystem*, i.e. an additional directory within the *src* directory of the CMSSW installation. The name of the subsystem is then hard-coded in internal ``include`` statements, e.g.:

```cpp
#include "MySubsystem/MyModule/interface/module.h"
```

The subsystem of the PXL library for CMSSW is also called ``Pxl`` so it is strongly recommended to clone this repository as:

```bash
addr=ssh://git@gitlab.cern.ch:7999/aachen-3a-cms/cmssw-pxl.git
git clone $addr Pxl --branch <tagName> [--depth 1]
```
