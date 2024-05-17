#!/usr/bin/env python3

import ROOT
import sys
import tdrstyle
import atlasplots as aplt
from colors import PROCESS_GROUP_STYLES
from decimal import Decimal
import os
import json
from multiprocessing import Pool
from pvalue import get_integral_pvalue
from ec_tools import get_source_files, to_root_latex
from rich.progress import Progress, track


years_glob = {
    "2016*": {"name": "2016", "lumi": "36.3"},  #
    "2017": {"name": "2017", "lumi": "41.5"},  #
    "2018": {"name": "2018", "lumi": "59.8"},  #
    # "[2017,2018]": {"name": "2017+2018", "lumi": "101"},  #
    "*": {"name": "", "lumi": "138"},
}


def make_plot_task(args):
    return make_plot(*args)


def make_plot(
    class_name,
    distribution_name,
    x_min,
    x_max,
    y_min,
    y_max,
    total_data_histogram_first_bin_content,
    data_graph,
    mc_histograms,
    mc_uncertainty,
    ratio_graph,
    ratio_mc_error,
    output_path,
    year,
    p_value,
):
    # print("--> ", class_name, distribution_name, x_min, x_max, y_min, y_max)
    # skip MET histogram for non-MET classes
    if distribution_name == "met" and "MET" not in class_name:
        print(f"[INFO] Skipping MET distribution for {class_name} ...")
        return

    # Create a figure and axes
    fig, (ax1, ax2) = aplt.ratio_plot(
        name=f"ratio_{class_name}_{distribution_name}",
        figsize=(int(1.5 * 800), int(1.5 * 600)),
        hspace=0.1,
    )

    # Set axis titles
    ax2.set_xlabel("XXXXX", loc="right", titlesize=30)
    ax1.set_ylabel("Events / 10 GeV", titlesize=30)
    if distribution_name == "counts":
        ax1.set_ylabel("Events", titlesize=30)

    ax2.set_ylabel("", loc="centre", titlesize=30)
    x_label_text = ""
    if distribution_name == "invariant_mass":
        if "1MET" in class_name:
            x_label_text = r"M_{T} [GeV]"
        else:
            x_label_text = r"M [GeV]"
    elif distribution_name == "sum_pt":
        x_label_text = r"S_{T} [GeV]"
    elif distribution_name == "met":
        x_label_text = r"p_{T}^{miss} [GeV]"
    elif distribution_name == "counts":
        x_label_text = r""
    else:
        print(
            f"ERROR: Could not set x axis label. Invalid option ({distribution_name})."
        )
        sys.exit(-1)

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

    # print class name
    event_class_str = "Event class: {}".format(to_root_latex(class_name))
    if p_value and distribution_name == "counts":
        if p_value > 0:
            event_class_str += f" (p = {p_value:.2g})"
    ax1.text(
        0.19,
        0.9,
        event_class_str,
        size=26,
        align=13,
    )

    if distribution_name == "counts":
        x_min = 0
        x_max = 2
    x_min = x_min - (x_max - x_min) * 0.05
    x_max = x_max + (x_max - x_min) * 0.05

    mc_hists = {}
    for pg, hist in mc_histograms:
        mc_hists[pg] = hist
    mc_hists_keys_sorted = sorted(
        filter(lambda pg: pg != "Data", mc_hists),
        key=lambda pg: mc_hists[pg].Integral(),
    )

    bkg_stack = ROOT.THStack("bkg", "")
    for pg in mc_hists_keys_sorted:
        # mc_hists[pg].Print("all")
        mc_hists[pg].SetFillColor(PROCESS_GROUP_STYLES[pg].color)
        mc_hists[pg].SetLineWidth(0)
        bkg_stack.Add(mc_hists[pg])

    # Draw the stacked histogram on the axes
    ax1.plot(bkg_stack)
    ax1.plot(mc_uncertainty, "2", fillcolor=13, fillstyle=3254, linewidth=0)

    ax1.set_yscale("log")  # uncomment to use log scale for y axis

    # print("##### Plotting data graph")
    ax1.plot(data_graph, "P0")

    ax1.set_xlim(x_min, x_max)

    # Use same x-range in lower axes as upper axes
    ax2.set_xlim(ax1.get_xlim())
    ax2.set_xlim(x_min, x_max)

    # Draw line at y=1 in ratio panel
    line = ROOT.TLine(ax1.get_xlim()[0], 1, ax1.get_xlim()[1], 1)
    ax2.plot(line)

    ax2.plot(ratio_mc_error, "2", fillcolor=12, fillstyle=3254, linewidth=0)

    ax2.plot(ratio_graph, "P0")

    if distribution_name != "counts":
        ax1.set_ylim(y_min / 1000, y_max)
    else:
        ax1.set_ylim(ax1.get_ylim()[0], y_max)

    # add extra space at top of plot to make room for labels
    ax1.add_margins(top=0.15)

    ax2.set_ylim(0, 2.5)
    # ax2.set_ylim(0, 3)

    ax2.draw_arrows_outside_range(ratio_graph)

    ax2.set_xlim(x_min, x_max)

    # go back to top axes to add labels
    ax1.cd()

    # add legend
    if distribution_name == "counts":
        legend = ax1.legend(
            loc=(
                0.6,
                0.1,
                1 - ROOT.gPad.GetRightMargin(),
                1 - ROOT.gPad.GetTopMargin() - 0.1,
            ),
            textsize=14,
        )

        legend.AddEntry(
            data_graph,
            f"Data ({Decimal(total_data_histogram_first_bin_content):.2e})",
            "ep",
        )

        for hist in reversed(mc_hists_keys_sorted):
            legend.AddEntry(
                mc_hists[hist],
                f"{hist} ({Decimal(mc_hists[hist].GetBinContent(1)):.2e})",
                "f",
            )

        legend.AddEntry(mc_uncertainty, "Bkg. Uncert.", "f")

    # add the cms label
    tdrstyle.CMS_lumi(
        fig.canvas,
        4,
        0,
        "",
        years_glob[year]["lumi"],
        years_glob[year]["name"],
    )

    # Save the plot
    output_file_path = f"{output_path}/{class_name}/{class_name}_{distribution_name}{(lambda x: f'_{x}' if x!=''  else '') (years_glob[year]['name'])}"
    output_file_path = output_file_path.replace("+", "_")

    fig.savefig(f"{output_file_path}.png")
    fig.savefig(f"{output_file_path}.pdf")
    fig.savefig(f"{output_file_path}.svg")
    # fig.savefig(f"{output_file_path}.C")

    return "{} - {} - {}".format(class_name, distribution_name, year)


