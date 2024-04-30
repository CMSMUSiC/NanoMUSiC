import ray
from itertools import product
import glob
import random
import json
import gzip
from condor_manager import CondorJob, CondorManager
import os
import classification_imp as clft
from typing import Optional, Union
import sys
from metadata import Lumi, Years, Process, load_toml
from rich.progress import track, Progress


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
    input_files: Union[list[str], str],
    generator_filter: str,
    first_event: Optional[int] = None,
    last_event: Optional[int] = None,
    debug: bool = False,
) -> None:
    if not (output_file.endswith(".music.gz")):
        print(
            'ERROR: Invalid output file name. Allowed extension: ".music.gz"',
            file=sys.stderr,
        )
        sys.exit(-1)

    for idx, _file in enumerate(input_files):
        event_classes = clft.EventClassContainer()
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
            first_event,
            last_event,
            debug,
        )
        _output_file = "{}_{}.music.gz".format(output_file.split(".music.gz")[0], idx)

        with gzip.open(_output_file, "wb") as f:
            f.write(clft.EventClassContainer.serialize(event_classes))


def build_classification_job(
    process: Process,
    year: Years,
    split_index: int,
    sub_input_files: list[str],
    max_files: int = sys.maxsize,
):
    template = r"""
import classification
import time
import json

# Record the start time
start_time = time.time()

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

# Record the end time
end_time = time.time()

# Calculate the time taken
time_taken = data = dict()
time_taken["process_name"] = "{}"
time_taken["year"] = "{}"
time_taken["split_index"] = {}
time_taken["num_input_files"] = {}
time_taken["processing_time"] = end_time - start_time

with open("timming_{}_{}_{}.json", "w") as json_file:
    json.dump(time_taken, json_file)


    """.format(
        f"{process.name}_{year}_{split_index}.music.gz",
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
        process.name,
        year,
        split_index,
        len(sub_input_files),
        process.name,
        year,
        split_index,
    )

    output_path = (
        f"classification_jobs/run_classification_{process.name}_{year}_{split_index}.py"
    )
    with open(output_path, "w") as f:
        f.write(template)


