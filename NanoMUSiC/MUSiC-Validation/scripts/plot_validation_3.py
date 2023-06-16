#!/usr/bin/env python3

###########################################
## PLOT VALIDATION 3 (with systematics!) ##
###########################################

from __future__ import annotations

import numpy as np
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
        help="Year to be processed. ALL for stacking all years.",
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
        help="File name of the exported plot, default is name of the histogram.",
    )
    parser.add_argument(
        "-pc", "--plotconfig", help="Plot configuration (TOML) file.", required=True
    )
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
    args = parser.parse_args()
    if (args.fileprefix and args.jetclass) or (
        not args.fileprefix and not args.jetclass
    ):
        raise RuntimeError(
            "Either give --fileprefix in normal plotting mode or --jetclass in jetclass mode. Only one of the arguments is required and allowed."
        )
    return args


# extracts task config
def extract_config(task_config, years):
    mcconfig, dataconfig = {}, {}
    print(f"Extracting samples from task config for years {years}...")
    for year in years:
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


# ----------------------------------------
# ratio uncertainty treatment and corresponding functions: taken from https://github.com/scikit-hep/hist/blob/main/src/hist/intervals.py
def poisson_interval(
    values: np.typing.NDArray[Any],
    variances: np.typing.NDArray[Any] | None = None,
    coverage: float | None = None,
) -> np.typing.NDArray[Any]:
    r"""
    The Frequentist coverage interval for Poisson-distributed observations.

    What is calculated is the "Garwood" interval, c.f.
    `V. Patil, H. Kulkarni (Revstat, 2012) <https://www.ine.pt/revstat/pdf/rs120203.pdf>`_
    or http://ms.mcmaster.ca/peter/s743/poissonalpha.html.
    If ``variances`` is supplied, the data is assumed to be weighted, and the
    unweighted count is approximated by ``values**2/variances``, which effectively
    scales the unweighted Poisson interval by the average weight.
    This may not be the optimal solution: see
    `10.1016/j.nima.2014.02.021 <https://doi.org/10.1016/j.nima.2014.02.021>`_
    (`arXiv:1309.1287 <https://arxiv.org/abs/1309.1287>`_) for a proper treatment.

    In cases where the value is zero, an upper limit is well-defined only in the case of
    unweighted data, so if ``variances`` is supplied, the upper limit for a zero value
    will be set to ``NaN``.

    Args:
        values: Sum of weights.
        variances: Sum of weights squared.
        coverage: Central coverage interval.
          Default is one standard deviation, which is roughly ``0.68``.

    Returns:
        The Poisson central coverage interval.
    """
    if coverage is None:
        coverage = stats.norm.cdf(1) - stats.norm.cdf(-1)
    if variances is None:
        interval_min = stats.chi2.ppf((1 - coverage) / 2, 2 * values) / 2.0
        interval_min[values == 0.0] = 0.0  # chi2.ppf produces NaN for values=0
        interval_max = stats.chi2.ppf((1 + coverage) / 2, 2 * (values + 1)) / 2.0
    else:
        scale = np.ones_like(values)
        mask = np.isfinite(values) & (values != 0)
        np.divide(variances, values, out=scale, where=mask)
        counts: np.typing.NDArray[Any] = values / scale
        interval_min = scale * stats.chi2.ppf((1 - coverage) / 2, 2 * counts) / 2.0
        interval_min[values == 0.0] = 0.0  # chi2.ppf produces NaN for values=0
        interval_max = (
            scale * stats.chi2.ppf((1 + coverage) / 2, 2 * (counts + 1)) / 2.0
        )
        interval_max[values == 0.0] = np.nan
    return np.stack((interval_min, interval_max))


def clopper_pearson_interval(
    num: np.typing.NDArray[Any],
    denom: np.typing.NDArray[Any],
    coverage: float | None = None,
) -> np.typing.NDArray[Any]:
    r"""
    Compute the Clopper-Pearson coverage interval for a binomial distribution.
    c.f. http://en.wikipedia.org/wiki/Binomial_proportion_confidence_interval

    Args:
        num: Numerator or number of successes.
        denom: Denominator or number of trials.
        coverage: Central coverage interval.
          Default is one standard deviation, which is roughly ``0.68``.

    Returns:
        The Clopper-Pearson central coverage interval.
    """
    if coverage is None:
        coverage = stats.norm.cdf(1) - stats.norm.cdf(-1)
    if np.any(num > denom):
        raise ValueError(
            "Found numerator larger than denominator while calculating binomial uncertainty"
        )
    interval_min = stats.beta.ppf((1 - coverage) / 2, num, denom - num + 1)
    interval_max = stats.beta.ppf((1 + coverage) / 2, num + 1, denom - num)
    interval = np.stack((interval_min, interval_max))
    interval[0, num == 0.0] = 0.0
    interval[1, num == denom] = 1.0


