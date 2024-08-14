import os
import sys
import tdrstyle
import fnmatch
import atlasplots as aplt
import json
from multiprocessing import Pool
from rich.progress import Progress
from metadata import Years
from typing import Any

from ROOT import gStyle

from distribution_plot import make_plot_task, p_value_task, build_plot_jobs_task


def plotter(
    input_dir: str,
    patterns: list[str],
    output_dir: str,
    num_cpus: int = 128,
    is_validation=False,
):
    if not os.path.isdir(input_dir):
        print("ERROR: Input directory does not exists.")
        sys.exit(-1)

    aplt.set_atlas_style()
    tdrstyle.setTDRStyle()
    gStyle.SetMarkerSize(0.5)
    gStyle.SetLabelSize(25, "XYZ")

    # os.system("rm -rf {}".format(output_dir))
    os.system("mkdir -p {}".format(output_dir))

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

    # Will calculate p-values
    # for validation, it should be None
    integral_pvalues_data = None

    if not is_validation:
        integral_pvalues_data = {}
        for year in Years.years_to_plot():
            integral_pvalues_data[Years.years_to_plot()[year]["name"]] = {}

        with Pool(min(len(distribution_files), num_cpus)) as p:
            with Progress() as progress:
                task = progress.add_task(
                    "Calculating p-values [{} distribution files] ...".format(
                        len(distribution_files)
                    ),
                    total=len(distribution_files),
                )
                for counts, _ in p.imap_unordered(p_value_task, distribution_files):
                    for year in counts:
                        for ec in counts[year]:
                            integral_pvalues_data[year][ec] = counts[year][ec]
                    # progress.console.print(dist_file)
                    progress.advance(task)

        with open(
            "{}/integral_pvalues_data.json".format(output_dir),
            "w",
            encoding="utf-8",
        ) as f:
            json.dump(integral_pvalues_data, f, ensure_ascii=False, indent=4)

    # Will build plot jobs
    plot_props: list[Any] = []
    with Pool(min(len(distribution_files), num_cpus)) as p:
        with Progress() as progress:
            task = progress.add_task(
                "Building plot jobs [{} {} files] ...".format(
                    len(distribution_files),
                    "validation" if is_validation else "distribution",
                ),
                total=len(distribution_files),
            )
            for this_plot_props in p.imap_unordered(
                build_plot_jobs_task,
                [(output_dir, integral_pvalues_data, d) for d in distribution_files],
            ):
                plot_props += this_plot_props
                progress.advance(task)

    # Will make and save plots
    with Pool(min(len(plot_props), num_cpus)) as p:
        with Progress() as progress:
            task = progress.add_task(
                "Saving {} plots ...".format(len(plot_props)),
                total=len(plot_props),
            )
            for job in p.imap_unordered(make_plot_task, plot_props):
                if is_validation:
                    progress.console.print("Done: {}".format(job))
                progress.advance(task)

    print("Copying index.php ...")
    os.system(
        r"find ___OUTPUT_DIR___/ -type d -exec cp $MUSIC_BASE/NanoMUSiC/Plotter/assets/index.php {} \;".replace(
            "___OUTPUT_DIR___", output_dir
        )
    )

    print("Done.")
