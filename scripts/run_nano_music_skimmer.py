#!/usr/bin/env python3

import shlex, subprocess
import argparse
import tomli
from pprint import pformat
from helpers import *
import tempfile


def parse_arguments():
    parser = argparse.ArgumentParser(description="Test nano_music in a given sample.")
    parser.add_argument(
        "--config",
        "-c",
        help="Task config file",
    )
    parser.add_argument(
        "--sample",
        "-s",
        help="MUSiC sample name",
    )

    parser.add_argument(
        "--year",
        "-y",
        choices=["2016", "2016APV", "2017", "2018"],
        help="Year to be processed",
        required=True,
    )

    parser.add_argument(
        "--executable",
        "-e",
        help="Path to nano_music_skimmer executable (i.e. from $PATH or local build directory).",
        default="nano_music_skimmer",
    )

    return parser.parse_args()


def get_file_from_das(das_name):
    das_query = subprocess.run(
        shlex.split(f'dasgoclient -query="file dataset={das_name}"'),
        capture_output=True,
    )
    if das_query.returncode != 0:
        raise RuntimeError(
            f"ERROR: Could not find files in DAS.\n {das_query.stderr.decode('utf-8')}"
        )

    query_result = das_query.stdout.decode("utf-8")
    if query_result:
        return f"root://cms-xrd-global.cern.ch//{list(query_result.splitlines())[0]}"

    raise RuntimeError(f"ERROR: DAS query returned and empty list of files (das_name).")


def get_sample_and_file(config_file, sample_name, year):
    with open(config_file, mode="rb") as f:
        samples = tomli.load(f)
    if sample_name in samples.keys():
        sample = samples[sample_name]
    else:
        raise RuntimeError(
            f"ERROR: Sample not found ({sample_name}).\nAvailable samples are: {pformat(list(samples.keys()))}"
        )

    try:
        if len(sample[f"das_name_{year}"]):
            das_name = sample[f"das_name_{year}"][0]
            sample["input_files"] = get_file_from_das(das_name)
            sample["das_name"] = das_name
            sample["name"] = sample_name
            return sample
    except:
        raise RuntimeError(
            f"ERROR: DAS sample name not found (Sample: {sample_name} - Year: {year})."
        )


def check_voms():
    ret_code = subprocess.run(
        shlex.split("voms-proxy-info"), capture_output=True
    ).returncode

    if ret_code == 0:
        return True
    return False


def get_era(sample):
    if not sample["is_data"]:
        return "DUMMY"

    return (
        sample["name"]
        .replace("-HIPM", "")
        .replace("_HIPM", "")
        .replace("-ver1", "")
        .replace("-ver2", "")
        .split("_")[-1]
    )


def make_task_config(config_file, sample_name, year):
    task_config = {
        "output": "",
        "is_data": False,
        "year": "",
        "era": "",
        "process": "",
        # "generator_filter_key": "",  # only if defined
        "dataset": "",
        "is_crab_job": False,
        "input_files": [],
    }

    sample = get_sample_and_file(config_file, sample_name, year)
    if sample:
        task_config["output"] = f"test_{sample_name}_{year}"
        task_config["is_data"] = sample["is_data"]
        task_config["year"] = year
        task_config["era"] = get_era(sample)
        task_config["process"] = sample_name
        if "generator_filter_key" in sample.keys():
            task_config["generator_filter_key"] = sample["generator_filter_key"]
        task_config["dataset"] = sample["das_name"]
        task_config["input_files"] = [sample["input_files"]]
        return to_toml_dumps(task_config)


def main():
    args = parse_arguments()

    if not (check_voms()):
        raise RuntimeError("ERROR: Could not find valid VOMS proxy.")

    task_config = make_task_config(args.config, args.sample, args.year)
    if task_config:
        with tempfile.NamedTemporaryFile(mode="w", delete=False) as f:
            f.write(task_config)
        str_command = f"{args.executable} --run-config {f.name}"
        print(f"Will exectute: {str_command}")
        music_process = subprocess.run(shlex.split(str_command), stderr=subprocess.PIPE)
        print(music_process.stderr)
        if music_process.returncode == 0:
            subprocess.run(shlex.split(f"rm -rf {f.name}"))
        else:
            print(f"Command: {str_command}")
            raise Exception("ERROR: Could not validate sample.")

    else:
        raise RuntimeError("ERROR: Task configuration not built.")


if __name__ == "__main__":
    main()
