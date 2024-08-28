crab_sub = r"""from CRABClient.UserUtilities import config

config = config()

config.General.requestName = "MUSIC_CLASSIFICATION"
config.General.transferOutputs = True
config.General.transferLogs = False

config.JobType.pluginName = "PrivateMC"
config.JobType.psetName = "pset.py"
config.JobType.scriptExe = "music.sh"
config.JobType.disableAutomaticOutputCollection = True
config.JobType.inputFiles = ["shifts.json", "temp_scan_cmssw_config_files.tar.gz", "___LUT___"]
config.JobType.outputFiles = ["scan_results.tar"]
# config.JobType.maxMemoryMB = 3000

config.Data.splitting = "EventBased"
config.Data.unitsPerJob = 1
# config.Data.totalUnits = 30
config.Data.totalUnits = ___NUM_JOBS___
config.Data.publication = False
config.Data.outputDatasetTag = config.General.requestName
config.Data.outLFNDirBase = "/store/user/___CMS_USER___/music"

config.Site.storageSite = "T2_DE_RWTH"
config.Site.blacklist = [
    "T3_*",
    "T2_BR_*",
    # "T2_US_Florida",
    # "T2_EE_Estonia",
    # "T2_TW_NCHC",
    # "T2_US_UCSD",
]
# config.Site.whitelist = [
#     "T2_DE_RWTH",
#     "T2_DE_DESY",
#     "T2_CH_CERN",
# ]
"""

pset = r"""
import FWCore.ParameterSet.Config as cms

process = cms.Process("SCANNER")

process.source = cms.Source("EmptySource")
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(0))

process.p = cms.Path()
# process.ep = cms.EndPath()
"""

crab_worker = r"""#!/usr/bin/env bash

# colors
BOLD_BLUE='\033[1;34m'
BOLD_GREEN='\033[1;32m'
BOLD_RED='\033[1;31m'
BOLD_WHITE='\033[1;37m'
BOLD_YELLOW='\033[1;33m'
RESET='\033[0m'

#reset colors
echo -e "${RESET} "


# Spinner function
spinner() {
    local spinstr='|/-\'
    while true; do
        for (( i=0; i<${#spinstr}; i++ )); do
            printf "[%c]  " "${spinstr:$i:1}"
            sleep 0.1
            printf "\b\b\b\b\b\b"
        done
    done
}


echo -e "${BOLD_BLUE}[CRAB Babysitter]${RESET} Starting ... "
DIR="./crab_MUSIC_CLASSIFICATION"

# Check if the directory exists
if [ -d "$DIR" ]; then
    echo ""
    echo -e "${BOLD_YELLOW}[CRAB Babysitter] WARNING: CRAB task directory already exists. Will skip submission.${RESET}"
    echo ""
else
    echo -e "${BOLD_BLUE}[CRAB Babysitter]${RESET} Launching task ... "
    crab submit crab_sub.py

    if [ $? -eq 0 ]; then
        echo -e "${BOLD_GREEN}[CRAB Babysitter] Submission succeeded.${RESET}"
    else
        echo -e "${BOLD_RED}[CRAB Babysitter] Submission failed.${RESET}"
        exit 1
    fi
fi

START=$(date)
COMMAND="crab status -d crab_MUSIC_CLASSIFICATION"
FAILED_PATTERN="   failed   "

echo -e "${BOLD_BLUE}[CRAB Babysitter]${RESET} Checking status ..."
echo ""
while true; do
    OUTPUT=$($COMMAND)
    
    echo -e "$OUTPUT"

    if echo "$OUTPUT" | grep Jobs | grep status | grep finished | grep -q '100.0%'; then
        echo -e "${BOLD_GREEN}[CRAB Babysitter] All finished.${RESET}"
        break
    fi

    if [[ $OUTPUT =~ $FAILED_PATTERN ]]; then
        echo ""
        echo -e "${BOLD_YELLOW}[CRAB Babysitter] Failed jobs:${RESET}"
	crab status --long | grep failed | grep -v Warning
        echo ""
        echo -e "${BOLD_YELLOW}[CRAB Babysitter] Resubmitting failed jobs ...${RESET}"
	crab resubmit
    fi

    echo -e "${BOLD_BLUE}[CRAB Babysitter]${RESET} Waiting 2 minutes before next check ..."
    spinner &
    spinner_pid=$!
    sleep 120
    kill $spinner_pid
    wait $spinner_pid 2>/dev/null

    echo ""
    echo -e "${BOLD_BLUE}[CRAB Babysitter]${RESET} Running since: $START"
    echo ""
done


echo ""
echo -e "${BOLD_GREEN}[CRAB Babysitter] Done!!${RESET}"
"""

