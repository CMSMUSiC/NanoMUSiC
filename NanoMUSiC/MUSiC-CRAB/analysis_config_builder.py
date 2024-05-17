#!/usr/bin/env python3

import tomli
from helpers import to_toml_dumps
import argparse
import os
import re
import subprocess
import shlex
from tqdm import tqdm
from pathlib import Path
import multiprocessing
import collections


parser = argparse.ArgumentParser()
parser.add_argument(
    "-u",
    "--username",
    required=True,
    help="Username of the dCache owner.",
)

parser.add_argument(
    "-dt",
    "--datetime",
    help="Submition date and time.",
    type=str,
    required=True,
)

parser.add_argument(
    "xsection_file_path",
    help="Give path to the toml file containing cross-section and das_name per sample.",
)

parser.add_argument(
    "-j", "--jobs", help="Simultanious number of jobs.", type=int, default=50
)

parser.add_argument(
    "--btag", help="Will collect outpouts for the BTagging Efficeincy code.", type=bool
)

args = parser.parse_args()


def make_outputfile_list():
    return next(os.walk("."))[1]


CollectorInputs = collections.namedtuple(
    "CollectorInputs", ["sample", "job_list", "xsection_list", "cmd_str"]
)


def collect_output_files(collector_inputs: CollectorInputs):
    address_list = []
    if "crab_task_name" in collector_inputs.job_list[collector_inputs.sample]:
        for task in collector_inputs.xsection_list[collector_inputs.sample][
            "crab_task_name"
        ]:
            scan_process = subprocess.run(
                shlex.split(collector_inputs.cmd_str.replace("__TASKNAME__", task)),
                capture_output=True,
                text=True,
            )
            if scan_process.returncode == 0:
                for addr in scan_process.stdout.split("\n"):
                    if r"nano_music" in addr and addr.endswith("root"):
                        # address = addr.replace(
                        #     r"davs://grid-webdav.physik.rwth-aachen.de:2889",
                        #     r"root://grid-dcache.physik.rwth-aachen.de//",
                        # )
                        # address_list.append(address)
                        address_list.append(addr)

    if len(address_list) == 0:
        print(
            f"WARNING: Could not find any ROOT file for sample: {collector_inputs}.\n\n"
        )
    return collector_inputs.sample, address_list


def main():
    print("[ Collecting results ... ]")
    directory_list = make_outputfile_list()
    xsection_list = tomli.loads(
        Path(args.xsection_file_path).read_text(encoding="utf-8")
    )
    for sample in xsection_list:
        xsection_list[sample]["crab_task_name"] = []
        for filename in directory_list:
            if re.search(f"crab_nano_music_{sample}", filename):
                xsection_list[sample]["crab_task_name"].append(filename)

    # remove unwanted entries
    unwanted_entries = []
    for sample in xsection_list:
        if "crab_task_name" not in xsection_list[sample]:
            unwanted_entries.append(sample)

    for sample in unwanted_entries:
        if sample in xsection_list.keys():
            del xsection_list[sample]

    job_list = xsection_list

    cmd_str = r"gfal-ls-recursive davs://grid-webdav.physik.rwth-aachen.de:2889/store/user/__USERNAME__/nano_music___DATETIME__/__TASKNAME__/"
    cmd_str = cmd_str.replace("__USERNAME__", args.username)
    cmd_str = cmd_str.replace("__DATETIME__", args.datetime)

    print("--> Collecting outputs path ...")
    with multiprocessing.Pool(args.jobs) as pool:
        results = list(
            tqdm(
                pool.imap(
                    collect_output_files,
                    [
                        CollectorInputs(sample, job_list, xsection_list, cmd_str)
                        for sample in job_list
                    ],
                ),
                total=len(job_list),
            )
        )

    def append_or_create(d, k, i):
        if k not in d:
            d[k] = []
        d[k].append(i)

    for r in results:
        sample, address_list = r
        for addr in address_list:
            if "2016APV" in addr:
                append_or_create(job_list[sample], "output_files_2016APV", addr)
            if "2016" in addr:
                append_or_create(job_list[sample], "output_files_2016", addr)
            if "2017" in addr:
                append_or_create(job_list[sample], "output_files_2017", addr)
            if "2018" in addr:
                append_or_create(job_list[sample], "output_files_2018", addr)

    # dump new config to string
    crab_job_list = to_toml_dumps(job_list)

    os.system("rm analysis_config.toml > /dev/null 2>&1")
    with open("analysis_config.toml", "w") as new_jobList_file:
        new_jobList_file.write(crab_job_list)

    print("Output saved to: analysis_config.toml")
    print("[ Done ]")


if __name__ == "__main__":
    main()
