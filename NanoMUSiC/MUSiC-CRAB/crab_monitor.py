#!/usr/bin/env python3

from multiprocessing import Pool
import tqdm
from contextlib import contextmanager
import sys
import os
import glob
import subprocess
import argparse

from CRABClient.UserUtilities import config
from CRABAPI.RawCommand import crabCommand


def parse_args():
    parser = argparse.ArgumentParser(description="Monitor CRAB MUSiC jobs.")
    parser.add_argument(
        "-ec",
        "--exclude-completed",
        action="store_true",
        help="will not report COMPLETED taks.",
        required=False,
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=10,
        help="number of parallel monitoring jobs.",
        required=False,
    )
    return vars(parser.parse_args())


def get_crab_dirs():
    return glob.glob("crab_nano_music_*/crab_*/", recursive=True)


# define clear function
def clear():
    # check and make call for specific operating system
    _ = subprocess.run(
        "clear ; clear ; clear ; clear ; clear " if os.name == "posix" else "cls",
        shell=True,
    )


@contextmanager
def suppress_stdout():
    with open(os.devnull, "w") as devnull:
        old_stdout = sys.stdout
        sys.stdout = devnull
        try:
            yield
        finally:
            sys.stdout = old_stdout


class bcolors:
    HEADER = "\033[95m"
    OKBLUE = "\033[94m"
    OKCYAN = "\033[96m"
    OKGREEN = "\033[92m"
    WARNING = "\033[93m"
    FAIL = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"


def get_emojis(status):
    if status == "finished" or status == "COMPLETED":
        return "😎✅"
    elif status == "failed":
        return "😡❌"
    elif status == "transfering":
        return "😋🆗"
    else:
        return "🤔"


def print_colored(status):
    if status == "finished" or status == "COMPLETED":
        return f"{bcolors.OKGREEN}{status.capitalize()}{bcolors.ENDC}"
    elif status == "failed":
        return f"{bcolors.FAIL}{status.capitalize()}{bcolors.ENDC}"
    elif status == "transfering":
        return f"{bcolors.OKBLUE}{status.capitalize()}{bcolors.ENDC}"
    else:
        return f"{status.capitalize()}"


def monitor(dir):
    try:
        with suppress_stdout():
            res = crabCommand("status", dir=dir)

        inputDataset = res["inputDataset"]
        jobsPerStatus = res["jobsPerStatus"]
        proxiedWebDir = res["proxiedWebDir"].split("/")[-1]
        task_status = res["status"]
        monitoring_url = f"https://cmsweb.cern.ch/crabserver/ui/task/{proxiedWebDir}"

        status = ""
        if "failed" in jobsPerStatus:
            status = "failed"
        elif task_status == "COMPLETED":
            status = "completed"
        else:
            status = "other"

        report = ""
        kill_command = ""
        retry_command = ""

        report += f"Task directory: {bcolors.BOLD}{dir}{bcolors.ENDC}\n"
        report += f"Input Dataset: {bcolors.BOLD}{inputDataset}{bcolors.ENDC}\n"
        report += f"Task status: {print_colored(task_status)}\n"
        report += f"Monitoring URL: {bcolors.BOLD}{monitoring_url}{bcolors.ENDC}\n"
        report += f"Jobs per status:\n"
        for _status in jobsPerStatus:
            report += f"{print_colored(_status)} : {jobsPerStatus[_status]} {get_emojis(_status)}\n"

        if "failed" in jobsPerStatus:
            report += f"{bcolors.WARNING}Resubmit command:{bcolors.ENDC}\n"
            report += f"crab resubmit -d {dir}\n"
            retry_command = f"crab resubmit -d {dir}"
        report += f"{bcolors.WARNING}Kill command:{bcolors.ENDC}\n"
        report += f"crab kill -d {dir}\n"
        kill_command = f"crab kill -d {dir}"

        return status, report, kill_command, retry_command

    except:
        status = "task_not_submitted"
        report = dir
        kill_command = ""
        retry_command = ""
        return status, report, kill_command, retry_command


