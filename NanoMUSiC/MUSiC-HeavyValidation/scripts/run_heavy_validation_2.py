#!/usr/bin/env python3

from multiprocessing import Pool
from typing import Any
from tqdm import tqdm
import toml
import argparse
import os
import subprocess
import shlex
import tempfile
from collections import defaultdict
from pprint import pprint
import time

years = ["2016APV", "2016", "2017", "2018"]

###################################################F
# THIS CODE IS FOR THE JET CLASS HEAVY VALIDATION #
###################################################


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        required=True,
        help='Task configuration (TOML) file, produced by "analysis_config_builder.py"',
    )
    parser.add_argument("-s", "--sample", help="Sample to be processed.")
    parser.add_argument("-y", "--year", help="Year to be processed.", required=True)
    parser.add_argument(
        "--all_data",
        help='Starts validation for all Data samples. Incompatible with "--sample" and "--all_mc".',
        action="store_true",
    )
    parser.add_argument(
        "--all_mc",
        help='Starts validation for all MC samples. Incompatible with "--sample" and "--all-data".',
        action="store_true",
    )
    parser.add_argument(
        "--all",
        help="Starts validation for all MC and data samples.",
        action="store_true",
    )
    parser.add_argument("-j", "--jobs", help="Pool size.", type=int, default=100)
    parser.add_argument(
        "-e", "--executable", help="Validation excutable.", default="heavy_validation"
    )
    parser.add_argument("--debug", help="print debugging info", action="store_true")
    parser.add_argument(
        "-p",
        "--savepath",
        help="Specify sub-directory of savepath. Saving files at /validation_outputs/[year]/[savepath]/files/.",
    )
    parser.add_argument(
        "-t",
        "--trigger",
        required=True,
        help="Specify triggers and trigger limits to be used. Format: 'HT1700,PT600' for HT/PT trigger with a lower threshold of 1700/600 GeV. Minimum values are HT1050 and PT500.",
    )
    parser.add_argument(
        "-tv",
        "--tovalidate",
        help="This argument specifies the classes for which validation plots are created. The argument should have the form classname1,classname2... with classnames of the form 'xJ+yBJ'/'xJ+yBJ+nJ'/''xJ+yBJ+X' for exclusice/jet-inclusive/inclusive classes. Include class name 'COUNTS' in the enumeration to also calculate the event counts (class inhabitation) for each class. Instead of giving the class names manually one can also run a class config with '--classconfig'.",
    )
    parser.add_argument(
        "-cc",
        "--classconfig",
        help="Class configuration (TOML) file containing the names of the classes to be validated.",
    )

    args = parser.parse_args()

    # quality control
    argcount = 0
    if args.all_data:
        argcount += 1
    if args.all_mc:
        argcount += 1
    if args.all:
        argcount += 1
    if args.sample:
        argcount += 1
    if argcount != 1:
        raise RuntimeError(
            'ERROR: Could not start validation. "--all_data" is incompatible with "--all_mc" and "--sample" and "--all".'
        )
    if (args.classconfig and args.tovalidate) or (
        not args.classconfig and not args.tovalidate
    ):
        raise RuntimeError(
            'ERROR: Either "--classconfig" or "--tovalidate is required to specify the classes to be validated".'
        )
    return args


def merge_cutflow_histograms(
    process, year, output_path, input_files, debug: bool = False
):
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
        raise RuntimeError(f"ERROR: could not merge cutflow files.\n{error}")


# subfunction that calls the validation C file with the specified arguments
def run_validation(
    process_name: str,
    year: str,
    is_data: bool,
    output_path: str,
    effective_x_section: float,
    executable: str,
    input_file: str,
    trigger: str,
    tvarg: str,
) -> bool:
    debug: bool = False

    # default is MC
    cmd_str: str = f"{executable} --process {process_name} --year {year} --output {output_path} --xsection {str(effective_x_section)} --input {input_file} --trigger {trigger} --tovalidate {tvarg}"
    if is_data:
        cmd_str: str = f"{executable} --process {process_name} --year {year} --is_data --output {output_path} --xsection {str(effective_x_section)} --input {input_file} --trigger {trigger} --tovalidate {tvarg}"

    if debug:
        print(f"Executing: {cmd_str}")

    validation_result = subprocess.run(
        shlex.split(cmd_str),
        capture_output=True,
    )
    if debug:
        print(validation_result.stdout.decode("utf-8"))

    if validation_result.returncode != 0:
        error = validation_result.stderr.decode("utf-8")
        output = validation_result.stdout.decode("utf-8")
        raise RuntimeError(
            f"ERROR: could process validation.\n{error}\n{output}\n{input_file}"
        )
    return True


