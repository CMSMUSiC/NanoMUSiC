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

    parser.add_argument("-y", "--year", help="Year to be processed.", default="")

    parser.add_argument(
        "--histogram",
        help="Which histogram to plot (dafeult: h_invariant_mass).",
        default="h_invariant_mass",
    )

    parser.add_argument("--debug", help="print debugging info", action="store_true")

    parser.add_argument(
        "-o",
        "--output",
        help="Output base path.",
        type=str,
        default=".",
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
        help="Path to the results of validation_outputs.",
        type=str,
        default="validation_outputs",
    )

    args = parser.parse_args()

    if not (args.year):
        raise Exception(
            'ERROR: Could not start plotter. For now, "--year" is required.'
        )

    return args


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
            f"Getting histogram for: {(args.analysis, sample, year, process_group, xsec_order)}"
        )
    return root_file.Get(
        f"[{args.analysis}]_[{process_group}]_[{xsec_order}]_[{sample}]_[{year}]_[Nominal]_[{args.histogram}]"
    )


def main():
    print("\n\n📶 [ MUSiC Validation - Fast Plotter ] 📶\n")

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
                    if year == args.year:
                        if not (
                            os.path.isdir(f"{args.output}/validation_plots/{year}")
                        ):
                            os.system(f"mkdir -p {args.output}/validation_plots/{year}")
                        if task_config[sample]["is_data"]:
                            plotter_arguments_data.append(
                                {
                                    "sample": sample,
                                    "year": year,
                                    "is_data": task_config[sample]["is_data"],
                                    "process_group": "Data",
                                    "xsec_order": "DUMMY",
                                }
                            )
                        else:
                            plotter_arguments_mc.append(
                                {
                                    "sample": sample,
                                    "year": year,
                                    "is_data": task_config[sample]["is_data"],
                                    "process_group": task_config[sample][
                                        "ProcessGroup"
                                    ],
                                    "xsec_order": task_config[sample]["XSecOrder"],
                                }
                            )

    input_files_data = {}
    for s in plotter_arguments_data:
        input_files_data[s["sample"]] = root.TFile.Open(
            f"{args.input}/{s['year']}/{s['sample']}/{s['sample']}_{s['year']}.root"
        )

    input_files_mc = {}
    for s in plotter_arguments_mc:
        input_files_mc[s["sample"]] = root.TFile.Open(
            f"{args.input}/{s['year']}/{s['sample']}/{s['sample']}_{s['year']}.root"
        )

    # [ttbar_to_1ele_2bjet_2jet_MET]_[DrellYan]_[NLO]_[DYJetsToLL_M-50_13TeV_AM]_[2018]_[JetScale_Down]_[h_ht_had_lep]
    # z_to_mu_mu_x_Z_mass

    # Set the ATLAS Style
    # aplt.set_atlas_style()

    # Create a figure and axes
    fig, (ax1, ax2) = aplt.ratio_plot(name="fig1", figsize=(800, 800), hspace=0.05)
    x_min = args.xmin
    x_max = args.xmax

    # add all Data histograms
    data_hist = get_histogram(
        args,
        input_files_data[plotter_arguments_data[0]["sample"]],
        plotter_arguments_data[0]["sample"],
        plotter_arguments_data[0]["year"],
        plotter_arguments_data[0]["process_group"],
        plotter_arguments_data[0]["xsec_order"],
    ).Clone()
    for i in range(1, len(plotter_arguments_data)):
        data_hist.Add(
            get_histogram(
                args,
                input_files_data[plotter_arguments_data[i]["sample"]],
                plotter_arguments_data[i]["sample"],
                plotter_arguments_data[i]["year"],
                plotter_arguments_data[i]["process_group"],
                plotter_arguments_data[i]["xsec_order"],
            )
        )

    if args.debug:
        print("data_hist:")
        data_hist.Print()

    # data_hist.SetMaximum(800.0)

    # Stack the background
    # TODO: check integral and reorder
    mc_hists = {}
    for i in range(0, len(plotter_arguments_mc)):
        print(f"Adding to stack: {plotter_arguments_mc[i]['sample']}")
        if plotter_arguments_mc[i]["process_group"] not in mc_hists:
            mc_hists[plotter_arguments_mc[i]["process_group"]] = get_histogram(
                args,
                input_files_mc[plotter_arguments_mc[i]["sample"]],
                plotter_arguments_mc[i]["sample"],
                plotter_arguments_mc[i]["year"],
                plotter_arguments_mc[i]["process_group"],
                plotter_arguments_mc[i]["xsec_order"],
            ).Clone()

            mc_hists[plotter_arguments_mc[i]["process_group"]].SetFillColor(
                PROCESS_GROUP_STYLES[plotter_arguments_mc[i]["process_group"]].color
                # root.TColor.GetColor("#4daf4a")
            )
            mc_hists[plotter_arguments_mc[i]["process_group"]].SetLineWidth(0)
        else:
            mc_hists[plotter_arguments_mc[i]["process_group"]].Add(
                get_histogram(
                    args,
                    input_files_mc[plotter_arguments_mc[i]["sample"]],
                    plotter_arguments_mc[i]["sample"],
                    plotter_arguments_mc[i]["year"],
                    plotter_arguments_mc[i]["process_group"],
                    plotter_arguments_mc[i]["xsec_order"],
                )
            )

    mc_hists_keys_sorted = sorted(mc_hists, key=lambda x: mc_hists[x].Integral())

    print(mc_hists_keys_sorted)

    # Add legend
    legend = ax1.legend(
        loc=(
            0.68,
            # 5,
            0.35,
            1 - root.gPad.GetRightMargin() - 0.03,
            1 - root.gPad.GetTopMargin() - 0.03,
        ),
        textsize=10,
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

    # # Create a figure and axes
    # fig, ax = aplt.subplots(1, 1, name="fig2", figsize=(800, 600))
    # print(ax.get_xlim())

    # # Define a distribution
    # sqroot = root.TF1("sqroot", "x*gaus(0) + [3]*abs(sin(x)/x)", 0, 10)
    # sqroot.SetParameters(10, 4, 1, 20)

    # data_hist = root.TH1F("hist", "Random Histogram", 50, 0, 10)
    # data_hist.FillRandom("sqroot", 20000)
    # print(ax.get_xlim())

    # # data_hist.SetAxisRange(0.0, 3.0, "X")
    # data_hist.GetXaxis().SetRangeUser(0, 3)
    # # data_hist.GetXaxis().SetLimits(0, 3)

    # print(ax.get_xlim())

    # # Draw the histogram on these axes
    # ax.plot(data_hist, label="data_hist", labelfmt="F")
    # print(ax.get_xlim())
    # ax.set_xlim(0, 3)
    # print(ax.get_xlim())

    # # Add extra space at top of plot to make room for labels
    # ax.add_margins(top=0.16)

    # # Set axis titles
    # ax.set_xlabel("X [GeV]")
    # ax.set_ylabel("Events / 0.2 GeV")

    # # Save the plot as a PNG
    # fig.savefig("data_vs_mc.png")

    # Draw the stacked histogram on these axes
    ax1.plot(bkg_stack)

    # Plot the MC stat error as a hatched band
    err_band = aplt.root_helpers.hist_to_graph(
        bkg_stack.GetStack().Last(), show_bin_width=True
    )
    ax1.plot(err_band, "2", fillcolor=1, fillstyle=3254, linewidth=0)

    # ax1.set_ylim(1e-9)
    ax1.set_yscale("log")  # uncomment to use log scale for y axis

    # Plot the data as a graph
    data_graph = aplt.root_helpers.hist_to_graph(data_hist)
    legend.AddEntry(data_graph, "Data", "EP")
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
    ax2.set_ylabel("Data / Pred.", loc="centre")

    ax2.set_ylim(0, 2)
    ax2.draw_arrows_outside_range(ratio_graph)

    ax2.set_xlim(x_min, x_max)

    # Go back to top axes to add labels
    ax1.cd()

    # Add the ATLAS Label
    # aplt.atlas_label(text="Not ATLAS! CMS...", loc="upper left", size=0)
    # ax1.text(0.2, 0.84, "#sqrt{s} = 13 TeV, ~60 fb^{-1}", size=22, align=13)

    # # Save the plot as a PDF
    fig.savefig("data_vs_mc.png")
    fig.savefig("data_vs_mc.pdf")
    # fig.savefig("data_vs_mc.root")
    # fig.savefig("data_vs_mc.C")


if __name__ == "__main__":
    root.gROOT.SetBatch()
    main()
