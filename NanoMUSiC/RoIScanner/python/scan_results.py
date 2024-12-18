from pydantic import BaseModel
import json
import glob
import sys
import numpy as np
from enum import Enum

from scipy.stats import ks_1samp


def ks_to_uniform(observed: list[float]):
    res = ks_1samp(observed, "uniform")
    print(res)
    print(type(res))

    # if res.pvalue < 0.05:
    #     print(
    #         "WARNING: The p-tilde distribution does not follow a uniform distribution."
    #     )


def get_num_taus(class_name: str) -> int:
    index = class_name.find("Tau")

    if index > 0:
        return int(class_name[index - 1])
    elif index == 0:
        print("The Tau is at the start, no character before it.", file=sys.stderr)
        sys.exit(-1)
    else:
        print("Tau not found in main class name.", file=sys.stderr)
        sys.exit(-1)


def get_dominant_process_group(scan_input_file_path: str):
    with open(scan_input_file_path, "r") as file:
        data = json.load(file)
    mc_bins = data["MCBins"]
    events_per_process_group: dict[str, float] = {}
    for bin in mc_bins:
        for process_group in bin["mcEventsPerProcessGroup"]:
            events_per_process_group[process_group] = bin["mcEventsPerProcessGroup"][
                process_group
            ] + events_per_process_group.get(process_group, 0.0)

    return max(events_per_process_group, key=lambda pg: events_per_process_group[pg])


class RoIType(str, Enum):
    excess = "excess"
    deficit = "deficit"


