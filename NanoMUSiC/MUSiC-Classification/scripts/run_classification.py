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

# globals ....
# I am sorry, me from the future ...
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
        help='Starts classification for all Data samples. Incompatible with "--sample" and "--all_mc".',
        action="store_true",
    )
    parser.add_argument(
        "--all_mc",
        help='Starts classification for all MC samples. Incompatible with "--sample" and "--all-data".',
        action="store_true",
    )
    parser.add_argument(
        "--all",
        help='Starts classification for all MC and Data samples. Incompatible with "--sample", "--all-data" and  "--all-mc".',
        action="store_true",
    )
    parser.add_argument("-j", "--jobs", help="Pool size.", type=int, default=100)
    parser.add_argument(
        "-e", "--executable", help="classification excutable.", default="classification"
    )
    parser.add_argument("--debug", help="print debugging info", action="store_true")

    args = parser.parse_args()

    # argments quality control
    if (args.all_data and (args.sample or args.all_mc)) or (
        args.all_mc and (args.sample or args.all_data)
    ):
        raise RuntimeError(
            'ERROR: Could not satrt classification. "--all_data" is incompatible with "--all_mc" and "--sample".'
        )

    if args.sample and not (args.year):
        raise RuntimeError(
            'ERROR: Could not start classification. When "--sample" is set, "--year" is required.'
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


def run_classification(
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

    classification_result = subprocess.run(
        shlex.split(cmd_str),
        capture_output=True,
    )
    if debug:
        print(classification_result.stdout.decode("utf-8"))

    if classification_result.returncode != 0:
        error = classification_result.stderr.decode("utf-8")
        output = classification_result.stdout.decode("utf-8")
        raise RuntimeError(
            f"ERROR: could process classification.\n{error}\n{output}\n{input_file}"
        )
    return True


def dump_list_to_temp_file(list_of_files):
    # Create a temporary file
    with tempfile.NamedTemporaryFile(mode="w", delete=False) as temp_file:
        # Write each list entry to a separate line in the file
        for item in list_of_files:
            temp_file.write(str(item) + "\n")

    # Return the path of the temporary file
    return temp_file.name


def classification(args):
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

    output_path: str = f"classification_outputs/{year}"

    effective_x_section: float = 1.0
    if not is_data:
        effective_x_section = xsection * filter_eff * k_factor * luminosity

    os.system(f"rm -rf classification_outputs/{year}/*_{process}_{year}.root")

    merge_cutflow_histograms(process, year, output_path, input_files)

    inputs = dump_list_to_temp_file(input_files)
    run_classification(
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
    print("\n\n📶 [ MUSiC Classification ] 📶\n")

    # parse arguments
    args = parse_args()

    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    # data workflow
    if args.all_data:
        classification_arguments = {}
        for sample in task_config:
            if (
                sample != "Lumi"
                and sample != "Global"
                and task_config[sample]["is_data"]
            ):
                year, era = get_year_era(sample)
                if f"Data_{year}_{era}" in classification_arguments:
                    classification_arguments[f"Data_{year}_{era}"][
                        "input_files"
                    ].extend(task_config[sample]["output_files"])
                else:
                    classification_arguments[f"Data_{year}_{era}"] = {
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

                if not (os.path.isdir(f"classification_outputs/{year}")):
                    os.system(f"mkdir -p classification_outputs/{year}")

        classification_arguments = list(classification_arguments.values())
        with Pool(min(args.jobs, len(classification_arguments))) as pool:
            # with Pool(1) as pool:
            list(
                tqdm(
                    pool.imap_unordered(classification, classification_arguments),
                    total=len(classification_arguments),
                    unit=" sample",
                )
            )

        exit(0)

    # MC workflow
    if args.all_mc:
        classification_arguments = []
        for sample in task_config:
            if (
                sample != "Lumi"
                and sample != "Global"
                and not (task_config[sample]["is_data"])
            ):
                for year in years:
                    if f"das_name_{year}" in task_config[sample].keys():
                        classification_arguments.append(
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
                    if not (os.path.isdir(f"classification_outputs/{year}")):
                        os.system(f"mkdir -p classification_outputs/{year}")

        with Pool(min(args.jobs, len(classification_arguments))) as pool:
            list(
                tqdm(
                    pool.imap_unordered(classification, classification_arguments),
                    total=len(classification_arguments),
                    unit="sample",
                )
            )

        exit(0)

    # MC and Data workflow
    if args.all:
        classification_arguments = {}
        for sample in task_config:
            if (
                sample != "Lumi"
                and sample != "Global"
                and task_config[sample]["is_data"]
            ):
                year, era = get_year_era(sample)
                if f"Data_{year}_{era}" in classification_arguments:
                    classification_arguments[f"Data_{year}_{era}"][
                        "input_files"
                    ].extend(task_config[sample]["output_files"])
                else:
                    classification_arguments[f"Data_{year}_{era}"] = {
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

                if not (os.path.isdir(f"classification_outputs/{year}")):
                    os.system(f"mkdir -p classification_outputs/{year}")

        classification_arguments_data = list(classification_arguments.values())

        classification_arguments = []
        for sample in task_config:
            if (
                sample != "Lumi"
                and sample != "Global"
                and not (task_config[sample]["is_data"])
            ):
                for year in years:
                    if f"das_name_{year}" in task_config[sample].keys():
                        classification_arguments.append(
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
                    if not (os.path.isdir(f"classification_outputs/{year}")):
                        os.system(f"mkdir -p classification_outputs/{year}")

        classification_arguments += classification_arguments_data
        with Pool(min(args.jobs, len(classification_arguments))) as pool:
            list(
                tqdm(
                    pool.imap_unordered(classification, classification_arguments),
                    total=len(classification_arguments),
                    unit="sample",
                )
            )


if __name__ == "__main__":
    main()
