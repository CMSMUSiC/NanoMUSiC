#!/usr/bin/env python3

import os
import datetime
import json
import getpass


class CondorScheduler:
    def __init__(self, output_folder="", input_folder="", prologue=""):
        self.prologue = "; ".join(prologue)
        self.job_id_to_args = {}
        self.job_id = 0
        self.current_dir = os.getenv("PWD")
        if output_folder == "":
            output_folder = os.getenv("PWD")

        now = datetime.datetime.now()
        self.output_file_path = (
            f"{output_folder}/outputs_{now.strftime('%Y_%m_%d_%H_%M_%S')}"
        )

        os.mkdir(self.output_file_path)

        self.input_folder = input_folder
        if self.input_folder != "":
            print("Packing input files ...")
            os.system(r"rm task.tar.gz > /dev/null 2>&1")
            tar_command = r'tar --exclude="*.log" --exclude="crab.log" --exclude="task.tar.gz" --exclude="__pycache*" --exclude="build" --exclude="docs_BKP" --exclude="docs" --exclude="*.root" --exclude="NanoMUSiC/MUSiC-CRAB/crab_nano_music*" -zcvf task.tar.gz __TAR_FOLDER__/*'
            tar_command = tar_command.replace("__TAR_FOLDER__", input_folder)
            os.system(tar_command)
            print("")

    def submit_task(self, executable_script, arguments):
        if not isinstance(arguments, list):
            arguments = [arguments]

        base_jdl = """
	universe     = vanilla
	getenv       = true
	executable   = __EXECUTABLE__
	LogDirectory= __LOG__
	log          = $(LogDirectory)/condor-$(CLUSTER)_$(PROCESS).log
	Output       = $(LogDirectory)/condor.out
	Error        = $(LogDirectory)/condor.err
	Stream_Output = true
	Stream_Error = true
	#requirements = Machine != "lx1b00.physik.rwth-aachen.de"
	request_memory=1300
	rank = Memory
	notification = never
	queue
	"""

        base_jdl = base_jdl.replace(
            "__LOG__", f"{self.output_file_path}/job_id_{self.job_id}"
        )
        string_arguments = []
        for var in arguments:
            string_arguments.append(str(var))
        executable = f"mkdir -p /user/scratch/{getpass.getuser()}; cd /user/scratch/{getpass.getuser()}; working_dir=$RANDOM; mkdir $working_dir; cd $working_dir;"
        if self.input_folder != "":
            executable += (
                f"/net/software_t2k/tools/cccp/cccp {self.current_dir}/task.tar.gz .; "
            )
            executable += r"tar -zxf task.tar.gz;"

        executable += f"{self.prologue}; "
        executable += f"{executable_script} {' '.join(string_arguments)}"
        os.mkdir(f"{self.output_file_path}/job_id_{self.job_id}")
        with open(
            f"{self.output_file_path}/job_id_{self.job_id}/executable.sh", "w"
        ) as f:
            f.write(executable)
        os.system(
            f"chmod +x {self.output_file_path}/job_id_{self.job_id}/executable.sh"
        )

        base_jdl = base_jdl.replace(
            "__EXECUTABLE__",
            f"{self.output_file_path}/job_id_{self.job_id}/executable.sh",
        )
        with open(f"{self.output_file_path}/job_id_{self.job_id}/condor.jdl", "w") as f:
            f.write(base_jdl)

        self.job_id_to_args[self.job_id] = {}
        self.job_id_to_args[self.job_id]["results"] = ""
        self.job_id_to_args[self.job_id]["args"] = [executable_script]
        for arg in arguments:
            self.job_id_to_args[self.job_id]["args"].append(str(arg))
        os.system(f"condor_submit {self.output_file_path}/job_id_{self.job_id}/condor.jdl")
        self.job_id += 1
         
    def finalise(self):
        with open(f"{self.output_file_path}/job_id_to_args.json", "w") as f:
            json.dump(self.job_id_to_args, f)

    @classmethod
    def harvest(cls, output_path):
        with open(f"{output_path}/job_id_to_args.json") as f:
            job_id_to_args = json.load(f)

        for id in job_id_to_args:
            with open(f"{output_path}/job_id_{id}/condor.out") as f:
                job_id_to_args[id]["results"] = f.read()

        with open(f"{output_path}/job_id_to_args.json", "w") as f:
            json.dump(job_id_to_args, f)

        return job_id_to_args
