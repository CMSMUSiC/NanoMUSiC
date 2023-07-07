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
import matplotlib.patheffects as pe
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
# APPLY NORMALIZATION TO PLOT
#################################

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
        "-n",
        "--histname",
        required=True,
        help="To plot only one histogram, specify the name. Useful when using custom display settings (e.g. -xl, -yl) or to only produce one plot.\nTo plot all histograms (with the same settings/default settings) use '-n ALL'.",
    )
    parser.add_argument(
        "-p",
        "--fileprefix",
        help="For normal plotting. Prefix of the root files containing the histograms (including the last '_').",
    )
    parser.add_argument(
        "-s",
        "--savepath",
        help="Directory of the files inside of /validation_outputs/[year]/ (if they are in a sub-directory). Search for files in the [year] directory if left blank.",
    )
    parser.add_argument(
        "-t",
        "--title",
        help="Optional: Title of the plot.",
    )
    parser.add_argument(
        "-pc", "--plotconfig", help="Plot configuration (TOML) file.", required=True
    )
    parser.add_argument("-j", "--jobs", help="Pool size.", type=int, default=30)
    parser.add_argument(
        "-jc",
        "--jetclass",
        help="Enables plot validation in 'JetClass' mode. Give the names of the class to be plotted separated by comma or the class configuration (TOML) file that includes all classes to be plotted.",
    )
    parser.add_argument(
        "-sf",
        "--subfolder",
        help="Save plot in [subfolder] of validation_outputs/[year]/[savepath]/plots/. If left out, saved in ../plots.",
    )
    parser.add_argument(
        "-xlim",
        "--xlimit",
        help="Optional: Override ylim in plot config. Pass in the format 'min,max'.",
    )
    parser.add_argument(
        "-ylim",
        "--ylimit",
        help="Optional: Override ylim in plot config. Pass in the format 'min,max'.",
    )
    parser.add_argument(
        "-ns",
        "--nosyst",
        help="Optional: Don't use systematics for plotting.",
        action="store_true",
    )
    parser.add_argument(
        "-nt",
        "--normalizethis",
        help="Optional: Normalize the QCD with the information of the file [normalizethis]/plots/QCD_normalization.toml, the subfolder is specified by this argument.",
    )
    parser.add_argument(
        "-is",
        "--injectsignal",
        help="Optional: Inject signal from separate task config (TOML) file given in the argument.",
    )
    parser.add_argument(
        "-samp",
        "--signalamplification",
        help="Optional: Signal amplification factor.",
    )
    parser.add_argument(
        "-qcdu",
        "--addqcduncertainty",
        help="Optional: Include the 50% QCD uncertainty.",
        action="store_true",
    )
    args = parser.parse_args()
    if (args.fileprefix and args.jetclass) or (
        not args.fileprefix and not args.jetclass
    ):
        raise RuntimeError(
            "Either give --fileprefix in normal plotting mode or --jetclass in jetclass mode. Only one of the arguments is required and allowed."
        )
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


# creates arguments for plotter() to be run with multiprocessing
def create_arguments(
    xlimit,
    ylimit,
    jetclass,
    title,
    savepath,
    datasamples,
    mcsamples,
    mcsorted,
    histname,
    color_dict,
    aggregation_dict,
    histproperties,
    year,
    fileprefix,
    classname,
    subfolder,
    plotting_arguments,
    signalsamples,
):
    plotting_arguments.append(
        {
            "xlimit": xlimit,
            "ylimit": ylimit,
            "jetclass": jetclass,
            "title": title,
            "savepath": savepath,
            "savepath": savepath,
            "datasamples": datasamples,
            "datasamples": datasamples,
            "mcsamples": mcsamples,
            "mcsorted": mcsorted,
            "histname": histname,
            "histname": histname,
            "color_dict": color_dict,
            "aggregation_dict": aggregation_dict,
            "histproperties": histproperties,
            "year": year,
            "fileprefix": fileprefix,
            "classname": classname,
            "subfolder": subfolder,
            "signalsamples": signalsamples,
        }
    )
    return plotting_arguments


