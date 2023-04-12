#!/usr/bin/env python3

import os
import argparse
import tomli
import ROOT
from pathlib import Path 


def calc_bTag_eff():
    parser = argparse.ArgumentParser()
    parser.add_argument("toml_file_path")
    args = parser.parse_args()
    ROOT.gSystem.Load("bTagEff_cpp.so");
    configs = tomli.loads(Path(args.toml_file_path).read_text(encoding="utf-8"))

    os.system("rm -rf outputs;mkdir outputs")
    ROOT.bTag_Eff(configs["input_files"])

def main():
    calc_bTag_eff()
    os.system("touch outputs/success_flag.out")
    
if __name__=="__main__":
    main()
  
