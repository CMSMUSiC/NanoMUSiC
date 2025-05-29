import hashlib
import logging
import os
import random
import shutil
import subprocess
import sys
from dataclasses import asdict, dataclass
from multiprocessing import Pool, process
from pathlib import Path

import tomli
from metadata import Lumi, Years
from rich.logging import RichHandler
from rich.progress import track

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
            f"hadd -f btag_eff_maps/btag_eff_map_{process_group}.root btag_eff_maps_buffer/{process_group}_*.root"
        )
        result = subprocess.run(
            f"hadd -f btag_eff_maps/btag_eff_map_{process_group}.root btag_eff_maps_buffer/{process_group}_*.root",
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

    # result = subprocess.run(
    #     f"rm -rf btag_eff_maps_buffer",
    #     shell=True,
    #     capture_output=True,
    #     text=True,
    # )
    # if not result.returncode == 0:
    #     print(
    #         "ERROR: Could not delete buffered files",
    #         result.stderr,
    #         file=sys.stderr,
    #     )


def plot_tefficiency_2d(
    teff,
    output_filename,
    canvas_width=800,
    canvas_height=600,
    title="",
    x_title="",
    y_title="",
    z_title="Efficiency",
    draw_option="COLZ text",
    z_min=None,
    z_max=None,
    palette=1,
):
    from ROOT import TCanvas, gStyle, gPad

    # Set ROOT style
    gStyle.SetOptStat(0)
    gStyle.SetPalette(palette)

    # Create canvas
    canvas = TCanvas("c1", "2D Efficiency Plot", canvas_width, canvas_height)
    canvas.SetRightMargin(0.15)  # Make room for color palette

    canvas.SetLogx()

    # Get the painted histogram for styling
    # Note: TEfficiency creates a temporary histogram when drawn
    teff.Draw(draw_option)

    # Get the histogram that was just drawn
    hist = teff.GetPaintedHistogram()

    if hist:
        # Set titles
        if title:
            hist.SetTitle(title)
        if x_title:
            hist.GetXaxis().SetTitle(x_title)
        if y_title:
            hist.GetYaxis().SetTitle(y_title)
        if z_title:
            hist.GetZaxis().SetTitle(z_title)

        # Set Z-axis range if specified
        if z_min is not None or z_max is not None:
            if z_min is None:
                z_min = hist.GetMinimum()
            if z_max is None:
                z_max = hist.GetMaximum()
            hist.GetZaxis().SetRangeUser(z_min, z_max)

    # Update canvas
    canvas.Update()

    # Save to PDF
    canvas.SaveAs(output_filename)

    # Clean up
    canvas.Close()


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
            print(f"-- Will make TEfficiency maps for {process_group} - {tagger}")
            num = getattr(out_file, f"[{process_group}]_{tagger}_num")
            den = getattr(out_file, f"[{process_group}]_{tagger}_den")
            if process_group == "tG" and tagger == "c":
                den.Print("all")
                print()
                num.Print("all")

            if not ROOT.TEfficiency.CheckConsistency(num, den):
                nx_bins = den.GetNbinsX()
                ny_bins = den.GetNbinsY()

                for x_bin in range(1, nx_bins + 1):
                    for y_bin in range(1, ny_bins + 1):
                        current_content_den = den.GetBinContent(x_bin, y_bin)
                        current_content_num = num.GetBinContent(x_bin, y_bin)

                        if (
                            current_content_den < current_content_num
                            and current_content_den < 0
                        ):
                            den.SetBinContent(x_bin, y_bin, current_content_num)

            pEff = ROOT.TEfficiency(num, den)
            pEff.SetName(f"{process_group}_{tagger}_eff")
            pEff.Write()

            plot_tefficiency_2d(
                pEff,
                f"btag_eff_maps/{process_group}_{tagger}_eff.pdf",
                canvas_width=800,
                canvas_height=600,
                title=f"{process_group}_{tagger}",
                x_title="pt",
                y_title="#eta",
                z_title="Efficiency",
                draw_option="COLZ",
                z_min=None,
                z_max=None,
                palette=1,
            )

        out_file.Close()
