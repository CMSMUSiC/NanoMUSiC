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

log = logging.getLogger("sampleListCreator")


class Sample:
    def __init__(self, dataset):
        self.dataset = dataset
        self.name = ""
        self.generator = ""
        self.year = ""
        self.era = ""
        self.is_data = False
        self.is_ext = False

    def parse_name(self, generators, showers=[], redlist=[]):
        # format of datasetpath: /.../.../...
        # first part contains name + additional tags ( cme, tune, .. )
        # second part has additional information ( campaign, extention sample? ,... )
        # third part contains sample 'format' (AOD, MINIAOD, ...)
        dataset_split = self.dataset.split("/")
        ds_pt1 = dataset_split[1]
        ds_pt2 = dataset_split[2]
        ds_pt3 = dataset_split[3].strip()

        # if Data
        if ds_pt3 == "NANOAOD":
            self.is_data = True
            self.generator = "data"

            # get year
            if re.search("Run2016.*UL2016", ds_pt2):
                self.year = "2016"
            if re.search("Run2016.*-HIPM.*UL2016", ds_pt2):
                self.year = "2016APV"
            if re.search("Run2017.*UL2017", ds_pt2):
                self.year = "2017"
            if re.search("Run2018.*UL2018", ds_pt2):
                self.year = "2018"

            # check for extensions
            if re.search("_ext", ds_pt2):
                self.is_ext = True

            # get era
            match_era = re.search("Run201.*UL", ds_pt2)
            if match_era.group():
                self.era = match_era.group(0)[7:-3]

            self.name = f"{ds_pt1}_13TeV_{self.year}_{self.era}"
        # if MC
        else:
            for generator in generators.keys():
                # subn() performs sub(), but returns tuple (new_string, number_of_subs_made)
                # using (?i) at the beginning of a regular expression makes it case insensitive
                (ds_pt1, n) = re.subn(r"(?i)[_-]" + generator, "", ds_pt1)
                if n > 0:
                    self.generator = generator
                    for shower in showers:
                        ds_pt1 = re.sub(r"(?i)[_-]" + shower, "", ds_pt1)
                    break
                else:
                    self.generator = "unknown"
            for item in redlist:
                ds_pt1 = re.sub(r"(?i)[_-]*" + item, "", ds_pt1)

            # get year
            if re.search("20UL16NanoAODAPVv9", ds_pt2):
                self.year = "2016APV"
            if re.search("20UL16NanoAODv9", ds_pt2):
                self.year = "2016"
            if re.search("20UL17NanoAODv9", ds_pt2):
                self.year = "2017"
            if re.search("20UL18NanoAODv9", ds_pt2):
                self.year = "2018"

            # match = re.search("ext\d\d*", ds_pt2)
            self.name = ds_pt1 + "_" + "13TeV_" + generators[self.generator]
            # end if MC


def readSamplesFromFile(filename):

    global error_count
    error_count = 0

    with open(filename, "r") as file:
        sample_list = file.read().splitlines()
        for l in sample_list:
            if l != "":
                if sample_list.count(l) > 1:
                    print(f"Error : Sample list has duplicate entries({l})")
                    error_count += 1
                if not (l.endswith("NANOAODSIM") or l.endswith("NANOAOD")):
                    print(f"Error : Invalid Sample ({l})")
                    error_count += 1

    with open(filename, "r") as file:
        return [
            Sample(l.rstrip("\n"))
            for l in file
            if (l != "\n" and not (l.startswith("#")))
        ]


