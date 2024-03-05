#!/usr/bin/env python3

from datetime import datetime
import subprocess
import os
import argparse
import shlex
from multiprocessing import Pool, cpu_count
from tqdm import tqdm
import json

from CRABClient.UserUtilities import config
from CRABAPI.RawCommand import crabCommand

from sample_list import make_sample_list

from helpers import *

from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.formatters import Terminal256Formatter


parser = argparse.ArgumentParser()
parser.add_argument(
    "xsection_file_path",
    help="Give path to the toml file containing cross-sections per sample.",
)
parser.add_argument(
    "--date-and-time",
    help="Overwrites the global date and time.",
    default="",
)
parser.add_argument(
    "-btg",
    "--btageff",
    action="store_true",
    help="Set true to run the b-tag efficiency code.",
)

parser.add_argument(
    "--skip-tarball",
    action="store_true",
    help="Will skip the tarball creation. Assumes there is already one available.",
)

parser.add_argument(
    "--sample",
    help="If defined, will submit jobs only for this sample. Could be provided together with the expected year, otherwise will submit to all year.",
    required=False,
    default="",
)

parser.add_argument(
    "--year",
    help="If defined, will submit of to a specific year.",
    required=False,
    default="",
)

parser.add_argument(
    "--jobs",
    help="Pool size.",
    required=False,
    default=min(cpu_count(), 24),
)

parser.add_argument(
    "--clean", help="Clear the submition files", required=False, action="store_true"
)

args = parser.parse_args()


def make_task_config_file(
    process_name, das_name, year, era, is_data, generator_filter_key
):
    task_config = {
        "output": "",
        "is_data": False,
        "year": "",
        "era": "",
        "process": "",
        # "generator_filter_key": "",  # only if defined
        "dataset": "",
        "is_crab_job": False,
        "input_files": [],  # dummy, for now. Inside the job, it will be modified.
    }

    task_config["output"] = "outputs"
    task_config["is_data"] = is_data
    task_config["year"] = year
    task_config["era"] = era
    task_config["process"] = process_name
    if "generator_filter_key" != "":
        task_config["generator_filter_key"] = generator_filter_key
    task_config["dataset"] = das_name

    new_config = to_toml_dumps(task_config)

    os.system(f"mkdir -p raw_configs/{process_name}_{year}")

    with open(
        f"raw_configs/{process_name}_{year}/raw_config.toml", "w"
    ) as new_config_file:
        new_config_file.write(new_config)


def get_username():
    res = subprocess.check_output(
        ["crab", "checkusername"], stderr=subprocess.STDOUT
    ).decode("utf-8")
    if "Username is:" in res:
        return res.replace("Username is: ", "").replace("\n", "")
    else:
        raise RuntimeError("[ERROR] Could not get username.")


def build_crab_config(process_name, das_name, year, is_data, global_now):
    this_config = config()

    process_name = f"{process_name}_{year}"

    this_config.General.requestName = process_name
    this_config.General.workArea = f"crab_nano_music_{process_name}"
    if args.btageff:
        this_config.General.workArea = f"crab_btageff_{process_name}"
    this_config.General.transferOutputs = True

    this_config.JobType.pluginName = "Analysis"
    this_config.JobType.psetName = f"{os.getenv('CRAB_MUSIC_BASE')}/crab_music_pset.py"
    if args.btageff:
        this_config.JobType.psetName = (
            f"{os.getenv('CRAB_MUSIC_BASE')}/crab_music_btageff_pset.py"
        )
    this_config.JobType.scriptExe = f"{os.getenv('CRAB_MUSIC_BASE')}/run_nano_music.sh"
    if args.btageff:
        print("Will submit BTag Efficiency code ...")
        this_config.JobType.scriptExe = f"{os.getenv('CRAB_MUSIC_BASE')}/run_btageff.sh"

    this_config.JobType.inputFiles = [
        "task.tar.gz",
        f"raw_configs/{process_name}/raw_config.toml",
    ]

    this_config.Data.inputDataset = das_name
    this_config.Data.inputDBS = "global"
    this_config.Data.splitting = "FileBased"
    if is_data:
        this_config.Data.unitsPerJob = 3
    else:
        this_config.Data.unitsPerJob = 3

    this_config.Data.totalUnits = -1
    this_config.Data.publication = False
    this_config.Data.outputDatasetTag = process_name
    this_config.Data.outLFNDirBase = f"/store/user/{get_username()}/nano_music_{global_now}/{this_config.General.workArea}"
    this_config.JobType.outputFiles = [r"nano_music.root"]
    if args.btageff:
        this_config.JobType.outputFiles = [r"efficiency_hist.root"]
    this_config.User.voGroup = "dcms"
    this_config.Site.storageSite = "T2_DE_RWTH"
    # this_config.Site.blacklist = ["T2_BR_*", "T2_US_*"]
    this_config.Site.blacklist = ["T2_BR_*"]

    crab_config_file_path = f"raw_configs/{process_name}/crab_config.py"
    with open(crab_config_file_path, "w") as f:
        f.write(str(this_config))

    return crab_config_file_path


