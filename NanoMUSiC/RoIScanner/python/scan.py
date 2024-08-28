from enum import Enum
import subprocess
import re
import hashlib
import random
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
from metadata import make_ec_nice_name
from tools import configure_root

from ROOT import TFile

from scan_results import ScanResults

from crab_scan import music_sh, crab_sub, crab_worker, pset
from cmssw_scan import make_scanner_config, exec_command


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


def do_scan(scan_props: ScanProps, scan_type: ScanType) -> str | None:
    lut_file_path = "{}/bin/lookuptable.bin".format(os.getenv("MUSIC_BASE"))
    shifts_file_path = "shifts.json"

    if not os.path.exists("{}/".format(os.getenv("MUSIC_BASE"))):
        print(
            'ERROR: Could not start scanner. LUT file does not exist. Did you executed "ninja lut"?',
            file=sys.stderr,
        )
        sys.exit(-1)

    if not os.path.exists(scan_props.json_file_path):
        print(
            "ERROR: Could not start scanner. Input file not found.",
            file=sys.stderr,
        )
        sys.exit(-1)

    with open(scan_props.json_file_path, "r") as json_file:
        data = json.load(json_file)
    distribution = ScanDistribution(**data)

    data_scan_status = False
    if scan_type == ScanType.Data:
        data_scan_status = scanner.scan(
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

        if not data_scan_status:
            print("ERROR: Could not perform Data scan.", file=sys.stderr)
            sys.exit(-1)

    mc_scan_status = False
    if scan_type == ScanType.Toys:
        mc_scan_status = scanner.scan(
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
        if not mc_scan_status:
            print("ERROR: Could not perform MC scan.", file=sys.stderr)
            sys.exit(-1)

    if data_scan_status:
        return "Distribution: {} - {} - {}".format(
            distribution.name,
            distribution.distribution,
            distribution.year,
        )

    if mc_scan_status:
        return "Distribution: {} - {} - {} | Start round: {}".format(
            distribution.name,
            distribution.distribution,
            distribution.year,
            scan_props.start_round,
        )


def make_starting_rounds(total: int, chunk_size: int) -> list[tuple[int, int]]:
    start = 0
    # Generate the interval range
    interval = list(range(start, total))

    # Split the interval into chunks
    chunks = [interval[i : i + chunk_size] for i in range(0, len(interval), chunk_size)]

    return [(c[0], len(c)) for c in chunks]


def build_scan_jobs_task(
    args: tuple[str, str, str, int, int, bool],
) -> tuple[list[ScanProps], list[str]]:
    distribution_file, output_dir, distribution_type, n_rounds, split_size, skip_lut = (
        args
    )
    temp_scan_props: list[ScanProps] = []
    this_variations: list[str] = []

    root_file = TFile.Open(distribution_file)
    distribution_names: list[str] = [
        str(k.GetName()) for k in root_file.GetListOfKeys()
    ]

    for dist_name in distribution_names:
        if (
            "Run2" in dist_name
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

    return temp_scan_props, this_variations


def launch_scan(
    input_dir: str,
    patterns: list[str],
    distribution_type: str,
    output_dir: str = "scan_results",
    num_cpus: int = 128,
    do_clean: bool = False,
    n_rounds: int = 100_000,
    split_size: int = 1000,
):
    if not os.path.isdir(input_dir):
        print("ERROR: Input directory does not exists.")
        sys.exit(-1)

    if do_clean:
        print("Cleanning output directory ...")
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
        print("WARNING: No distribution matches the requirements.")
        sys.exit(1)

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
                    (dist, output_dir, distribution_type, n_rounds, split_size, False)
                    for dist in distribution_files
                ],
            ):
                scan_props += this_scan_props
                variations = list(set(variations + this_variations))
                progress.advance(task)

    make_shifts(n_rounds, variations)

    # Will launch scan and save results
    data_scan_props = [scan for scan in scan_props if scan.start_round == 0]
    data_scan_props = sorted(data_scan_props, key=lambda scan: len(scan.ec_name))
    scan_props = sorted(scan_props, key=lambda scan: len(scan.ec_name))
    with Pool(max(1, min(max(len(data_scan_props), len(scan_props)), num_cpus))) as p:
        if len(data_scan_props) > 0:
            with Progress() as progress:
                task = progress.add_task(
                    "Performing {} scans (Data)...".format(len(data_scan_props)),
                    total=len(data_scan_props),
                )
                for job in p.imap_unordered(do_scan_data, data_scan_props):
                    if job:
                        progress.console.print("Done: {}".format(job))
                        progress.advance(task)
                    else:
                        print(
                            "ERROR: Could not run the scanner (Data). Unknown error.",
                            file=sys.stderr,
                        )
                        sys.exit(-1)

        if len(scan_props) > 0:
            with Progress() as progress:
                task = progress.add_task(
                    "Performing {} scans (toys)...".format(len(scan_props)),
                    total=len(scan_props),
                )
                for job in p.imap_unordered(do_scan_toys, scan_props):
                    if job:
                        progress.console.print("Done: {}".format(job))
                        progress.advance(task)
                    else:
                        print(
                            "ERROR: Could not run the scanner (Toys). Unknown error.",
                            file=sys.stderr,
                        )
                        sys.exit(-1)

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
):
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
        print("WARNING: No distribution matches the requirements.")
        sys.exit(1)

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
                        (dist, output_dir, dist_type.value, n_rounds, split_size, False)
                        for dist in distribution_files
                    ],
                ):
                    scan_props += this_scan_props
                    variations = list(set(variations + this_variations))
                    progress.advance(task)

    print("Making shifts file ...")
    make_shifts(n_rounds, variations)

    data_scan_props = [scan for scan in scan_props if scan.start_round == 0]
    random.shuffle(data_scan_props)
    random.shuffle(scan_props)

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
        'cmssw-el9 --cleanenv --command-to-run "gfal-rm -r root://eoscms.cern.ch///eos/cms/store/user/ftorresd/cmsmusic/scan_results.tar"'
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
        'cmssw-el9 --cleanenv --command-to-run "gfal-copy -r -f scan_results.tar root://eoscms.cern.ch///eos/cms/store/user/ftorresd/cmsmusic/."'
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
            ).replace(
                "___NUM_JOBS___",
                str(num_jobs),
            )
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


def get_p_tilde(scan_result_data_file_path: str, scan_mc_data_files: str) -> None:
    scan_results = ScanResults.make_scan_results(
        scan_result_data_file_path, scan_mc_data_files
    )

    print("Event class: {}".format(scan_results.class_name))
    print("Distribution: {}".format(scan_results.distribution))
    print(
        "RoI: [{} - {}]".format(
            scan_results.lower_edge, scan_results.lower_edge + scan_results.width
        )
    )
    print("p-value: {}".format(scan_results.p_value_data))

    print("p-tilde: {}".format(scan_results.p_tilde()))
    print()
