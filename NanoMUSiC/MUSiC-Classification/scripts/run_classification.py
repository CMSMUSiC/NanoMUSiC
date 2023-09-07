#!/usr/bin/env python3

import multiprocessing
from multiprocessing import Pool
from typing import Any
from tqdm import tqdm
import toml  # type: ignore
import argparse
import os
import sys
import glob
import subprocess
import shlex
from pprint import pprint
import random
import ROOT

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


def get_splitting_factor(
    sample, sample_config, default_splitting_data=10, default_splitting_mc=4
):
    if sample_config["is_data"]:
        splitting = default_splitting_data

        # if sample.startswith("EGamma"):
        #     splitting = min(3, default_splitting_data)
        # if sample.startswith("SingleMuon"):
        #     splitting = min(3, default_splitting_data)

    else:
        # get splitting for Data/MC
        splitting = default_splitting_mc

        if "QCD" in sample_config["ProcessGroup"]:
            return 20

        if "NuNu" in sample_config["ProcessGroup"]:
            return 10

        # TTZToLL samples take the longest to run
        if sample.startswith("TTZToLL"):
            return 1

    return splitting


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
        help='Starts classification for all Data samples. Incompatible with "--sample" and "--all_mc".',
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--all-mc",
        help='Starts classification for all MC samples. Incompatible with "--sample" and "--all-data".',
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--all",
        help='Starts classification for all MC and Data samples. Incompatible with "--sample", "--all-data" and  "--all-mc".',
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--condor",
        help="Run classification on on the Linux Cluster. The merging of the outputs has to be done separeted",
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
        "-e", "--executable", help="Classification excutable.", default="classification"
    )
    parser.add_argument("--debug", help="print debugging info", action="store_true")

    parser.add_argument(
        "--file-limit",
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
        "-u", "--username", help="dCache username.", type=str, required=True
    )

    parser.add_argument(
        "--harvest",
        help="Will only harvest results, after condor execution is complete.",
        action="store_true",
        default=False,
    )

    parser.add_argument(
        "--merge",
        help="Will merge all outputs.",
        action="store_true",
        default=False,
    )

    parser.add_argument(
        "--clear",
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
    #         'ERROR: Could not satrt classification. "--all_data" is incompatible with "--all_mc" and "--sample".'
    #     )

    if args.sample and not (args.year):
        raise Exception(
            'ERROR: Could not start classification. When "--sample" is set, "--year" is required.'
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


def run_classification(
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
    buffer_index: str,
    input_file: str,
    debug: bool,
) -> bool:
    # debug: bool = False

    # default is MC
    cmd_str: str = f"{executable} --process {process_name} --year {year} --output {output_path} --buffer_index {buffer_index} --xsection {str(xsection)} --filter_eff {str(filter_eff)} --k_factor {str(k_factor)} --luminosity {str(luminosity)} --xs_order {processOrder} --process_group {processGroup} --input {input_file}"
    if is_data:
        cmd_str: str = f"{executable} --process {process_name} --year {year} --is_data --output {output_path} --xsection {str(xsection)} --filter_eff {str(filter_eff)} --k_factor {str(k_factor)} --luminosity {str(luminosity)} --xs_order {processOrder} --process_group {processGroup} --input {input_file}"

    if debug:
        cmd_str = cmd_str + " --debug"
        print(f"\nExecuting: {cmd_str}")

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


def classification(args):
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
        buffer_index,
        input_files,
        executable,
        # shift,
        output_base,
        debug,
    ) = list(args.values())

    output_path: str = f"{output_base}/classification_outputs/{year}/{process}/buffer/buffer_{buffer_index}"

    # print("[ MUSiC Classification ] Loading samples ...\n")
    # print("[ MUSiC Classification ] Preparing output directory ...\n")
    os.system(
        f"rm -rf {output_base}/classification_outputs/{year}/{process}/buffer/buffer_{buffer_index}/*_{process}_{year}_{buffer_index}.root"
    )

    dump_list_to_file(input_files, f"{output_path}/inputs.txt")

    # print("[ MUSiC Classification ] Starting classification ...\n")
    run_classification(
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
        buffer_index,
        os.path.abspath(f"{output_path}/inputs.txt"),
        debug,
    )


def classification_condor(args):
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
        buffer_index,
        input_files,
        executable,
        # shift,
        output_base,
        username,
        debug,
    ) = list(args.values())

    output_path: str = f"{output_base}/classification_outputs/{year}/{process}/buffer/buffer_{buffer_index}"

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
        buffer_index,
        username,
        debug,
    )


