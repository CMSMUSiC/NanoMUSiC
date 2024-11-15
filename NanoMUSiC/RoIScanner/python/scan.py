from enum import Enum, StrEnum
import time
from multiprocessing.pool import AsyncResult
import re
from rich.progress import track
import subprocess
import hashlib
import scanner_imp as scanner
from multiprocessing import Pool
from rich.progress import Progress
import fnmatch
import os
import sys
import json
import numpy as np
from scan_props import ScanProps
from distribution_model import (
    ScanDistribution,
    DistributionType,
    ScanYear,
    MCBinsBuilder,
)
from metadata import make_ec_nice_name, make_raw_ec_name, ClassType
from tools import configure_root

from ROOT import TFile


from crab_scan import music_sh, crab_sub, crab_worker, pset
from cmssw_scan import make_scanner_config, exec_command
from pydantic import BaseModel


configure_root()
MC_THRESHOLD = 0.1


def make_shifts(num_rounds: int, variations: list[str]) -> None:
    shifts = dict(
        zip(
            variations,
            np.random.normal(
                loc=1.0, scale=1.0, size=(len(variations), num_rounds)
            ).tolist(),
        )
    )

    with open("shifts.json", "w") as json_file:
        json.dump(shifts, json_file, indent=4)


class ScanType(Enum):
    Data = 0
    Toys = 1
    Signal = 2


def do_scan_data(scan_props: ScanProps) -> str | None:
    return do_scan(scan_props, ScanType.Data)


def do_scan_toys(scan_props: ScanProps) -> str | None:
    return do_scan(scan_props, ScanType.Toys)


def do_scan(scan_props: ScanProps, scan_type: ScanType) -> str:
    lut_file_path = "{}/bin/lookuptable.bin".format(os.getenv("MUSIC_BASE"))
    shifts_file_path = "shifts.json"

    if not os.path.exists("{}/".format(os.getenv("MUSIC_BASE"))):
        raise FileNotFoundError(
            'ERROR: Could not start scanner. LUT file does not exist. Did you executed "ninja lut"?'
        )

    if not os.path.exists(scan_props.json_file_path):
        raise FileNotFoundError("ERROR: Could not start scanner. Input file not found.")

    with open(scan_props.json_file_path, "r") as json_file:
        data = json.load(json_file)
    distribution = ScanDistribution(**data)

    scan_status = False
    if scan_type == ScanType.Data:
        scan_status = scanner.scan(
            scan_props.json_file_path,
            "{}/{}".format(
                scan_props.output_directory, distribution.name.replace("+", "_")
            ),
            scan_props.rounds,
            scan_props.start_round,
            shifts_file_path,
            lut_file_path,
            scan_type="data",
        )

        if not scan_status:
            raise Exception("ERROR: Could not perform Data scan.")

    if scan_type == ScanType.Toys:
        scan_status = scanner.scan(
            scan_props.json_file_path,
            "{}/{}".format(
                scan_props.output_directory, distribution.name.replace("+", "_")
            ),
            scan_props.rounds,
            scan_props.start_round,
            shifts_file_path,
            lut_file_path,
            scan_type="mc",
        )
        if not scan_status:
            raise Exception("ERROR: Could not perform MC scan.")

    if scan_type == ScanType.Toys:
        return "Distribution: {} - {} - {} | Start round: {}".format(
            distribution.name,
            distribution.distribution,
            distribution.year,
            scan_props.start_round,
        )
    else:
        return "Distribution: {} - {} - {}".format(
            distribution.name,
            distribution.distribution,
            distribution.year,
        )


def make_starting_rounds(total: int, chunk_size: int) -> list[tuple[int, int]]:
    start = 0
    # Generate the interval range
    interval = list(range(start, total))

    # Split the interval into chunks
    chunks = [interval[i : i + chunk_size] for i in range(0, len(interval), chunk_size)]

    return [(c[0], len(c)) for c in chunks]


