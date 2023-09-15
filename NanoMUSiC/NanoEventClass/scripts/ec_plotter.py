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
            # "*"
        ],
    )

    classes = list(ec_collection.get_classes())
    print(classes, len(classes))

    test_class = ec_collection.get_class("EC_2Muon")
    print(test_class.to_string())

    for h in test_class.m_counts:
        if h.process_group == "Data":
            print(h.to_string())
    # pprint(input_files)

    test_dist = ROOT.Distribution(test_class, "counts")

    for dist in ROOT.distribution_factory(ec_collection):
        print(dist.m_distribution_name, dist.m_event_class_name)


if __name__ == "__main__":
    main()
