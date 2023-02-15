#!/usr/bin/env python

import os
import glob
import shlex, subprocess
from datetime import datetime

# Set environment variables
MUSIC_BASE = os.getenv('MUSIC_BASE')

task_files = glob.glob(f"{MUSIC_BASE}/configs/task_configs/*.toml")

print(f"[{datetime.now().strftime('%d/%m/%Y %H:%M:%S')}] Starting ...")

procs = []
for task_file in task_files:
    print(f"[{datetime.now().strftime('%d/%m/%Y %H:%M:%S')}] Launching: nano_music --run-config {task_file}")
    output_file = open(task_file.replace(f"{MUSIC_BASE}/configs/task_configs/", "Test_Ouputs_").replace(".toml", ".txt"), "ab")
    procs.append(subprocess.Popen(shlex.split(f"nano_music --run-config {task_file}"), 
    stdout=output_file, stderr=output_file))


print("")
print(f"[{datetime.now().strftime('%d/%m/%Y %H:%M:%S')}] Waiting tasks to finish ...")
return_codes = []
for p in procs:
    return_codes.append(p.wait())

if all(r == 0 for r in return_codes):
    print(f"[{datetime.now().strftime('%d/%m/%Y %H:%M:%S')}] YAY !!!")
else:
    print(f"[{datetime.now().strftime('%d/%m/%Y %H:%M:%S')}] Oops ...")

