from concurrent import futures
import tomli
from concurrent.futures import ProcessPoolExecutor, as_completed
from enum import StrEnum
from pathlib import Path
from helpers import to_toml_dumps, cyclic_iterator
import os
from metadata import Years
from rich.progress import track
from dbs.apis.dbsClient import DbsApi, sys
import getpass
from rich.prompt import Confirm
from rich.panel import Panel
from rich.text import Text
from rich.console import Console

console = Console()


def get_files(
    dataset, sample, year, processing_site
) -> tuple[str, str, str, list[str], list[str]]:
    # import ROOT
    import uproot

    dbs = DbsApi("https://cmsweb.cern.ch/dbs/prod/global/DBSReader")

    raw_files_per_dataset = [
        file["logical_file_name"].strip() for file in dbs.listFiles(dataset=dataset)
    ]

    all_redirectors = [
        # "davs://grid-webdav.physik.rwth-aachen.de:2889",
        "root://grid-dcache.physik.rwth-aachen.de//",
        "root://cmsxrootd.fnal.gov//",
        "root://xrootd-cms.infn.it//",
        "root://cms-xrd-global.cern.ch//",
    ]

    match processing_site:
        case ProcessingSite.RWTH:
            start_pos = 0
        case ProcessingSite.Americas:
            start_pos = 1
        case ProcessingSite.Europe | ProcessingSite.Asia:
            start_pos = 2
        case ProcessingSite.Other:
            start_pos = 3
        case _:
            raise RuntimeError("Invalid processing site")

    files_per_dataset = []
    files_not_found = []
    for f in raw_files_per_dataset:
        found_site = False
        for redirector in cyclic_iterator(all_redirectors, start_pos):
            try:
                # ROOT.TFile.Open(redirector + f).Close()
                uproot.open(redirector + f)
                files_per_dataset.append(redirector + f)
                found_site = True
                break
            except Exception as _:
                continue

        if not found_site:
            files_not_found.append(f)

    return (dataset, sample, year, files_per_dataset, files_not_found)


class ProcessingSite(StrEnum):
    RWTH = "RWTH"
    Americas = "Americas"
    Europe = "Europe"
    Asia = "Asia"
    Other = "Other"


def build_analysis_config(input_file: str, processing_site: ProcessingSite) -> None:
    samples_list = tomli.loads(Path(input_file).read_text(encoding="utf-8"))

    try:
        print(f"Username: {os.environ['USER']}")

    except KeyError as _:
        os.environ["USER"] = getpass.getuser()
        print(f"Username set to: {os.environ['USER']}")

    with ProcessPoolExecutor() as executor:
        futures = []
        for sample in samples_list:
            for year in Years:
                samples_list[sample]["input_files_{}".format(year)] = []
                for dataset in samples_list[sample].get("das_name_{}".format(year), []):
                    futures.append(
                        executor.submit(
                            get_files, dataset, sample, year, processing_site
                        )
                    )

        for future in track(
            as_completed(futures),
            description="Processing {} samples ...".format(len(futures)),
            total=len(futures),
            console=console,
        ):
            result = future.result()
            dataset, sample, year, files_per_dataset, files_not_found = result
            if len(files_per_dataset):
                samples_list[sample]["input_files_{}".format(year)] += files_per_dataset
            if len(files_not_found):
                console.print(
                    Panel(
                        Text(
                            f"Warning: Files not found for {dataset}: {len(files_not_found)}/{(len(files_not_found)+len(files_per_dataset))} => {float(len(files_not_found))*100/float(len(files_not_found)+len(files_per_dataset)):.2f}% lost",
                            style="bold yellow",
                        )
                    )
                )

    # dump new config to string
    os.system("rm analysis_config.toml > /dev/null 2>&1")
    with open("analysis_config.toml", "w") as f:
        f.write(to_toml_dumps(samples_list))
