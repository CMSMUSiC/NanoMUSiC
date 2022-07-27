#!/usr/bin/env python

import os
import sys
import argparse
import pprint

from dbutilscms import DasClient
from ramlib import ramutils

filter_types = {"misc": ["MinBias",
                         "RelVal",
                         "isrup",
                         "isrdown",
                         "fsrup",
                         "fsrdown",
                         "scaleup",
                         "scaledown"],
                "higgs": ["GluGluH",
                          "ttH",
                          "bbH",
                          "ZH",
                          "TH",
                          "HW",
                          "HZJ",
                          "ggZH",
                          "tqH",
                          "ZH_",
                          "WplusH",
                          "WminusH",
                          "VBF_H",
                          "VBFH",
                          "GluGluToH",
                          "Higgs",
                          "VHTo",
                          "TTToH",
                          "HToJPsiG",
                          "WPlusH",
                          "WMinusH"],
                "signal": ["ADD",
                           "ZPrime",
                           "WPrime",
                           "Zprime",
                           "Wprime",
                           "WpWp",
                           "WmWm",
                           "BsTo",
                           "Branon",
                           "X53X53",
                           "WRTo",
                           "VBF_RadionTo",
                           "Unpart",
                           "TstarTstar",
                           "Tprime",
                           "TTbarDM",
                           "VBF_BulkGrav",
                           "SUSY",
                           "SLQ_",
                           "RadionTo",
                           "SMS",
                           "RSGrav",
                           "LongLivedChi0",
                           "LQTo",
                           "Monotop",
                           "HToZATo",
                           "ST_FCNC",
                           "RSGluon",
                           "Qstar",
                           "Spin0_ggPhi",
                           "HSCP",
                           "H0ToUps",
                           "Graviton",
                           "LFV",
                           "ExtendedWeakIsospinModel",
                           "DarkMatter",
                           "DMS_NNPDF30",
                           "ChargedHiggs",
                           "BulkGrav",
                           "Bstar",
                           "Bprime",
                           "BBbarDM",
                           "Seesaw",
                           "XXTo",
                           "gluino",
                           "GluGluSpin0",
                           "Estar",
                           "RPVresonant",
                           "Mustar",
                           "AToZ",
                           "RPVS",
                           "X53",
                           "Taustar",
                           "DMV",
                           "BlackHole",
                           "DM_Scalar",
                           "DM_Pseudo",
                           "HPlusPlus",
                           "VBFDM",
                           "MonoH",
                           "MonoTop",
                           "MonoJ",
                           "MonoZ",
                           "H1H1",
                           "H1H2",
                           "Gluino",
                           "Spin2",
                           "BdToKstarMuMu",
                           "BdToPsiKstar",
                           "Contin",
                           "Gstar",
                           "GluGlu_HToMuMu",
                           "Neutralino",
                           "SMM_MonoW",
                           "Pseudo_MonoZ",
                           "ZBJets",
                           "Vector",
                           "Axial",
                           "NMSSM",
                           "Spin0_ggPhi",
                           "QBHToE"]
                }


def parse_arguments():
    description = "Explore availble samples in das"
    parser = argparse.ArgumentParser(description=description)
    description = "Available input options"
    in_group = parser.add_argument_group(description=description)
    in_options = in_group.add_mutually_exclusive_group(required=True)
    help = "Dataset path of query for das_client"
    in_options.add_argument("-d", "--dataset", help=help)
    help = "Use query /*/*production_campaign*/DATATIER"
    in_options.add_argument("-p", "--production_campaign", help=help)
    help = "Path to a file with a list of dataset paths, e.g. from previos run"
    in_options.add_argument("-f", "--samplelist", help=help)
    help = "Path to a ram yaml file to find samples for"
    parser.add_argument("-r", "--ram-config", help=help)
    help = "output file (a second file with tag _full will be created)"
    parser.add_argument("-o", "--out",
                        help=help,
                        default="discovered_samples.txt")
    help = "Default datatier default: MINIAODSIM"
    parser.add_argument("-t", "--datatier", help=help, default="MINIAODSIM")
    help = "Omit printout of dataset list to prompt"
    parser.add_argument("-q", "--quiet", action="store_true", help=help)
    description = "Filter options"
    in_group = parser.add_argument_group(description=description)
    help = "Do not filter signals"
    parser.add_argument("--include-signal", action="store_true", help=help)
    help = "Do not filter relval, minbias etc."
    parser.add_argument("--include-misc", action="store_true", help=help)
    help = "Do not filter higgs samples"
    parser.add_argument("--include-higgs", action="store_true", help=help)
    help = "Allow to look for status != VALID e.g. running"
    parser.add_argument("--all-status", action="store_true", help=help)
    args = parser.parse_args()
    if args.production_campaign:
        args.dataset = "/*/*" + args.production_campaign.strip("/") + "*/"
        args.dataset += args.datatier

    return args

def filter_by_ram_config(args):
    settings, samplecfg = ramutils.load_config( args.ram_config )
    samples = ramutils.get_samples(settings, samplecfg)
    datasetpaths = []
    for sample in samples:
        datasetpaths.append(sample.skim.datasetpath)
    return datasetpaths

def is_same_first_part(datasetpath_1, datasetpath_2):
    if datasetpath_1.split("/")[1] == datasetpath_2.split("/")[1]:
        return True
    return False

def main():
    args = parse_arguments()

    dsets = []
    if not args.samplelist:
        das = DasClient()

        data = das.get_data(args.dataset, all_status=args.all_status)

        dsets = [d['dataset'][0]['name'] for d in data]
        dsets.sort()
        if not dsets:
            pprint.pprint(data)
            print("Found no samples, see das response above.")
        # dump full file list befor filtering
        filename, file_extension = os.path.splitext(args.out)
        with open(filename + "_full" + file_extension, "wb") as full_file:
            full_file.write("\n".join(dsets))
    else:
        with open(args.samplelist, "r") as sfile:
            dsets = sfile.read().split("\n")
    if args.ram_config:
        data = das.get_data(args.dataset, all_status=True)
        dsets_all = [d['dataset'][0]['name'] for d in data]
        dsets_production = list(set(dsets_all) - set(dsets))
        datasetpaths = filter_by_ram_config(args)
        found, in_production, missing = [], [], []
        for dp in datasetpaths:
            is_found = False
            for dp2 in dsets:
                if is_same_first_part(dp, dp2):
                    found.append(dp2)
                    is_found = True
                    break
            for dp2 in dsets_production:
                if is_same_first_part(dp, dp2):
                    in_production.append(dp2)
                    is_found = True
                    break
            if not is_found:
                missing.append(dp)
        found.sort()
        missing.sort()
        in_production.sort()
        found_str = "\n".join(found)
        if not args.quiet:
            print "Found samples"
            print found_str
            print "########################################"
            print "Samples in production"
            print "\n".join(in_production)
            print "########################################"
            print "Missing samples"
            print "\n".join(missing)
        with open(args.out, "w") as filtered_file:
            filtered_file.write(found_str)
    else:
        for filter_type, tags in filter_types.items():
            if not getattr(args, "include_" + filter_type):
                if filter_type == "higgs":
                    print(tags)
                dsets = [d for d in dsets if not any([t in d for t in tags])]

        dsets_str = "\n".join(dsets)
        if not args.quiet:
            print(dsets_str)
        with open(args.out, "w") as filtered_file:
            filtered_file.write(dsets_str)


if __name__ == "__main__":
    main()
