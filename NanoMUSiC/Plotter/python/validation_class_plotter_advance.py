#!/usr/bin/env python3
import argparse
import glob
import os
import sys
import warnings
from decimal import Decimal
from functools import partial
from multiprocessing import Pool
from pathlib import Path

import atlasplots as aplt
import ROOT
import tdrstyle
import tomli
from colors import PROCESS_GROUP_STYLES
from tqdm import tqdm
from validation_class_advance import EventClassCollection

warnings.simplefilter("ignore", UserWarning)


# ROOT.gErrorIgnoreLevel = ROOT.kError
ROOT.gErrorIgnoreLevel = 6000


years_glob = {
    "2016*": {"name": "2016", "lumi": "36.3"},  #
    "2017": {"name": "2017", "lumi": "41.5"},  #
    "2018": {"name": "2018", "lumi": "59.8"},  #
    "*": {"name": "", "lumi": "138"},
}


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-i",
        "--input",
        help="Path to the results of classification.",
        type=str,
        default="classification_outputs",
    )

    parser.add_argument(
        "-o",
        "--output",
        help="Path where to save the plots.",
        type=str,
        default="classification_plots",
    )

    parser.add_argument(
        "--year",
        help="Will merge and plot for a given year.",
        choices=list(years_glob.keys()),
        required=True,
    )

    parser.add_argument(
        "--histogram", help="Histogram to plot.", type=str, default="all"
    )

    parser.add_argument(
        "-p",
        "--patterns",
        nargs="+",
        help="List of pattern to filter the Event Classes.",
    )

    parser.add_argument(
        "--plot_config",
        help="Configuration file for the plots in toml format.",
        required=True,
    )

    args = parser.parse_args()

    return args


def get_source_files(path, year):
    return list(
        filter(
            lambda f: ("cutflow" not in f),
            glob.glob(f"{path}/{year}/*/*.root"),
        )
    )


def plot_event_class(ec, histogram_name, histograms_to_plot, year, output_path):
    # skip MET histogram for non-MET classes
    # if histogram_name == "met":
    #     if not ec.has_met:
    #         return

    # print(ec.histos)
    # print(histogram_name)
    if histogram_name in ec.histos:
        # Create a figure and axes
        fig, (ax1, ax2) = aplt.ratio_plot(
            name=f"ratio_{ec.name}_{histogram_name}",
            figsize=(int(1.5 * 800), int(1.5 * 600)),
            hspace=0.1,
        )

        # Set axis titles
        ax2.set_xlabel("XXXXX", loc="right", titlesize=30)
        ax1.set_ylabel("Events / 10 GeV", titlesize=30)
        ax2.set_ylabel("", loc="centre", titlesize=30)
        x_label_text = histograms_to_plot[histogram_name]["xlabel"]

        # if histogram_name == "sum_pt":
        #     ax1.set_ylabel("Events", titlesize=30)

        # ax2.set_ylabel("", loc="centre", titlesize=30)
        # x_label_text = ""
        # if histogram_name == "invariant_mass":
        #     if "MET" in ec.name:
        #         x_label_text = r"M_{T} [GeV]"
        #     else:
        #         x_label_text = r"M [GeV]"
        # elif histogram_name == "sum_pt":
        #     x_label_text = r"S_{T} [GeV]"
        # elif histogram_name == "met":
        #     x_label_text = r"p_{T}^{miss} [GeV]"
        # elif histogram_name == "sum_pt":
        #     x_label_text = r""
        # else:
        #     print(
        #         f"ERROR: Could not set x axis label. Invalid option ({histogram_name})."
        #     )
        #     sys.exit(-1)

        ax2.text(
            0.85,
            0.15,
            x_label_text,
            size=30,
        )

        y_label_text = r"#frac{Data}{Simulation}"
        ax2.text(
            0.1,
            0.47,
            y_label_text,
            size=30,
            angle=90,
        )

        data_hist = ec.get_data_histogram(histogram_name)
        if (
            "eta" not in histogram_name
            or "phi" not in histogram_name
            or "delta" not in histogram_name
            or "dR" not in histogram_name
        ):
            data_hist.scale_for_plot()
        limits = data_hist.get_limits()

        # if limits is None:
        #     if histogram_name == "sum_pt":
        #         limits = -0.1, 2.5
        #     else:
        #         return

        x_min, x_max = limits
        x_min = x_min - (x_max - x_min) * 0.05
        x_max = x_max + (x_max - x_min) * 0.05

        # build the data as a graph
        data_graph = aplt.root_helpers.hist_to_graph(data_hist.histo)

        mc_hists = ec.get_mc_histograms_per_process_group(histogram_name)
        mc_hists_keys_sorted = sorted(mc_hists, key=lambda x: mc_hists[x].integral())

        total_mc_histo = None
        bkg_stack = ROOT.THStack("bkg", "")
        for hist in mc_hists_keys_sorted:
            if histogram_name != "sum_pt":
                mc_hists[hist].scale_for_plot()
            mc_hists[hist].histo.SetFillColor(
                PROCESS_GROUP_STYLES[mc_hists[hist].process_group].color
            )
            mc_hists[hist].histo.SetLineWidth(0)
            bkg_stack.Add(mc_hists[hist].histo)
            if total_mc_histo is None:
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
        ax1.plot(err_band, "2", fillcolor=13, fillstyle=3254, linewidth=0)

        # if histogram_name != "sum_pt":
        #     ax1.set_ylim(ec.get_y_low(histogram_name) / 50)
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
        # if histogram_name != "sum_pt":
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

        ax2.plot(err_band_ratio, "2", fillcolor=12, fillstyle=3254)

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

        ax2.draw_arrows_outside_range(ratio_graph)

        ax2.set_xlim(x_min, x_max)

        # Go back to top axes to add labels
        ax1.cd()

        # Add legend
        # if histogram_name == "sum_pt":
        #     legend = ax1.legend(
        #         loc=(
        #             0.6,
        #             0.15,
        #             1 - ROOT.gPad.GetRightMargin(),
        #             1 - ROOT.gPad.GetTopMargin() - 0.05,
        #         ),
        #         textsize=14,
        #     )
        #     legend.AddEntry(
        #         data_graph,
        #         f"Data ({Decimal(data_hist.histo.GetBinContent(1)):.2E})",
        #         "EP",
        #     )

        #     for hist in reversed(mc_hists_keys_sorted):
        #         legend.AddEntry(
        #             mc_hists[hist].histo,
        #             f"{hist} ({Decimal(mc_hists[hist].histo.GetBinContent(1)):.2E})",
        #             "F",
        #         )

        #     legend.AddEntry(err_band, "Stat. Uncert.", "F")

        # Add the CMS Label
        tdrstyle.CMS_lumi(
            fig.canvas,
            4,
            0,
            ec.name,
            years_glob[year]["lumi"],
            years_glob[year]["name"],
        )

        # Save the plot
        ec_nice_name = ec.name.replace("+", "_")

        os.system(f"mkdir -p {output_path}/{ec_nice_name}")

        fig.savefig(
            f"{output_path}/{ec_nice_name}/{ec_nice_name}_{histogram_name}{(lambda x: f'_{x}' if x != '' else '')(years_glob[year]['name'])}.png"
        )
        fig.savefig(
            f"{output_path}/{ec_nice_name}/{ec_nice_name}_{histogram_name}{(lambda x: f'_{x}' if x != '' else '')(years_glob[year]['name'])}.pdf"
        )
        fig.savefig(
            f"{output_path}/{ec_nice_name}/{ec_nice_name}_{histogram_name}{(lambda x: f'_{x}' if x != '' else '')(years_glob[year]['name'])}.svg"
        )


