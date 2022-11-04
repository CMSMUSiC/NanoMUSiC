#!/usr/bin/env python3

import sys
import ROOT
from tabulate import tabulate

def main():
    nano_music_file_path = sys.argv[1]

    f = ROOT.TFile(nano_music_file_path)
    f.Print("all")
    f.nano_music.Print("all")
    # f.nano_music.Scan()

    branches = []
    for b in f.nano_music.GetListOfBranches():
        if not b.GetName().startswith("weight"):
            branches.append(b.GetName())
    branches.append("weight")

    data = []
    i_evt = 0
    for evt in f.nano_music:
        if i_evt < 1:
            data.append([getattr(evt, b) for b in branches])
            i_evt += 1

    branches_with_type = []
    for b in f.nano_music.GetListOfBranches():
        if not b.GetName().startswith("weight"):
            branches_with_type.append(f"{b.GetName()}/{(b.GetListOfLeaves()[0].GetTypeName())}")
    branches_with_type.append("weight/Float_t")

    
    # display table
    print("\n\n")
    print(tabulate(data, headers=branches_with_type, tablefmt="grid"))
    print(f"Number of entries: {f.nano_music.GetEntries()}")

if __name__ == "__main__":
    main()

