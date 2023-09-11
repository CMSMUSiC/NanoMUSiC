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
    executable.append(f"ninja classification")
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


# universe = grid
# grid_resource = condor grid-ce-1-rwth.gridka.de grid-ce-1-rwth.gridka.de:9619
# arguments    = ____ARGUMENTS___
# requirements = Machine != "lx3bgpu2.physik.rwth-aachen.de" &&  Machine != "lx1b02.physik.RWTH-Aachen.de" && Machine != "lx3a03.physik.rwth-aachen.de" && Machine != "lx3a05.physik.RWTH-Aachen.de" && Machine != "lx3a06.physik.RWTH-Aachen.de" && Machine != "lx3a09.physik.RWTH-Aachen.de" && Machine != "lx3a13.physik.rwth-aachen.de" && Machine != "lx3a14.physik.rwth-aachen.de" && Machine != "lx3a15.physik.rwth-aachen.de" && Machine != "lx3a23.physik.RWTH-Aachen.de" && Machine != "lx3a25.physik.rwth-aachen.de" && Machine != "lx3a27.physik.RWTH-Aachen.de" && Machine != "lx3a46.physik.rwth-aachen.de" && Machine != "lx3a44.physik.rwth-aachen.de" && Machine != "lx3a47.physik.RWTH-Aachen.de" && Machine != "lx3a55.physik.RWTH-Aachen.de" && Machine != "lx3a56.physik.rwth-aachen.de" && Machine != "lx3b08.physik.RWTH-Aachen.de" && Machine != "lx3b09.physik.RWTH-Aachen.de" && Machine != "lx3b13.physik.rwth-aachen.de" && Machine != "lx3b18.physik.RWTH-Aachen.de" && Machine != "lx3b24.physik.RWTH-Aachen.de" && Machine != "lx3b29.physik.RWTH-Aachen.de" && Machine != "lx3b32.physik.rwth-aachen.de" && Machine != "lx3b33.physik.RWTH-Aachen.de" && Machine != "lx3b34.physik.rwth-aachen.de" && Machine != "lx3b41.physik.rwth-aachen.de" && Machine != "lx3b46.physik.RWTH-Aachen.de" && Machine != "lx3b47.physik.rwth-aachen.de" && Machine != "lx3b48.physik.rwth-aachen.de" && Machine != "lx3b49.physik.rwth-aachen.de" && Machine != "lx3b52.physik.RWTH-Aachen.de" && Machine != "lx3b55.physik.RWTH-Aachen.de" && Machine != "lx3b57.physik.rwth-aachen.de" && Machine != "lx3b62.physik.rwth-aachen.de" && Machine != "lx3b66.physik.rwth-aachen.de" && Machine != "lx3b68.physik.RWTH-Aachen.de" && Machine != "lx3b69.physik.rwth-aachen.de" && Machine != "lx3b70.physik.rwth-aachen.de" && Machine != "lx3b71.physik.rwth-aachen.de" && Machine != "lx3b99.physik.rwth-aachen.de" && Machine != "lxblade01.physik.RWTH-Aachen.de" && Machine != "lxblade02.physik.RWTH-Aachen.de" && Machine != "lxblade03.physik.RWTH-Aachen.de" && Machine != "lxblade04.physik.RWTH-Aachen.de" && Machine != "lxblade05.physik.rwth-aachen.de" && Machine != "lxblade06.physik.RWTH-Aachen.de" && Machine != "lxblade07.physik.RWTH-Aachen.de" && Machine != "lxblade08.physik.rwth-aachen.de" && Machine != "lxblade09.physik.rwth-aachen.de" && Machine != "lxblade10.physik.RWTH-Aachen.de" && Machine != "lxblade11.physik.RWTH-Aachen.de" && Machine != "lxblade12.physik.rwth-aachen.de" && Machine != "lxblade13.physik.rwth-aachen.de" && Machine != "lxblade14.physik.RWTH-Aachen.de" && Machine != "lxblade15.physik.RWTH-Aachen.de" && Machine != "lxblade16.physik.RWTH-Aachen.de" && Machine != "lxblade17.physik.RWTH-Aachen.de" && Machine != "lxblade18.physik.rwth-aachen.de" && Machine != "lxblade19.physik.rwth-aachen.de" && Machine != "lxblade20.physik.RWTH-Aachen.de" && Machine != "lxblade21.physik.RWTH-Aachen.de" && Machine != "lxblade22.physik.RWTH-Aachen.de" && Machine != "lxblade23.physik.RWTH-Aachen.de" && Machine != "lxblade24.physik.RWTH-Aachen.de" && Machine != "lxblade25.physik.RWTH-Aachen.de" && Machine != "lxblade26.physik.rwth-aachen.de" && Machine != "lxblade27.physik.RWTH-Aachen.de" && Machine != "lxblade28.physik.RWTH-Aachen.de" && Machine != "lxblade29.physik.RWTH-Aachen.de" && Machine != "lxblade30.physik.RWTH-Aachen.de" && Machine != "lxblade31.physik.rwth-aachen.de" && Machine != "lxblade32.physik.rwth-aachen.de" && Machine != "lxcip01.physik.rwth-aachen.de" && Machine != "lxcip02.physik.RWTH-Aachen.de" && Machine != "lxcip05.physik.RWTH-Aachen.de" && Machine != "lxcip06.physik.RWTH-Aachen.de" && Machine != "lxcip09.physik.rwth-aachen.de" && Machine != "lxcip10.physik.rwth-aachen.de" && Machine != "lxcip11.physik.RWTH-Aachen.de" && Machine != "lxcip12.physik.rwth-aachen.de" && Machine != "lxcip14.physik.RWTH-Aachen.de" && Machine != "lxcip15.physik.rwth-aachen.de" && Machine != "lxcip16.physik.rwth-aachen.de" && Machine != "lxcip17.physik.rwth-aachen.de" && Machine != "lxcip18.physik.RWTH-Aachen.de" && Machine != "lxcip19.physik.rwth-aachen.de" && Machine != "lxcip24.physik.RWTH-Aachen.de" && Machine != "lxcip25.physik.rwth-aachen.de" && Machine != "lxcip26.physik.RWTH-Aachen.de" && Machine != "lxcip27.physik.rwth-aachen.de" && Machine != "lxcip28.physik.rwth-aachen.de" && Machine != "lxcip29.physik.RWTH-Aachen.de" && Machine != "lxcip30.physik.RWTH-Aachen.de" && Machine != "lxcip31.physik.RWTH-Aachen.de" && Machine != "lxcip32.physik.RWTH-Aachen.de" && Machine != "lxcip34.physik.RWTH-Aachen.de" && Machine != "lxcip35.physik.RWTH-Aachen.de" && Machine != "lxcip50.physik.RWTH-Aachen.de" && Machine != "lxcip51.physik.RWTH-Aachen.de" && Machine != "lxcip52.physik.RWTH-Aachen.de" && Machine != "lxcip53.physik.RWTH-Aachen.de" && Machine != "lxcip54.physik.RWTH-Aachen.de" && Machine != "lxcip55.physik.rwth-aachen.de" && Machine != "lxcip56.physik.rwth-aachen.de" && Machine != "lxcip57.physik.rwth-aachen.de" && Machine != "lxcip58.physik.rwth-aachen.de" && Machine != "lxcip59.physik.rwth-aachen.de"
# log          = $(LogDirectory)/condor_$(CLUSTER)_$(PROCESS).log
# Output       = $(LogDirectory)/condor_$(CLUSTER)_$(PROCESS).out
# Error        = $(LogDirectory)/condor_$(CLUSTER)_$(PROCESS).err
# transfer_input_files = ___EXECUTABLE___
# transfer_output_files = *.root

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
