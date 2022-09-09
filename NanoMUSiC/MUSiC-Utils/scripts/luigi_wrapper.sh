#!/bin/bash

function exec_in_clean_env() {
    env -i bash --login -c $*
}

if [ -z "$*" ]; then
    echo "Usage: $0 action args" 1>&2
    exit 1
fi

ACTION=$1
shift
ARGS=$*

case $ACTION in
    classify)
      REAL_TASK=ECPlotTask
      ;;
    classify-process-group)
      REAL_TASK=SampleGroupMergeTask
      ;;
    classify-all-process-groups)
      REAL_TASK=AllSampleGroupsMergeTask
      ;;
    classify-process-sample)
      REAL_TASK=SampleMergeTask
      ;;
    scan)
      REAL_TASK=ScanTask
      ;;
    scan-integral)
      REAL_TASK=SingleBinScanTask
      ;;
    *)
      echo "Usage: $0 action [args]"
      echo "Available commands:"
      echo "classify, classify-process-group, classify-sample, scan, scan-integral"
      echo "No valid command specified, exiting"
      exit 0
      ;;
esac

LUIGI_PXLANALYZER_BASE=$MUSIC_UTILS/python/luigi_music/

CMD="luigi --module luigi_music.tasks $REAL_TASK $ARGS"
echo "Executing $CMD..."

$CMD
