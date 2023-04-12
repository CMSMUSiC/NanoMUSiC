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
parser.add_argument(
    "xsection_file_path",
    help="Give path to the toml file containing cross-section and das_name per sample.",
)
parser.add_argument(
    "output_file_path",
    help="Give path to the file containing output directories.",
    type=Path
)

args = parser.parse_args()

def make_outputfile_list(output_folder):
    return next(os.walk('.'))[1]

def main():
    
    directory_list=make_outputfile_list(args.output_file_path)
    xsection_list= tomli.loads(Path(args.xsection_file_path).read_text(encoding="utf-8"))
    for sample in xsection_list:
        for filename in directory_list:
            if re.search(sample, filename):
                xsection_list[sample]["crab_task_name"]=filename

    #remove unwanted entries                                                                                                  

    unwanted_entries=[]
    for sample in xsection_list:
        if "crab_task_name" not in xsection_list[sample]:
             unwanted_entries.append(sample)

    for sample in unwanted_entries:
        if sample in xsection_list.keys():
            del xsection_list[sample]

    # dump new config to string                                                                                               
    crab_job_list = to_toml_dumps(xsection_list)
    # add lumi and global scale factor to toml file                                                                          
 
    crab_job_list= crab_job_list+r"""                                                                                       

[Lumi]                                                                                                                        
2016APV = 19520.0                                                                                                             
2016 = 16810.0                                                                                                                
2017 = 41480.0                                                                                                                
2018 = 59830.0                                                                                                                
Unit = "pb-1"

[Global]                                                                                                                      
ScalefactorError = 0.026                                                                                                      
"""

    os.system("rm crab_job_list.toml > /dev/null 2>&1")
    with open("crab_job_list.toml", "w") as new_jobList_file:
        new_jobList_file.write(crab_job_list)

    
if __name__ == "__main__":
    main()
