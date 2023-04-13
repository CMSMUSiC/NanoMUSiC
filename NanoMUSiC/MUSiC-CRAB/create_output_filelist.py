#!/usr/bin/env python3                                                                                                         
import sys

import subprocess
import shlex
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
parser.add_argument(
    "crab_job_list",
    help="Give path to the toml file containing list of crab jobs per sample.",
)

args = parser.parse_args()

def main():
    job_list= tomli.loads(Path(args.crab_job_list).read_text(encoding="utf-8"))
    
    cmd_str=r'''srmls  -recursion_depth=999 "srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/cseth/nano_music/__taskname__" '''
    for sample in job_list:
        if "crab_task_name" in job_list[sample]:
            scan_process=subprocess.run(

                shlex.split(cmd_str.replace("__taskname__",job_list[sample]["crab_task_name"])),
                capture_output=True,
                text=True,
            )
            address_list=[]
            for addr in scan_process.stdout.split("\n"):
                if "efficiency_hist" in addr: 
                    address="dcap://grid-dcap-extern.physik.rwth-aachen.de"+addr.split(" ")[-1]
                    address_list.append(address)
            job_list[sample]["output_files"]=address_list
    
    
     # dump joblist to string 
    Job_list = to_toml_dumps(job_list)
    with open("output_file_list.toml", "w") as new_output_list:
        new_output_list.write(Job_list)

if __name__ == "__main__":
    main()


"""
    output_root_list = to_toml_dumps(root_file_list)
    with open("output_root_list.toml", "w") as new_jobList_file:
        new_jobList_file.write(output_root_list)

        if re.search(root_file_list[sample],".root"):

"""






