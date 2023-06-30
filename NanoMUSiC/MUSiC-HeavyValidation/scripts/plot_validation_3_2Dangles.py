#!/usr/bin/env python3

###########################################
## PLOT VALIDATION 3 (with systematics!) ##
## 2D plots                              ##
###########################################

from __future__ import annotations

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from matplotlib.colors import LogNorm
from mpl_toolkits.axes_grid1 import make_axes_locatable
from matplotlib import ticker
import uproot
import toml
import argparse
from typing import Any
from pprint import pprint
from collections import defaultdict
from tqdm import tqdm
import matplotlib.gridspec as gridspec
from typing import Any
import numpy as np
import mplhep as hep
import os
from multiprocessing import Pool

try:
    from scipy import stats
except ModuleNotFoundError:
    from sys import stderr

    print(  # noqa: T201
        "hist.intervals requires scipy. Please install hist[plot] or manually install scipy.",
        file=stderr,
    )
    raise
__all__ = ("poisson_interval", "clopper_pearson_interval", "ratio_uncertainty")


def __dir__() -> tuple[str, ...]:
    return __all__


# valid years to enter as an argument
valid_years = {"2016APV", "2016", "2017", "2018"}

# path of the validation files
validation_path = "./validation_outputs"

# debug flag, if true more detailed console output is active
debug = False

# parses arguments
def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        required=True,
        help='Task configuration (TOML) file, produced by "analysis_config_builder.py"',
    )
    parser.add_argument(
        "-y",
        "--year",
        required=True,
        help="Year to be processed. Currently only plotting for one year is possible.",
    )
    parser.add_argument(
        "-d",
        "--distribution",
        required=True,
        help="Distribution to be plotted. 'ALL' for all",
    )
    parser.add_argument(
        "-s",
        "--savepath",
        help="Directory of the files inside of /validation_outputs/[year]/ (if they are in a sub-directory). Search for files in the [year] directory if left blank.",
    )
    parser.add_argument(
        "-sf",
        "--subfolder",
        help="Save plot in [subfolder] of validation_outputs/[year]/[savepath]/plots/. If left out, saved in ../plots.",
    )
    args = parser.parse_args()
    return args


