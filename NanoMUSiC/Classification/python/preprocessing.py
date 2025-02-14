import random
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
from rich.progress import track

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

    compilation_res = gSystem.CompileMacro(
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_sum_weights.cpp",
        "fgO-",
        "",
        "",
        1,
    )
    if not compilation_res:
        log.error("Could not compile compute_sum_weights.cpp.")
        sys.exit(-1)


class SumWeights:
    def __init__(
        self,
        sum_genWeight: float = 0.0,
        sum_LHEWeight: float = 0.0,
        raw_events: float = 0.0,
        has_genWeight: bool = False,
        has_LHEWeight_originalXWGTUP: bool = False,
    ):
        self.sum_genWeight = sum_genWeight
        self.sum_LHEWeight = sum_LHEWeight
        self.raw_events = raw_events
        self.has_genWeight = has_genWeight
        self.has_LHEWeight_originalXWGTUP = has_LHEWeight_originalXWGTUP


def compute_sum_weights_imp(args):
    sample, file, year = args
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

    try:
        res = ROOT.compute_sum_weights(file)
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
    except Exception as e:
        log.warning("Could not process {}. Will try reading it from RWTH.".format(file))

    try:
        file = file.replace(
            "root://cms-xrd-global.cern.ch//",
            "davs://grid-webdav.physik.rwth-aachen.de:2889",
        )
        res = ROOT.compute_sum_weights(file)
        log.info("Looks like reading {} from RWTH worked.".format(file))
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
    except Exception as e:
        log.warning(
            "Could not process {}. Will try reading it from FNAL global redirector.".format(
                file
            )
        )

    try:
        file = file.replace(
            "davs://grid-webdav.physik.rwth-aachen.de:2889",
            "root://cmsxrootd.fnal.gov//",
        )
        res = ROOT.compute_sum_weights(file)

        log.info(
            "Looks like reading {} from FNAL global redirector worked.".format(file)
        )
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

    except Exception as e:
        log.warning(
            "Could not process {}. Will try reading it from INFN global redirector.".format(
                file
            )
        )

    try:
        file = file.replace(
            "root://cmsxrootd.fnal.gov//",
            "root://xrootd-cms.infn.it//",
        )
        res = ROOT.compute_sum_weights(file)
        log.info(
            "Looks like reading {} from INFN global redirector worked.".format(file)
        )
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
    except Exception as e:
        log.error(
            "Could not process {}. All redirectors have failed.\n{}".format(file, e)
        )
        # we can allow this to fail and treat it later
        # raise RuntimeError("Could not process file {}".format(file))

        return (
            sample,
            year,
            SumWeights(),
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
                if f"input_files_{y}" in configs[sample]:
                    if len(configs[sample][f"input_files_{y}"]):
                        for f in configs[sample][f"input_files_{y}"]:
                            args.append(
                                (
                                    sample,
                                    f,
                                    y,
                                )
                            )

    weights = {}
    with Pool() as p:
        for idx, job in track(
            enumerate(
                p.imap_unordered(
                    compute_sum_weights_imp,
                    random.sample(args, len(args)),
                )
            ),
            description="Processing...",
            total=len(args),
        ):
            sample, y, result = job
            if not sample in weights.keys():
                weights[sample] = {}

            if not y in weights[sample].keys():
                weights[sample][y] = {}
                weights[sample][y]["sum_genWeight"] = 0
                weights[sample][y]["sum_LHEWeight"] = 0
                weights[sample][y]["raw_events"] = 0
                weights[sample][y]["has_genWeight"] = False
                weights[sample][y]["has_LHEWeight_originalXWGTUP"] = False

            weights[sample][y]["sum_genWeight"] += result.sum_genWeight
            weights[sample][y]["sum_LHEWeight"] += result.sum_LHEWeight
            weights[sample][y]["raw_events"] += result.raw_events
            weights[sample][y]["has_genWeight"] += result.has_genWeight
            weights[sample][y]["has_LHEWeight_originalXWGTUP"] += (
                result.has_LHEWeight_originalXWGTUP
            )
            log.info("Done: {} - {} | {}/{}".format(sample, y, idx, len(args)))

    with open("sum_weights.json", "w") as outfile:
        json.dump(weights, outfile, indent=4)
