import fnmatch
import glob
import hashlib
import json
import multiprocessing
import os
import random
import shlex
import subprocess
import sys
import time
from collections import defaultdict
from concurrent.futures import ProcessPoolExecutor, as_completed
from enum import Enum, auto
from itertools import product
from multiprocessing import Pool, process
from multiprocessing.pool import AsyncResult
from typing import Any, Iterator, Optional, Union

import classification_imp as clft
from metadata import Lumi, Process, Years, load_toml
from parallel_resume import parallel_resume_loop
from pydantic import BaseModel
from rich import print as rprint
from rich.progress import Progress


class XrdcpResult(Enum):
    OK = auto()
    FAILED = auto()
    TIMEOUT = auto()


def xrdcp(
    src: str, dest: str, redirector: str, timeout: Union[int, None]
) -> XrdcpResult:
    if not redirector.endswith("//"):
        redirector += "//"
    try:
        download_proc = subprocess.run(
            shlex.split(
                "xrdcp -f {}{} {}".format(redirector, src, dest),
            ),
            capture_output=True,
            timeout=timeout,
            text=True,
        )
        if download_proc.returncode != 0:
            return XrdcpResult.FAILED

        return XrdcpResult.OK

    except subprocess.TimeoutExpired:
        return XrdcpResult.TIMEOUT


def fetch_file(file):
    hashed_file_name = "root_files_buffer/{}.root".format(
        hashlib.sha256(file.encode("utf-8")).hexdigest()
    )

    print("Downloading input file {} ...".format(file))
    print("Will try from RWTH")
    xrdcp_res = xrdcp(
        file,
        hashed_file_name,
        "root://grid-dcache.physik.rwth-aachen.de//",
        360,
    )

    if xrdcp_res != XrdcpResult.OK:
        print("RWTH failed. Will try from Europe/Asia redirector.")
        xrdcp_res = xrdcp(
            file,
            hashed_file_name,
            "root://xrootd-cms.infn.it//",
            360,
        )

        if xrdcp_res != XrdcpResult.OK:
            print("Europe/Asia failed. Will try from Americas redirector.")
            xrdcp_res = xrdcp(
                file,
                hashed_file_name,
                "root://cmsxrootd.fnal.gov//",
                360,
            )

            if xrdcp_res != XrdcpResult.OK:
                print("Americas failed. Will try from Global redirector.")
                xrdcp_res = xrdcp(
                    file,
                    hashed_file_name,
                    "root://cms-xrd-global.cern.ch//",
                    None,
                )

                if xrdcp_res != XrdcpResult.OK:
                    return None

    return hashed_file_name


def download_files(files: list[str]) -> Iterator[str]:
    if len(files) == 0:
        print("ERROR: No input file was provided.")
        sys.exit(-1)

    if not files[0].startswith("/store"):
        for f in files:
            yield f
    else:
        with Pool(processes=1) as pool:
            result: Union[AsyncResult, None] = None
            for i, f in enumerate(files):
                this_file = None
                if i == 0:
                    if len(files) > 1:
                        result = pool.apply_async(fetch_file, (files[1],))
                    this_file = fetch_file(f)
                else:
                    if isinstance(result, AsyncResult):
                        result.wait()
                        if result.successful():
                            this_file = result.get()
                        if i + 1 < len(files):
                            result = pool.apply_async(fetch_file, (files[i + 1],))
                if not this_file:
                    print("ERROR: Could not download input file {}.".format(f))
                    sys.exit(-1)
                yield this_file


