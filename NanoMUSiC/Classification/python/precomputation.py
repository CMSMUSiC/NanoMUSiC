#!/usr/bin/env python3

import os
import ROOT
import argparse
import tomli
from pathlib import Path
import json

def preamble():
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/include")
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC/include")

    ROOT.gSystem.CompileMacro(
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/src/GeneratorFilters.cpp",
        "fgO",
        "",
        f"{os.getenv('MUSIC_BASE')}/lib",
        1,
    )

    ROOT.gSystem.CompileMacro(
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/src/NanoAODGenInfo.cpp",
        "fgO",
        "",
        f"{os.getenv('MUSIC_BASE')}/lib",
        1,
    )

    ROOT.gSystem.CompileMacro(
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_sum_weights.cpp",
        "fgO",
        "",
        f"{os.getenv('MUSIC_BASE')}/lib",
        1,
    )


def process(files, year, generator_filter_key):
    if generator_filter_key:
        return ROOT.process(files, year, generator_filter_key)
    return ROOT.process(files, year, ROOT.std.nullopt)


def compute_sum_weights(analysis_config:str)->None:
    configs = tomli.loads(Path(analysis_config).read_text(encoding="utf-8"))
    total_samples = 0
    for sample in configs:
        if not configs[sample]["is_data"]:
            total_samples += 1

    weights = {}
    years = ["2016APV", "2016", "2017", "2018"]
    for idx_sample, sample in enumerate(configs):
        if not configs[sample]["is_data"]:
            print()
            weights[sample] = {}
            for y in years:
                weights[sample][y] = {}
                if f"output_files_{y}" in configs[sample]:
                    print(
                        f"Processing sample: {sample} [{idx_sample}/{total_samples}] - {y}"
                    )
                    _weights = process(configs[sample][f"output_files_{y}"], y, configs[sample].get("generator_filter_key"))
                    weights[sample][y]["sum_genWeight"] = _weights.sum_genWeight
                    weights[sample][y]["sum_genWeight_pass_generator_filter"] = _weights.sum_genWeight_pass_generator_filter
                    weights[sample][y]["sum_LHEWeight"] = _weights.sum_LHEWeight
                    weights[sample][y]["sum_LHEWeight_pass_generator_filter"] = _weights.sum_LHEWeight_pass_generator_filter
                    weights[sample][y]["raw_events"] = _weights.raw_events
                    weights[sample][y]["pass_generator_filter"] = _weights.pass_generator_filter

    with open("sum_weights.json", "w") as outfile:
        json.dump(weights, outfile, indent=4)


