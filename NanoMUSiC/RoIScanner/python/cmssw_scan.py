import subprocess
import os
import sys

from scan_props import ScanProps

from distribution_model import ScanDistribution
import json
from rich.progress import track


def exec_command(cmd: str) -> None:
    try:
        subprocess.run(
            cmd,
            check=True,  # Raises CalledProcessError on non-zero exit
            shell=True,
            stdout=subprocess.PIPE,  # Capture standard output
            stderr=subprocess.PIPE,  # Capture standard error
        )

    except subprocess.CalledProcessError as e:
        print(f"Error during execution: {e}")
        print(f"Standard Output: {e.stdout.decode()}")
        print(f"Standard Error: {e.stderr.decode()}")
        sys.exit(-1)


def prepare_cmssw_env():
    """
    Will configure a CMSSW area for the scanner.
    """

    exec_command("rm -rf CMSSW_14_0_7")
    exec_command(r'cmssw-el9 --cleanenv --command-to-run "cmsrel CMSSW_14_0_7"')
    exec_command("mkdir -p CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/plugins")
    exec_command(
        "cp -r {}/NanoMUSiC/RoIScanner/* CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/.".format(
            os.getenv("MUSIC_BASE")
        )
    )
    exec_command("rm -rf CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/python/*")
    exec_command("rm -rf CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/src/pybind_imp.cpp")
    exec_command("cd CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/src && cp ../include/* .")
    exec_command(
        "cd CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/src && cp {}/NanoMUSiC/MUSiC/external/json.hpp .".format(
            os.getenv("MUSIC_BASE")
        )
    )
    exec_command(
        "cd CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/src && cp {}/NanoMUSiC/MUSiC/external/BS_thread_pool.hpp .".format(
            os.getenv("MUSIC_BASE")
        )
    )
    exec_command(
        "cd CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/src && cp {}/NanoMUSiC/MUSiC/external/indicators.hpp .".format(
            os.getenv("MUSIC_BASE")
        )
    )
    exec_command("cd CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/src && rm pCalc.cpp")
    exec_command(
        "cd CMSSW_14_0_7/src/NanoMUSiC/RoIScanner/src && rm createLookupTable.cpp"
    )
    os.system(
        r'cmssw-el9 --cleanenv --command-to-run "cd CMSSW_14_0_7/src && cmsenv && scram b -j"'
    )


def make_scanner_config(
    input_json_file: str,
    output_dir: str,
    rounds: int,
    start_round: int,
    shifts_file: str,
    lut_file: str,
    scan_type: str,
    ec_name: str,
    distribution_type: str,
    # now: str,
    is_debug: bool = False,
) -> str:
    scanner_config = r"""
import FWCore.ParameterSet.Config as cms

process = cms.Process("SCANNER")

process.source = cms.Source("EmptySource")
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(0))
process.scanner = cms.EDAnalyzer(
    "RoIScanner", 
    jsonFilePath=cms.string("___INPUT_JSON_FILE___"),
    outputDirectory=cms.string("___OUTPUT_DIR___"),
    rounds=cms.int32(___ROUNDS___),
    start_round=cms.int32(___START_ROUND___),
    shiftsFilePath=cms.string("___SHIFTS_FILE___"),
    lutFilePath=cms.string("___LUT_FILE___"),
    scanType=cms.string("___SCAN_TYPE___"),
    ec_name = cms.string("___EC_NAME___"),
    distribution_type = cms.string("___DISTRIBUTION_TYPE___"),
    is_debug=cms.bool(___IS_DEBUG___),
)

process.p = cms.Path(process.scanner)
# process.ep = cms.EndPath()
    """

    return (
        scanner_config.replace("___INPUT_JSON_FILE___", input_json_file)
        .replace("___OUTPUT_DIR___", output_dir)
        .replace("___ROUNDS___", str(rounds))
        .replace("___START_ROUND___", str(start_round))
        .replace("___SHIFTS_FILE___", str(shifts_file))
        .replace("___LUT_FILE___", lut_file)
        .replace("___SCAN_TYPE___", scan_type)
        .replace("___EC_NAME___", ec_name)
        .replace("___DISTRIBUTION_TYPE___", distribution_type)
        .replace("___IS_DEBUG___", str(is_debug))
    )
