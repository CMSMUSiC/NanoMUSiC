#!/usr/bin/env python3

#############################################
## GENERATE TEX TABLE FROM ANALYSIS CONFIG ##
#############################################

from __future__ import annotations

import numpy as np
import math
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import matplotlib.patheffects as pe
import matplotlib.patches as mpatches
import uproot
import toml
import argparse
from typing import Any
from pprint import pprint
from collections import defaultdict
from tqdm import tqdm
import matplotlib.gridspec as gridspec
from typing import Any
import numpy as np
import mplhep as hep
import os
from multiprocessing import Pool
import re


#################################
# GENERATE TEX TABLE
#################################

# valid years to enter as an argument
valid_years = {"2016APV", "2016", "2017", "2018"}

# path of the validation files
validation_path = "./validation_outputs"

# debug flag, if true more detailed console output is active
debug = False

# parses arguments
def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        required=True,
        help='Task configuration (TOML) file, produced by "analysis_config_builder.py"',
    )
    parser.add_argument(
        "-y",
        "--year",
        required=True,
        help="Year to be processed. Currently only plotting for one year is possible.",
    )
    parser.add_argument(
        "-pc", "--plotconfig", help="Plot configuration (TOML) file.", required=True
    )
    args = parser.parse_args()
    return args


# extracts task config
def extract_config(task_config, year):
    mcconfig, dataconfig = {}, {}
    print(f"Extracting samples from task config for year {year}...")
    for sample in task_config:
        if sample != "Lumi" and sample != "Global":
            if year == "2016":
                if (
                    f"das_name_{year}" in task_config[sample].keys()
                    and "APV" not in task_config[sample].keys()
                ):  # only import samples of the right year
                    if not task_config[sample]["is_data"]:  # mc case
                        mcconfig.update({sample: task_config[sample]})
                    else:  # data case
                        dataconfig.update({sample: task_config[sample]})
            else:
                if (
                    f"das_name_{year}" in task_config[sample].keys()
                ):  # only import samples of the right year
                    if not task_config[sample]["is_data"]:  # mc case
                        mcconfig.update({sample: task_config[sample]})
                    else:  # data case
                        dataconfig.update({sample: task_config[sample]})
    print(
        "Found",
        len(mcconfig),
        "mc samples and",
        len(dataconfig),
        "data samples in the selected task config.",
    )
    return mcconfig, dataconfig


# sorting function for samples
def sorter(x):
    if len(numbers_in_name[x]) == 1:
        return 0
    elif "13TeV" in x:
        refpos = numbers_in_name[x].index("13")
        if "To" in x:
            if "Inf" in x:
                return int(numbers_in_name[x][refpos - 1])
            else:
                return int(numbers_in_name[x][refpos - 2])
        else:
            return int(numbers_in_name[x][refpos - 1])
    else:
        return 0


###################################################################################################

# table header and footer
header = "" # "\\begin{landscape}\n\\begin{longtable}{| L{0.18\\textwidth} | L{0.95\\textwidth} | L{0.18\\textwidth} | L{0.1\\textwidth} | L{0.1\\textwidth} |}\n\\hline\nProcess group & DAS name & Cross section $\sigma$ [$\\si{\\pico\\barn}$] & $k$-factor & Order \\\\\n\\hline\\endhead\n"
footer = "" # "\n\\caption{Full list of all MC samples}\n\\label{tab:mcsamples}\n\\end{longtable}\n\\end{landscape}"

