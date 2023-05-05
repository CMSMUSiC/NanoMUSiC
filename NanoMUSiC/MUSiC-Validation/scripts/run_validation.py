#!/usr/bin/env python3
from functools import partial

# from itertools import repeat
from multiprocessing import Pool
from typing import Any
from tqdm import tqdm
import toml
import argparse
import shutil
import os
import subprocess
import glob
import shlex


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        required=True,
        help='Task configuration (TOML) file, produced by "analysis_config_builder.py"',
    )
    parser.add_argument("-s", "--sample", help="Sample to be processed.", required=True)
    parser.add_argument("-y", "--year", help="Year to be processed.", required=True)
    parser.add_argument("-j", "--jobs", help="Pool size.", type=int, default=100)
    # parser.add_argument("--veto", help="path to run_number/event_number veto maps")
    # parser.add_argument(
    #     "--merge", help="will merge validation results", action="store_true"
    # )
    parser.add_argument("--debug", help="print debugging info", action="store_true")

    return parser.parse_args()


def merge_cutflow_histograms(output_path, input_files, debug: bool = False):
    merge_result = subprocess.run(
        ["hadd", "-f", "-T", f"{output_path}/cutflow.root", *input_files],
        capture_output=True,
    )
    if debug:
        print(merge_result.stdout.decode("utf-8"))
    if merge_result.returncode != 0:
        error = merge_result.stderr.decode("utf-8")
        raise RuntimeError(f"ERROR: could not merge cutflow files.\n{error}")


def run_validation(
    process_name: str,
    year: str,
    is_data: bool,
    output_path: str,
    effective_x_section: float,
    input_file: str,
    debug: bool = False,
) -> bool:
    validation_result = subprocess.run(
        shlex.split(
            f"validation --process {process_name} --year {year} {(lambda is_data: '--is_data' if is_data else '')(is_data)} --output {output_path} --xsection {str(effective_x_section)} --input {input_file}"
        ),
        capture_output=True,
    )
    if debug:
        print(validation_result.stdout.decode("utf-8"))

    if validation_result.returncode != 0:
        error = validation_result.stderr.decode("utf-8")
        raise RuntimeError(f"ERROR: could process validation.\n{error}\n{input_file}")
    return True


def main():
    print("\n\n📶 [ MUSiC Validation ] 📶\n")

    # parse arguments
    args = parse_args()

    # parse config file
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)
    year: str = args.year
    luminosity: float = task_config["Lumi"][year]

    task_config: dict[str, Any] = task_config[args.sample]
    is_data: bool = task_config["is_data"]
    process: str = args.sample
    input_files: list[str] = list(
        filter(lambda file: f"{year}_date" in file, task_config["output_files"])
    )

    output_path: str = f"validation_outputs/{year}/{process}"

    print(f"[ MUSiC Validation ] Process: {process} - Year: {year}\n")

    print("[ MUSiC Validation ] Loading samples ...\n")
    effective_x_section: float = 1.0
    if not is_data:
        effective_x_section = (
            task_config["XSec"]
            * task_config["FilterEff"]
            * task_config["kFactor"]
            * luminosity
        )

    print("[ MUSiC Validation ] Preparing output directory ...\n")
    shutil.rmtree(output_path, ignore_errors=True)
    os.system(f"rm -rf validation_outputs/{year}/{process}*.root")
    os.makedirs(output_path)

    print("[ MUSiC Validation ] Merging cutflow histograms ...\n")
    merge_cutflow_histograms(output_path, input_files)

    print("[ MUSiC Validation ] Launching processes ...\n\n")
    with Pool(min(args.jobs, len(input_files))) as pool:
        list(
            tqdm(
                pool.imap_unordered(
                    partial(
                        run_validation,
                        process,
                        year,
                        is_data,
                        output_path,
                        effective_x_section,
                    ),
                    input_files,
                ),
                total=len(input_files),
            )
        )

    print("\n[ MUSiC Validation ] Merging results ...\n\n")
    outputs_file_names = [
        "z_to_ele_ele_x",
        "z_to_mu_mu_x",
        "z_to_ele_ele_x_Z_mass",
        "z_to_mu_mu_x_Z_mass",
    ]
    for output in outputs_file_names:
        merge_result = subprocess.run(
            [
                "hadd",
                "-f",
                "-T",
                f"validation_outputs/{year}/{process}_{output}.root",
                *glob.glob(f"{output_path}/{output}_[0-9]*.root"),
            ],
            capture_output=True,
        )
        if merge_result.returncode != 0:
            error = merge_result.stderr.decode("utf-8")
            raise RuntimeError(f"ERROR: could not merge validation files.\n{error}")

        # cleanning ...
        cleanning_result = subprocess.run(
            ["rm", "-rf", *glob.glob(f"{output_path}/{output}_*.root")],
            capture_output=True,
        )
        if cleanning_result.returncode != 0:
            error = cleanning_result.stderr.decode("utf-8")
            raise RuntimeError(f"ERROR: could not clear output path.\n{error}")


if __name__ == "__main__":
    main()
