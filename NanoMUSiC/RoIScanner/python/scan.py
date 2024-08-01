import scanner_imp as scanner
import os
import sys
import json
import numpy as np
from pydantic import BaseModel
from distribution_model import Distribution, DEFAULT_NUM_ROUNDS


class ScanResult(BaseModel):
    event_class_name: str
    distribution: str
    status: int
    roi_start: float
    roi_end: float
    p_data: float
    p_tilde: float


def make_shifts(num_rounds: int, variations: list[str]) -> None:
    shifts = dict(
        zip(
            variations,
            np.random.normal(loc=0.0, scale=1.0, size=(len(variations), num_rounds)),
        )
    )

    with open("shifts.json", "w") as json_file:
        json.dump(shifts, json_file, indent=4)


def pass_quality_control(distribution: Distribution) -> bool:
    return True


def do_scan(
    json_file_path: str,
    output_directory: str = "scan_outputs",
    rounds: int = DEFAULT_NUM_ROUNDS,
    start_round: int = 0,
) -> ScanResult | None:
    lut_file_path = "{}/".format(os.getenv("MUSIC_BASE"))
    shifts_file_path = "shifts.json"

    if not os.path.exists("{}/".format(os.getenv("MUSIC_BASE"))):
        print(
            'ERROR: Could not start scanner. LUT file does not exist. Did you executed "ninja lut"?',
            file=sys.stderr,
        )
        sys.exit(-1)

    if not os.path.exists(json_file_path):
        print(
            "ERROR: Could not start scanner. Input file not found.",
            file=sys.stderr,
        )
        sys.exit(-1)

    with open(json_file_path, "r") as json_file:
        data = json.load(json_file)
    distribution = Distribution(**data)

    if pass_quality_control(distribution):
        data_scan: bool = scanner.scan(
            json_file_path,
            output_directory,
            rounds,
            start_round,
            shifts_file_path,
            lut_file_path,
            scan_type="data",
        )
        mc_scan: bool = scanner.scan(
            json_file_path,
            output_directory,
            rounds,
            start_round,
            shifts_file_path,
            lut_file_path,
            scan_type="mc",
        )

        if data_scan and mc_scan:
            return ScanResult(
                event_class_name="",
                distribution="",
                status=0,
                roi_start=1.0,
                roi_end=1.0,
                p_data=1.0,
                p_tilde=1.0,
            )


def main():
    do_scan("demo.json")


if __name__ == "__main__":
    main()
