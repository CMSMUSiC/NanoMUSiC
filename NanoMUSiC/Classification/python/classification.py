from typing import Optional, Union, Iterator
import time
import math
from multiprocessing.pool import AsyncResult
import fnmatch
from multiprocessing import Pool
from itertools import product
import glob
import random
import json
from condor_manager import CondorJob, CondorManager
import os
import classification_imp as clft
import sys
from metadata import Lumi, Years, Process, load_toml, get_distributions
from numpy import dot
from rich.progress import Progress
from rich import print as rprint
import hashlib
import subprocess
import shlex
from enum import Enum, auto
from collections import defaultdict
import multiprocessing


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
    max_files: int = sys.maxsize,
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
        year,
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

    output_path = (
        f"classification_jobs/run_classification_{process.name}_{year}_{split_index}.py"
    )
    with open(output_path, "w") as f:
        f.write(template)


def launch_condor(
    config_file_path: str,
    process_name: Union[str, None] = None,
    year: Union[Years, None] = None,
    max_files: int = sys.maxsize,
    split_size: int = sys.maxsize,
    dry_run: bool = False,
    skip_tar: bool = False,
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
        os.system("rm -rf classification_jobs && mkdir -p classification_jobs")
        os.system("rm -rf classification_src.tar.gz")

    if not skip_tar:
        os.system(
            "tar --exclude-from=$MUSIC_BASE/.gitignore --exclude=build --exclude=.git --exclude=Legacy --exclude=opt --exclude=bin --exclude=lib -czvf classification_src.tar.gz -C $MUSIC_BASE ."
        )

    preamble = [
        r"source /cvmfs/sft.cern.ch/lcg/views/LCG_106/x86_64-el9-gcc13-opt/setup.sh",
        r"cd $_CONDOR_SCRATCH_DIR",
        r"tar -zxf classification_src.tar.gz",
        r"mkdir -p lib",
        r"mkdir -p opt",
        r"mkdir -p bin",
        r"source setenv.sh",
        r"mkdir -p build",
        r"cd build",
        r"cmake ..",
        r"ninja classification_imp",
        r"mkdir -p ../lib/python",
        r"cp NanoMUSiC/Classification/classification_imp.cpython-39-x86_64-linux-gnu.so ../lib/python/.",
        r"cd ..",
    ]

    def chunks(lst: list[str], n: Union[int, None]) -> list[list[str]]:
        if not n:
            return [lst]
        if split_size > len(lst):
            return [lst]
        assert n > 0
        return [lst[i : i + n] for i in range(0, len(lst), n)]

    def request_memory(process: Process) -> int:
        if process.name.startswith("tt") or process.name.startswith("TT"):
            return 5
        if process.name.startswith("ggZH_HToBB_ZToNuNu_M-125_13TeV_PH"):
            return 4
        if process.name.startswith("QCD"):
            return 2
        if process.is_data:
            return 2
        return 3

    def year_filter(y: str) -> bool:
        if year:
            return y in year
        return True

    jobs = []
    for p in processes:
        for this_year in filter(year_filter, Years):
            input_files = chunks(p.get_files(this_year, max_files), split_size)
            for split_index, sub_input_files in enumerate(input_files):
                if len(sub_input_files):
                    build_classification_job(
                        p, this_year, split_index, sub_input_files, max_files
                    )
                    jobs.append(
                        CondorJob(
                            f"{p.name}_{this_year}_{split_index}",
                            actions=[
                                f'echo "--- {p.name} {this_year}"',
                                r"mkdir -p classification_outputs",
                                r"ls -lha",
                                f"python3 run_classification_{p.name}_{this_year}_{split_index}.py",
                                f"mv {p.name}_{this_year}_{split_index}*.root classification_outputs/.",
                                r"ls -lha classification_outputs",
                            ],
                            preamble=preamble,
                            input_files=[
                                f"classification_jobs/run_classification_{p.name}_{this_year}_{split_index}.py",
                                "sum_weights.json",
                                r"classification_src.tar.gz",
                            ],
                            output_files=[r"classification_outputs"],
                            request_memory=request_memory(p),
                        )
                    )

    if not dry_run:
        manager = CondorManager(jobs)
        manager.submit()
        manager.nanny()


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
    launch_condor(
        config_file,
        process_name,
        year,
        max_files,
        split_size,
        True,
        True,
        do_cleanning,
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

    for y in years_to_process:
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
            random.shuffle(generated_jobs)
            for j in generated_jobs:
                if y in j:
                    file.write("../{}\n".format(j))

        parallel_cmd = r"mkdir -p classification_outputs && cd classification_outputs && cp ../sum_weights.json . && /usr/bin/cat ../classification_jobs/inputs_parallel.txt | parallel -j ___NUM_CPUS___ --halt now,fail=1 --eta --progress --noswap --retries 4 --joblog job____YEAR___.log 'python3 {} > {/.}.stdout 2> {/.}.stderr' && cd ..".replace(
            "___NUM_CPUS___", str(num_cpus)
        ).replace("___YEAR___", y)
        rprint(
            "[bold magenta]Parallel command for year {}: [/bold magenta][white]{}[/white]".format(
                y, parallel_cmd
            )
        )

        os.system("date")
        parallel_proc = subprocess.Popen(
            parallel_cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            shell=True,
        )

        print()
        max_line_size = 0
        for line in iter(parallel_proc.stdout.readline, ""):
            if line.startswith("ETA"):
                line = line.replace("\n", "")
                max_line_size = max(max_line_size, len(line))
                print(" " * max_line_size, end="\r")
                print(line.replace("\n", ""), end="\r")
                sys.stdout.flush()  # Ensure it prints in real-time
            else:
                print(line, end="")
        print()

        parallel_proc.stdout.close()
        return_code = parallel_proc.wait()
        os.system("date")
        if return_code:
            print(
                "ERROR: Could not run parallel jobs for year {}.".format(y),
                file=sys.stderr,
            )
            sys.exit(-1)

        parallel_proc = subprocess.run(
            r"awk '$7 != 0' classification_outputs/job____YEAR___.log | wc -l".replace(
                "___YEAR___", y
            ),
            capture_output=True,
            shell=True,
            text=True,
        )
        if parallel_proc.returncode != 0:
            print(
                "ERROR: Could not status of parallel jobs for year {}.".format(y),
                file=sys.stderr,
            )
            sys.exit(-1)
        if parallel_proc.stdout.replace("\n", "").replace(" ", "") != "1":
            print(
                "ERROR: Could not run parallel jobs for year {}. At least one job have failed.".format(
                    y
                ),
                file=sys.stderr,
            )
            sys.exit(-1)


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


def merge_task(args):
    files_to_merge, validation_files_to_merge, process, year = args
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
    config_file_path: str,
    inputs_dir: str,
):
    config_file = load_toml(config_file_path)

    processes = [
        Process(name=process_name, **config_file[process_name])
        for process_name in config_file
    ]

    os.system("rm -rf classification_merged_results")
    os.system("mkdir -p classification_merged_results")
    os.system("rm -rf validation_merged_results")
    os.system("mkdir -p validation_merged_results")
    # os.system("rm -rf /tmp/ray")

    def do_merge(year: Years):
        merge_jobs = []
        for process in processes:
            files_to_merge = glob.glob(
                "{}/{}_{}_*.root".format(inputs_dir, process.name, year)
            )
            validation_files_to_merge = glob.glob(
                "{}/validation_{}_{}_*.root".format(inputs_dir, process.name, year)
            )

            if len(files_to_merge):
                merge_jobs.append(
                    (files_to_merge, validation_files_to_merge, process, year)
                )

        random.shuffle(merge_jobs)

        print("Merging Classification results for {}...".format(year))
        with Pool(30) as p:
            with Progress() as progress:
                task = progress.add_task(
                    "Merging {} ...".format(year), total=len(merge_jobs)
                )
                for job in p.imap_unordered(merge_task, merge_jobs):
                    progress.console.print("Done: {}".format(job))
                    progress.advance(task)

    for year in Years:
        do_merge(year)


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
    # os.system("rm -rf /tmp/ray")

    serialization_jobs = []
    for process, year in product(processes, Years):
        file_to_process = "{}/{}_{}.root".format(inputs_dir, process.name, year)
        validation_files_to_process = "{}/validation_{}_{}.root".format(
            validation_inputs_dir, process.name, year
        )
        if os.path.exists(file_to_process):
            serialization_jobs.append(
                (file_to_process, validation_files_to_process, process, year)
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


def do_fold(input_files, output_dir, classes_names):
    clft.Distribution.make_distributions(
        input_files,
        output_dir,
        classes_names,
    )


def make_distributions(
    inputs_dir: str,
    validation_inputs_dir: str,
    class_name_filter_patterns: list[str],
    validation_filter_patterns: list[str],
) -> None:
    with open("{}/classes_to_files.json".format(inputs_dir), "r") as file:
        classes_to_files = json.load(file)

    with open("{}/validation_to_files.json".format(validation_inputs_dir), "r") as file:
        validation_to_files = json.load(file)

    os.system("rm -rf classification_distributions")
    os.system("mkdir -p classification_distributions")
    os.system("rm -rf validation_distributions")
    os.system("mkdir -p validation_distributions")

    def get_input_files(inputs_dir: str) -> list[str]:
        return glob.glob("{}/*.root".format(inputs_dir))

    def filter_classes(analysis_name: str, patterns: list[str]) -> bool:
        return any(fnmatch.fnmatch(analysis_name, pattern) for pattern in patterns)

    def get_analysis_names(
        analysis_to_files: dict[str, list[str]], patterns: list[str]
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

    def chunk_list(lst, n_parts):
        if n_parts <= 0:
            print("ERROR: n_parts should be > 0.")
            sys.exit(-1)

        chunk_size = math.ceil(len(lst) / n_parts)
        for i in range(0, len(lst), chunk_size):
            yield lst[i : i + chunk_size]

    print("Will fold Classification ...")
    n_parts = 3
    classes_names = get_analysis_names(classes_to_files, class_name_filter_patterns)
    if classes_names:
        random.shuffle(classes_names)
        input_files = get_input_files("classification_root_files")
        random.shuffle(input_files)
        for this_classes_names in chunk_list(classes_names, n_parts):
            p = multiprocessing.Process(
                target=do_fold,
                args=(
                    input_files,
                    "classification_distributions",
                    this_classes_names,
                ),
            )
            p.start()
            p.join()
            if p.exitcode != 0:
                print("ERROR: Could not make distribution files for Classification.")

    print("Will fold Validation ...")
    validation_names = get_analysis_names(
        validation_to_files, validation_filter_patterns
    )
    if validation_names:
        p = multiprocessing.Process(
            target=do_fold,
            args=(
                get_input_files("validation_root_files"),
                "validation_distributions",
                validation_names,
            ),
        )
        p.start()
        p.join()
        if p.exitcode != 0:
            print("ERROR: Could not make distribution files for Validation.")