def count_objects(class_name: str, count_jets: bool = True) -> int:
    """For a given event class, it will count the number of physics objects."""
    if count_jets:
        return sum([int(c) for c in class_name if c.isdigit()])

    parts = (
        class_name.replace("_X", "")
        .replace("+X", "")
        .replace("_NJet", "")
        .replace("+NJet", "")
        .split("_")
    )

    n_muons: int = 0
    n_electrons: int = 0
    n_taus: int = 0
    n_photons: int = 0
    n_bjets: int = 0
    n_met: int = 0

    for i, p in enumerate(parts):
        if i == 0:
            continue

        count = p[0]
        if p[1:] == "Muon":
            n_muons = int(count)
            continue
        if p[1:] == "Electron":
            n_electrons = int(count)
            continue
        if p[1:] == "Tau":
            n_taus = int(count)
            continue
        if p[1:] == "Photon":
            n_photons = int(count)
            continue
        if p[1:] == "bJet":
            n_bjets = int(count)
            continue
        if p[1:] == "Jet":
            _ = int(count)
            continue
        if p[1:] == "MET":
            n_met = int(count)
            continue

        print(
            "ERROR: Could not count objects class name: {}".format(class_name),
            file=sys.stderr,
        )
        sys.exit(-1)

    return n_muons + n_electrons + n_taus + n_photons + n_bjets + n_met


def build_scan_jobs_task(
    args: tuple[str, str, str, int, int, bool, ClassType],
) -> tuple[list[ScanProps], list[str]]:
    (
        distribution_file,
        output_dir,
        distribution_type,
        n_rounds,
        split_size,
        skip_lut,
        class_type,
    ) = args
    temp_scan_props: list[ScanProps] = []
    this_variations: list[str] = []

    root_file = TFile.Open(distribution_file)
    distribution_names: list[str] = [
        str(k.GetName()) for k in root_file.GetListOfKeys()
    ]

    for dist_name in distribution_names:
        if (
            "Run2" in dist_name
            # "2016" in dist_name
            and "counts" not in dist_name
            and distribution_type in dist_name
        ):
            dist = root_file.Get(dist_name)
            if dist.has_mc(MC_THRESHOLD) and dist.has_data():
                this_variations = [
                    str(var) for var, _ in dist.m_systematics_uncertainties
                ]

                scan_distribution_type = DistributionType.invariant_mass
                if dist.m_distribution_name == "sum_pt":
                    scan_distribution_type = DistributionType.sum_pt
                if dist.m_distribution_name == "met":
                    scan_distribution_type = DistributionType.met

                ec_nice_name = make_ec_nice_name(
                    make_ec_nice_name(str(dist.m_event_class_name))
                )

                raw_data_counts = dist.get_data_counts()
                data_counts = []
                for i in range(raw_data_counts.size()):
                    data_counts.append(raw_data_counts[i])

                for start_round, n_rds in make_starting_rounds(n_rounds, split_size):
                    temp_scan_props.append(
                        ScanProps(
                            ec_name=ec_nice_name,
                            distribution_type=distribution_type,
                            json_file_path=ScanDistribution(
                                name=ec_nice_name,
                                distribution=scan_distribution_type,
                                year=ScanYear.Run2,
                                # year=ScanYear.Run2016,
                                MCBins=MCBinsBuilder(dist.get_mcbins_props()).build(),
                                DataBins=data_counts,
                                FirstRound=start_round,
                                skipLookupTable=skip_lut,
                            ).save(output_dir),
                            output_directory=output_dir,
                            rounds=n_rds,
                            start_round=start_round,
                        )
                    )

                # prepare output area
                if not os.path.exists(
                    "{}/{}".format(output_dir, ec_nice_name.replace("+", "_"))
                ):
                    os.makedirs(
                        "{}/{}".format(output_dir, ec_nice_name.replace("+", "_"))
                    )

    root_file.Close()

    # ClassType.All is the default case
    if class_type == ClassType.Inclusive:
        temp_scan_props = [scan for scan in temp_scan_props if "+X" in scan.ec_name]
    if class_type == ClassType.Exclusive:
        temp_scan_props = [scan for scan in temp_scan_props if "+" not in scan.ec_name]
    if class_type == ClassType.JetInclusive:
        temp_scan_props = [scan for scan in temp_scan_props if "+N" not in scan.ec_name]

    return temp_scan_props, this_variations


class DataOrMC(StrEnum):
    Data = "data"
    MC = "mc"
    All = "all"


