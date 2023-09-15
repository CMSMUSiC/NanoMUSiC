#!/usr/bin/env python3

import ROOT
from glob import glob
import sys

from pprint import pprint

ROOT.gSystem.AddIncludePath("-I../NanoMUSiC/NanoEventClass/include")

if (
    ROOT.gSystem.CompileMacro(
        "../NanoMUSiC/NanoEventClass/src/NanoEventClass.cpp", "Ok"
    )
    == 0
):
    sys.exit("ERROR: Could not compile NanoEventClass.cpp.")
if (
    ROOT.gSystem.CompileMacro("../NanoMUSiC/NanoEventClass/src/Distribution.cpp", "Ok")
    == 0
):
    sys.exit("ERROR: Could not compile Distribution.cpp.")
if (
    ROOT.gSystem.CompileMacro(
        "../NanoMUSiC/NanoEventClass/src/distribution_factory.cpp", "Ok"
    )
    == 0
):
    sys.exit("ERROR: Could not compile Distribution.cpp.")

ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.TH1.AddDirectory(False)
ROOT.TDirectory.AddDirectory(False)
ROOT.gROOT.SetBatch(True)
ROOT.EnableThreadSafety()


def get_source_files(path, year):
    return list(
        filter(
            lambda f: ("cutflow" not in f),
            glob(f"{path}/*.root"),
        )
    )


def main():
    input_files = get_source_files("/disk1/silva/classification_histograms", "2018")

    print("Creating EC Collection ...")
    ec_collection = ROOT.NanoEventClassCollection(
        input_files,
        # ["EC_2Muon*", "EC_2Electron*"],
        [
            "EC_2Muon",
            # "EC_2Muon+X",
            #  "*"
        ],
    )

    for dist in ROOT.distribution_factory(ec_collection, True):
        pvalue_props = dist.get_pvalue_props()
        print(pvalue_props.total_mc)
        print(dist.m_distribution_name, dist.m_event_class_name)


if __name__ == "__main__":
    main()
