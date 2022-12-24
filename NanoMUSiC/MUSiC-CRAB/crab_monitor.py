#!/usr/bin/env python3

from contextlib import contextmanager
import sys
import os
import glob
import subprocess

from CRABClient.UserUtilities import config
from CRABAPI.RawCommand import crabCommand


from time import sleep
 
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

def print_colored(status):
    if status == "finished" or status == "COMPLETED":
        return f"{bcolors.OKGREEN}{status.capitalize()}{bcolors.ENDC}"
    elif status == "failed":
        return f"{bcolors.FAIL}{status.capitalize()}{bcolors.ENDC}"
    elif status == "transfering":
        return f"{bcolors.OKBLUE}{status.capitalize()}{bcolors.ENDC}"
    else: 
        return f"{status.capitalize()}"

def monitor(dir, idx, total_task):
    with suppress_stdout():
        res = crabCommand("status", dir=dir)

    inputDataset = res["inputDataset"]
    jobsPerStatus = res["jobsPerStatus"]
    proxiedWebDir=res["proxiedWebDir"].split("/")[-1]
    task_status = res["status"]
    monitoring_url = f"https://cmsweb.cern.ch/crabserver/ui/task/{proxiedWebDir}"
    if idx == 0:
        clear()
    
    print(f"\n{bcolors.HEADER}===================={bcolors.ENDC}{bcolors.BOLD} CRAB Task: [{idx+1}/{total_task}] {bcolors.ENDC}{bcolors.ENDC}{bcolors.HEADER}===================={bcolors.ENDC}")
    print(f"Task directory: {bcolors.BOLD}{dir}{bcolors.ENDC}")
    print(f"Input Dataset: {bcolors.BOLD}{inputDataset}{bcolors.ENDC}")
    print(f"Task status: {print_colored(task_status)}")
    print(f"Monitoring URL: {bcolors.BOLD}{monitoring_url}{bcolors.ENDC}")
    print(f"Jobs per status:")
    for status in jobsPerStatus:
        print(f"{print_colored(status)} : {jobsPerStatus[status]}")
    if "failed" in jobsPerStatus:
        print(f"{bcolors.WARNING}Resubmit command:{bcolors.ENDC}")
        print(f"crab resubmit -d {dir}")
    
def next_command():
    next_cmd = "invalid"
    while next_cmd == "invalid":
        cmd = input(f"{bcolors.BOLD}\nCommand: (q) quit - (<ENTER>) reload{bcolors.ENDC}: ")
        if cmd == "q":
            exit(0)
        elif cmd == "r":
            print("Reloading ...")
            return "reload"
        else:
            print("[ ERROR ] Invalid option.")

def main():
    while(True):
        for idx, dir in enumerate(get_crab_dirs()):
            monitor(dir, idx, len(get_crab_dirs()))
        if next_command() == "reload":
            pass


            
        

if __name__=="__main__":
    main()
