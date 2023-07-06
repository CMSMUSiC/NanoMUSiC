#!/usr/bin/env python3

################################################################
## PLOT EVENT COUNTS (validate classes) 3 (with systematics!) ##
################################################################
# plotter for normalization factors

from __future__ import annotations

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from matplotlib.colors import Normalize
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
        "-y",
        "--year",
        required=True,
        help="Year to be processed. ALL for stacking all years.",
    )
    parser.add_argument(
        "-p",
        "--fileprefix",
        help="For normal plotting. Prefix of the root files containing the histograms (including the last '_').",
    )
    parser.add_argument(
        "-s",
        "--savepath",
        help="Searching for files at /validation_outputs/[year]/[savepath]/files/. Exporting plots to /validation_outputs/[year]/[savepath]/plots/",
    )
    parser.add_argument(
        "-t",
        "--title",
        help="File name of the exported plot, default is name of the histogram.",
    )
    parser.add_argument(
        "-ci",
        "--cincludes",
        help="Optional: Plot only classes containing this string in their name.",
    )
    parser.add_argument(
        "-tp",
        "--classestoplot",
        help="List classes to plot, separated with comma.",
        required=True,
    )
    args = parser.parse_args()
    return args


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


# performs a plotting job
def normfacplotter(
    savepath,
    year,
    classes,
):

    # distributions list
    distributions = [
        "$S_{T}$",
        "$m_{inv}$",
        "$m_{T}$",
    ]  # "$MET$ $p_{T}$"]
    # matching histnames
    histnames = [
        "h_sum_pt",
        "h_m_inv",
        "h_m_tr",
    ]  # "h_pt_met"]

    # holds all normalization factors
    matrix = np.zeros((len(distributions), len(classes)))

    # read in normalization factors
    for i in range(len(histnames)):
        for j in range(len(classes)):
            histname = histnames[i]
            classname = classes[j]
            norm_filepath = (
                validation_path + "/" + str(year) + "/" + savepath + "/plots/"
            )
            norm_filepath += f"QCD_normalization_{classname}_{histname}.toml"
            try:
                norm_dict: dict[str, Any] = toml.load(norm_filepath)
                norm_fac = float(norm_dict["normalization"])
                err_norm_fac = float(norm_dict["error"])
                matrix[i][j] = norm_fac
            except:
                True
                # print(f"No normalization for {classname}, {histname}")

    # ----------------- plotting -----------------

    # prepare plot
    hep.style.use(hep.style.ROOT)
    fig, ax = plt.subplots(1, 1)

    left = 0.12
    right = 0.93
    top = 0.95
    bottom = 0.16
    fig.subplots_adjust(left=left, right=right, bottom=bottom, top=top)

    # plot
    im = ax.matshow(
        matrix,
        cmap="YlGnBu",
        norm=Normalize(vmin=0, vmax=1),
        aspect="auto",
    )

    # add colorbar
    divider = make_axes_locatable(ax)
    cax = divider.append_axes("right", size="7%", pad=0.1)
    plt.colorbar(im, cax=cax)

    # change x and y axis labels
    ax.xaxis.set_major_locator(ticker.MultipleLocator(1))
    ax.yaxis.set_major_locator(ticker.MultipleLocator(1))
    # set ticks
    ax.set_xticks([n for n in range(len(matrix[0]))], minor=True)
    ax.set_xticks(
        np.linspace(-0.5, len(matrix[0]) - 0.5, len(matrix[0]) + 1), minor=False
    )
    ax.set_yticks([n for n in range(len(matrix))], minor=True)
    ax.set_yticks(np.linspace(-0.5, len(matrix) - 0.5, len(matrix) + 1), minor=False)
    ax.tick_params(axis="both", which="minor", length=0)
    ax.tick_params(
        axis="x",
        which="minor",
        top=True,
        bottom=True,
        labelbottom=True,
        labeltop=False,
        direction="in",
    )
    ax.tick_params(
        axis="both",
        which="major",
        labelbottom=False,
        labeltop=False,
        labelleft=False,
        labelright=False,
    )
    ax.set_xticklabels(
        [display_classname(classname) for classname in classes],
        minor=True,
        rotation=90,
    )
    ax.set_yticklabels(distributions, minor=True)

    # plot cosmetics and legend
    printdebug("Exporting plot...")

    # add CMS text
    plt.figtext(0.12, 0.958, "CMS", fontsize=19, ha="left", fontweight="bold")
    plt.figtext(
        0.184, 0.958, "Private work", fontsize=13, ha="left", fontstyle="italic"
    )

    # add text in plot
    plt.figtext(
        0.31,
        0.958,
        "QCD normalization $\\alpha_{QCD}$",
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
    ax.set_xlabel("", fontsize=20, loc="right")
    ax.set_ylabel("", fontsize=20, loc="bottom")

    # export plot
    figname = "norm_factors_2D"
    outputpath = validation_path + "/" + str(year) + "/plots/"
    if savepath != "":
        outputpath = validation_path + "/" + str(year) + "/" + savepath + "/plots/"
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

    # classestoplot
    classestoplot = []
    if args.classestoplot:
        for temp in args.classestoplot.split(","):
            classestoplot += [temp]

    # parse years
    year = args.year

    # run plotting task#
    print("Start plot validation job.")
    normfacplotter(
        savepath,
        year,
        classestoplot,
    )

    print("Finished plot validation job.\n")
    exit(0)


if __name__ == "__main__":
    main()
