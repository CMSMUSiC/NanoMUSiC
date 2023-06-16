#!/bin/env python

# STL imports
import sys
import os
import math
import array

# ROOT imports
import ROOT


def get_scale_hist():
    # create histogram
    eta_bins = array.array('d', [-2.4, -2.1, -1.2, 0.0, 1.2, 2.1, 2.4])
    phi_bins = array.array('d', [-math.pi, -1./3. * math.pi, 1./3. * math.pi, math.pi])
    muo_scale = ROOT.TH2D('h2_muo_scale', 'h2_muo_scale', 6, eta_bins, 3, phi_bins)

    # set bin content / uncertainties
    # CAREFUL: y-order is inverse compared to plot
    bin_content = [
        [-0.39, -0.039, -0.0041, -0.012,  0.0051, -0.24],
        [ 0.38,  0.041,  0.023,  -0.017,  0.036,  -0.12],
        [-0.15, -0.11,  -0.035,   0.0039, 0.07,    0.092]
    ]
    bin_uncertainties = [
        [0.046,  0.032,  0.025,  0.022,  0.033,  0.078],
        [0.09,   0.03,   0.023,  0.022,  0.039,  0.061],
        [0.063,  0.029,  0.023,  0.024,  0.035,  0.075]
    ]

    for j in range(muo_scale.GetNbinsY()):
        for i in range(muo_scale.GetNbinsX()):
            # bins start at 1, not 0
            muo_scale.SetBinContent(i + 1, j + 1, bin_content[j][i])
            muo_scale.SetBinError(i + 1, j + 1, bin_uncertainties[j][i])

    return muo_scale


def main():
    tfile = ROOT.TFile("../ConfigFiles/ConfigInputs/muon_systematics.root", "RECREATE");
    scale = get_scale_hist()
    scale.Write()
    tfile.Close()

    return 0


if __name__=='__main__':
    sys.exit(main())
