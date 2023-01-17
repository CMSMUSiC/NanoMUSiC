## The Region of Interest (RoI) Scanner

The applications in this repository allow to search for regions of interest in frequency distributions.

### Running the scanClass executable
The main executable is scanClass and is used to scan for a RoI in a single distribution.
```scanClass inputJSON```
You may also set the number of pseudo experiments with the ``` -m  ``` option.

### Input for scanClass
The input information for scanClass is given as a json file.
The tool testJson.py is placed in /python and allows to create a dummy input file for basic testing. You may use it to find out how the input file needs to be structured.

#### Some general explanation about inputs in json file
The following json objects are part of the root json object
* MCBins: A list of objects containing all informatin for each monte carlo bin (lowerEdge, width, weighted number entries, unweighted         events and xs by process, systematic shifted number of weighted entries)
* DataBins (optional): A list of objects with entry numbers for each bin ( it is assumed that a corresponding entry exists in MCBins). Only one RoI is calculated if this option is used.
* Seeds (optional): A json object with structue systName : seed . The seeds are used to initalize a random number generator for each systematic type. This option is only needed if pseudo experiment dicing should be performed. The resulting json file contains information about the RoI for each diced pseudo-experiment.