def ratio_uncertainty(
    num: np.typing.NDArray[Any],
    denom: np.typing.NDArray[Any],
    uncertainty_type="poisson",
) -> Any:
    r"""
    Calculate the uncertainties for the values of the ratio ``num/denom`` using
    the specified coverage interval approach.

    Args:
        num: Numerator or number of successes.
        denom: Denominator or number of trials.
        uncertainty_type: Coverage interval type to use in the calculation of
         the uncertainties.

         * ``"poisson"`` (default) implements the Garwood confidence interval for
           a Poisson-distributed numerator scaled by the denominator.
           See :func:`hist.intervals.poisson_interval` for further details.
         * ``"poisson-ratio"`` implements a confidence interval for the ratio ``num / denom``
           assuming it is an estimator of the ratio of the expected rates from
           two independent Poisson distributions.
           It over-covers to a similar degree as the Clopper-Pearson interval
           does for the Binomial efficiency parameter estimate.
         * ``"efficiency"`` implements the Clopper-Pearson confidence interval
           for the ratio ``num / denom`` assuming it is an estimator of a Binomial
           efficiency parameter.
           This is only valid if the entries contributing to ``num`` are a strict
           subset of those contributing to ``denom``.

    Returns:
        The uncertainties for the ratio.
    """
    with np.errstate(divide="ignore", invalid="ignore"):
        ratio = num / denom
    if uncertainty_type == "poisson":
        with np.errstate(divide="ignore", invalid="ignore"):
            ratio_variance = num * np.power(denom, -2.0)
        ratio_uncert = np.abs(poisson_interval(ratio, ratio_variance) - ratio)
    elif uncertainty_type == "poisson-ratio":
        p_lim = clopper_pearson_interval(num, num + denom)
        with np.errstate(divide="ignore", invalid="ignore"):
            r_lim: np.typing.NDArray[Any] = p_lim / (1 - p_lim)
            ratio_uncert = np.abs(r_lim - ratio)
    elif uncertainty_type == "efficiency":
        ratio_uncert = np.abs(clopper_pearson_interval(num, denom) - ratio)
    else:
        raise TypeError(
            f"'{uncertainty_type}' is an invalid option for uncertainty_type."
        )
    return ratio_uncert


# ----------------------------------------

