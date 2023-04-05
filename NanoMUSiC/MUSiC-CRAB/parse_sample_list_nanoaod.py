#!/usr/bin/env python3
import sys

if not sys.version_info >= (3, 6):
    print("This script should run only with Python 3.6+.")
    exit(9)

import tomli
from helpers import *
import logging
import optparse
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
        self.is_ext= False 

    def parse_name(self, options, generators, showers=[], redlist=[]):
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
            if re.search("Run2016.*-HIPM.*UL2016", ds_pt2):
                self.year = "2016APV"
            if re.search("Run2016FUL2016", ds_pt2):
                self.year = "2016"
            if re.search("Run2017.*UL2017", ds_pt2):
                self.year = "2017"
            if re.search("Run2018.*UL2018", ds_pt2):
                self.year = "2018"

            #check for extensions
            if re.search("_ext", ds_pt2):
                self.is_ext = True
            
                
            # get era
            match_era = re.search("Run201.*-UL", ds_pt2)
            if match_era.group():
                self.era = match_era.group(0)[7:-3]

            self.name = f"{ds_pt1}_{options.cme}TeV_{self.year}_{self.era}"
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

            #match = re.search("ext\d\d*", ds_pt2)
            self.name = (
                    ds_pt1
                    + "_"
                    + options.cme
                    + "TeV_"
                    + generators[self.generator]
                )
                # end if MC

        

def readSamplesFromFile(filename):
    with open(filename, "r") as file:
        return [Sample(l.rstrip("\n")) for l in file if l != "\n"]


def main():

<<<<<<< HEAD
    usage = "%prog [options] SAMPLELIST YEAR"
    description = "This script parses a given file SAMPLELIST and writes a customizable list."
=======
    usage = "%prog [options] SAMPLELIST CROSS_SECTION_FILE"
    description = "This script parses a given file with cross-sections and a sample list and writes a custom list that can be provided to music_crab."
>>>>>>> origin/auto_mapping_sampleList

    parser = optparse.OptionParser(
        usage=usage, description=description, version="%prog 0"
    )
    parser.add_option(
        "-e",
        "--cme",
        action="store",
        default="13",
        metavar="ENERGY",
        help="The center-of-mass energy for this sample",
    )
    parser.add_option(
        "-o",
        "--output_filename",
        action="store",
        default="xSections_list",
        metavar="FILENAME",
        help="Specify a filename for your output file.",
    )
    
    
    (options, args) = parser.parse_args()

    if len(args) != 2:
        parser.error("Exactly 2 argument needed: state the file you want to parse and the cross-section file.")

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
    sample_list = readSamplesFromFile(args[0])
    for sample in tqdm(sample_list):
        sample.parse_name(options, generators, showers, redlist)

    # write .json file for each sample
    datasets = {}
    for sample in sample_list:
        datasets[sample.name] = {
            "dataset": sample.dataset,
            "name": sample.name,
            "generator": sample.generator,
            "year": sample.year,
            "is_data": sample.is_data,
            }
    xsections = tomli.loads(Path(args[1]).read_text(encoding="utf-8"))
    for sample in sample_list:
        try:
            dName="das_name_"+sample.year
            if not dName in xsections[sample.name]:
                xsections[sample.name][dName]=[]
            xsections[sample.name][dName].append(sample.dataset)
        except:
            print (sample.name)
        
    # dump new config to file                                                                                                                                                           
    xSections_list = to_toml_dumps(xsections)
    os.system("rm xSections.toml > /dev/null 2>&1")
    with open(options.output_filename, "w") as new_xsection_file:
        new_xsection_file.write(xSections_list)

        
if __name__ == "__main__":
    main()
