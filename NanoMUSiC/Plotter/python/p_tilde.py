import glob
from multiprocessing.pool import AsyncResult
from typing import Callable
import sys
import json
import os
import multiprocessing as mp
from distribution_model import DistributionType
from matplotlib.lines import lineStyles
from rich.progress import Progress
import matplotlib.pyplot as plt
import matplotlib as mpl
import mplhep as hep  # HEP (CMS) extensions/styling on top of mpl
import scipy
import numpy as np
from numpy.typing import NDArray
from functools import partial
from scan_results import ScanResults
from metadata import ClassType
from class_selections import class_selections
from tools import change_exponent


mpl.use("Agg")

# Load style sheet
plt.style.use(hep.style.CMS)  # or ATLAS/LHCb2

z_to_p = scipy.stats.norm.sf
p_to_z = scipy.stats.norm.cdf


N_BINS = 10
xlabel = r"$-log_{10}(\tilde{p})$"
xlabel_linear = r"$\tilde{p}$"
ylabel = "number of classes"


def plot_ptilde_cumulative(
    props: dict[str, ScanResults],
    n_rounds: int,
    output_dir: str,
    file_name: str,
    title: str,
    sub_title: str,
) -> None:
    ylabel = r"number of event classes with $\tilde{p} < \tilde{p}_{max}$"
    xlabel = r"$-log_{10}(\tilde{p}_{max})$"
    x_common = np.linspace(0, -np.log10(1.0 / n_rounds), 1000)  # Common x-axis

    temp_ptildes: list[list[float] | None] = []

    for ec in props:
        if not props[ec].skipped_scan:
            temp_ptildes.append(props[ec].p_tilde_toys())
    ptildes: NDArray[np.float64] = np.atleast_2d(-np.log10(np.array(temp_ptildes)))
    ptildes = np.atleast_2d(np.transpose(ptildes))

    comps = []
    for x in x_common:
        comps.append(np.sum(ptildes >= x, axis=1)[:, None])
    ptildes_ccdf = np.hstack(comps)

    temp_pdata: list[float | None] = []
    for ec in props:
        if not props[ec].skipped_scan:
            temp_pdata.append(props[ec].p_tilde())
    p_tilde_data: NDArray[np.float64] = np.sort(-np.log10(np.array(temp_pdata)))

    comps = []
    for x in p_tilde_data:
        comps.append(np.sum(p_tilde_data >= x))
    p_tilde_data_ccdf = np.hstack(comps)

    y_median = np.median(ptildes_ccdf, axis=0)
    lower_bound_outer = np.percentile(ptildes_ccdf, 2.5, axis=0)
    lower_bound = np.percentile(ptildes_ccdf, 16, axis=0)
    upper_bound = np.percentile(ptildes_ccdf, 84, axis=0)
    upper_bound_outer = np.percentile(ptildes_ccdf, 97.5, axis=0)

    fig, ax = plt.subplots()
    ax.set_yscale("log", nonpositive="clip")
    hep.cms.label(label="Work in progress", data=True, loc=0, ax=ax, lumi=138)

    ax.fill_between(
        x_common,
        lower_bound_outer,
        upper_bound_outer,
        color="#FFDF7F",
        label=r"$2\sigma$",
    )
    ax.fill_between(
        x_common,
        lower_bound,
        upper_bound,
        color="#85D1FB",
        label=r"$1\sigma$",
    )
    ax.plot(x_common, y_median, label="Median", color="red", linestyle="--")
    ax.plot(p_tilde_data, p_tilde_data_ccdf, "ko", markersize=3, label="Data")

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

    y_min, y_max = ax.get_ylim()
    ax.set_ylim(y_min, change_exponent(y_max, lambda x: x * 1.5))
    ax.set_xlim(0, (N_BINS + 1) * -np.log10(1.0 / n_rounds) / N_BINS)

    if sub_title != "":
        title = (
            title
            + r"""
"""
            + sub_title
        )

    ax.text(
        0.15,
        0.9,
        title,
        fontsize=20,
        horizontalalignment="left",
        verticalalignment="top",
        fontproperties="Tex Gyre Heros",
        transform=plt.gca().transAxes,
    )

    # print(f"Saving plots for {title} ...")
    fig.savefig("{}/{}_cumulative.pdf".format(output_dir, file_name))
    fig.savefig("{}/{}_cumulative.png".format(output_dir, file_name))
    fig.savefig("{}/{}_cumulative.svg".format(output_dir, file_name))
    plt.close()


