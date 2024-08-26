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
    skipped_scan: bool

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

        skipped_scan = data["ScanResults"][0]["skippedScan"]

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
            skipped_scan=skipped_scan,
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



    def p_tilde_toys(self):
        if self.skipped_scan:
            return None

        p_tilde_toys = []

        # for p_val in self.p_values_mc:
            # if np.sum(np.array(self.p_values_mc) <= p_val) - 1 == 0.0:
            #     p_tilde = 1 / float(len(self.p_values_mc) - 1)
            #     p_tilde_toys.append(p_tilde)
            #     continue

            # p_tilde = np.sum(np.array(self.p_values_mc) <= self.p_value_data) - 1/ float(len(self.p_values_mc) - 1 )
            # p_tilde_toys.append(p_tilde)

        p_values_mc = np.array(self.p_values_mc)
        counts = np.sum(p_values_mc[:, None] <= p_values_mc, axis=0) - 1
        p_tilde_toys = counts / float(len(p_values_mc)-1)
        p_tilde_toys[p_tilde_toys == 0.0] = 1 / float(len(self.p_values_mc)-1)


        return p_tilde_toys