def launch_condor(
    config_file: str,
    process_name: Union[str, None] = None,
    year: Union[Years, None] = None,
    max_files: int = sys.maxsize,
    split_size: Union[int, None] = None,
    dry_run: bool = False,
):
    if process_name or year:
        print(
            "WARINING: Launching a Condor Classification will run over all samples. Process name and Year will be ignored.",
            file=sys.stderr,
        )

    config_file = load_toml(config_file)

    processes = [
        Process(name=process_name, **config_file[process_name])
        for process_name in config_file
    ]
    random.shuffle(processes)

    def expected_timming(process: Process) -> int:
        if process.name.startswith("TT"):
            return 0
        if process.name.startswith("QCD"):
            return 100
        if process.is_data:
            return sys.maxsize
        return 10

    processes = sorted(processes, key=expected_timming)

    os.system("rm -rf classification_outputs")
    os.system("rm -rf classification_jobs && mkdir -p classification_jobs")
    os.system("rm -rf classification_src.tar.gz")
    os.system(
        "tar --exclude-from=$MUSIC_BASE/.gitignore --exclude=build --exclude=.git --exclude=Legacy --exclude=opt --exclude=bin --exclude=lib -czvf classification_src.tar.gz -C $MUSIC_BASE ."
    )

    preamble = [
        r"source /cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/setup.sh",
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
        if process.name.startswith("TT"):
            return 6
        if process.name.startswith("ggZH_HToBB_ZToNuNu_M-125_13TeV_PH"):
            return 4
        if process.name.startswith("QCD"):
            return 2
        if process.is_data:
            return 2
        return 3

    jobs = []
    for p in processes:
        for year in Years:
            input_files = chunks(p.get_files(year, max_files), split_size)
            for split_index, sub_input_files in enumerate(input_files):
                if len(sub_input_files):
                    build_classification_job(
                        p, year, split_index, sub_input_files, max_files
                    )
                    jobs.append(
                        CondorJob(
                            f"{p.name}_{year}_{split_index}",
                            actions=[
                                f'echo "--- {p.name} {year}"',
                                r"mkdir -p classification_outputs",
                                r"ls -lha",
                                f"python3 run_classification_{p.name}_{year}_{split_index}.py",
                                f"mv {p.name}_{year}_{split_index}*.music.gz classification_outputs/.",
                                f"mv timming_{p.name}_{year}_{split_index}*.json classification_outputs/.",
                                r"ls -lha classification_outputs",
                            ],
                            preamble=preamble,
                            input_files=[
                                f"classification_jobs/run_classification_{p.name}_{year}_{split_index}.py",
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


def launch_dev(
    config_file: str,
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

    config_file = load_toml(config_file)
    process = Process(name=process_name, **config_file[process_name])

    run_classification(
        output_file=f"{process.name}_{year}.music.gz",
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


def launch_local(
    config_file: str,
    process_name: Union[str, None] = None,
    year: Union[Years, None] = None,
    max_files: int = sys.maxsize,
    num_cpus: int = 100,
    split_size: Union[int, None] = None,
):
    if process_name or year:
        print(
            "WARINING: Launching a Local Classification will run over all samples. Process name and Year will be ignored.",
            file=sys.stderr,
        )

    config_file = load_toml(config_file)

    processes = [
        Process(name=process_name, **config_file[process_name])
        for process_name in config_file
    ]
    random.shuffle(processes)

    def expected_timming(process: Process) -> int:
        if process.name.startswith("TT"):
            return 0
        if process.name.startswith("QCD"):
            return 100
        if process.is_data:
            return sys.maxsize
        return 10

    processes = sorted(processes, key=expected_timming)

    os.system("rm -rf classification_outputs")
    os.system("mkdir -p  classification_outputs")
    os.system("rm -rf ray_spill_objects")
    os.system("mkdir -p  ray_spill_objects")
    os.system("rm -rf tmp_ray")
    os.system("mkdir -p  tmp_ray")

    @ray.remote
    def run_classification_fwd(*args, **kwargs):
        return run_classification(*args, **kwargs)

    print("Starting ray cluster ...")
    ray.init(
        log_to_driver=False,
        num_cpus=num_cpus,
        _system_config={
            "object_spilling_config": json.dumps(
                {
                    "type": "filesystem",
                    "params": {
                        "directory_path": "{}/ray_spill_objects".format(os.getcwd())
                    },
                },
            )
        },
        _temp_dir="{}/tmp_ray".format(os.getcwd()),
    )

    def chunks(lst: list[str], n: Union[int, None]) -> list[list[str]]:
        if not n:
            return [lst]
        if split_size > len(lst):
            return [lst]
        assert n > 0
        return [lst[i : i + n] for i in range(0, len(lst), n)]

    print("Launching jobs...")
    jobs = []
    for process in processes:
        for year in Years:
            input_files = chunks(process.get_files(year, max_files), split_size)
            for split_index, sub_input_files in enumerate(input_files):
                if len(sub_input_files):
                    jobs.append(
                        run_classification_fwd.remote(
                            output_file=f"classification_outputs/{process.name}_{year}_{split_index}.music.gz",
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
                    )

    # ray.get(jobs)
    def to_iterator(obj_ids):
        while obj_ids:
            done, obj_ids = ray.wait(obj_ids)
            yield ray.get(done[0])

    print("Collecting results ...")
    for x in track(
        to_iterator(jobs),
        description="Processing {} jobs ...".format(len(jobs)),
        total=len(jobs),
    ):
        pass


@ray.remote
def merge_task(files_to_merge: list[str], process: Process, year: Years):
    with gzip.open(files_to_merge[0], "rb") as f:
        uncompressed_data = f.read()
    buffer = clft.EventClassContainer.deserialize(uncompressed_data)

    for f in files_to_merge[1:]:
        with gzip.open(f, "rb") as f:
            uncompressed_data = f.read()
        ec = clft.EventClassContainer.deserialize(uncompressed_data)
        buffer.merge_inplace(ec)

    # dump buffer to file
    with gzip.open(
        "merged_results/{}_{}.music.gz".format(process.name, year), "wb"
    ) as f:
        f.write(clft.EventClassContainer.serialize(buffer))

    return "{} - {}".format(process.name, year)


# tqdm for the merger
# https://github.com/ray-project/ray/issues/5554#issuecomment-558397627
def merge_classification_outputs(
    config_file: str,
    inputs_dir: str,
):
    config_file = load_toml(config_file)

    processes = [
        Process(name=process_name, **config_file[process_name])
        for process_name in config_file
    ]

    os.system("rm -rf merged_results")
    os.system("mkdir -p merged_results")
    # os.system("rm -rf /tmp/ray")

    print("Merging Classification results ...")
    merge_jobs = []
    for process, year in product(processes, Years):
        files_to_merge = glob.glob(
            "{}/{}_{}*.music.gz".format(inputs_dir, process.name, year)
        )
        if len(files_to_merge):
            merge_jobs.append(merge_task.remote(files_to_merge, process, year))

    def to_iterator(obj_ids):
        while obj_ids:
            done, obj_ids = ray.wait(obj_ids)
            yield ray.get(done[0])

    with Progress() as progress:
        task = progress.add_task("Merging ...", total=len(merge_jobs))
        for job in to_iterator(merge_jobs):
            progress.console.print("Done: {}".format(job))
            progress.advance(task)


@ray.remote
def serialize_to_root_task(file_to_process: str, process: Process, year: Years):
    with gzip.open(file_to_process, "rb") as f:
        uncompressed_data = f.read()
    ec = clft.EventClassContainer.deserialize(uncompressed_data)

    clft.EventClassContainer.serialize_to_root(
        ec,
        "classification_root_files/{}_{}.root".format(process.name, year),
        process.name,
        process.ProcessGroup,
        process.XSecOrder,
        year,
        process.is_data,
    )

    return "{} - {}".format(process.name, year)


def serialize_to_root(
    config_file: str,
    inputs_dir: str,
):
    config_file = load_toml(config_file)

    processes = [
        Process(name=process_name, **config_file[process_name])
        for process_name in config_file
    ]

    os.system("rm -rf classification_root_files")
    os.system("mkdir -p classification_root_files")
    # os.system("rm -rf /tmp/ray")

    print("Serializing Classification results to ROOT ...")
    serialization_jobs = []
    for process, year in product(processes, Years):
        file_to_process = "{}/{}_{}.music.gz".format(inputs_dir, process.name, year)
        if os.path.exists(file_to_process):
            serialization_jobs.append(
                serialize_to_root_task.remote(file_to_process, process, year)
            )
        else:
            print("WARINING: File ({}) does not exist.".format(file_to_process))

    def to_iterator(obj_ids):
        while obj_ids:
            done, obj_ids = ray.wait(obj_ids)
            yield ray.get(done[0])

    with Progress() as progress:
        task = progress.add_task("Serializing ...", total=len(serialization_jobs))
        for job in to_iterator(serialization_jobs):
            progress.console.print("Done: {}".format(job))
            progress.advance(task)


def main():
    os.system("rm -rf foo.musicz")

    input_files_1 = [
        # "davs://grid-webdav.physik.rwth-aachen.de:2889/store/user/ftorresd/nano_music_date_2024_03_08_time_15_29_22/crab_nano_music_DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM_2016APV/DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_TuneCP5_13TeV-amcatnloFXFX-pythia8/DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM_2016APV/240308_142938/0000/nano_music_7.root",
        "root://grid-dcache.physik.rwth-aachen.de///store/user/ftorresd/nano_music_date_2024_03_08_time_15_29_22/crab_nano_music_DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM_2016APV/DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_TuneCP5_13TeV-amcatnloFXFX-pythia8/DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM_2016APV/240308_142938/0000/nano_music_7.root",
        "root://grid-dcache.physik.rwth-aachen.de///store/user/ftorresd/nano_music_date_2024_03_08_time_15_29_22/crab_nano_music_DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM_2016APV/DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_TuneCP5_13TeV-amcatnloFXFX-pythia8/DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM_2016APV/240308_142938/0000/nano_music_8.root",
    ]
    input_files_2 = [
        "root://grid-dcache.physik.rwth-aachen.de///store/user/ftorresd/nano_music_date_2024_03_08_time_15_29_22/crab_nano_music_DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM_2016APV/DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_TuneCP5_13TeV-amcatnloFXFX-pythia8/DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM_2016APV/240308_142938/0000/nano_music_9.root",
    ]

    run_classification(
        output_file="foo_1.musicz",
        process="DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM",
        year="2016APV",
        is_data=False,
        x_section=84.97,
        filter_eff=1.0,
        k_factor=1.0,
        luminosity=19520.0,
        xs_order="NLO",
        process_group="DrellYan",
        sum_weights_json_filepath="sum_weights.json",
        input_files=input_files_1,
        generator_filter="",
        first_event=None,
        last_event=None,
        debug=True,
    )

    run_classification(
        output_file="foo_2.musicz",
        process="DYJetsToLL_LHEFilterPtZ-100To250_MatchEWPDG20_13TeV_AM",
        year="2016APV",
        is_data=False,
        x_section=84.97,
        filter_eff=1.0,
        k_factor=1.0,
        luminosity=19520.0,
        xs_order="NLO",
        process_group="DrellYan",
        sum_weights_json_filepath="sum_weights.json",
        input_files=input_files_2,
        generator_filter="",
        first_event=None,
        last_event=None,
        debug=True,
    )

    with gzip.open("foo_1.musicz", "rb") as f:
        uncompressed_data = f.read()
    print(f"Event Classes size before merge: {len(uncompressed_data)}")
    ec1 = clft.EventClassContainer.deserialize(uncompressed_data)

    with gzip.open("foo_2.musicz", "rb") as f:
        uncompressed_data = f.read()
    ec2 = clft.EventClassContainer.deserialize(uncompressed_data)

    ec1.merge_inplace(ec2)
    print(
        f"Event Classes size after merge: {len(clft.EventClassContainer.serialize(ec1))}"
    )


if __name__ == "__main__":
    main()
