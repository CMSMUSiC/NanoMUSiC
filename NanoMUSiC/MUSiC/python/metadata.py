from enum import Enum
import random
import sys
import tomli
from pydantic import BaseModel, Field, NonNegativeFloat
from typing import Any, Optional


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
    AltXSec: Optional[NonNegativeFloat] = None
    AltFilterEff: Optional[NonNegativeFloat] = None
    AltkFactor: Optional[NonNegativeFloat] = None
    XSecOrder: str = Field(default_factory=str)
    ProcessGroup: str = Field(default_factory=str)
    generator_filter_key: str = Field(default_factory=str)
    is_data: bool
    das_name_2016APV: list[str] = Field(default_factory=list)
    das_name_2016: list[str] = Field(default_factory=list)
    das_name_2017: list[str] = Field(default_factory=list)
    das_name_2018: list[str] = Field(default_factory=list)
    output_files_2016APV: list[str] = Field(default_factory=list)
    output_files_2016: list[str] = Field(default_factory=list)
    output_files_2017: list[str] = Field(default_factory=list)
    output_files_2018: list[str] = Field(default_factory=list)

    def get_files(self, year: Years, max_files: int = sys.maxsize) -> list[str]:
        if year == Years.Run2016APV:
            files = self.output_files_2016APV[
                : min(max_files, len(self.output_files_2016APV))
            ]
            random.shuffle(files)
            return files
        if year == Years.Run2016:
            files = self.output_files_2016[
                : min(max_files, len(self.output_files_2016))
            ]
            random.shuffle(files)
            return files
        if year == Years.Run2017:
            files = self.output_files_2017[
                : min(max_files, len(self.output_files_2017))
            ]
            random.shuffle(files)
            return files
        if year == Years.Run2018:
            files = self.output_files_2018[
                : min(max_files, len(self.output_files_2018))
            ]
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
