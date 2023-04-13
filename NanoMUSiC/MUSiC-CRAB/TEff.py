#!/usr/bin/env python3                                                                                                        \
                                                                                                                               
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
import ROOT

parser = argparse.ArgumentParser()
parser.add_argument(
    "output_file_list",
    help="Give path to the toml file containing list of output files from crab jobs.",
)

args = parser.parse_args()

def main():
     output_file_list= tomli.loads(Path(args.output_file_list).read_text(encoding="utf-8"))
     ROOT.gSystem.Load("Teff_cpp.so");
     for sample in output_file_list:
         for addr in output_file_list[sample]["output_files"]:
             
    
    
    
    
    
    
    
    
    