def run_classification(
    output_file: str,
    process: str,
    year: str,
    is_data: bool,
    x_section: float,
    filter_eff: float,
    k_factor: float,
    luminosity: float,
    xs_order: str,
    process_group: str,
    sum_weights_json_filepath: str,
    input_files: list[str],
    generator_filter: str,
    first_event: Optional[int] = None,
    last_event: Optional[int] = None,
    debug: bool = False,
) -> None:
    if not output_file.endswith(".root"):
        print(
            "ERROR: Invalid output file name.",
            file=sys.stderr,
        )
        sys.exit(-1)

    os.system("mkdir -p root_files_buffer")
    for idx, _file in enumerate(download_files(input_files)):
        # this is failed tentative to preprocess the MC files, filtering by trigger and reducing the amount of stored branches.
        # there is not enough local storage in the music machine to do so. Also, sending to dCache would take forever.
        # if not is_data:
        #     _file = skim(_file, process, year, is_dev_job)

        event_classes = clft.EventClassContainer()

        if is_data:
            process_group = "Data"
            xs_order = "DUMMY"
        validation_container = clft.ValidationContainer(
            process_group, xs_order, process, year
        )

        clft.classification(
            process,
            year,
            is_data,
            x_section,
            filter_eff,
            k_factor,
            luminosity,
            xs_order,
            process_group,
            sum_weights_json_filepath,
            _file,
            generator_filter,
            event_classes,
            validation_container,
            first_event,
            last_event,
            # 100,
            debug,
        )
        _output_file = "{}_{}.root".format(output_file.split(".root")[0], idx)

        if input_files[0].startswith("/store"):
            os.system("rm {}".format(_file))

        print("Saving output file: {}.".format(_output_file))
        clft.EventClassContainer.save(
            event_classes, "{}_{}".format(process, year), _output_file
        )

        print("Saving validation output file: validation_{}.".format(_output_file))
        clft.ValidationContainer.save(
            validation_container, "validation_{}".format(_output_file)
        )
        print("Done.")


def build_classification_job(
    process: Process,
    year: Years,
    split_index: int,
    sub_input_files: list[str],
):
    template = r"""
import classification

classification.run_classification(
    output_file="{}",
    process="{}",
    year="{}",
    is_data={},
    x_section={},
    filter_eff={},
    k_factor={},
    luminosity={},
    xs_order="{}",
    process_group="{}",
    sum_weights_json_filepath="sum_weights.json",
    input_files={},
    generator_filter="{}",
    first_event=None,
    last_event=None,
    debug=False,
)

print("YAY! Done _o/")

    """.format(
        f"{process.name}_{year}_{split_index}.root",
        process.name,
        year.value,
        process.is_data,
        process.XSec,
        process.FilterEff,
        process.kFactor,
        Lumi.lumi[year],
        process.XSecOrder,
        process.ProcessGroup,
        sub_input_files,
        process.generator_filter_key,
    )

    output_path = f"classification_jobs/run_classification_{process.name}_{year.value}_{split_index}.py"
    with open(output_path, "w") as f:
        f.write(template)


def prepare_classification(
    *,
    config_file_path: str,
    process_name: Union[str, None] = None,
    year: Union[Years, None] = None,
    max_files: int = sys.maxsize,
    split_size: int = sys.maxsize,
    do_cleanning: bool = True,
):
    config_file = load_toml(config_file_path)

    processes = [
        Process(name=process, **config_file[process]) for process in config_file
    ]
    if process_name:
        processes = list(filter(lambda proc: proc.name == process_name, processes))
    random.shuffle(processes)

    def expected_timming(process: Process) -> int:
        if process.name.startswith("tt") or process.name.startswith("TT"):
            return 0
        if process.name.startswith("QCD"):
            return 100
        if process.is_data:
            return sys.maxsize
        return 10

    processes = sorted(processes, key=expected_timming)

    if do_cleanning:
        os.system("rm -rf classification_outputs")
        os.system("rm -rf classification_jobs")
    os.system("mkdir -p classification_jobs")

    def chunks(lst: list[str], n: Union[int, None]) -> list[list[str]]:
        if not n:
            return [lst]
        if split_size > len(lst):
            return [lst]
        assert n > 0
        return [lst[i : i + n] for i in range(0, len(lst), n)]

    def year_filter(y: str) -> bool:
        if year:
            return y in year
        return True

    for p in processes:
        for this_year in filter(year_filter, Years):
            input_files = chunks(p.get_files(this_year, max_files), split_size)
            for split_index, sub_input_files in enumerate(input_files):
                if len(sub_input_files):
                    build_classification_job(p, this_year, split_index, sub_input_files)