def make_processed_events():
    with open("dummy_processed_events.bin", mode="wb") as f:
        pass
    return "dummy_processed_events.bin"


def dump_list_to_temp_file(list_of_files):
    # Create a temporary file
    with tempfile.NamedTemporaryFile(mode="w", delete=False) as temp_file:
        # Write each list entry to a separate line in the file
        for item in list_of_files:
            temp_file.write(str(item) + "\n")

    # Return the path of the temporary file
    return temp_file.name


# runs validation process for one sample, it also does the rescaling
def validation(args):
    (
        process,
        year,
        luminosity,
        is_data,
        xsection,
        filter_eff,
        k_factor,
        input_files,
        executable,
        savepath,
        trigger,
        tvarg,
    ) = list(args.values())

    output_path = f"validation_outputs/{year}/files"
    if savepath != "":
        output_path = f"validation_outputs/{year}/{savepath}/files"  # option: save inside another directory of the output path

    # print("[ MUSiC Validation ] Loading samples ...\n")
    effective_x_section: float = 1.0
    if not is_data:
        effective_x_section = xsection * filter_eff * k_factor * luminosity

    # print("[ MUSiC Validation ] Preparing output directory ...\n")
    if savepath != "":
        os.system(
            f"rm -rf validation_outputs/{year}/{savepath}/files/*_{process}_{year}.root"
        )
    else:
        os.system(f"rm -rf validation_outputs/{year}/files/*_{process}_{year}.root")

    # print("[ MUSiC Validation ] Merging cutflow histograms ...\n")
    merge_cutflow_histograms(process, year, output_path, input_files)

    # print("[ MUSiC Validation ] Starting validation ...\n")
    inputs = dump_list_to_temp_file(input_files)
    # call the actual validation process
    run_validation(
        process,
        year,
        is_data,
        output_path,
        effective_x_section,
        executable,
        inputs,
        trigger,
        tvarg,
    )
    os.system(f"rm -rf {inputs} > /dev/null")

    # remove the cutflow files
    if savepath != "":
        os.system(
            f"rm -rf validation_outputs/{year}/{savepath}/files/cutflow_{process}_{year}.root"
        )
    else:
        os.system(
            f"rm -rf validation_outputs/{year}/files/cutflow_{process}_{year}.root"
        )


def get_year_era(process_name):
    process_name_components = (
        process_name.replace("-HIPM", "")
        .replace("_HIPM", "")
        .replace("-ver1", "")
        .replace("-ver2", "")
        .split("_")
    )
    return process_name_components[-2], process_name_components[-1]


def create_arguments(
    configuration,
    year,
    lumi,
    ismc,
    executable,
    savepath,
    trigger,
    tvarg,
    validation_arguments,
):
    for sample in configuration:
        if ismc:  # generate mc argument
            if f"das_name_{year}" in configuration[sample].keys():
                validation_arguments.append(
                    {
                        "process": sample,
                        "year": year,
                        "luminosity": lumi[year],
                        "is_data": configuration[sample]["is_data"],
                        "xsection": configuration[sample]["XSec"],
                        "filter_eff": configuration[sample]["FilterEff"],
                        "k_factor": configuration[sample]["kFactor"],
                        "input_files": list(
                            filter(
                                lambda file: f"{year}_date" in file,
                                configuration[sample]["output_files"],
                            )
                        ),
                        "executable": executable,
                        "savepath": savepath,
                        "trigger": trigger,
                        "tovalidate": tvarg,
                    }
                )
        else:  # generate data argument
            validation_arguments.append(
                {
                    "process": sample,
                    "year": year,
                    "luminosity": 1,
                    "is_data": True,
                    "xsection": 1,
                    "filter_eff": 1,
                    "k_factor": 1,
                    "input_files": configuration[sample]["output_files"],
                    "executable": executable,
                    "savepath": savepath,
                    "trigger": trigger,
                    "tovalidate": tvarg,
                }
            )
    return validation_arguments


