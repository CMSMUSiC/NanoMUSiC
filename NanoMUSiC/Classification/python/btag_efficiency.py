import random
import shutil
import os
from dataclasses import dataclass, asdict
import subprocess
import ROOT
from ROOT import gSystem
from multiprocessing import Pool
import tomli
from pathlib import Path
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
        f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_btag_efficiency.cpp",
        "fgO-",
        "",
        "",
        1,
    )
    if not compilation_res:
        log.error("Could not compile compute_btag_efficiency.cpp.")
        sys.exit(-1)


@dataclass
class BTagEffInputs:
    sample: str
    process_group: str
    generator_filter: str | None
    input_file: str
    year: str


def compute_btag_efficiency_imp(inputs: BTagEffInputs):
    import ROOT

    gSystem.AddIncludePath(
        f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/include"
    )
    gSystem.AddIncludePath(f"-I{os.getenv('MUSIC_BASE')}/NanoMUSiC/MUSiC/include")

    libs = [
        gSystem.Load(
            f"{os.getenv('MUSIC_BASE')}/NanoMUSiC/Classification/Utils/PreProcessing/compute_btag_efficiency_cpp.so",
        ),
        # gSystem.Load("libfmt.so"),
    ]

    if any([lib < 0 for lib in libs]):
        print("ERROR: Could not load libraries.")
        sys.exit(-1)

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
            "davs://grid-webdav.physik.rwth-aachen.de:2889",
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
            "davs://grid-webdav.physik.rwth-aachen.de:2889",
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

        # we can allow it to fail
        # raise RuntimeError("Could not process file {}".format(inputs.input_file))
        return inputs, None


def compute_btag_efficiency(analysis_config: str) -> None:
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
                        for file in configs[sample][f"input_files_{year}"]:
                            btag_eff_inputs.append(
                                BTagEffInputs(
                                    sample=sample,
                                    process_group=configs[sample][f"ProcessGroup"],
                                    generator_filter=ROOT.std.optional[str](),
                                    input_file=file,
                                    year=year,
                                )
                            )
    try:
        shutil.rmtree("btag_eff_maps_buffer")
    except FileNotFoundError:
        pass

    os.mkdir("btag_eff_maps_buffer")

    with Pool() as p:
        # for idx, job in enumerate(
        #     p.imap_unordered(
        #         compute_btag_efficiency_imp,
        #         random.sample(btag_eff_inputs, len(btag_eff_inputs)),
        #     )
        # ):
        for idx, job in track(
            enumerate(
                p.imap_unordered(
                    compute_btag_efficiency_imp,
                    random.sample(btag_eff_inputs, len(btag_eff_inputs)),
                )
            ),
            description="Processing...",
            total=len(btag_eff_inputs),
        ):
            inputs, _ = job
            log.info(
                "Done: {} - {} | {}/{}".format(
                    inputs.sample, inputs.year, idx, len(btag_eff_inputs)
                )
            )


def merge_btag_efficiency_maps() -> None:
    print("TODO: Will merge btag efficiency maps...")
