#!/bin/bash -e
# This script serves as a intermediate executable to run the PxlAnalyzer with
# its default arguments.

# source the same environment variables as in the local pxlanalyzer environment
source set_env.sh
export MYPXLANA=LEDge

# run the application
bin/music "$@"

# compress the results
tar cjf AnalysisOutput.tar.bz2 AnalysisOutput/

# return success exit code
exit 0