def launch_parallel(
    config_file: str,
    process_name: Union[str, None] = None,
    year: Union[Years, None] = None,
    max_files: int = sys.maxsize,
    split_size: int = sys.maxsize,
    num_cpus: int = 120,
):
    do_cleanning = True
    if year or process_name:
        do_cleanning = False

    # launch a dry-run of a condor classification
    prepare_classification(
        config_file_path=config_file,
        process_name=process_name,
        year=year,
        max_files=max_files,
        split_size=split_size,
        do_cleanning=do_cleanning,
    )

    years_to_process = ["2016", "2017", "2018"]

    if year:
        if year == "2016" or year == "2017" or year == "2018":
            years_to_process = [year]
        else:
            print(
                "ERROR: Could not launch Classification. The requested year ({}) is not valid.".format(
                    year
                ),
                file=sys.stderr,
            )
            sys.exit(-1)
    if not process_name:
        process_name = "*"

    with open("classification_jobs/inputs_parallel.txt", "w") as file:
        generated_jobs = glob.glob(
            os.path.join(
                "classification_jobs",
                "*{}_{}*.py".format(process_name, years_to_process),
            )
        )
        if len(generated_jobs) == 0:
            print(
                "ERROR: Could not launch Classification. The requested combination of sample ({}) and year ({}) is not valid.".format(
                    process_name, year
                ),
                file=sys.stderr,
            )
            sys.exit(-1)

        # random.shuffle(generated_jobs)
        generated_jobs = sorted(generated_jobs, reverse=True)
        generated_jobs = sorted(
            generated_jobs, key=lambda j: "classification_DYJetsToLL" not in j
        )
        generated_jobs = sorted(
            generated_jobs, key=lambda j: "classification_T" not in j
        )
        generated_jobs = sorted(
            generated_jobs, key=lambda j: "classification_TTT" not in j
        )
        generated_jobs = sorted(
            generated_jobs, key=lambda j: "classification_TTTo2L" not in j
        )
        generated_jobs = sorted(
            generated_jobs, key=lambda j: "classification_TTTT" not in j
        )
        generated_jobs = sorted(
            generated_jobs, key=lambda j: "classification_TTZ" not in j
        )

        for j in generated_jobs:
            file.write("../{}\n".format(j))

    # start GNU parallel
    os.system("date")
    parallel_resume_loop(
        joblog_path="classification_outputs/job.log",
        parallel_command="cd classification_outputs && parallel --joblog job.log --resume-failed --memfree 50G -j ___NUM_CPUS___ --eta --progress --noswap --retry-failed 'python3 {} > {/.}.stdout 2> {/.}.stderr' :::: ../classification_jobs/inputs_parallel.txt".replace(
            "___NUM_CPUS___", str(num_cpus)
        ),
        max_attempts=10,
        wait_between_attempts=30,
        max_runtime_hours=24,
        preamble="mkdir -p classification_outputs && cd classification_outputs && cp -r ../btag_eff_maps . && cp ../sum_weights.json .",
        epilog=None,
    )

    os.system("cp classification_outputs/job.log last_parallel_job.log")


def launch_dev(
    config_file_path: str,
    process_name: Union[str, None] = None,
    year: Union[Years, None] = None,
    max_files: int = sys.maxsize,
):
    if not process_name or not year:
        print(
            "ERROR: Dev target, only supported if specifying Process name and Year.",
            file=sys.stderr,
        )
        sys.exit(-1)

    config_file = load_toml(config_file_path)
    process = Process(name=process_name, **config_file[process_name])

    run_classification(
        output_file=f"{process.name}_{year}.root",
        process=process.name,
        year=year,
        is_data=process.is_data,
        x_section=process.XSec,
        filter_eff=process.FilterEff,
        k_factor=process.kFactor,
        luminosity=Lumi.lumi[year],
        xs_order=process.XSecOrder,
        process_group=process.ProcessGroup,
        sum_weights_json_filepath="sum_weights.json",
        input_files=process.get_files(year, max_files),
        generator_filter=process.generator_filter_key,
        first_event=None,
        last_event=None,
        debug=True,
    )


def merge_task(files_to_merge, validation_files_to_merge, process, year):
    clft.EventClassContainer.merge_many(
        "{}_{}".format(process.name, year),
        files_to_merge,
        "classification_merged_results/{}_{}.root".format(process.name, year),
    )
    clft.ValidationContainer.merge_many(
        validation_files_to_merge,
        "validation_merged_results/validation_{}_{}.root".format(process.name, year),
    )
    return "{} - {}".format(process.name, year)