def plot_ptilde(
    props: dict[str, ScanResults],
    n_rounds: int,
    output_dir: str,
    file_name: str,
    title: str,
    sub_title: str,
) -> None:
    bin_width: float = -np.log10(1.0 / n_rounds) / N_BINS

    bins: NDArray[np.float64] = np.linspace(0, (N_BINS + 1) * bin_width, N_BINS + 1)
    bin_centers: NDArray[np.float64] = 0.5 * (bins[:-1] + bins[1:])
    # bin_widths: NDArray[np.float64] = np.diff(bins)
    n_dist: int = len(props)

    list_of_histograms: list[NDArray[np.float64]] = []

    # print(f"Building p-tilde toys for {title} - {sub_title} ...")
    temp_ptildes: list[list[float] | None] = []

    for ec in props:
        if not props[ec].skipped_scan:
            temp_ptildes.append(props[ec].p_tilde_toys())
    ptildes: NDArray[np.float64] = np.atleast_2d(-np.log10(np.array(temp_ptildes)))
    ptildes = np.atleast_2d(np.transpose(ptildes))

    # print(f"Building p-tilde histograms for {title} - {sub_title} ...")
    for r in range(n_rounds):
        list_of_histograms.append((np.histogram(ptildes[r], bins)[0]))

    histograms: NDArray[np.float64] = np.array(list_of_histograms)

    # print(f"Building p-tilde data for {title} - {sub_title} ...")
    temp_pdata: list[float | None] = []
    for ec in props:
        if not props[ec].skipped_scan:
            temp_pdata.append(props[ec].p_tilde())
    p_tilde_data: NDArray[np.float64] = -np.log10(np.array(temp_pdata))

    # print(f"Plotting {title} - {sub_title}...")
    fig, ax = plt.subplots()
    ax.set_yscale("log", nonpositive="clip")
    hep.cms.label(label="Work in progress", data=True, loc=0, ax=ax, lumi=138)

    q_m2s = np.percentile(histograms, 2.5, axis=0, method="lower")
    q_m1s = np.percentile(histograms, 16, axis=0, method="lower")
    q_median = np.percentile(histograms, 50, axis=0, method="linear")
    q_p1s = np.percentile(histograms, 84, axis=0, method="higher")
    q_p2s = np.percentile(histograms, 97.5, axis=0, method="higher")

    def add_band(q_m, q_p, color, label):
        x = []
        y1 = []
        y2 = []
        for i in range(len(bins)):
            if i > 0:
                x.append(bins[i])
                y1.append(q_m[i - 1])
                y2.append(q_p[i - 1])

            if i < len(bins) - 1:
                x.append(bins[i])
                y1.append(q_m[i])
                y2.append(q_p[i])

        ax.fill_between(x, y1, y2, color=color, label=label)

    add_band(q_m2s, q_p2s, "#FFDF7F", r"SM expectation $\pm 2\sigma$")
    add_band(q_m1s, q_p1s, "#85D1FB", r"SM expectation $\pm 1\sigma$")

    ax.stairs(
        q_median,
        bins,
        color="black",
        linestyle="--",
        linewidth=2,
        label="Median SM expectation",
    )

    # yerr = np.abs(2 * np.random.normal(0, 1, N_BINS))
    # ax.errorbar(
    #     bin_centers,
    #     np.percentile(histograms, 50, axis=0) + yerr,
    #     xerr=bin_widths / 2,
    #     yerr=yerr,
    #     fmt="o",
    #     color="purple",
    #     ecolor="#7a21dd",
    #     elinewidth=2,
    #     capsize=0,
    #     capthick=2,
    #     linestyle="None",
    #     label="Signal",
    # )

    counts, _ = np.histogram(p_tilde_data, bins=bins)
    if np.sum(p_tilde_data > bins[-1]):
        print("WARNING: Has overflow!!!", file=sys.stderr)

    ax.stairs(
        (10 ** (-bins[:-1]) - 10 ** (-bins[1:])) * n_dist,
        bins,
        color="#bd1f01",
        linestyle="-",
        linewidth=2,
        label="Uniform distribution",
    )

    ax.plot(
        bin_centers,
        counts,
        "o",
        color="black",
        label="Observed deviations",
    )
    # ax.errorbar(
    #     bin_centers,
    #     np.histogram(
    #         p_tilde_data,
    #         bins=bins,
    #     )[0],
    #     xerr=bin_widths / 2,
    #     yerr=None,
    #     fmt="o",
    #     color="black",
    #     ecolor="black",
    #     elinewidth=2,
    #     capsize=0,
    #     capthick=2,
    #     linestyle="None",
    #     label="Observed deviations",
    # )

    # Calculate the lowest visible y-value
    # In log scale, the minimum visible value is the smallest positive number above the lower limit
    y_min, y_max = ax.get_ylim()
    y_min_visible = np.power(10, np.floor(np.log10(y_min)))
    if y_min_visible < y_min:
        y_min_visible = np.power(10, np.floor(np.log10(y_min)) + 1)

    y_max = ax.get_ylim()[1] * 10

    # for z in range( 1, int( -np.log10(1.0/n_rounds) ) ):
    #     log10p = -math.log10( z_to_p( z ) )
    #     ax.axvline(log10p, 0, 1, color='#717581', linestyle='--', linewidth=2, alpha=0.6)
    #     ax.text(log10p-bin_width/10, y_max*0.5, '${}\\sigma$'.format(z), color='#717581', fontsize=20, ha='right', alpha=0.6)

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    legend = ax.legend(loc="upper right", edgecolor="black")
    legend.get_frame().set_alpha(None)
    legend.get_frame().set_facecolor("white")
    ax.set_ylim(y_min_visible, y_max)

    ax.text(
        0.15,
        y_max * 0.65,
        title,
        fontsize=20,
        horizontalalignment="left",
        verticalalignment="top",
        fontproperties="Tex Gyre Heros",
    )
    if sub_title != "":
        ax.text(
            0.15,
            y_max * 0.45,
            sub_title,
            fontsize=20,
            horizontalalignment="left",
            verticalalignment="top",
            fontproperties="Tex Gyre Heros",
        )

    ax.set_xlim(0, (N_BINS + 1) * bin_width)

    # print(f"Saving plots for {title} - { sub_title } ...")
    fig.savefig("{}/{}.pdf".format(output_dir, file_name))
    fig.savefig("{}/{}.png".format(output_dir, file_name))
    fig.savefig("{}/{}.svg".format(output_dir, file_name))
    plt.close()

    with open("{}/{}.json".format(output_dir, file_name), "w") as f:
        json.dump(
            {
                ec: props[ec].dict()
                for ec in sorted(
                    props, key=lambda ec: props[ec].unsafe_p_tilde(), reverse=True
                )
            },
            f,
            indent=4,
        )


