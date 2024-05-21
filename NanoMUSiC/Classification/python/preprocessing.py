import os
import ROOT
import tomli
from pathlib import Path
import json
import logging
from rich.logging import RichHandler
import ray
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
    log.info("Compiling libraries ...")

    ROOT.gSystem.AddIncludePath(
        f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/include"
    )
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC/include")

    ROOT.gSystem.CompileMacro(
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/src/GeneratorFilters.cpp",
        "fgO-",
        "",
        # f"{os.getenv('MUSIC_BASE')}/lib",
        "",
        1,
    )

    ROOT.gSystem.CompileMacro(
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/src/NanoAODGenInfo.cpp",
        "fgO-",
        "",
        # f"{os.getenv('MUSIC_BASE')}/lib",
        "",
        1,
    )

    ROOT.gSystem.CompileMacro(
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_sum_weights.cpp",
        "fgO-",
        "",
        # f"{os.getenv('MUSIC_BASE')}/lib",
        "",
        1,
    )


class SumWeights:
    def __init__(
        self,
        sum_genWeight: float,
        sum_genWeight_pass_generator_filter: float,
        sum_LHEWeight: float,
        sum_LHEWeight_pass_generator_filter: float,
        raw_events: float,
        pass_generator_filter: float,
        has_genWeight: bool,
        has_LHEWeight_originalXWGTUP: bool,
    ):
        self.sum_genWeight = sum_genWeight
        self.sum_genWeight_pass_generator_filter = sum_genWeight_pass_generator_filter
        self.sum_LHEWeight = sum_LHEWeight
        self.sum_LHEWeight_pass_generator_filter = sum_LHEWeight_pass_generator_filter
        self.raw_events = raw_events
        self.pass_generator_filter = pass_generator_filter
        self.has_genWeight = has_genWeight
        self.has_LHEWeight_originalXWGTUP = has_LHEWeight_originalXWGTUP


@ray.remote
def process(files, year, generator_filter_key):
    import ROOT

    ROOT.gSystem.AddIncludePath(
        f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/include"
    )
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC/include")

    libs = [
        ROOT.gSystem.Load(
            f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_sum_weights_cpp.so",
        ),
        ROOT.gSystem.Load(
            f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/src/NanoAODGenInfo_cpp.so",
        ),
        ROOT.gSystem.Load(
            f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/src/GeneratorFilters_cpp.so",
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

    # if generator_filter_key:
    #     res = ROOT.process(files_to_process[:1], year, generator_filter_key)
    # res = ROOT.process(files_to_process[:1], year, ROOT.std.nullopt)
    res = ROOT.process(files_to_process, year, "")
    return SumWeights(
        res.sum_genWeight,
        res.sum_genWeight_pass_generator_filter,
        res.sum_LHEWeight,
        res.sum_LHEWeight_pass_generator_filter,
        res.raw_events,
        res.pass_generator_filter,
        res.has_genWeight,
        res.has_LHEWeight_originalXWGTUP,
    )


def compute_sum_weights(analysis_config: str) -> None:
    configs = tomli.loads(Path(analysis_config).read_text(encoding="utf-8"))

    total_samples = 0
    for sample in configs:
        if not configs[sample]["is_data"]:
            total_samples += 1

    print("Starting ray cluster ...")
    ray.init(
        log_to_driver=False,
        num_cpus=100,
        _temp_dir="{}/tmp_ray".format(os.getcwd()),
    )
    results = {}
    for idx_sample, sample in enumerate(configs):
        if not configs[sample]["is_data"]:
            results[sample] = {}
            for y in Years:
                if f"output_files_{y}" in configs[sample]:
                    if len(configs[sample][f"output_files_{y}"]):
                        results[sample][y] = process.remote(
                            configs[sample][f"output_files_{y}"],
                            y,
                            configs[sample].get("generator_filter_key"),
                        )

    weights = {}
    for idx_sample, sample in enumerate(configs):
        if not configs[sample]["is_data"]:
            weights[sample] = {}
            for y in Years:
                weights[sample][y] = {}
                if f"output_files_{y}" in configs[sample]:
                    log.info(
                        f"Collecting results for sample: {sample} [{idx_sample}/{total_samples}] - {y}"
                    )
                    if y in results[sample].keys():
                        _weights = ray.get(results[sample][y])
                        weights[sample][y]["sum_genWeight"] = _weights.sum_genWeight
                        weights[sample][y]["sum_genWeight_pass_generator_filter"] = (
                            _weights.sum_genWeight_pass_generator_filter
                        )
                        weights[sample][y]["sum_LHEWeight"] = _weights.sum_LHEWeight
                        weights[sample][y]["sum_LHEWeight_pass_generator_filter"] = (
                            _weights.sum_LHEWeight_pass_generator_filter
                        )
                        weights[sample][y]["raw_events"] = _weights.raw_events
                        weights[sample][y]["pass_generator_filter"] = (
                            _weights.pass_generator_filter
                        )
                        weights[sample][y]["has_genWeight"] = _weights.has_genWeight
                        weights[sample][y]["has_LHEWeight_originalXWGTUP"] = (
                            _weights.has_LHEWeight_originalXWGTUP
                        )

    with open("sum_weights.json", "w") as outfile:
        json.dump(weights, outfile, indent=4)
