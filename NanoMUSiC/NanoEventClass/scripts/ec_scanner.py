#!/usr/bin/env python3

import ROOT
from glob import glob
import sys
import copy
from tqdm import tqdm
from decimal import Decimal
import os
import json
from multiprocessing import Pool
from ec_tools import *


years_glob = {
    # "2016*": {"name": "2016", "lumi": "36.3"},  #
    # "2017": {"name": "2017", "lumi": "41.5"},  #
    "2018": {"name": "2018", "lumi": "59.8"},  #
    # "[2017,2018]": {"name": "2017+2018", "lumi": "101"},  #
    # "*": {"name": "", "lumi": "138"},
}


def main():
    for year in years_glob:
        input_files = get_source_files(
            "/disk1/silva/classification_histograms_2023_10_04", year
        )
        print("Creating EC Collection ...")
        ec_collection = ROOT.NanoEventClassCollection(
            input_files,
            # ["EC_2Muon*", "EC_2Electron*"],
            [
                # "*",
                # # "EC_2Muon*",
                # "EC_1Electron+NJet",
                # "EC_2Muon_1MET",
                # "EC_2Muon_2Tau_1MET",
                # "EC_2Muon_1Photon_1bJet_1MET+X",
                # "EC_2Muon_1Electron_3bJet+X",
                # "EC_2Muon+X",
                # "EC_4Muon",
                # "EC_4Electron",
                # "EC_2Muon_2Electron",
                # "EC_1Muon_1Electron+1MET",
                # "EC_1Muon_2Jet",
                # "EC_1Muon+X",
                # "EC_1Electron+X",
                # "EC_1Photon+X",
                # "EC_1Muon_2bJet",
                # "EC_1Muon_1Jet",
                # "EC_1Electron_2Jet",
                # "EC_2Muon+NJet",
                "EC_1Electron_2Tau_1Jet_2bJet_1MET",
                "EC_2Muon_1Tau_2Jet_1bJet_1MET+NJet",
                # "EC_2Muon_2Tau_1MET",
                # "EC_2Muon_1Electron_1Tau_2Jet_1MET+X",
            ],
        )

        print("Build scanner ...")
        scanner = ROOT.Scanner()

        print("Building Distributions ...")
        shifts, n_shifts = make_shifts({"a": list(range(1000)), "b": list(range(1000))})
        distributions = ROOT.distribution_factory(ec_collection, False, False)
        for dist in tqdm(distributions):
            # print(f"-- {dist.serialize()}")
            scan_results = scanner.scan(dist, shifts, n_shifts)
            # for res in scan_results:
            # print(res.p_value, res.lower_edge, res.upper_edge, res.scan_status)
            # pass

    print("Done.")


if __name__ == "__main__":
    main()
