from pydantic import BaseModel, NonNegativeFloat, NonNegativeInt, root_validator
from enum import Enum
import os
from typing import Optional


class ScanYear(str, Enum):
    Run2016 = "Run2016"
    Run2017 = "Run2017"
    Run2018 = "Run2018"
    Run2 = "Run2"

    def __str__(self):
        return self.value


class DistributionType(str, Enum):
    sum_pt = "sum_pt"
    invariant_mass = "invariant_mass"
    met = "met"

    def __str__(self):
        return self.value


class MCBin(BaseModel):
    lowerEdge: float
    width: float
    mcEventsPerProcessGroup: dict[str, float]
    mcStatUncertPerProcessGroup: dict[str, NonNegativeFloat]
    mcSysUncerts: dict[str, float]


class MCBinsBuilder:
    def __init__(self, mc_bin_props) -> None:
        self.mc_bins = []

        for prop in mc_bin_props:
            mcEventsPerProcessGroup = {}
            for process_group, count in prop.mcEventsPerProcessGroup:
                mcEventsPerProcessGroup[str(process_group)] = count

            mcStatUncertPerProcessGroup = {}
            for process_group, count in prop.mcStatUncertPerProcessGroup:
                mcStatUncertPerProcessGroup[str(process_group)] = count

            mcSysUncerts = {}
            for variation, uncert in prop.mcSysUncerts:
                mcSysUncerts[str(variation)] = uncert

            this_bin = MCBin(
                mcEventsPerProcessGroup=mcEventsPerProcessGroup,
                mcStatUncertPerProcessGroup=mcStatUncertPerProcessGroup,
                lowerEdge=prop.lowerEdge,
                width=prop.width,
                mcSysUncerts=mcSysUncerts,
            )
            self.mc_bins.append(this_bin)

    def build(self):
        return self.mc_bins


class ScanDistribution(BaseModel):
    name: str
    distribution: DistributionType
    year: ScanYear
    MCBins: list[MCBin]
    DataBins: list[float]
    minRegionWidth: NonNegativeInt = 3
    coverageThreshold: NonNegativeFloat = 0.0
    regionYieldThreshold: NonNegativeFloat = 0.0
    sigmaThreshold: NonNegativeFloat = 0.0
    integralScan: bool = False
    skipLookupTable: bool = False
    noLowStatsTreatment: bool = False
    widthLowStatsRegions: NonNegativeInt = 4
    thresholdLowStatsDominant: NonNegativeFloat = 0.95
    FirstRound: Optional[NonNegativeInt] = 0
    NumRounds: Optional[NonNegativeInt]

    @root_validator(pre=True)
    def check_bins_length(cls, values):
        if len(values.get("MCBins")) != len(values.get("DataBins")):
            raise ValueError(
                "Could not create Distribution model. MCBins and DataBins have different lengths."
            )

        return values

    @root_validator(pre=True)
    def set_min_region_width(cls, values):
        if values.get("distribution") == DistributionType.invariant_mass:
            values["minRegionWidth"] = 1

        return values

    def save(self, output_directory) -> str:
        os.system(
            "mkdir -p {}/{}".format(
                output_directory,
                self.name.replace("+", "_"),
            )
        )

        output_file = "{}/{}/{}_{}_{}.json".format(
            output_directory,
            self.name.replace("+", "_"),
            self.name.replace("+", "_"),
            self.distribution,
            self.year,
        )
        with open(
            output_file,
            "w",
        ) as f:
            f.write(self.json(indent=4))
        return output_file
