#!/usr/bin/env python3

from typing import Any
import ROOT as root
import atlasplots as aplt
from array import array
import multiprocessing
from tqdm import tqdm
import toml  # type: ignore
import argparse
import os
from decimal import Decimal

aplt.set_atlas_style()
import tdrstyle

root.gStyle.SetMarkerSize(0.5)

from colors import PROCESS_GROUP_STYLES

years = ["2016APV", "2016", "2017", "2018"]


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-c",
        "--config",
        required=True,
        help='Task configuration (TOML) file, produced by "analysis_config_builder.py"',
    )

    parser.add_argument(
        "-a", "--analysis", help="Which analysis to plot", required=True
    )

    parser.add_argument("-y", "--year", help="Year to be processed.", default="all")

    parser.add_argument(
        "--histogram",
        help="Which histogram to plot (dafeult: h_invariant_mass).",
        default="h_invariant_mass",
    )

    parser.add_argument(
        "--debug", help="Will print debugging info", action="store_true"
    )

    parser.add_argument(
        "--no-data", help="Will not plot data.", action="store_true", default=False
    )

    parser.add_argument(
        "-o",
        "--output",
        help="Output base path.",
        type=str,
        default=".",
    )

    parser.add_argument(
        "--output-suffix",
        help="Suffix to the output file.",
        type=str,
        default="",
    )

    parser.add_argument(
        "--mc-sample",
        help="MC sample to plot. Will filter all other samples.",
        type=str,
        default="",
    )

    parser.add_argument(
        "--xmin",
        help="xmin",
        type=float,
        default=0,
    )

    parser.add_argument(
        "--xmax",
        help="xmax",
        type=float,
        default=2000,
    )

    parser.add_argument(
        "-i",
        "--input",
        help="Path to the results of classification_outputs.",
        type=str,
        default="classification_outputs",
    )

    args = parser.parse_args()

    return args


def get_reweighting_factor(config):
    if "reweighting_factor" in config.keys():
        return config["reweighting_factor"]
    return 1.0


def files_to_process(file_limit, year, output_files):
    if file_limit < 0:
        return list(
            filter(
                lambda file: f"_{year}/" in file,
                output_files,
            )
        )
    return list(
        filter(
            lambda file: f"_{year}/" in file,
            output_files,
        )
    )[:file_limit]


def get_histogram(args, root_file, sample, year, process_group, xsec_order):
    # [ttbar_to_1mu_2bjet_2jet_MET]_[DrellYan]_[NLO]_[DYJetsToLL_M-50_13TeV_AM]_[2018]_[JetScale_Down]_[h_ht_had_lep]
    if args.debug:
        print(
            f"Getting histogram for: {(args.analysis, sample, year, process_group, xsec_order)} : [{args.analysis}]_[{process_group}]_[{xsec_order}]_[{sample}]_[{year}]_[Nominal]_[{args.histogram}]"
        )
        root_file.Print("all")

    available_objects = [key.GetName() for key in root_file.GetListOfKeys()]
    requested_key = f"[{args.analysis}]_[{process_group}]_[{xsec_order}]_[{sample}]_[{year}]_[Nominal]_[{args.histogram}]"
    if requested_key in available_objects:
        return root_file.Get(
            f"[{args.analysis}]_[{process_group}]_[{xsec_order}]_[{sample}]_[{year}]_[Nominal]_[{args.histogram}]"
        )
    else:
        return None