def launch_scan(
    input_dir: str,
    patterns: list[str],
    distribution_type: str,
    class_type: ClassType,
    output_dir: str = "scan_results",
    num_cpus: int = 128,
    do_clean: bool = False,
    n_rounds: int = 100_000,
    split_size: int = 1000,
    data_or_mc: DataOrMC = DataOrMC.All,
    do_make_shifts: bool = True,
    do_copy_index_files: bool = True,
) -> None:
    if not os.path.isdir(input_dir):
        print("ERROR: Input directory does not exists.")
        sys.exit(-1)

    if do_clean:
        print("Cleanning output directory ...")
        os.system("rm -rf {}".format(output_dir))
    os.system("mkdir -p {}".format(output_dir))

    def make_distribution_paths() -> list[str]:
        distribution_paths: list[str] = []
        for root, _, files in os.walk(input_dir):
            for file in files:
                if any(
                    fnmatch.fnmatch(file, "*" + pattern.replace("+", "_") + ".root")
                    for pattern in patterns
                ):
                    distribution_paths.append(os.path.join(root, file))
        return distribution_paths

    distribution_files = make_distribution_paths()
    if len(distribution_files) == 0:
        print("ERROR: No distribution matches the requirements.")
        sys.exit(-1)

    # Will build scan jobs
    variations: list[str] = []
    scan_props: list[ScanProps] = []
    with Pool(min(len(distribution_files), num_cpus)) as p:
        with Progress() as progress:
            task = progress.add_task(
                "Building scan jobs {} - {} distribution files ...".format(
                    distribution_type,
                    len(distribution_files),
                ),
                total=len(distribution_files),
            )
            for this_scan_props, this_variations in p.imap_unordered(
                build_scan_jobs_task,
                [
                    (
                        dist,
                        output_dir,
                        distribution_type,
                        n_rounds,
                        split_size,
                        False,
                        class_type,
                    )
                    for dist in distribution_files
                ],
            ):
                scan_props += this_scan_props
                variations = list(set(variations + this_variations))
                progress.advance(task)

    if do_make_shifts:
        make_shifts(n_rounds, variations)

    # Will launch scan and save results
    data_scan_props = [scan for scan in scan_props if scan.start_round == 0]
    data_scan_props = [
        scan
        for scan in data_scan_props
        if not (
            count_objects(scan.ec_name) == 1
            and scan.distribution_type == "invariant_mass"
        )
    ]
    data_scan_props = sorted(
        data_scan_props, key=lambda scan: count_objects(scan.ec_name)
    )
    scan_props = [
        scan
        for scan in scan_props
        if not (
            count_objects(scan.ec_name) == 1
            and scan.distribution_type == "invariant_mass"
        )
    ]
    scan_props = sorted(scan_props, key=lambda scan: count_objects(scan.ec_name))
    with Pool(max(1, min(max(len(data_scan_props), len(scan_props)), num_cpus))) as p:
        if (
            len(data_scan_props) > 0
            and data_or_mc == DataOrMC.Data
            or data_or_mc == DataOrMC.All
        ):
            with Progress() as progress:
                task = progress.add_task(
                    "Performing {} scans (Data)...".format(len(data_scan_props)),
                    total=len(data_scan_props),
                )
                jobs: dict[ScanProps, AsyncResult] = {}
                for prop in data_scan_props:
                    jobs[prop] = p.apply_async(do_scan_data, (prop,))
                while len(jobs):
                    ready: list[ScanProps] = []
                    not_ready: list[str] = []
                    # progress.console.print("Checking ...")
                    for job in jobs:
                        if jobs[job].ready():
                            progress.console.print("Done: {}".format(jobs[job].get()))
                            ready.append(job)
                            progress.advance(task)
                        else:
                            not_ready.append(
                                "{} - {}".format(job.ec_name, job.distribution_type)
                            )

                    for job_ready in ready:
                        jobs.pop(job_ready)

                    # if len(not_ready) <= 100 and len(not_ready) > 0:
                    #     progress.console.print("Not ready: {}".format(not_ready))
                    # progress.console.print(
                    #     "Waiting 10 seconds before next iteration ..."
                    # )
                    # time.sleep(10)

        if (
            len(scan_props) > 0
            and data_or_mc == DataOrMC.MC
            or data_or_mc == DataOrMC.All
        ):
            with Progress() as progress:
                task = progress.add_task(
                    "Performing {} scans (toys)...".format(len(scan_props)),
                    total=len(scan_props),
                )
                jobs: dict[ScanProps, AsyncResult] = {}
                for prop in scan_props:
                    jobs[prop] = p.apply_async(do_scan_toys, (prop,))
                while len(jobs):
                    ready: list[ScanProps] = []
                    not_ready: list[str] = []
                    # progress.console.print("Checking ...")
                    for job in jobs:
                        if jobs[job].ready():
                            progress.console.print("Done: {}".format(jobs[job].get()))
                            ready.append(job)
                            progress.advance(task)
                        else:
                            not_ready.append(
                                "{} - {} - {}".format(
                                    job.ec_name, job.distribution_type, job.rounds
                                )
                            )
                    for job_ready in ready:
                        jobs.pop(job_ready)

                    # if len(not_ready) <= 100 and len(not_ready) > 0:
                    #     progress.console.print("Not ready: {}".format(not_ready))
                    # progress.console.print("Waiting 2 mins before next iteration ...")
                    # time.sleep(120)

    if do_copy_index_files:
        print("Copying index.php ...")
        os.system(
            r"find ___OUTPUT_DIR___/ -type d -exec cp $MUSIC_BASE/NanoMUSiC/Plotter/assets/index.php {} \;".replace(
                "___OUTPUT_DIR___", output_dir
            )
        )

    print("Done.")


