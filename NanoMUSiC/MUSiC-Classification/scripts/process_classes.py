import ROOT
import glob
import tqdm
import matplotlib.pyplot as plt 
import numpy as np
import random
import mplhep as hep
hep.style.use("CMS")

from colors import PROCESS_GROUP_STYLES


print("\n\n📶 [ MUSiC classification - Process Classes ] 📶\n")


# sorting the classes
#files = glob.glob("~/music/NanoMUSiC/build/classification_outputs_newest_one/2018/*/*.root")
#files = glob.glob("classification_outputs_signal_08_30/2018/*/*.root")
#and check the output paths!!

event_classes_incl = {}
event_classes_jincl = {}
event_classes_excl = {}

print("Sorting classes...")
for f in tqdm.tqdm(files):
    root_file =  ROOT.TFile.Open(f)

    for k in root_file.GetListOfKeys():

        if "h_counts" in k.GetName():

            event_class = k.GetName().split("]_[")[0][1:]
            process_group = k.GetName().split("]_[")[1]
                    
            if "+X" in event_class:

                if event_class in event_classes_incl.keys():
                    if process_group in event_classes_incl[event_class].keys():
                        event_classes_incl[event_class][process_group] += root_file.Get(k.GetName()).Integral()
                    else:
                        event_classes_incl[event_class][process_group] = root_file.Get(k.GetName()).Integral()
                else:
                    event_classes_incl[event_class] = {}
                    event_classes_incl[event_class][process_group] = root_file.Get(k.GetName()).Integral()

            elif "+NJet" in event_class:

                if event_class in event_classes_jincl.keys():
                        if process_group in event_classes_jincl[event_class].keys():
                            event_classes_jincl[event_class][process_group] += root_file.Get(k.GetName()).Integral()
                        else:
                            event_classes_jincl[event_class][process_group] = root_file.Get(k.GetName()).Integral()
                else:
                    event_classes_jincl[event_class] = {}
                    event_classes_jincl[event_class][process_group] = root_file.Get(k.GetName()).Integral()
                
            else:

                if event_class in event_classes_excl.keys():
                        if process_group in event_classes_excl[event_class].keys():
                            event_classes_excl[event_class][process_group] += root_file.Get(k.GetName()).Integral()
                        else:
                            event_classes_excl[event_class][process_group] = root_file.Get(k.GetName()).Integral()
                else:
                    event_classes_excl[event_class] = {}
                    event_classes_excl[event_class][process_group] = root_file.Get(k.GetName()).Integral()


# ------------------------------- INCLUSIVE CLASSES -------------------------------------------

#look for the most occupied classes 
def adding_up(event_class):
    count = 0
    for pg in event_classes_incl[event_class]:
        if "Tau" in event_class:
            count += event_classes_incl[event_class][pg]
    return count

sorted_classes =sorted(event_classes_incl.keys(), key=adding_up) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes


#create a dictionary in a dictionary with EC -> ProcessGroup -> counts
dict_process_group_and_counts = {}
data_counts = []

for ec in event_classes_to_plot:
    for pg in event_classes_incl[ec]:
        if pg != "Data":
            dict_process_group_and_counts[pg] = []


print("Sorting the event classes for plotting (inclusive classes)...")
for ec in tqdm.tqdm(event_classes_to_plot):
    for pg in dict_process_group_and_counts:
        if pg != "Data":
            if (pg in event_classes_incl[ec].keys() and event_classes_incl[ec][pg] >= 0.1):
                dict_process_group_and_counts[pg].append(event_classes_incl[ec][pg])
            else:
                dict_process_group_and_counts[pg].append(0)

    if ("Data" in event_classes_incl[ec].keys() and event_classes_incl[ec]["Data"] >= 1):
        data_counts.append(event_classes_incl[ec]["Data"])
    else:
        data_counts.append(0)


#### Plotting
width = 0.5
fig, ax = plt.subplots(figsize=(16 * 1.8, 9 * 1.8 + 2))
bottom = np.zeros(len(event_classes_to_plot))

for process_group, counts in reversed(dict_process_group_and_counts.items()):
    #print(process_group, counts )
    p = ax.bar(event_classes_to_plot, counts, label=process_group, bottom=bottom, color=PROCESS_GROUP_STYLES[process_group].colorhex)
    bottom += counts
