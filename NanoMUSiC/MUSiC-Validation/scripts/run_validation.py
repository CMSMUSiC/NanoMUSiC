#!/usr/bin/env python3
from functools import partial

# from itertools import repeat
from multiprocessing import Pool
from tqdm import tqdm
import toml
import argparse
import shutil
import os
import subprocess
import glob


def load_samples(process, year):
    samples_files = glob.glob(
        f"{os.getenv('MUSIC_BASE')}/configs/xsections_and_lumi/samples/*.toml"
    )
    samples = {}
    for f in samples_files:
        samples = samples | toml.load(f)

    luminosity = toml.load(
        f"{os.getenv('MUSIC_BASE')}/configs/xsections_and_lumi/luminosities.toml"
    )["Lumi"][year]
    return luminosity, samples[process]


parser = argparse.ArgumentParser()
parser.add_argument("config", help="task configuration file")
parser.add_argument("--veto", help="path to run_number/event_number veto maps")
parser.add_argument(
    "--merge", help="will merge validation results", action="store_true"
)
parser.add_argument(
    "--debug", help="print debugging info", action="store_true"
)

args = parser.parse_args()

# parse config file
task_config_file = args.config
task_config = toml.load(task_config_file)
is_data = task_config["is_data"]
x_section_file = task_config["x_section_file"]
process = task_config["process"]
year = task_config["year"]
input_files = task_config["input_files"]

output_path = f"validation_outputs/{process}_{year}"


def merge_cutflow_histograms(output_path, input_files):
    merge_result = subprocess.run(
        ["hadd", "-f", "-T", f"{output_path}/cutflow.root", *input_files],
        capture_output=True,
    )
    if args.debug:
        print(merge_result.stdout.decode("utf-8"))
    if merge_result.returncode != 0:
        error = merge_result.stderr.decode("utf-8")
        raise RuntimeError(f"ERROR: could not merge cutflow files.\n{error}")


def run_validation(input_file, config_file, output_path, effective_x_section):
    # print(f"Exectuting: {input_file, config_file, output_path}")
    validation_result = subprocess.run(
        [
            "validation",
            "-c",
            config_file,
            "-o",
            output_path,
            "-x",
            str(effective_x_section),
            "-i",
            input_file,
        ],
        capture_output=True,
    )
    if args.debug:
        print(validation_result.stdout.decode("utf-8"))
    if validation_result.returncode != 0:
        error = validation_result.stderr.decode("utf-8")
        raise RuntimeError(f"ERROR: could process validation.\n{error}\n{input_file}")
    return True


def main():
    print("\n\n📶 [ MUSiC Validation ] 📶\n")

    print(f"[ MUSiC Validation ] Process: {process} - Year: {year}\n")

    print("[ MUSiC Validation ] Loading samples ...\n")
    effective_x_section = 1.0
    if not is_data:
        luminosity, sample_config = load_samples(process, year)
        effective_x_section = (
            sample_config["x_section"]
            * sample_config["filter_efficiency"]
            * sample_config["k_factor"]
        ) * luminosity

    print("[ MUSiC Validation ] Preparing output directory ...\n")
    shutil.rmtree(output_path, ignore_errors=True)
    os.system(f"rm -rf validation_outputs/{process}*.root")
    os.makedirs(output_path)
    shutil.copy(task_config_file, f"{output_path}/validation_config.toml")

    print("[ MUSiC Validation ] Merging cutflow histograms ...\n")
    merge_cutflow_histograms(output_path, input_files)

    print("[ MUSiC Validation ] Launching processes ...\n\n")
    with Pool(100) as pool:
        list(
            tqdm(
                pool.imap_unordered(
                    partial(
                        run_validation,
                        config_file=task_config_file,
                        output_path=output_path,
                        effective_x_section=effective_x_section,
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
                f"validation_outputs/{process}_{output}.root",
                *glob.glob(f"{output_path}/{output}_[0-9]*.root"),
            ],
            capture_output=True,
        )
        if merge_result.returncode != 0:
            error = merge_result.stderr.decode("utf-8")
            raise RuntimeError(f"ERROR: could not merge validation files.\n{error}")

        # cleanning ...
        cleanning_result = subprocess.run(["rm", "-rf", *glob.glob(f"{output_path}/{output}_*.root")], capture_output=True)
        if cleanning_result.returncode != 0:
            error = cleanning_result.stderr.decode("utf-8")
            raise RuntimeError(f"ERROR: could not clear output path.\n{error}")


if __name__ == "__main__":
    main()