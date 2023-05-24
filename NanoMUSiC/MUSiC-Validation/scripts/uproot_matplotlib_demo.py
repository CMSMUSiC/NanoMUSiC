#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import uproot

validation_path = "./validation_outputs"
year = 2018
# file_name = "jet_val_DYJetsToLL_M-50_13TeV_AM_2018"
# file_path = validation_path + "/" + str(year) + "/" + file_name + ".root"
# hist_name = "h_2jet_invariant_mass"
figname = "test"
# 
# print(file_path)
# f = uproot.open(file_path)
# h = f[hist_name]
# 
# counts = h.values()
# edges = h.axis().edges()

fig, ax = plt.subplots(1, 1)
# ax.set_xlim(np.amin(edges), np.amax(edges) / 4)
# ax.set_ylim(1e-2, np.amax(counts) * 1.05)
# ax.set_yscale("log")

edges = [0, 1, 3, 4]
counts2 = [1, 1, 1]
counts = [1, 2, 3]
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
bins = np.array(bins)
barwidth = np.array(barwidth)

ax.bar(bins, counts, width=barwidth, color="tab:blue")
ax.bar(bins, counts2, width=barwidth, bottom=counts, color="tab:orange")

fig.savefig(validation_path + "/" + str(year) + "/" + figname + ".png", dpi=500)
fig.tight_layout()
