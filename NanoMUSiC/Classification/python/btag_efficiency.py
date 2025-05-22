import subprocess
import random
from pathlib import Path
import shutil
import os
import hashlib
from dataclasses import dataclass, asdict
from multiprocessing import Pool, process
import tomli
import logging
from rich.logging import RichHandler
from metadata import Years
import sys
from rich.progress import track

from metadata import Lumi

FORMAT = "%(message)s"
logging.basicConfig(
    level=logging.DEBUG,
    format="%(message)s",
    datefmt="%m/%d/%Y %I:%M:%S %p",
    handlers=[RichHandler()],
)
log = logging.getLogger("main")


def compile_lib(path: str) -> None:
    if "ROOT" not in sys.modules:
        import ROOT

    ROOT.gSystem.AddIncludePath(
        f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/include"
    )
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC/src")
    ROOT.gSystem.AddIncludePath(
        f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/src"
    )
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC/include")
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('LCGVIEW_PATH')}/include")
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('LCGVIEW_PATH')}/include/nlohmann")

    compilation_res = ROOT.gSystem.CompileMacro(
        path,
        "fgO-",
        "",
        "",
        1,
    )

    if not compilation_res:
        raise RuntimeError(f"Could not compile {path}.")


def preamble():
    # check VOMS Proxy
    p = subprocess.run(["voms-proxy-info"], capture_output=True)
    if p.returncode != 0:
        log.error("No proxy found.")
        sys.exit(-1)

    log.info("Compiling libraries ...")
    libs = [
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_btag_efficiency.cpp",
    ]
    with Pool(processes=len(libs)) as pool:
        pool.map(compile_lib, libs)


@dataclass
class BTagEffInputs:
    sample: str
    process_group: str
    generator_filter: str
    input_file: str
    year: str
    sum_weights_json_filepath: str
    x_section: float
    luminosity: float
    filter_eff: float
    k_factor: float


def compute_btag_efficiency_imp(inputs: BTagEffInputs):
    import ROOT

    ROOT.gErrorIgnoreLevel = ROOT.kError  # Suppresses warnings, shows only errors

    ROOT.gSystem.AddIncludePath(
        f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/include"
    )
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC/include")
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('LCGVIEW_PATH')}/include")
    ROOT.gSystem.AddIncludePath(f"-I{os.getenv('LCGVIEW_PATH')}/include/nlohmann")

    libs = [
        ROOT.gSystem.Load(
            f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_btag_efficiency_cpp.so",
        ),
    ]

    if any([lib < 0 for lib in libs]):
        raise RuntimeError("ERROR: Could not load libraries.")

    try:
        res = ROOT.compute_btag_efficiency(**asdict(inputs))
        return inputs, res
    except Exception as e:
        log.warning(
            "Could not process {}. Will try reading it from RWTH.".format(
                inputs.input_file
            )
        )

    try:
        inputs.input_file = inputs.input_file.replace(
            "root://cms-xrd-global.cern.ch//",
            # "davs://grid-webdav.physik.rwth-aachen.de:2889",
            "root://grid-dcache.physik.rwth-aachen.de//",
        )

        res = ROOT.compute_btag_efficiency(**asdict(inputs))
        log.info("Looks like reading {} from RWTH worked.".format(inputs.input_file))
        return inputs, res
    except Exception as e:
        log.warning(
            "Could not process {}. Will try reading it from FNAL global redirector.".format(
                inputs.input_file
            )
        )

    try:
        inputs.input_file = inputs.input_file.replace(
            # "davs://grid-webdav.physik.rwth-aachen.de:2889",
            "root://grid-dcache.physik.rwth-aachen.de//",
            "root://cmsxrootd.fnal.gov//",
        )

        res = ROOT.compute_btag_efficiency(**asdict(inputs))
        log.info(
            "Looks like reading {} from FNAL global redirector worked.".format(
                inputs.input_file
            )
        )
        return inputs, res
    except Exception as e:
        log.warning(
            "Could not process {}. Will try reading it from INFN global redirector.".format(
                inputs.input_file
            )
        )

    try:
        inputs.input_file = inputs.input_file.replace(
            "root://cmsxrootd.fnal.gov//",
            "root://xrootd-cms.infn.it//",
        )

        res = ROOT.compute_btag_efficiency(**asdict(inputs))
        log.info(
            "Looks like reading {} from INFN global redirector worked.".format(
                inputs.input_file
            )
        )
        return inputs, res
    except Exception as e:
        log.error(
            "Could not process {}. All redirectors have failed.\n{}".format(
                inputs.input_file, e
            )
        )

        raise RuntimeError("Could not process file {}. {}".format(inputs.input_file, e))
        # return inputs, None


