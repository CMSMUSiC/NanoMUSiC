import ROOT
import glob
import tqdm
import matplotlib.pyplot as plt 
import numpy as np
import random
import mplhep as hep
hep.style.use("CMS")
import json
from colors import PROCESS_GROUP_STYLES

signals = ["ChargedHiggsToHW", "ChargedHiggsToTauNu", "Taustar", "ZprimeToTauTau"]
is_signal = False

#define aggregation groups
list_TTbar = ["TTW", "TTZ", "TTbar", "TTbarTTbar", "TTG", "TTGG", "TTWW", "TTZZ"]
list_Multi_Boson = ["WW", "ZZ", "WZ", "WG", "DiPhoton", "GG", "WWW", "WWZ", "WZZ", "ZZZ", "WWG", "WGG", "WZG", "ZG", "Di-Boson"]
list_DrellYan = ["DrellYan", "ZToInvisible", "ZToInvis", "ZTOQQ"]
list_Top = ["TG", "tG", "TZQ", "tZQ"]


def change_ec_name(event_class_name):
    parts = event_class_name.split("_")
    muons = 0
    electrons = 0
    taus = 0
    photons = 0
    jets = 0
    bjets = 0
    MET = 0
    exclusive = False
    inclusive = False
    jetinclusive = False
    for idx, p in enumerate(parts):
        if "EC" in p:
            continue
        if "Muon" in p:
            muons = int(p[0])
        if "Electron" in p:
            electrons = int(p[0])
        if "Tau" in p:
            taus = int(p[0])
        if "Photon" in p:
            photons = int(p[0])
        if (("1Jet" in p or "2Jet" in p or "3Jet" in p or "4Jet" in p or "5Jet" in p or "6Jet" in p) and "bJet" not in p):
            jets = int(p[0])
        if "bJet" in p:
            bjets = int(p[0])
        if "MET" in p:
            MET = int(p[0])
        if idx == len(parts)-1:
            if "+X" in p:
                inclusive = True
            elif "+NJet" in p:
                jetinclusive = True
            else:
                exclusive = True
    latex_name = ""
    if muons > 0:
        latex_name = latex_name + str(muons) + r"$\mu$"

    if electrons > 0 and muons > 0:
        latex_name = latex_name + "+" + str(electrons) + r"$e$"
    if electrons > 0 and muons == 0:
        latex_name = latex_name + str(electrons) + r"$e$"

    if taus > 0 and (muons > 0 or electrons > 0):
        latex_name = latex_name + "+" + str(taus) + r"$\tau$"
    if taus > 0 and muons == 0 and electrons == 0:
        latex_name = latex_name + str(taus) + r"$\tau$"


    if photons > 0:
        latex_name = latex_name + "+" + str(photons) + r"$\gamma$"
    if bjets > 0:
        latex_name = latex_name + "+" + str(bjets) + r"$bJet$"
    if jets > 0:
        latex_name = latex_name + "+" + str(jets) + r"$Jet$"
    if MET > 0:
        latex_name = latex_name + "+" + r"$\vec{p}_{T}^{miss}$"
    if inclusive:
        latex_name = latex_name + r" inc."
    elif jetinclusive:
        latex_name = latex_name + r" Jet inc."
    elif exclusive:
        latex_name = latex_name +r" exc."
    return latex_name


