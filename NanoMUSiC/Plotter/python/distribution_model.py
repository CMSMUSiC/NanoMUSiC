from pydantic import BaseModel, NonNegativeFloat, NonNegativeInt, root_validator
from enum import Enum
from typing import Optional

DEFAULT_NUM_ROUNDS: int = 100_000


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
    mcEventsPerProcessGroup: dict[str, float]
    mcStatUncertPerProcessGroup: dict[str, NonNegativeFloat]
    lowerEdge: float
    width: float
    mcSysUncerts: dict[str, float]


class MCBinBuilder:
    def __init__(self) -> None:
        self.mc_bin = MCBin(
            mcEventsPerProcessGroup={},
            mcStatUncertPerProcessGroup={},
            lowerEdge=100.0,
            width=10.0,
            mcSysUncerts={},
        )

    def build(self):
        return self.mc_bin


class Distribution(BaseModel):
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
    NumRounds: Optional[NonNegativeInt] = DEFAULT_NUM_ROUNDS

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

    def save(self, output_directory):
        with open(
            "{}/{}_{}_{}.json".format(
                output_directory, self.name, self.distribution, self.year
            ),
            "w",
        ) as f:
            f.write(self.json(indent=4))


def main():
    dist = Distribution(
        name="FooEC",
        distribution=DistributionType.sum_pt,
        year=ScanYear.Run2018,
        MCBins=[MCBinBuilder().build()],
        DataBins=[12.0],
        sigmaThreshold=0.0,
    )
    dist.save(".")


if __name__ == "__main__":
    main()