def merge_classification_outputs(
    *,
    config_file_path: str,
    inputs_dir: str,
    num_cpus: int = 124,
) -> None:
    config_file = load_toml(config_file_path)

    processes = [
        Process(name=process_name, **config_file[process_name])
        for process_name in config_file
    ]
    random.shuffle(processes)
    # merge_jobs = sorted(merge_jobs, reverse=True, key=lambda proc: proc[2].name)
    # merge_jobs = sorted(
    #     merge_jobs, key=lambda proc: "classification_DYJetsToLL" not in proc[2].name
    # )
    # merge_jobs = sorted(
    #     merge_jobs, key=lambda proc: "classification_T" not in proc[2].name
    # )
    # merge_jobs = sorted(
    #     merge_jobs, key=lambda proc: "classification_TTT" not in proc[2].name
    # )
    # merge_jobs = sorted(
    #     merge_jobs, key=lambda proc: "classification_TTTo2L" not in proc[2].name
    # )
    # merge_jobs = sorted(
    #     merge_jobs, key=lambda proc: "classification_TTTT" not in proc[2].name
    # )
    # merge_jobs = sorted(
    #     merge_jobs, key=lambda proc: "classification_TTZ" not in proc[2].name
    # )

    os.system("rm -rf classification_merged_results")
    os.system("mkdir -p classification_merged_results")
    os.system("rm -rf validation_merged_results")
    os.system("mkdir -p validation_merged_results")

    for year in Years:
        merge_jobs = []
        with ProcessPoolExecutor(max_workers=num_cpus) as executor:
            with Progress() as progress:
                task = progress.add_task(
                    f"Building merge jobs for {year.value}...", total=len(processes)
                )
                for process in processes:
                    files_to_merge = glob.glob(
                        "{}/{}_{}_*.root".format(inputs_dir, process.name, year.value)
                    )
                    validation_files_to_merge = glob.glob(
                        "{}/validation_{}_{}_*.root".format(
                            inputs_dir, process.name, year.value
                        )
                    )

                    if len(files_to_merge):
                        merge_jobs.append(
                            executor.submit(
                                merge_task,
                                files_to_merge,
                                validation_files_to_merge,
                                process,
                                year.value,
                            )
                        )
                    progress.advance(task)

            print(f"Merging Classification results for {year.value}...")
            with Progress() as progress:
                task = progress.add_task(
                    f"Merging for {year.value} ...", total=len(merge_jobs)
                )
                completed = 0
                for future in as_completed(merge_jobs):
                    progress.console.print(
                        f"Done: {future.result()} - {completed}/{len(merge_jobs)}"
                    )
                    completed += 1
                    progress.advance(task)


def serialize_to_root_task(args):
    file_to_process, validation_file_to_process, process, year = args
    event_classes = clft.EventClassContainer.serialize_to_root(
        file_to_process,
        "classification_root_files/{}_{}.root".format(process.name, year),
        process.name,
        process.ProcessGroup,
        process.XSecOrder,
        year,
        process.is_data,
    )
    validation_analysis = clft.ValidationContainer.serialize_to_root(
        validation_file_to_process,
        "validation_root_files/validation_{}_{}.root".format(process.name, year),
    )

    return (
        "{} - {}".format(process.name, year),
        event_classes,
        validation_analysis,
        "classification_root_files/{}_{}.root".format(process.name, year),
        "validation_root_files/validation_{}_{}.root".format(process.name, year),
    )


