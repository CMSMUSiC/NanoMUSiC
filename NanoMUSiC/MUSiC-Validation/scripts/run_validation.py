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
    executable: str,
    processed_events_file: str,
    input_file: str,
    debug: bool = False,
) -> bool:
    # default is MC
    cmd_str: str = f"{executable} --process {process_name} --year {year} --output {output_path} --xsection {str(effective_x_section)} --input {input_file}"
    if is_data:
        cmd_str: str = f"{executable} --process {process_name} --year {year} --is_data --processed-events {processed_events_file} --output {output_path} --xsection {str(effective_x_section)} --input {input_file}"

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


def validation(
    process,
    year,
    luminosity,
    is_data,
    xsection,
    filter_eff,
    k_factor,
    input_files,
    jobs,
    executable,
):
    # make a new processed events
    processed_events_file = make_processed_events()

    output_path: str = f"validation_outputs/{year}/{process}"

    print("[ MUSiC Validation ] Loading samples ...\n")
    effective_x_section: float = 1.0
    if not is_data:
        effective_x_section = xsection * filter_eff * k_factor * luminosity

    print("[ MUSiC Validation ] Preparing output directory ...\n")
    shutil.rmtree(output_path, ignore_errors=True)
    os.system(f"rm -rf validation_outputs/{year}/{process}*.root")
    os.makedirs(output_path)

    print("[ MUSiC Validation ] Merging cutflow histograms ...\n")
    merge_cutflow_histograms(output_path, input_files)

    print("[ MUSiC Validation ] Launching processes ...\n\n")
    with Pool(min(jobs, len(input_files))) as pool:
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
                        executable,
                        processed_events_file,
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


years = ["2016APV", "2016", "2017", "2018"]


def main():
    print("\n\n📶 [ MUSiC Validation ] 📶\n")

    # parse arguments
    args = parse_args()

    # data workflow
    if args.all_data:
        # validation(args)
        exit(0)

    if args.all_mc:
        task_config_file: str = args.config
        task_config: dict[str, Any] = toml.load(task_config_file)
        for idx_sample, sample in enumerate(task_config):
            if (
                sample != "Lumi"
                and sample != "Global"
                and not (task_config[sample]["is_data"])
            ):
                print(
                    f"[ MUSiC Validation ] Starting validation of {sample} ... [{idx_sample+1}/{len(task_config)-2}]"
                )
                for year in years:
                    if f"das_name_{year}" in task_config[sample].keys():
                        print(f"[ MUSiC Validation - {sample} ] Year: {year}")

                        validation(
                            process=sample,
                            year=year,
                            luminosity=task_config["Lumi"][year],
                            is_data=task_config[sample]["is_data"],
                            xsection=task_config[sample]["XSec"],
                            filter_eff=task_config[sample]["FilterEff"],
                            k_factor=task_config[sample]["kFactor"],
                            input_files=list(
                                filter(
                                    lambda file: f"{year}_date" in file,
                                    task_config[sample]["output_files"],
                                )
                            ),
                            jobs=args.jobs,
                            executable=args.executable,
                        )
        exit(0)

    # validation(args)


if __name__ == "__main__":
    main()