def next_command():
    next_cmd = "invalid"
    while next_cmd == "invalid":
        cmd = input(
            f"{bcolors.BOLD}\nCommand: [q | quit] quit CRAB monitor - [k | kill] kill all tasks - [r | retry] retry all failed tasks - [d | del] delete directories of non-submitted jobs - [<ENTER>] reload:{bcolors.ENDC} "
        )
        if cmd == "q" or cmd == "quit":
            return "quit"
        elif cmd == "r" or cmd == "retry":
            return "retry"
        elif cmd == "k" or cmd == "kill":
            return "kill"
        elif cmd == "del" or cmd == "d":
            return "delete"
        elif cmd == "":
            return "reload"
        else:
            print("[ ERROR ] Invalid option.")


def main():
    args = parse_args()
    exclude_completed = args["exclude_completed"]
    jobs = min(args["jobs"], len(get_crab_dirs()))
    while True:
        idx = 0
        completed = 0
        failing = 0
        others = 0
        kill_commands = []
        retry_commands = []
        args = []
        non_submitted_jobs = []
        with Pool(jobs) as p:
            # submit monitoring tasks
            monitoring_results = list(
                tqdm.tqdm(
                    p.imap(monitor, get_crab_dirs()),
                    total=len(get_crab_dirs()),
                    desc=f"Monitoring tasks [{jobs} jobs] ...",
                    unit=" tasks",
                )
            )

            total_tasks = len(monitoring_results)


            # loop over monitroting results
            for idx, (status, report, kill_command, retry_command) in enumerate(
                monitoring_results
            ):
                if status == "task_not_submitted":
                    non_submitted_jobs.append(report)

                if not (exclude_completed and status == "completed"):
                    # clear screen for first task report
                    if idx == 0:
                        clear()

                    # report header
                    print(
                        f"\n{bcolors.HEADER}===================={bcolors.ENDC}{bcolors.BOLD} CRAB Task: [{idx+1}/{total_tasks}] {bcolors.ENDC}{bcolors.ENDC}{bcolors.HEADER}===================={bcolors.ENDC}"
                    )

                    # print report
                    print(report)

                # store kill and retry commands
                kill_commands.append(kill_command)
                retry_commands.append(retry_command)

                # count states
                if status == "failed":
                    failing += 1
                elif status == "completed":
                    completed += 1
                else:
                    others += 1

            # print summary report
            print(
                f"\n{bcolors.BOLD}==================== [ REPORT ] ===================={bcolors.ENDC}"
            )
            print(
                f"{bcolors.OKGREEN}Completed{bcolors.ENDC}: {completed}/{len(get_crab_dirs())} tasks{bcolors.ENDC}"
            )
            print(
                f"{bcolors.FAIL}Failed{bcolors.ENDC}: {failing}/{len(get_crab_dirs())} tasks{bcolors.ENDC}"
            )
            print(
                f"{bcolors.ENDC}Others{bcolors.ENDC}: {others}/{len(get_crab_dirs())} tasks{bcolors.ENDC}"
            )

            # print("Following files were not submitted:")
            # for job in non_submitted_jobs:
            #     print(job)


            # read next command
            cmd = next_command()
            if cmd == "quit":
                exit(0)
            if cmd == "delete":
                if(len(non_submitted_jobs) != 0):
                    directories = len(non_submitted_jobs)
                    print(f"Deleting {directories} directories")
                    all_jobs = " ".join(non_submitted_jobs)
                    del_command = "rm -rf " + all_jobs
                    print("Deleting directories for non-submitted tasks")
                    os.system(del_command)
                else:
                    print("No directories to delete")
            if cmd == "retry":
                print("Resubminting all failed tasks ...")
                for command in retry_commands:
                    os.system(command)
            if cmd == "kill":
                print("Killing all tasks ...")
                for command in kill_commands:
                    os.system(command)
                exit(0)


if __name__ == "__main__":
    main()