def compute_btag_efficiency(analysis_config: str) -> None:
    import ROOT

    configs = tomli.loads(Path(analysis_config).read_text(encoding="utf-8"))

    total_files = 0
    for sample in configs:
        if not configs[sample]["is_data"]:
            for y in Years:
                if f"input_files_{y}" in configs[sample]:
                    total_files += len(configs[sample][f"input_files_{y}"])

    btag_eff_inputs = []
    for sample in configs:
        if not configs[sample]["is_data"]:
            for year in Years:
                if f"input_files_{year}" in configs[sample]:
                    if len(configs[sample][f"input_files_{year}"]):
                        for idx, file in enumerate(
                            configs[sample][f"input_files_{year}"]
                        ):
                            # # DEBUG
                            # if idx == 0:
                            btag_eff_inputs.append(
                                BTagEffInputs(
                                    sample=sample,
                                    process_group=configs[sample][f"ProcessGroup"],
                                    generator_filter=configs[sample][
                                        "generator_filter_key"
                                    ]
                                    if "generator_filter_key" in configs[sample]
                                    else "",
                                    input_file=file,
                                    year=year,
                                    sum_weights_json_filepath="sum_weights.json",
                                    x_section=configs[sample][f"XSec"],
                                    luminosity=Lumi.lumi[year],
                                    filter_eff=configs[sample][f"FilterEff"],
                                    k_factor=configs[sample][f"kFactor"],
                                )
                            )
    try:
        shutil.rmtree("btag_eff_maps_buffer")
    except FileNotFoundError:
        pass

    os.mkdir("btag_eff_maps_buffer")

    with Pool() as p:
        for idx, res in track(
            enumerate(
                p.imap_unordered(
                    compute_btag_efficiency_imp,
                    random.sample(btag_eff_inputs, len(btag_eff_inputs)),
                )
            ),
            description="Computing b-tag efficiency ...",
            total=len(btag_eff_inputs),
        ):
            inputs, _ = res
            log.info(
                "Done: {} - {} | {}/{}".format(
                    inputs.sample, inputs.year, idx, len(btag_eff_inputs)
                )
            )


def merge_btag_efficiency_maps() -> None:
    try:
        shutil.rmtree("btag_eff_maps")
    except FileNotFoundError:
        pass
    os.mkdir("btag_eff_maps")

    path = Path("btag_eff_maps_buffer")

    processes_groups = set(
        [
            str(file).replace("btag_eff_maps_buffer", "").split("_")[0].replace("/", "")
            for file in path.glob("*")
            if str(file).endswith(".root")
        ]
    )
    for process_group in track(processes_groups):
        print(
            f"hadd -f -j btag_eff_maps/btag_eff_map_{process_group}.root btag_eff_maps_buffer/{process_group}_*.root"
        )
        result = subprocess.run(
            f"hadd -f -j btag_eff_maps/btag_eff_map_{process_group}.root btag_eff_maps_buffer/{process_group}_*.root",
            shell=True,
            capture_output=True,
            text=True,
        )
        if not result.returncode == 0:
            print(
                f"ERROR: Could not merge btag maps for: {process_group}",
                result.stderr,
                file=sys.stderr,
            )
            sys.exit(-1)

    result = subprocess.run(
        f"rm -rf btag_eff_maps_buffer",
        shell=True,
        capture_output=True,
        text=True,
    )
    if not result.returncode == 0:
        print(
            "ERROR: Could not delete buffered files",
            result.stderr,
            file=sys.stderr,
        )


def make_teffs() -> None:
    import ROOT

    ROOT.gErrorIgnoreLevel = ROOT.kError  # Suppresses warnings, shows only errors

    path = Path("btag_eff_maps")

    processes_groups = set(
        [
            str(file).replace("btag_eff_map", "").split("_")[1].replace(".root", "")
            for file in path.glob("*")
            if str(file).endswith(".root")
        ]
    )
    for process_group in track(processes_groups):
        out_file = ROOT.TFile(
            f"btag_eff_maps/btag_eff_map_{process_group}.root", "UPDATE"
        )
        for tagger in ["light", "c", "b"]:
            num = getattr(out_file, f"[{process_group}]_{tagger}_num")
            den = getattr(out_file, f"[{process_group}]_{tagger}_den")

            if ROOT.TEfficiency.CheckConsistency(num, den):
                pEff = ROOT.TEfficiency(num, den)
                pEff.SetName(f"{process_group}_{tagger}_eff")
                pEff.Write()

        out_file.Close()