def serialize_to_root(
    config_file_path: str, inputs_dir: str, validation_inputs_dir: str, num_cpus: int
):
    config_file = load_toml(config_file_path)

    processes = [
        Process(name=process_name, **config_file[process_name])
        for process_name in config_file
    ]

    for process in processes:
        if process.is_data:
            process.ProcessGroup = "Data"
            process.XSecOrder = "DUMMY"

    os.system("rm -rf classification_root_files")
    os.system("mkdir -p classification_root_files")
    os.system("rm -rf validation_root_files")
    os.system("mkdir -p validation_root_files")

    serialization_jobs = []
    for process, year in product(processes, Years):
        file_to_process = "{}/{}_{}.root".format(inputs_dir, process.name, year.value)
        validation_files_to_process = "{}/validation_{}_{}.root".format(
            validation_inputs_dir, process.name, year.value
        )
        if os.path.exists(file_to_process):
            serialization_jobs.append(
                (file_to_process, validation_files_to_process, process, year.value)
            )
        else:
            if not process.is_data:
                print("WARINING: File ({}) does not exist.".format(file_to_process))

    random.shuffle(serialization_jobs)

    print("Unwrapping (serializing) Classification results to ROOT ...")
    classes_to_files = defaultdict(list)
    validation_to_files = defaultdict(list)
    with Pool(num_cpus) as p:
        with Progress() as progress:
            task = progress.add_task("Unwrapping ...", total=len(serialization_jobs))
            for job in p.imap_unordered(serialize_to_root_task, serialization_jobs):
                (
                    sample_year,
                    event_classes,
                    validation_analysis,
                    file_to_process,
                    validation_file_to_process,
                ) = job
                progress.console.print("Done: {}".format(sample_year))

                for ec in event_classes:
                    classes_to_files[ec].append(file_to_process.split("/")[1])
                for val in validation_analysis:
                    validation_to_files[val].append(
                        validation_file_to_process.split("/")[1]
                    )

                progress.advance(task)

    with open("classification_root_files/classes_to_files.json", "w") as json_file:
        json.dump(dict(classes_to_files), json_file, indent=4)

    with open("validation_root_files/validation_to_files.json", "w") as json_file:
        json.dump(dict(validation_to_files), json_file, indent=4)


def make_distributions_task(
    args: tuple[list[str], str, list[tuple[str, bool]], str, str],
):
    (
        files_to_process,
        analysis_name,
        distribution_props,
        year,
        output_file_path,
    ) = args

    start_time = time.time()
    clft.Distribution.make_distributions(
        files_to_process,
        analysis_name,
        year,
        distribution_props,
        output_file_path,
    )

    end_time = time.time()

    # clft.Distribution.save(distribution, output_file_path)

    # Calculate the elapsed time
    elapsed_time = end_time - start_time

    print(f"Elapsed time: {elapsed_time} seconds")

    return analysis_name, year


class MakeDistributionsInputs(BaseModel):
    input_file: str
    output_dir: str
    class_name: str
    skip_per_year: bool


def do_make_distributions(inputs: MakeDistributionsInputs) -> tuple[bool, str]:
    try:
        if clft.Distribution.make_distributions(
            inputs.input_file,
            inputs.output_dir,
            inputs.class_name,
            inputs.skip_per_year,
        ):
            return True, inputs.class_name

        return False, inputs.class_name
    except Exception as e:
        print("Exception: {}".format(e))
        return False, inputs.class_name


def do_fold(input_files: list[list[str]], output_dir: str):
    clft.Distribution.fold(input_files, output_dir)


def fold(
    inputs_dir: str,
    validation_inputs_dir: str,
) -> None:
    os.system("rm -rf classification_folded_files")
    os.system("mkdir -p classification_folded_files")
    os.system("rm -rf validation_folded_files")
    os.system("mkdir -p validation_folded_files")

    with open("{}/classes_to_files.json".format(inputs_dir), "r") as file:
        classes_to_files: dict[str, list[str]] = json.load(file)

    with open("{}/validation_to_files.json".format(validation_inputs_dir), "r") as file:
        validation_to_files: dict[str, list[str]] = json.load(file)

    def get_input_files(inputs_dir: str) -> list[str]:
        return glob.glob("{}/*.root".format(inputs_dir))

    print("Will fold Classification ...")
    classes_names = [name for name in classes_to_files.keys()]

    if classes_names:
        if not (
            clft.Distribution.fold(
                get_input_files(inputs_dir),
                "classification_folded_files",
            )
        ):
            print(
                "ERROR: Could not fold files for Classification.",
                file=sys.stderr,
            )
            sys.exit(-1)

    print("Will fold Validation ...")
    validation_names = [name for name in validation_to_files.keys()]

    if validation_names:
        if not clft.Distribution.fold(
            get_input_files(validation_inputs_dir),
            "validation_folded_files",
        ):
            print(
                "ERROR: Could not make distribution files for Validation.",
                file=sys.stderr,
            )
            sys.exit(-1)

    os.system(
        "cp {}/classes_to_files.json classification_folded_files/.".format(inputs_dir)
    )

    os.system(
        "cp {}/validation_to_files.json validation_folded_files/.".format(
            validation_inputs_dir
        )
    )


