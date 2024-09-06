from pydantic import BaseModel
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


class ScanResults(BaseModel):
    class_name: str
    distribution: str
    lower_edge: float
    width: float
    p_value_data: float
    p_values_mc: list[float]
    skipped_scan: bool

    @property
    def upper_edge(self) -> float:
        return self.lower_edge + self.width

    @staticmethod
    def make_scan_results(args: tuple[str, str]) -> "ScanResults":
        scan_result_data_file_path, scan_mc_data_files = args

        with open(scan_result_data_file_path, "r") as file:
            data = json.load(file)
        p_value_data = data["ScanResults"][0]["CompareScore"]
        class_name = data["name"]
        distribution = data["distribution"]
        lower_edge = data["ScanResults"][0]["lowerEdge"]
        width = data["ScanResults"][0]["width"]

        skipped_data_scan = data["ScanResults"][0]["skippedScan"]

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

        # ks_to_uniform(p_values_toys)
        # res = ks_1samp(p_values_toys, uniform.cdf)
        # print(res)
        # print(type(res))

        return ScanResults(
            class_name=class_name,
            distribution=distribution,
            lower_edge=lower_edge,
            width=width,
            p_value_data=p_value_data,
            p_values_mc=p_values_toys,
            skipped_scan=skipped_data_scan and len(p_values_toys) != 10_000,
        )

    def p_tilde(self) -> float | None:
        if self.skipped_scan:
            return None

        if np.sum(np.array(self.p_values_mc) <= self.p_value_data) == 0.0:
            p_tilde = 1 / float(len(self.p_values_mc))
            return p_tilde

        if np.sum(np.array(self.p_values_mc) <= self.p_value_data) < 0.0:
            print(
                "ERROR: Could not calculate valid p-tilde. P-Tilde can not be less than 0.",
                file=sys.stderr,
            )
            sys.exit(1)

        p_tilde = np.sum(np.array(self.p_values_mc) <= self.p_value_data) / float(
            len(self.p_values_mc)
        )
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
        p_tilde_toys[p_tilde_toys == 0.0] = 1 / float(len(self.p_values_mc) - 1)

        return p_tilde_toys

    def unsafe_p_tilde_toys(self) -> list[float]:
        p_tilde_toys = self.p_tilde_toys()
        if p_tilde_toys is not None:
            return p_tilde_toys
        return [-1 for _ in range(len(self.p_values_mc))]

    def count_objects(self) -> int:
        """For a given event class, it will count the number of physics objects."""
        return sum([int(c) for c in self.class_name if c.isdigit()])

    def dict(self, *args, **kwargs):
        # Get the original dict representation
        original_dict = super().dict(*args, **kwargs)

        # Add property fields manually
        original_dict["upper_edge"] = self.upper_edge
        original_dict["p_tilde"] = self.p_tilde()
        original_dict.pop("p_values_mc", None)

        return original_dict
