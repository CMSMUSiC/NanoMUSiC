#!/usr/bin/env python3

import multiprocessing
from multiprocessing import Pool
from typing import Any
from tqdm import tqdm
import toml  # type: ignore
import argparse
import os
import sys
import subprocess
import shlex
import random
from functools import partial
import itertools
import ROOT

from local_condor import submit_condor_task


from multiprocessing import Pool
import time


years = ["2016APV", "2016", "2017", "2018"]

nproc = multiprocessing.cpu_count()


def number_of_events(file_path):
    root_file = ROOT.TFile.Open(file_path)
    return root_file.Get("nano_music").GetEntries()


def chunk_limits(lower_limit, upper_limit, chunk_size):
    chunk_limits = []

    lower = lower_limit
    while lower < upper_limit:
        upper = min(lower + chunk_size - 1, upper_limit)
        chunk_limits.append((lower, upper))
        lower = upper + 1

    return chunk_limits


def split_list_of_blocks(iterable, n):
    "Batch data into lists of length n. The last batch may be shorter."
    # batched('ABCDEFG', 3) --> ABC DEF G
    if n < 1:
        raise ValueError("n must be at least one")
    it = iter(iterable)
    while batch := list(itertools.islice(it, n)):
        yield batch


def get_splitting_imp(f, splitting_factor):
    return [
        [f, lower, upper]
        for lower, upper in chunk_limits(0, number_of_events(f), splitting_factor)
    ]


def get_spliting(files, sample, sample_config):
    # get splitting for Data/MC
    def splitting_factor(
        sample_size,
        sample,
        sample_config,
        default_splitting_data=100,
        default_splitting_mc=30,
    ):
        if sample_config["is_data"]:
            return max(default_splitting_data, int(sample_size / 200))
        else:
            if "QCD" in sample_config["ProcessGroup"]:
                return max(default_splitting_mc, int(sample_size / 100))

            if "NuNu" in sample_config["ProcessGroup"]:
                return max(default_splitting_mc, int(sample_size / 100))

            if sample.startswith("TTTo2L2Nu"):
                return max(50, int(sample_size / 100))

            if sample.startswith("TT"):
                return max(10, int(sample_size / 100))

            if sample.startswith("ttH"):
                return max(10, int(sample_size / 100))

            if sample.startswith("ZZZ"):
                return max(10, int(sample_size / 100))

            if sample.startswith("WZZ"):
                return max(10, int(sample_size / 100))

            if sample.startswith("WWZ"):
                return max(10, int(sample_size / 100))

            if sample.startswith("tZq"):
                return max(10, int(sample_size / 100))

            return max(default_splitting_mc, int(sample_size / 100))

    with Pool(min(len(files), 50)) as pool:
        packed_splitting = pool.map(
            partial(get_splitting_imp, splitting_factor=10000), files
        )

    unpacked_splitting = list(itertools.chain(*packed_splitting))

    return split_list_of_blocks(
        unpacked_splitting,
        splitting_factor(len(unpacked_splitting), sample, sample_config),
    )


def dump_list_to_file(lst, file_path):
    with open(file_path, "w") as file:
        for item in lst:
            file.write(f"{item[0]} {item[1]} {item[2]}" + "\n")


def files_to_process(year, output_files):
    return list(
        filter(
            lambda file: f"_{year}/" in file,
            output_files,
        )
    )


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
        "-j",
        "--jobs",
        help="Multiprocessing pool size.",
        type=int,
        default=min(80, nproc),
    )

    parser.add_argument("--debug", help="print debugging info", action="store_true")

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

    args = parser.parse_args()

    if args.sample and not (args.year):
        raise Exception(
            'ERROR: Could not start classification. When "--sample" is set, "--year" is required.'
        )
    return args


def submit_classification(
    task_inputs,
    process,
    year,
    luminosity,
    is_data,
    xsection,
    filter_eff,
    k_factor,
    processOrder,
    processGroup,
    output_base,
    username,
    debug,
):
    buffer_index, input_files = task_inputs

    output_path: str = f"{output_base}/classification_outputs/{year}/{process}/buffer/buffer_{buffer_index}"
    if not (os.path.isdir(output_path)):
        os.system(f"mkdir -p {output_path}")

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