# imports, sorts and stacks data and mc from input files
def stacker(
    savepath,
    datasamples,
    mcsamples,
    mcsorted,
    histname,
    color_dict,
    aggregation_dict,
    years,
    fileprefix,
):
    mccounts, mcedges, mcerrors = {}, {}, {}
    datacounts, dataedges, dataerrors = {}, {}, {}
    validation_edges = []
    nbins = 0
    for year in years:
        # import mc histograms
        printdebug(f"Importing {len(mcsamples)} mc histograms for year {year}...")
        for sample in mcsamples:
            samplecounts, sampleedges, sampleerrors = import_hist(
                year, sample, fileprefix, histname, savepath
            )
            mccounts.update({sample: samplecounts})
            mcedges.update({sample: sampleedges})
            mcerrors.update({sample: sampleerrors})

        # import data histograms
        printdebug(f"Importing {len(datasamples)} data histograms for year {year}...")
        for sample in datasamples:
            samplecounts, sampleedges, sampleerrors = import_hist(
                year, sample, fileprefix, histname, savepath
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
            # nbins = len(dataedges[sample]) - 1  # no. of bins is (no. of edges) - 1
        printdebug(
            "All histogram edges are matching, therefore the validation can continue."
        )

    # calculate bins and bin width (only once since they are the same for every sample)
    edges = validation_edges
    bins, barwidth = calculatebins(edges)
    nbins = len(bins)
    printdebug("Calculated bins from histogram edges.")

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

    # stack all samples for each category and calculate errors for each category
    categories_counts, categories_errors = {}, {}
    for category in categories_samples.keys():
        categorysum = np.zeros(nbins)
        sqerr_categorysum = np.zeros(nbins)
        for sample in categories_samples[category]:
            categorysum += np.array(mccounts[sample])
            sqerr_categorysum += (np.array(mcerrors[sample])) ** 2
        categories_counts.update(
            {category: categorysum}
        )  # dictionary: {category: counts for stacked bins for all samples of this category}
        categories_errors.update(
            {category: np.sqrt(sqerr_categorysum)}
        )  # dictionary: {category: combined error after stacking for all samples of this category}
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

    # stack mc categories, calculate errors
    mcsum = np.zeros(nbins)
    sqmcerrsum = np.zeros(nbins)
    categories_stacked = sorted_categories_samples.keys()
    counts_stacked = []
    colors_stacked = []
    for category in categories_stacked:
        counts_stacked += [categories_counts[category]]
        colors_stacked += [categories_colors[category]]
        mcsum += categories_counts[category]
        sqmcerrsum += (
            np.array(categories_errors[category])
        ) ** 2  # add errors together
    mcerr = np.sqrt(sqmcerrsum)  # plot combined mc error

    # stack data, calculate errors
    printdebug("Start data stacking and plotting.")
    datasum = np.zeros(nbins)
    sqdataerrsum = np.zeros(nbins)
    for sample in datasamples:
        counts = np.array(datacounts[sample])
        datasum += counts  # stack data samples
        sqdataerrsum += (
            np.array(dataerrors[sample])
        ) ** 2  # calculate square sum of errors, assuming uncorrelated samples
    error_data = np.sqrt(
        sqdataerrsum
    )  # calculate final error value (square root of square sum)

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
        mcerr,
        datasum,
        error_data,
    )


# ----------------------------------------

# performs a plotting job
def plotter(
    args,
    savepath,
    datasamples,
    mcsamples,
    mcsorted,
    histname,
    color_dict,
    aggregation_dict,
    histproperties,
    years,
    fileprefix,
    classname,
    subfolder,
):
    # import, sort and stack data and mc
    (
        bins,
        edges,
        barwidth,
        nbins,
        categories_stacked,
        categories_counts,
        categories_colors,
        mcsum,
        mcerr,
        datasum,
        error_data,
    ) = stacker(
        savepath,
        datasamples,
        mcsamples,
        mcsorted,
        histname,
        color_dict,
        aggregation_dict,
        years,
        fileprefix,
    )

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
    right = 0.98
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
    ax[0].bar(
        bins,
        2 * mcerr,
        width=barwidth,
        bottom=(mcsum - mcerr),
        fill=False,
        hatch="xxxxxxxx",
        linewidth=0,
        edgecolor="tab:gray",
        label="MC uncertainty",
    )

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
    )

    # create data/mc subplot
    printdebug("Create Data/MC subplot...")
    divisionidx = []
    for i in range(len(mcsum)):
        if mcsum[i] > 0 and datasum[i] > 0:
            divisionidx += [i]
    data_overmc = np.array([datasum[i] / mcsum[i] for i in divisionidx])
    bins_overmc = np.array([bins[i] for i in divisionidx])
    mcerr_overmc = ratio_uncertainty(
        np.array([datasum[i] for i in divisionidx]),
        np.array([mcsum[i] for i in divisionidx]),
        "poisson",
    )
    # asymmetric error interval (contains 2d array with [[-err],[+err]])
    dataerr_overmc = np.array([error_data[i] / mcsum[i] for i in divisionidx])
    barwidth_overmc = np.array([barwidth[i] for i in divisionidx])
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
        mcerr_overmc[1, :] + mcerr_overmc[0, :],
        width=barwidth_overmc,
        bottom=1 - mcerr_overmc[0, :],
        fill=False,
        hatch="xxxxxxxx",
        linewidth=0,
        edgecolor="tab:gray",
    )
    ax[1].axhline(1, linewidth=0.4, color="black")

    # only plot when there is data
    datapresent = False
    for sumpoint in datasum:
        if sumpoint > 0:
            datapresent = True
    if datapresent:
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
            if args.xlimit:  # for xlim argument override the limit in the file
                xlim_string = args.xlimit.split(",")
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
        ylim = (1e-4, ymax)
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
            if args.ylimit:  # for ylim argument override the limit in the file
                ylim_string = args.ylimit.split(",")
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
            return
        leftidx = 0
        rightidx = len(bins_overmc) - 1
        while bins_overmc[leftidx] <= xlim[0]:
            leftidx += 1
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
                    np.amin(
                        [
                            np.amin(
                                [
                                    data_overmc[i] - dataerr_overmc[i] - whitespace2
                                    for i in indices
                                ]
                            ),
                            # np.amin(
                            #    [1 - mcerr_overmc[0, i] - whitespace2 for i in indices]
                            # ),
                        ]
                    ),
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
                    # np.amax([1 + mcerr_overmc[1, i] + whitespace2 for i in indices]),
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
            loc="center", prop={"size": 12}, bbox_to_anchor=(0.9, 0.84), frameon=True
        )

        # add text in plot with class name
        if classname != "":
            fig.suptitle("class: " + classname, ha="left", size=20, x=0.09, y=0.98)

        # set plot title
        plottitle = histname
        if histproperties["title"] != "":
            plottitle = histproperties["title"]
        ax[0].set_title(plottitle, fontsize=25)

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
        if args.jetclass:
            figname = classname + "_" + histname
        if args.title:  # optional custom file title
            figname = args.title
        outputpath = validation_path + "/" + str(args.year) + "/plots/"
        if savepath != "":
            outputpath = (
                validation_path + "/" + str(args.year) + "/" + savepath + "/plots/"
            )
        if subfolder != "":
            outputpath += subfolder + "/"
        outputpath += figname + ".pdf"
        fig.savefig(outputpath, dpi=500)
    plt.close(fig=fig)


