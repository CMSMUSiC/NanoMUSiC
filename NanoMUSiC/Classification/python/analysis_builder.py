import tomli
from multiprocessing import Pool
from enum import StrEnum
from pathlib import Path
from helpers import to_toml_dumps
import os
from metadata import Years
import subprocess
import shlex
import sys
from rich.progress import track
from dbs.apis.dbsClient import DbsApi


# def get_files(dataset: str) -> list[str]:
#     proc = subprocess.run(
#         shlex.split(
#             'dasgoclient -query="file dataset={}"'.format(dataset),
#         ),
#         capture_output=True,
#         text=True,
#     )
#
#     if proc.returncode != 0:
#         print("ERROR: Could not get list of files for dataset: {}.".format(dataset))
#         sys.exit(-1)
#
#     return list(proc.stdout.splitlines())
#
def get_files(arg) -> tuple[str, str, str, list[str]]:
    dataset, sample, year = arg
    dbs = DbsApi("https://cmsweb.cern.ch/dbs/prod/global/DBSReader")
    return (
        dataset,
        sample,
        year,
        [file["logical_file_name"].strip() for file in dbs.listFiles(dataset=dataset)],
    )


class ProcessingSite(StrEnum):
    RWTH = "RWTH"
    Other = "Other"


def rwth_has(dataset: str, processing_site: ProcessingSite) -> bool:
    if processing_site != ProcessingSite.RWTH:
        return False

    # proc = subprocess.run(
    #     shlex.split(
    #         'dasgoclient -query="site dataset={}"'.format(dataset),
    #     ),
    #     capture_output=True,
    #     text=True,
    # )
    #
    # if proc.returncode != 0:
    #     print("ERROR: Could not get list of sites for dataset: {}.".format(dataset))
    #     sys.exit(-1)
    #
    # if "T2_DE_RWTH" not in proc.stdout:
    #     print(
    #         "WARNING: Dataset not found in RWTH storage: {}".format(dataset),
    #         file=sys.stderr,
    #     )
    #     return False

    return True


def build_analysis_config(input_file: str, processing_site: ProcessingSite) -> None:
    samples_list = tomli.loads(Path(input_file).read_text(encoding="utf-8"))

    args = []
    for sample in samples_list:
        for year in Years:
            samples_list[sample]["input_files_{}".format(year)] = []
            for dataset in samples_list[sample].get("das_name_{}".format(year), []):
                args.append((dataset, sample, year))

    with Pool() as p:
        for _, result in track(
            enumerate(p.imap_unordered(get_files, args)),
            description="Processing {} samples ...".format(len(args)),
            total=len(args),
        ):
            dataset, sample, year, files_per_dataset = result
            is_at_rwth = rwth_has(dataset, processing_site)
            if len(files_per_dataset):
                if is_at_rwth and processing_site == ProcessingSite.RWTH:
                    for i, f in enumerate(files_per_dataset):
                        files_per_dataset[i] = (
                            "davs://grid-webdav.physik.rwth-aachen.de:2889{}".format(f)
                        )
                else:
                    for i, f in enumerate(files_per_dataset):
                        files_per_dataset[i] = (
                            "root://cms-xrd-global.cern.ch//{}".format(f)
                        )
                samples_list[sample]["input_files_{}".format(year)] += files_per_dataset

    # dump new config to string
    os.system("rm analysis_config.toml > /dev/null 2>&1")
    with open("analysis_config.toml", "w") as f:
        f.write(to_toml_dumps(samples_list))
