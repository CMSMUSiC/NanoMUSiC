from pydantic import BaseModel
from metadata import ClassType
import json
import glob
import sys
import numpy as np

from scipy.stats import ks_1samp, uniform


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


class ScanResults(BaseModel):
    class_name: str
    distribution: str
    lower_edge: float
    width: float
    p_value_data: float
    p_values_mc: list[float]
    skipped_scan: bool
    dominant_process_group: str

    @property
    def upper_edge(self) -> float:
        return self.lower_edge + self.width

    @staticmethod
    def make_scan_results(
        scan_result_data_file_path: str,
        scan_mc_data_files: str,
        scan_input_file_path: str,
    ) -> "ScanResults":
        with open(scan_result_data_file_path, "r") as file:
            data = json.load(file)

        p_value_data = data["ScanResults"][0]["CompareScore"]
        class_name = data["name"]
        distribution = data["distribution"]
        lower_edge = data["ScanResults"][0]["lowerEdge"]
        width = data["ScanResults"][0]["width"]

        skipped_data_scan = data["ScanResults"][0]["skippedScan"]

        inv_mass_of_one_object = (
            distribution == "invariant_mass"
            and sum([int(c) for c in class_name if c.isdigit()]) == 1
        )

        has_only_one_tau = (
            "Muon" not in class_name
            and "Electron" not in class_name
            and "Photon" not in class_name
            and "Tau" in class_name
        )
        if has_only_one_tau:
            has_only_one_tau = has_only_one_tau and get_num_taus(class_name) == 1

        p_values_toys = []
        for f in glob.glob(scan_mc_data_files):
            with open(f, "r") as file:
                mc = json.load(file)
                for item in mc["ScanResults"]:
                    if not item["skippedScan"]:
                        p_values_toys.append(item["CompareScore"])

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
            skipped_scan=(
                skipped_data_scan
                or len(p_values_toys) != 10_000
                or inv_mass_of_one_object
                or has_only_one_tau
            ),
            dominant_process_group=get_dominant_process_group(scan_input_file_path),
        )

    def p_tilde(self) -> float | None:
        if self.skipped_scan:
            return None

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
    def count_objects(class_name: str) -> int:
        """For a given event class, it will count the number of physics objects."""
        return sum([int(c) for c in class_name if c.isdigit()])

    def dict(self, *args, **kwargs):
        # Get the original dict representation
        original_dict = super().dict(*args, **kwargs)

        # Add property fields manually
        original_dict["upper_edge"] = self.upper_edge
        original_dict["p_tilde"] = self.p_tilde()
        original_dict.pop("p_values_mc", None)

        return original_dict
