#!/usr/bin/env python3

import multiprocessing
from multiprocessing import Pool
from typing import Any
from tqdm import tqdm
import toml  # type: ignore
import argparse
import os
import subprocess
import shlex
import pprint

from local_condor import submit_condor_task

years = ["2016APV", "2016", "2017", "2018"]

nproc = multiprocessing.cpu_count()


def split_list(lst, max_size):
    sublists = []
    for i in range(0, len(lst), max_size):
        sublist = lst[i : i + max_size]
        sublists.append(sublist)
    return sublists


def dump_list_to_file(lst, file_path):
    with open(file_path, "w") as file:
        for item in lst:
            file.write(item + "\n")


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        required=True,
        help='Task configuration (TOML) file, produced by "analysis_config_builder.py"',
    )
    parser.add_argument("-s", "--sample", help="Sample to be processed.", default="")
    parser.add_argument("-y", "--year", help="Year to be processed.", default="")
    parser.add_argument(
        "--all-data",
        help='Starts validation for all Data samples. Incompatible with "--sample" and "--all_mc".',
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--all-mc",
        help='Starts validation for all MC samples. Incompatible with "--sample" and "--all-data".',
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--all",
        help='Starts validation for all MC and Data samples. Incompatible with "--sample", "--all-data" and  "--all-mc".',
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--condor",
        help="Run validation on on the Linux Cluster. The merging of the outputs has to be done separeted",
        action="store_true",
        default=False,
    )

    parser.add_argument(
        "-j",
        "--jobs",
        help="Multiprocessing pool size.",
        type=int,
        default=min(60, nproc),
    )

    parser.add_argument(
        "-e", "--executable", help="Validation excutable.", default="validation"
    )
    parser.add_argument("--debug", help="print debugging info", action="store_true")

    parser.add_argument(
        "-fl",
        "--file_limit",
        help="Limit the number of files to be processed, per sample.",
        type=int,
        default=-1,
    )

    parser.add_argument(
        "-o",
        "--output",
        help="Output base path.",
        type=str,
        default=".",
    )

    parser.add_argument(
        "-sdt",
        "--split-data",
        help="Limit the number of Data files to be processed, per job.",
        type=int,
        default=1,
    )

    parser.add_argument(
        "-smc",
        "--split-mc",
        help="Limit the number of MC files to be processed, per job.",
        type=int,
        default=3,
    )

    parser.add_argument(
        "--harvest-only",
        help="Will only harvest results, after condor execution is complete.",
        action="store_true",
        default=False,
    )

    parser.add_argument(
        "--clear-buffers",
        help="Will only clear buffer areas.",
        action="store_true",
        default=False,
    )

    args = parser.parse_args()

    # quality control
    # if (args.all_data and (args.sample or args.all_mc)) or (
    #     args.all_mc and (args.sample or args.all_data)
    # ):
    #     raise Exception(
    #         'ERROR: Could not satrt validation. "--all_data" is incompatible with "--all_mc" and "--sample".'
    #     )

    if args.sample and not (args.year):
        raise Exception(
            'ERROR: Could not start validation. When "--sample" is set, "--year" is required.'
        )
    return args


def merge_cutflow_histograms(args):
    process, year, output_path, input_files, debug = args
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
    xsection: float,
    filter_eff: float,
    k_factor: float,
    luminosity: float,
    processOrder: str,
    processGroup: str,
    executable: str,
    # shift: str,
    file_index: str,
    input_file: str,
    debug: bool,
) -> bool:
    # debug: bool = False

    # default is MC
    cmd_str: str = f"{executable} --process {process_name} --year {year} --output {output_path} --file_index {file_index} --xsection {str(xsection)} --filter_eff {str(filter_eff)} --k_factor {str(k_factor)} --luminosity {str(luminosity)} --xs_order {processOrder} --process_group {processGroup} --input {input_file}"
    if is_data:
        cmd_str: str = f"{executable} --process {process_name} --year {year} --is_data --output {output_path} --xsection {str(xsection)} --filter_eff {str(filter_eff)} --k_factor {str(k_factor)} --luminosity {str(luminosity)} --xs_order {processOrder} --process_group {processGroup} --input {input_file}"

    if debug:
        cmd_str = cmd_str + " --debug"
        print(f"\nExecuting: {cmd_str}")

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