def control_plot(res: ScanResults, output_dir: str) -> None:
    if res.distribution == DistributionType.met and "MET" not in res.class_name:
        return None
    if (
        res.distribution == DistributionType.invariant_mass
        and ScanResults.count_objects(res.class_name) == 1
    ):
        return None

    fig, ax = plt.subplots()
    ax.set_yscale("log")
    # ax.set_xscale("log")

    N_BINS = 20
    _, _, _ = ax.hist(
        res.unsafe_p_tilde_toys(),
        bins=N_BINS,
        histtype="step",
        color="blue",
        linewidth=2,
        label=r"$\tilde{p}_{toys}$",
    )
    x = np.linspace(0, 1, 1000)
    y = (
        scipy.stats.uniform.pdf(x, 0.0, 1.0)
        * float(len(res.p_values_mc))
        / float(N_BINS)
    )
    ax.plot(
        x,
        y,
        color="orange",
        linestyle="--",
        linewidth=2,
        label="Uniform dist.",
    )

    ax.hist(
        res.p_values_mc,
        bins=50,
        histtype="step",
        color="black",
        linewidth=2,
        label=r"$p_{toys}$",
    )
    ax.axvline(
        res.unsafe_p_tilde(),
        color="red",
        linestyle="--",
        linewidth=2,
        label=r"$\tilde{p}_{data}$",
    )
    ax.axvline(
        res.p_value_data,
        color="purple",
        linestyle="--",
        linewidth=2,
        label=r"$p_{data}$",
    )

    ax.set_ylabel("number of toy scans")
    ax.set_xlabel(r"$p_{toys}$ or $\tilde{p}_{toys}$")
    ax.set_xlim(-0.1, 1.1)
    ax.legend(edgecolor="none")
    plt.title("{} - {}".format(res.class_name, res.distribution))

    fig.tight_layout()
    fig.savefig(
        "{}/control_plot_{}_{}.pdf".format(
            output_dir,
            res.class_name.replace("+", "_"),
            res.distribution,
        ),
        dpi=100,
        transparent=False,
    )

    plt.close()