def sort_and_plot(event_classes_to_plot):
    #create a dictionary in a dictionary with EC -> ProcessGroup -> counts/errors
    dict_process_group_and_counts = {}

    for ec in event_classes_to_plot:
        for pg in ec_data_json[ec]["mc"]:
            dict_process_group_and_counts[pg] = []

    for ec in event_classes_to_plot:
        for pg in dict_process_group_and_counts:
            if (pg in ec_data_json[ec]["mc"].keys() and ec_data_json[ec]["mc"][pg] >= 0.1):
                dict_process_group_and_counts[pg].append(ec_data_json[ec]["mc"][pg])
            else:
                dict_process_group_and_counts[pg].append(0)

    ec_pg_counts = {}
    ec_data_counts = {}
    ec_data_errors = {}
    for idx, ec in enumerate(event_classes_to_plot):
        ec_pg_counts[ec] = {}
        for pg in dict_process_group_and_counts.keys():
            ec_pg_counts[ec][pg] = dict_process_group_and_counts[pg][idx]
        if ("data_count" in ec_data_json[ec].keys() and ec_data_json[ec]["data_count"] >= 1):
            ec_data_counts[ec] = ec_data_json[ec]["data_count"]
        else:
            ec_data_counts[ec] = 0

    #sorting for aggregation group
    ec_ag_counts = {}
    
    for ec in event_classes_to_plot:
        ec_ag_counts[ec] = {}
        counts_DrellYan = 0
        counts_Multi_Boson = 0
        counts_TTbar = 0
        counts_Top = 0
        for pg in ec_pg_counts[ec].keys():
            if pg in list_DrellYan:
                counts_DrellYan += ec_pg_counts[ec][pg]
            elif pg in list_Multi_Boson:
                counts_Multi_Boson += ec_pg_counts[ec][pg]
            elif pg in list_TTbar:
                counts_TTbar += ec_pg_counts[ec][pg]
            elif pg in list_Top:
                counts_Top += ec_pg_counts[ec][pg]

        ec_ag_counts[ec]["DrellYan"] = counts_DrellYan
        ec_ag_counts[ec]["Multi-Boson"] = counts_Multi_Boson
        ec_ag_counts[ec]["TTbar"] = counts_TTbar
        ec_ag_counts[ec]["Top"] = counts_Top

    for ec in event_classes_to_plot: 
        for pg in ec_pg_counts[ec].keys():
            if pg not in list_DrellYan and pg not in list_Multi_Boson and pg not in list_Top and pg not in list_TTbar:
                ec_ag_counts[ec][pg] = ec_pg_counts[ec][pg]



    #### Plotting
    set_of_labels = set()
    width = 0.5
    fig, ax = plt.subplots(figsize=(16 * 1.8, 9 * 1.8 + 2))

    for ec in tqdm.tqdm(ec_ag_counts.keys()):
        bottom = 0
        for pg in sorted(ec_ag_counts[ec].keys(), key=lambda x: ec_ag_counts[ec][x]):
            if pg not in signals:
                if pg in set_of_labels:
                    p = ax.bar(change_ec_name(ec), ec_ag_counts[ec][pg], bottom=bottom, color=PROCESS_GROUP_STYLES[pg].colorhex)
                    bottom += ec_ag_counts[ec][pg]
                else:
                    p = ax.bar(change_ec_name(ec), ec_ag_counts[ec][pg], label=pg, bottom=bottom, color=PROCESS_GROUP_STYLES[pg].colorhex)
                    bottom += ec_ag_counts[ec][pg]
                    set_of_labels.add(pg)

        for pg in sorted(ec_ag_counts[ec].keys(), key=lambda x: ec_ag_counts[ec][x]):
            if pg in signals:
                if pg in set_of_labels:
                    p = ax.bar(change_ec_name(ec), ec_ag_counts[ec][pg], bottom=bottom, color=PROCESS_GROUP_STYLES[pg].colorhex)
                    bottom += ec_ag_counts[ec][pg]
                else:
                    p = ax.bar(change_ec_name(ec), ec_ag_counts[ec][pg], label=pg, bottom=bottom, color=PROCESS_GROUP_STYLES[pg].colorhex)
                    bottom += ec_ag_counts[ec][pg]
                    set_of_labels.add(pg)

        mc_uncert = ec_data_json[ec]["mc_uncert"]
        p_value = ec_data_json[ec]["p_value"]
        p = ax.bar(change_ec_name(ec), 2*mc_uncert, bottom=(bottom-mc_uncert), color="grey", alpha=0.01, hatch="//")
        bottom = bottom + mc_uncert
        if p_value != None and p_value >= 0.1:
            plt.text(change_ec_name(ec), bottom*(1+0.05), f"p = {p_value:.2f}", ha="center", va="bottom", rotation=90)
        elif p_value != None and p_value < 0.1:
            plt.text(change_ec_name(ec), bottom*(1+0.05), f"p = {p_value:.1e}", ha="center", va="bottom", rotation=90)
        else:
            plt.text(change_ec_name(ec), bottom*(1+0.05), "p = " + str(p_value), ha="center", va="bottom", rotation=90)
        data_uncert = ec_data_json[ec]["data_uncert"]
        if not(is_signal):
            if "Data" in set_of_labels:
                ax.errorbar(p[0].xy[0]+p[0].get_width()/2, ec_data_counts[ec] , xerr=0., yerr=data_uncert, fmt='o', color='k')
            else:
                ax.errorbar(p[0].xy[0]+p[0].get_width()/2, ec_data_counts[ec] , xerr=0., yerr=data_uncert, fmt='o', color='k', label="Data")
                set_of_labels.add("Data")

    #sorting the legend so that Data is the first element of the legend
    legend_elements, labels = plt.gca().get_legend_handles_labels()
    reordered_legend_elements = [0]
    reordered_labels = [0]
    for i in range(0, (len(legend_elements)-1)):
        reordered_legend_elements.append(legend_elements[i])
        reordered_labels.append(labels[i])
    reordered_legend_elements[0] = legend_elements[-1]
    reordered_labels[0] = labels[-1]

    #final options

    ax.legend(handles=reordered_legend_elements, labels=reordered_labels, loc="upper right", ncol=5)
    plt.xticks(rotation=90, va='top')
    plt.yscale("log")
    ylim_bottom, ylim_top = ax.get_ylim()
    plt.ylim(ylim_bottom, 3*ylim_top)
    hep.cms.label("Work in progress", data=True, lumi=60, year="2018")
    ax.set_ylabel("Events per class")
    plt.tight_layout()