if __name__ == "__main__":
    print("\n\n📶 [ MUSiC classification - Plotter ] 📶\n")

    args = parse_args()

    aplt.set_atlas_style()
    tdrstyle.setTDRStyle()
    ROOT.gStyle.SetMarkerSize(0.5)
    ROOT.gStyle.SetLabelSize(25, "XYZ")

    year = args.year
    plot_config = tomli.loads(Path(args.plot_config).read_text(encoding="utf-8"))

    val_classes = plot_config["validation_classes"]
    if args.patterns:
        val_classes = {val_class: val_classes[val_class] for val_class in args.patterns}

    if years_glob[year]["name"] != "":
        print(f"\n-- Building Event Classes for year {years_glob[year]['name']}...")
    else:
        print("\n-- Building Event Classes for Run2...")

    histograms_to_plot = plot_config
    hist_to_validate = {}
    hist_to_validate = histograms_to_plot["validation_classes"]
    histograms_to_plot.pop("color_dict")
    histograms_to_plot.pop("aggregation_dict")
    histograms_to_plot.pop("validation_classes")
    print(f"Args.histogram type: {type(args.histogram)}")
    if args.histogram != "all":
        histograms_to_plot = {args.histogram: histograms_to_plot[args.histogram]}

    validation_classes = EventClassCollection(
        get_source_files(args.input, year),
        val_classes,
        histograms_to_plot,
        hist_to_validate,
    )

    for histogram_name in histograms_to_plot:
        print(f"\n---- Plotting {histogram_name}...")
        with Pool(min(len(validation_classes), 100)) as p:
            list(
                tqdm(
                    p.imap(
                        partial(
                            plot_event_class,
                            histogram_name=histogram_name,
                            histograms_to_plot=histograms_to_plot,
                            year=year,
                            output_path=args.output,
                        ),
                        validation_classes,
                    ),
                    total=len(validation_classes),
                )
            )

    os._exit(os.EX_OK)
