import argparse
import json

import numpy as np
import matplotlib
import matplotlib.pyplot as plt


def read_files(filenames):
    results = {}
    for filename in filenames:
        with open(filename) as file:
            obj = json.load(file)
        timings = obj["timing"]
        with open(obj["JsonFile"]) as file:
            inobj = json.load(file)
        ec_name = inobj["name"]

        if ec_name not in results:
            results[ec_name] = timings
        else:
            results[ec_name]["count"] += timings["count"]
            results[ec_name]["total"] += timings["total"]
            results[ec_name]["mean"] = (
                float(results[ec_name]["count"]) / results[ec_name]["total"]
            )
            results[ec_name]["min"] = min(results[ec_name]["min"], timings["min"])
            results[ec_name]["max"] = max(results[ec_name]["max"], timings["min"])
            results[ec_name]["std"] = None

    return results


def plot_results(results):
    plt.clf()

    rearranged = {}
    ecs_sorted = sorted(results.keys(), key=lambda x: results[x]["roiFinding"]["total"])
    for ec in ecs_sorted:
        for timing, value in results[ec].iteritems():
            if timing not in rearranged:
                rearranged[timing] = []
            rearranged[timing].append(value)

    bottom = np.array([1e-3] * len(ecs_sorted))
    x = np.arange(len(ecs_sorted) + 1)
    width = 0.8 * np.diff(x)
    left = x[:-1] - width / 2

    colors = [
        "#e41a1c",
        "#377eb8",
        "#4daf4a",
        "#984ea3",
        "#ff7f00",
    ]

    legend_proxies = []

    for i, timing in enumerate(rearranged):
        color = colors[i % len(colors)]
        height = np.array(list(value["total"] for value in rearranged[timing]))
        plt.bar(left, height, width=width, bottom=bottom, color=color, label=timing)
        bottom += height

    plt.xticks(x, ecs_sorted)
    plt.yscale("log")
    plt.legend()
    plt.xlabel("Event Class")
    plt.ylabel("Total time in seconds")
    plt.savefig("timing.pdf")


def write_results(results):
    cleaned = {}
    for ec, timings in results.iteritems():
        total = sum(timing["total"] for timing in timings.values())
        count = timings["roiFinding"]["count"]
        cleaned[ec] = total / count

    with open("timing.json", "w") as file:
        json.dump(cleaned, file, indent=2)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", nargs="+", type=str, help=".json files to analyze")
    args = parser.parse_args()
    results = read_files(args.filename)
    plot_results(results)
    write_results(results)
