import scanner_imp as scanner
from multiprocessing import Pool
from rich.progress import Progress
import fnmatch
import os
import sys
import json
import numpy as np
from pydantic import BaseModel
from distribution_model import (
    ScanDistribution,
    DistributionType,
    ScanYear,
    MCBinsBuilder,
)
from metadata import make_ec_nice_name
from tools import configure_root

from ROOT import TFile


configure_root()

MC_THRESHOLD = 0.1


def make_shifts(num_rounds: int, variations: list[str]) -> None:
    shifts = dict(
        zip(
            variations,
            np.random.normal(
                loc=1.0, scale=1.0, size=(len(variations), num_rounds)
            ).tolist(),
        )
    )

    with open("shifts.json", "w") as json_file:
        json.dump(shifts, json_file, indent=4)


class ScanProps(BaseModel):
    json_file_path: str
    output_directory: str
    rounds: int
    start_round: int = 0


def do_scan(scan_props: ScanProps) -> str | None:
    lut_file_path = "{}/bin/lookuptable.bin".format(os.getenv("MUSIC_BASE"))
    shifts_file_path = "shifts.json"

    if not os.path.exists("{}/".format(os.getenv("MUSIC_BASE"))):
        print(
            'ERROR: Could not start scanner. LUT file does not exist. Did you executed "ninja lut"?',
            file=sys.stderr,
        )
        sys.exit(-1)

    if not os.path.exists(scan_props.json_file_path):
        print(
            "ERROR: Could not start scanner. Input file not found.",
            file=sys.stderr,
        )
        sys.exit(-1)

    with open(scan_props.json_file_path, "r") as json_file:
        data = json.load(json_file)
    distribution = ScanDistribution(**data)

    data_scan: bool = scanner.scan(
        scan_props.json_file_path,
        "{}/{}".format(
            scan_props.output_directory, distribution.name.replace("+", "_")
        ),
        scan_props.rounds,
        scan_props.start_round,
        shifts_file_path,
        lut_file_path,
        scan_type="data",
    )

    if not data_scan:
        print("ERROR: Could not perform Data scan.", file=sys.stderr)
        sys.exit(-1)

    mc_scan: bool = scanner.scan(
        scan_props.json_file_path,
        "{}/{}".format(
            scan_props.output_directory, distribution.name.replace("+", "_")
        ),
        scan_props.rounds,
        scan_props.start_round,
        shifts_file_path,
        lut_file_path,
        scan_type="mc",
    )
    if not mc_scan:
        print("ERROR: Could not perform MC scan.", file=sys.stderr)
        sys.exit(-1)

    if data_scan and mc_scan:
        return "{} - {} - {}".format(
            distribution.name, distribution.distribution, distribution.year
        )


def build_scan_jobs_task(
    args: tuple[str, str, str, int],
) -> tuple[list[ScanProps], list[str]]:
    distribution_file, output_dir, distribution_type, n_rounds = args
    temp_scan_props: list[ScanProps] = []
    this_variations: list[str] = []

    root_file = TFile.Open(distribution_file)
    distribution_names: list[str] = [
        str(k.GetName()) for k in root_file.GetListOfKeys()
    ]

    for dist_name in distribution_names:
        if (
            "Run2" in dist_name
            and "counts" not in dist_name
            and distribution_type in dist_name
        ):
            dist = root_file.Get(dist_name)
            if dist.has_mc(MC_THRESHOLD) and dist.has_data():
                this_variations = [
                    str(var) for var, _ in dist.m_systematics_uncertainties
                ]

                scan_distribution_type = DistributionType.invariant_mass
                if dist.m_distribution_name == "sum_pt":
                    scan_distribution_type = DistributionType.sum_pt
                if dist.m_distribution_name == "met":
                    scan_distribution_type = DistributionType.met

                ec_nice_name = make_ec_nice_name(
                    make_ec_nice_name(str(dist.m_event_class_name))
                )

                raw_data_counts = dist.get_data_counts()
                data_counts = []
                for i in range(raw_data_counts.size()):
                    data_counts.append(raw_data_counts[i])

                temp_scan_props.append(
                    ScanProps(
                        json_file_path=ScanDistribution(
                            name=ec_nice_name,
                            distribution=scan_distribution_type,
                            year=ScanYear.Run2,
                            MCBins=MCBinsBuilder(dist.get_mcbins_props()).build(),
                            DataBins=data_counts,
                        ).save(output_dir),
                        output_directory=output_dir,
                        rounds=n_rounds,
                    )
                )

                # prepare output area
                if not os.path.exists(
                    "{}/{}".format(output_dir, ec_nice_name.replace("+", "_"))
                ):
                    os.makedirs(
                        "{}/{}".format(output_dir, ec_nice_name.replace("+", "_"))
                    )

    root_file.Close()

    return temp_scan_props, this_variations