def main():
    print("\n\n📶 [ MUSiC Heavy Validation 2 ] 📶\n")

    # parse arguments
    args = parse_args()

    # option: save in output directory "savepath" inside of year directory
    savepath = ""
    if args.savepath:
        savepath = args.savepath

    # specify trigger: pass string in the format "HT1600,PT600" to heavy_validation
    trigger = ""
    if args.trigger:
        trigger = args.trigger

    # import task config file that includes references to all files that should be validated
    print(f"Importing task config...")
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    # extract data and mc samples given in task config file
    mcconfig, dataconfig = {}, {}
    print(f"Extracting samples from task config...")
    for sample in task_config:
        if sample != "Lumi" and sample != "Global":
            if not task_config[sample]["is_data"]:  # mc case
                mcconfig.update({sample: task_config[sample]})
            else:  # data case
                dataconfig.update({sample: task_config[sample]})
    print(
        "Found",
        len(mcconfig),
        "mc samples and",
        len(dataconfig),
        "data samples in the selected task config.",
    )

    # extract lumi
    lumi = task_config["Lumi"]

    # generate mc and data sample list
    mcsamples = [i for i in mcconfig]
    datasamples = [i for i in dataconfig]

    # specify classes to be plotted in JetClass mode
    tvarg = ""  # default: no plots only count class inhabitation
    if args.tovalidate:  # manual specification of the classes that should be validated
        tvarg = args.tovalidate
    if (
        args.classconfig
    ):  # import class config to parse classes that should be validated
        print(f"Importing class config...")
        class_config_file: str = args.classconfig
        class_config: dict[str, Any] = toml.load(class_config_file)
        to_validate = class_config["to_validate"]
        for i in range(
            len(to_validate)
        ):  # create tovalidate argument from class config
            if i < len(to_validate) - 1:
                tvarg += to_validate[i] + ","
            if i == len(to_validate) - 1:
                tvarg += to_validate[i]

    # generate validation arguments for the different cases
    validation_arguments = []
    print(f"Generating validation arguments...")
    # run all mc files in task config
    if args.all_mc and args.year:
        validation_arguments = create_arguments(
            mcconfig,
            args.year,
            lumi,
            True,
            args.executable,
            savepath,
            trigger,
            tvarg,
            validation_arguments,
        )
    # run all data files in task config
    elif args.all_data and args.year:
        validation_arguments = create_arguments(
            dataconfig,
            args.year,
            lumi,
            False,
            args.executable,
            savepath,
            trigger,
            tvarg,
            validation_arguments,
        )
    # run one sample from task config
    elif args.sample and args.year:
        ismcsample = False  # default data sample
        if args.sample in mcsamples:
            ismcsample = True
        validation_arguments = create_arguments(
            {args.sample: task_config[args.sample]},
            args.year,
            lumi,
            ismcsample,
            args.executable,
            savepath,
            trigger,
            tvarg,
            validation_arguments,
        )
    # run all samples in the config
    elif args.all and args.year:
        validation_arguments = create_arguments(
            mcconfig,
            args.year,
            lumi,
            True,
            args.executable,
            savepath,
            trigger,
            tvarg,
            validation_arguments,
        )
        validation_arguments = create_arguments(
            dataconfig,
            args.year,
            lumi,
            False,
            args.executable,
            savepath,
            trigger,
            tvarg,
            validation_arguments,
        )

    # create output directory if not existing
    print(f"Checking output directory...")
    if not args.savepath:
        if not (os.path.isdir(f"validation_outputs/{args.year}/files")):
            os.system(f"mkdir -p validation_outputs/{args.year}/files")
    if savepath != "":
        if not (os.path.isdir(f"validation_outputs/{args.year}/{savepath}/files")):
            os.system(f"mkdir -p validation_outputs/{args.year}/{savepath}/files")

    # run validation jobs with the generated arguments
    print(f"Starting {len(validation_arguments)} validation jobs...")
    with Pool(min(args.jobs, len(validation_arguments))) as pool:
        list(
            tqdm(
                pool.imap_unordered(validation, validation_arguments),
                total=len(validation_arguments),
                unit="sample",
            )
        )

    # exit after performing all jobs
    print(f"Finished {len(validation_arguments)} validation jobs.\n")
    exit(0)


if __name__ == "__main__":
    main()
