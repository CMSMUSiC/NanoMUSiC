#!/usr/bin/env python3

###########################################
## PLOT VALIDATION 3 (with systematics!) ##
## for new heavy val. with all syst      ##
## normalization code included           ##
###########################################

from __future__ import annotations

import numpy as np
import math
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
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


#################################
# MERGE NORMALIZATION FACTORS
#################################
# merge factors and plot all factors


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
        "-mn",
        "--mergenormalization",
        help="List classes to merge normalization factors (and to plot), separate with comma.",
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

    # specify hist to take normalization from
    distribution = "$S_{T}$"
    histname = "h_sum_pt"

    # read in normalization factors
    print("Read in the normalization factors.")
    norm_fac = []  # normfac
    err_norm_fac = []  # error on normfac
    classes_names = []  # classnames in order with normfac
    for classname in classes:
        norm_filepath = validation_path + "/" + str(year) + "/" + savepath + "/plots/"
        norm_filepath += f"QCD_normalization_{classname}_{histname}.toml"
        norm_dict: dict[str, Any] = toml.load(norm_filepath)
        norm_fac += [float(norm_dict["normalization"])]
        err_norm_fac += [float(norm_dict["error"])]
        classes_names += [classname]
    norm_fac = np.array(norm_fac)
    err_norm_fac = np.array(err_norm_fac)

    # ------ merge normalization factors ------
    norm_fac_merged = -1
    err_norm_fac_merged = 0
    # check whether all factors are compatible within their errors
    difference = []
    for i in range(len(norm_fac)):
        for j in range(len(norm_fac)):
            difference += [np.abs(norm_fac[i] - norm_fac[j])]
    compatible = True
    if np.amax(difference) > np.amin([err_norm_fac[i], err_norm_fac[j]]):
        compatible = False
    # -- weighted mean if compatible --
    if compatible:
        print(
            "Merge normalization factors with WEIGHTED MEAN method since the factors are compatible within their errors."
        )
        norm_fac_merged = np.sum(norm_fac / (err_norm_fac**2)) / np.sum(
            1 / (err_norm_fac**2)
        )
        err_norm_fac_merged = np.sqrt(1 / np.sum(1 / (err_norm_fac**2)))
    # -- if not compatible --
    else:
        print(
            "Merge normalization factors with MEAN method since the factors are not compatible within their errors."
        )
        norm_fac_merged = np.mean(norm_fac)
        err_norm_fac_merged = np.amax(difference)

    # ------ save the merged normalization ------
    print("Save the merged normalization factor.")
    norm_filepath = validation_path + "/" + str(year) + "/plots/"
    if savepath != "":
        norm_filepath = validation_path + "/" + str(year) + "/" + savepath + "/plots/"
    norm_filepath += f"QCD_normalization.toml"
    norm_dict = {}
    norm_dict.update({"normalization": norm_fac_merged})
    norm_dict.update({"error": err_norm_fac_merged})
    with open(norm_filepath, "w") as toml_file:
        toml.dump(norm_dict, toml_file)

    # ----------------- plotting -----------------

    # prepare plot
    print("Start plotting.")
    hep.style.use(hep.style.ROOT)
    fig, ax = plt.subplots(1, 1)
    # plt.axis('on')
    left = 0.11  # 0.11
    right = 0.98
    top = 0.95
    bottom = 0.17  # 0.13
    fig.subplots_adjust(left=left, right=right, bottom=bottom, top=top)

    # plot normalization factors
    x = [n + 0.5 for n in range(len(norm_fac))]
    ax.bar(x, norm_fac, width=1, color="steelblue", label="Normalization factors")
    # plot errors
    ax.bar(
        x,
        2 * err_norm_fac,
        width=1,
        bottom=(norm_fac - err_norm_fac),
        fill=False,
        hatch="xxxxx",
        linewidth=0,
        edgecolor="tab:gray",
        label="Normalization uncertainty",
    )
    # plot mean
    ax.axhline(
        norm_fac_merged, linewidth=2, color="red", label="Average normalization\nfactor"
    )
    # plot mean error
    ax.bar(
        len(classes_names) / 2,
        2 * err_norm_fac_merged,
        width=len(classes_names),
        bottom=(norm_fac_merged - err_norm_fac_merged),
        fill=False,
        hatch="/",
        linewidth=0.5,
        edgecolor="red",
        label="Average uncertainty",
    )

    # add legend
    ax.legend(
        loc="upper right",
        prop={"size": 14},
        bbox_to_anchor=(0.99, 0.985),
        frameon=True,
        facecolor="white",
        framealpha=0.5,
        edgecolor="white",
        fancybox=False,
        ncol=2,
    )

    # add text with mean
    plt.figtext(
        0.14,
        0.9,
        f"$\\alpha_{{QCD}}={np.array(float(norm_fac_merged)).round(decimals=2)}\\pm{np.array(float(err_norm_fac_merged)).round(decimals=2)}$",
        fontsize=18,
        ha="left",
        fontweight="bold",
        color="red",
    )

    # add CMS text
    plt.figtext(0.11, 0.958, "CMS", fontsize=19, ha="left", fontweight="bold")
    plt.figtext(
        0.174, 0.958, "Private work", fontsize=13, ha="left", fontstyle="italic"
    )

    # add text with info
    plt.figtext(
        0.3,
        0.958,
        f"QCD normalization from lepton partner {distribution} distribution",
        fontsize=19,
        ha="left",
    )

    ## add text with lumi info
    # int_lumi = 59.8  # hardcoded for 2018 for now
    # com_energy = 13
    # plt.figtext(
    #    0.982,
    #    0.958,
    #    str(int_lumi) + " fb${}^{-1}$ (" + str(com_energy) + " TeV)",
    #    fontsize=19,
    #    ha="right",
    # )

    # set plot axis labels
    ax.set_xlabel("", fontsize=20, loc="right")
    ax.set_ylabel("", fontsize=20, loc="bottom")

    # set plot limits
    ax.set_ylim(0, 1)
    ax.set_xlim(0, len(classes_names))

    # set y label
    ax.set_ylabel("QCD normalization $\\alpha_{QCD}$")

    # set x ticks (class names)
    ax.set_xticks([n for n in range(len(classes_names) + 1)], minor=False)
    ax.set_xticks([n + 0.5 for n in range(len(classes_names))], minor=True)
    ax.tick_params(
        axis="x",
        which="major",
        top=False,
        bottom=True,
        labelbottom=False,
        labeltop=False,
        direction="out",
    )
    ax.tick_params(
        axis="x",
        which="minor",
        top=False,
        bottom=False,
        labelbottom=True,
        labeltop=False,
        direction="in",
    )
    ax.set_xticklabels(
        [display_classname(classname) + " " for classname in classes_names],
        fontsize="18",
        rotation="vertical",
        color="black",
        ha="center",
        va="top",
        minor=True,
    )

    # export plot
    figname = "norm_factors"
    outputpath = validation_path + "/" + str(year) + "/plots/"
    if savepath != "":
        outputpath = validation_path + "/" + str(year) + "/" + savepath + "/plots/"
    outputpath += figname + ".pdf"
    fig.savefig(outputpath, dpi=500)


###################################################################################################

# note that data sets with no data points at all are not plotted, then the plots are simply skipped

##### MAIN FUNCTION #####
def main():
    print(
        "\n\n📶 [ MUSiC Validation Plotter 4 (Merge and plot normalization factors) ] 📶\n"
    )

    # parse arguments
    args = parse_args()

    # check for sub-directory
    savepath = ""
    if args.savepath:
        savepath = args.savepath

    # classestoplot
    mergenormalization = []
    if args.mergenormalization:
        for temp in args.mergenormalization.split(","):
            mergenormalization += [temp]

    # parse years
    year = args.year

    # run plotting task#
    print("Start plot validation job.")
    normfacplotter(
        savepath,
        year,
        mergenormalization,
    )

    print("Finished plot validation job.\n")
    exit(0)


if __name__ == "__main__":
    main()