def validation(args):
    (
        process,
        year,
        luminosity,
        is_data,
        xsection,
        filter_eff,
        k_factor,
        processOrder,
        processGroup,
        file_index,
        input_files,
        executable,
        # shift,
        output_base,
        debug,
    ) = list(args.values())

    output_path: str = (
        f"{output_base}/validation_outputs/{year}/{process}/buffer/buffer_{file_index}"
    )

    # print("[ MUSiC Validation ] Loading samples ...\n")
    # print("[ MUSiC Validation ] Preparing output directory ...\n")
    os.system(
        f"rm -rf {output_base}/validation_outputs/{year}/{process}/buffer/buffer_{file_index}/*_{process}_{year}_{file_index}.root"
    )

    dump_list_to_file(input_files, f"{output_path}/inputs.txt")

    # print("[ MUSiC Validation ] Starting validation ...\n")
    run_validation(
        process,
        year,
        is_data,
        output_path,
        xsection,
        filter_eff,
        k_factor,
        luminosity,
        processOrder,
        processGroup,
        executable,
        # shift,
        file_index,
        os.path.abspath(f"{output_path}/inputs.txt"),
        debug,
    )


def validation_condor(args):
    (
        process,
        year,
        luminosity,
        is_data,
        xsection,
        filter_eff,
        k_factor,
        processOrder,
        processGroup,
        file_index,
        input_files,
        executable,
        # shift,
        output_base,
        debug,
    ) = list(args.values())

    output_path: str = (
        f"{output_base}/validation_outputs/{year}/{process}/buffer/buffer_{file_index}"
    )

    dump_list_to_file(input_files, f"{output_path}/inputs.txt")

    submit_condor_task(
        process,
        year,
        is_data,
        output_path,
        xsection,
        filter_eff,
        k_factor,
        luminosity,
        processOrder,
        processGroup,
        os.path.abspath(f"{output_path}/inputs.txt"),
        output_path,
        debug,
    )


def get_file_paths(root_dir, extension=".root"):
    file_paths = []
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.endswith(extension):
                file_paths.append(os.path.join(dirpath, filename))
    return file_paths


def validation_merger(args):
    process, year, output_base, debug = args

    output_file_path: str = (
        f"{output_base}/validation_outputs/{year}/{process}/{process}_{year}.root"
    )

    validation_merge_result = subprocess.run(
        [
            "hadd",
            "-f",
            output_file_path,
            *get_file_paths(
                f"{output_base}/validation_outputs/{year}/{process}/buffer"
            ),
        ],
        capture_output=True,
    )
    if debug:
        print(validation_merge_result.stdout.decode("utf-8"))

    if validation_merge_result.returncode != 0:
        error = validation_merge_result.stderr.decode("utf-8")
        raise RuntimeError(f"ERROR: could not merge cutflow files.\n{error}")


def process_filter(args, is_data: bool, process: str, year: str) -> bool:
    if (
        args.sample != ""
        and args.year != ""
        and args.sample == process
        and args.year == year
    ):
        return True
    if args.sample != "" and args.year == "" and args.sample == process:
        return True

    if args.all_data and is_data and args.year != "" and args.year == year:
        return True

    if args.all_data and args.year == "" and is_data:
        return True

    if args.all_mc and not is_data and args.year != "" and args.year == year:
        return True

    if args.all_mc and args.year == "" and not is_data:
        return True

    if args.all and args.year != "" and args.year == year:
        return True

    if args.all and args.year == "":
        return True

    return False


def files_to_process(file_limit, year, output_files):
    if file_limit < 0:
        return list(
            filter(
                lambda file: f"_{year}/" in file,
                output_files,
            )
        )
    return list(
        filter(
            lambda file: f"_{year}/" in file,
            output_files,
        )
    )[:file_limit]