def root_file_merger(input_files, output_file_path):
    merger = ROOT.TFileMerger(False, True)
    merger.SetFastMethod()
    merger.SetNotrees(True)

    for f in input_files:
        in_file = ROOT.TFile.Open(f)
        merger.AddFile(in_file, True)

    merger.OutputFile(output_file_path)
    return merger.Merge()


def merge_cutflow_histograms(input_files, process, year, output_path, debug):
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
        sys.exit(f"ERROR: could not merge cutflow files.\n{error}")


def call_srmls(username, count, offset, debug):
    time.sleep(random.uniform(1, 3))

    command = f"srmls -count={count} -offset={offset} srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2\\?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/{username}/classification_buffer/"
    srmls_merge_result = subprocess.run(
        shlex.split(command),
        capture_output=True,
    )
    if debug:
        print(srmls_merge_result.stdout.decode("utf-8"))

    if srmls_merge_result.returncode != 0:
        error = srmls_merge_result.stderr.decode("utf-8")
        # sys.exit(f"ERROR: could not merge output files, per sample.\n{error}")
        sys.exit(
            f"ERROR: could not merge output files, per sample. Executed command: {command}"
        )

    return srmls_merge_result.stdout.decode("utf-8")


def get_files_in_buffer_area(
    srmls_output: str,
    process: str,
    year: str,
    process_group: str,
    xsec_order: str,
    username: str,
    debug: bool,
):
    # count = 9000
    # offset = 0
    # srmls_output = call_srmls(username, count, offset, debug)
    # while ".root" in srmls_output or "ec_classes" in srmls_output:
    #     offset += count
    #     srmls_output = srmls_output + "\n" + call_srmls(username, count, offset, debug)

    if ".root" not in srmls_output or "ec_classes" not in srmls_output:
        sys.exit(
            # f"ERROR: could not merge output files, per sample. No ROOT file found.\n{srmls_output}"
            f"ERROR: could not merge output files, per sample. No ROOT file found."
        )

    buffer_files = []
    for line in srmls_output.split("\n"):
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


def merge_classification_results(args):
    (
        srmls_output,
        process,
        year,
        process_group,
        xsec_order,
        output_file_path,
        username,
        debug,
    ) = args

    input_files = get_files_in_buffer_area(
        srmls_output,
        process,
        year,
        process_group,
        xsec_order,
        username,
        debug,
    )

    print(
        f"[Merger - {process} {year}] Merging output files ({len(input_files)} files ) ..."
    )

    classification_merge_result = root_file_merger(input_files, output_file_path)

    if not (classification_merge_result):
        sys.exit(f"ERROR: could not merge output files, per sample.")

    print(f"[Merger - {process} {year}] Done.")


