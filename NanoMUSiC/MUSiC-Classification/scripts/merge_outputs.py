#!/usr/bin/env python3

import os
import ROOT


def main():
    ROOT.gSystem.CompileMacro(
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC-Classification/scripts/merge_outputs.C",
        "gsO",
    )
    ROOT.merge_outputs("classification_outputs", ["SingleMuon"], "foo.root")


if __name__ == "__main__":
    main()