def get_file_paths(root_dir, extension=".root"):
    file_paths = []
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.endswith(extension):
                file_paths.append(os.path.join(dirpath, filename))
    return file_paths


def get_files_in_buffer_area(
    process: str,
    year: str,
    process_group: str,
    xsec_order: str,
    username: str,
    debug: bool,
):
    srmls_merge_result = subprocess.run(
        shlex.split(
            f"srmls srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2\\?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/{username}/classification_buffer/"
        ),
        capture_output=True,
    )
    if debug:
        print(srmls_merge_result.stdout.decode("utf-8"))

    if srmls_merge_result.returncode != 0:
        error = srmls_merge_result.stderr.decode("utf-8")
        raise RuntimeError(f"ERROR: could not merge output files, per sample.\n{error}")

    buffer_files = []
    for line in srmls_merge_result.stdout.decode("utf-8").split("\n"):
        if (
            "ec_classes" in line
            and ".root" in line
            and f"_{process}_" in line
            and f"_{year}_" in line
            and f"_{process_group}_" in line
            and f"_{xsec_order}_" in line
        ):
            buffer_files.append(
                f"dcap://grid-dcap-extern.physik.rwth-aachen.de/pnfs/physik.rwth-aachen.de/cms/store/user/{username}/classification_buffer/ec_classes"
                + line.split("ec_classes")[1].split(".root")[0]
                + ".root"
            )

    return buffer_files


def classification_merger_per_sample(args):
    process, year, process_group, xsec_order, output_base, username, debug = args

    output_file_path: str = (
        f"{output_base}/classification_outputs/{year}/{process}/{process}_{year}.root"
    )

    classification_merge_result = subprocess.run(
        [
            "hadd",
            "-f",
            output_file_path,
            *get_files_in_buffer_area(
                process, year, process_group, xsec_order, username, debug
            ),
        ],
        capture_output=True,
    )
    if debug:
        print(classification_merge_result.stdout.decode("utf-8"))

    if classification_merge_result.returncode != 0:
        error = classification_merge_result.stderr.decode("utf-8")
        raise RuntimeError(f"ERROR: could not merge output files, per sample.\n{error}")


def classification_final_merger_imp(year):
    files_to_merge = filter(
        lambda f: "cutflow" not in f, glob.glob("classification_outputs/2016*/*/*.root")
    )
    classification_final_merger_result = subprocess.run(
        shlex.split(
            f'hadd -f -T -j 25 ./classification_outputs/classification_outputs_{year.replace("*", "")}.root {" ".join(files_to_merge)}'
        ),
        capture_output=True,
    )

    if classification_final_merger_result.returncode != 0:
        error = classification_final_merger_result.stderr.decode("utf-8")
        raise RuntimeError(f"ERROR: Could not merge output files.\n{error}")