def make_distributions(
    inputs_dir: str,
    validation_inputs_dir: str,
    class_name_filter_pattern: str | None,
    validation_filter_pattern: str | None,
    skip_per_year: bool,
) -> None:
    with open("{}/classes_to_files.json".format(inputs_dir), "r") as file:
        classes_to_files = json.load(file)

    with open("{}/validation_to_files.json".format(validation_inputs_dir), "r") as file:
        validation_to_files = json.load(file)

    if class_name_filter_pattern:
        os.system("rm -rf classification_distributions")
        os.system("mkdir -p classification_distributions")

    if validation_filter_pattern:
        os.system("rm -rf validation_distributions")
        os.system("mkdir -p validation_distributions")

    def filter_classes(analysis_name: str, pattern: str) -> bool:
        return fnmatch.fnmatch(analysis_name, pattern)

    def get_analysis_names(
        analysis_to_files: dict[str, list[str]], patterns: str
    ) -> Union[list[str], None]:
        analysis_names = []
        for analysis in analysis_to_files:
            if filter_classes(analysis, patterns):
                analysis_names.append(analysis)
        if len(analysis_names):
            return analysis_names
        print(
            "WARNING: Could not build distribution jobs. No distributions matches the requirements."
        )
        return None

    def chunks(lst: list[str] | None, n: int = 4) -> list[list[str]] | None:
        if not lst:
            return None

        if len(lst) < 6000:
            return [lst]

        assert n > 0
        k, m = divmod(len(lst), n)
        return [lst[i * k + min(i, m) : (i + 1) * k + min(i + 1, m)] for i in range(n)]

    if class_name_filter_pattern:
        print("Will make Distributions for Classification ...")
        classes_names = get_analysis_names(classes_to_files, class_name_filter_pattern)

        if classes_names:
            distribution_jobs = [
                MakeDistributionsInputs(
                    input_file="{}/folded_histograms_{}.root".format(
                        inputs_dir, name.replace("+", "_")
                    ),
                    output_dir="classification_distributions",
                    class_name=name,
                    skip_per_year=skip_per_year,
                )
                for name in classes_names
            ]

            with Pool(min(multiprocessing.cpu_count(), len(distribution_jobs))) as p:
                with Progress() as progress:
                    task = progress.add_task(
                        "Making Classification distribution files [{}] ...".format(
                            len(distribution_jobs)
                        ),
                        total=len(classes_names),
                    )
                    for job in p.imap_unordered(
                        do_make_distributions, distribution_jobs
                    ):
                        status, analysis_name = job
                        if not status:
                            print(
                                "ERROR: Could not process event class: {}".format(
                                    analysis_name
                                ),
                                file=sys.stderr,
                            )
                            sys.exit(-1)

                        progress.console.print("Done: {}".format(analysis_name))
                        progress.advance(task)

    if validation_filter_pattern:
        print("Will make Distributions for Validation ...")
        validation_names = get_analysis_names(
            validation_to_files, validation_filter_pattern
        )

        if validation_names:
            distribution_jobs = [
                MakeDistributionsInputs(
                    input_file="{}/folded_histograms_{}.root".format(
                        validation_inputs_dir, name.replace("+", "_")
                    ),
                    output_dir="validation_distributions",
                    class_name=name,
                    skip_per_year=skip_per_year,
                )
                for name in validation_names
            ]

            with Pool(min(multiprocessing.cpu_count(), len(distribution_jobs))) as p:
                with Progress() as progress:
                    task = progress.add_task(
                        "Making Validation distribution files [{}] ...".format(
                            len(distribution_jobs)
                        ),
                        total=len(validation_names),
                    )
                    for job in p.imap_unordered(
                        do_make_distributions, distribution_jobs
                    ):
                        status, analysis_name = job
                        if not status:
                            print(
                                "ERROR: Could not process validation analysis: {}".format(
                                    analysis_name
                                ),
                                file=sys.stderr,
                            )
                            sys.exit(-1)

                        progress.console.print("Done: {}".format(analysis_name))
                        progress.advance(task)