# imports, sorts and stacks data and mc from input files
# and processes mc errors
def stacker(
    xlimit,
    ylimit,
    jetclass,
    title,
    savepath,
    datasamples,
    mcsamples,
    mcsorted,
    histname,
    color_dict,
    aggregation_dict,
    histproperties,
    year,
    fileprefix,
    classname,
    subfolder,
    signalsamples,
):
    # names of the systematics (mc error) data sets (same as in the heavy validation code named 'shifts')
    systematics = {  # all activated shifts in the Shifts.hpp
        "Nominal",
        "PU_Up",
        "PU_Down",
        "Luminosity_Up",
        "Luminosity_Down",
        "xSecOrder_Up",
        "xSecOrder_Down",
        "PDF_As_Up",
        "PDF_As_Down",
        "PreFiring_Up",
        "PreFiring_Down",
        "JetResolution_Up",
        "JetResolution_Down",
        "JetScale_Up",
        "JetScale_Down",
    }
    if nosyst == True:
        systematics = {"Nominal"}

    mccounts, mcedges = {}, {}  # {systname: {sample: counts}}
    datacounts, dataedges = {}, {}  # {systname: {sample: counts}}
    signalcounts, signaledges = {}, {}  # {systname: {sample: counts}}
    mcstaterrors, datastaterrors, signalstaterrors = {}, {}, {}  # {sample: staterror}
    validation_edges = []
    nbins = 0

    # import mc histograms for every systematic
    printdebug(f"Importing {len(mcsamples)} mc histograms for year {year}...")
    for syst in systematics:
        for sample in mcsamples:
            fileprefix_syst = fileprefix + syst + "_"  # add syst name to file prefix
            samplecounts, sampleedges, sampleerrors = import_hist(
                year, sample, fileprefix_syst, histname, savepath
            )
            if syst in mccounts.keys():
                mccounts[syst].update({sample: samplecounts})
                mcedges[syst].update({sample: sampleedges})
            else:
                mccounts[syst] = {sample: samplecounts}
                mcedges[syst] = {sample: sampleedges}
            if syst == "Nominal":  # fill statistical error
                mcstaterrors.update({sample: sampleerrors})

    # import data histograms (only nominal)
    printdebug(f"Importing {len(datasamples)} data histograms for year {year}...")
    for syst in ["Nominal"]:  # only nominal for data
        for sample in datasamples:
            fileprefix_syst = fileprefix + syst + "_"  # add syst name to file prefix
            samplecounts, sampleedges, sampleerrors = import_hist(
                year, sample, fileprefix_syst, histname, savepath
            )
            if syst in datacounts.keys():
                datacounts[syst].update({sample: samplecounts})
                dataedges[syst].update({sample: sampleedges})
            else:
                datacounts[syst] = {sample: samplecounts}
                dataedges[syst] = {sample: sampleedges}
            if syst == "Nominal":  # fill statistical error
                datastaterrors.update({sample: sampleerrors})

    # ----------- "inject" (add) signal to data ------------
    signalsum = []
    if len(signalsamples) > 0:  # if signal is given
        # read in and inject signal
        printdebug(
            f"Importing {len(signalsamples)} signal histograms for year {year}..."
        )
        for syst in ["Nominal"]:  # only nominal for signal
            for sample in signalsamples:
                # import signal histograms (only nominal)
                fileprefix_syst = (
                    fileprefix + syst + "_"
                )  # add syst name to file prefix
                samplecounts, sampleedges, sampleerrors = import_hist(
                    year, sample, fileprefix_syst, histname, savepath
                )
                # optional signal amplification (default factor 1)
                samplecounts *= signalamplification
                # add signal sample add to data set
                datasamples += [sample]
                # save signal nominal count and inject signal to data
                if syst in signalcounts.keys():
                    signalcounts[syst].update({sample: samplecounts})
                    signaledges[syst].update({sample: sampleedges})
                else:
                    signalcounts[syst] = {sample: samplecounts}
                    signaledges[syst] = {sample: sampleedges}
                datacounts[syst].update({sample: samplecounts})
                dataedges[syst].update({sample: sampleedges})
                if syst == "Nominal":  # fill statistical error
                    signalstaterrors.update({sample: sampleerrors})
                    datastaterrors.update({sample: sampleerrors})

        # stack nominal signal counts for plotting
        signalsum = np.zeros(len(signalcounts["Nominal"][signalsamples[0]]))
        for sample in signalsamples:
            signalsum += signalcounts["Nominal"][sample]
    # ------------------------------------------------------------------

    # validation, check whether all histograms have the same edges
    validation_edges = mcedges["Nominal"][mcsamples[0]]
    validation_len_edges = len(validation_edges)
    for syst in systematics:
        for sample in mcsamples:
            if len(mcedges[syst][sample]) != validation_len_edges:
                binerror()
            for i in range(validation_len_edges):
                if mcedges[syst][sample][i] != validation_edges[i]:
                    binerror()
    for syst in ["Nominal"]:  # only nominal for data
        for sample in datasamples:
            if len(dataedges[syst][sample]) != validation_len_edges:
                binerror()
            for i in range(validation_len_edges):
                if dataedges[syst][sample][i] != validation_edges[i]:
                    binerror()
    printdebug(
        "All histogram edges are matching, therefore the validation can continue."
    )

    # calculate bins and bin width (only once since they are the same for every sample)
    edges = validation_edges
    bins, barwidth = calculatebins(edges)
    nbins = len(bins)
    printdebug("Calculated bins from histogram edges.")

    # check for nan in mc hist
    foundnan = False
    nanlist = set()
    for sample in mcsamples:
        for syst in systematics:
            for i in mccounts[syst][sample]:
                if math.isnan(i):
                    foundnan = True
                    nanlist.add(f"{sample},{syst}")
    if foundnan:
        # simply ignore these errors
        # """ # raise error and dont plot
        raise RuntimeError(
            f"NaN in {fileprefix} {histname} for the mc samples {nanlist}."
        )
        """
        for nancase in nanlist:
            sample = nancase.split(",")[0]
            syst = nancase.split(",")[1]
            mccounts[syst][sample] = np.zeros(nbins)
        """

    # remove 'nominal' from 'systematics' set
    systematics.discard("Nominal")
    # store nominal value in mcnominal
    mcnominal = {}  # {sample: nominal counts}
    for sample in mcsamples:
        mcnominal.update({sample: mccounts["Nominal"][sample]})

    # calculate errors as difference from systematics to nominal
    mcsystematics = (
        {}
    )  # {systname: {sample: error (deviation for systematic from nominal value)}}
    for syst in systematics:
        for sample in mcsamples:
            if syst in mcsystematics.keys():
                mcsystematics[syst].update(
                    {sample: np.abs(mcnominal[sample] - mccounts[syst][sample])}
                )
            else:
                mcsystematics[syst] = {
                    sample: np.abs(mcnominal[sample] - mccounts[syst][sample])
                }

    # re-sort mc groups into mc categories with aggregation dictionary
    categories_samples = {}
    notfoundflag = False
    notfound = set()
    for category in set(aggregation_dict.values()):
        categories_samples.update(
            {category: set()}
        )  # dictionary: {category: {samples in category}}
    for group in mcsorted.keys():
        for sample in mcsorted[group]:
            if group in aggregation_dict.keys():
                categories_samples[aggregation_dict[group]].add(sample)
            else:
                notfoundflag = True
                notfound.add(group)
    if notfoundflag:
        raise RuntimeError(
            f"The mc groups {notfound} are not listed in the aggregation dictionary."
        )
    # exclude categries without any members
    nomembers = set()
    for category in categories_samples.keys():
        if categories_samples[category] == set():
            nomembers.add(category)
    for category in nomembers:
        categories_samples.pop(category)
    # create color dictionary for the mc categories
    categories_colors = {}
    notfoundflag = False
    notfound = set()
    for category in categories_samples.keys():
        if category in color_dict.keys():
            categories_colors.update(
                {category: color_dict[category]}
            )  # dictionary: {category: color}
        else:
            notfoundflag = True
            notfound.add(category)
    if notfoundflag:
        raise RuntimeError(
            f"The mc categories {notfound} are not listed in the color dictionary."
        )
    printdebug(
        f"Sorted {len(mcsorted.keys())} mc groups into {len(categories_samples.keys())} mc categories given by the aggregation dictionary and matched category colors."
    )
    printdebug(
        f"   The mc categories {nomembers} have no member mc groups for the given task config."
    )

    # ------------------- normalize if required ------------------------
    norm_fac = 1
    err_norm_fac = 0
    if normalizethis != "":
        # ------ read in normalization ------
        # apply previously calculated normalization factors
        printdebug(f"Apply the QCD normalization stored in the given file.")
        norm_filepath = (
            validation_path + "/" + str(year) + "/" + normalizethis + "/plots/"
        )
        norm_filepath += f"QCD_normalization.toml"
        norm_dict: dict[str, Any] = toml.load(norm_filepath)
        norm_fac = float(norm_dict["normalization"])
        err_norm_fac = float(norm_dict["error"])

        # ------ apply normalization ------
        # add normalization uncertainty originating from the uncertainty of the normalization factor
        systematics.add("norm")
        mcsystematics.update({"norm": {}})
        for sample in mcsamples:
            if sample in categories_samples["QCD"]:
                mcsystematics["norm"].update(
                    {sample: mcnominal[sample] * err_norm_fac}
                )  # error is *OLD* count times the normalization factor uncertainty, thats why it is calculated before rescaling the nominal counts
            else:
                mcsystematics["norm"].update({sample: np.zeros(nbins)})
        # rescale all QCD samples for all syst with the normalization factor
        for sample in categories_samples["QCD"]:
            mcnominal[sample] *= norm_fac  # rescale nominal
            for syst in systematics:
                if syst != "norm":
                    mcsystematics[syst][
                        sample
                    ] *= norm_fac  # rescale all other syst shifts
    # ----------------------------------

    # stack all samples for each category for the nominal values
    categories_counts = {}
    for category in categories_samples.keys():
        categorysum = np.zeros(nbins)
        for sample in categories_samples[category]:
            categorysum += np.array(mccounts["Nominal"][sample])
        categories_counts.update(
            {category: categorysum}
        )  # dictionary: {category: counts for stacked bins for all samples of this category}
    printdebug("Stacked the samples in each mc category.")

    # sort categories after their comulated contribution (sum of all bins), smallest contribution first
    # categories_samples = {category: samples for category, samples in sorted(categories_samples.items())} # to sort categories alphabetically
    categories_max = {}
    for category in categories_samples.keys():
        categories_max.update(
            {category: np.sum(categories_counts[category])}
        )  # create dictionary with total contribution for each category (bin sum)
    categories_max = {
        k: v for k, v in sorted(categories_max.items(), key=lambda item: item[1])
    }  # reorder after minimum value
    sorted_categories_samples = {}
    for max in categories_max.values():
        category = getkeyfromvalue(categories_max, max)
        sorted_categories_samples.update(
            {category: categories_samples[category]}
        )  # dictionary: {category: {samples in category}}
        # with categories ordered after their cumulated contribution (sum of all bins), smallest contribution first
    printdebug("Sorted mc categories by their maximum contribution.")

    # symmetrize all mc up/down errors to a single error that is applied in both directions
    # the systematics set content is changed to the symmetrized errors as well as the mc counts dict content
    for syst in systematics.copy():
        if "Up" in syst:  # only once for each systematic (choose up_), exclude nominal
            for sample in mcsamples:
                newname = ""
                for i in range(len(syst.split("_")) - 1):
                    newname += syst.split("_")[i] + "_"
                newname = newname[
                    :-1
                ]  # new syst name without up/down prefix (remove last "_")
                systematics.discard(newname + "_Up")
                systematics.discard(newname + "_Down")
                systematics.add(newname)
                newsyst = (
                    np.array(
                        np.abs(mcsystematics[newname + "_Up"][sample])
                        + np.abs(mcsystematics[newname + "_Down"][sample])
                    )
                    / 2
                )  # calculate mean error for each sample and bin
                mcsystematics[newname + "_Up"].pop(sample)
                mcsystematics[newname + "_Down"].pop(sample)
                if newname in mcsystematics.keys():  # update mccounts
                    mcsystematics[newname].update({sample: newsyst})
                else:
                    mcsystematics.update({newname: {sample: newsyst}})
    printdebug("Symmetrized all systematics.")

    # include statistical error for mc
    systematics.add("stat")
    for sample in mcsamples:
        if "stat" in mcsystematics.keys():
            mcsystematics["stat"].update({sample: mcstaterrors[sample]})
        else:
            mcsystematics.update({"stat": {sample: mcstaterrors[sample]}})
    printdebug("Added statistical error.")

    # create combined sums (QCD, Non-QCD)
    # also create all-combined mcsum
    categories_stacked = sorted_categories_samples.keys()
    mcsum = np.zeros(nbins)
    qcd_sum = np.zeros(nbins)
    non_qcd_sum = np.zeros(nbins)
    for category in categories_stacked:
        mcsum += categories_counts[category]
    printdebug("Stacked mc categories (All MC).")

    # calculate all mc errors and merge them for QCD and Non-QCD
    mc_errors = (
        {}
    )  # {systname: merged error for all mc samples} for all mc (merged qcd and non-qcd)
    # calculate stacked error for all samples for each systematic separately
    # usually, for a given systematic the errors are treated fully correlated (linear addition) for all samples and process groups
    # exceptions exist e.g. for the xsection errors where the errors of different groups are assumed to be uncorrelated
    for syst in systematics:
        s_error = np.zeros(nbins)  # for all mc
        # TREAT ALL SAMPLES FULLY CORRELATED
        if syst in [
            "Luminosity",
            "PU",
            "PDF_As",
            "PreFiring",
            "JetResolution",
            "JetScale",
            "norm",
        ]:
            for sample in mcsamples:  # for all mc
                s_error += mcsystematics[syst][sample]
        # TREAT ALL SAMPLES UNCORRELATED
        elif syst in [
            "stat",
        ]:
            for sample in mcsamples:  # for all mc
                s_error += np.array(mcsystematics[syst][sample]) ** 2
            s_error = np.sqrt(s_error)
        # ONLY TREAT ONE GROUP CORRELATED
        elif syst in ["xSecOrder"]:
            # error only for LO order, others have error 0 currently, therefore this code does not differentiate between different orders
            for category in categories_samples.keys():
                if (addqcduncertainty == True) or (
                    addqcduncertainty == False and category != "QCD"
                ):  # only apply XSec error to non-QCD since QCD is normalized and the large XSec error is not used anymore
                    # QCD does not get an xSec uncertainty anymore
                    temp = np.zeros(nbins)
                    for sample in categories_samples[category]:
                        temp += mcsystematics[syst][
                            sample
                        ]  # assumed correlated for samples of the same category
                    s_error += (
                        temp**2
                    )  # assumed uncorrelated for different categories
            s_error = np.sqrt(s_error)
            # also not apply QCD uncertainty here, as dicussed in comment above
        # save error value for systematic source
        mc_errors.update({syst: s_error})
    printdebug("Calculated the mc errors for the different systematics.")

    # combine all different systematic errors together for plotting separately for QCD and Non-QCD
    # assume the different systematic sources to be uncorrelated
    total_mc_errors = np.zeros(nbins)  # holds squared combined errors
    for syst in systematics:
        if syst != "Nominal":
            total_mc_errors += mc_errors[syst] ** 2
    total_mc_errors = np.sqrt(total_mc_errors)  # combined errors
    printdebug("Merged the mc errors.")

    # stack nominal data counts
    datasum = np.zeros(nbins)
    for sample in datasamples:
        counts = np.array(datacounts["Nominal"][sample])
        datasum += counts  # stack data samples
    printdebug("Stacked data.")

    # calculate data errors, only stat error
    total_data_errors = np.zeros(nbins)
    for sample in datasamples:  # stat error for every sample
        total_data_errors += np.array(datastaterrors[sample])
    total_data_errors = np.sqrt(total_data_errors)  # combined data errors
    printdebug("Calculated data errors.")

    # DONT USE THE total_qcd_errors AND total_non_qcd_errors BECAUSE THEY ARE CALCULATED PER BIN
    # THE NORMALIZATION NEEDS THEM CALCULATED OVER THE WHOLE NORMALIZATION INTERVAL

    # ----------------------------------------------------------------------------------

    # return stacked data and mc
    return (
        bins,
        edges,
        barwidth,
        nbins,
        categories_stacked,
        categories_counts,
        categories_colors,
        mcsum,
        total_mc_errors,
        datasum,
        total_data_errors,
        norm_fac,
        err_norm_fac,
        signalsum,
    )


