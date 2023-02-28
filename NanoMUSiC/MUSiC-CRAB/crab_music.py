#!/usr/bin/env python3

from datetime import datetime
import subprocess
import os

from CRABClient.UserUtilities import config
from CRABAPI.RawCommand import crabCommand

from sample_list import sample_list

from pathlib import Path
import tomli
from helpers import *

from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.formatters import Terminal256Formatter, HtmlFormatter

def make_task_config_file(process_name, das_name, year, era, is_data):
    # copy config TOML to current directory
    # True for Data, False for MC
    config_toml_file = {
        True: "../../configs/task_configs/template_Data.toml", # Data
        False: "../../configs/task_configs/template_MC.toml", # MC
    }

    config = tomli.loads(Path("../../configs/task_configs/CRAB_template.toml").read_text(encoding="utf-8"))
    config["era"] = era
    config["is_crab_job"] = True
    config["output"] = "outputs"
    config["process"] = process_name
    config["dataset"] = das_name
    config["year"] = year
    config["is_data"] = is_data 

    new_config = to_toml_dumps(config)
    print("\n*************** Modified task config file: ******************\n")
    print(highlight(new_config, 
                lexer=get_lexer_by_name("toml"), 
                formatter=Terminal256Formatter(style="monokai")))
    print("\n" + "*" * 56)

    # dump new config to file
    os.system("rm raw_config.toml > /dev/null 2>&1")
    with open("raw_config.toml", "w") as new_config_file:
        new_config_file.write(new_config)

def get_username():
    res = subprocess.check_output(
        ["crab", "checkusername"], stderr=subprocess.STDOUT
    ).decode("utf-8")
    if "Username is:" in res:
        return res.replace("Username is: ", "").replace("\n", "")
    else:
        print("[ERROR] Could not get username.")
        exit()


def build_crab_config(process_name, das_name, year, is_data):
    this_config = config()

    process_name = f"{process_name}_{year}"
    now = datetime.now().strftime(r"date_%Y_%m_%d_time_%H_%M_%S")

    this_config.General.requestName = process_name
    this_config.General.workArea = f"crab_nano_music_{now}"
    this_config.General.transferOutputs = True

    this_config.JobType.pluginName = "Analysis"
    this_config.JobType.psetName = "crab_music_pset.py"
    this_config.JobType.scriptExe = "run_nano_music.sh"

    this_config.JobType.inputFiles = ["task.tar.gz", "raw_config.toml"]

    this_config.Data.inputDataset = das_name
    this_config.Data.inputDBS = "global"
    this_config.Data.splitting = "FileBased"
    this_config.Data.unitsPerJob = 10
    this_config.Data.totalUnits = -1
    this_config.Data.publication = False
    this_config.Data.outputDatasetTag = process_name
    this_config.Data.outLFNDirBase = (
        f"/store/user/{get_username()}/nano_music/{this_config.General.workArea}"
    )

    this_config.JobType.outputFiles = [
        r"nano_music.root"
    ]
    this_config.User.voGroup = "dcms"
    this_config.Site.storageSite = "T2_DE_RWTH"

    return this_config


def submit(sample):
    process_name, das_name, year, era, is_data = sample
    make_task_config_file(process_name, das_name, year, era, is_data)
    crabCommand(
        "submit", config=build_crab_config(process_name, das_name, year, is_data)
    )


def build_task_tarball():
    print("Packing input files ...")
    os.system(r"rm task.tar.gz > /dev/null 2>&1")
    os.system(
        r'tar --exclude="crab.log" --exclude="raw_config.toml" --exclude="crab_nano_music_date_*" --exclude="crab_music_pset.py" --exclude="task.tar.gz" --exclude="CMSSW_*" --exclude="__pycache*" --exclude="build" --exclude="docs_BKP" --exclude="docs" --exclude="crab_nano_music_date_*" --exclude="NanoMUSiC/tools" --exclude="NanoMUSiC/PxlAnalyzer" --exclude="*.root" --exclude="NanoMUSiC/PlotLib" --exclude="NanoMUSiC/MUSiC-Configs" --exclude="NanoMUSiC/MUSiC-RoIScanner" --exclude="NanoMUSiC/MUSiC-Utils"  -zcvf task.tar.gz ../../*'
    )
    print("")


def main():
    build_task_tarball()
    for sample in sample_list:
        submit(sample)


if __name__ == "__main__":
    main()