music_sh = r"""#!/usr/bin/env bash

# set -e

echo "================= MUSiC script (BEGIN) ===================="
cat music.sh
echo "================= MUSiC script (END) ======================"

echo "================= MUSiC starting ===================="
pwd
ls

tar -zxf temp_scan_cmssw_config_files.tar.gz
if [ $? -ne 0 ]; then
    echo "The command tar failed."
    exit 1
fi

# Check if CRAB_Id is set
if [ -z "${CRAB_Id}" ]; then
    echo "Error: CRAB_Id is not set. Please set the CRAB_Id environment variable."
    exit 1
fi

echo "--> CRAB_Id is: $CRAB_Id"

max_retries=100
for ((i=1; i<=max_retries; i++)); do
    sleep_time=$((RANDOM % 120 + 1))
    echo "Attempt $i: Trying in $sleep_time seconds..."
    sleep $sleep_time

    # Attempt to download the file
    xrdcp -s -f root://eoscms.cern.ch///eos/cms/store/user/ftorresd/cmsmusic/scan_results.tar . 
    if [ $? -eq 0 ]; then
        echo "Download done."
        break
    fi
done
if [ $i -gt $max_retries ]; then
    echo "Download failed."
    exit 1
fi

# Final check if the download was successful
if [ ! -f "scan_results.tar" ]; then
    echo "Download failed after $max_retries attempts."
    exit 1
fi

tar -xf scan_results.tar
if [ $? -ne 0 ]; then
    echo "The command tar failed."
    exit 1
fi

ls -lha
if [ $? -ne 0 ]; then
    echo "The command ls failed."
    exit 1
fi

pattern="temp_scan_cmssw_config_files/*.py"

total_file_counter=1
file_counter=1
chunk_counter=1  

for file in $pattern
do
    if [[ -f "$file" ]]; then
        echo "*********************************************************"
        echo "File counter: $file_counter"
        echo "Chunk counter: $chunk_counter"
        echo "Total file counter: $total_file_counter"

        if [ $chunk_counter -eq $CRAB_Id ]; then
                echo "Processing file #$total_file_counter - $file "
                cmsRun $file
                if [ $? -ne 0 ]; then
                    echo "The command cmsRun failed."
                    exit 1
                fi
                echo "... done."
        else
                echo "Skipping file #$total_file_counter - $file"
        fi

        if [ $file_counter -eq ___SPLIT_SIZE___ ]; then
            ((chunk_counter++))
            file_counter=0
        fi
        
        if [ $chunk_counter -gt ___NUM_JOBS___ ]; then
            chunk_counter=1
            file_counter=0
        fi

        ((file_counter++))
        ((total_file_counter++))

    fi
done

# Find and list directories that contain 'yay.txt'
input_dir="scan_results"
for dir in "$input_dir"/*; do
  if [ -d "$dir" ] && [ ! -f "$dir/yay.txt" ]; then
    rm -rf $dir
    if [ $? -ne 0 ]; then
        echo "The command rm dir failed."
        exit 1
    fi
  fi
done


rm -rf scan_results.tar
if [ $? -ne 0 ]; then
    echo "The command rm scan_results.tar failed."
    exit 1
fi

tar -cvf scan_results.tar scan_results
if [ $? -ne 0 ]; then
    echo "The command tar failed."
    exit 1
fi
        


echo "================= MUSiC finished ===================="

echo "================= CMSRUN starting ===================="
cmsRun -j FrameworkJobReport.xml PSet.py
if [ $? -ne 0 ]; then
    echo "The command cmsRun (final) failed."
    exit 1
fi
echo "================= CMSRUN finished ===================="
"""