def classification_merger(output_base):
    year_to_merge = ["2016APV", "2016", "2017", "2018"]
    with Pool(len(year_to_merge)) as pool:
        list(
            tqdm(
                pool.imap_unordered(classification_final_merger_imp, year_to_merge),
                total=len(year_to_merge),
                unit=" years",
            )
        )


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
    print("\n\n📶 [ MUSiC Classification ] 📶\n")

    # parse arguments
    args = parse_args()

    if args.debug:
        print("Will run in DEBUG mode ...")

    if args.condor and (args.harvest or args.merge):
        print("ERROR: Option --condor is not compatible with --harvest or --merge.")
        sys.exit(-1)

    if args.harvest and args.merge:
        print("ERROR: Option --harvest is not compatible with --merge.")
        sys.exit(-1)

    # cleanning job
    if args.clear:
        os.system(
            f"rm -rf {args.output}/classification_outputs/*/*/buffer/buffer_*/*.root > /dev/null 2>&1"
        )
        sys.exit(0)

    if args.merge:
        # merge final outputs
        print("\nMerging outputs ...")
        classification_merger(args.output)
        sys.exit(0)

    # load analysis config file
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    if args.sample:
        if not (args.sample in task_config.keys()):
            raise Exception(
                f"ERROR: Could not start classification. Requested sample ({args.sample}) not found in analysis config.\n Available samples are: {list(task_config.keys())}"
            )

    print("Building tasks ...")
    merge_cutflow_arguments = []
    classification_arguments = []
    merge_per_sample_arguments = []
    merge_arguments = []

    for sample in tqdm(task_config, unit=" tasks"):
        if sample != "Lumi" and sample != "Global":
            for year in years:
                if f"das_name_{year}" in task_config[sample].keys():
                    if process_filter(
                        args, task_config[sample]["is_data"], sample, year
                    ):
                        if not args.harvest:
                            if not (
                                os.path.isdir(
                                    f"{args.output}/classification_outputs/{year}/{sample}"
                                )
                            ):
                                os.system(
                                    f"mkdir -p {args.output}/classification_outputs/{year}/{sample}"
                                )
                            merge_cutflow_arguments.append(
                                (
                                    sample,
                                    year,
                                    f"{args.output}/classification_outputs/{year}/{sample}",
                                    files_to_process(
                                        -1,
                                        year,
                                        task_config[sample]["output_files"],
                                    ),
                                    args.debug,
                                )
                            )

                            # get splitting for Data/MC
                            splitting = get_splitting_factor(
                                sample,
                                task_config[sample],
                            )

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
                                classification_arguments.append(
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
                                        "buffer_index": idx,
                                        "input_files": f,
                                        "executable": args.executable,
                                        "output_base": args.output,
                                        "username": args.username,
                                        "debug": args.debug,
                                    }
                                )
                                if not (task_config[sample]["is_data"]):
                                    classification_arguments[-1][
                                        "xsection"
                                    ] = task_config[sample]["XSec"]
                                    classification_arguments[-1][
                                        "filter_eff"
                                    ] = task_config[sample]["FilterEff"]
                                    classification_arguments[-1][
                                        "k_factor"
                                    ] = task_config[sample]["kFactor"]
                                    classification_arguments[-1][
                                        "processOrder"
                                    ] = task_config[sample]["XSecOrder"]
                                    classification_arguments[-1][
                                        "processGroup"
                                    ] = task_config[sample]["ProcessGroup"]

                                if not (
                                    os.path.isdir(
                                        f"{args.output}/classification_outputs/{year}/{sample}/buffer/buffer_{idx}"
                                    )
                                ):
                                    os.system(
                                        f"mkdir -p {args.output}/classification_outputs/{year}/{sample}/buffer/buffer_{idx}"
                                    )

                        # prepare merge jobs
                        if task_config[sample]["is_data"]:
                            merge_per_sample_arguments.append(
                                (
                                    sample,
                                    year,
                                    "Data",
                                    "DUMMY",
                                    args.output,
                                    args.username,
                                    args.debug,
                                )
                            )
                        else:
                            merge_per_sample_arguments.append(
                                (
                                    sample,
                                    year,
                                    task_config[sample]["ProcessGroup"],
                                    task_config[sample]["XSecOrder"],
                                    args.output,
                                    args.username,
                                    args.debug,
                                )
                            )

                        if args.merge:
                            try:
                                is_signal = task_config[sample]["is_signal"]
                            except:
                                is_signal = False
                            merge_arguments.append(
                                (
                                    sample,
                                    year,
                                    task_config[sample]["is_data"],
                                    is_signal,
                                )
                            )

    print(f"\n--> Will submit {len(classification_arguments)} jobs ...")

    if not args.harvest and not args.merge:
        if len(classification_arguments) == 0:
            raise Exception(
                "ERROR: Could not start classification with the provided arguments. Not enough files to validate."
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
            # submit classification
            print("\nSubmiting classification ...")
            with Pool(min(args.jobs, len(classification_arguments))) as pool:
                list(
                    tqdm(
                        pool.imap_unordered(
                            classification_condor,
                            random.sample(
                                classification_arguments, len(classification_arguments)
                            ),
                        ),
                        total=len(classification_arguments),
                        unit=" jobs",
                    )
                )
        else:
            # run classification
            print("\nProcessing classification ...")
            with Pool(min(args.jobs, len(classification_arguments))) as pool:
                list(
                    tqdm(
                        pool.imap_unordered(
                            classification,
                            random.sample(
                                classification_arguments, len(classification_arguments)
                            ),
                        ),
                        total=len(classification_arguments),
                        unit=" files",
                    )
                )

    if not args.condor and args.harvest:
        # merge outputs per sample
        print("\nMerging outputs per sample ...")
        with Pool(min([args.jobs, len(merge_per_sample_arguments), 30])) as pool:
            list(
                tqdm(
                    pool.imap_unordered(
                        classification_merger_per_sample, merge_per_sample_arguments
                    ),
                    total=len(merge_per_sample_arguments),
                    unit=" samples",
                )
            )


if __name__ == "__main__":
    main()