###################################################################################################

# note that data sets with no data points at all are not plotted, then the plots are simply skipped

##### MAIN FUNCTION #####
def main():
    print("\n\n📶 [ MUSiC Validation Plotter 2 ] 📶\n")

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

    # parse years
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
    print(f"Plotting job extends over the years {years}.")

    # extract data and mc samples given in task config file
    mcconfig, dataconfig = extract_config(task_config, years)
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
            print(f"Found {len(classnames)} in the selected class config.")
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

    # check output path
    outputpath = validation_path + "/" + str(args.year) + "/plots/"
    if savepath != "":
        outputpath = validation_path + "/" + str(args.year) + "/" + savepath + "/plots/"
    if subfolder != "":
        outputpath += subfolder + "/"
    if not (os.path.isdir(f"{outputpath}")):
        os.system(f"mkdir -p {outputpath}")

    # run plotting task, depending on user input
    nfile = 0
    print(f"Starting plotting job for the {len(fileprefixes)} files {fileprefixes}.")
    for fileprefix in tqdm(fileprefixes):  # run plotter for each file set
        nfile += 1
        # show class name if this is a plotting task of a class
        classname = ""
        if args.jetclass:
            classname = fileprefix[:-1]
            printdebug(
                f"Starting plotting job for class {classname} ({nfile}/{len(fileprefixes)})."
            )
        else:
            printdebug(
                f"Starting plotting job for the given file prefix {fileprefix} ({nfile}/{len(fileprefixes)})."
            )
        # plotting tasks
        if args.histname != "ALL":  # plot single histogram
            histname = args.histname
            printdebug(f"    Starting plotting job for histogram {histname}.")
            plotter(
                args,
                savepath,
                datasamples,
                mcsamples,
                mcsorted,
                histname,
                color_dict,
                aggregation_dict,
                histograms[histname],
                years,
                fileprefix,
                classname,
                subfolder,
            )
        else:  # plot all validation histograms
            printdebug(
                f"    Starting plotting job for all {len(histograms.keys())} histograms for the category."
            )
            n = 0
            for histname in histograms.keys():
                printdebug(f"\nStarting plotting job for histogram {histname}.")
                plotter(
                    args,
                    savepath,
                    datasamples,
                    mcsamples,
                    mcsorted,
                    histname,
                    color_dict,
                    aggregation_dict,
                    histograms[histname],
                    years,
                    fileprefix,
                    classname,
                    subfolder,
                )
                printdebug(
                    f"Finished plotting job for histogram {histname} for the category."
                )

    print("Finished plot validation job.\n")
    exit(0)


if __name__ == "__main__":
    main()
