#!/usr/bin/env python3

################################################################
## PLOT EVENT COUNTS (validate classes) 3 (with systematics!) ##
################################################################

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
        "-pc", "--plotconfig", help="Plot configuration (TOML) file.", required=True
    )
    parser.add_argument(
        "-ct",
        "--classtype",
        help="Specify class types to be validated: Either 'all-incl'/'+X', 'jet-incl'/'+nJ', 'excl'/'+0'. Separate multiple entries by comma. If all three categories should be validated use 'ALL'.",
        required=True,
    )
    args = parser.parse_args()
    return args


# extracts task config
def extract_config(task_config, year):
    mcconfig, dataconfig = {}, {}
    printdebug(f"Extracting samples from task config for years {year}...")
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
def import_counts(year, sample, savepath):
    savepath += "/files/"
    file_prefix = "classes_"
    file_path = (
        validation_path
        + "/"
        + str(year)
        + "/"
        + file_prefix
        + sample
        + "_"
        + str(year)
        + ".toml"
    )
    if savepath != "":
        file_path = (
            validation_path
            + "/"
            + str(year)
            + "/"
            + savepath
            + "/"
            + file_prefix
            + sample
            + "_"
            + str(year)
            + ".toml"
        )
    file_contents = toml.load(file_path)
    return file_contents  # returns dictionary: {systname: {classname: counts}} for the sample


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


