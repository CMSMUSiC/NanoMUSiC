#!/usr/bin/env python3
import warnings

warnings.simplefilter("ignore", UserWarning)

import argparse
import glob
from decimal import Decimal
import os
import sys
from tqdm import tqdm
from multiprocessing import Pool

import ROOT

# ROOT.gErrorIgnoreLevel = ROOT.kError
ROOT.gErrorIgnoreLevel = 6000

from event_class import EventClassCollection
from colors import PROCESS_GROUP_STYLES


import atlasplots as aplt

import tdrstyle


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-i",
        "--inputs",
        help="Path to the results of classification.",
        type=str,
        default="classification_outputs",
    )

    parser.add_argument(
        "PATTERNS",
        nargs="+",
        help="List of pattern to filter the Event Classes.",
    )

    args = parser.parse_args()

    return args


def get_source_files(path):
    return list(
        filter(
            lambda f: ("cutflow" not in f),
            glob.glob(f"{path}/*/*/*.root"),
        )
    )


def plot_event_class(ec):
    for histogram_name in ["invariant_mass", "sum_pt", "counts", "met"]:
        # skip MET histogram for non-MET classes
        if histogram_name == "met" and not ec.has_met:
            continue

        # Create a figure and axes
        fig, (ax1, ax2) = aplt.ratio_plot(
            name=f"ratio_{ec.name}_{histogram_name}",
            figsize=(int(1.5 * 800), int(1.5 * 600)),
            hspace=0.1,
        )

        # Set axis titles
        ax2.set_xlabel("XXXXX", loc="right", titlesize=25)
        ax1.set_ylabel("Events / 10 GeV", titlesize=25)
        if histogram_name == "counts":
            ax1.set_ylabel("Events", titlesize=25)

        ax2.set_ylabel("Obs./Pred.", loc="centre", titlesize=25)
        x_label_text = ""
        if histogram_name == "invariant_mass":
            if "MET" in ec.name:
                x_label_text = r"m_{T} (GeV)"
            else:
                x_label_text = r"m_{inv} (GeV)"
        elif histogram_name == "sum_pt":
            x_label_text = r"#Sigma p_{T} (GeV)"
        elif histogram_name == "met":
            x_label_text = r"MET (GeV)"
        elif histogram_name == "counts":
            x_label_text = r""
        else:
            print(
                f"ERROR: Could not set x axis label. Invalid option ({histogram_name})."
            )
            sys.exit(-1)

        ax2.text(
            0.85,
            0.15,
            x_label_text,
            size=25,
        )

        data_hist = ec.get_data_histogram(histogram_name)
        data_hist.scale_for_plot()
        limits = data_hist.get_limits()

        if limits == None:
            if histogram_name == "counts":
                limits = -0.1, 2.5
            else:
                continue
        x_min, x_max = limits
        x_min = x_min - (x_max - x_min) * 0.05

        # build the data as a graph
        data_graph = aplt.root_helpers.hist_to_graph(data_hist.histo)

        mc_hists = ec.get_mc_histograms_per_process_group(histogram_name)
        mc_hists_keys_sorted = sorted(mc_hists, key=lambda x: mc_hists[x].integral())

        # Add legend
        if histogram_name == "counts":
            legend = ax1.legend(
                loc=(
                    0.55,
                    0.15,
                    1 - ROOT.gPad.GetRightMargin(),
                    1 - ROOT.gPad.GetTopMargin() - 0.05,
                ),
                textsize=6,
            )
            legend.AddEntry(
                data_graph, f"Data ({Decimal(data_hist.integral()):.2E})", "EP"
            )

            for hist in reversed(mc_hists_keys_sorted):
                legend.AddEntry(
                    mc_hists[hist].histo,
                    f"{hist} ({Decimal(mc_hists[hist].integral()):.2E})",
                    "F",
                )

        total_mc_histo = None
        bkg_stack = ROOT.THStack("bkg", "")
        for hist in mc_hists_keys_sorted:
            mc_hists[hist].scale_for_plot()
            mc_hists[hist].histo.SetFillColor(
                PROCESS_GROUP_STYLES[mc_hists[hist].process_group].color
            )
            mc_hists[hist].histo.SetLineWidth(0)
            bkg_stack.Add(mc_hists[hist].histo)
            if total_mc_histo == None:
                total_mc_histo = mc_hists[hist].histo.Clone()
            else:
                total_mc_histo.Add(mc_hists[hist].histo.Clone())

        # Draw the stacked histogram on the axes
        ax1.plot(bkg_stack)

        # Plot the MC stat error as a hatched band
        err_band = aplt.root_helpers.hist_to_graph(
            # bkg_stack.GetStack().Last(), show_bin_width=True
            total_mc_histo,
            show_bin_width=True,
        )
        ax1.plot(err_band, "2", fillcolor=1, fillstyle=3254, linewidth=0)

        # ax1.set_ylim(1e-9)
        if histogram_name != "counts":
            ax1.set_ylim(ec.get_y_low(histogram_name) * 0.1)
        ax1.set_yscale("log")  # uncomment to use log scale for y axis

        ax1.plot(data_graph, "P")
        ax1.set_xlim(x_min, x_max)

        # Use same x-range in lower axes as upper axes
        ax2.set_xlim(ax1.get_xlim())
        ax2.set_xlim(x_min, x_max)

        # Draw line at y=1 in ratio panel
        line = ROOT.TLine(ax1.get_xlim()[0], 1, ax1.get_xlim()[1], 1)
        ax2.plot(line)

        # Plot the relative error on the ratio axes
        data_rebinned_for_ratio, mc_rebinned_for_ratio = data_hist.histo, total_mc_histo
        if histogram_name != "counts":
            data_rebinned_for_ratio, mc_rebinned_for_ratio = ec.get_ratio_histogram(
                histogram_name
            )

        err_band_ratio = aplt.root_helpers.hist_to_graph(
            # bkg_stack.GetStack().Last(), show_bin_width=True, norm=True
            # total_mc_histo,
            mc_rebinned_for_ratio,
            show_bin_width=True,
            norm=True,
        )

        ax2.plot(err_band_ratio, "2", fillcolor=1, fillstyle=3254)

        # Calculate and draw the ratio
        # ratio_hist = data_hist.histo.Clone("ratio_hist")
        ratio_hist = data_rebinned_for_ratio
        # ratio_hist.Divide(bkg_stack.GetStack().Last())
        # ratio_hist.Divide(total_mc_histo)
        ratio_hist.Divide(mc_rebinned_for_ratio)
        ratio_graph = aplt.root_helpers.hist_to_graph(
            ratio_hist,
            show_bin_width=True,
        )
        ax2.plot(ratio_graph, "P0")

        # Add extra space at top of plot to make room for labels
        ax1.add_margins(top=0.05)

        ax2.set_ylim(0, 2.5)
        # ax2.set_ylim(0, 3)
        # ax2.set_ylim(1e-4, 3.5)

        ax2.draw_arrows_outside_range(ratio_graph)

        ax2.set_xlim(x_min, x_max)

        # Go back to top axes to add labels
        ax1.cd()

        # Add the ATLAS Label
        # aplt.atlas_label(text="Not ATLAS! CMS...", loc="upper left", size=0)
        # ax1.text(0.2, 0.84, "#sqrt{s} = 13 TeV, 137 fb^{-1}", size=22, align=13)

        tdrstyle.CMS_lumi(fig.canvas, 4, 0, ec.name)

        # Save the plot as a PDF
        ec_nice_name = ec.name.replace("+", "_")
        os.system(f"mkdir -p classification_plots/{ec_nice_name}")

        fig.savefig(
            f"classification_plots/{ec_nice_name}/{ec_nice_name}_{histogram_name}.png"
        )
        fig.savefig(
            f"classification_plots/{ec_nice_name}/{ec_nice_name}_{histogram_name}.pdf"
        )

        os.system(
            f"cp $MUSIC_BASE/NanoMUSiC/NanoEventClass/scripts/index.php classification_plots/{ec_nice_name}/index.php"
        )


if __name__ == "__main__":
    print("\n\n📶 [ MUSiC classification - Plotter ] 📶\n")

    args = parse_args()

    aplt.set_atlas_style()
    tdrstyle.setTDRStyle()
    ROOT.gStyle.SetMarkerSize(0.5)
    ROOT.gStyle.SetLabelSize(25, "XYZ")

    with EventClassCollection(
        get_source_files(args.inputs), args.PATTERNS
    ) as event_classes:
        with Pool(min(len(event_classes), 120)) as p:
            r = list(
                tqdm(p.imap(plot_event_class, event_classes), total=len(event_classes))
            )
        # for ec in tqdm(event_classes, total=len(event_classes)):
        # print(ec)

        # plot_event_class(ec, h)

    os.system(
        f"cp $MUSIC_BASE/NanoMUSiC/NanoEventClass/scripts/index.php classification_plots/index.php"
    )
    os._exit(os.EX_OK)
