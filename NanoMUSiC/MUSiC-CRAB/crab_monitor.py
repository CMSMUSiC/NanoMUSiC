#!/usr/bin/env python3

from contextlib import contextmanager
import sys
import os
import glob
import subprocess
import argparse

from CRABClient.UserUtilities import config
from CRABAPI.RawCommand import crabCommand


def parse_args():
    parser = argparse.ArgumentParser(description='Monitor CRAB MUSiC jobs.')
    parser.add_argument('-ec','--exclude-completed', action='store_true', help='will not report COMPLETED taks.', required=True)
    # parser.add_argument('-b','--bar', help='Description for bar argument', required=True)
    return vars(parser.parse_args())



def get_crab_dirs():
    return glob.glob("crab_nano_music_*/crab_*/", recursive=True)


# define clear function
def clear():
    # check and make call for specific operating system
    _ = subprocess.run('clear ; clear ; clear ; clear ; clear ' if os.name == 'posix' else 'cls', shell=True)

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
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

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

def monitor(dir, idx, completed, failing, others, total_task, exclude_completed):
    with suppress_stdout():
        res = crabCommand("status", dir=dir)

    inputDataset = res["inputDataset"]
    jobsPerStatus = res["jobsPerStatus"]
    proxiedWebDir=res["proxiedWebDir"].split("/")[-1]
    task_status = res["status"]
    monitoring_url = f"https://cmsweb.cern.ch/crabserver/ui/task/{proxiedWebDir}"
    
    if "failed" in jobsPerStatus:
        failing += 1
    elif task_status == "COMPLETED":
        completed += 1
    else:
        others += 1

    if not (exclude_completed and task_status == "COMPLETED"):
        if idx == 0:
            clear()
        
        print(f"\n{bcolors.HEADER}===================={bcolors.ENDC}{bcolors.BOLD} CRAB Task: [{idx+1}/{total_task}] {bcolors.ENDC}{bcolors.ENDC}{bcolors.HEADER}===================={bcolors.ENDC}")
        print(f"Task directory: {bcolors.BOLD}{dir}{bcolors.ENDC}")
        print(f"Input Dataset: {bcolors.BOLD}{inputDataset}{bcolors.ENDC}")
        print(f"Task status: {print_colored(task_status)}")
        print(f"Monitoring URL: {bcolors.BOLD}{monitoring_url}{bcolors.ENDC}")
        print(f"Jobs per status:")
        for status in jobsPerStatus:
            print(f"{print_colored(status)} : {jobsPerStatus[status]} {get_emojis(status)}")
        if "failed" in jobsPerStatus:
            print(f"{bcolors.WARNING}Resubmit command:{bcolors.ENDC}")
            print(f"crab resubmit -d {dir}")
        idx += 1
    return idx, completed, failing, others 
    
def next_command():
    next_cmd = "invalid"
    while next_cmd == "invalid":
        cmd = input(f"{bcolors.BOLD}\nCommand: [q] quit - [<ENTER>] reload:{bcolors.ENDC} ")
        if cmd == "q":
            exit(0)
        elif cmd == "":
            print("Reloading ...")
            return "reload"
        else:
            print("[ ERROR ] Invalid option.")



def main():
    args = parse_args()
    exclude_completed = args["exclude_completed"]
    while(True):
        idx = 0
        completed = 0
        failing = 0
        others = 0
        for dir in get_crab_dirs():
            idx, completed, failing, others = monitor(dir, idx, completed, failing, others, len(get_crab_dirs()), exclude_completed)
        if exclude_completed:
            print(f"\n{bcolors.BOLD}==================== [ REPORT ] ===================={bcolors.ENDC}")
            print(f"{bcolors.OKGREEN}Completed{bcolors.ENDC}: {completed}/{len(get_crab_dirs())} tasks{bcolors.ENDC}")
            print(f"{bcolors.FAIL}Failing{bcolors.ENDC}: {failing}/{len(get_crab_dirs())} tasks{bcolors.ENDC}")
            print(f"{bcolors.ENDC}Others{bcolors.ENDC}: {others}/{len(get_crab_dirs())} tasks{bcolors.ENDC}")
        if next_command() == "reload":
            pass


            
        

if __name__=="__main__":
    main()