print("\n\n📶 [ MUSiC classification - Process Classes ] 📶\n")

input_file = open("/disk1/silva/plot_data_2017_2018.json")
ec_data_json = json.load(input_file)
input_file.close()

# ------------------------------- JET-INCLUSIVE CLASSES -------------------------------------------

#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "+NJet" in event_class:
                count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (jet-inclusive classes: all)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_jincl.png")

# -------------------------------------- 1muon+1tau -------------------------------------------

#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "Muon" in event_class and "+NJet" in event_class:
                count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (jet-inclusive classes: muon trigger)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_jincl_muon_trigger.png")

# ------------------------------------- END: 1muon+1tau -------------------------------------------

# ------------------------------------- 1ele+1tau -------------------------------------------
#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "Electron" in event_class and "+NJet" in event_class:
                if not("Muon" in event_class):
                    count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (jet-inclusive classes: electron trigger)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_jincl_electron_trigger.png")

# ------------------------------------- END: 1ele+1tau -------------------------------------------

# ------------------------------------- 1photon+1tau -------------------------------------------
#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "Photon" in event_class and "+NJet" in event_class:
                if not("Muon" in event_class) and not("Electron" in event_class):
                    count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes


print("Plotting the event classes (jet-inclusive classes: photon trigger)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_jincl_photon_trigger.png")

# ------------------------------------- END: 1photon+1tau -------------------------------------------
# ------------------------------------- END: jinclusive -------------------------------------------


# ------------------------------- INCLUSIVE CLASSES -------------------------------------------

#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "+X" in event_class:
                count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (inclusive classes: all)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_incl.png")

# ------------------------------------- 1muon+1tau -------------------------------------------
#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "Muon" in event_class and "+X" in event_class:
                count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (inclusive classes: muon trigger)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_incl_muon_trigger.png")

# ------------------------------------- END: 1muon+1tau -------------------------------------------

# ------------------------------------- 1ele+1tau -------------------------------------------
#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "Electron" in event_class and "+X" in event_class:
                if not("Muon" in event_class):
                    count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (inclusive classes: electron trigger)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_incl_electron_trigger.png")

# ------------------------------------- END: 1ele+1tau -------------------------------------------

# ------------------------------------- 1photon+1tau -------------------------------------------
#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "Photon" in event_class and "+X" in event_class:
                if not("Muon" in event_class) and not("Electron" in event_class):
                    count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (inclusive classes: photon trigger)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_incl_photon_trigger.png")

# ------------------------------------- END: 1photon+1tau -------------------------------------------
# ------------------------------------- END: inclusive -------------------------------------------


# ------------------------------- EXCLUSIVE CLASSES -------------------------------------------

#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "+X" not in event_class and "+NJet" not in event_class:
                count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (exclusive classes: all)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_excl.png")

# ------------------------------------- 1muon+1tau -------------------------------------------
#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "Muon" in event_class and "+X" not in event_class and "+NJet" not in event_class:
                count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes


print("Plotting the event classes (exclusive classes: muon trigger)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_excl_muon_trigger.png")

# ------------------------------------- END: 1muon+1tau -------------------------------------------

# ------------------------------------- 1ele+1tau -------------------------------------------
#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "Electron" in event_class and "+X" not in event_class and "+NJet" not in event_class:
                if not("Muon" in event_class):
                    count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (exclusive classes: electron trigger)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_excl_electron_trigger.png")

# ------------------------------------- END: 1ele+1tau -------------------------------------------

# ------------------------------------- 1photon+1tau -------------------------------------------
#look for the most occupied classes 
def sorting_class(event_class):
    count = 0
    for pg in ec_data_json[event_class]["mc"]:
        if pg not in signals: 
            if "Tau" in event_class and "Photon" in event_class and not("+X" in event_class) and not("+N" in event_class):
                if not("Muon" in event_class) and not("Electron" in event_class):
                    count += ec_data_json[event_class]["mc"][pg]
    return count

sorted_classes =sorted(ec_data_json.keys(), key=sorting_class) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes

print("Plotting the event classes (exclusive classes: photon trigger)...")
sort_and_plot(event_classes_to_plot)
plt.savefig("pval_plots/plot_excl_photon_trigger.png")

# ------------------------------------- END: 1photon+1tau -------------------------------------------
# ------------------------------------- END: exclusive -------------------------------------------
