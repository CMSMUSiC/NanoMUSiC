#!/usr/bin/env python3

import os
from tqdm import tqdm
import re
import argparse
import time


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--no-resubmit",
        required=False,
        action="store_true",
        default=False,
        help="Should ask for resubmition of failed jobs.",
    )

    parser.add_argument(
        "--buffer-dir",
        required=False,
        type=str,
        help="Condor buffer directory.",
        default="classification_outputs",
    )

    parser.add_argument(
        "--always-resubmit",
        required=False,
        action="store_true",
        help="Will not ask in case of resubmition.",
        default=False,
    )

    return parser.parse_args()


def check_file_for_pattern(file_path, pattern):
    with open(file_path, "r") as file:
        for line in file:
            if re.search(pattern, line):
                return True
    return False


def find_directories_with_prefix(directory, prefix):
    matching_directories = []

    for root, dirs, files in os.walk(directory):
        for dir in dirs:
            if dir.startswith(prefix):
                matching_directories.append(os.path.join(root, dir))

    return matching_directories


def check_file_for_string(file_path, target_string):
    with open(file_path, "r") as file:
        for line in file:
            if target_string in line:
                return True
    return False


def resubmit(job, always_resubmit=False):
    print(f"\n\n----- {job}")
    print(
        f"\n\n \033[1m*************************** Log ***************************\033[0m"
    )
    if os.path.isfile(f"{job}/condor.log"):
        os.system(f"head {job}/condor.log")
        print("\n[...]\n")
        os.system(f"tail -20 {job}/condor.log")
    else:
        print("Log file not found.")

    print(
        f"\n\n \033[1m*************************** Out ***************************\033[0m"
    )
    if os.path.isfile(f"{job}/condor.out"):
        os.system(f"head {job}/condor.out")
        print("\n[...]\n")
        os.system(f"tail -20 {job}/condor.out")
    else:
        print("Output file not found.")

    print(
        f"\n\n \033[1m*************************** Error ***************************\033[0m"
    )
    if os.path.isfile(f"{job}/condor.err"):
        os.system(f"head {job}/condor.err")
        print("\n[...]\n")
        os.system(f"tail -20 {job}/condor.err")
    else:
        print("Error file not found.")

    if not (always_resubmit) and os.path.isfile(f"{job}/condor.jdl"):
        resubmit = input(f"Resubmit {job}: [r - resubmit / <ENTER> - skip] ")
        if resubmit == "y":
            os.system(f"rm {job}/*.root")
            os.system(f"rm {job}/condor.out")
            os.system(f"rm {job}/condor.err")
            os.system(f"rm {job}/condor.log")
            os.system(f"condor_submit {job}/condor.jdl")
    else:
        os.system(f"rm {job}/*.root")
        os.system(f"rm {job}/condor.out")
        os.system(f"rm {job}/condor.err")
        os.system(f"rm {job}/condor.log")
        os.system(f"condor_submit {job}/condor.jdl")


def main():
    # parse arguments
    args = parse_args()

    sleep_time = 5

    # Call the recursive function to find matching directories
    matching_directories = find_directories_with_prefix(args.buffer_dir, "buffer_")

    completed_jobs = []
    while True:
        job_status = {}
        for idx in tqdm(range(len(matching_directories)), unit=" job"):
            directory = matching_directories[idx]
            if idx not in completed_jobs:
                job_status[directory] = False
                if os.path.isfile(f"{directory}/condor.err"):
                    if check_file_for_string(
                        f"{directory}/condor.err", "ERROR: NaN or INF weight found"
                    ):
                        print(
                            f"ERROR: NaN or INF weight was found at file {directory}/condor.err!"
                        )
                        exit(-1)
                if os.path.isfile(f"{directory}/condor.out"):
                    if check_file_for_string(
                        f"{directory}/condor.out", "ERROR: NaN or INF weight found"
                    ):
                        print(
                            f"ERROR: NaN or INF weight was found at file {directory}/condor.out!"
                        )
                        exit(-1)
                    if (
                        check_file_for_string(f"{directory}/condor.out", "YAY!")
                        and check_file_for_string(f"{directory}/condor.out", "COPIED!")
                        and check_file_for_string(
                            f"{directory}/condor.log", "(return value 0)"
                        )
                    ):
                        job_status[directory] = True
                        completed_jobs.append(idx)

                if (
                    (
                        check_file_for_string(f"{directory}/condor.log", "return value")
                        and (
                            not check_file_for_string(
                                f"{directory}/condor.log", "(return value 0)"
                            )
                        )
                    )
                    or (
                        os.path.isfile(f"{directory}/condor.err")
                        and os.stat(f"{directory}/condor.err").st_size != 0
                    )
                    or check_file_for_string(f"{directory}/condor.log", "aborted")
                ) and not job_status[directory]:
                    if not args.no_resubmit:
                        resubmit(directory, args.always_resubmit)
            else:
                job_status[directory] = True

        print("")
        if all(job_status.values()):
            print(f"All done ({list(job_status.values()).count(True)} jobs)!")
            os.system("condor_q | tail -5")
            exit(0)
        else:
            num_running_jobs = len(
                list(filter(lambda job: job_status[job] == False, job_status))
            )
            if num_running_jobs <= 200:
                print("Running jobs:")
                for job in job_status:
                    if job_status[job] == False:
                        print(job)
            print("")
            done_jobs = list(job_status.values()).count(True)
            jobs_todo = list(job_status.values()).count(False)
            print(f"Done: {done_jobs} / {len(list(job_status.values()))}")
            print(f"Other: {jobs_todo} / {len(list(job_status.values()))}")
            os.system("condor_q | tail -5")

        time.sleep(sleep_time)


if __name__ == "__main__":
    main()