def ec_plotter(
    input_dir: str, patterns: list[str], output_dir: str = "classification_plots"
):
    aplt.set_atlas_style()
    tdrstyle.setTDRStyle()
    ROOT.gStyle.SetMarkerSize(0.5)
    ROOT.gStyle.SetLabelSize(25, "XYZ")

    for year in years_glob:
        input_files = get_source_files(input_dir, year)
        print("Creating EC Collection ...")
        ec_collection = ROOT.NanoEventClassCollection(
            input_files,
            patterns,
            # # ["EC_2Muon*", "EC_2Electron*"],
            # [
            #     # "*",
            #     # # "EC_2Muon*",
            #     # "EC_1Electron+NJet",
            #     # "EC_2Muon_1MET",
            #     # "EC_2Muon_2Tau_1MET",
            #     # "EC_2Muon_1Photon_1bJet_1MET+X",
            #     # "EC_2Muon_1Electron_3bJet+X",
            #     # "EC_2Muon+X",
            #     # "EC_4Muon",
            #     # "EC_4Electron",
            #     # "EC_2Muon_2Electron",
            #     # "EC_1Muon_1Electron+1MET",
            #     # "EC_1Muon_2Jet",
            #     # "EC_1Muon+X",
            #     # "EC_1Electron+X",
            #     # "EC_1Photon+X",
            #     # "EC_1Muon_2bJet",
            #     # "EC_1Muon_1Jet",
            #     # "EC_1Electron_2Jet",
            #     # "EC_2Muon+NJet",
            #     "EC_1Electron_2Tau_1Jet_2bJet_1MET",
            #     "EC_2Muon_1Tau_2Jet_1bJet_1MET+NJet",
            #     # "EC_2Muon_2Tau_1MET",
            #     # "EC_2Muon_1Electron_1Tau_2Jet_1MET+X",
            # ],
        )

        print("Building distributions ...")
        plot_props = []
        plots_data = {}
        distributions = ROOT.distribution_factory(ec_collection, True)

        if len(distributions) == 0:
            print("ERROR: No matching classes were found.")
            sys.exit(1)

        for dist in track(distributions):
            plot = dist.get_plot_props()
            if dist.m_distribution_name == "counts":
                p_value_props = dist.get_integral_pvalue_props()
                p_value_data = get_integral_pvalue(
                    p_value_props.total_data,
                    p_value_props.total_mc,
                    p_value_props.sigma_total,
                    p_value_props.sigma_stat,
                    p_value_props.total_per_process_group,
                )
                # print(p_value_data)
                p_value = p_value_data["p-value"]
                veto_reason = p_value_data["Veto Reason"]

                # json for counts plot
                plots_data[plot.class_name] = {}
                plots_data[plot.class_name]["data_count"] = (
                    plot.total_data_histogram.GetBinContent(1)
                )
                plots_data[plot.class_name]["data_uncert"] = (
                    plot.total_data_histogram.GetBinError(1)
                )
                plots_data[plot.class_name]["mc"] = {}
                mc_hists = {}
                for pg, hist in plot.mc_histograms:
                    mc_hists[pg] = hist
                mc_hists_keys_sorted = sorted(
                    filter(lambda pg: pg != "Data", mc_hists),
                    key=lambda pg: mc_hists[pg].Integral(),
                )
                for pg in mc_hists_keys_sorted:
                    plots_data[plot.class_name]["mc"][pg] = mc_hists[pg].GetBinContent(
                        1
                    )
                plots_data[plot.class_name]["mc_uncert"] = (
                    plot.mc_uncertainty.GetErrorY(0)
                )
                plots_data[plot.class_name]["p_value"] = p_value
                plots_data[plot.class_name]["veto_reason"] = veto_reason

        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        with open(
            "{}/plot_data_{}.json".format(
                output_dir, years_glob[year]["name"].replace("+", "_")
            ),
            "w",
            encoding="utf-8",
        ) as f:
            json.dump(plots_data, f, ensure_ascii=False, indent=4)

        distributions = ROOT.distribution_factory(ec_collection, False)
        for dist in track(distributions):
            plot = dist.get_plot_props()
            plot_props.append(
                (
                    plot.class_name,
                    plot.distribution_name,
                    plot.x_min,
                    plot.x_max,
                    plot.y_min,
                    plot.y_max,
                    plot.total_data_histogram.GetBinContent(1),
                    plot.data_graph,
                    plot.mc_histograms,
                    plot.mc_uncertainty,
                    plot.ratio_graph,
                    plot.ratio_mc_error_band,
                    output_dir,
                    year,
                    plots_data[plot.class_name]["p_value"],
                )
            )

            # prepare output area
            ec_nice_name = plot.class_name.replace("+", "_")
            if not os.path.exists("{}/{}".format(output_dir, ec_nice_name)):
                os.makedirs("{}/{}".format(output_dir, ec_nice_name))

            # make_plot(
            #     (
            #         plot.class_name,
            #         plot.distribution_name,
            #         plot.x_min,
            #         plot.x_max,
            #         plot.y_min,
            #         plot.y_max,
            #         plot.total_data_histogram.GetBinContent(1),
            #         plot.data_graph,
            #         plot.mc_histograms,
            #         plot.mc_uncertainty,
            #         plot.ratio_graph,
            #         plot.ratio_mc_error_band,
            #         output_dir,
            #         year,
            #     )
            # )

        print()

        print("Saving plots ...")
        with Pool(min(len(plot_props), 124)) as p:
            with Progress() as progress:
                task = progress.add_task("Plotting ...", total=len(plot_props))
                for job in p.imap_unordered(make_plot_task, plot_props):
                    progress.console.print("Done: {}".format(job))
                    progress.advance(task)

    print("Copying index.php ...")
    os.system(
        r"find ___OUTPUT_DIR___/ -type d -exec cp $MUSIC_BASE/NanoMUSiC/NanoEventClass/scripts/index.php {} \;".replace(
            "___OUTPUT_DIR___", output_dir
        )
    )

    print("Done.")


if __name__ == "__main__":
    ec_plotter()