# extracts task config
def extract_config(task_config, year):
    mcconfig, dataconfig = {}, {}
    print(f"Extracting samples from task config for year {year}...")
    for sample in task_config:
        if sample != "Lumi" and sample != "Global":
            if year == "2016":
                if (
                    f"das_name_{year}" in task_config[sample].keys()
                    and "APV" not in task_config[sample].keys()
                ):  # only import samples of the right year
                    if not task_config[sample]["is_data"]:  # mc case
                        mcconfig.update({sample: task_config[sample]})
                    else:  # data case
                        dataconfig.update({sample: task_config[sample]})
            else:
                if (
                    f"das_name_{year}" in task_config[sample].keys()
                ):  # only import samples of the right year
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
def import_hist(year, sample, file_prefix, hist_name, savepath):
    file_path = (
        validation_path
        + "/"
        + str(year)
        + "/files/"
        + file_prefix
        + sample
        + "_"
        + str(year)
        + ".root"
    )
    if savepath != "":
        file_path = (
            validation_path
            + "/"
            + str(year)
            + "/"
            + savepath
            + "/files/"
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


# raises bin error
def binerror():
    raise RuntimeError("The edges of the histograms have to be the same for all files.")


# calculates bins from edges
def calculatebins(edges):
    bins = []
    barwidth = []
    currentx = edges[0]
    for i in range(len(edges) - 1):
        currentwidth = edges[i + 1] - edges[i]
        bins += [currentx + currentwidth / 2]
        barwidth += [currentwidth]
        currentx += currentwidth
    # bins = np.array([(edges[i + 1] + edges[i]) / 2 for i in range(len(edges) - 1)])
    # barwidth = (edges[-1] - edges[0]) / len(bins)
    return np.array(bins), np.array(barwidth)


def getkeyfromvalue(dict, value):
    return [k for k, v in dict.items() if v == value][0]


def printdebug(toprint):
    if debug:
        print(toprint)


def display_classname(classname):
    # required order of the class name:
    # nJ+nBJ+nMET+XJ
    displayclassname = ""
    splitname = classname.split("+")
    alreadyone = False
    # jets
    n = int(splitname[0].split("J")[0])
    if n > 0:
        alreadyone = True
        if n > 1:
            displayclassname += str(n) + "jets"
        else:
            displayclassname += str(n) + "jet"
    # bjets
    n = int(splitname[1].split("BJ")[0])
    if n > 0:
        if alreadyone:
            displayclassname += "+"
        alreadyone = True
        if n > 1:
            displayclassname += str(n) + "bjets"
        else:
            displayclassname += str(n) + "bjet"
    # met
    n = int(splitname[2].split("MET")[0])
    if n > 0:
        if alreadyone:
            displayclassname += "+"
        alreadyone = True
        displayclassname += "MET"
    # jet-/bjet-inclusive/exclusive
    if len(splitname) > 3:
        if splitname[3] == "XJ":
            displayclassname += " j-incl."
        else:
            raise RuntimeError(f"{classname} is no valid class name.")
    else:
        displayclassname += ""
    return displayclassname


# imports, sorts and stacks data and mc from input files
# and processes mc errors
def stacker(
    savepath,
    datasamples,
    mcsamples,
    histname,
    year,
    fileprefix,
    normalize,
):
    mccounts, mcedges = {}, {}  # {systname: {sample: counts}}
    datacounts, dataedges = {}, {}  # {systname: {sample: counts}}
    mcstaterrors, datastaterrors = {}, {}  # {sample: staterror}
    validation_edges = []
    nbins = 0

    # define ptbins
    ptbins = [
        [0, 100],
        [100, 200],
        [200, 300],
        [300, 400],
        [400, 500],
        [500, 600],
        [600, 700],
        [700, 800],
        [800, 900],
        [900, 1000],
    ]
    ptbinsstrgs = [str((str(ptbin[0]) + "_" + str(ptbin[1]))) for ptbin in ptbins]

    # import mc histograms for all ptbins
    for ptbinsstr in ptbinsstrgs:
        for sample in mcsamples:
            fileprefix_syst = fileprefix + "_" + ptbinsstr + "_Nominal_"
            samplecounts, sampleedges, sampleerrors = import_hist(
                year, sample, fileprefix_syst, histname, savepath
            )
            if ptbinsstr in mccounts.keys():
                mccounts[ptbinsstr].update({sample: samplecounts})
                mcedges[ptbinsstr].update({sample: sampleedges})
            else:
                mccounts.update({ptbinsstr: {sample: samplecounts}})
                mcedges.update({ptbinsstr: {sample: sampleedges}})

    # import data histograms for all ptbins
    for ptbinsstr in ptbinsstrgs:
        for sample in datasamples:
            fileprefix_syst = fileprefix + "_" + ptbinsstr + "_Nominal_"
            samplecounts, sampleedges, sampleerrors = import_hist(
                year, sample, fileprefix_syst, histname, savepath
            )
            if ptbinsstr in datacounts.keys():
                datacounts[ptbinsstr].update({sample: samplecounts})
            else:
                datacounts.update({ptbinsstr: {sample: samplecounts}})

    # calculate bins and bin width (only once since they are the same for every sample)
    edges = mcedges[ptbinsstrgs[0]][mcsamples[0]]
    bins, barwidth = calculatebins(edges)
    nbins = len(bins)

    # stack mc for all ptbins and all samples
    mcsum = np.zeros((len(ptbinsstrgs), nbins))
    for i in range(len(ptbinsstrgs)):
        for sample in mcsamples:
            mcsum[i] += np.array(mccounts[ptbinsstrgs[i]][sample])

    # stack data for all ptbins
    datasum = np.zeros((len(ptbinsstrgs), nbins))
    for i in range(len(ptbinsstrgs)):
        for sample in datasamples:
            datasum[i] += np.array(datacounts[ptbinsstrgs[i]][sample])

    # calculate ratio
    ratiosum = np.zeros((len(ptbinsstrgs), nbins))
    for i in range(len(datasum)):
        for j in range(len(datasum[i])):
            if datasum[i, j] == 0 or mcsum[i, j] == 0:
                continue
            ratiosum[i, j] = datasum[i, j] / mcsum[i, j]

    # normalize for better visibility
    if normalize:
        for i in range(len(mcsum)):
            mcsum[i] /= np.sum(mcsum[i])
        for i in range(len(datasum)):
            datasum[i] /= np.sum(datasum[i])

    # return stacked data and mc
    return (
        bins,
        edges,
        barwidth,
        nbins,
        mcsum,
        datasum,
        ratiosum,
    )


# ----------------------------------------

# performs a plotting job
def plotter(
    savepath,
    datasamples,
    mcsamples,
    histname,
    year,
    fileprefix,
    subfolder,
    dataset,
    normalize,
):
    # ----------------- import, sort and stack data and mc -----------------
    (bins, edges, barwidth, nbins, mcsum, datasum, ratiosum,) = stacker(
        savepath,
        datasamples,
        mcsamples,
        histname,
        year,
        fileprefix,
        normalize,
    )

    # ----------------- plotting -----------------

    # prepare plot
    hep.style.use(hep.style.ROOT)
    fig, ax = plt.subplots(1, 1)

    left = 0.11
    right = 0.93
    top = 0.95
    bottom = 0.117
    fig.subplots_adjust(left=left, right=right, bottom=bottom, top=top)

    # reference maps for labels
    pt_edges_2d = [0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000]
    hist_edges_2d = {
        "h_deltar_jetjet": np.linspace(0, 10, 26).round(decimals=2),
        "h_deltaphi_jetjet": np.linspace(-np.pi, np.pi, 21).round(decimals=2),
        "h_deltaeta_jetjet": np.linspace(-3, 3, 21).round(decimals=2),
    }

    # collect plotting data matrix
    im = 0
    matrix = [[0]]
    if dataset == "mc":
        matrix = np.copy(mcsum)
    elif dataset == "data":
        matrix = np.copy(datasum)
    elif dataset == "ratio":
        matrix = np.copy(ratiosum)

    # """ # set appropriate limits
    # cut matrix zero columns from left
    newmatrix = []
    leftcutidx = 0
    isnotempty = False
    for j in range(len(matrix[0])):
        for i in range(len(matrix)):
            if matrix[i][j] > 0:
                isnotempty = True
        if isnotempty:
            leftcutidx = j
            break
    # cut matrix zero columns from right
    newmatrix = []
    rightcutidx = len(matrix[0]) - 1
    isnotempty = False
    for j in range(len(matrix[0]))[::-1]:
        for i in range(len(matrix)):
            if matrix[i][j] > 0:
                isnotempty = True
        if isnotempty:
            rightcutidx = np.amax([leftcutidx, np.amin([j + 1, len(matrix[0]) - 1])])
            break
    newmatrix = np.zeros((len(matrix), rightcutidx - leftcutidx + 1))
    for i in range(len(matrix)):
        for j in range(leftcutidx, rightcutidx + 1):
            newmatrix[i][j - leftcutidx] = matrix[i][j]
    matrix = np.copy(newmatrix)
    # change labels accordingly
    for edge_hist in hist_edges_2d.keys():
        hist_edges_2d[edge_hist] = np.copy(hist_edges_2d[edge_hist])[
            leftcutidx : rightcutidx + 2
        ]
    # """

    # plot
    if not dataset == "ratio":
        if not normalize:
            im = ax.matshow(
                matrix,
                cmap="YlGnBu",
                norm=LogNorm(vmin=0.1, vmax=max([max(r) for r in mcsum])),
                aspect="auto",
            )
        else:
            im = ax.matshow(
                matrix,
                cmap="YlGnBu",
                norm=LogNorm(vmin=0.01, vmax=1),
                aspect="auto",
        )
    else:
        im = ax.matshow(
                matrix,
                cmap="seismic",
                norm=LogNorm(vmin=0.01, vmax=100),
                aspect="auto",
            )


    # add colorbar#
    divider = make_axes_locatable(ax)
    cax = divider.append_axes("right", size="7%", pad=0.1)
    plt.colorbar(im, cax=cax)

    # change x and y axis labels
    ax.xaxis.set_major_locator(ticker.MultipleLocator(1))
    ax.yaxis.set_major_locator(ticker.MultipleLocator(1))
    # set ticks
    ax.tick_params(
        axis="x",
        which="major",
        top=True,
        bottom=True,
        labelbottom=True,
        labeltop=False,
        direction="in",
    )
    ax.set_xticks([], minor=True)
    ax.set_xticks(
        np.linspace(-0.5, len(matrix[0]) - 0.5, len(matrix[0]) + 1), minor=False
    )
    ax.set_yticks([], minor=True)
    ax.set_yticks(np.linspace(-0.5, len(matrix) - 0.5, len(matrix) + 1), minor=False)
    ax.set_xticklabels(
        hist_edges_2d[histname],
        minor=False,
        rotation=90,
    )
    ax.set_yticklabels(pt_edges_2d, minor=False)

    # plot cosmetics and legend
    printdebug("Exporting plot...")

    # add CMS text
    plt.figtext(0.11, 0.958, "CMS", fontsize=19, ha="left", fontweight="bold")
    plt.figtext(
        0.174, 0.958, "Private work", fontsize=13, ha="left", fontstyle="italic"
    )

    # add text in plot
    displayname = ""
    if dataset == "mc":
        displayname = "MC"
    elif dataset == "data":
        displayname = "Data"
    elif dataset == "ratio":
        displayname = "Data/MC ratio"
    if normalize:
        displayname += " (normalized)"
    plt.figtext(
        0.3,
        0.958,
        display_classname(fileprefix) + ", " + displayname,
        fontsize=19,
        ha="left",
    )

    # add text with lumi info
    int_lumi = 59.8  # hardcoded for 2018 for now
    com_energy = 13
    plt.figtext(
        0.87,
        0.958,
        str(int_lumi) + " fb${}^{-1}$ (" + str(com_energy) + " TeV)",
        fontsize=19,
        ha="right",
    )

    """ # leave out title because there is no space
    # set plot title
    plottitle = histname
    if histproperties["title"] != "":
        plottitle = histproperties["title"]
    ax[0].set_title(plottitle, fontsize=19)
    """

    # set plot axis labels
    xlabel = {
        "h_deltar_jetjet": "$|\\Delta R|$(Jet, Jet)",
        "h_deltaphi_jetjet": "$|\\Delta \\phi|$(Jet, Jet) [rad]",
        "h_deltaeta_jetjet": "$|\\Delta \\eta|$(Jet, Jet)",
    }
    ax.set_xlabel(xlabel[histname], fontsize=20, loc="right")
    ax.set_ylabel("$p_{T}$(2nd Jet) [GeV]", fontsize=20, loc="bottom")

    # export plot
    normtext = ""
    if normalize:
        normtext = "_normalized"
    figname = "2D_" + histname + "_" + dataset + normtext
    outputpath = validation_path + "/" + str(year) + "/plots/"
    if savepath != "":
        outputpath = validation_path + "/" + str(year) + "/" + savepath + "/plots/"
    if subfolder != "":
        outputpath += subfolder + "/"
    outputpath += figname + ".pdf"
    fig.savefig(outputpath, dpi=500)


###################################################################################################

# note that data sets with no data points at all are not plotted, then the plots are simply skipped

##### MAIN FUNCTION #####
def main():
    print("\n\n📶 [ MUSiC Validation Plotter 3 2D ] 📶\n")

    # parse arguments
    args = parse_args()

    # check for sub-directory
    savepath = ""
    if args.savepath:
        savepath = args.savepath

    # check for sub-sub-directory
    subfolder = ""
    if args.subfolder:
        subfolder = args.subfolder

    # import task config file that includes references to all files that should be validated
    print(f"Importing task config...")
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    # parse year
    year = args.year

    # extract data and mc samples given in task config file
    mcconfig, dataconfig = extract_config(task_config, year)
    datasamples = [sample for sample in dataconfig]
    mcsamples = [sample for sample in mcconfig]

    # define fileprefix/classname
    fileprefix = "2J+0BJ+0MET"

    # check output path
    outputpath = validation_path + "/" + str(args.year) + "/plots/"
    if savepath != "":
        outputpath = validation_path + "/" + str(args.year) + "/" + savepath + "/plots/"
    if subfolder != "":
        outputpath += subfolder + "/"
    if not (os.path.isdir(f"{outputpath}")):
        os.system(f"mkdir -p {outputpath}")

    # run plotting task, depending on user input
    if args.distribution != "ALL":  # plot single histogram
        histname = args.distribution
        print(f"2D plotting for {histname}...")
        for dataset in ["data", "mc", "ratio"]:
            print("  " + dataset)
            for normalize in [True, False]:
                if dataset == "ratio" and normalize:
                    continue
                plotter(
                    savepath,
                    datasamples,
                    mcsamples,
                    histname,
                    year,
                    fileprefix,
                    subfolder,
                    dataset,
                    normalize,
                )
    else:  # plot all validation histograms
        for histname in ["h_deltar_jetjet", "h_deltaphi_jetjet", "h_deltaeta_jetjet"]:
            print(f"2D plotting for {histname}...")
            for dataset in ["data", "mc", "ratio"]:
                print("  " + dataset)
                for normalize in [True, False]:
                    if dataset == "ratio" and normalize:
                        continue
                    plotter(
                        savepath,
                        datasamples,
                        mcsamples,
                        histname,
                        year,
                        fileprefix,
                        subfolder,
                        dataset,
                        normalize,
                    )

    print("Finished plot validation job.\n")
    exit(0)


if __name__ == "__main__":
    main()
