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
    parser.add_argument(
        "-s",
        "--savepath",
        help="Directory of the files inside of /validation_outputs/[year]/ (if they are in a sub-directory). Search for files in the [year] directory if left blank.",
    )
    parser.add_argument(
        "-t",
        "--title",
        help="Title of the exported plot, default is name of the histogram.",
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
def import_hist(year, sample, file_prefix, hist_name, savepath):
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
    bins = []
    barwidth = []
    currentx = edges[0]
    for i in range(len(edges) - 1):
        currentwidth = (edges[i + 1] - edges[i])
        bins += [currentx + currentwidth/2]
        barwidth += [currentwidth]
        currentx += currentwidth
    # bins = np.array([(edges[i + 1] + edges[i]) / 2 for i in range(len(edges) - 1)])
    # barwidth = (edges[-1] - edges[0]) / len(bins)
    return np.array(bins), np.array(barwidth)


# aggregation dictionary for the plot
aggregation_dict =  {"TTW": "TTbar",
                    "TTZ": "TTbar",
                    "TTbar_13TeV_P8": "TTbar",
                    "WW": "Multi-Boson",
                    "WW_13TeV_P8": "Multi-Boson",
                    "ZZ": "Multi-Boson",
                    "ZZ_13TeV_P8": "Multi-Boson",
                    "WZ": "Multi-Boson",
                    "WZ_13TeV_P8": "Multi-Boson",
                    "WG": "Multi-Boson",
                    "DiPhoton": "Multi-Boson",
                    "GG": "Multi-Boson",
                    "WWW": "Multi-Boson",
                    "WWZ": "Multi-Boson",
                    "WZZ": "Multi-Boson",
                    "ZZZ": "Multi-Boson",
                    "WWG": "Multi-Boson",
                    "WGG": "Multi-Boson",
                    "WZG": "Multi-Boson",
                    "ZG": "Multi-Boson",
                    "ttHJetToNonbb_M125_13TeV_AM": "HIG",
                    "ttHJetTobb_M125_13TeV_AM": "HIG",
                    "BlackHole*": "Signal",
                    "SeesawTypeIII*": "Signal",
                    "QBHToEMu*": "Signal",
                    "RPVresonantToEMu*": "Signal",
                    "WprimeToTB*": "Signal",
                    "Di-Boson": "Multi-Boson",
                    "ZToInvisible": "DrellYan",
                    "ZToInvis": "DrellYan",
                    "ZToQQ": "DrellYan",
                    "TTbarTTbar": "TTbar",
                    "TTG": "TTbar",
                    "TTGG": "TTbar",
                    "TTWW": "TTbar",
                    "TG": "Top",
                    "tG": "Top",
                    "tG": "Top",
                    "TZQ": "Top",
                    "tZQ": "Top",
                    "TTZZ": "TTbar",
                    "W": "W",
                    "TTbar": "TTbar",
                    "DrellYan": "DrellYan",
                    "Gamma": "Gamma",
                    "Top": "Top",
                    "QCD": "QCD",
                    "HIG": "HIG"
                    }

# color dictionary
color_dict = {
    "QCD": '#4daf4a',
    "Gamma": '#337a33',
    "DiPhoton": '#b683eb',
    "GG": '#b683eb',
    "TTbar": '#ffdb1a',
    "TTbarV": '#ccaf14',
    "Top": '#e41a1c',
    "TTTT": '#99830f',
    "TTG": '#ccaf14',
    "TTGG": '#ccaf14',
    "W": '#514608',
    "WG": '#0c3468',
    "WGStar": '#0c3468',
    "WGG": '#244877',
    "WWG": '#793e82',
    "DrellYan": '#ee7600',
    "tZQ": '#e63032',
    "TZQ": '#e63032',
    "ZG":  '#c194ee',
    "DiBoson": '#984ee3',
    "TriBoson": '#793E82',
    "Multi-Boson": '#984ee3',
    "WW": '#984ee3',
    "WZ": '#793eb5',
    "ZZ": '#5b2e88',
    "ZToInvisible": '#f6ba7f',
    "TTW": '#ee7600',
    "WZG": '#86518e',
    "ZToQQ": '#ee7600',
    "TTZ": '#ec5e60',
    "TG": '#9f1213',
    "tG": '#9f1213',
    "WWZ": '#6c3775',
    "WZZ": '#542b5b',
    "TTbarTTbar": '#99830f',
    "HIG": '#48d1cc',
    "ZZZ": '#3c1f41',
    "WWW": '#793E82',
    "mixed": 'gray',
    "Signal": 'red',
    "Wprime4000": '#cc0066',
    "proc1": '#0c3468',
    "proc2": '#e63032',
    "proc3": '#793e82',
    "proc4": '#0c3468',
    "proc5": '#e63032',
    "proc6": '#244877',
    "proc7": '#c194ee',
    "proc8": '#984ee3',
    "proc9": '#9f1213',
}

def getkeyfromvalue(dict,value):
    return [k for k, v in dict.items() if v == value][0]

# general information:
# the files are imported according to the config file and the user input
# first all mc samples are read in and sorted by their 'group' which is given in the config file
# then all data files are read in
# the mc groups are again grouped together to 'categories' defined by the aggregation dictionary
# colors for the categories are loaded from the color dictionary
# mc data is stacked in the given color
# then data is stacked and plotted
# plot limits and style is set
# the plot is exported


def main():
    print("\n\n📶 [ MUSiC Validation Plotter (Nils' version) ] 📶\n")

    # parse arguments
    args = parse_args()

    # check for sub-directory
    savepath = ""
    if args.savepath:
        savepath = args.savepath

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
        tempset = set()
        for sample in mcsamples: # check whether sample belongs to group
            if mcconfig[sample]["ProcessGroup"] == mcgroup:
                tempset.add(sample)
        mcsorted.update({mcgroup : tempset}) # dictionary: {group: {samples in this group}}
    print("Found", len(mcgroups), "mc groups in the selected task config.")

    # import mc histograms
    print("Importing", len(mcsamples), "mc histograms...")
    mccounts, mcedges, mcerrors = {}, {}, {}
    for sample in mcsamples:
        samplecounts, sampleedges, sampleerrors = import_hist(
            args.year, sample, args.fileprefix, args.histname, savepath
        )
        mccounts.update({sample: samplecounts})
        mcedges.update({sample: sampleedges})
        mcerrors.update({sample: sampleerrors})

    # import data histograms
    print("Importing", len(datasamples), "data histograms...")
    datacounts, dataedges, dataerrors = {}, {}, {}
    for sample in datasamples:
        samplecounts, sampleedges, sampleerrors = import_hist(
            args.year, sample, args.fileprefix, args.histname, savepath
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
        nbins = len(dataedges[sample]) - 1 # no. of bins is (no. of edges) - 1
    print("All histogram edges are matching, therefore the validation can continue.")

    # calculate bins and bin width (only once since they are the same for every sample)
    edges = validation_edges
    bins, barwidth = calculatebins(edges)
    print("Calculated bins from histogram edges.")

    # re-sort mc groups into mc categories with aggregation dictionary
    categories_samples = {}
    notfoundflag = False
    notfound = set()
    for category in set(aggregation_dict.values()):
        categories_samples.update({category: set()}) # dictionary: {category: {samples in category}}
    for group in mcgroups:
        for sample in mcsorted[group]:
            if(group in aggregation_dict.keys()):
                categories_samples[aggregation_dict[group]].add(sample)
            else:
                notfoundflag = True
                notfound.add(group)
    if notfoundflag:
        raise RuntimeError(f"The mc groups {notfound} are not listed in the aggregation dictionary.")
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
        if(category in color_dict.keys()):
            categories_colors.update({category: color_dict[category]}) # dictionary: {category: color}
        else:
            notfoundflag = True
            notfound.add(group)
    if notfoundflag:
        raise RuntimeError(f"The mc categories {notfound} are not listed in the dictionary.")
    print("Sorted", len(mcgroups), "mc groups into", len(categories_samples.keys()), "mc categories given by the aggregation dictionary and matched category colors.")
    print("   The mc categories", nomembers, "have no member mc groups for the given task config.")

    # stack all samples for each category and calculate errors for each category
    categories_counts, categories_errors = {}, {}
    for category in categories_samples.keys():
        categorysum = np.zeros(nbins)
        sqerr_categorysum  = np.zeros(nbins)
        for sample in categories_samples[category]:
            categorysum += mccounts[sample]
            sqerr_categorysum += (np.array(mcerrors[sample]))**2
        categories_counts.update({category: categorysum}) # dictionary: {category: counts for stacked bins for all samples of this category}
        categories_errors.update({category: np.sqrt(sqerr_categorysum)}) # dictionary: {category: combined error after stacking for all samples of this category}
    print("Stacked the samples in each mc category.")

    # sort categories after their contribution, smallest contribution first
    #categories_samples = {category: samples for category, samples in sorted(categories_samples.items())} # to sort categories alphabetically
    categories_max = {}
    for category in categories_samples.keys():
        categories_max.update({category: np.amax(categories_counts[category])}) # create dictionary with maximum contribution for each category at any bin
    categories_max = {k: v for k, v in sorted(categories_max.items(), key=lambda item: item[1])} # reorder after minimum value
    sorted_categories_samples = {}
    for max in categories_max.values():
        category = getkeyfromvalue(categories_max, max)
        sorted_categories_samples.update({category: categories_samples[category]}) # dictionary: {category: {samples in category}}
            # with categories ordered after the contribution, smallest contribution first
    print("Sorted mc categories by their maximum contribution.")
    
    # prepare plot
    fig, ax = plt.subplots(1, 1)

    # stack mc categories, calculate errors and plot mc
    print("Start mc plotting.")
    mcsum = np.zeros(nbins)
    sqmcerrsum = np.zeros(nbins)
    for category in sorted_categories_samples.keys():
        print("   Plotting mc category {}...".format(category))
        ax.bar(bins, categories_counts[category], width=barwidth, bottom=mcsum, label=category, color=categories_colors[category])
        mcsum += categories_counts[category]
        sqmcerrsum += (np.array(categories_errors[category]))**2 # add errors together
    mcerr = np.sqrt(sqmcerrsum) # plot combined mc error
    ax.bar(bins, 2*mcerr, width=barwidth, bottom=(mcsum-mcerr), label="MC uncertainty", fill=False, hatch="xxxxxxxx", linewidth=0, edgecolor="tab:gray")

    # stack data, calculate errors and plot data
    print("Start data stacking and plotting.")
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
    
    # plot cosmetics
    print("Exporting plot...")
    ax.legend(loc="upper right", prop={'size': 8}, frameon=False)
    ax.set_title(args.histname)
    fig.tight_layout()

    # export plot
    figname = args.histname
    if args.title: # optional custom title
        figname = args.title
    outputpath = validation_path + "/" + str(args.year) + "/" + figname + ".png"
    if savepath != "":
        outputpath = validation_path + "/" + str(args.year) + "/" + savepath + "/" + figname + ".png"
    fig.savefig(outputpath, dpi=500)

    print("Finished plot validation job.\n")
    exit(0)


if __name__ == "__main__":
    main()
