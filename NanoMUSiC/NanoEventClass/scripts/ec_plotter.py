#!/usr/bin/env python3

import ROOT
from glob import glob
import sys
import copy
import tdrstyle
import atlasplots as aplt
from colors import PROCESS_GROUP_STYLES
from tqdm import tqdm
from decimal import Decimal
import os
from multiprocessing import Pool

# import warnings
# warnings.simplefilter("ignore", UserWarning)

# ROOT.gErrorIgnoreLevel = ROOT.kError
ROOT.gErrorIgnoreLevel = 6000
ROOT.gSystem.AddIncludePath("-I../NanoMUSiC/NanoEventClass/include")

if (
    ROOT.gSystem.CompileMacro(
        "../NanoMUSiC/NanoEventClass/src/NanoEventClass.cpp", "Ok"
    )
    == 0
):
    sys.exit("ERROR: Could not compile NanoEventClass.cpp.")
if (
    ROOT.gSystem.CompileMacro("../NanoMUSiC/NanoEventClass/src/Distribution.cpp", "Ok")
    == 0
):
    sys.exit("ERROR: Could not compile Distribution.cpp.")
if (
    ROOT.gSystem.CompileMacro(
        "../NanoMUSiC/NanoEventClass/src/distribution_factory.cpp", "Ok"
    )
    == 0
):
    sys.exit("ERROR: Could not compile Distribution.cpp.")

ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.TH1.AddDirectory(False)
ROOT.TDirectory.AddDirectory(False)
ROOT.gROOT.SetBatch(True)
ROOT.EnableThreadSafety()


def get_source_files(path, year):
    return list(
        filter(
            lambda f: ("cutflow" not in f),
            glob(f"{path}/*.root"),
        )
    )


years_glob = {
    "2016*": {"name": "2016", "lumi": "36.3"},  #
    "2017": {"name": "2017", "lumi": "41.5"},  #
    "2018": {"name": "2018", "lumi": "59.8"},  #
    "*": {"name": "", "lumi": "138"},
}


def to_root_latex(class_name):
    root_latex_name = ""
    has_suffix = False
    is_first_object = True

    for i, p in enumerate(class_name.split("_")):
        if i > 0:
            if "Muon" in p:
                root_latex_name += str(p[0]) + r"#mu"
                is_first_object = False

            if "Electron" in p:
                if is_first_object:
                    root_latex_name += str(p[0]) + r"e"
                    is_first_object = False
                else:
                    root_latex_name += r" + " + str(p[0]) + r"e"

            if "Tau" in p:
                if is_first_object:
                    root_latex_name += str(p[0]) + r"#tau"
                    is_first_object = False
                else:
                    root_latex_name += r" + " + str(p[0]) + r"#tau"

            if "Photon" in p:
                if is_first_object:
                    root_latex_name += str(p[0]) + r"#gamma"
                    is_first_object = False
                else:
                    root_latex_name += r" + " + str(p[0]) + r"#gamma"

            if "bJet" in p:
                root_latex_name += r" + " + str(p[0]) + r"bjet"

            if p[1:] == "Jet":
                root_latex_name += r" + " + str(p[0]) + r"jet"

            if "MET" in p:
                root_latex_name += r" + " + r"p_{T}^{miss}"

            if r"+X" in p:
                root_latex_name += r" " + r"incl."
                has_suffix = True

            if r"+NJet" in p:
                root_latex_name += r" " + r"jet inc."
                has_suffix = True

    if not has_suffix:
        root_latex_name += " excl."

    return root_latex_name


def make_plot(args):
    (
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
    ) = args

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
        if "MET" in class_name:
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
    ax1.text(
        0.19,
        0.9,
        f"Event class: {to_root_latex(class_name)}",
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
                1 - ROOT.gPad.GetTopMargin() - 0.05,
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
    ec_nice_name = class_name.replace("+", "_")
    # os.system(f"mkdir -p {output_path}/{ec_nice_name}")

    fig.savefig(
        f"{output_path}/{ec_nice_name}/{ec_nice_name}_{distribution_name}{(lambda x: f'_{x}' if x!=''  else '') (years_glob[year]['name'])}.png"
    )
    fig.savefig(
        f"{output_path}/{ec_nice_name}/{ec_nice_name}_{distribution_name}{(lambda x: f'_{x}' if x!=''  else '') (years_glob[year]['name'])}.pdf"
    )
    fig.savefig(
        f"{output_path}/{ec_nice_name}/{ec_nice_name}_{distribution_name}{(lambda x: f'_{x}' if x!=''  else '') (years_glob[year]['name'])}.svg"
    )

    # os.system(
    #     f"cp $MUSIC_BASE/NanoMUSiC/NanoEventClass/scripts/index.php {output_path}/{ec_nice_name}/index.php"
    # )


def main():
    aplt.set_atlas_style()
    tdrstyle.setTDRStyle()
    ROOT.gStyle.SetMarkerSize(0.5)
    ROOT.gStyle.SetLabelSize(25, "XYZ")

    input_files = get_source_files("/disk1/silva/classification_histograms", "2018")

    print("Creating EC Collection ...")
    ec_collection = ROOT.NanoEventClassCollection(
        input_files,
        # ["EC_2Muon*", "EC_2Electron*"],
        [
            # "EC_2Muon_1MET",
            # "EC_2Muon*",
            # "*",
            # "EC_2Muon_1Electron_3bJet+X",
            # "EC_2Muon+X",
            # "EC_2Muon+NJet",
            # "EC_1Electron_2Tau_1Jet_2bJet_1MET",
            "*",
            # "EC_2Muon_2Tau_1MET",
            # "EC_2Muon_1Electron_1Tau_2Jet_1MET+X"
        ],
    )

    print("Building plots ...")
    plot_props = []
    for dist in tqdm(ROOT.distribution_factory(ec_collection, False)):
        plot = dist.get_plot_props()
        plot_props.append(
            [
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
                "/disk1/silva/classification_plots",
                "2018",
            ]
        )

        # prepare output area
        ec_nice_name = plot.class_name.replace("+", "_")
        # mkdir_command += f"/disk1/silva/classification_plots/{ec_nice_name} "
        if not os.path.exists(f"/disk1/silva/classification_plots/{ec_nice_name}"):
            os.makedirs(f"/disk1/silva/classification_plots/{ec_nice_name}")

        # os.system(
        #     f"cp $MUSIC_BASE/NanoMUSiC/NanoEventClass/scripts/index.php /disk1/silva/classification_outputs/{ec_nice_name}/index.php"
        # )

        # make_plot(plot_props[-1])

    # os.system(f"mkdir -p {mkdir_command}")
    print()

    print("Saving plots ...")
    with Pool(min(len(plot_props), 30)) as p:
        list(
            tqdm(
                p.imap_unordered(
                    func=make_plot,
                    iterable=plot_props,
                    # chunksize=int(len(plot_props) / 15),
                ),
                total=len(plot_props),
            )
        )

    os.system(r"find classification_plots/ -type d -exec cp $MUSIC_BASE/NanoMUSiC/NanoEventClass/scripts/index.php {} \;")

    print("Done.")


if __name__ == "__main__":
    main()