# ----------------------------------------


# performs a plotting job
def plotter(args):
    (
        xlimit,
        ylimit,
        jetclass,
        title,
        savepath,
        datasamples,
        mcsamples,
        mcsorted,
        histname,
        color_dict,
        aggregation_dict,
        histproperties,
        year,
        fileprefix,
        classname,
        subfolder,
        signalsamples,
    ) = list(args.values())
    # ----------------- import, sort and stack data and mc of histogram to plot -----------------

    (
        bins,
        edges,
        barwidth,
        nbins,
        categories_stacked,
        categories_counts,
        categories_colors,
        mcsum,
        error_mc,
        datasum,
        error_data,
        print_norm_fac,
        print_norm_fac_err,
        signalsum,
    ) = stacker(
        xlimit,
        ylimit,
        jetclass,
        title,
        savepath,
        datasamples,
        mcsamples,
        mcsorted,
        histname,
        color_dict,
        aggregation_dict,
        histproperties,
        year,
        fileprefix,
        classname,
        subfolder,
        signalsamples,
    )

    # ----------------- plotting -----------------

    # prepare plot
    hep.style.use(hep.style.ROOT)
    wspace = 0
    if histproperties["wspace"] != "":
        wspace = float(histproperties["wspace"])
    hspace = 0
    if histproperties["hspace"] != "":
        hspace = float(histproperties["hspace"])
    fig, ax = plt.subplots(
        2,
        1,
        gridspec_kw={"height_ratios": [5, 1], "wspace": wspace, "hspace": hspace},
        sharex=True,
    )
    ax[0].set_yscale("log")
    # plt.axis('on')
    left = 0.11
    if histproperties["left"] != "":
        left = float(histproperties["left"])
    right = 0.95  # 0.98
    if histproperties["right"] != "":
        right = float(histproperties["right"])
    top = 0.95
    if histproperties["top"] != "":
        top = float(histproperties["top"])
    bottom = 0.08
    if histproperties["bottom"] != "":
        bottom = float(histproperties["bottom"])
    fig.subplots_adjust(left=left, right=right, bottom=bottom, top=top)

    # plot mc
    ax[0].hist(
        [bins for category in categories_stacked],
        edges,
        label=[category for category in categories_stacked],
        weights=[categories_counts[category] for category in categories_stacked],
        color=[categories_colors[category] for category in categories_stacked],
        histtype="stepfilled",
        stacked=True,
    )
    # plot mc error
    # error_mc: [mcerr_down, mcerr_up] where mcerr_up/down includes the up/down errors for each bin
    ax[0].bar(
        bins,
        error_mc * 2,
        width=barwidth,
        bottom=(mcsum - error_mc),
        fill=False,
        hatch="xxxxx",
        linewidth=0,
        edgecolor="tab:gray",
        label="MC uncertainty",
    )

    datalabel = "Data"
    if len(signalsamples) > 0:
        datalabel = "Data + Signal"
    # plot data
    ax[0].errorbar(
        bins,
        datasum,
        yerr=error_data,
        xerr=barwidth / 2,
        color="black",
        marker=".",
        linestyle="",
        elinewidth=0.8,
        capsize=1,
        markersize=3,
        label=datalabel,
    )

    # optional: plot injected signal
    if len(signalsamples) > 0:
        ax[0].stairs(
            signalsum,
            edges,
            label="Signal",
            color="red",
            linewidth=2,
            path_effects=[pe.Stroke(linewidth=3, foreground="white"), pe.Normal()],
        )

    # only plot when there is data
    datapresent = False
    for sumpoint in datasum:
        if sumpoint > 0:
            datapresent = True
    if datapresent:
        # ----------------- create data/mc subplot -----------------
        # only rescaling the errors with the mcsum, no propagation of some sort, the EC plotter also does not do that
        printdebug("Create Data/MC subplot...")

        # bin merging (somewhat like the EC plotter but a bit different)
        mergebins = True  # enable/disable bin merging in data/mc subplot
        mergethreshold = 1  # combine bins until the mc count for the merged bins exceeds this threshold
        # define new bins, data, mc counts and errors
        new_bins = []
        new_mcsum = []
        new_error_mc = []
        new_datasum = []
        new_error_data = []
        new_barwidth = []
        if (
            mergebins and len(bins) > 1
        ):  # do bin merge (only if merging is possible, therefore nbins > 1)
            # first find the last bin that is plotted (the last one with data)
            plotidx = [0, len(bins) - 1]
            for i in range(len(bins))[
                ::-1
            ]:  # search for first datapoint from the right
                if datasum[i] > 0:
                    plotidx[1] = i
                    break
            for i in range(len(bins)):  # search for first datapoint from the left
                if datasum[i] > 0:
                    plotidx[0] = i
                    break
            newbin = True
            (
                temp_left,
                temp_mcsum,
                temp_error_mc,
                temp_datasum,
                temp_error_data,
                temp_nmerge,
                temp_barwidth,
            ) = (
                0,
                0,
                0,
                0,
                0,
                0,
                0,
            )
            for i in range(plotidx[0], np.amin([plotidx[1] + 1, len(bins) + 1]))[
                ::-1
            ]:  # start at last bin (with data since the axis limits are set there)
                if newbin == True:  # reset variables after every merge
                    temp_nmerge = 1  # count merged bins in order to later calculate the correct bin coordinate
                    temp_left = bins[i] + barwidth[i] / 2
                    temp_mcsum = mcsum[i]
                    temp_error_mc = error_mc[i]  # bin errors assumed correlated
                    temp_datasum = datasum[i]
                    temp_error_data = error_data[i]  # bin errors assumed correlated
                    temp_barwidth = barwidth[i]
                    newbin = False
                else:
                    temp_nmerge += 1
                    temp_mcsum += mcsum[i]
                    temp_error_mc += error_mc[i]  # bin errors assumed correlated
                    temp_datasum += datasum[i]
                    temp_error_data += error_data[i]  # bin errors assumed correlated
                    temp_barwidth += barwidth[i]  # bin widths are adding up
                if (i == plotidx[0]) or (
                    i > 0
                    and (
                        temp_mcsum >= mergethreshold
                        and temp_datasum > 0
                        and mcsum[i - 1] > 0
                    )
                ):  # perform merge when threshold is reached
                    new_bins += [
                        (temp_left + (bins[i] - barwidth[i] / 2)) / 2
                    ]  # new bin is calculated
                    new_mcsum += [temp_mcsum]
                    new_error_mc += [temp_error_mc]
                    new_datasum += [temp_datasum]
                    new_error_data += [temp_error_data]
                    new_barwidth += [temp_barwidth]
                    newbin = True
            new_bins = new_bins[::-1]
            new_mcsum = new_mcsum[::-1]
            new_error_mc = new_error_mc[::-1]
            new_datasum = new_datasum[::-1]
            new_error_data = new_error_data[::-1]
            new_barwidth = new_barwidth[::-1]
        else:  # if no bin merge simply use the old bins
            new_bins = np.copy(bins)
            new_mcsum = np.copy(mcsum)
            new_error_mc = np.copy(error_mc)
            new_datasum = np.copy(datasum)
            new_error_data = np.copy(error_data)
            new_barwidth = np.copy(barwidth)
        # create np arrays
        new_bins = np.array(new_bins)
        new_mcsum = np.array(new_mcsum)
        new_error_mc = np.array(new_error_mc)
        new_datasum = np.array(new_datasum)
        new_error_data = np.array(new_error_data)
        new_barwidth = np.array(new_barwidth)

        # from now on use the (possibly merged) new bins
        # calculate and create data/mc subplot
        divisionidx = []
        for i in range(len(new_mcsum)):
            if new_mcsum[i] > 0:
                divisionidx += [i]
        data_overmc = np.array([new_datasum[i] / new_mcsum[i] for i in divisionidx])
        bins_overmc = np.array([new_bins[i] for i in divisionidx])
        mcerr_overmc = np.array([new_error_mc[i] / new_mcsum[i] for i in divisionidx])
        dataerr_overmc = np.array(
            [new_error_data[i] / new_mcsum[i] for i in divisionidx]
        )
        barwidth_overmc = np.array([new_barwidth[i] for i in divisionidx])
        ax[1].errorbar(
            bins_overmc,
            data_overmc,
            yerr=dataerr_overmc,
            xerr=barwidth_overmc / 2,
            color="black",
            marker=".",
            linestyle="",
            elinewidth=0.8,
            capsize=1,
            markersize=3,
        )
        ax[1].bar(
            bins_overmc,
            mcerr_overmc * 2,
            width=barwidth_overmc,
            bottom=1 - mcerr_overmc,
            fill=False,
            hatch="xxxxx",
            linewidth=0,
            edgecolor="tab:gray",
        )
        ax[1].axhline(1, linewidth=0.4, color="black")

        # ----------------- set axis limits and legends -----------------

        # set all axis limits (this is a bit ugly but it works..)
        printdebug("Setting axis limits...")
        # auto find x limits where data > 0
        leftidx = 0
        rightidx = nbins - 1
        while datasum[leftidx] <= 0:
            leftidx += 1
        while datasum[rightidx] <= 0:
            rightidx -= 1
            if rightidx <= 0:
                rightidx = 0
                break
        if rightidx < nbins:
            rightidx += 1
        whitespace = 0
        xlim = (edges[leftidx] - whitespace, edges[rightidx] + whitespace)
        # read custom x limits
        xlim_string = ["", ""]
        try:
            if histproperties["xlim"] != "":
                xlim_string = histproperties["xlim"].split(",")
            if xlimit:  # for xlim argument override the limit in the file
                xlim_string = xlimit.split(",")
            if xlim_string[0] != "" and xlim_string[1] != "":
                xlim = (float(xlim_string[0]), float(xlim_string[1]))
            elif xlim_string[0] != "" and xlim_string[1] == "":
                xlim = (float(xlim_string[0]), xlim[1])
            elif xlim_string[0] == "" and xlim_string[1] != "":
                xlim = (xlim[0], float(xlim_string[1]))
        except:
            raise RuntimeError("Invalid x limit given.")
            exit(0)
        ax[0].set_xlim(xlim)
        # identify indices of data points inside of determined xlim
        leftidx = 0
        rightidx = nbins - 1
        while bins[leftidx] <= xlim[0]:
            leftidx += 1
        while bins[rightidx] >= xlim[1]:
            rightidx -= 1
            if rightidx <= 0:
                rightidx = 0
                break
        indices = range(leftidx, min([rightidx + 1, nbins]))
        if len(indices) == 0:  # avoid bugs
            printdebug(f"{classname}, {histname}: Skip plotting... [len(indices) == 0]")
            return
        # set y ticks
        ymax = (
            np.amax(
                [
                    np.amax([mcsum[k] for k in indices]),
                    np.amax([datasum[k] for k in indices]),
                ]
            )
            * 3
        )
        # auto find y limits in this index set
        ylim = (2e-4, ymax)
        nmax = 0
        while 10**nmax < ymax:
            nmax += 1
        majoryticks = [10**n for n in range(-4, nmax)]
        ax[0].set_yticks(majoryticks, minor=False)
        minoryticks = [i * n for n in range(0, 11) for i in majoryticks]
        ax[0].set_yticks(minoryticks, minor=True)
        # read custom y limits
        ylim_string = ["", ""]
        try:
            if histproperties["ylim"] != "":
                ylim_string = histproperties["ylim"].split(",")
            if ylimit:  # for ylim argument override the limit in the file
                ylim_string = ylimit.split(",")
            if ylim_string[0] != "" and ylim_string[1] != "":
                ylim = (float(ylim_string[0]), float(ylim_string[1]))
            elif ylim_string[0] != "" and ylim_string[1] == "":
                ylim = (float(ylim_string[0]), ylim[1])
            elif ylim_string[0] == "" and ylim_string[1] != "":
                ylim = (ylim[0], float(ylim_string[1]))
        except:
            raise RuntimeError("Invalid y limit given.")
            exit(0)
        ax[0].set_ylim(ylim)

        # find y limits for data/mc plot
        if len(xlim) == 0 or len(bins_overmc) == 0:  # avoid bugs
            printdebug(
                f"{classname}, {histname}: Skip plotting... [len(xlim) == 0 or len(bins_overmc) == 0]"
            )
            return

        leftidx = 0
        rightidx = len(bins_overmc) - 1
        if len(bins_overmc) > 1:
            while bins_overmc[leftidx] <= xlim[0]:
                leftidx += 1
                if leftidx >= len(bins_overmc):
                    leftidx = 0
                    break
            while bins_overmc[rightidx] >= xlim[1]:
                rightidx -= 1
                if rightidx <= 0:
                    rightidx = 0
                    break
        indices = range(leftidx, min([rightidx + 1, len(bins_overmc)]))
        whitespace2 = 0.1
        ylim2 = (
            np.amax(
                [
                    # np.amin(
                    #    [
                    #        np.amin(
                    #            [
                    #                data_overmc[i] - dataerr_overmc[i] - whitespace2
                    #                for i in indices
                    #            ]
                    #        ),
                    #        # np.amin(
                    #        #    [1 - mcerr_overmc[0, i] - whitespace2 for i in indices]
                    #        # ),
                    #    ]
                    # ),
                    0,
                ]
            ),
            np.amax(
                [
                    np.amax(
                        [
                            data_overmc[i] + dataerr_overmc[i] + whitespace2
                            for i in indices
                        ]
                    ),
                    1 + whitespace2,
                    # np.amax([1 + mcerr_overmc[i] + whitespace2 for i in indices]),
                ]
            ),
        )
        ax[1].set_ylim(ylim2)

        """
        # add grid into plot
        ax[0].grid(which='both', linewidth=0.4, alpha=0.7)
        """

        # plot cosmetics and legend
        printdebug("Exporting plot...")
        ax[0].legend(
            loc="upper right",
            prop={"size": 14},
            bbox_to_anchor=(0.99, 0.98),
            frameon=True,
            facecolor="white",
            framealpha=0.5,
            edgecolor="white",
            fancybox=False,
        )

        # ax[0].legend(loc="center", prop={"size": 12}, bbox_to_anchor=(0.9, 0.84), frameon=True)

        # add CMS text
        plt.figtext(0.11, 0.958, "CMS", fontsize=19, ha="left", fontweight="bold")
        plt.figtext(
            0.174, 0.958, "Private work", fontsize=13, ha="left", fontstyle="italic"
        )

        # add text in plot with class name
        if title != "":  # optional custom title
            plt.figtext(0.3, 0.958, title, fontsize=19, ha="left")
        elif classname != "":
            plt.figtext(
                0.3, 0.958, display_classname(classname), fontsize=19, ha="left"
            )

        # add text with lumi info
        int_lumi = 59.8  # hardcoded for 2018 for now
        com_energy = 13
        plt.figtext(
            0.952,
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

        # add normalized text
        if normalizethis != "":
            plt.figtext(
                0.3,
                0.983,
                f"QCD normalized with $\\alpha_{{QCD}} = {np.array(float(print_norm_fac)).round(decimals=2)}\\pm{np.array(float(print_norm_fac_err)).round(decimals=2)}$",
                fontsize=12,
                ha="left",
            )

        # set plot axis labels
        xlabel = ""
        if histproperties["xlabel"] != "":
            xlabel = histproperties["xlabel"]
        ax[1].set_xlabel(xlabel, fontsize=20)
        ylabel = ""
        if histproperties["ylabel"] != "":
            ylabel = histproperties["ylabel"]
        ax[0].set_ylabel(ylabel, fontsize=20, loc="top")
        ax[1].set_ylabel("Data/MC", fontsize=20, loc="bottom")

        # export plot
        figname = histname
        if jetclass:
            figname = classname + "_" + histname
        outputpath = validation_path + "/" + str(year) + "/plots/"
        if savepath != "":
            outputpath = validation_path + "/" + str(year) + "/" + savepath + "/plots/"
        if subfolder != "":
            outputpath += subfolder + "/"
        if normalizethis != "":
            figname += "_normalized"
        outputpath += figname + ".pdf"
        fig.savefig(outputpath, dpi=500)
    else:
        printdebug(f"{classname}, {histname}: Skip plotting... [No Data]")
    plt.close(fig=fig)


###################################################################################################

# note that data sets with no data points at all are not plotted, then the plots are simply skipped


##### MAIN FUNCTION #####
def main():
    print("\n\n📶 [ MUSiC Validation Plotter 4 ] 📶")
    # print("  (plot, apply QCD normalization or inject signal)\n")

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

    # nosyst flag
    global nosyst
    nosyst = False
    if args.nosyst:
        nosyst = True

    # normalize this
    global normalizethis
    normalizethis = ""
    if args.normalizethis:
        normalizethis = args.normalizethis

    # signal amplification factor
    global signalamplification
    signalamplification = 1
    if args.signalamplification:
        signalamplification = float(args.signalamplification)

    # addqcduncertainty
    global addqcduncertainty
    addqcduncertainty = False
    if args.addqcduncertainty:
        addqcduncertainty = True

    # import task config file that includes references to all files that should be validated
    print(f"Importing task config...")
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    # parse year(s)
    """
    years = set()
    for year_string in (args.year).split(","):
        if year_string == "ALL":  # put all years in
            for valid_year in valid_years:
                years.add(valid_year)
        elif year_string in valid_years:
            years.add(year_string)  # years: set with all years as float
        else:
            raise RuntimeError(
                str(
                    f"The years that were put in are not valid. Valid are only {valid_years}. Separate the years by comma wihout any space. If you want to run all years, type 'ALL'."
                )
            )
    """
    year = args.year

    # extract data and mc samples given in task config file
    mcconfig, dataconfig = extract_config(task_config, year)
    datasamples = [sample for sample in dataconfig]
    mcsamples = [sample for sample in mcconfig]

    # sort mc samples in their groups
    mcgroups = set(
        [mcconfig[i]["ProcessGroup"] for i in mcconfig.keys()]
    )  # get set of mc groups
    mcsorted = {}
    for mcgroup in mcgroups:  # iterate over all groups
        tempset = set()
        for sample in mcsamples:  # check whether sample belongs to group
            if mcconfig[sample]["ProcessGroup"] == mcgroup:
                tempset.add(sample)
        mcsorted.update(
            {mcgroup: tempset}
        )  # dictionary: {group: {samples in this group}}
    print("Found", len(mcgroups), "mc groups in the selected task config.")

    # import plot config file that includes information on the plots to be produced
    print(f"Importing plot config...")
    plot_config_file: str = args.plotconfig
    plot_config: dict[str, Any] = toml.load(plot_config_file)
    if not (("color_dict" in plot_config) or ("aggregation_dict" in plot_config)):
        raise RuntimeError(
            "A color and an aggregation dictionary has to be included in the plot config file."
        )
    color_dict = plot_config["color_dict"]
    aggregation_dict = plot_config["aggregation_dict"]
    histograms = plot_config
    histograms.pop("color_dict")  # rmemove unwanted options from dict
    histograms.pop("aggregation_dict")
    if "COUNTS" in histograms.keys():
        histograms.pop("COUNTS")
    # histograms is a dict {histname: {properties: values}}

    # option: import signal config if given
    # signal is imported as MC samples, all parameters for the MC samples do not matter
    # data samples in the signal config are ignored
    signalsamples = set()
    if args.injectsignal:
        print(f"Importing signal config...")
        signal_config_file: str = args.injectsignal
        signal_config: dict[str, Any] = toml.load(signal_config_file)
        signalconfig, _ = extract_config(signal_config, year)
        signalsamples = [sample for sample in signalconfig]

    # option: jetclass, read in jetclass names to plot
    classnames = (
        set()
    )  # set or array that includes the names of the classes to be validated
    if args.jetclass:
        if ".toml" in args.jetclass:  # class config is given
            print(f"Importing class config...")
            class_config_file: str = args.jetclass
            class_config: dict[str, Any] = toml.load(class_config_file)
            for classname in class_config["to_validate"]:
                classnames.add(classname)
            classnames.discard("COUNTS")
            print(f"Found {len(classnames)} classes in the selected class config.")
        else:  # manual list of classes is given
            for classname in (args.jetclass).split(","):
                classnames.add(classname)
            classnames.discard("COUNTS")
            print(f"Found {len(classnames)} in the given class argument.")

    # create list of fileprefixes for the files to be validated
    fileprefixes = set()
    if args.fileprefix:
        fileprefixes.add(args.fileprefix)
        print(f"Normal plotting with fileprefix {fileprefixes}.")
    if len(classnames) >= 1:
        for classname in classnames:
            fileprefixes.add(classname + "_")
        print(f"Jet class plotting with for the given classes.")

    # read in other arguments
    xlimit = ""
    if args.xlimit:
        xlimit = args.xlimit
    ylimit = ""
    if args.ylimit:
        ylimit = args.ylimit
    title = ""
    if args.title:
        title = args.title
    jetclass = False
    if args.jetclass:
        jetclass = True

    # check output path
    outputpath = validation_path + "/" + str(args.year) + "/plots/"
    if savepath != "":
        outputpath = validation_path + "/" + str(args.year) + "/" + savepath + "/plots/"
    if subfolder != "":
        outputpath += subfolder + "/"
    if not (os.path.isdir(f"{outputpath}")):
        os.system(f"mkdir -p {outputpath}")

    # run plotting task, depending on user input
    print(f"Prepare plotting for the {len(fileprefixes)} files {fileprefixes}.")
    plotting_arguments = []  # stores plotter() arguments for multipool
    for fileprefix in fileprefixes:  # prepare plotting job for each file set
        classname = ""
        if args.jetclass:
            classname = fileprefix[:-1]
        # create plotting arguments
        if args.histname != "ALL":  # plot single histogram
            histname = args.histname
            plotting_arguments = create_arguments(
                xlimit,
                ylimit,
                jetclass,
                title,
                savepath,
                datasamples,
                mcsamples,
                mcsorted,
                histname,
                color_dict,
                aggregation_dict,
                histograms[histname],
                year,
                fileprefix,
                classname,
                subfolder,
                plotting_arguments,
                signalsamples,
            )
        else:  # plot all validation histograms
            for histname in histograms.keys():
                plotting_arguments = create_arguments(
                    xlimit,
                    ylimit,
                    jetclass,
                    title,
                    savepath,
                    datasamples,
                    mcsamples,
                    mcsorted,
                    histname,
                    color_dict,
                    aggregation_dict,
                    histograms[histname],
                    year,
                    fileprefix,
                    classname,
                    subfolder,
                    plotting_arguments,
                    signalsamples,
                )

    # perform plotting with multitasking
    print(f"Start the {len(plotting_arguments)} plotting jobs.")
    with Pool(min(args.jobs, len(plotting_arguments))) as pool:
        list(
            tqdm(
                pool.imap_unordered(plotter, plotting_arguments),
                total=len(plotting_arguments),
                unit="",
            )
        )

    print("Finished plot validation job.\n")
    exit(0)


if __name__ == "__main__":
    main()
