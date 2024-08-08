from enum import Enum
import random
import sys
import tomli
from pydantic import BaseModel, Field, NonNegativeFloat
from typing import Any


def load_toml(path: str) -> dict[str, Any]:
    with open(path, mode="rb") as fp:
        return tomli.load(fp)


class Years(str, Enum):
    Run2016APV = "2016APV"
    Run2016 = "2016"
    Run2017 = "2017"
    Run2018 = "2018"

    def __str__(self):
        return self.value

    @staticmethod
    def years_to_plot() -> dict[str, dict[str, str]]:
        return {
            "2016*": {"name": "2016", "lumi": "36.3"},  #
            "2017": {"name": "2017", "lumi": "41.5"},  #
            "2018": {"name": "2018", "lumi": "59.8"},  #
            # "[2017,2018]": {"name": "2017+2018", "lumi": "101"},  #
            "*": {"name": "Run2", "lumi": "138"},
        }

    @staticmethod
    def get_lumi(year: str) -> str:
        year_to_lumi = {
            "2016": "36.3",  #
            "2017": "41.5",  #
            "2018": "59.8",  #
            "Run2": "138",
        }
        return year_to_lumi[year]


class Lumi:
    lumi: dict[str, float] = {
        Years.Run2016APV: 19520.0,
        Years.Run2016: 16810.0,
        Years.Run2017: 41480.0,
        Years.Run2018: 59830.0,
    }
    Unit: str = "pb-1"


class Process(BaseModel):
    name: str
    XSec: NonNegativeFloat = Field(default_factory=lambda: 1.0)
    FilterEff: NonNegativeFloat = Field(default_factory=lambda: 1.0)
    kFactor: NonNegativeFloat = Field(default_factory=lambda: 1.0)
    XSecOrder: str = Field(default_factory=str)
    ProcessGroup: str = Field(default_factory=str)
    generator_filter_key: str = Field(default_factory=str)
    is_data: bool
    das_name_2016APV: list[str] = Field(default_factory=list)
    das_name_2016: list[str] = Field(default_factory=list)
    das_name_2017: list[str] = Field(default_factory=list)
    das_name_2018: list[str] = Field(default_factory=list)
    input_files_2016APV: list[str] = Field(default_factory=list)
    input_files_2016: list[str] = Field(default_factory=list)
    input_files_2017: list[str] = Field(default_factory=list)
    input_files_2018: list[str] = Field(default_factory=list)

    def get_files(self, year: Years, max_files: int = sys.maxsize) -> list[str]:
        if year == Years.Run2016APV:
            files = self.input_files_2016APV[
                : min(max_files, len(self.input_files_2016APV))
            ]
            random.shuffle(files)
            return files
        if year == Years.Run2016:
            files = self.input_files_2016[: min(max_files, len(self.input_files_2016))]
            random.shuffle(files)
            return files
        if year == Years.Run2017:
            files = self.input_files_2017[: min(max_files, len(self.input_files_2017))]
            random.shuffle(files)
            return files
        if year == Years.Run2018:
            files = self.input_files_2018[: min(max_files, len(self.input_files_2018))]
            random.shuffle(files)
            return files


def get_distributions(analysis_name: str) -> list[tuple[str, bool]]:
    if analysis_name.startswith("EC_"):
        return [
            ("counts", False),
            ("sum_pt", True),
            ("invariant_mass", True),
            ("met", True),
        ]

    return [
        ("invariant_mass", True),
    ]
    # print("ERROR: Could not find distributions for {}.".format(analysis_name))
    # sys.exit(-1)


def make_ec_nice_name(s: str) -> str:
    if s.startswith("EC_"):
        parts = s.replace("+X", "").replace("+NJet", "").split("_")
        result = []

        for i, p in enumerate(parts):
            if i == 0:
                continue

            count = p[0]
            if count != "0":
                result.append(p)

        class_modifier = ""
        if s.endswith("+X"):
            class_modifier = "+X"
        if s.endswith("+NJet"):
            class_modifier = "+NJet"
        return "EC_" + "_".join(result) + class_modifier

    return s


def make_raw_ec_name(nice_name: str) -> str:
    parts = nice_name.replace("+X", "").replace("+NJet", "").split("_")

    n_muons = 0
    n_electrons = 0
    n_taus = 0
    n_photons = 0
    n_bjets = 0
    n_jets = 0
    n_met = 0

    for i, p in enumerate(parts):
        if i == 0:
            continue

        count = p[0]
        if p[1:] == "Muon":
            n_muons = count
            continue
        if p[1:] == "Electron":
            n_electrons = count
            continue
        if p[1:] == "Tau":
            n_taus = count
            continue
        if p[1:] == "Photon":
            n_photons = count
            continue
        if p[1:] == "bJet":
            n_bjets = count
            continue
        if p[1:] == "Jet":
            n_jets = count
            continue
        if p[1:] == "MET":
            n_met = count
            continue

        print(
            "ERROR: Could not create raw EC name from nice name ({})".format(nice_name),
            file=sys.stderr,
        )
        sys.exit(-1)

    class_modifier = ""
    if nice_name.endswith("+X"):
        class_modifier = "+X"
    if nice_name.endswith("+NJet"):
        class_modifier = "+NJet"

    return "EC_{}Muon_{}Electron_{}Tau_{}bJet_{}Jet_{}MET{}".format(
        n_muons, n_electrons, n_taus, n_photons, n_bjets, n_jets, n_met, class_modifier
    )