def make_summary_plots(
    results: list[ScanResults],
    file_name: str,
    file_name_modifier: str,
    title: str,
    sub_title: str,
    n_rounds: int,
    output_dir: str,
) -> None:
    scan_results = {}
    for result in results:
        scan_results[result.class_name] = result

    if len(scan_results):
        plot_ptilde_cumulative(
            scan_results,
            n_rounds,
            output_dir,
            file_name + "_" + file_name_modifier,
            title,
            sub_title,
        )
        plot_ptilde(
            scan_results,
            n_rounds,
            output_dir,
            file_name + "_" + file_name_modifier,
            title,
            sub_title,
        )


def plot_summary(
    distribution: DistributionType,
    input_dir: str = "scan_results",
    output_dir: str = "scan_summary_plots",
    num_cpus: int = 128,
    class_type: ClassType = ClassType.All,
    expected_n_rounds: int = 10_000,
):
    os.system(f"mkdir -p {output_dir}")
    os.system(f"mkdir -p {output_dir}/control_plots")

    def make_args(ec: str) -> tuple[str, str, str, int]:
        return (
            "{}/{}/{}_{}_data_0_info.json".format(
                input_dir,
                ec.replace("+", "_"),
                ec.replace("+", "_"),
                distribution,
            ),
            "{}/{}/{}_{}_mc_*_info.json".format(
                input_dir,
                ec.replace("+", "_"),
                ec.replace("+", "_"),
                distribution,
            ),
            "{}/{}/{}_{}_Run2.json".format(
                input_dir,
                ec.replace("+", "_"),
                ec.replace("+", "_"),
                distribution,
            ),
            expected_n_rounds,
        )

    async_results: list[AsyncResult] = []
    results: list[ScanResults] = []
    with mp.Pool(num_cpus) as p:
        for ec in glob.glob(f"{input_dir}/*EC_*"):
            ec = ec.replace("scan_results/", "")
            if distribution == DistributionType.met and "MET" not in ec:
                continue
            if (
                distribution == DistributionType.invariant_mass
                and ScanResults.count_objects(ec) == 1
            ):
                continue

            if class_type != ClassType.All:
                if class_type == ClassType.Exclusive:
                    if "_X" not in ec and "_N" not in ec:
                        async_results.append(
                            p.apply_async(
                                ScanResults.make_scan_results,
                                make_args(ec),
                            )
                        )

                if class_type == ClassType.Inclusive:
                    if "_X" in ec:
                        async_results.append(
                            p.apply_async(
                                ScanResults.make_scan_results,
                                make_args(ec),
                            )
                        )

                if class_type == ClassType.JetInclusive:
                    if "_N" in ec:
                        async_results.append(
                            p.apply_async(
                                ScanResults.make_scan_results,
                                make_args(ec),
                            )
                        )
            else:
                async_results.append(
                    p.apply_async(ScanResults.make_scan_results, make_args(ec))
                )

        with Progress() as progress:
            task = progress.add_task(
                "Building list of Scan Results ({}) ...".format(distribution.value),
                total=len(async_results),
            )
            while len(async_results):
                for i, async_res in enumerate(async_results):
                    if async_res.ready():
                        results.append(async_res.get())
                        async_results.pop(i)
                        progress.advance(task)

    results = [
        res
        for res in results
        if not (res.skipped_scan)
        and res.dominant_process_group != "QCD"
        and res.dominant_process_group != "Gamma"
    ]

    n_rounds = 0
    if len(results):
        n_rounds = len(results[0].p_values_mc)
    for res in results:
        assert n_rounds == len(res.p_values_mc)
        assert not res.skipped_scan

    with mp.Pool(num_cpus) as p:
        with Progress() as progress:
            task = progress.add_task(
                "Control plots ...",
                total=len(results),
            )
            for res in p.imap_unordered(
                partial(control_plot, output_dir=f"{output_dir}/control_plots"),
                results,
            ):
                progress.advance(task)

    plots_results: list[AsyncResult] = []
    with mp.Pool(num_cpus) as p:
        for selection in class_selections:
            # Plotting exclusive classes
            if class_type == ClassType.All or class_type == ClassType.Exclusive:
                plots_results.append(
                    p.apply_async(
                        make_summary_plots,
                        (
                            list(
                                filter(
                                    lambda result: "+" not in result.class_name
                                    and class_selections[selection].filter_func(result),
                                    results,
                                )
                            ),
                            f"{distribution}_exclusive",
                            selection,
                            "Exclusive Classes: {}".format(distribution.latex_name()),
                            class_selections[selection].title,
                            n_rounds,
                            output_dir,
                        ),
                    )
                )

            # Plotting inclusive classes
            if class_type == ClassType.All or class_type == ClassType.Inclusive:
                plots_results.append(
                    p.apply_async(
                        make_summary_plots,
                        (
                            list(
                                filter(
                                    lambda result: "+X" in result.class_name
                                    and class_selections[selection].filter_func(result),
                                    results,
                                )
                            ),
                            f"{distribution}_inclusive",
                            selection,
                            "Inclusive Classes: {}".format(distribution.latex_name()),
                            class_selections[selection].title,
                            n_rounds,
                            output_dir,
                        ),
                    )
                )

            # Plotting jet inclusive classes
            if class_type == ClassType.All or class_type == ClassType.JetInclusive:
                plots_results.append(
                    p.apply_async(
                        make_summary_plots,
                        (
                            list(
                                filter(
                                    lambda result: "+NJet" in result.class_name
                                    and class_selections[selection].filter_func(result),
                                    results,
                                )
                            ),
                            f"{distribution}_jet_inclusive",
                            selection,
                            "Jet Inclusive Classes: {}".format(
                                distribution.latex_name()
                            ),
                            class_selections[selection].title,
                            n_rounds,
                            output_dir,
                        ),
                    )
                )

        with Progress() as progress:
            task = progress.add_task(
                "Summary plots ({} plots) ...".format(len(plots_results)),
                total=len(plots_results),
            )
            while len(plots_results):
                for i, async_res in enumerate(plots_results):
                    if async_res.ready():
                        _ = async_res.get()
                        plots_results.pop(i)
                        progress.advance(task)
