import tomli
from pathlib import Path
from helpers import to_toml_dumps
import os
from metadata import Years
import subprocess
import shlex
import sys
from rich.progress import track


def get_files(dataset: str) -> list[str]:
    proc = subprocess.run(
        shlex.split(
            'dasgoclient -query="file dataset={}"'.format(dataset),
        ),
        capture_output=True,
        text=True,
    )

    if proc.returncode != 0:
        print("ERROR: Could not get list of files for dataset: {}.".format(dataset))
        sys.exit(-1)

    return list(proc.stdout.splitlines())


def rwth_has(dataset: str) -> list[str]:
    proc = subprocess.run(
        shlex.split(
            'dasgoclient -query="site dataset={}"'.format(dataset),
        ),
        capture_output=True,
        text=True,
    )

    if proc.returncode != 0:
        print("ERROR: Could not get list of sites for dataset: {}.".format(dataset))
        sys.exit(-1)

    return "T2_DE_RWTH" in proc.stdout


def build_analysis_config(input_file: str) -> None:
    samples_list = tomli.loads(Path(input_file).read_text(encoding="utf-8"))

    for sample in track(
        samples_list,
        description="Processing {} samples ...".format(len(samples_list)),
        total=len(samples_list),
    ):
        for year in Years:
            samples_list[sample]["output_files_{}".format(year)] = []
            for dataset in samples_list[sample].get("das_name_{}".format(year), []):
                files_per_dataset = get_files(dataset)
                is_at_rwth = rwth_has(dataset)
                if len(files_per_dataset):
                    if is_at_rwth:
                        for i, f in enumerate(files_per_dataset):
                            files_per_dataset[i] = (
                                "davs://grid-webdav.physik.rwth-aachen.de:2889{}".format(
                                    f
                                )
                            )
                    samples_list[sample]["output_files_{}".format(year)] += (
                        files_per_dataset
                    )

    # print(samples_list)
    # dump new config to string
    os.system("rm analysis_config.toml > /dev/null 2>&1")
    with open("analysis_config.toml", "w") as f:
        f.write(to_toml_dumps(samples_list))
