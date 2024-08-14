import sys
import tdrstyle
import atlasplots as aplt
from colors import PROCESS_GROUP_STYLES
from decimal import Decimal
from tools import to_root_latex, configure_root
from metadata import Years
from pvalue import get_integral_pvalue
from typing import Any
import os
from metadata import make_ec_nice_name

from ROOT import TFile, THStack, TLine, gPad

import matplotlib as mpl


configure_root()
mpl.use("Agg")

# def make_uncertainties_plot(
#     class_name,
#     distribution_name,
#     x_min,
#     x_max,
#     y_min,
#     y_max,
#     total_data_histogram_first_bin_content,
#     data_graph,
#     mc_histograms,
#     mc_uncertainty,
#     ratio_graph,
#     ratio_mc_error,
#     output_path,
#     year,
#     p_value,
#     systematic_uncertainties,
#     statistical_uncertainties,
#     total_mc_histogram,
# ) -> None:
#     x = []
#     for _, hist in mc_histograms:
#         for i in range(1, hist.GetNbinsX() + 2):
#             x.append(hist.GetXaxis().GetBinLowEdge(i))
#         break
#
#     grid_dim = math.ceil(math.sqrt(systematic_uncertainties.size() + 1))
#     fig, axs = plt.subplots(grid_dim, grid_dim, figsize=(grid_dim * 6, grid_dim * 2))
#     for i, data in enumerate(systematic_uncertainties):
#         syst_name, values = data
#         y_to_plot = [
#             v / total_mc_histogram.GetBinContent(i + 1) for i, v in enumerate(values)
#         ]
#         y_to_plot = [y_to_plot[0]] + y_to_plot
#         axs[int(i / grid_dim)][int(i % grid_dim)].step(x, y_to_plot, label=syst_name)
#         axs[int(i / grid_dim)][int(i % grid_dim)].set_ylabel("Relative Uncert.")
#         axs[int(i / grid_dim)][int(i % grid_dim)].legend()
#         # axs[int(i / grid_dim)][int(i % grid_dim)].set_yscale("log")
#
#     y_to_plot = []
#     for i, v in enumerate(statistical_uncertainties):
#         if total_mc_histogram.GetBinContent(i + 1) > 0:
#             y_to_plot.append(v / total_mc_histogram.GetBinContent(i + 1))
#
#         else:
#             y_to_plot.append(0)
#     y_to_plot = [y_to_plot[0]] + y_to_plot
#     i = systematic_uncertainties.size()
#     axs[int(i / grid_dim)][int(i % grid_dim)].step(x, y_to_plot, label="Statist.")
#     axs[int(i / grid_dim)][int(i % grid_dim)].set_ylabel("Relative Uncert.")
#     axs[int(i / grid_dim)][int(i % grid_dim)].legend()
#     # axs[int(i / grid_dim)][int(i % grid_dim)].set_yscale("log")
#
#     plt.tight_layout()
#     year_label = year
#     if year_label == "Run2":
#         year_label = ""
#     output_file_path = f"{output_path}/{class_name}/Uncerts_{class_name}_{distribution_name}{(lambda x: f'_{x}' if x!='' else '_Run2') (year_label)}"
#     output_file_path = output_file_path.replace("+", "_")
#
#     fig.savefig(f"{output_file_path}.png")
#     fig.savefig(f"{output_file_path}.pdf")
#     fig.savefig(f"{output_file_path}.svg")
#


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
        if dist.has_mc():
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
                for pg in mc_hists_keys_sorted:
                    counts[dist.m_year_to_plot][plot.class_name.decode("utf-8")]["mc"][
                        pg.decode("utf-8")
                    ] = mc_hists[pg].GetBinContent(1)
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


def build_plot_jobs_task(args: tuple[str, dict[str, Any] | None, str]) -> list[Any]:
    output_dir, plots_data, distribution_file = args

    temp_plot_props: list[Any] = []

    root_file = TFile.Open(distribution_file)
    distribution_names = [k.GetName() for k in root_file.GetListOfKeys()]

    for dist_name in distribution_names:
        dist = root_file.Get(dist_name)
        if dist.has_mc():
            plot = dist.make_plot_props()
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
                    dist.m_systematics_uncertainties,
                    dist.m_statistical_uncert,
                    dist.m_total_mc_histogram,
                )
            )

            # prepare output area
            ec_nice_name = make_ec_nice_name(plot.class_name).replace("+", "_")
            if not os.path.exists("{}/{}".format(output_dir, ec_nice_name)):
                os.makedirs("{}/{}".format(output_dir, ec_nice_name))

    root_file.Close()

    return temp_plot_props


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
    _systematic_uncertainties,
    _statistical_uncertainties,
    _total_mc_histogram,
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

    # print("##### Plotting data graph")
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
    ax1.add_margins(top=0.15)

    ax2.set_ylim(0, 2.5)

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
    output_file_path = f"{output_path}/{ec_nice_name}/{ec_nice_name}_{distribution_name}{(lambda x: f'_{x}' if x!='' else '_Run2') (year_label)}"
    output_file_path = output_file_path.replace("+", "_")

    fig.savefig(f"{output_file_path}.png")
    fig.savefig(f"{output_file_path}.pdf")
    fig.savefig(f"{output_file_path}.svg")
    # fig.savefig(f"{output_file_path}.C")

    return "{} - {} - {}".format(class_name, distribution_name, year)