#print("Data", data_counts)
#for data_counts look above
data_counts_err = [0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
data_x = []
#take coordinates for data points being in the middle of the bar
for i in range(len(event_classes_to_plot)):
  data_x.append(p[i].xy[0]+p[i].get_width()/2)

ax.errorbar(data_x,data_counts , xerr=0., yerr=data_counts_err, fmt='o', color='k', label="Data")

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
hep.cms.label("Work in progress", data=True, lumi=60, year="2018")
ax.set_ylabel("Events per class")
plt.tight_layout()
plt.savefig("processed_classes_plots/signal/plot_incl.png")

# ------------------------------------- END -------------------------------------------


# ------------------------------- JET-INCLUSIVE CLASSES -------------------------------------------

#look for the most occupied classes 
def adding_up(event_class):
    count = 0
    for pg in event_classes_jincl[event_class]:
        if "Tau" in event_class:
            count += event_classes_jincl[event_class][pg]
    return count

sorted_classes =sorted(event_classes_jincl.keys(), key=adding_up) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes


#create a dictionary in a dictionary with EC -> ProcessGroup -> counts
dict_process_group_and_counts = {}
data_counts = []

for ec in event_classes_to_plot:
    for pg in event_classes_jincl[ec]:
        if pg != "Data":
            dict_process_group_and_counts[pg] = []

print("Sorting the event classes for plotting (jet-inclusive classes)...")
for ec in tqdm.tqdm(event_classes_to_plot):
    for pg in dict_process_group_and_counts:
        if pg != "Data":
            if (pg in event_classes_jincl[ec].keys() and event_classes_jincl[ec][pg] >= 0.1):
                dict_process_group_and_counts[pg].append(event_classes_jincl[ec][pg])
            else:
                dict_process_group_and_counts[pg].append(0)

    if ("Data" in event_classes_jincl[ec].keys() and event_classes_jincl[ec]["Data"] >= 1):
        data_counts.append(event_classes_jincl[ec]["Data"])
    else:
        data_counts.append(0)


#### Plotting
width = 0.5
fig, ax = plt.subplots(figsize=(16 * 1.8, 9 * 1.8 + 2))
bottom = np.zeros(len(event_classes_to_plot))

for process_group, counts in reversed(dict_process_group_and_counts.items()):
    #print(process_group, counts )
    p = ax.bar(event_classes_to_plot, counts, label=process_group, bottom=bottom, color=PROCESS_GROUP_STYLES[process_group].colorhex)
    bottom += counts
#print("Data", data_counts)
#for data_counts look above
data_counts_err = [0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
data_x = []
#take coordinates for data points being in the middle of the bar
for i in range(len(event_classes_to_plot)):
  data_x.append(p[i].xy[0]+p[i].get_width()/2)

ax.errorbar(data_x,data_counts , xerr=0., yerr=data_counts_err, fmt='o', color='k', label="Data")

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
hep.cms.label("Work in progress", data=True, lumi=60, year="2018")
ax.set_ylabel("Events per class")
plt.tight_layout()
plt.savefig("processed_classes_plots/signal/plot_jincl.png")

# ------------------------------------- END -------------------------------------------


# ------------------------------- EXCLUSIVE CLASSES -------------------------------------------

#look for the most occupied classes 
def adding_up(event_class):
    count = 0
    for pg in event_classes_excl[event_class]:
        if "Tau" in event_class:
            count += event_classes_excl[event_class][pg]
    return count

sorted_classes =sorted(event_classes_excl.keys(), key=adding_up) #sorting the classes after counts
event_classes_to_plot = list(reversed(sorted_classes))[0:30] #take the 30 most occupied classes


#create a dictionary in a dictionary with EC -> ProcessGroup -> counts
dict_process_group_and_counts = {}
data_counts = []

for ec in event_classes_to_plot:
    for pg in event_classes_excl[ec]:
        if pg != "Data":
            dict_process_group_and_counts[pg] = []

print("Sorting the event classes for plotting (exclusive classes)...")
for ec in tqdm.tqdm(event_classes_to_plot):
    for pg in dict_process_group_and_counts:
        if pg != "Data":
            if (pg in event_classes_excl[ec].keys() and event_classes_excl[ec][pg] >= 0.1):
                dict_process_group_and_counts[pg].append(event_classes_excl[ec][pg])
            else:
                dict_process_group_and_counts[pg].append(0)

    if ("Data" in event_classes_excl[ec].keys() and event_classes_excl[ec]["Data"] >= 1):
        data_counts.append(event_classes_excl[ec]["Data"])
    else:
        data_counts.append(0)


#### Plotting
width = 0.5
fig, ax = plt.subplots(figsize=(16 * 1.8, 9 * 1.8 + 2))
bottom = np.zeros(len(event_classes_to_plot))

for process_group, counts in reversed(dict_process_group_and_counts.items()):
    #print(process_group, counts )
    p = ax.bar(event_classes_to_plot, counts, label=process_group, bottom=bottom, color=PROCESS_GROUP_STYLES[process_group].colorhex)
    bottom += counts
#print("Data", data_counts)
#for data_counts look above
data_counts_err = [0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
data_x = []
#take coordinates for data points being in the middle of the bar
for i in range(len(event_classes_to_plot)):
  data_x.append(p[i].xy[0]+p[i].get_width()/2)

ax.errorbar(data_x,data_counts , xerr=0., yerr=data_counts_err, fmt='o', color='k', label="Data")

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
ax.legend(handles=reordered_legend_elements, labels=reordered_labels, loc="upper right", ncol=3)
plt.xticks(rotation=90, va='top')
plt.yscale("log")
hep.cms.label("Work in progress", data=True, lumi=60, year="2018")
ax.set_ylabel("Events per class")
plt.tight_layout()
plt.savefig("processed_classes_plots/signal/plot_excl.png")

# ------------------------------------- END -------------------------------------------