import sys, os
from pathlib import Path
import tomli
import subprocess

from helpers import *
import ROOT

# get process parameters
if len(sys.argv) < 2:
    print(f"Only {len(sys.argv)} were provided. Expected 2.")
    exit(1)
job_id = sys.argv[1]
# toml_config = sys.argv[2]
toml_config = "raw_config.toml"

# get input files from PSet.py
def get_input_files(debug=False):
    if not debug:
        import PSet
    else:
        import crab_music_pset as PSet

    raw_input_files = PSet.process.source.fileNames.value()
    if len(raw_input_files) == 0:
        print(f"No input files were provided.")
        exit(1)

    input_files = []
    for f in raw_input_files:
        print(f"\n\n ------ Checking local file PFN ...")
        local_pfn_proc = subprocess.run(
            ["edmFileUtil", "-d", f], capture_output=True, check=True, text=True
        )
        local_pfn = (local_pfn_proc.stdout).rstrip()
        local_pfn_returncode = local_pfn_proc.returncode

        # sets global pfn
        global_pfn = ("root://cms-xrd-global.cern.ch//" + f).rstrip()

        # test local vs global load
        try:
            testfile = ROOT.TFile.Open(local_pfn)
            if testfile and testfile.IsOpen():
                print(f"-->Local load test OK: {local_pfn}")
                input_files.append(local_pfn)
            else:
                print(f"Local test open failed, forcing GLOBAL XROOTD: {global_pfn}")
                input_files.append(global_pfn)
        except:
            print(f"Local test open failed, forcing GLOBAL XROOTD: {global_pfn}")
            input_files.append(global_pfn)
    
      
    print("\nInput files:")
    print(input_files)

    return input_files


def modify_config():
    config = tomli.loads(Path(toml_config).read_text(encoding="utf-8"))
    config["input_files"] = get_input_files()
    config["is_crab_job"] = True
    config["n_threads"] = 1
    config["output"] = "outputs"

    new_config = to_toml_dumps(config)
    print("\n*************** Modified config file: ******************\n")
    print(new_config)
    print("\n" + "*" * 56)

    # dump new config to file
    os.system("rm config.toml > /dev/null 2>&1")
    with open("config.toml", "w") as new_config_file:
        new_config_file.write(new_config)


def DEV_DEBUG():
    _inputs = get_input_files()
    # print(_inputs)
    # try:
    #     print("--> Loading local PFN ...")
    #     ROOT.TFile.Open(_inputs[0][0]).Print()
    # except:
    #     print("[ERROR] Could not open local PFN.")

    # try:
    #     print("--> Loading global PFN ...")
    #     ROOT.TFile.Open(_inputs[0][1]).Print()
    # except:
    #     print("[ERROR] Could not open global PFN.")

    # os.system("touch config.toml")
    # os.system("touch nano_music_DYJetsToLL_M-50_13TeV_AM_0.root")
    # os.system("touch nano_music_DYJetsToLL_M-50_13TeV_AM_0.classes")


def main():
    print("----> CRAB-MUSiC <----")
    modify_config()
    # DEV_DEBUG()


if __name__ == "__main__":
    main()
