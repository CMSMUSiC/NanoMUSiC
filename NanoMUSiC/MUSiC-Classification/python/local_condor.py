import os
import subprocess
import shlex


def get_absolute_path(relative_path):
    return os.path.abspath(relative_path)


def get_is_data_argument(is_data: bool):
    if is_data:
        return "--is_data"
    return ""


def make_condor_executable(
    process,
    year,
    is_data,
    output_path,
    xsection,
    filter_eff,
    k_factor,
    luminosity,
    xs_order,
    process_group,
    input_file,
    log_dir,
    buffer_index,
    username,
):
    current_dir = os.getcwd()
    executable = []
    executable.append(r"#!/usr/bin/bash")
    executable.append(" ")
    executable.append("set -e")
    executable.append("date")
    executable.append("hostname")
    executable.append(f"cd {current_dir}/..")
    executable.append(r"source setenv.sh")
    executable.append(r"cd $_CONDOR_SCRATCH_DIR")
    executable.append(r"pwd")
    executable.append(f"cmake -S {current_dir}/.. -B .")
    executable.append("ninja classification")
    executable.append("date")
    executable.append("hostname")
    executable.append(
        f"$_CONDOR_SCRATCH_DIR/NanoMUSiC/MUSiC-Classification/classification --process {process} --year {year} {get_is_data_argument(is_data)} --output {get_absolute_path(output_path)} --buffer {buffer_index} --xsection {xsection} --filter_eff {filter_eff} --k_factor {k_factor} --luminosity {luminosity} --xs_order {xs_order} --process_group {process_group} --input {input_file} && echo YAY!"
    )
    executable.append("date")
    executable.append("ls")
    executable.append(
        'find . -type f -name "ec_*.root" | sed \'s/^\\.\\///\' | xargs -I {} srmcp file://`pwd`/{} "srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/__USERNAME__/classification_buffer/{}" && echo COPIED!'.replace(
            "__USERNAME__", username
        )
    )
    executable.append("date")

    file_path = f"{get_absolute_path(log_dir)}/condor.sh"
    with open(file_path, "w") as file:
        file.write("\n".join(executable))

    os.system(f"chmod +x {file_path}")

    return file_path


def cluster_block_list() -> str:
    block_list = [
        "lx3agpu1.physik.rwth-aachen.de",
        "lx3agpu2.physik.rwth-aachen.de",
        "lx3bgpu2.physik.rwth-aachen.de",
        "lx3bgpu1.physik.rwth-aachen.de",
        # "lx1b24.physik.RWTH-Aachen.de",
        # "lx3a30.physik.RWTH-Aachen.de",
        # "lxblade18.physik.RWTH-Aachen.de",
        # "lxblade17.physik.RWTH-Aachen.de",
        # "lxblade33.physik.RWTH-Aachen.de",
        # "lx1b14.physik.RWTH-Aachen.de",
        # "lx3a14.physik.RWTH-Aachen.de",
        # "lxcip24.physik.RWTH-Aachen.de",
        # "lx3b26.physik.RWTH-Aachen.de",
        # "lx3b46.physik.RWTH-Aachen.de",
        # "lx3b47.physik.RWTH-Aachen.de",
        # "lx3b34.physik.RWTH-Aachen.de",
        # "lx3b06.physik.RWTH-Aachen.de",
        # "lxcip54.physik.RWTH-Aachen.de",
        # "lxblade36.physik.RWTH-Aachen.de",
        # "lxblade39.physik.RWTH-Aachen.de",
    ]

    block_list_str = ""
    for idx, machine in enumerate(block_list):
        block_list_str = block_list_str + f'Machine != "{machine}"'
        if idx < len(block_list) - 1:
            block_list_str = block_list_str + " && "

    return block_list_str


base_jdl = r"""
universe     = vanilla
getenv       = true
use_x509userproxy = true

LogDirectory  = ___LOGDIR___
executable   = ___EXECUTABLE___ 

log          = $(LogDirectory)/condor.log
Output       = $(LogDirectory)/condor.out
Error        = $(LogDirectory)/condor.err
Stream_Output = true
Stream_Error = true

requirements = ___BLOCKLIST___ && OpSysAndVer == "SL7"

request_memory = 3072

rank = Memory
notification = never
queue 
"""


def submit_condor_task(
    process,
    year,
    is_data,
    output_path,
    xsection,
    filter_eff,
    k_factor,
    luminosity,
    xs_order,
    process_group,
    input_file,
    log_dir,
    buffer_index,
    username,
    debug,
):
    executable = make_condor_executable(
        process,
        year,
        is_data,
        output_path,
        xsection,
        filter_eff,
        k_factor,
        luminosity,
        xs_order,
        process_group,
        input_file,
        log_dir,
        buffer_index,
        username,
    )
    updated_jdl = (
        base_jdl.replace("___EXECUTABLE___", executable)
        .replace("___LOGDIR___", get_absolute_path(log_dir))
        .replace("___BLOCKLIST___", cluster_block_list())
    )

    jdl_file_name = f"{log_dir}/condor.jdl"
    with open(jdl_file_name, "w") as file:
        file.write(updated_jdl)

    submit_result = subprocess.run(
        shlex.split(f"condor_submit {jdl_file_name}"),
        # shlex.split(f"ls {jdl_file_name}"),
        capture_output=True,
    )
    if debug:
        print(submit_result.stdout.decode("utf-8"))

    if submit_result.returncode != 0:
        error = submit_result.stderr.decode("utf-8")
        raise RuntimeError(f"ERROR: Could not submit condor job. Output:\n{error}")
