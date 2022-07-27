# !/usr/bin/bash
# Script to load CMSSW
#
# This function is meant to be sourced into your .bashrc
# Example: source ~/cmssw_environment.sh /user/radziej/CMSSW/
#
# The (optional) path argument tells the function where the CMSSW installations
# are located. By default, it assumes you have linked them to your home
# directory.
#
# Should you be working on different machines, where the paths to the CMSSW
# versions are not identical, you may want to setup a conditional as follows:
# HOST=$(hostname -s)
# if [ "$HOST" == "lx3a56" ]; then
#     source ~/cmssw_environment.sh /disk1/radziej/CMSSW/
# else
#     source ~/cmssw_environment.sh /user/radziej/CMSSW/
# fi
#
# Call the function by typing its name and the version number of CMSSW. The
# CMSSW versions are sorted in reverse by the 'natural' number sorting of the
# 'ls' call. This means 110 comes before 12, resulting in the latest CMSSW
# version always being at the front. The first matching CMSSW version will be
# loaded.
#
# Example: cmssw 72
# If versions CMSSW_7_1_2, CMSSW_7_2_1_patch3 and CMSSW_7_2_3 are available, it
# will load CMSSW_7_2_3.

# Define directory where CMSSW versions are installed
if [ -z "$1" ]; then
    CMSSW_DIR="${HOME}"
else
    CMSSW_DIR="$1"
fi

function cmssw() {
    # Check if any input has been given, else print help text
    if [ -z "$1" ]; then
        echo "Usage: $0 A[BCD...]"
        echo ""
        echo "A B C D etc. represent the numbers that are part of the CMSSW release."
        echo "The environment of the first match will be loaded."
        echo "If no matches are found, an error is thrown."
        return 1
    fi

    # Get cmssw directories and put them in a bash array
    mapfile -t cmssw_versions <<< "`ls -rv ${CMSSW_DIR} | grep CMSSW`"
    if [ -z "${cmssw_versions[0]}" ]; then
        echo "No CMSSW versions found!"
        return 1
    fi

    # Build regex to check whether CMSSW version matches input
    # CMSSW_8123 -> CMSSW_8.*1.*2.*3.* -> CMSSW_8.*1.*2.*3
    local regex=`echo "CMSSW_$1" | sed 's/[0-9]/&.*/g' | sed 's/\.\*$//'`

    # Search for the shortest match
    local match_length=100
    local match=""
    for version in "${cmssw_versions[@]}"; do
        match_tmp=`echo ${version} | sed -n "s/.*\(${regex}\).*/\1/p"`
        if [ ! -z ${match_tmp} ]; then
            if (( match_length > ${#match_tmp} )); then
                match_length=${#match_tmp}
                match=${version}
            fi
        fi
    done

    if [ -z "${match}" ]; then
        echo "No matching CMSSW version found!"
        return 1
    fi

    # Set SCRAM_ARCH
    mapfile -t scram_archs <<< "`ls -rv ${CMSSW_DIR}/${match}/.SCRAM | grep slc`"
    export SCRAM_ARCH=${scram_archs[0]}
    echo "SCRAM_ARCH set to ${scram_archs[0]}"
    # Load CMSSW
    echo "Loading ${match}"
    source /cvmfs/cms.cern.ch/cmsset_default.sh
    cd ${CMSSW_DIR}/${match} && eval `scramv1 runtime -sh`;
    echo "Returning to"
    cd -
    return 0
}
