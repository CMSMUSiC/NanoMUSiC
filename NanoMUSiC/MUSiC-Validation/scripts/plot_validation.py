#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import uproot
import toml
import argparse
from typing import Any
from pprint import pprint
from collections import defaultdict

validation_path = "./validation_outputs"

# debug flag, if true more detailed console output is active
debug = False

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        required=True,
        help='Task configuration (TOML) file, produced by "analysis_config_builder.py"',
    )
    parser.add_argument("-y", "--year", required=True, help="Year to be processed.")
    parser.add_argument(
        "-n", "--histname", required=True, help="Name of the histogram."
    )
    parser.add_argument(
        "-p",
        "--fileprefix",
        required=True,
        help="Prefix of the root files containing the histograms (including the last '_').",
    )
    parser.add_argument(
        "-xl",
        "--xlim",
        required=False,
        help="Optional argument: Set x axis limits, specify limits as 'a,b', where one number can be left out to only set one boundary. When a boundary is not specified, it is set automatically.",
    )
    parser.add_argument(
        "-yl",
        "--ylim",
        required=False,
        help="Optional argument: Set y axis limits, specify limits as 'a,b', where one number can be left out to only set one boundary. When a boundary is not specified, it is set automatically.",
    )

    args = parser.parse_args()
    return args


def extract_config(task_config):
    mcconfig, dataconfig = {}, {}
    print(f"Extracting samples from task config...")
    for sample in task_config:
        if sample != "Lumi" and sample != "Global":
            if not task_config[sample]["is_data"]:  # mc case
                mcconfig.update({sample: task_config[sample]})
            else:  # data case
                dataconfig.update({sample: task_config[sample]})
    print(
        "Found",
        len(mcconfig),
        "mc samples and",
        len(dataconfig),
        "data samples in the selected task config.",
    )
    return mcconfig, dataconfig


# import one histogram from given root file
def import_hist(year, sample, file_prefix, hist_name):
    file_path = (
        validation_path
        + "/"
        + str(year)
        + "/"
        + file_prefix
        + sample
        + "_"
        + str(year)
        + ".root"
    )
    rootfile = uproot.open(file_path)
    hist = rootfile[hist_name]
    counts = hist.values()
    errcounts = hist.errors()
    edges = hist.axis().edges()
    return counts, edges, errcounts


def binerror():
    raise RuntimeError("The edges of the histograms have to be the same for all files.")


def calculatebins(edges):
    bins = np.array([(edges[i + 1] + edges[i]) / 2 for i in range(len(edges) - 1)])
    barwidth = (edges[-1] - edges[0]) / len(bins)
    return bins, barwidth


