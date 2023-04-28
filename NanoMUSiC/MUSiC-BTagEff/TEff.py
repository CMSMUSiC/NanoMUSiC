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
import ROOT

ROOT.gROOT.SetBatch(True)

parser = argparse.ArgumentParser()
parser.add_argument(
    "output_file_list",
    help="Give path to the toml file containing list of output files from crab jobs.",
)

args = parser.parse_args()


def main():
    output_file_list = tomli.loads(
        Path(args.output_file_list).read_text(encoding="utf-8")
    )
    os.system(f"root -b -q -l {os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC-BTagEff/TEff.cpp++")
    ROOT.gSystem.Load(f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC-BTagEff/TEff_cpp.so")
    addr_cat_16APV = ""
    addr_cat_16 = ""
    addr_cat_17 = ""
    addr_cat_18 = ""
    if not os.path.exists("Outputs"):
        os.system(r"mkdir Outputs")

    if not os.path.exists("Outputs/PNG"):
        os.system(r"mkdir -p Outputs/PNG")

    if not os.path.exists("Outputs/PDF"):
        os.system(r"mkdir -p Outputs/PDF")

    if not os.path.exists("Outputs/C"):
        os.system(r"mkdir -p Outputs/C")

    if not os.path.exists("Outputs/RootFiles"):
        os.system(r"mkdir -p Outputs/RootFiles")

    for sample in output_file_list:
        if "output_files" in output_file_list[sample]:
            for addr in output_file_list[sample]["output_files"]:
                if re.search("2016APV", addr):
                    addr_cat_16APV = addr_cat_16APV + addr + " "
                    print (addr_cat_16APV,"\n\n")
                elif re.search("2016", addr):
                    addr_cat_16 = addr_cat_16 + addr + " "
                    print (addr_cat_16,"\n\n")
                elif re.search("2017", addr):
                    addr_cat_17 = addr_cat_17 + addr + " "
                    print (addr_cat_17,"\n\n")
                elif re.search("2018", addr):
                    addr_cat_18 = addr_cat_18 + addr + " "
                    print (addr_cat_18,"\n\n")
            os.system(r"hadd -f Outputs/RootFiles/" + sample + "_2016APV" + ".root" + " " + addr_cat_16APV)
            ROOT.BTagEff("Outputs/RootFiles/" + sample + "_2016APV" + ".root", sample , "2016APV")
            os.system(r"hadd -f Outputs/RootFiles/"+ sample + "_2016" + ".root" + " " + addr_cat_16)
            ROOT.BTagEff("Outputs/RootFiles/" + sample + "_2016" + ".root", sample , "2016")
            os.system(r"hadd -f Outputs/RootFiles/" + sample + "_2017" + ".root" + " " + addr_cat_17)
            ROOT.BTagEff("Outputs/RootFiles/" + sample + "_2017" + ".root", sample , "2017")
            os.system(r"hadd -f Outputs/RootFiles/" + sample + "_2018" + ".root" + " " + addr_cat_18)
            ROOT.BTagEff("Outputs/RootFiles/" + sample + "_2018" + ".root", sample , "2018")

if __name__ == "__main__":
    main()



"""
            os.system(r"hadd -f Outputs/RootFiles/" + sample + ".root" + " " + addr_cat)
            ROOT.BTagEff("Outputs/RootFiles/" + sample + ".root", sample)
"""
