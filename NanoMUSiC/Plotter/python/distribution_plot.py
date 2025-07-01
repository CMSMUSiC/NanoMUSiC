import os
import sys
from decimal import Decimal
from itertools import cycle
from multiprocessing import Value
from typing import Any

import matplotlib as mpl
import matplotlib.cm as cm
import matplotlib.pyplot as plt

mpl.use("Agg")

import atlasplots as aplt
import numpy as np
import tdrstyle
from colors import PROCESS_GROUP_STYLES
from metadata import Years, make_ec_nice_name
from pvalue import get_integral_pvalue
from ROOT import TFile, THStack, TLine, gPad, kRed
from tools import configure_root, to_root_latex

configure_root()
mpl.use("Agg")


MC_THRESHOLD = 0.1


def p_value_task(distribution_file: str):
    counts = {}
    for year in Years.years_to_plot():
        counts[Years.years_to_plot()[year]["name"]] = {}

    root_file = TFile.Open(distribution_file)
    if root_file.GetListOfKeys().GetEntries() == 0:
        print(
            "ERROR: Could not collect keys from input file: {}.".format(
                distribution_file
            )
        )
        sys.exit(-1)

    distribution_names = [k.GetName() for k in root_file.GetListOfKeys()]
    for dist_name in distribution_names:
        dist = root_file.Get(dist_name)
        if dist.has_mc(MC_THRESHOLD):
            if dist.m_distribution_name == "counts":
                plot = dist.make_plot_props()
                p_value_props = dist.make_integral_pvalue_props()
                try:
                    p_value_data = get_integral_pvalue(
                        p_value_props.total_data,
                        p_value_props.total_mc,
                        p_value_props.sigma_total,
                        p_value_props.sigma_stat,
                        p_value_props.total_per_process_group,
                    )
                except ZeroDivisionError as e:
                    print("ZeroDivisionError caught.")
                    print(
                        "ERROR: Could not calculate p-value for {}.".format(dist_name)
                    )
                    print("ZeroDivisionError caugth. Details: {}".format(e))
                    sys.exit(-1)
                except Exception as e:
                    print(
                        "ERROR: Could not calculate p-value for {} ({}).\n{}".format(
                            dist_name, distribution_file, e
                        )
                    )
                    sys.exit(-1)

                # print(p_value_data)
                p_value = p_value_data["p-value"]
                veto_reason = p_value_data["Veto Reason"]

                # json for counts plot
                counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")] = {}
                counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")][
                    "data_count"
                ] = plot.total_data_histogram.GetBinContent(1)
                counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")][
                    "data_uncert"
                ] = plot.total_data_histogram.GetBinError(1)
                counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")]["mc"] = {}
                mc_hists = {}
                for pg, hist in plot.mc_histograms:
                    mc_hists[pg] = hist
                mc_hists_keys_sorted = sorted(
                    filter(lambda pg: pg != "Data", mc_hists),
                    key=lambda pg: mc_hists[pg].Integral(),
                )
                total_mc = 0
                for pg in mc_hists_keys_sorted:
                    counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")]["mc"][
                        pg.decode("utf-8")
                    ] = mc_hists[pg].GetBinContent(1)
                    total_mc += mc_hists[pg].GetBinContent(1)
                counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")][
                    "total_mc"
                ] = total_mc
                counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")][
                    "mc_uncert"
                ] = plot.mc_uncertainty.GetErrorY(0)
                counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")][
                    "p_value"
                ] = p_value
                counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")][
                    "veto_reason"
                ] = veto_reason

    root_file.Close()

    return counts, distribution_file


def build_plot_jobs_task(
    args: tuple[str, dict[str, Any] | None, dict[str, Any] | None, str],
) -> list[Any]:
    output_dir, plots_data, scan_data, distribution_file = args

    temp_plot_props: list[Any] = []

    root_file = TFile.Open(distribution_file)
    distribution_names = [k.GetName() for k in root_file.GetListOfKeys()]
    for dist_name in distribution_names:
        dist = root_file.Get(dist_name)
        if dist.has_mc(MC_THRESHOLD):
            plot = dist.make_plot_props()

            this_scan_data = None
            if (
                scan_data
                and plot.distribution_name != "counts"
                and plot.year_to_plot == "Run2"
            ):
                if plot.distribution_name != "met":
                    this_scan_data = scan_data[plot.distribution_name].get(
                        make_ec_nice_name(plot.class_name)
                    )
                else:
                    if "MET" in make_ec_nice_name(plot.class_name):
                        this_scan_data = scan_data[plot.distribution_name].get(
                            make_ec_nice_name(plot.class_name)
                        )

            temp_plot_props.append(
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
                    plot.year_to_plot,
                    plots_data[plot.year_to_plot][plot.class_name]["p_value"]
                    if plots_data
                    else None,
                    this_scan_data,
                    dist.m_systematics_uncertainties,
                    dist.m_statistical_uncert,
                    dist.m_total_mc_histogram,
                    plot.uncert_props,
                )
            )

            # prepare output area
            ec_nice_name = make_ec_nice_name(plot.class_name).replace("+", "_")
            if not os.path.exists("{}/{}".format(output_dir, ec_nice_name)):
                os.makedirs("{}/{}".format(output_dir, ec_nice_name))

    root_file.Close()

    return temp_plot_props