def main():
    print("\n\n📶 [ MUSiC Classification ] 📶\n")

    # parse arguments
    args = parse_args()

    if args.debug:
        print("Will run in DEBUG mode ...")

    # load analysis config file
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    if args.harvest:
        print("Getting list of files ...")
        srmls_output = call_srmls(args.username, 9000, 0, args.debug)
        srmls_output = srmls_output + call_srmls(args.username, 9000, 9000, args.debug)

        print("Harvesting classification results...")
        harvest_arguments = []
        for sample in task_config:
            if sample != "Lumi" and sample != "Global":
                for year in years:
                    if f"das_name_{year}" in task_config[sample].keys():
                        if process_filter(
                            args, task_config[sample]["is_data"], sample, year
                        ):
                            # merge EventClass histograms
                            harvest_arguments.append(
                                (
                                    srmls_output,
                                    sample,
                                    year,
                                    "Data"
                                    if task_config[sample]["is_data"]
                                    else task_config[sample]["ProcessGroup"],
                                    "DUMMY"
                                    if task_config[sample]["is_data"]
                                    else task_config[sample]["XSecOrder"],
                                    # f"{args.output}/classification_outputs/{year}/{sample}/{sample}_{year}.root",
                                    f"/disk1/silva/classification_histograms/{sample}_{year}.root",
                                    args.username,
                                    args.debug,
                                )
                            )

        # data first
        harvest_arguments = sorted(
            harvest_arguments, key=lambda x: x[3] == "Data", reverse=True
        )

        # TTbar samples first
        harvest_arguments = sorted(
            harvest_arguments, key=lambda x: x[1].startswith("TT"), reverse=True
        )

        # then ZZ
        harvest_arguments = sorted(
            harvest_arguments, key=lambda x: x[1].startswith("ZZ"), reverse=True
        )

        # them WW
        harvest_arguments = sorted(
            harvest_arguments, key=lambda x: x[1].startswith("WW"), reverse=True
        )

        with Pool(min([args.jobs, len(harvest_arguments), 100])) as pool:
            list(
                tqdm(
                    pool.imap_unordered(
                        merge_classification_results, harvest_arguments
                    ),
                    total=len(harvest_arguments),
                    unit=" samples",
                )
            )

        sys.exit(0)

    print("Merging cutflow histograms...")
    with Pool(args.jobs) as pool:
        res = []
        for sample in tqdm(task_config, unit=" tasks"):
            if sample != "Lumi" and sample != "Global":
                for year in years:
                    if f"das_name_{year}" in task_config[sample].keys():
                        if process_filter(
                            args, task_config[sample]["is_data"], sample, year
                        ):
                            if not (
                                os.path.isdir(
                                    f"{args.output}/classification_outputs/{year}/{sample}"
                                )
                            ):
                                os.system(
                                    f"mkdir -p {args.output}/classification_outputs/{year}/{sample}"
                                )

                            # merge cutflow histograms
                            res.append(
                                pool.map_async(
                                    partial(
                                        merge_cutflow_histograms,
                                        process=sample,
                                        year=year,
                                        output_path=f"{args.output}/classification_outputs/{year}/{sample}",
                                        debug=args.debug,
                                    ),
                                    [
                                        files_to_process(
                                            year, task_config[sample]["output_files"]
                                        )
                                    ],
                                ),
                            )
        print()
        print("Waiting for all merging processes to complete ...")
        [r.wait() for r in tqdm(res, unit=" samples")]

        print(
            f"Done. Merged {len(list(itertools.chain(*[r.get() for r in res])))} cutflow histograms."
        )

    print()

    print("Submiting jobs ...")
    with Pool(args.jobs) as pool:
        res = []
        for sample in tqdm(task_config, unit=" tasks"):
            if sample != "Lumi" and sample != "Global":
                for year in years:
                    if f"das_name_{year}" in task_config[sample].keys():
                        if process_filter(
                            args, task_config[sample]["is_data"], sample, year
                        ):
                            if not (
                                os.path.isdir(
                                    f"{args.output}/classification_outputs/{year}/{sample}"
                                )
                            ):
                                os.system(
                                    f"mkdir -p {args.output}/classification_outputs/{year}/{sample}"
                                )

                            # submit job
                            res.append(
                                (
                                    sample,
                                    pool.map_async(
                                        partial(
                                            submit_classification,
                                            process=sample,
                                            year=year,
                                            luminosity=task_config["Lumi"][year],
                                            is_data=task_config[sample]["is_data"],
                                            xsection=1.0
                                            if task_config[sample]["is_data"]
                                            else task_config[sample]["XSec"],
                                            filter_eff=1.0
                                            if task_config[sample]["is_data"]
                                            else task_config[sample]["FilterEff"],
                                            k_factor=1.0
                                            if task_config[sample]["is_data"]
                                            else task_config[sample]["kFactor"],
                                            processOrder="DUMMY"
                                            if task_config[sample]["is_data"]
                                            else task_config[sample]["XSecOrder"],
                                            processGroup="Data"
                                            if task_config[sample]["is_data"]
                                            else task_config[sample]["ProcessGroup"],
                                            output_base=args.output,
                                            username=args.username,
                                            debug=args.debug,
                                        ),
                                        enumerate(
                                            get_spliting(
                                                files_to_process(
                                                    year,
                                                    task_config[sample]["output_files"],
                                                ),
                                                sample,
                                                task_config[sample],
                                            )
                                        ),
                                    ),
                                ),
                            )
        print()
        print("Waiting for all submitions to complete ...")
        [print(r[0], r[1].wait()) for r in tqdm(res, unit=" samples")]

        print(
            f"Done. Submitted {len(list(itertools.chain(*[r[1].get() for r in res])))} jobs."
        )


if __name__ == "__main__":
    main()
