#!/usr/bin/env python3
import sys

if not sys.version_info >= (3, 6):
    print("This script should run only with Python 3.6+.")
    exit(9)

import logging
import optparse
import os
import re
from collections import OrderedDict
import json
import subprocess
import shlex
from tqdm import tqdm

log = logging.getLogger("parseSampleList")


def get_files_list(dataset):
    files = subprocess.run(
        shlex.split(f'dasgoclient -query="file dataset={dataset}"'),
        shell=False,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding="UTF-8",
    ).stdout.splitlines()
    files = [f"root://cms-xrd-global.cern.ch//{f}" for f in files]
    return files


class Sample:
    def __init__(self, dataset):
        self.dataset = dataset
        self.name = ""
        self.generator = ""
        self.year = ""
        self.era = ""
        self.is_data = False

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

            # get era
            match_era = re.search("Run201.*-UL", ds_pt2)
            if match_era.group():
                self.era = match_era.group(0)[7:-3]

            self.name = f"{ds_pt1}_{options.cme}TeV_{options.postfix}{self.year}_{self.era}"
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

            match = re.search("ext\d\d*", ds_pt2)
            if match:
                self.name = (
                    ds_pt1
                    + "_"
                    + options.cme
                    + "TeV_"
                    + match.group()
                    + "_"
                    + options.postfix
                    + generators[self.generator]
                    + "_"
                    + self.year
                )
            else:
                self.name = (
                    ds_pt1
                    + "_"
                    + options.cme
                    + "TeV_"
                    + options.postfix
                    + generators[self.generator]
                    + "_"
                    + self.year
                )
                # end if MC

        # get files
        self.files = get_files_list(self.dataset)


def readSamplesFromFile(filename):
    with open(filename, "r") as file:
        return [Sample(l.rstrip("\n")) for l in file if l != "\n"]


def main():

    usage = "%prog [options] SAMPLELIST YEAR"
    description = "This script parses a given file SAMPLELIST and writes a customizable list."

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
        "-p",
        "--prefix",
        action="store",
        default=None,
        metavar="PREFIX",
        help="Specify a PREFIX for your output filename (e.g.  production version). [default = %default]",
    )
    parser.add_option(
        "-P",
        "--postfix",
        action="store",
        default=None,
        metavar="POSTFIX",
        help="Specify a POSTFIX for every process name in the output file. [default = %default]",
    )
    parser.add_option(
        "-f",
        "--force",
        action="store_true",
        default=False,
        help="Force overwriting of output files.",
    )
    parser.add_option(
        "--debug",
        metavar="LEVEL",
        default="INFO",
        help="Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default: %default]",
    )
    (options, args) = parser.parse_args()

    # Set loggig format and level
    #
    format = "%(levelname)s (%(name)s) [%(asctime)s]: %(message)s"
    date = "%F %H:%M:%S"
    logging.basicConfig(level=options.debug, format=format, datefmt=date)

    if options.prefix:
        options.prefix += "_"
    if options.prefix == None:
        options.prefix = ""

    if options.postfix:
        options.postfix += "_"
    if options.postfix == None:
        options.postfix = ""

    log.debug("Parsing files: " + " ".join(args))

    if len(args) != 1:
        parser.error("Exactly 1 argument needed: state the file you want to parse.")

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
            "files": sample.files,
        }

    outfile_json_name = "datasets.json"
    if os.path.exists(outfile_json_name) and not options.force:
        raise Exception(
            "Found file '%s'! If you want to overwrite it use --force."
            % outfile_json_name
        )
    else:
        with open(outfile_json_name, "w") as fp:
            json.dump(datasets, fp)

    # write .txt files for separated by generator
    unknown_gen = False
    for generator in generators:
        temp_list = list(
            filter(lambda sample: sample.generator == generator, sample_list)
        )
        temp_list.sort(key=lambda sample: sample.name)
        if generator == "unknown" and len(temp_list) > 0:
            unknown_gen = True
        elif len(temp_list) > 0:
            print("")
            print("Generator:\t" + generator.upper())
            outfile_name = "mc_" + options.prefix + generator + ".txt"
            if os.path.exists(outfile_name) and not options.force:
                raise Exception(
                    "Found file '%s'! If you want to overwrite it use --force."
                    % outfile_name
                )
            else:
                outfile = open(outfile_name, "w")
                outfile.write("generator = " + generator.upper() + "\n")
                outfile.write("CME = " + options.cme + "\n")
                outfile.write("\n")
                for sample in temp_list:
                    print(f"{sample.name}:\t {sample.dataset}")
                    outfile.write(f"{sample.name}: {sample.dataset}\n")
                outfile.close()
    print("")
    print(" ----> Total number of datasets: %s  \n" % len(sample_list))
    if unknown_gen:
        log.warning("There are dataset(s) produced with unknown generator(s)!")
        temp_list = list(
            filter(lambda sample: sample.generator == generator, sample_list)
        )
        for sample in temp_list:
            print("")
            print(
                f"Dataset produced with unknown generator:\n{sample.name}:\t{sample.dataset}"
            )


if __name__ == "__main__":
    main()