def crab_username():
    try:
        # Run the command and capture the output
        result = subprocess.run(
            r'cmssw-el9 --cleanenv --command-to-run "cd CMSSW_14_0_7/src && cmsenv && crab checkusername"',
            shell=True,
            text=True,
            capture_output=True,
            check=True,
        )

        # Get the output as a string
        output = result.stdout.strip()

        # Define the pattern to extract the username
        pattern = r"Username is: (\S+)"

        # Search for the pattern in the output
        match = re.search(pattern, output)

        if match:
            # Extract the username
            username = match.group(1)
            return username
        else:
            print("ERROR: CRAB Username not found in the output.", file=sys.stderr)
            sys.exit(-1)

    except subprocess.CalledProcessError as e:
        print(f"ERROR: Could not get CRAB username. Command failed with error: {e}")
        sys.exit(-1)
    except ValueError as e:
        print(e)
        sys.exit(-1)


def launch_crab_scan(
    input_dir: str,
    patterns: list[str],
    output_dir: str = "scan_results",
    num_cpus: int = 128,
    do_clean: bool = False,
    n_rounds: int = 100_000,
    split_size: int = 10_000,
) -> None:
    cms_user = crab_username()
    print("CRAB username is: {}".format(cms_user))

    if not os.path.isdir(input_dir):
        print("ERROR: Input directory does not exists.")
        sys.exit(-1)

    if do_clean:
        os.system("rm -rf {}".format(output_dir))
    os.system("mkdir -p {}".format(output_dir))

    def make_distribution_paths(inputs_dir: str, patterns: list[str]) -> list[str]:
        distribution_paths: list[str] = []
        for root, _, files in os.walk(inputs_dir):
            for file in files:
                if any(
                    fnmatch.fnmatch(file, "*" + pattern.replace("+", "_") + ".root")
                    for pattern in patterns
                ):
                    distribution_paths.append(os.path.join(root, file))
        return distribution_paths

    distribution_files = make_distribution_paths(input_dir, patterns)
    if len(distribution_files) == 0:
        print("ERROR: No distribution matches the requirements.")
        sys.exit(-1)

    # Will build scan jobs
    variations: list[str] = []
    scan_props: list[ScanProps] = []
    for dist_type in DistributionType:
        with Pool(min(len(distribution_files), num_cpus)) as p:
            with Progress() as progress:
                task = progress.add_task(
                    "Building scan jobs: {} - {} distribution files ...".format(
                        dist_type.value,
                        len(distribution_files),
                    ),
                    total=len(distribution_files),
                )
                for this_scan_props, this_variations in p.imap_unordered(
                    build_scan_jobs_task,
                    [
                        (
                            dist,
                            output_dir,
                            dist_type.value,
                            n_rounds,
                            split_size,
                            False,
                            ClassType.All,
                        )
                        for dist in distribution_files
                    ],
                ):
                    scan_props += this_scan_props
                    variations = list(set(variations + this_variations))
                    progress.advance(task)

    print("Making shifts file ...")
    make_shifts(n_rounds, variations)

    data_scan_props = [scan for scan in scan_props if scan.start_round == 0]
    data_scan_props = [
        scan
        for scan in data_scan_props
        if not (
            count_objects(scan.ec_name) == 1
            and scan.distribution_type == "invariant_mass"
        )
    ]
    data_scan_props = sorted(
        data_scan_props, key=lambda scan: count_objects(scan.ec_name, count_jets=False)
    )
    scan_props = [
        scan
        for scan in scan_props
        if not (
            count_objects(scan.ec_name) == 1
            and scan.distribution_type == "invariant_mass"
        )
    ]
    scan_props = sorted(
        scan_props, key=lambda scan: count_objects(scan.ec_name, count_jets=False)
    )

    print("Writing CMSSW files ...")
    exec_command("rm -rf temp_scan_cmssw_config_files")

    exec_command("mkdir -p temp_scan_cmssw_config_files")
    os.system(
        'cmssw-el9 --cleanenv --command-to-run "gfal-rm -r davs://grid-webdav.physik.rwth-aachen.de:2889///store/user/{}/music"'.format(
            cms_user
        )
    )
    os.system(
        'cmssw-el9 --cleanenv --command-to-run "gfal-mkdir -p davs://grid-webdav.physik.rwth-aachen.de:2889///store/user/{}/music"'.format(
            cms_user
        )
    )
    os.system(
        r'cmssw-el9 --cleanenv --command-to-run "gfal-rm -r root://eoscms.cern.ch///eos/cms/store/user/___CMS_USER___/cmsmusic/scan_results.tar"'.replace(
            "___CMS_USER___", cms_user
        )
    )

    cmsRun_calls = ""
    for scan in data_scan_props:
        hash_object = hashlib.sha256()
        hash_object.update((scan.json_file_path + "data").encode("utf-8"))
        cmssw_scanner_config_file = (
            "temp_scan_cmssw_config_files/cmssw_scanner_{}.py".format(
                hash_object.hexdigest()
            )
        )
        with open(cmssw_scanner_config_file, "w") as file:
            file.write(
                make_scanner_config(
                    scan.json_file_path,
                    "{}/{}".format(
                        scan.output_directory, scan.ec_name.replace("+", "_")
                    ),
                    scan.rounds,
                    scan.start_round,
                    "shifts.json",
                    "lookuptable.bin",
                    scan_type="data",
                    ec_name=scan.ec_name,
                    distribution_type=scan.distribution_type,
                    # now=now,
                )
            )
        cmsRun_calls += "cmsrun {}\n".format(cmssw_scanner_config_file)

    for scan in scan_props:
        hash_object = hashlib.sha256()
        hash_object.update((scan.json_file_path + "mc").encode("utf-8"))
        cmssw_scanner_config_file = (
            "temp_scan_cmssw_config_files/cmssw_scanner_{}.py".format(
                hash_object.hexdigest()
            )
        )
        with open(cmssw_scanner_config_file, "w") as file:
            file.write(
                make_scanner_config(
                    scan.json_file_path,
                    "{}/{}".format(
                        scan.output_directory, scan.ec_name.replace("+", "_")
                    ),
                    scan.rounds,
                    scan.start_round,
                    "shifts.json",
                    "lookuptable.bin",
                    scan_type="mc",
                    ec_name=scan.ec_name,
                    distribution_type=scan.distribution_type,
                )
            )
        cmsRun_calls += "cmsrun {}\n".format(cmssw_scanner_config_file)

    print("Creating input files tarball ...")
    os.system("rm -rf scan_results.tar")
    exec_command("tar -cf scan_results.tar scan_results")

    print("Copying scan results to EOS ...")
    res = os.system(
        r'cmssw-el9 --cleanenv --command-to-run "gfal-copy -r -f -p scan_results.tar root://eoscms.cern.ch///eos/cms/store/user/___CMS_USER___/cmsmusic/."'.replace(
            "___CMS_USER___", cms_user
        )
    )
    if res != 0:
        print("ERROR: Could not copy input files.", file=sys.stderr)
        sys.exit(-1)

    exec_command(
        "tar -zcvf temp_scan_cmssw_config_files.tar.gz temp_scan_cmssw_config_files"
    )

    MAX_CRAB_JOBS_PER_TASK = 10_000

    num_jobs = min(len(data_scan_props) + len(scan_props), MAX_CRAB_JOBS_PER_TASK)
    crab_split_size = (len(data_scan_props) + len(scan_props)) // num_jobs

    print("Writing CRAB files ...")
    exec_command("rm -rf music.sh > /dev/null 2>&1")
    with open("music.sh", "w") as file:
        file.write(
            music_sh.replace(
                "___SPLIT_SIZE___",
                str(crab_split_size),
            )
            .replace(
                "___NUM_JOBS___",
                str(num_jobs),
            )
            .replace("___CMS_USER___", str(cms_user))
        )
    exec_command("chmod +x music.sh")

    exec_command("rm -rf pset.py > /dev/null 2>&1")
    with open("pset.py", "w") as file:
        file.write(pset)

    exec_command("rm -rf crab_sub.py > /dev/null 2>&1")
    with open("crab_sub.py", "w") as file:
        file.write(
            crab_sub.replace("___NUM_JOBS___", str(num_jobs))
            .replace("___CMS_USER___", str(cms_user))
            .replace(
                "___LUT___", "{}/bin/lookuptable.bin".format(os.getenv("MUSIC_BASE"))
            )
        )

    exec_command("rm -rf crab_worker.sh > /dev/null 2>&1")
    with open("crab_worker.sh", "w") as file:
        file.write(crab_worker)
    exec_command("chmod +x crab_worker.sh")

    print("Cleanning old files ...")
    exec_command("rm -rf __pycache__ crab.log crab_MUSIC_CLASSIFICATION")

    print("Launching CRAB task ...")
    os.system(
        r'cmssw-el9 --cleanenv --command-to-run "cd CMSSW_14_0_7/src && cmsenv && cd ../.. && ./crab_worker.sh"'
    )

    exec_command("rm -rf temp_scan_cmssw_config_files")


