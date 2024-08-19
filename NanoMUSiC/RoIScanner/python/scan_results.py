from pydantic import BaseModel
import json
import glob
import sys
import numpy as np


class ScanResults(BaseModel):
    class_name: str
    distribution: str
    lower_edge: float
    width: float
    p_value_data: float
    p_values_mc: list[float]

    @staticmethod
    def make_scan_results(
        scan_result_data_file_path: str, scan_mc_data_files: str
    ) -> "ScanResults":
        with open(scan_result_data_file_path, "r") as file:
            data = json.load(file)
        p_value_data = data["ScanResults"][0]["CompareScore"]
        class_name = data["name"]
        distribution = data["distribution"]
        lower_edge = data["ScanResults"][0]["lowerEdge"]
        width = data["ScanResults"][0]["width"]
        assert data["ScanResults"][0]["skippedScan"] == False

        p_values_toys = []
        for f in glob.glob(scan_mc_data_files):
            with open(f, "r") as file:
                mc = json.load(file)
                for item in mc["ScanResults"]:
                    if not item["skippedScan"]:
                        p_values_toys.append(item["CompareScore"])

        return ScanResults(
            class_name=class_name,
            distribution=distribution,
            lower_edge=lower_edge,
            width=width,
            p_value_data=p_value_data,
            p_values_mc=p_values_toys,
        )

    def p_tilde(self) -> float:
        if np.sum(np.array(self.p_values_mc) <= self.p_value_data) == 0.0:
            p_tilde = 1 / float(len(self.p_values_mc))

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