def main():
    global error_count
    description = "This script parses the given file with cross-sections and a sample list and writes a custom list that can be provided to music_crab.\nusage: parse_sample_list_nanoaod.py [-h] [-o OUTPUT_FILENAME] sample_list xsection_file_path"

    parser = argparse.ArgumentParser(usage=description)

    parser.add_argument(
        "sample_list",
        help="Give path to the txt file containing sample das_names per line.",
    )

    parser.add_argument(
        "xsection_file_path",
        help="Give path to the toml file containing cross-section per sample.",
    )

    parser.add_argument(
        "-o",
        "--output_filename",
        action="store",
        default="xSections_list.toml",
        help="Specify a filename for your output file.",
    )

    args = parser.parse_args()

    # dict of generators with shortname
    generators = OrderedDict()
    generators["madgraph"] = "MG"
    generators["powheg"] = "PH"
    generators["herwig6"] = "HW"
    generators["herwigpp"] = "HP"
    generators["herwig"] = "HW"
    generators["sherpa"] = "SP"
    generators["amcatnlo"] = "AM"
    generators["amcnlo"] = "AM"
    generators["alpgen"] = "AG"
    generators["calchep"] = "CA"
    generators["comphep"] = "CO"
    generators["lpair"] = "LP"
    generators["pythia6"] = "P6"
    generators["pythia8"] = "P8"
    generators["pythia"] = "PY"
    generators["gg2ww"] = "GG"
    generators["gg2zz"] = "GG"
    generators["gg2vv"] = "GG"
    generators["JHUGen"] = "JG"
    generators["blackmax"] = "BM"
    generators["data"] = ""
    generators["unknown"] = "??"

    # list of generators used for hadronization on top of another generator (will be removed from name)
    showers = ["pythia8", "pythia6", "pythia", "herwigpp"]

    # list of tags which will be removed from name (case insensitive)
    redlist = [
        "13tev",
        "madspin",
        "FXFX",
        "MLM",
        "NNPDF30",
        "NNPDF31",
        "TuneCUEP8M1",
        "TuneCUETP8M1",
        "TuneCUETP8M2T4",
        "TuneCP5",
    ]

    # read samples from file and parse name
    sample_list = readSamplesFromFile(args.sample_list)
    for sample in tqdm(sample_list):
        sample.parse_name(generators, showers, redlist)

    # read from the cross section toml file
    xsections = tomli.loads(Path(args.xsection_file_path).read_text(encoding="utf-8"))
    # xsec_miss_samples = []
    for sample in sample_list:
        try:
            dName = "das_name_" + sample.year
            xsections[sample.name]["is_data"] = False
            if not dName in xsections[sample.name]:
                xsections[sample.name][dName] = []
            xsections[sample.name][dName].append(sample.dataset)
        except:
            if sample.is_data:
                xsections[sample.name] = {}
                dName = "das_name_" + sample.year
                xsections[sample.name]["is_data"] = True
                if not dName in xsections[sample.name]:
                    xsections[sample.name][dName] = []
                xsections[sample.name][dName].append(sample.dataset)
            else:
                # xsec_miss_samples.append(sample.name)
                # if not xsec_miss_samples.count(sample.name) > 1:
                print(
                    f"ERROR: Cross-section information not available for {sample.name} ({sample.dataset})."
                )  # Searching for samples with no cross section information
                error_count += 1

    # remove unwanted entries
    unwanted_samples = []
    for sample in xsections:
        dName_16 = "das_name_2016"
        dName_16APV = "das_name_2016APV"
        dName_17 = "das_name_2017"
        dName_18 = "das_name_2018"
        if (
            not (dName_16APV in xsections[sample])
            and not (dName_16 in xsections[sample])
            and not (dName_17 in xsections[sample])
            and not (dName_18 in xsections[sample])
        ):
            unwanted_samples.append(sample)

    for sample in unwanted_samples:
        if sample in xsections.keys():
            del xsections[sample]

    # Loops over all samples and warns if a sample for a year is missing
    for sample in xsections:
        dName_16 = "das_name_2016"
        dName_16APV = "das_name_2016APV"
        dName_17 = "das_name_2017"
        dName_18 = "das_name_2018"
        if not xsections[sample]["is_data"]:
            if dName_16 not in xsections[sample]:
                print(f"WARNING: Sample for 2016 missing for {sample}.")
            if dName_16APV not in xsections[sample]:
                print(f"WARNING: Sample for 2016APV missing for {sample}.")
            if dName_17 not in xsections[sample]:
                print(f"WARNING: Sample for 2017 missing for {sample}.")
            if dName_18 not in xsections[sample]:
                print(f"WARNING: Sample for 2018 missing for {sample}.")

    # dump new config to string
    xSections_list = to_toml_dumps(xsections)
    # add lumi and global scale factor to toml file
    lumi_string = r"""

[Lumi]
2016APV = 19520.0
2016 = 16810.0
2017 = 41480.0
2018 = 59830.0
Unit = "pb-1"

[Global]
ScalefactorError = 0.026
"""

    if error_count == 0:
        os.system(f"rm {args.output_filename} > /dev/null 2>&1")
        with open(args.output_filename, "w") as new_xsection_file:
            new_xsection_file.write(xSections_list)
        print(f"\nOutput file saved: {args.output_filename}")

    else:
        print("\nOutput file not saved")


if __name__ == "__main__":
    main()