def download_result_file(
    args: tuple[str, str, str],
) -> tuple[subprocess.CompletedProcess, str] | tuple[None, str]:
    result_file, cms_user, job_time_hash = args

    if not os.path.isdir("scan_downloads/{}".format(result_file)):
        res = subprocess.run(
            r'cmssw-el9 --cleanenv --command-to-run "gfal-copy -n 50 -p -r -f --abort-on-failure -t 9999999 davs://grid-webdav.physik.rwth-aachen.de:2889///store/user/___CMS_USER___/music/CRAB_PrivateMC/MUSIC_CLASSIFICATION/___JOB_TIME_HASH___/___RES_FILE___ scan_downloads/."'.replace(
                "___CMS_USER___", cms_user
            )
            .replace("___JOB_TIME_HASH___", job_time_hash)
            .replace("___RES_FILE___", result_file),
            shell=True,
            capture_output=True,
        )

        return res, result_file

    return None, result_file


def get_output(skip_download: bool = False) -> None:
    # os.system("rm -rf scan_downloads")
    os.system("mkdir -p  scan_downloads")

    if not skip_download:
        res = subprocess.run(
            r'cmssw-el9 --cleanenv --command-to-run "cd CMSSW_14_0_7/src && cmsenv && cd ../.. && crab getoutput --jobids 1 --dump"',
            shell=True,
            capture_output=True,
            text=True,
        )

        if res.returncode != 0:
            print("ERROR: Could not get results PFN.", file=sys.stderr)
            print(res.stdout, file=sys.stderr)
            print(res.stderr, file=sys.stderr)
            sys.exit(-1)

        cms_user = crab_username()

        job_time_hash: None | str = None
        for line in res.stdout.splitlines():
            if "PFN:" in line:
                job_time_hash = line.split("MUSIC_CLASSIFICATION")[1].split("/")[1]
                break

        if not job_time_hash:
            print("ERROR: Could not get job time hash.", file=sys.stderr)
            sys.exit(-1)

        print("Job date and time: {}".format(job_time_hash))

        download_args: list[tuple[str, str, str]] = []
        for line in res.stdout.splitlines():
            if "PFN:" in line:
                result_file = line.split(job_time_hash)[1]
                download_args.append((result_file[1:], cms_user, job_time_hash))

        with Pool(min(len(download_args), 100)) as p:
            with Progress() as progress:
                task = progress.add_task(
                    description="Downloading {} result files ...".format(
                        len(download_args)
                    ),
                    total=len(download_args),
                )
                for res, filepath in p.imap_unordered(
                    download_result_file,
                    download_args,
                ):
                    # progress.console.print("File: {}".format(filepath))
                    if res:
                        if res.returncode != 0:
                            print(
                                "ERROR: Could not download file.\n{}\n{} ".format(
                                    res.stdout, res.stderr
                                ),
                                file=sys.stderr,
                            )
                            sys.exit(-1)

                    progress.advance(task)
    else:
        exec_command("rm -rf scan_results")
        exec_command("tar -xvf scan_results.tar")

    # unpack results
    result_files: list[str] = []
    for dirpath, _, filenames in os.walk("scan_downloads"):
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            if filepath.endswith(".tar"):
                result_files.append(filepath)

    for f in track(result_files, description="Unpacking results"):
        exec_command("tar -xf {}".format(f))

    print("Cleanning ...")
    exec_command("rm -rf scan_downloads")
    print("... done.")