class ScanResults(BaseModel):
    class_name: str
    distribution: str
    lower_edge: float
    width: float
    p_value_data: float
    p_values_mc: list[float]
    p_values_mc_roi_type: list[RoIType]
    skipped_scan: bool
    dominant_process_group: str
    roi_type: RoIType
    data_counts: int
    mc_counts: float
    mc_total_uncert: float

    @property
    def upper_edge(self) -> float:
        return self.lower_edge + self.width

    @staticmethod
    def make_scan_results(
        scan_result_data_file_path: str,
        scan_mc_data_files: str,
        scan_input_file_path: str,
        expected_n_rounds: int = 10_000,
    ) -> "ScanResults":
        with open(scan_result_data_file_path, "r") as file:
            data = json.load(file)

        p_value_data = data["ScanResults"][0]["CompareScore"]

        data_counts = data["ScanResults"][0]["dataEvents"]
        mc_counts = data["ScanResults"][0]["mcEvents"]
        mc_total_uncert = data["ScanResults"][0]["mcTotalUncert"]
        roi_type = RoIType.excess
        if data["ScanResults"][0]["dataEvents"] < data["ScanResults"][0]["mcEvents"]:
            roi_type = RoIType.deficit

        class_name = data["name"]
        distribution = data["distribution"]
        lower_edge = data["ScanResults"][0]["lowerEdge"]
        width = data["ScanResults"][0]["width"]

        skipped_data_scan = data["ScanResults"][0]["skippedScan"]

        inv_mass_of_one_object = (
            distribution == "invariant_mass"
            and sum([int(c) for c in class_name if c.isdigit()]) == 1
        )

        has_only_taus = (
            "Muon" not in class_name
            and "Electron" not in class_name
            and "Photon" not in class_name
            and "Tau" in class_name
        )
        if has_only_taus:
            has_only_taus = has_only_taus and get_num_taus(class_name) == 1

        p_values_toys = []
        p_values_toys_roi_type = []
        for f in glob.glob(scan_mc_data_files):
            with open(f, "r") as file:
                mc = json.load(file)
                for item in mc["ScanResults"]:
                    if not item["skippedScan"]:
                        p_values_toys.append(item["CompareScore"])
                        toy_roi_type = RoIType.excess
                        if item["dataEvents"] < item["mcEvents"]:
                            toy_roi_type = RoIType.deficit
                        p_values_toys_roi_type.append(toy_roi_type)

        if any(([p < 0 or p > 1 for p in p_values_toys])):
            print("ERROR: Found p-values out of bounds.", file=sys.stderr)
            sys.exit(-1)

        return ScanResults(
            class_name=class_name,
            distribution=distribution,
            lower_edge=lower_edge,
            width=width,
            p_value_data=p_value_data,
            p_values_mc=p_values_toys,
            p_values_mc_roi_type=p_values_toys_roi_type,
            skipped_scan=(
                skipped_data_scan
                or len(p_values_toys) != expected_n_rounds
                or inv_mass_of_one_object
                or has_only_taus
            ),
            dominant_process_group=get_dominant_process_group(scan_input_file_path),
            roi_type=roi_type,
            data_counts=data_counts,
            mc_counts=mc_counts,
            mc_total_uncert=mc_total_uncert,
        )

    def p_tilde(self) -> float | None:
        if self.skipped_scan:
            return None

        # counts = np.sum(
        #     (
        #         np.array(self.p_values_mc) <= self.p_value_data
        #         # * (
        #         #     # np.array([roi == self.roi_type for roi in self.p_values_mc_roi_type])
        #         #     <= self.p_value_data
        #         # )
        #     )
        # )
        counts = np.sum(np.array(self.p_values_mc) <= self.p_value_data)
        if counts == 0.0:
            p_tilde = 1 / float(len(self.p_values_mc) + 1)
            return p_tilde

        if counts < 0.0:
            print(
                "ERROR: Could not calculate valid p-tilde. p-tilde can not be less than 0.",
                file=sys.stderr,
            )
            sys.exit(1)

        p_tilde = counts / float(len(self.p_values_mc))
        return float(p_tilde)

    def unsafe_p_tilde(self) -> float:
        p_tilde = self.p_tilde()
        if p_tilde:
            return p_tilde
        return -1.0

    def p_tilde_toys(self) -> list[float] | None:
        if self.skipped_scan:
            return None

        p_tilde_toys = []

        p_values_mc = np.array(self.p_values_mc)
        roi_types = np.array(
            [1 if roi == RoIType.excess else 0 for roi in self.p_values_mc_roi_type]
        )
        # counts = (
        #     np.sum(
        #         p_values_mc[:, None] <= p_values_mc,
        #         # * (roi_types[:, None] == roi_types)
        #         axis=0,
        #     )
        #     - 1
        # )
        counts = np.sum(p_values_mc[:, None] <= p_values_mc, axis=0) - 1
        p_tilde_toys = counts / float(len(p_values_mc) - 1)
        p_tilde_toys[p_tilde_toys == 0.0] = 1 / float(len(self.p_values_mc))

        return p_tilde_toys

    def unsafe_p_tilde_toys(self) -> list[float]:
        p_tilde_toys = self.p_tilde_toys()
        if p_tilde_toys is not None:
            return p_tilde_toys
        return [-1 for _ in range(len(self.p_values_mc))]

    @staticmethod
    def count_objects(class_name: str, count_jets: bool = True) -> int:
        """For a given event class, it will count the number of physics objects."""
        if count_jets:
            return sum([int(c) for c in class_name if c.isdigit()])

        parts = (
            class_name.replace("_X", "")
            .replace("+X", "")
            .replace("_NJet", "")
            .replace("+NJet", "")
            .split("_")
        )

        n_muons: int = 0
        n_electrons: int = 0
        n_taus: int = 0
        n_photons: int = 0
        n_bjets: int = 0
        n_met: int = 0

        for i, p in enumerate(parts):
            if i == 0:
                continue

            count = p[0]
            if p[1:] == "Muon":
                n_muons = int(count)
                continue
            if p[1:] == "Electron":
                n_electrons = int(count)
                continue
            if p[1:] == "Tau":
                n_taus = int(count)
                continue
            if p[1:] == "Photon":
                n_photons = int(count)
                continue
            if p[1:] == "bJet":
                n_bjets = int(count)
                continue
            if p[1:] == "Jet":
                _ = int(count)
                continue
            if p[1:] == "MET":
                n_met = int(count)
                continue

            print(
                "ERROR: Could not count objects class name: {}".format(class_name),
                file=sys.stderr,
            )
            sys.exit(-1)

        return n_muons + n_electrons + n_taus + n_photons + n_bjets + n_met

    def dict(self, *args, **kwargs):
        # Get the original dict representation
        original_dict = super().dict(*args, **kwargs)

        # Add property fields manually
        original_dict["upper_edge"] = self.upper_edge
        original_dict["p_tilde"] = self.p_tilde()
        original_dict.pop("p_values_mc", None)
        original_dict.pop("p_values_mc_roi_type", None)
        original_dict["number_of_toy_roy_type_matches"] = sum(
            [roi == self.roi_type for roi in self.p_values_mc_roi_type]
        )

        return original_dict
