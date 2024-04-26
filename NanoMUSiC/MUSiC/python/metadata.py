from enum import Enum
import sys
import tomli
from pydantic import BaseModel, Field
from typing import Any, Optional


def load_toml(path: str) -> dict[str, Any]:
    with open(path, mode="rb") as fp:
        return tomli.load(fp)


class Years(str, Enum):
    Run2016APV = "2016APV"
    Run2016 = "2016"
    Run2017 = "2017"
    Run2018 = "2018"


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
    XSec: float = Field(default_factory=float)
    FilterEff: float = Field(default_factory=float)
    kFactor: float = Field(default_factory=float)
    XSecOrder: str = Field(default_factory=str)
    ProcessGroup: str = Field(default_factory=str)
    is_data: bool
    das_name_2016APV: list[str] = Field(default_factory=list)
    das_name_2016: list[str] = Field(default_factory=list)
    das_name_2017: list[str] = Field(default_factory=list)
    das_name_2018: list[str] = Field(default_factory=list)
    crab_task_name: list[str]
    output_files_2016APV: list[str] = Field(default_factory=list)
    output_files_2016: list[str] = Field(default_factory=list)
    output_files_2017: list[str] = Field(default_factory=list)
    output_files_2018: list[str] = Field(default_factory=list)

    def get_files(self, year: Years, max_files: int = sys.maxsize) -> list[str]:
        if year == Years.Run2016APV:
            return self.output_files_2016APV[
                : min(max_files, len(self.output_files_2016APV))
            ]
        if year == Years.Run2016:
            return self.output_files_2016[: min(max_files, len(self.output_files_2016))]
        if year == Years.Run2017:
            return self.output_files_2017[: min(max_files, len(self.output_files_2017))]
        if year == Years.Run2018:
            return self.output_files_2018[: min(max_files, len(self.output_files_2018))]
