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
    parser.add_argument(
        "-ns",
        "--nosyst",
        help="Optional: Don't use systematics for plotting.",
        action="store_true",
    )
    parser.add_argument(
        "-ci",
        "--cincludes",
        help="Optional: Plot only classes containing this string in their name.",
    )
    parser.add_argument(
        "-ce",
        "--cexcludes",
        help="Optional: Plot only classes not containing this string in their name.",
    )
    parser.add_argument(
        "-mm",
        "--mcthreshold",
        help="Optional: Plot only classes with MC > threshold.",
    )
    parser.add_argument(
        "-tt",
        "--titletext",
        help="Optional: Add this string to title text. Put spaces as underscores.",
    )
    parser.add_argument(
        "-nt",
        "--normalizethis",
        help="Optional: Normalize the QCD with the information of the file [normalizethis]/plots/QCD_normalization.toml, the subfolder is specified by this argument.",
    )
    parser.add_argument(
        "-qcdu",
        "--addqcduncertainty",
        help="Optional: Include the 50% QCD uncertainty.",
        action="store_true",
    )
    parser.add_argument(
        "-perr",
        "--printerrors",
        help="Optional: Print relative errors.",
        action="store_true",
    )
    parser.add_argument(
        "--labelsize",
        help="Optional: Customize xlabel size.",
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
def import_counts(year, sample, savepath, systematics):
    savepath += "/files"
    file_prefix = "classes_"
    returndict = {}
    for syst in systematics:
        file_path = (
            validation_path
            + "/"
            + str(year)
            + "/"
            + file_prefix
            + syst
            + "_"
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
                + syst
                + "_"
                + sample
                + "_"
                + str(year)
                + ".toml"
            )
        file_contents = toml.load(file_path)
        returndict.update({syst: file_contents})
    return returndict  # returns dictionary: {systname: {'counts'/'stat': {classname: counts}}} for the sample


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

    mcsamples_classes = {}
    datasamples_classes = {}
    # import mc event counts
    printdebug(f"Importing {len(mcsamples)} mc event counts for year {year}...")
    for sample in mcsamples:
        samplecountdict = import_counts(
            year, sample, savepath, systematics
        )  # returns {systname: {'counts'/'stat': {classname: counts}}}
        mcsamples_classes.update(
            {sample: samplecountdict}
        )  # dictionary: {sample: {systname: {'counts'/'stat': {classname: counts}}}} for the sample

    # import data event counts
    printdebug(f"Importing {len(datasamples)} data event counts for year {year}...")
    for sample in datasamples:
        samplecountdict = import_counts(
            year, sample, savepath, ["Nominal"]
        )  # returns {systname: {'counts'/'stat': {classname: counts}}}
        datasamples_classes.update(
            {sample: samplecountdict}
        )  # dictionary: {sample: {systname: {'counts'/'stat': {classname: counts}}}} for the sample

    # add statistical error
    systematics.add("stat")

    # sort samples into class dictionary
    # for mc
    mcclassdict = {}  # dictionary: {classname: {sample: {systname: counts}}}
    mcclassnames = set()  # set of all classnames with mc events
    # fill classname set
    for sample in mcsamples:
        for classname in mcsamples_classes[sample]["Nominal"]["counts"].keys():
            mcclassnames.add(classname)
    # fill mcclassdict
    for classname in mcclassnames:
        for sample in mcsamples:
            templist = {}
            for syst in systematics:
                if syst != "stat":
                    if classname in mcsamples_classes[sample][syst]["counts"].keys():
                        templist.update(
                            {syst: mcsamples_classes[sample][syst]["counts"][classname]}
                        )
                    else:
                        templist.update({syst: 0})
                else:  # stat error
                    if classname in mcsamples_classes[sample]["Nominal"]["stat"].keys():
                        templist.update(
                            {
                                syst: mcsamples_classes[sample]["Nominal"]["stat"][
                                    classname
                                ]
                            }
                        )
                    else:
                        templist.update({syst: 0})
            if classname in mcclassdict.keys():
                mcclassdict[classname].update({sample: templist.copy()})
            else:
                mcclassdict.update({classname: {sample: templist.copy()}})

    # for data
    dataclassdict = {}  # dictionary: {classname: {sample: {systname: counts}}}
    dataclassnames = set()  # set of all classnames with data events
    # fill classname set
    for sample in datasamples:
        for classname in datasamples_classes[sample]["Nominal"]["counts"].keys():
            dataclassnames.add(classname)
    # fill dataclassdict
    for classname in dataclassnames:
        for sample in datasamples:
            templist = {}
            for syst in ["Nominal", "stat"]:
                if syst != "stat":
                    if classname in datasamples_classes[sample][syst]["counts"].keys():
                        templist.update(
                            {
                                syst: datasamples_classes[sample][syst]["counts"][
                                    classname
                                ]
                            }
                        )
                    else:
                        templist.update({syst: 0})
                else:  # stat error
                    if (
                        classname
                        in datasamples_classes[sample]["Nominal"]["stat"].keys()
                    ):
                        templist.update(
                            {
                                syst: datasamples_classes[sample]["Nominal"]["stat"][
                                    classname
                                ]
                            }
                        )
                    else:
                        templist.update({syst: 0})
            if classname in dataclassdict.keys():
                dataclassdict[classname].update({sample: templist.copy()})
            else:
                dataclassdict.update({classname: {sample: templist.copy()}})
    # print class counts
    print(
        f"There are {len(dataclassdict)} event classes for data and {len(mcclassdict)} event classes for mc in total (no differentiation between inclusive/exclusive)."
    )

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
        specifier = "jet-inclusive"
    elif classsuffix == "+0":
        specifier = "exclusive"
    else:
        raise RuntimeError(f"No valid class type specified.")
    print(f"-- ANAYLZE {specifier} CLASSES: --")
    print(
        f"There are {nclass['data'][classsuffix]} {specifier} classes in data and {nclass['MC'][classsuffix]} {specifier} classes in mc."
    )
    print("Only the classes with data are now analyzed.")

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

    # -------------- normalize if needed -----------------
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
        for classname in mcclasstypedict[classsuffix].keys():
            for sample in mcsamples:
                mcclasstypedict[classsuffix][classname][sample].update({"norm": 0.0})
                if (sample in categories_samples["QCD"]) and ("0MET" in classname):
                    mcclasstypedict[classsuffix][classname][sample]["norm"] = (
                        mcclasstypedict[classsuffix][classname][sample]["Nominal"]
                        * err_norm_fac
                    )
                    # error is *OLD* count times the normalization factor uncertainty, thats why it is calculated before rescaling the nominal counts
        # rescale all QCD samples for all syst with the normalization factor
        for classname in mcclasstypedict[classsuffix].keys():
            if "0MET" in classname:
                for sample in categories_samples["QCD"]:
                    for syst in systematics:
                        if syst != "norm":
                            mcclasstypedict[classsuffix][classname][sample][
                                syst
                            ] *= norm_fac  # rescale all syst shifts (including nominal)
        # ----------------------------------

    # set of all classnames to be analyzed
    classnames = set(dataclasstypedict[classsuffix].keys())

    # stack all nominal event counts for each category for mc
    classes_categories = {}  # dictionary: {classname: {category: nominal count}} for mc
    # fill count dict for every class
    for classname in classnames:
        for category in categories_samples.keys():
            # add counts for all samples in every category
            counts = 0
            for sample in categories_samples[category]:
                if sample in mcclasstypedict[classsuffix][classname].keys():
                    counts += mcclasstypedict[classsuffix][classname][sample]["Nominal"]
            # save count sum per category in dict
            if classname in classes_categories.keys():
                classes_categories[classname].update({category: counts})
            else:
                classes_categories.update({classname: {category: counts}})
    printdebug("Stacked the event counts for each mc category.")

    # sort mc categories after their contribution for each class
    for classname in classnames:
        classes_categories[classname] = {
            k: v
            for k, v in sorted(
                classes_categories[classname].items(), key=lambda item: item[1]
            )
        }  # reorder after minimum value
        # category with lowest event count first
    printdebug("Sorted mc categories by their maximum contribution.")

    # calculate errors as difference from systematics to nominal
    mcclasstypedict_syst = (
        {}
    )  # {suffix: {classname: {sample: {systname: error (deviation from nominal value)}}}}
    for classname in classnames:
        for sample in mcsamples:
            for syst in systematics:
                temp = mcclasstypedict[classsuffix][classname][sample][
                    syst
                ]  # for 'stat' simply take error
                if syst not in [
                    "stat",
                    "norm",
                ]:  # calculate absolute deviation (for 'stat' this is already done)
                    temp = np.abs(
                        mcclasstypedict[classsuffix][classname][sample]["Nominal"]
                        - mcclasstypedict[classsuffix][classname][sample][syst]
                    )
                if classname in mcclasstypedict_syst.keys():
                    if sample in mcclasstypedict_syst[classname].keys():
                        if syst in mcclasstypedict_syst[classname][sample].keys():
                            mcclasstypedict_syst[classname][sample][syst] = temp
                        else:
                            mcclasstypedict_syst[classname][sample].update({syst: temp})
                    else:
                        mcclasstypedict_syst[classname].update({sample: {syst: temp}})
                else:
                    mcclasstypedict_syst.update({classname: {sample: {syst: temp}}})

    # stat errors are already read in as a systematic

    # symmetrize all mc up/down errors to a single error that is applied in both directions
    # the systematics set content is changed to the symmetrized errors as well as the mc counts dict content
    for syst in systematics.copy():
        if (
            "Up" in syst
        ):  # only once for each systematic (choose up_), exclude nominal and stat
            for classname in classnames:
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
                        np.abs(mcclasstypedict_syst[classname][sample][newname + "_Up"])
                        + np.abs(
                            mcclasstypedict_syst[classname][sample][newname + "_Down"]
                        )
                    ) / 2  # calculate mean error for each sample
                    mcclasstypedict_syst[classname][sample].pop(newname + "_Up")
                    mcclasstypedict_syst[classname][sample].pop(newname + "_Down")
                    mcclasstypedict_syst[classname][sample].update(
                        {newname: newsyst}
                    )  # update mccounts

    # calculate all mc errors and merge them
    mc_errors = {}  # {classname: {systname: merged error for all samples}}
    # calculate stacked error for all samples for each systematic separately
    # usually, for a given systematic the errors are treated fully correlated (linear addition) for all samples and process groups
    # exceptions exist e.g. for the xsection errors where the errors of different groups are assumed to be uncorrelated
    for classname in classnames:
        for syst in systematics:
            s_error = 0
            # TREAT FULLY CORRELATED
            if syst in [
                "Luminosity",
                "PU",
                "PDF_As",
                "PreFiring",
                "JetResolution",
                "JetScale",
                "norm",
            ]:
                for sample in mcsamples:
                    s_error += mcclasstypedict_syst[classname][sample][
                        syst
                    ]  # assumed correlated for every sample
            # TREAT ALL SAMPLES UNCORRELATED
            elif syst in [
                "stat",
            ]:
                temp = 0
                for sample in mcsamples:
                    temp += mcclasstypedict_syst[classname][sample][
                        syst
                    ]  # assumed uncorrelated for every sample
                    # sum the squared stat errors together and take sqrt in the end
                    # since the squared error is stored in the classes_ [stat] file do not square again
                s_error = np.sqrt(temp)
            # ONLY TREAT ONE GROUP CORRELATED
            elif syst in ["xSecOrder"]:
                # error only for LO order, others have error 0 currently, therefore this code does not decide between different orders
                for category in categories_samples.keys():
                    if (addqcduncertainty == True) or (
                        addqcduncertainty == False and category != "QCD"
                    ):
                        # only apply XSec error to non-QCD since QCD is normalized and the large XSec error is not used anymore
                        # QCD does not get an xSec uncertainty anymore
                        temp = 0
                        for sample in categories_samples[category]:
                            temp += mcclasstypedict_syst[classname][sample][
                                syst
                            ]  # assumed correlated for samples of the same category
                        s_error += (
                            temp**2
                        )  # assumed uncorrelated for different categories
                s_error = np.sqrt(s_error)
            # save error value for systematic source
            if classname in mc_errors.keys():
                mc_errors[classname].update({syst: s_error})
            else:
                mc_errors.update({classname: {syst: s_error}})

    # combine all different systematic errors together for plotting
    # assume the different systematic sources to be uncorrelated
    total_mc_errors = {}  # dict: {classname: total combined mc errors}
    temp = 0  # holds squared combined errors
    for classname in classnames:
        temp = 0
        for syst in systematics:
            if syst != "Nominal":
                temp += mc_errors[classname][syst] ** 2
        total_mc_errors.update({classname: np.sqrt(temp)})  # combined errors
    printdebug("Calculated the mc errors.")

    # calculate total mc counts for each class
    total_mc = {}  # dict: {classname: total mc count}
    for classname in classnames:
        temp = 0
        for category in categories_samples.keys():
            temp += classes_categories[classname][category]
        total_mc.update({classname: temp})

    # stack all nominal event counts for data
    classes_data = {}  # dictionary: {classname: nominal count} for data
    for classname in classnames:
        counts = 0
        for sample in datasamples:
            counts += dataclasstypedict[classsuffix][classname][sample]["Nominal"]
        classes_data.update({classname: counts})
    printdebug("Stacked the event counts for data.")

    # calculate data errors
    # assume uncorrelated stat errors
    total_data_errors = {}  # dict: {classname: total combined data errors}
    for classname in classnames:
        temp = 0
        for sample in datasamples:
            temp += dataclasstypedict[classsuffix][classname][sample]["stat"]
            # sum the squared stat errors together and take sqrt in the end
            # since the squared error is stored in the classes_ [stat] file do not square again
        total_data_errors.update({classname: np.sqrt(temp)})  # only statistical error

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

    classnames_selected = []  # this contains the classnames to plot
    n = 0
    # always plot 30 classes maximum
    for classname in classes_data.keys():
        temp_mc_sum = 0
        for category in allcategories:
            temp_mc_sum += classes_categories[classname][category]
        # selection by name
        if cincludes != "":
            if not (cincludes in classname):
                continue
        if cexcludes != "":
            if cexcludes in classname:
                continue
        # mc threshold
        if temp_mc_sum < mcthreshold_classes:
            break
        if n >= 30:
            break
        n += 1
        classnames_selected += [classname]

    # fill the _toplot variables from the selected classnames to plot in classnames_selected
    classes_data_toplot = {}
    for classname in classnames_selected:
        classes_data_toplot.update({classname: classes_data[classname]})
    print(f"The {n} classes for the current selection are:\n{classes_data_toplot}")
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

    # --- printerrors of classes to plot if needed ---
    if printerrors:
        for classname in classnames_selected:
            print(f"################## {classname} ##################")
            for syst in systematics:
                if syst != "Nominal":
                    print(f"{syst}: {mc_errors[classname][syst]/total_mc[classname]}")
            print(f"Combined: {total_mc_errors[classname]/total_mc[classname]}")
        print(f"###################################################")

    # ---------------------- start plotting the class counts ----------------------

    # prepare plot
    print("Start plotting.")
    hep.style.use(hep.style.ROOT)
    wspace = 0
    if histproperties["wspace"] != "":
        wspace = float(histproperties["wspace"])
    hspace = 0.05
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
    bottom = 0.16  # 0.13
    if histproperties["bottom"] != "":
        bottom = float(histproperties["bottom"])
    fig.subplots_adjust(left=left, right=right, bottom=bottom, top=top)

    # plot mc
    printdebug("Start mc plotting.")
    xstart = 0.5
    x = xstart
    firstrun = True
    mcsum = []
    mcerr = []
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
        x += 1
        firstrun = False
        mcsum += [countsum]
        mcerr += [total_mc_errors[classname]]
    mcerr = np.array(mcerr)
    mcsum = np.array(mcsum)
    mcerrorplot = ax[0].bar(
        [xstart + i for i in range(len(classes_data_toplot.keys()))],
        2 * mcerr,
        width=1,
        bottom=(mcsum - mcerr),
        fill=False,
        hatch="xxx",
        linewidth=0,
        edgecolor="tab:gray",
    )

    # plot data
    printdebug("Start data plotting.")
    x = xstart
    datasum = []
    dataerr = []
    firstrun = True
    dataplot = 0
    for classname in classes_data_toplot.keys():
        datasum += [classes_data_toplot[classname]]
        if firstrun:
            dataplot = ax[0].errorbar(
                x,
                classes_data_toplot[classname],
                yerr=total_data_errors[classname],
                xerr=1 / 2,
                color="black",
                marker=".",
                linestyle="",
                elinewidth=0.8,
                capsize=1,
                markersize=5,
            )
            dataerr += [total_data_errors[classname]]
        else:
            ax[0].errorbar(
                x,
                classes_data_toplot[classname],
                yerr=total_data_errors[classname],
                xerr=1 / 2,
                color="black",
                marker=".",
                linestyle="",
                elinewidth=0.8,
                capsize=1,
                markersize=5,
            )
            dataerr += [total_data_errors[classname]]
        x += 1
        firstrun = False

    # create data/mc subplot
    printdebug("Create Data/MC subplot...")
    bins = [xstart + i for i in range(nbins)]
    divisionidx = []
    for i in range(len(mcsum)):
        if mcsum[i] > 0 and datasum[i] > 0:
            divisionidx += [i]
    data_overmc = np.array([datasum[i] / mcsum[i] for i in divisionidx])
    bins_overmc = np.array([bins[i] for i in divisionidx])
    mcerr_overmc = np.array([mcerr[i] / mcsum[i] for i in divisionidx])
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
        markersize=5,
    )
    ax[1].bar(
        bins_overmc,
        mcerr_overmc * 2,
        width=barwidth_overmc,
        bottom=1 - mcerr_overmc,
        fill=False,
        hatch="xxx",
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
        [display_classname(name) + " " for name in list(classes_data_toplot.keys())],
        fontsize=labelsize,
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
                        #        np.amin(
                        #            [1 - mcerr_overmc[i] - whitespace2 for i in divisionidx]
                        #        ),
                    ]
                ),
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
                # np.amax([1 + mcerr_overmc[i] + whitespace2 for i in divisionidx]),
            ]
        ),
    )
    # check for override over the ylimit of the data/mc plot (plot config option "ratiolim")
    ylim2_string = ["", ""]
    try:
        if histproperties["ratiolim"] != "":
            ylim2_string = histproperties["ratiolim"].split(",")
        if ylim2_string[0] != "" and ylim2_string[1] != "":
            ylim2 = (float(ylim2_string[0]), float(ylim2_string[1]))
        elif ylim2_string[0] != "" and ylim2_string[1] == "":
            ylim2 = (float(ylim2_string[0]), ylim2[1])
        elif ylim2_string[0] == "" and ylim2_string[1] != "":
            ylim2 = (ylim2[0], float(ylim2_string[1]))
    except:
        raise RuntimeError("Invalid y limit for data/mc ratio plot given.")
        exit(0)
    # apply determined ylimit to data/mc plot
    ax[1].set_ylim(ylim2)

    # optional: add arrows in data/mc plot
    # watch out: if ylim2 has been set manually, it is possible that the data points of the data/mc plot lie out of the selected plotting range
    # arrows should be plotted in the cases where the data points lie out of range
    dataovermc_height = ylim2[1] - ylim2[0]
    maxarrowwidth = len(bins) / 110
    for i in range(len(data_overmc)):
        if (
            data_overmc[i] + 0.1 * dataovermc_height > ylim2[1]
        ):  # if overflow of data, add big up arrow
            if data_overmc[i] + 0.03 * dataovermc_height > ylim2[1]:
                ax[1].arrow(
                    bins_overmc[i],
                    ylim2[1] - 0.2 * dataovermc_height,
                    0,
                    0.2 * dataovermc_height,
                    color="black",
                    width=min([barwidth_overmc[i] * 1.5, maxarrowwidth]) * 0.3,
                    head_width=min([barwidth_overmc[i] * 1.5, maxarrowwidth]),
                    head_length=0.1 * dataovermc_height,
                    length_includes_head=True,
                )
            else:
                ax[1].arrow(
                    bins_overmc[i],
                    ylim2[1] - 0.1 * dataovermc_height,
                    0,
                    0.1 * dataovermc_height,
                    color="black",
                    width=0,
                    head_width=min([barwidth_overmc[i] * 1.5, maxarrowwidth]),
                    head_length=abs(ylim2[1] - data_overmc[i]),
                    length_includes_head=True,
                )
        elif (
            data_overmc[i] + dataerr_overmc[i] > ylim2[1]
        ):  # if overflow of data errorbar, add up arrow
            ax[1].arrow(
                bins_overmc[i],
                ylim2[1] - 0.1 * dataovermc_height,
                0,
                0.1 * dataovermc_height,
                color="black",
                width=0,
                head_width=min([barwidth_overmc[i] * 1.5, maxarrowwidth]),
                head_length=0.1 * dataovermc_height,
                length_includes_head=True,
            )
        if (
            data_overmc[i] - 0.1 * dataovermc_height < ylim2[0]
        ):  # if underflow of data, add big up arrow
            if data_overmc[i] < ylim2[0]:
                ax[1].arrow(
                    bins_overmc[i],
                    ylim2[0] + 0.2 * dataovermc_height,
                    0,
                    -0.2 * dataovermc_height,
                    color="black",
                    width=min([barwidth_overmc[i] * 1.5, maxarrowwidth]) * 0.3,
                    head_width=min([barwidth_overmc[i] * 1.5, maxarrowwidth]),
                    head_length=0.1 * dataovermc_height,
                    length_includes_head=True,
                )
            else:
                ax[1].arrow(
                    bins_overmc[i],
                    ylim2[0] + 0.1 * dataovermc_height,
                    0,
                    -0.1 * dataovermc_height,
                    color="black",
                    width=0,
                    head_width=min([barwidth_overmc[i] * 1.5, maxarrowwidth]),
                    head_length=abs(ylim2[0] - data_overmc[i]),
                    length_includes_head=True,
                )
        elif (
            data_overmc[i] - dataerr_overmc[i] < ylim2[0] and ylim2[0] > 0
        ):  # if underflow of data errorbar, add up arrow (if above 0 since below 0 does not make sense)
            ax[1].arrow(
                bins_overmc[i],
                ylim2[0] + 0.1 * dataovermc_height,
                0,
                -0.1 * dataovermc_height,
                color="black",
                width=0.8,
                head_width=min([barwidth_overmc[i] * 1.5, maxarrowwidth]),
                head_length=0.1 * dataovermc_height,
                length_includes_head=True,
            )

    # plot cosmetics and legend
    printdebug("Exporting plot...")
    plots = barplot[::-1] + [mcerrorplot] + [dataplot]
    labels = barlabel[::-1] + ["MC uncertainty"] + ["Data"]
    ax[0].legend(
        plots,
        labels,
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

    # add CMS text
    plt.figtext(0.11, 0.958, "CMS", fontsize=19, ha="left", fontweight="bold")
    plt.figtext(
        0.174, 0.958, "Private work", fontsize=13, ha="left", fontstyle="italic"
    )

    # add text with class type (excl, j-incl)
    plt.figtext(0.3, 0.958, specifier + " classes " + titletext, fontsize=19, ha="left")

    # add text with lumi info
    int_lumi = 59.8  # hardcoded for 2018 for now
    com_energy = 13
    plt.figtext(
        0.982,
        0.958,
        str(int_lumi) + " fb${}^{-1}$ (" + str(com_energy) + " TeV)",
        fontsize=19,
        ha="right",
    )

    """ # leave out title because there is no space
    # set plot title
    plottitle = "counts"
    if histproperties["title"] != "":
        plottitle = histproperties["title"]
    ax[0].set_title(plottitle, fontsize=19)
    """

    # add normalized text
    if normalizethis != "":
        plt.figtext(
            0.3,
            0.983,
            f"QCD normalized with $\\alpha_{{QCD}} = {np.array(float(norm_fac)).round(decimals=2)}\\pm{np.array(float(err_norm_fac)).round(decimals=2)}$",
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
    ax[0].set_ylabel(ylabel, fontsize=20)
    ax[1].set_ylabel("Data/MC", fontsize=20)

    # export plot
    if classsuffix == "+XJ":
        specifier = "j-incl"
    elif classsuffix == "+0":
        specifier = "excl"
    figname = specifier + "_counts"
    if args.title:  # optional custom file title
        figname = args.title
    if mcthreshold_classes != 0:
        figname += "_mcthr_" + str(mcthreshold_classes)
    if cincludes != "":
        figname += "_incl_" + cincludes
    if cexcludes != "":
        figname += "_excl_" + cexcludes
    if normalizethis != "":
        figname += "_normalized"
    outputpath = validation_path + "/" + str(args.year) + "/plots/" + figname + ".pdf"
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
    print("\n\n📶 [ MUSiC Validation Plotter 3 ] 📶\n")

    # parse arguments
    args = parse_args()

    # check for sub-directory
    savepath = ""
    if args.savepath:
        savepath = args.savepath

    # nosyst flag
    global nosyst
    nosyst = False
    if args.nosyst:
        nosyst = True

    # includes/excludes from classname to plot
    global cincludes
    cincludes = ""
    if args.cincludes:
        cincludes = args.cincludes
    global cexcludes
    cexcludes = ""
    if args.cexcludes:
        cexcludes = args.cexcludes

    # import mc threshold
    global mcthreshold_classes
    mcthreshold_classes = 0
    if args.mcthreshold:
        mcthreshold_classes = float(args.mcthreshold)

    # read in optional title text
    global titletext
    titletext = ""
    if args.titletext:
        for temp in args.titletext.split("_"):
            titletext += temp + " "

    # normalize this
    global normalizethis
    normalizethis = ""
    if args.normalizethis:
        normalizethis = args.normalizethis

    # addqcduncertainty
    global addqcduncertainty
    addqcduncertainty = False
    if args.addqcduncertainty:
        addqcduncertainty = True

    # printerrors
    global printerrors
    printerrors = False
    if args.printerrors:
        printerrors = True

    # labelsize
    global labelsize
    labelsize = "12.5"
    if args.labelsize:
        labelsize = args.labelsize

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
