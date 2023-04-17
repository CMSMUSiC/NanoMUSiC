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
    addr_cat = ""

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
                addr_cat = addr_cat + addr + " "
            os.system(r"hadd -f Outputs/RootFiles/" + sample + ".root" + " " + addr_cat)
            ROOT.BTagEff("Outputs/RootFiles/" + sample + ".root", sample)


if __name__ == "__main__":
    main()