def uncertainty_plot(output_path, uncert_props):
    # Combine multiple
    colors = (
        list(cm.get_cmap("tab10").colors)  #
        + list(cm.get_cmap("tab20").colors)  #
        + list(cm.get_cmap("tab20b").colors)  #
        + list(cm.get_cmap("tab20c").colors)  #
    )
    colors = cycle(colors)  # infinite iterator

    bins = list(uncert_props.bins)
    bins_idx = []
    this_bins = []
    for idx, _bin in enumerate(bins):
        if uncert_props.x_min <= _bin and _bin <= uncert_props.x_max:
            this_bins.append(_bin)
            bins_idx.append(idx)
    bins_idx.pop()

    # Create figure and axis
    fig, ax = plt.subplots(figsize=(8, 6))

    # Plot histograms
    uncert_xsec_LO = np.zeros(len(bins_idx))
    uncert_xsec_NLO = np.zeros(len(bins_idx))

    for uncert, color in zip(uncert_props.uncertanties, colors):
        uncert, values = str(uncert.first), list(uncert.second)
        assert len(values) == len(bins) - 1

        if uncert.startswith("xsec"):
            if "NLO" in uncert:
                uncert_xsec_NLO = uncert_xsec_NLO + np.power(values, 2)[bins_idx]
            else:
                uncert_xsec_LO = uncert_xsec_LO + np.power(values, 2)[bins_idx]

            continue

        this_vals = np.array(values)[bins_idx]
        ax.stairs(
            this_vals,
            this_bins,
            label=uncert,
            fill=False,
            color=color,
            linewidth=2.0,
            alpha=0.8,
            baseline=None,
        )

    ax.stairs(
        np.sqrt(uncert_xsec_NLO),
        this_bins,
        label="xsec_NLO",
        fill=False,
        color=next(colors),
        linewidth=2.0,
        alpha=0.8,
        baseline=None,
    )

    ax.stairs(
        np.sqrt(uncert_xsec_LO),
        this_bins,
        label="xsec_LO",
        fill=False,
        color=next(colors),
        linewidth=2.0,
        alpha=0.8,
        baseline=None,
    )

    # Get handles and labels
    handles, labels = ax.get_legend_handles_labels()
    # Sort them by label
    sorted_pairs = sorted(zip(labels, handles), key=lambda x: x[0])
    sorted_labels, sorted_handles = zip(*sorted_pairs)

    # Add sorted legend
    ax.legend(
        sorted_handles,
        sorted_labels,
        # loc="upper left",
        ncol=1,
        fontsize="small",
        bbox_to_anchor=(1.01, 1.0),  # x = just outside (1.05), y = top (1.0)
        borderaxespad=0.0,
    )

    plt.yscale("log")

    ax.set_ylabel("Relative Uncert.")
    match uncert_props.distribution_name:
        case "sum_pt":
            ax.set_xlabel("Sum pT")
        case "invariant_mass":
            ax.set_xlabel("Inv. Mass")
        case "met":
            ax.set_xlabel("MET")
        case "counts":
            ax.set_xlabel("")
        case _:
            raise ValueError("Invalid distribution name")

    fig.tight_layout()

    # Save the plot
    year_label = uncert_props.year_to_plot
    if year_label == "Run2":
        year_label = ""

    ec_nice_name = make_ec_nice_name(uncert_props.class_name)
    output_file_path = f"{output_path}/{ec_nice_name}/{uncert_props.distribution_name}{(lambda x: f'_{x}' if x != '' else '_Run2')(year_label)}_uncertainties"
    output_file_path = output_file_path.replace("+", "_")

    fig.savefig(f"{output_file_path}.pdf")
    plt.close()