##### MAIN FUNCTION #####
def main():
    print("\n\n📶 [ MUSiC Validation Table Maker ] 📶")

    # parse arguments
    args = parse_args()

    # import task config file that includes references to all files that should be validated
    print(f"Importing task config...")
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    # import plot config file that includes information on the aggregation groups
    print(f"Importing plot config...")
    plot_config_file: str = args.plotconfig
    plot_config: dict[str, Any] = toml.load(plot_config_file)
    if not (("color_dict" in plot_config) or ("aggregation_dict" in plot_config)):
        raise RuntimeError(
            "A color and an aggregation dictionary has to be included in the plot config file."
        )
    color_dict = plot_config["color_dict"]
    aggregation_dict = plot_config["aggregation_dict"]
    histograms = plot_config
    histograms.pop("color_dict")  # rmemove unwanted options from dict
    histograms.pop("aggregation_dict")
    if "COUNTS" in histograms.keys():
        histograms.pop("COUNTS")
    # histograms is a dict {histname: {properties: values}}

    year = args.year

    # extract data and mc samples given in task config file
    mcconfig, dataconfig = extract_config(task_config, year)
    datasamples = [sample for sample in dataconfig]
    mcsamples = [sample for sample in mcconfig]

    # sort mc samples in their groups
    mcgroups = set(
        [mcconfig[i]["ProcessGroup"] for i in mcconfig.keys()]
    )  # get set of mc groups
    mcsorted = {}
    for mcgroup in mcgroups:  # iterate over all groups
        tempset = set()
        for sample in mcsamples:  # check whether sample belongs to group
            if mcconfig[sample]["ProcessGroup"] == mcgroup:
                tempset.add(sample)
        mcsorted.update(
            {mcgroup: tempset}
        )  # dictionary: {group: {samples in this group}}
    print("Found", len(mcgroups), "mc groups in the selected task config.")

    mcgroups = set(mcsorted.keys())
    sample_prefix = {}  # {group: prefix: {samples}}

    # prepare and sort groups
    for group in mcgroups.copy():
        if len(mcsorted[group]) == 0:  # remove groups without samples
            mcgroups.remove(group)
            mcsorted.pop(group)
            continue
        for sample in mcsorted[group]:  # extract numbers from sample name
            # sub-division of samples
            prefix = ""
            temp = sample.split("_")
            if temp[0] == "QCD":
                prefix = temp[0] + "_" + temp[1]
            else:
                prefix = temp[0]
            if group not in sample_prefix.keys():
                sample_prefix.update({group: {prefix: [sample]}})
            else:
                if prefix not in sample_prefix[group].keys():
                    sample_prefix[group].update({prefix: [sample]})
                else:
                    sample_prefix[group][prefix].append(sample)

    # sort per prefix
    for group in mcgroups:
        for prefix in sample_prefix[group].keys():
            global numbers_in_name
            numbers_in_name = {}
            for sample in sample_prefix[group][prefix]:
                numbers = re.findall(r"[\d]+", sample)
                numbers_in_name.update({sample: numbers})
            sample_prefix[group][prefix] = sorted(
                sample_prefix[group][prefix], key=sorter, reverse=False
            )

    # make group with aggregation groups
    aggregation_group = {}  # {aggregation group : group}
    for group in mcgroups:
        for ag in aggregation_dict.values():
            if aggregation_dict[group] == ag:
                if ag in aggregation_group.keys():
                    aggregation_group[ag].add(group)
                else:
                    aggregation_group.update({ag: {group}})

    # merge all prefixes for one ag
    prefixes_ag = {}  # {ag: [prefix]} # note that this is a list
    for ag in aggregation_group.keys():
        for group in aggregation_group[ag]:
            if group in sample_prefix.keys():
                for prefix in sample_prefix[group].keys():
                    if ag not in prefixes_ag:
                        prefixes_ag.update({ag: [prefix]})
                    else:
                        prefixes_ag[ag].append(prefix)

    # sort all prefixes within one ag
    for ag in aggregation_group.keys():
        prefixes_ag[ag].sort(key=lambda v: v.upper())

    # sort aggregation groups
    a_groups = list(aggregation_group.keys())
    a_groups.sort(key=lambda v: v.upper())

    # make dict with all prefixes together
    prefix_samples = {}  # {prefix: [samples]}
    for group in sample_prefix.keys():
        for prefix in sample_prefix[group].keys():
            for sample in sample_prefix[group][prefix]:
                if prefix not in prefix_samples:
                    prefix_samples.update({prefix: [sample]})
                else:
                    prefix_samples[prefix].append(sample)

    file_contents = header  # holds file content

    # fill table
    for ag in a_groups:
        firstingroup = True
        didsomething = False
        if ag in prefixes_ag.keys():
            for prefix in prefixes_ag[ag]:
                for sample in prefix_samples[prefix]:
                    didsomething = True
                    # das name column
                    dasname = task_config[sample]["das_name_2018"]
                    dasname = dasname[0].split("/")[1]
                    dasname = dasname.replace("_", "\\_\\allowbreak ")
                    dasname = dasname.replace("-", "-\\allowbreak ")
                    # cross section
                    xsec = (
                        "\\num{"
                        + "{:.3E}".format(float(task_config[sample]["XSec"]))
                        + "}"
                    )
                    # k-factor
                    kfac = "\\num{" + str(round(float(task_config[sample]["kFactor"]), 3)) + "}" #"{:.3f}".format(float(task_config[sample]["kFactor"])) + "}"
                    # order
                    order = task_config[sample]["XSecOrder"]
                    # fill table row
                    if not firstingroup:
                        ag = ""
                    else:
                        ag += " "
                    file_contents += str(
                        ag
                        + "& "
                        + dasname
                        + " & "
                        + xsec
                        + " & "
                        + kfac
                        + " & "
                        + order
                        + " \\\\\n"
                    )
                    firstingroup = False
        if didsomething:
            file_contents += "\\hline\n"

    # write to text file
    f = open("mc_samples_table_tex.txt", "w")
    f.write(file_contents + footer)
    f.close()

    print("Finished table job.\n")
    exit(0)


if __name__ == "__main__":
    main()