def main():
    print("\n\n📶 [ MUSiC Validation Plotter 2 ] 📶\n")

    # parse arguments
    args = parse_args()

    # import task config file that includes references to all files that should be validated
    print(f"Importing task config...")
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    # extract data and mc samples given in task config file
    mcconfig, dataconfig = extract_config(task_config)
    datasamples = [sample for sample in dataconfig]
    mcsamples = [sample for sample in mcconfig]

    # sort mc samples in their groups
    mcgroups = set([mcconfig[i]["ProcessGroup"] for i in mcconfig.keys()]) # get set of mc groups
    mcsorted = {}
    for mcgroup in mcgroups: # iterate over all groups
        tempset = set({})
        for sample in mcsamples: # check whether sample belongs to group
            if mcconfig[sample]["ProcessGroup"] == mcgroup:
                tempset.add(sample)
        mcsorted.update({mcgroup : tempset}) # dictionary {group: {members}}
    print("Found", len(mcgroups), "mc groups in the selected task config.")

    # import mc histograms
    print("Importing", len(mcsamples), "mc histograms...")
    mccounts, mcedges, mcerrors = {}, {}, {}
    for sample in mcsamples:
        samplecounts, sampleedges, sampleerrors = import_hist(
            args.year, sample, args.fileprefix, args.histname
        )
        mccounts.update({sample: samplecounts})
        mcedges.update({sample: sampleedges})
        mcerrors.update({sample: sampleerrors})

    # import data histograms
    print("Importing", len(datasamples), "data histograms...")
    datacounts, dataedges, dataerrors = {}, {}, {}
    for sample in datasamples:
        samplecounts, sampleedges, sampleerrors = import_hist(
            args.year, sample, args.fileprefix, args.histname
        )
        datacounts.update({sample: samplecounts})
        dataedges.update({sample: sampleedges})
        dataerrors.update({sample: sampleerrors})

    # validation, check whether all histograms have the same edges
    validation_edges = mcedges[mcsamples[0]]
    validation_len_edges = len(validation_edges)
    for sample in mcsamples:
        if len(mcedges[sample]) != validation_len_edges:
            binerror()
        for i in range(validation_len_edges):
            if mcedges[sample][i] != validation_edges[i]:
                binerror()
    for sample in datasamples:
        if len(dataedges[sample]) != validation_len_edges:
            binerror()
        for i in range(validation_len_edges):
            if dataedges[sample][i] != validation_edges[i]:
                binerror()
        nbins = len(dataedges[sample]) - 1
    print("All histogram edges are matching, therefore the validation can continue.")

    # prepare plot
    fig, ax = plt.subplots(1, 1)

    # import colors
    colors = []
    n = 0
    while n < len(mcgroups): # generate color array from tab colors (one color for each mc group)
        for color in mcolors.TABLEAU_COLORS:
            n += 1
            colors += [color]
            if n == len(mcgroups):
                break

    # calculate bins and bin width (only once since they are the same for every sample)
    edges = validation_edges
    bins, barwidth = calculatebins(edges)

    # stack mc groups, calculate errors and plot mc
    print("Start processing of mc files.")
    n = 0
    mcsum = np.zeros(nbins)
    sqmcerrsum = np.zeros(nbins)
    for group in mcgroups:
        print("   Processing mc group {}...".format(group))
        newgroup = True
        for sample in mcsorted[group]:
            if debug:
                print("      {}...".format(sample))
            counts = np.array(mccounts[sample])
            if newgroup: # add label for first sample
                ax.bar(bins, counts, width=barwidth, bottom=mcsum, label=group, color=colors[n])
                newgroup = False
            else: # dont add label for other samples
                ax.bar(bins, counts, width=barwidth, bottom=mcsum, color=colors[n])
            mcsum += counts # stack mc samples
            sqmcerrsum += (np.array(mcerrors[sample]))**2 # calculate square sum of errors, assuming uncorrelated samples
        n += 1
    mcerr = np.sqrt(sqmcerrsum)
    ax.bar(bins, 2*mcerr, width=barwidth, bottom=(mcsum-mcerr), label="MC uncertainty", fill=False, hatch="xxxxxxxx", linewidth=0, edgecolor="tab:gray")

    # stack data, calculate errors and plot data
    print("Start processing of data files.")
    datasum = np.zeros(nbins)
    sqdataerrsum = np.zeros(nbins)
    for sample in datasamples:
        if debug:
            print("   {}...".format(sample))
        counts = np.array(datacounts[sample])
        datasum += counts # stack data samples
        sqdataerrsum += (np.array(dataerrors[sample]))**2 # calculate square sum of errors, assuming uncorrelated samples
    error_data = np.sqrt(sqdataerrsum) # calculate final error value (square root of square sum)
    ax.errorbar(bins, datasum, yerr=error_data, color="black", marker=".", linestyle="", elinewidth=0.8, capsize=1, markersize=3, label="Data")  

    # axis limits
    print("Setting axis limits...")
    i = 0
    while (mcsum[i] == 0 and datasum[i] == 0):
        i += 1
    j = nbins - 1
    while (mcsum[j] == 0 and datasum[j] == 0):
        j -= 1
    autoxlim = (edges[i], edges[j])
    if args.xlim: # x limits
        try:
            xlim_string = args.xlim.split(",")
            if xlim_string[0] != "" and xlim_string[1] != "":
                ax.set_xlim(float(xlim_string[0]),float(xlim_string[1]))
            elif xlim_string[0] != "" and xlim_string[1] == "":
                ax.set_xlim(float(xlim_string[0]),autoxlim[1])
            elif xlim_string[0] == "" and xlim_string[1] != "":
                ax.set_xlim(autoxlim[0],float(xlim_string[1]))
        except:
            raise RuntimeError("Invalid x limit given.")
            exit(0)
    else:
        ax.set_xlim(autoxlim)
    ax.set_yscale("log")
    autoylim = (1e-2, np.amax([np.amax(mcsum),np.amax(datasum)])*3)
    if args.ylim: # y limits
        try:
            ylim_string = args.ylim.split(",")
            if ylim_string[0] != "" and ylim_string[1] != "":
                ax.set_ylim(float(ylim_string[0]),float(ylim_string[1]))
            elif ylim_string[0] != "" and ylim_string[1] == "":
                ax.set_ylim(float(ylim_string[0]),autoylim[1])
            elif ylim_string[0] == "" and ylim_string[1] != "":
                ax.set_ylim(autoylim[0],float(ylim_string[1]))
        except:
            raise RuntimeError("Invalid y limit given.")
            exit(0)
    else:
        ax.set_ylim(autoylim)

    # plot cosmetics and export
    print("Exporting plot...")
    ax.legend()
    ax.set_title(args.histname)
    fig.tight_layout()
    figname = args.histname
    fig.savefig(validation_path + "/" + str(args.year) + "/" + figname + ".png", dpi=500)

    print("Finished plot validation job.\n")
    exit(0)


if __name__ == "__main__":
    main()