def make_plot_task(args):
    # make_uncertainties_plot(*args)
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
    p_value: float | None,
    scan_data: Any,
    _systematic_uncertainties,
    _statistical_uncertainties,
    _total_mc_histogram,
    uncert_props,
) -> str:
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
        x_label_text = str(distribution_name)
        # print(
        #     f"ERROR: Could not set x axis label. Invalid option ({distribution_name})."
        # )
        # sys.exit(-1)

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
    event_class_str = str(class_name)
    if class_name.startswith("EC_"):
        event_class_str = "Event class: {}".format(to_root_latex(class_name))
    if p_value and distribution_name == "counts":
        if p_value > 0:
            event_class_str += f" (Integral p = {p_value:.2g})"

    ax1.text(
        0.2,
        0.9,
        event_class_str,
        size=26,
        align=13,
    )

    if scan_data:
        ax1.text(
            0.2,
            0.85,
            r"p: ___P_DATA___  - #tilde{p}: ___P_TILDE___".replace(
                "___P_DATA___", f"{scan_data['p_value_data']:.2g}"
            ).replace("___P_TILDE___", f"{scan_data['p_tilde']:.2g}"),
            size=26,
            align=13,
        )
        ax1.text(
            0.2,
            0.8,
            r"RoI = [___LOWER___ - ___UPPER___] GeV".replace(
                "___LOWER___", f"{scan_data['lower_edge']:.1f}"
            ).replace("___UPPER___", f"{scan_data['upper_edge']:.1f}"),
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

    bkg_stack = THStack("bkg", "")
    for pg in mc_hists_keys_sorted:
        mc_hists[pg].SetFillColor(PROCESS_GROUP_STYLES[pg].color)
        mc_hists[pg].SetLineWidth(0)
        bkg_stack.Add(mc_hists[pg])

    # Draw the stacked histogram on the axes
    ax1.plot(bkg_stack, expand=False)
    ax1.plot(
        mc_uncertainty, "2", fillcolor=13, fillstyle=3254, linewidth=0, expand=False
    )

    ax1.set_yscale("log")  # uncomment to use log scale for y axis

    ax1.plot(data_graph, "P0", expand=False)

    ax1.set_xlim(x_min, x_max)

    # Use same x-range in lower axes as upper axes
    ax2.set_xlim(ax1.get_xlim())
    ax2.set_xlim(x_min, x_max)

    # Draw line at y=1 in ratio panel
    line = TLine(ax1.get_xlim()[0], 1, ax1.get_xlim()[1], 1)
    ax2.plot(line)

    ax2.plot(ratio_mc_error, "2", fillcolor=12, fillstyle=3254, linewidth=0)

    ax2.plot(ratio_graph, "P0")

    def ylimits(y_min: float, y_max: float) -> tuple[float, float]:
        LOWEST_LEVEL = 1e-8
        if y_min / 1000.0 < LOWEST_LEVEL:
            y_min = LOWEST_LEVEL
        else:
            y_min = y_min / 1000.0

        if y_min > y_max:
            y_max = y_min * 1.1

        return y_min, y_max

    if distribution_name != "counts":
        ax1.set_ylim(*ylimits(y_min, y_max))
    else:
        try:
            ax1.set_ylim(*ylimits(y_min, y_max))
        except ValueError as e:
            print(
                "Error [ {} - {} - {} ]: {}".format(
                    class_name, distribution_name, year, e
                )
            )
            sys.exit(-1)

    # add extra space at top of plot to make room for labels
    if scan_data:
        ax1.add_margins(top=0.25)
    else:
        ax1.add_margins(top=0.15)

    ratio_y_min = 0
    ratio_y_max = 2.5
    ax2.set_ylim(ratio_y_min, ratio_y_max)

    ax2.draw_arrows_outside_range(ratio_graph)

    ax2.set_xlim(x_min, x_max)

    if scan_data:
        ymin, ymax = ax1.get_ylim()

        roi_lower_edge = scan_data["lower_edge"]
        roi_line_lower = TLine(roi_lower_edge, ymin, roi_lower_edge, ymax)
        roi_line_lower.SetLineColor(kRed)
        roi_line_lower.SetLineStyle(2)
        ax1.plot(roi_line_lower, expand=True)

        roi_line_lower_ratio = TLine(
            roi_lower_edge, ratio_y_min, roi_lower_edge, ratio_y_max
        )
        roi_line_lower_ratio.SetLineColor(kRed)
        roi_line_lower_ratio.SetLineStyle(2)
        ax2.plot(roi_line_lower_ratio, expand=True)

        roi_upper_edge = scan_data["upper_edge"]
        roi_line_upper = TLine(roi_upper_edge, ymin, roi_upper_edge, ymax)
        roi_line_upper.SetLineColor(kRed)
        roi_line_upper.SetLineStyle(2)
        expand = True
        if roi_upper_edge > x_max:
            expand = False
        ax1.plot(roi_line_upper, expand=expand)

        roi_line_upper_ratio = TLine(
            roi_upper_edge, ratio_y_min, roi_upper_edge, ratio_y_max
        )
        roi_line_upper_ratio.SetLineColor(kRed)
        roi_line_upper_ratio.SetLineStyle(2)
        ax2.plot(roi_line_upper_ratio, expand=expand)

    # go back to top axes to add labels
    ax1.cd()

    # add legend
    if distribution_name == "counts":
        legend = ax1.legend(
            loc=(
                0.6,
                0.1,
                1 - gPad.GetRightMargin(),
                1 - gPad.GetTopMargin() - 0.1,
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

    year_label = year
    if year_label == "Run2":
        year_label = ""

    # add the cms label
    tdrstyle.CMS_lumi(
        fig.canvas,
        4,
        0,
        "",
        Years.get_lumi(year),
        year_label,
    )

    # Save the plot
    ec_nice_name = make_ec_nice_name(class_name)
    output_file_path = f"{output_path}/{ec_nice_name}/{distribution_name}{(lambda x: f'_{x}' if x != '' else '_Run2')(year_label)}"
    output_file_path = output_file_path.replace("+", "_")

    fig.savefig(f"{output_file_path}.pdf")
    # fig.savefig(f"{output_file_path}.png")
    # fig.savefig(f"{output_file_path}.svg")
    # fig.savefig(f"{output_file_path}.C")

    uncertainty_plot(output_path, uncert_props)

    return "{} - {} - {}".format(class_name, distribution_name, year)