class RemainingScan(BaseModel):
    class_name: str
    distribution: str
    start_round: int
    is_data: bool

    @property
    def data_or_mc(self) -> str:
        if self.is_data:
            return "data"

        return "mc"


def scan_remaining(results_dir: str, input_dir: str, n_rounds: int, split_size: int):
    scans: list[RemainingScan] = []
    for root, _, files in os.walk(results_dir):
        if root == results_dir:
            continue
        ec = root.split("/")[1]
        for distribution in DistributionType:
            if distribution == DistributionType.met and "MET" not in root:
                continue
            for file in files:
                if distribution.value in file:
                    prefix = "{}_{}".format(ec, distribution.value)
                    suffix = ".json"

                    if file.startswith(prefix) and file.endswith(suffix):
                        core_part = file[len(prefix) : -len(suffix)]
                        _, _, start_round = core_part.rpartition("_")

                        if start_round.isdigit():
                            if start_round == "0":
                                scans.append(
                                    RemainingScan(
                                        class_name=ec,
                                        distribution=distribution.value,
                                        start_round=int(start_round),
                                        is_data=True,
                                    )
                                )
                            scans.append(
                                RemainingScan(
                                    class_name=ec,
                                    distribution=distribution.value,
                                    start_round=int(start_round),
                                    is_data=False,
                                )
                            )

    remaining_scans: dict[str, list[RemainingScan]] = {}
    for d in DistributionType:
        remaining_scans[d.value] = []

    for scan in scans:
        if not os.path.isfile(
            f"{results_dir}/{scan.class_name}/{scan.class_name}_{scan.distribution}_{scan.data_or_mc}_{scan.start_round}_info.json"
        ):
            remaining_scans[scan.distribution].append(scan)

    for dist in DistributionType:
        if len(remaining_scans[dist]):
            this_data_scans = [
                make_raw_ec_name(scan.class_name)
                for scan in remaining_scans[dist]
                if scan.is_data
            ]
            if len(this_data_scans):
                launch_scan(
                    input_dir,
                    this_data_scans,
                    dist,
                    ClassType.All,
                    results_dir,
                    n_rounds=n_rounds,
                    split_size=split_size,
                    data_or_mc=DataOrMC.Data,
                    do_make_shifts=False,
                    do_copy_index_files=False,
                )
            else:
                print("Nothing to do for {} (data).".format(dist))

            this_mc_scans = [
                make_raw_ec_name(scan.class_name)
                for scan in remaining_scans[dist]
                if not scan.is_data
            ]
            if len(this_mc_scans):
                launch_scan(
                    input_dir,
                    this_mc_scans,
                    dist,
                    ClassType.All,
                    results_dir,
                    n_rounds=n_rounds,
                    split_size=split_size,
                    data_or_mc=DataOrMC.MC,
                    do_make_shifts=False,
                    do_copy_index_files=False,
                )
            else:
                print("Nothing to do for {} (mc).".format(dist))
        else:
            print("Nothing to do for {} (mc).".format(dist))