def submit(sample):
    (
        process_name,
        das_name,
        year,
        era,
        is_data,
        generator_filter_key,
        global_now,
    ) = sample
    make_task_config_file(
        process_name, das_name, year, era, is_data, generator_filter_key
    )
    sub_res = crabCommand(
        "submit",
        config=build_crab_config(process_name, das_name, year, is_data, global_now),
    )
    print(sub_res)
    return {"process_name": process_name, "year": year}


def build_task_tarball():
    print("Building gridpack ...")
    os.system(r"rm task.tar.gz > /dev/null 2>&1")
    os.system(
        r'tar \
        --exclude="*.log" \
        --exclude="rootlogon.C" \
        --exclude="crab.log" \
        --exclude="raw_config.toml" \
        --exclude="crab_nano_music_*" \
        --exclude="crab_music_pset.py" \
        --exclude="task.tar.gz" \
        --exclude="CMSSW_*" \
        --exclude="__pycache*" \
        --exclude="build" \
        --exclude="build_before_alma9" \
        --exclude="opt" \
        --exclude="docs_BKP" \
        --exclude="docs" \
        --exclude="crab_nano_music_date_*" \
        --exclude="NanoMUSiC/tools" \
        --exclude="NanoMUSiC/PxlAnalyzer" \
        --exclude="*.root" \
        --exclude="NanoMUSiC/PlotLib" \
        --exclude="NanoMUSiC/MUSiC-Configs" \
        --exclude="NanoMUSiC/MUSiC-RoIScanner" \
        --exclude="NanoMUSiC/MUSiC-Utils" \
        --exclude="NanoMUSiC/MUSiC-CRAB/crab_nano_music_DYJetsToLL*" \
        --exclude="cache" \
        --exclude="NanoMUSiC/MUSiC-BTagEff/Outputs" \
        -zcvf task.tar.gz $CRAB_MUSIC_BASE/../../*'
    )
    print("")


def check_voms():
    ret_code = subprocess.run(
        shlex.split("voms-proxy-info"), capture_output=True
    ).returncode

    if ret_code == 0:
        return True
    return False


def main():
    if args.clean:
        os.system("rm -rf task.tar.gz")
        os.system("rm -rf crab_nano_music_*")
        os.system("rm -rf last_*.txt")
        os.system("rm -rf submited_samples_date_*.json")
        exit(0)

    global_now = datetime.now().strftime(r"date_%Y_%m_%d_time_%H_%M_%S")
    if args.date_and_time != "":
        global_now = args.date_and_time

    os.system("rm -rf last_CRAB_submition_{global_now}.txt")
    os.system(f"touch last_CRAB_submition_{global_now}.txt")
    os.system(f"echo {global_now} > last_CRAB_submition_{global_now}.txt")

    # check for VOMS proxy
    if not (check_voms()):
        raise RuntimeError("ERROR: Could not find valid VOMS proxy.")

    # create the task tarball and submit the jobs
    if not args.skip_tarball:
        build_task_tarball()

    os.system("rm -rf raw_configs")
    os.system("mkdir raw_configs")
    sample_list = make_sample_list(
        args.xsection_file_path,
        global_now,
        args.sample,
        args.year,
    )

    with Pool(min(int(args.jobs), len(sample_list))) as pool:
        submited_samples = list(
            tqdm(
                # pool.imap_unordered(submit, sample_list),
                pool.imap_unordered(submit, list(sample_list[0])),
                total=len(sample_list),
                unit="sample",
            )
        )

    # Writing to sample.json
    json_object = []
    if os.path.isfile(f"submited_samples_{global_now}.json"):
        with open(f"submited_samples_{global_now}.json", "r") as f:
            json_object = json.load(f) + submited_samples

        with open(f"submited_samples_{global_now}.json", "w") as f:
            f.write(json.dumps(json_object))

    else:
        with open(f"submited_samples_{global_now}.json", "a") as f:
            f.write(json.dumps(submited_samples))


if __name__ == "__main__":
    main()
