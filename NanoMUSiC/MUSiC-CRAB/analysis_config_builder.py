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


parser = argparse.ArgumentParser()
parser.add_argument("username", help="Username of the dCache owner.", required=True)
parser.add_argument(
    "xsection_file_path",
    help="Give path to the toml file containing cross-section and das_name per sample.",
    required=True,
)
args = parser.parse_args()


def make_outputfile_list():
    return next(os.walk("."))[1]


def main():

    directory_list = make_outputfile_list()
    xsection_list = tomli.loads(
        Path(args.xsection_file_path).read_text(encoding="utf-8")
    )
    for sample in xsection_list:
        for filename in directory_list:
            if re.search(sample, filename):
                xsection_list[sample]["crab_task_name"] = filename

    # remove unwanted entries
    unwanted_entries = []
    for sample in xsection_list:
        if "crab_task_name" not in xsection_list[sample]:
            unwanted_entries.append(sample)

    for sample in unwanted_entries:
        if sample in xsection_list.keys():
            del xsection_list[sample]

    job_list = xsection_list

    cmd_str = r"""srmls  -recursion_depth=999 "srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/__USERNAME__/nano_music/__taskname__" """
    cmd_str = cmd_str.replace("__USERNAME__", args.username)
    for sample in job_list:
        if "crab_task_name" in job_list[sample]:
            scan_process = subprocess.run(
                shlex.split(
                    cmd_str.replace("__taskname__", job_list[sample]["crab_task_name"])
                ),
                capture_output=True,
                text=True,
            )
            address_list = []
            for addr in scan_process.stdout.split("\n"):
                if "efficiency_hist" in addr:
                    address = (
                        "dcap://grid-dcap-extern.physik.rwth-aachen.de"
                        + addr.split(" ")[-1]
                    )
                    address_list.append(address)
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


if __name__ == "__main__":
    main()