def main():
    print("\n\n📶 [ MUSiC Validation ] 📶\n")

    # parse arguments
    args = parse_args()

    if args.debug:
        print("Will run in DEBUG mode ...")

    # cleanning job
    if args.clear_buffers:
        os.system(
            f"rm -rf {args.output}/validation_outputs/*/*/buffer/buffer_*/*.root > /dev/null 2>&1"
        )
        exit(0)

    # load analysis config file
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    if args.sample:
        if not (args.sample in task_config.keys()):
            raise Exception(
                f"ERROR: Could not start validation. Requested sample ({args.sample}) not found in analysis config.\n Available samples are: {list(task_config.keys())}"
            )

    print("Building jobs ...")
    merge_cutflow_arguments = []
    validation_arguments = []
    merge_arguments = []

    for sample in tqdm(task_config, unit=" tasks"):
        if sample != "Lumi" and sample != "Global":
            for year in years:
                if f"das_name_{year}" in task_config[sample].keys():
                    if process_filter(
                        args, task_config[sample]["is_data"], sample, year
                    ):
                        if not args.harvest_only:
                            if not (
                                os.path.isdir(
                                    f"{args.output}/validation_outputs/{year}/{sample}"
                                )
                            ):
                                os.system(
                                    f"mkdir -p {args.output}/validation_outputs/{year}/{sample}"
                                )
                            merge_cutflow_arguments.append(
                                (
                                    sample,
                                    year,
                                    f"{args.output}/validation_outputs/{year}/{sample}",
                                    files_to_process(
                                        -1,
                                        year,
                                        task_config[sample]["output_files"],
                                    ),
                                    args.debug,
                                )
                            )

                            # get splitting for Data/MC
                            splitting = args.split_mc
                            if task_config[sample]["is_data"]:
                                splitting = args.split_data

                            for idx, f in enumerate(
                                split_list(
                                    files_to_process(
                                        args.file_limit,
                                        year,
                                        task_config[sample]["output_files"],
                                    ),
                                    splitting,
                                )
                            ):
                                validation_arguments.append(
                                    {
                                        "process": sample,
                                        "year": year,
                                        "luminosity": task_config["Lumi"][year],
                                        "is_data": task_config[sample]["is_data"],
                                        "xsection": 1.0,
                                        "filter_eff": 1.0,
                                        "k_factor": 1.0,
                                        "processOrder": "DUMMY",
                                        "processGroup": "Data",
                                        "file_index": idx,
                                        "input_files": f,
                                        "executable": args.executable,
                                        # "shift": "Nominal",
                                        "output_base": args.output,
                                        "debug": args.debug,
                                    }
                                )
                                if not (task_config[sample]["is_data"]):
                                    validation_arguments[-1]["xsection"] = task_config[
                                        sample
                                    ]["XSec"]
                                    validation_arguments[-1][
                                        "filter_eff"
                                    ] = task_config[sample]["FilterEff"]
                                    validation_arguments[-1]["k_factor"] = task_config[
                                        sample
                                    ]["kFactor"]
                                    validation_arguments[-1][
                                        "processOrder"
                                    ] = task_config[sample]["XSecOrder"]
                                    validation_arguments[-1][
                                        "processGroup"
                                    ] = task_config[sample]["ProcessGroup"]

                                if not (
                                    os.path.isdir(
                                        f"{args.output}/validation_outputs/{year}/{sample}/buffer/buffer_{idx}"
                                    )
                                ):
                                    os.system(
                                        f"mkdir -p {args.output}/validation_outputs/{year}/{sample}/buffer/buffer_{idx}"
                                    )

                        # prepare merge jobs
                        merge_arguments.append((sample, year, args.output, args.debug))

    if not args.harvest_only:
        if len(validation_arguments) == 0:
            raise Exception(
                "ERROR: Could not start validation with the provided arguments. Not enough files to validate."
            )

        # merge cutflow histograms
        print("\nMerging cutflow histograms ...")
        with Pool(min(args.jobs, len(merge_cutflow_arguments))) as pool:
            list(
                tqdm(
                    pool.imap_unordered(
                        merge_cutflow_histograms, merge_cutflow_arguments
                    ),
                    total=len(merge_cutflow_arguments),
                    unit=" samples",
                )
            )

        if args.condor:
            # submit validation
            print("\nSubmiting validation ...")
            # with Pool(min(args.jobs, 10, len(validation_arguments))) as pool:
            with Pool(min(args.jobs, len(validation_arguments))) as pool:
                list(
                    tqdm(
                        pool.imap_unordered(
                            validation_condor,
                            sorted(
                                validation_arguments,
                                key=lambda x: x["is_data"],
                                reverse=True,
                            ),
                        ),
                        total=len(validation_arguments),
                        unit=" jobs",
                    )
                )
        else:
            # run validation
            print("\nProcessing validation ...")
            with Pool(min(args.jobs, len(validation_arguments))) as pool:
                list(
                    tqdm(
                        pool.imap_unordered(
                            validation,
                            sorted(
                                validation_arguments,
                                key=lambda x: x["is_data"],
                                reverse=True,
                            ),
                        ),
                        total=len(validation_arguments),
                        unit=" files",
                    )
                )

    if not args.condor:
        # merge outputs
        print("\nMerging outputs ...")
        with Pool(min(args.jobs, len(merge_arguments))) as pool:
            list(
                tqdm(
                    pool.imap_unordered(validation_merger, merge_arguments),
                    total=len(merge_arguments),
                    unit=" samples",
                )
            )


if __name__ == "__main__":
    main()
