#!/usr/bin/env python3

from multiprocessing import Pool
from typing import Any
from tqdm import tqdm
import toml
import argparse
import os
import subprocess
import shlex
import tempfile
from collections import defaultdict
from pprint import pprint

from sample_helpers import get_year_era

years = ["2016APV", "2016", "2017", "2018"]


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        required=True,
        help='Task configuration (TOML) file, produced by "analysis_config_builder.py"',
    )
    parser.add_argument("-s", "--sample", help="Sample to be processed.")
    parser.add_argument("-y", "--year", help="Year to be processed.")
    parser.add_argument(
        "--all_data",
        help='Starts validation for all Data samples. Incompatible with "--sample" and "--all_mc".',
        action="store_true",
    )
    parser.add_argument(
        "--all_mc",
        help='Starts validation for all MC samples. Incompatible with "--sample" and "--all-data".',
        action="store_true",
    )
    parser.add_argument("-j", "--jobs", help="Pool size.", type=int, default=100)
    parser.add_argument(
        "-e", "--executable", help="Validation excutable.", default="validation"
    )
    parser.add_argument("--debug", help="print debugging info", action="store_true")

    args = parser.parse_args()

    # quality control
    if (args.all_data and (args.sample or args.all_mc)) or (
        args.all_mc and (args.sample or args.all_data)
    ):
        raise RuntimeError(
            'ERROR: Could not satrt validation. "--all_data" is incompatible with "--all_mc" and "--sample".'
        )

    if args.sample and not (args.year):
        raise RuntimeError(
            'ERROR: Could not start validation. When "--sample" is set, "--year" is required.'
        )
    return args


def merge_cutflow_histograms(
    process, year, output_path, input_files, debug: bool = False
):
    merge_result = subprocess.run(
        [
            "hadd",
            "-f",
            "-T",
            f"{output_path}/cutflow_{process}_{year}.root",
            *input_files,
        ],
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
    executable: str,
    input_file: str,
) -> bool:
    debug: bool = False

    # default is MC
    cmd_str: str = f"{executable} --process {process_name} --year {year} --output {output_path} --xsection {str(effective_x_section)} --input {input_file}"
    if is_data:
        cmd_str: str = f"{executable} --process {process_name} --year {year} --is_data --output {output_path} --xsection {str(effective_x_section)} --input {input_file}"

    if debug:
        print(f"Executing: {cmd_str}")

    validation_result = subprocess.run(
        shlex.split(cmd_str),
        capture_output=True,
    )
    if debug:
        print(validation_result.stdout.decode("utf-8"))

    if validation_result.returncode != 0:
        error = validation_result.stderr.decode("utf-8")
        output = validation_result.stdout.decode("utf-8")
        raise RuntimeError(
            f"ERROR: could process validation.\n{error}\n{output}\n{input_file}"
        )
    return True


def make_processed_events():
    with open("dummy_processed_events.bin", mode="wb") as f:
        pass
    return "dummy_processed_events.bin"


def dump_list_to_temp_file(list_of_files):
    # Create a temporary file
    with tempfile.NamedTemporaryFile(mode="w", delete=False) as temp_file:
        # Write each list entry to a separate line in the file
        for item in list_of_files:
            temp_file.write(str(item) + "\n")

    # Return the path of the temporary file
    return temp_file.name


def validation(args):
    (
        process,
        year,
        luminosity,
        is_data,
        xsection,
        filter_eff,
        k_factor,
        input_files,
        executable,
    ) = list(args.values())

    output_path: str = f"validation_outputs/{year}"

    # print("[ MUSiC Validation ] Loading samples ...\n")
    effective_x_section: float = 1.0
    if not is_data:
        effective_x_section = xsection * filter_eff * k_factor * luminosity

    # print("[ MUSiC Validation ] Preparing output directory ...\n")
    os.system(f"rm -rf validation_outputs/{year}/*_{process}_{year}.root")

    # print("[ MUSiC Validation ] Merging cutflow histograms ...\n")
    merge_cutflow_histograms(process, year, output_path, input_files)

    # print("[ MUSiC Validation ] Starting validation ...\n")
    inputs = dump_list_to_temp_file(input_files)
    run_validation(
        process,
        year,
        is_data,
        output_path,
        effective_x_section,
        executable,
        inputs,
    )
    os.system(f"rm -rf {inputs} > /dev/null")


def main():
    print("\n\n📶 [ MUSiC Validation ] 📶\n")

    # parse arguments
    args = parse_args()

    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    # data workflow
    if args.all_data:
        validation_arguments = {}
        for sample in task_config:
            if (
                sample != "Lumi"
                and sample != "Global"
                and task_config[sample]["is_data"]
            ):
                year, era = get_year_era(sample)
                if f"Data_{year}_{era}" in validation_arguments:
                    validation_arguments[f"Data_{year}_{era}"]["input_files"].extend(
                        task_config[sample]["output_files"]
                    )
                else:
                    validation_arguments[f"Data_{year}_{era}"] = {
                        "process": f"Data_{year}_{era}",
                        "year": year,
                        "luminosity": 1,
                        "is_data": True,
                        "xsection": 1,
                        "filter_eff": 1,
                        "k_factor": 1,
                        "input_files": task_config[sample]["output_files"],
                        "executable": args.executable,
                    }

                if not (os.path.isdir(f"validation_outputs/{year}")):
                    os.system(f"mkdir -p validation_outputs/{year}")

        validation_arguments = list(validation_arguments.values())
        with Pool(min(args.jobs, len(validation_arguments))) as pool:
            # with Pool(1) as pool:
            list(
                tqdm(
                    pool.imap_unordered(validation, validation_arguments),
                    total=len(validation_arguments),
                    unit=" sample",
                )
            )

        exit(0)

    if args.all_mc:
        validation_arguments = []
        for sample in task_config:
            if (
                sample != "Lumi"
                and sample != "Global"
                and not (task_config[sample]["is_data"])
            ):
                for year in years:
                    if f"das_name_{year}" in task_config[sample].keys():
                        validation_arguments.append(
                            {
                                "process": sample,
                                "year": year,
                                "luminosity": task_config["Lumi"][year],
                                "is_data": task_config[sample]["is_data"],
                                "xsection": task_config[sample]["XSec"],
                                "filter_eff": task_config[sample]["FilterEff"],
                                "k_factor": task_config[sample]["kFactor"],
                                "input_files": list(
                                    filter(
                                        lambda file: f"{year}_date" in file,
                                        task_config[sample]["output_files"],
                                    )
                                ),
                                "executable": args.executable,
                            }
                        )
                    if not (os.path.isdir(f"validation_outputs/{year}")):
                        os.system(f"mkdir -p validation_outputs/{year}")

        with Pool(min(args.jobs, len(validation_arguments))) as pool:
            list(
                tqdm(
                    pool.imap_unordered(validation, validation_arguments),
                    total=len(validation_arguments),
                    unit="sample",
                )
            )

        exit(0)


if __name__ == "__main__":
    main()