def main():
    print("\n\n📶 [ MUSiC classification - Fast Plotter ] 📶\n")

    # parse arguments
    args = parse_args()

    if args.debug:
        print("Will run in DEBUG mode ...")

    # load analysis config file
    task_config_file: str = args.config
    task_config: dict[str, Any] = toml.load(task_config_file)

    print("Building jobs ...")
    plotter_arguments_mc = []
    plotter_arguments_data = []
    for sample in tqdm(task_config, unit=" tasks"):
        if sample != "Lumi" and sample != "Global":
            for year in years:
                if f"das_name_{year}" in task_config[sample].keys():
                    if year == args.year or args.year == "all":
                        # if not (
                        #     os.path.isdir(f"{args.output}/classification_plots/{year}")
                        # ):
                        # os.system(
                        #     # f"mkdir -p {args.output}/classification_plots/{year}"
                        # )
                        if task_config[sample]["is_data"]:
                            plotter_arguments_data.append(
                                {
                                    "sample": sample,
                                    "year": year,
                                    "is_data": task_config[sample]["is_data"],
                                    "process_group": "Data",
                                    "xsec_order": "DUMMY",
                                    "reweighting_factor": 1.0,
                                }
                            )
                        else:
                            if args.mc_sample == "" or args.mc_sample == sample:
                                plotter_arguments_mc.append(
                                    {
                                        "sample": sample,
                                        "year": year,
                                        "is_data": task_config[sample]["is_data"],
                                        "process_group": task_config[sample][
                                            "ProcessGroup"
                                        ],
                                        "xsec_order": task_config[sample]["XSecOrder"],
                                        "reweighting_factor": get_reweighting_factor(
                                            task_config[sample]
                                        ),
                                    }
                                )

    input_files_data = {}
    for s in plotter_arguments_data:
        input_files_data[f"{s['sample']}_{s['year']}"] = root.TFile.Open(
            f"{args.input}/{s['year']}/{s['sample']}/{s['sample']}_{s['year']}.root"
        )

    input_files_mc = {}
    for s in plotter_arguments_mc:
        input_files_mc[f"{s['sample']}_{s['year']}"] = root.TFile.Open(
            f"{args.input}/{s['year']}/{s['sample']}/{s['sample']}_{s['year']}.root"
        )

    # [ttbar_to_1ele_2bjet_2jet_MET]_[DrellYan]_[NLO]_[DYJetsToLL_M-50_13TeV_AM]_[2018]_[JetScale_Down]_[h_ht_had_lep]
    # z_to_mu_mu_x_Z_mass

    # Set the ATLAS Style
    # aplt.set_atlas_style()

    # Create a figure and axes
    fig, (ax1, ax2) = aplt.ratio_plot(name="ratio", figsize=(800, 600), hspace=0.05)

    x_min = args.xmin
    x_max = args.xmax

    # add all Data histograms
    data_histograms = []
    for i in range(len(plotter_arguments_data)):
        hist_ptr = get_histogram(
            args,
            input_files_data[
                f"{plotter_arguments_data[i]['sample']}_{plotter_arguments_data[i]['year']}"
            ],
            plotter_arguments_data[i]["sample"],
            plotter_arguments_data[i]["year"],
            plotter_arguments_data[i]["process_group"],
            plotter_arguments_data[i]["xsec_order"],
        )
        if hist_ptr != None:
            data_histograms.append(hist_ptr)

    if len(data_histograms) == 0:
        print(
            f"ERROR: Could not get any Data histogram ({args.analysis} - {args.histogram})."
        )

    data_hist = data_histograms[0].Clone()
    for i in range(1, len(data_histograms)):
        data_hist.Add(data_histograms[i])
    data_hist.Scale(10.0, "width")

    if args.debug:
        print("data_hist:")
        data_hist.Print()

    # data_hist.SetMaximum(800.0)

    # Stack the background
    mc_hists = {}
    for i in range(0, len(plotter_arguments_mc)):
        if plotter_arguments_mc[i]["process_group"] not in mc_hists:
            this_histogram = get_histogram(
                args,
                input_files_mc[
                    f"{plotter_arguments_mc[i]['sample']}_{plotter_arguments_mc[i]['year']}"
                ],
                plotter_arguments_mc[i]["sample"],
                plotter_arguments_mc[i]["year"],
                plotter_arguments_mc[i]["process_group"],
                plotter_arguments_mc[i]["xsec_order"],
            )
            if this_histogram != None:
                this_histogram = this_histogram.Clone()
                this_histogram.Scale(plotter_arguments_mc[i]["reweighting_factor"])
                this_histogram.Scale(10.0, "width")
                this_histogram.SetFillColor(
                    PROCESS_GROUP_STYLES[plotter_arguments_mc[i]["process_group"]].color
                )
                this_histogram.SetLineWidth(0)

                mc_hists[plotter_arguments_mc[i]["process_group"]] = this_histogram
        else:
            this_histogram = get_histogram(
                args,
                input_files_mc[
                    f"{plotter_arguments_mc[i]['sample']}_{plotter_arguments_mc[i]['year']}"
                ],
                plotter_arguments_mc[i]["sample"],
                plotter_arguments_mc[i]["year"],
                plotter_arguments_mc[i]["process_group"],
                plotter_arguments_mc[i]["xsec_order"],
            )
            if this_histogram != None:
                this_histogram = this_histogram.Clone()
                this_histogram.Scale(plotter_arguments_mc[i]["reweighting_factor"])
                this_histogram.Scale(10.0, "width")

                mc_hists[plotter_arguments_mc[i]["process_group"]].Add(this_histogram)

    mc_hists_keys_sorted = sorted(mc_hists, key=lambda x: mc_hists[x].Integral())

    # Add legend
    legend = ax1.legend(
        loc=(
            0.58,
            0.3,
            1 - root.gPad.GetRightMargin() - 0.03,
            1 - root.gPad.GetTopMargin() - 0.03,
        ),
        textsize=6,
    )

    bkg_stack = root.THStack("bkg", "")
    for hist in mc_hists_keys_sorted:
        bkg_stack.Add(mc_hists[hist])

    for hist in reversed(mc_hists_keys_sorted):
        legend.AddEntry(
            mc_hists[hist],
            f"{hist} ({Decimal(mc_hists[hist].Integral()):.2E})",
            "F",
        )

    # Draw the stacked histogram on these axes
    ax1.plot(bkg_stack)

    # Plot the MC stat error as a hatched band
    err_band = aplt.root_helpers.hist_to_graph(
        bkg_stack.GetStack().Last(), show_bin_width=True
    )
    ax1.plot(err_band, "2", fillcolor=1, fillstyle=3254, linewidth=0)

    ax1.set_ylim(1e-6)
    ax1.set_yscale("log")  # uncomment to use log scale for y axis

    # Plot the data as a graph
    data_graph = aplt.root_helpers.hist_to_graph(data_hist)
    legend.AddEntry(data_graph, f"Data ({Decimal(data_hist.Integral()):.2E})", "EP")
    if not (args.no_data):
        ax1.plot(data_graph, "P")
    ax1.set_xlim(x_min, x_max)

    # Use same x-range in lower axes as upper axes
    ax2.set_xlim(ax1.get_xlim())
    ax2.set_xlim(x_min, x_max)

    # Draw line at y=1 in ratio panel
    line = root.TLine(ax1.get_xlim()[0], 1, ax1.get_xlim()[1], 1)
    ax2.plot(line)

    # Plot the relative error on the ratio axes
    err_band_ratio = aplt.root_helpers.hist_to_graph(
        bkg_stack.GetStack().Last(), show_bin_width=True, norm=True
    )
    ax2.plot(err_band_ratio, "2", fillcolor=1, fillstyle=3254)

    # Calculate and draw the ratio
    ratio_hist = data_hist.Clone("ratio_hist")
    ratio_hist.Divide(bkg_stack.GetStack().Last())
    ratio_graph = aplt.root_helpers.hist_to_graph(ratio_hist)
    ax2.plot(ratio_graph, "P0")

    # Add extra space at top of plot to make room for labels
    ax1.add_margins(top=0.16)

    # Set axis titles
    ax2.set_xlabel("X [GeV]")
    ax1.set_ylabel("Events / 10 GeV")
    ax2.set_ylabel("Obs./Pred.", loc="centre")

    ax2.set_ylim(0, 2.5)
    # ax2.set_ylim(0, 3)
    # ax2.set_ylim(1e-4, 3.5)

    ax2.draw_arrows_outside_range(ratio_graph)

    ax2.set_xlim(x_min, x_max)

    # Go back to top axes to add labels
    ax1.cd()

    # Add the ATLAS Label
    # aplt.atlas_label(text="Not ATLAS! CMS...", loc="upper left", size=0)
    # ax1.text(0.2, 0.84, "#sqrt{s} = 13 TeV, ~60 fb^{-1}", size=22, align=13)

    # # Save the plot as a PDF
    fig.savefig(f"data_vs_mc_{args.output_suffix}.png")
    fig.savefig(f"data_vs_mc_{args.output_suffix}.pdf")
    # fig.savefig("data_vs_mc.root")
    # fig.savefig("data_vs_mc.C")


if __name__ == "__main__":
    root.gROOT.SetBatch()
    main()