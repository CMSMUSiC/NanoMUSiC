import os
import sys
import tdrstyle
import fnmatch
import atlasplots as aplt
import json
from multiprocessing import Pool
from pvalue import get_integral_pvalue
from rich.progress import Progress, track
from metadata import Years

from ROOT import TFile, gStyle

from distribution_plot import make_plot_task


def plotter(
    input_dir: str,
    patterns: list[str],
    output_dir: str = "classification_plots",
    num_cpus: int = 120,
):
    aplt.set_atlas_style()
    tdrstyle.setTDRStyle()
    gStyle.SetMarkerSize(0.5)
    gStyle.SetLabelSize(25, "XYZ")

    os.system("rm -rf {}".format(output_dir))
    os.system("mkdir {}".format(output_dir))

    def make_distribution_paths(inputs_dir: str, patterns: list[str]) -> list[str]:
        distribution_paths: list[str] = []
        for root, _, files in os.walk(inputs_dir):
            for file in files:
                if any(
                    fnmatch.fnmatch(file, "*" + pattern.replace("+", "_") + ".root")
                    for pattern in patterns
                ):
                    distribution_paths.append(os.path.join(root, file))
        return distribution_paths

    distribution_files = make_distribution_paths(input_dir, patterns)
    if len(distribution_files) == 0:
        print("WARNING: No distribution matches the requirements.")
        sys.exit(1)

    plots_data = {}
    for year in Years.years_to_plot():
        plots_data[Years.years_to_plot()[year]["name"]] = {}

    for f in track(
        distribution_files,
        description="Calculating p-values [{} distributions] ...".format(
            len(distribution_files)
        ),
    ):
        root_file = TFile.Open(f)
        distribution_names = [k.GetName() for k in root_file.GetListOfKeys()]

        for dist_name in distribution_names:
            dist = TFile.Open(f).Get(dist_name)
            if dist.has_mc():
                if dist.m_distribution_name == "counts":
                    plot = dist.make_plot_props()
                    p_value_props = dist.make_integral_pvalue_props()
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
                    plots_data[dist.m_year_to_plot][plot.class_name] = {}
                    plots_data[dist.m_year_to_plot][plot.class_name]["data_count"] = (
                        plot.total_data_histogram.GetBinContent(1)
                    )
                    plots_data[dist.m_year_to_plot][plot.class_name]["data_uncert"] = (
                        plot.total_data_histogram.GetBinError(1)
                    )
                    plots_data[dist.m_year_to_plot][plot.class_name]["mc"] = {}
                    mc_hists = {}
                    for pg, hist in plot.mc_histograms:
                        mc_hists[pg] = hist
                    mc_hists_keys_sorted = sorted(
                        filter(lambda pg: pg != "Data", mc_hists),
                        key=lambda pg: mc_hists[pg].Integral(),
                    )
                    for pg in mc_hists_keys_sorted:
                        plots_data[dist.m_year_to_plot][plot.class_name]["mc"][pg] = (
                            mc_hists[pg].GetBinContent(1)
                        )
                    plots_data[dist.m_year_to_plot][plot.class_name]["mc_uncert"] = (
                        plot.mc_uncertainty.GetErrorY(0)
                    )
                    plots_data[dist.m_year_to_plot][plot.class_name]["p_value"] = (
                        p_value
                    )
                    plots_data[dist.m_year_to_plot][plot.class_name]["veto_reason"] = (
                        veto_reason
                    )

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    with open(
        "{}/plot_data.json".format(output_dir),
        "w",
        encoding="utf-8",
    ) as f:
        json.dump(plots_data, f, ensure_ascii=False, indent=4)

    plot_props = []
    for f in track(
        distribution_files,
        description="Building plot jobs [{} distributions] ...".format(
            len(distribution_files)
        ),
    ):
        root_file = TFile.Open(f)
        distribution_names = [k.GetName() for k in root_file.GetListOfKeys()]

        for dist_name in distribution_names:
            dist = TFile.Open(f).Get(dist_name)
            if dist.has_mc():
                plot = dist.make_plot_props()
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
                        plot.year_to_plot,
                        plots_data[plot.year_to_plot][plot.class_name]["p_value"],
                    )
                )

                # prepare output area
                ec_nice_name = plot.class_name.replace("+", "_")
                if not os.path.exists("{}/{}".format(output_dir, ec_nice_name)):
                    os.makedirs("{}/{}".format(output_dir, ec_nice_name))

    with Pool(min(len(plot_props), num_cpus)) as p:
        with Progress() as progress:
            task = progress.add_task(
                "Saving {} plots ...".format(len(plot_props)),
                total=len(plot_props),
            )
            for job in p.imap_unordered(make_plot_task, plot_props):
                progress.console.print("Done: {}".format(job))
                progress.advance(task)

    print("Copying index.php ...")
    os.system(
        r"find ___OUTPUT_DIR___/ -type d -exec cp $MUSIC_BASE/NanoMUSiC/Plotter/assets/index.php {} \;".replace(
            "___OUTPUT_DIR___", output_dir
        )
    )

    print("Done.")