def launch_scan(
    input_dir: str,
    patterns: list[str],
    distribution_type: str,
    output_dir: str = "scan_results",
    num_cpus: int = 128,
    do_clean: bool = False,
    n_rounds: int = 100_000,
):
    if not os.path.isdir(input_dir):
        print("ERROR: Input directory does not exists.")
        sys.exit(-1)

    if do_clean:
        print("Cleanning output directory ...")
        os.system("rm -rf {}".format(output_dir))
    os.system("mkdir -p {}".format(output_dir))

    def make_distribution_paths(inputs_dir: str, patterns: list[str]) -> list[str]:
        distribution_paths: list[str] = []
        for root, _, files in os.walk(inputs_dir):
            for file in files:
                if any(
                    fnmatch.fnmatch(file, "*" + pattern.replace("+", "_") + ".root")
                    for pattern in patterns
                ):
                    distribution_paths.append(os.path.join(root, file))
        return distribution_paths

    distribution_files = make_distribution_paths(input_dir, patterns)
    if len(distribution_files) == 0:
        print("WARNING: No distribution matches the requirements.")
        sys.exit(1)

    # Will build scan jobs
    variations: list[str] = []
    scan_props: list[ScanProps] = []
    with Pool(min(len(distribution_files), num_cpus)) as p:
        with Progress() as progress:
            task = progress.add_task(
                "Building scan jobs [{} distribution files] ...".format(
                    len(distribution_files),
                ),
                total=len(distribution_files),
            )
            for this_scan_props, this_variations in p.imap_unordered(
                build_scan_jobs_task,
                [
                    (dist, output_dir, distribution_type, n_rounds)
                    for dist in distribution_files
                ],
            ):
                scan_props += this_scan_props
                variations = list(set(variations + this_variations))
                progress.advance(task)

    make_shifts(n_rounds, variations)

    # Will make launch scan and save results
    with Pool(min(len(scan_props), num_cpus)) as p:
        with Progress() as progress:
            task = progress.add_task(
                "Performing {} scans ...".format(len(scan_props)),
                total=len(scan_props),
            )
            for job in p.imap_unordered(do_scan, scan_props):
                progress.console.print("Done: {}".format(job))
                progress.advance(task)

    print("Copying index.php ...")
    os.system(
        r"find ___OUTPUT_DIR___/ -type d -exec cp $MUSIC_BASE/NanoMUSiC/Plotter/assets/index.php {} \;".replace(
            "___OUTPUT_DIR___", output_dir
        )
    )

    print("Done.")


def get_p_tilde(scan_result_data_file_path: str, scan_mc_data_file_path: str) -> None:
    print("Loading data results ...")
    with open(scan_result_data_file_path, "r") as file:
        data = json.load(file)
    p_val_data = data["ScanResults"][0]["CompareScore"]
    name = data["name"]
    distribution = data["distribution"]
    lower_edge = data["ScanResults"][0]["lowerEdge"]
    width = data["ScanResults"][0]["width"]
    assert data["ScanResults"][0]["skippedScan"] == False
    print("... done.")

    p_val_mc = []
    print("Loading MC results ...")
    with open(scan_mc_data_file_path, "r") as file:
        data = json.load(file)
        for item in data["ScanResults"]:
            if not item["skippedScan"]:
                p_val_mc.append(item["CompareScore"])
    print("... done.")

    print()
    print("Event class: {}".format(name))
    print("Distribution: {}".format(distribution))
    print("RoI: [{} - {}]".format(lower_edge, lower_edge + width))
    print("p-value: {}".format(p_val_data))

    p_tilde = np.sum(np.array(p_val_mc) < p_val_data) / float(len(p_val_mc))
    if np.sum(np.array(p_val_mc) <= p_val_data) == 0.0:
        p_tilde = 1 / float(len(p_val_mc))

    print("p-tilde: {}".format(p_tilde))
    print()