# performs a plotting job
def countplotter(
    args,
    savepath,
    datasamples,
    mcsamples,
    mcsorted,
    color_dict,
    aggregation_dict,
    year,
    histproperties,
    classsuffix,
):
    # names of the systematics (mc error) data sets (same as in the heavy validation code named 'systematics')
    systematics = {
        "nominal",
        "up_lumi",
        "down_lumi",
        "up_pu",
        "down_pu",
        "up_xsec",
        "down_xsec",
    }

    mcsamples_classes = {}
    datasamples_classes = {}
    # import mc event counts
    printdebug(f"Importing {len(mcsamples)} mc event counts for year {year}...")
    for sample in mcsamples:
        samplecountdict = import_counts(
            year, sample, savepath
        )  # returns {systname: {classname: counts}}
        mcsamples_classes.update(
            {sample: samplecountdict}
        )  # dictionary: {sample: {systname: {classname: counts}}} for the sample

    # import data event counts
    printdebug(f"Importing {len(datasamples)} data event counts for year {year}...")
    for sample in datasamples:
        samplecountdict = import_counts(
            year, sample, savepath
        )  # returns {systname: {classname: counts}}
        datasamples_classes.update(
            {sample: samplecountdict}
        )  # dictionary: {sample: {systname: {classname: counts}}} for the sample

    # sort samples into class dictionary
    # for mc
    mcclassdict = {}  # dictionary: {classname: {sample: {systname: counts}}}
    for sample in mcsamples:
        for classname in mcsamples_classes[sample]["nominal"].keys():
            templist = {}
            for syst in systematics:
                templist.update({syst: mcsamples_classes[sample][syst][classname]})
            if classname in mcclassdict.keys():
                mcclassdict[classname].update({sample: templist.copy()})
            else:
                mcclassdict.update({classname: {sample: templist.copy()}})
    
    for classname in mcclassdict.keys():
        print(len(mcclassdict[classname].keys()), len(mcsamples))

    """
    for classname in mcclassdict:
        for sample in mcsamples:
            if sample not in mcclassdict[classname].keys():
                print("ERROR:", classname, sample)
    """
                
    # for data
    dataclassdict = {}  # dictionary: {classname: {sample: {systname: counts}}}
    for sample in datasamples:
        for classname in datasamples_classes[sample]["nominal"].keys():
            templist = {}
            for syst in systematics:
                templist.update({syst: datasamples_classes[sample][syst][classname]})
            if classname in dataclassdict.keys():
                dataclassdict[classname].update({sample: templist.copy()})
            else:
                dataclassdict.update({classname: {sample: templist.copy()}})
    print(
        f"There are {len(dataclassdict)} event classes for data and {len(mcclassdict)} event classes for mc in total (no differentiation between inclusive/exclusive)."
    )

    """ # quality control
    for sample in mcsamples:
        for classname in mcsamples_classes[sample]["nominal"].keys():
            if not (sample in mcclassdict[classname].keys()):
                print("ERROR:", sample)
    """

    # analyze only one category of {jet-/bjet-inclusive, exclusive} in the following steps
    # this is specified by the classsuffix
    # discard any classes that should not be analyzed
    dataclasstypedict = {
        "+XJ": {},
        "+0": {},
    }  # dictionary: {suffix: {classname: {sample: {systname: counts}}}} only for classes of the specified type
    mcclasstypedict = {
        "+XJ": {},
        "+0": {},
    }  # dictionary: {suffix: {classname: {sample: {systname: counts}}}} for all classes of the specified type that have data > 0
    nclass = {
        "MC": {"+XJ": 0, "+0": 0},
        "data": {"+XJ": 0, "+0": 0},
    }  # holds total number of classes
    # suffixes are +XJ: all-jet-inclusive, +0: exclusive
    toremove = set()
    if classsuffix in ["+XJ", "+0"]:
        for classname in dataclassdict.keys():
            if "+XJ" in classname:  # all-jet-inclusive classes
                nclass["data"]["+XJ"] += 1
                if classname in dataclasstypedict["+XJ"].keys():
                    dataclasstypedict["+XJ"][classname].update(
                        dataclassdict[classname].copy()
                    )
                else:
                    dataclasstypedict["+XJ"].update(
                        {classname: dataclassdict[classname].copy()}
                    )
                toremove.add(classname)
        for classname in toremove:
            dataclassdict.pop(classname)
        # since all bjet-/jet-inclusive classes are removed, the remaining classes are the exclusive ones
        dataclasstypedict["+0"] = dataclassdict.copy()
        nclass["data"]["+0"] = len(dataclasstypedict["+0"])
        # fill mc classes with data > 0

        zerocounts = {}
        for sample in mcsamples:
            for syst in systematics:
                if sample in zerocounts.keys():
                    zerocounts[sample].update({syst: 0})
                else:
                    zerocounts.update({sample: {syst: 0}})
        toremove = set()
        # inclusive
        for classname in mcclassdict.keys():
            if "+XJ" in classname:
                nclass["MC"]["+XJ"] += 1
                if classname in dataclasstypedict["+XJ"].keys():
                    if classname in mcclasstypedict["+XJ"].keys():
                        mcclasstypedict["+XJ"][classname].update(
                            mcclassdict[classname].copy()
                        )
                    else:
                        mcclasstypedict["+XJ"].update(
                            {classname: mcclassdict[classname].copy()}
                        )
                    toremove.add(classname)
        for classname in toremove:
            mcclassdict.pop(classname)
        # remaining classes are exclusive
        for classname in mcclassdict.keys():
            nclass["MC"]["+0"] += 1
            if (
                classname in dataclasstypedict["+0"].keys()
            ):  # only take the classes with at least one data point
                if (
                    classname in mcclasstypedict["+0"].keys()
                ):  # check if key already exists
                    if (
                        classname in mcclassdict.keys()
                    ):  # check whether this class already exists in mc, else fill zeros
                        mcclasstypedict["+0"][classname].update(
                            mcclassdict[classname].copy()
                        )
                    else:  # if no mc for this class fill zeros
                        mcclasstypedict["+0"].update({classname: zerocounts.copy()})
                else:
                    if (
                        classname in mcclassdict.keys()
                    ):  # check whether this class already exists in mc, else fill zeros
                        mcclasstypedict["+0"].update(
                            {classname: mcclassdict[classname].copy()}
                        )
                    else:  # if no mc for this class fill zeros
                        mcclasstypedict["+0"].update({classname: zerocounts.copy()})

    # print result
    if classsuffix == "+XJ":
        specifier = "jet-/bjet-inclusive"
    elif classsuffix == "+0":
        specifier = "exclusive"
    else:
        raise RuntimeError(f"No valid class type specified.")
    print(f"-- ANAYLZE {specifier} CLASSES: --")
    print(
        f"There are {nclass['data'][classsuffix]} {specifier} classes in data and {nclass['MC'][classsuffix]} {specifier} classes in mc."
    )
    print("Only the classes with data are now analyzed.")

    # quality control
    if (mcclasstypedict["+0"].keys() != dataclasstypedict["+0"].keys()) or (
        mcclasstypedict["+XJ"].keys() != dataclasstypedict["+XJ"].keys()
    ):
        raise RuntimeError(f"Not the same keys!")

    # re-sort mc groups into mc categories with aggregation dictionary
    categories_samples = {}  # dictionary: {category: {samples in category}}
    notfoundflag = False
    notfound = set()
    for category in set(aggregation_dict.values()):
        categories_samples.update({category: set()})
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

    """
    for classname in mcclasstypedict[classsuffix].keys():
        for sample in mcsamples:
            if not (sample in mcclasstypedict[classsuffix][classname].keys()):
                print("ERROR:", classname, sample)
    """

    # stack all mc samples for each category for the nominal values
    # mc/data classtypedict: {suffix: {classname: {sample: {systname: counts}}}}
    categories_counts = {}
    for classname in mcclasstypedict[classsuffix].keys():
        for category in categories_samples.keys():
            categorysum = 0
            for sample in categories_samples[category]:
                categorysum += np.array(
                    mcclasstypedict[classsuffix][classname][sample]["nominal"]
                )
            categories_counts.update(
                {category: categorysum}
            )  # dictionary: {category: {nominal counts for stacked bins for all samples of this category}}
    printdebug("Stacked the samples in each mc category.")

    print(categories_counts)

    ######## WORK IN PROGRESS ########
    print("")
    raise RuntimeError(f"SHOULD WORK UP TO HERE.")

    # stack all event counts for each category for mc
    classes_categories = {}  # dictionary: {classname: {category: count}} for mc
    # fill count dict for every class
    for classname in dataclassdict.keys():
        for category in categories_samples.keys():
            # add counts for all samples in every category
            counts = 0
            for sample in categories_samples[category]:
                if sample in mcclassdict[classname]:
                    counts += mcclassdict[classname][sample]
            # save count sum per category in dict
            if classname in classes_categories.keys():
                classes_categories[classname].update({category: counts})
            else:
                classes_categories.update({classname: {category: counts}})
    printdebug("Stacked the event counts for each mc category.")

    # sort mc categories after their contribution for each class
    for classname in classes_categories.keys():
        classes_categories[classname] = {
            k: v
            for k, v in sorted(
                classes_categories[classname].items(), key=lambda item: item[1]
            )
        }  # reorder after minimum value
        # category with lowest event count first
    printdebug("Sorted mc categories by their maximum contribution.")

    # stack all event counts for data
    classes_data = {}  # dictionary: {classname: count} for data
    for classname in dataclassdict.keys():
        counts = 0
        for sample in dataclassdict[classname].keys():
            counts += dataclassdict[classname][sample]
        classes_data.update({classname: counts})
    printdebug("Stacked the event counts for data.")

    # sort classes after the event counts in data (first the highest count)
    for classname in classes_data.keys():
        classes_data = {
            k: v
            for k, v in sorted(
                classes_data.items(), key=lambda item: item[1], reverse=True
            )
        }  # reorder after minimum value
        # category with lowest event count first
    printdebug("Sorted classes by their maximum data event count contribution.")

    # in classes_data and classes_categories the classes (keys) are the same, these are the classes that have data > 0
    # classes_data: {classname: count} for data
    # classes_categories: {classname: {category: count}} for mc

    # create set of all classes
    allclasses = set(classes_data.keys())
    # create set of all categories
    allcategories = set()
    for classname in allclasses:
        for category in classes_categories[classname].keys():
            allcategories.add(category)
    # fill in 0 event count for all categories that are not present for the classes
    # so that in the end of this process all categories are listed in all classes in classes_categories
    # that makes plotting easier
    for classname in allclasses:
        for category in allcategories:
            if not (category in classes_categories[classname]):
                classes_categories[classname].update({category: 0})

    # ---------------------- reduce the data sets to fewer sets that should be plotted ----------------------

    # default selection: 30 most inhabited classes
    classes_data_toplot = {}
    n = 0
    for classname in classes_data.keys():
        if n >= 30:
            break
        n += 1
        classes_data_toplot.update({classname: classes_data[classname]})
    print(
        f"The <=30 classes with the highest data event counts are:\n{classes_data_toplot}"
    )
    classes_categories_toplot = {}
    for classname in classes_data_toplot.keys():
        for category in classes_categories[classname]:
            if classname in classes_categories_toplot.keys():
                classes_categories_toplot[classname].update(
                    {category: classes_categories[classname][category]}
                )
            else:
                classes_categories_toplot.update(
                    {classname: {category: classes_categories[classname][category]}}
                )

    # reduced class/category list
    allclasses_toplot = set(classes_data_toplot.keys())
    allcategories_toplot = set()
    for classname in allclasses_toplot:
        for category in classes_categories_toplot[classname].keys():
            allcategories_toplot.add(category)

    # number of classes that are plotted
    nbins = len(classes_data_toplot)

    # ---------------------- start plotting the class counts ----------------------

    # prepare plot
    print("Start plotting.")
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
    bottom = 0.13
    if histproperties["bottom"] != "":
        bottom = float(histproperties["bottom"])
    fig.subplots_adjust(left=left, right=right, bottom=bottom, top=top)

    # plot mc
    printdebug("Start mc plotting.")
    xstart = 0.5
    x = xstart
    firstrun = True
    mcsum = []
    barplot = []
    barlabel = []
    for classname in classes_data_toplot.keys():
        countsum = 0
        for category in classes_categories_toplot[classname].keys():
            if firstrun:
                tempplot = ax[0].bar(
                    x,
                    classes_categories_toplot[classname][category],
                    width=1,
                    bottom=countsum,
                    color=categories_colors[category],
                )
                barplot += [tempplot]
                barlabel += [category]
            else:
                ax[0].bar(
                    x,
                    classes_categories_toplot[classname][category],
                    width=1,
                    bottom=countsum,
                    color=categories_colors[category],
                )
            countsum += classes_categories_toplot[classname][category]
            mcerr = np.sqrt(countsum)
        x += 1
        firstrun = False
        mcsum += [countsum]
    mcerr = np.sqrt(np.array(mcsum))
    mcerrorplot = ax[0].bar(
        [xstart + i for i in range(len(classes_data_toplot.keys()))],
        2 * mcerr,
        width=1,
        bottom=(mcsum - mcerr),
        fill=False,
        hatch="xxxxxxxx",
        linewidth=0,
        edgecolor="tab:gray",
    )

    # plot data
    printdebug("Start data plotting.")
    x = xstart
    datasum = []
    firstrun = True
    for classname in classes_data_toplot.keys():
        datasum += [classes_data_toplot[classname]]
        if firstrun:
            dataplot = ax[0].errorbar(
                x,
                classes_data_toplot[classname],
                yerr=np.sqrt(classes_data_toplot[classname]),
                xerr=1 / 2,
                color="black",
                marker=".",
                linestyle="",
                elinewidth=0.8,
                capsize=1,
                markersize=3,
            )
        else:
            ax[0].errorbar(
                x,
                classes_data_toplot[classname],
                yerr=np.sqrt(classes_data_toplot[classname]),
                xerr=1 / 2,
                color="black",
                marker=".",
                linestyle="",
                elinewidth=0.8,
                capsize=1,
                markersize=3,
            )
        x += 1
        firstrun = False
    dataerr = np.sqrt(np.array(datasum))

    # create data/mc subplot
    printdebug("Create Data/MC subplot...")
    bins = [xstart + i for i in range(nbins)]
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
    dataerr_overmc = np.array([dataerr[i] / mcsum[i] for i in divisionidx])
    barwidth_overmc = np.array([1 for i in divisionidx])
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

    # set x ticks (class names)
    ax[1].set_xticks([xstart + i - 0.5 for i in range(nbins + 1)], minor=False)
    ax[1].set_xticks([bins[i] for i in range(len(bins))], minor=True)
    ax[1].tick_params(
        axis="x",
        which="major",
        top=False,
        bottom=True,
        labelbottom=False,
        labeltop=False,
        direction="out",
    )
    ax[0].tick_params(
        axis="x",
        which="minor",
        top=False,
        bottom=False,
        labelbottom=False,
        labeltop=False,
        direction="in",
    )
    ax[1].tick_params(
        axis="x",
        which="minor",
        top=False,
        bottom=False,
        labelbottom=True,
        labeltop=False,
        direction="in",
    )
    ax[1].set_xticklabels(
        [name + " " for name in list(classes_data_toplot.keys())],
        fontsize="15",
        rotation="vertical",
        color="black",
        ha="center",
        va="top",
        minor=True,
    )

    # set y ticks
    ymax = 3 * np.amax(
        [
            np.amax(mcsum),
            np.amax(
                [
                    classes_data_toplot[classname]
                    for classname in classes_data_toplot.keys()
                ]
            ),
        ]
    )
    nmax = 0
    while 10**nmax < ymax:
        nmax += 1
    majoryticks = [10**n for n in range(-1, nmax)]
    ax[0].set_yticks(majoryticks, minor=False)
    minoryticks = [i * n for n in range(0, 11) for i in majoryticks]
    ax[0].set_yticks(minoryticks, minor=True)

    # set limits
    ax[0].set_xlim(0, nbins)
    ax[0].set_ylim(0.5, ymax)

    # find y limits for data/mc plot
    whitespace2 = 0.1
    ylim2 = (
        np.amax(
            [
                np.amin(
                    [
                        np.amin(
                            [
                                data_overmc[i] - dataerr_overmc[i] - whitespace2
                                for i in divisionidx
                            ]
                        ),
                        np.amin(
                            [1 - mcerr_overmc[0, i] - whitespace2 for i in divisionidx]
                        ),
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
                        for i in divisionidx
                    ]
                ),
                np.amax([1 + mcerr_overmc[1, i] + whitespace2 for i in divisionidx]),
            ]
        ),
    )
    ax[1].set_ylim(ylim2)

    # plot cosmetics and legend
    printdebug("Exporting plot...")
    plots = barplot[::-1] + [mcerrorplot] + [dataplot]
    labels = barlabel[::-1] + ["MC uncertainty"] + ["Data"]
    ax[0].legend(
        plots,
        labels,
        loc="center",
        prop={"size": 12},
        bbox_to_anchor=(0.9, 0.815),
        frameon=True,
    )

    # add text in plot with class category/type
    if classsuffix == "+X":
        specifier = "all-inclusive"
    elif classsuffix == "+nJ":
        specifier = "jet-inclusive"
    elif classsuffix == "BJ":
        specifier = "exclusive"
    fig.suptitle(specifier + " classes", ha="left", size=20, x=0.09, y=0.98)

    # set plot title
    plottitle = "counts"
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
    ax[0].set_ylabel(ylabel, fontsize=20)
    ax[1].set_ylabel("Data/MC", fontsize=20)

    # export plot
    if classsuffix == "+X":
        specifier = "all_incl"
    elif classsuffix == "+0":
        specifier = "excl"
    figname = specifier + "_counts"
    if args.title:  # optional custom file title
        figname = args.title
    outputpath = validation_path + "/" + str(args.year) + "/" + figname + ".pdf"
    if savepath != "":
        outputpath = (
            validation_path
            + "/"
            + str(args.year)
            + "/"
            + savepath
            + "/plots/"
            + figname
            + ".pdf"
        )
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

    # import class type(s)
    classtypes = args.classtype.split(",")
    classsuffixes = set()
    if len(classtypes) == 1 and classtypes[0] == "ALL":
        classsuffixes.add("+XJ")
        classsuffixes.add("+0")
    for classtype in classtypes:
        if classtype == "+XJ" or classtype == "jet-/bjet-incl":
            classsuffixes.add("+XJ")
        elif classtype == "+0" or classtype == "excl":
            classsuffixes.add("+0")

    # import task config file that includes references to all files that should be validated
    print(f"Importing task config...")
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    # parse years
    year = args.year
    print(f"Plotting job extends over the years {year}.")

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
    histograms.pop("color_dict")
    histograms.pop(
        "aggregation_dict"
    )  # histograms is a dict {histname: {properties: values}}

    # run plotting task
    print(f"Start {len(classsuffixes)} class event count validation jobs.")
    for classsuffix in classsuffixes:
        countplotter(
            args,
            savepath,
            datasamples,
            mcsamples,
            mcsorted,
            color_dict,
            aggregation_dict,
            year,
            histograms["COUNTS"],
            classsuffix,
        )

    print("Finished plot validation job.\n")
    exit(0)


if __name__ == "__main__":
    main()
