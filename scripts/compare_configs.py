#!/usr/bin/env python3

import tomllib
import sys


def main():
    with open(sys.argv[1], "rb") as file:
        file_1 = tomllib.load(file)

    with open(sys.argv[2], "rb") as file:
        file_2 = tomllib.load(file)

    for sample in file_1:
        if not file_1[sample]["is_data"]:
            if sample in file_2.keys():
                if (
                    file_1[sample]["XSec"] != file_2[sample]["XSec"]
                    or file_1[sample]["FilterEff"] != file_2[sample]["FilterEff"]
                    or file_1[sample]["kFactor"] != file_2[sample]["kFactor"]
                    or (
                        "generator_filter_key" in file_1[sample].keys()
                        and "generator_filter_key" in file_2[sample].keys()
                        and file_1[sample]["generator_filter_key"]
                        != file_2[sample]["generator_filter_key"]
                    )
                ):
                    print(sample)
                    print("XSec", file_1[sample]["XSec"], file_2[sample]["XSec"])
                    print(
                        "FilterEff",
                        file_1[sample]["FilterEff"],
                        file_2[sample]["FilterEff"],
                    )
                    print(
                        "kFactor", file_1[sample]["kFactor"], file_2[sample]["kFactor"]
                    )
                    print(
                        "generator_filter_key",
                        (
                            lambda: file_1[sample]["generator_filter_key"]
                            if "generator_filter_key" in file_1[sample].keys()
                            else "----"
                        )(),
                        (
                            lambda: file_2[sample]["generator_filter_key"]
                            if "generator_filter_key" in file_2[sample].keys()
                            else "----"
                        )(),
                    )
                    print("=" * 30)
            else:
                print("--> Could not find {}.".format(sample))


if __name__ == "__main__":
    main()
