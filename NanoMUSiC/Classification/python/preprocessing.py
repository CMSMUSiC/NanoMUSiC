import os
import subprocess
import ROOT
from ROOT import gSystem
from multiprocessing import Pool
import tomli
from pathlib import Path
import json
import logging
from rich.logging import RichHandler
from metadata import Years
import sys

FORMAT = "%(message)s"
logging.basicConfig(
    level=logging.DEBUG,
    format="%(message)s",
    datefmt="%m/%d/%Y %I:%M:%S %p",
    handlers=[RichHandler()],
)
log = logging.getLogger("main")


def preamble():
    # check proxy
    p = subprocess.run(["voms-proxy-info"], capture_output=True)
    if p.returncode != 0:
        log.error("No proxy found.")
        sys.exit(-1)

    log.info("Compiling libraries ...")

    gSystem.AddIncludePath(
        f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/include"
    )
    gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC/include")

    gSystem.CompileMacro(
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_sum_weights.cpp",
        "fgO-",
        "",
        "",
        1,
    )


class SumWeights:
    def __init__(
        self,
        sum_genWeight: float,
        sum_LHEWeight: float,
        raw_events: float,
        has_genWeight: bool,
        has_LHEWeight_originalXWGTUP: bool,
    ):
        self.sum_genWeight = sum_genWeight
        self.sum_LHEWeight = sum_LHEWeight
        self.raw_events = raw_events
        self.has_genWeight = has_genWeight
        self.has_LHEWeight_originalXWGTUP = has_LHEWeight_originalXWGTUP


def process(args):
    sample, files, year = args
    import ROOT

    gSystem.AddIncludePath(
        f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/include"
    )
    gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC/include")

    libs = [
        gSystem.Load(
            f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_sum_weights_cpp.so",
        ),
    ]
    if any([lib < 0 for lib in libs]):
        print("ERROR: Could not load libraries.")
        sys.exit(-1)

    files_to_process = []
    for f in files:
        if f.startswith("/store"):
            if (
                "WGG_5f_TuneCP5_13TeV_amcatnlo-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v11-v2"
                in f
            ):
                files_to_process.append("root://cmsxrootd.fnal.gov//{}".format(f))
            else:
                files_to_process.append("root://xrootd-cms.infn.it//{}".format(f))
        else:
            files_to_process.append(f)

    res = ROOT.process(files_to_process)
    return (
        sample,
        year,
        SumWeights(
            res.sum_genWeight,
            res.sum_LHEWeight,
            res.raw_events,
            res.has_genWeight,
            res.has_LHEWeight_originalXWGTUP,
        ),
    )


def compute_sum_weights(analysis_config: str) -> None:
    configs = tomli.loads(Path(analysis_config).read_text(encoding="utf-8"))

    total_samples = 0
    for sample in configs:
        if not configs[sample]["is_data"]:
            total_samples += 1

    args = []
    for sample in configs:
        if not configs[sample]["is_data"]:
            for y in Years:
                if f"output_files_{y}" in configs[sample]:
                    if len(configs[sample][f"output_files_{y}"]):
                        args.append(
                            (
                                sample,
                                configs[sample][f"output_files_{y}"],
                                y,
                            )
                        )

    weights = {}
    with Pool(124) as p:
        for idx, job in enumerate(p.imap_unordered(process, args)):
            sample, y, result = job
            if not sample in weights.keys():
                weights[sample] = {}

            if not y in weights[sample].keys():
                weights[sample][y] = {}

            weights[sample][y]["sum_genWeight"] = result.sum_genWeight
            weights[sample][y]["sum_LHEWeight"] = result.sum_LHEWeight
            weights[sample][y]["raw_events"] = result.raw_events
            weights[sample][y]["has_genWeight"] = result.has_genWeight
            weights[sample][y]["has_LHEWeight_originalXWGTUP"] = (
                result.has_LHEWeight_originalXWGTUP
            )
            log.info("Done: {} - {} | {}/{}".format(sample, y, idx, len(args)))

    with open("sum_weights.json", "w") as outfile:
        json.dump(weights, outfile, indent=4)
