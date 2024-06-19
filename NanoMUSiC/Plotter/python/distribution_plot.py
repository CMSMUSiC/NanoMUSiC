import sys
import tdrstyle
import atlasplots as aplt
from colors import PROCESS_GROUP_STYLES
from decimal import Decimal
from tools import to_root_latex
from metadata import Years

from ROOT import THStack, TLine, gPad


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
) -> str:
    # print("--> ", class_name, distribution_name, x_min, x_max, y_min, y_max)
    # skip MET histogram for non-MET classes
    if distribution_name == "met" and "MET" not in class_name:
        print(f"[INFO] Skipping MET distribution for {class_name} ...")
        return "{} - {} - {}".format(class_name, distribution_name, year)

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

    bkg_stack = THStack("bkg", "")
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
    line = TLine(ax1.get_xlim()[0], 1, ax1.get_xlim()[1], 1)
    ax2.plot(line)

    ax2.plot(ratio_mc_error, "2", fillcolor=12, fillstyle=3254, linewidth=0)

    ax2.plot(ratio_graph, "P0")

    if distribution_name != "counts":
        ax1.set_ylim(y_min / 1000, y_max)
    else:
        try:
            ax1.set_ylim(ax1.get_ylim()[0], y_max)
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
    output_file_path = f"{output_path}/{class_name}/{class_name}_{distribution_name}{(lambda x: f'_{x}' if x!='' else '_Run2') (year_label)}"
    output_file_path = output_file_path.replace("+", "_")

    fig.savefig(f"{output_file_path}.png")
    fig.savefig(f"{output_file_path}.pdf")
    fig.savefig(f"{output_file_path}.svg")
    # fig.savefig(f"{output_file_path}.C")

    return "{} - {} - {}".format(class_name, distribution_name, year)
