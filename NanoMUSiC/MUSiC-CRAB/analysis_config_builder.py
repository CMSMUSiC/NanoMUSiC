#!/usr/bin/env python3
import sys

import tomli
from helpers import *
import logging
import argparse
import os
import re
from collections import OrderedDict
import json
import subprocess
import shlex
from tqdm import tqdm
from pathlib import Path
import multiprocessing
import collections


parser = argparse.ArgumentParser()
parser.add_argument("username", help="Username of the dCache owner.")
parser.add_argument(
    "xsection_file_path",
    help="Give path to the toml file containing cross-section and das_name per sample.",
)
parser.add_argument(
    "-j", "--jobs", help="Simultanious number of jobs.", type=int, default=10
)
parser.add_argument(
    "--btag", help="Will collect outpouts for the BTagging Efficeincy code.", type=bool
)
args = parser.parse_args()


def make_outputfile_list():
    return next(os.walk("."))[1]


CollectorInputs = collections.namedtuple(
    "CollectorInputs", ["sample", "job_list", "xsection_list", "cmd_str"]
)


def collect_output_files(collector_inputs: CollectorInputs):
    address_list = []
    if "crab_task_name" in collector_inputs.job_list[collector_inputs.sample]:
        for task in collector_inputs.xsection_list[collector_inputs.sample][
            "crab_task_name"
        ]:
            scan_process = subprocess.run(
                shlex.split(collector_inputs.cmd_str.replace("__taskname__", task)),
                capture_output=True,
                text=True,
            )
            for addr in scan_process.stdout.split("\n"):
                condition = r"nano_music" in addr and addr.endswith("root")
                if args.btag:
                    condition = "efficiency_hist" in addr and addr.endswith("root")
                if condition:
                    address = (
                        "dcap://grid-dcap-extern.physik.rwth-aachen.de"
                        + addr.split(" ")[-1]
                    )
                    address_list.append(address)
    return collector_inputs.sample, address_list


def main():
    print("[ Collecting results ... ]")
    directory_list = make_outputfile_list()
    xsection_list = tomli.loads(
        Path(args.xsection_file_path).read_text(encoding="utf-8")
    )
    for sample in xsection_list:
        xsection_list[sample]["crab_task_name"] = []
        for filename in directory_list:
            if re.search(f"crab_nano_music_{sample}", filename):
                xsection_list[sample]["crab_task_name"].append(filename)

    # remove unwanted entries
    unwanted_entries = []
    for sample in xsection_list:
        if "crab_task_name" not in xsection_list[sample]:
            unwanted_entries.append(sample)

    for sample in unwanted_entries:
        if sample in xsection_list.keys():
            del xsection_list[sample]

    job_list = xsection_list

    cmd_str = r"""srmls  -recursion_depth=999 "srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/__USERNAME__/nano_music/__taskname__/" """
    cmd_str = cmd_str.replace("__USERNAME__", args.username)

    print(f"--> Collecting outputs path ...")
    with multiprocessing.Pool(args.jobs) as pool:
        results = list(
            tqdm(
                pool.imap(
                    collect_output_files,
                    [
                        CollectorInputs(sample, job_list, xsection_list, cmd_str)
                        for sample in job_list
                    ],
                ),
                total=len(job_list),
            )
        )
    for r in results:
        sample, address_list = r
        job_list[sample]["output_files"] = address_list

    # dump new config to string
    crab_job_list = to_toml_dumps(job_list)
    # add lumi and global scale factor to toml file

    crab_job_list = (
        crab_job_list
        + r"""                                                                                       

[Lumi]                                                                                                                        
2016APV = 19520.0                                                                                                             
2016 = 16810.0                                                                                                                
2017 = 41480.0                                                                                                                
2018 = 59830.0                                                                                                                
Unit = "pb-1"

[Global]                                                                                                                      
ScalefactorError = 0.026                                                                                                      
"""
    )

    os.system("rm analysis_config.toml > /dev/null 2>&1")
    with open("analysis_config.toml", "w") as new_jobList_file:
        new_jobList_file.write(crab_job_list)

    print(f"Output saved to: analysis_config.toml")
    print("[ Done ]")


if __name__ == "__main__":
    main()
