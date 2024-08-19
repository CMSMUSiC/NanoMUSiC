import matplotlib.pyplot as plt  # matplotlib library
import matplotlib as mpl
import mplhep as hep  # HEP (CMS) extensions/styling on top of mpl
from scan import ScanResults
import scipy
import numpy as np
from numpy.typing import NDArray

mpl.use("Agg")

# Load style sheet
plt.style.use(hep.style.CMS)  # or ATLAS/LHCb2

z_to_p = scipy.stats.norm.sf
p_to_z = scipy.stats.norm.cdf


N_BINS = 9
xlabel = r"$-log_{10}(\tilde{p})$"
ylabel = "number of classes"


def plot_ptilde(
    props: dict[str, ScanResults],
    n_rounds: int,
    output_dir: str,
    file_name: str,
    title: str,
) -> None:
    bin_width: float = -np.log10(1.0 / n_rounds) / N_BINS
    bins: NDArray[np.float64] = np.linspace(0, (N_BINS + 1) * bin_width, N_BINS + 1)
    bin_centers: NDArray[np.float64] = 0.5 * (bins[:-1] + bins[1:])
    bin_widths: NDArray[np.float64] = np.diff(bins)
    n_dist: int = len(props)

    temp_ptildes: list[list[float]] = []
    for ec in props:
        temp_ptildes.append(props[ec].p_toys)
    ptildes: NDArray[np.float64] = -np.log10(np.array(temp_ptildes))

    temp_pdata: list[float] = []
    for ec in props:
        temp_pdata.append(props[ec].p_data)
    p_data: NDArray[np.float64] = -np.log10(np.array(temp_pdata))

    list_of_histograms: list[NDArray[np.float64]] = []
    for r in range(n_rounds):
        list_of_histograms.append((np.histogram(ptildes[r], bins)[0]))

    histograms: NDArray[np.float64] = np.array(list_of_histograms)

    fig, ax = plt.subplots()
    ax.set_yscale("log", nonpositive="clip")
    hep.cms.label(label="Preliminary", data=True, loc=0, ax=ax, lumi=138)

    q_m2s = np.percentile(histograms, 2.5, axis=0)
    q_m1s = np.percentile(histograms, 16, axis=0)
    q_median = np.percentile(histograms, 50, axis=0)
    q_p1s = np.percentile(histograms, 84, axis=0)
    q_p2s = np.percentile(histograms, 97.5, axis=0)

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
        (10 ** (-bins[:-1]) - 10 ** (-bins[1:])) * n_dist,
        bins,
        color="#bd1f01",
        linestyle="-",
        linewidth=2,
        label="Uniform distribution",
    )

    ax.stairs(
        q_median,
        bins,
        color="black",
        linestyle="--",
        linewidth=2,
        label="Median SM expectation",
    )

    yerr = np.abs(2 * np.random.normal(0, 1, N_BINS))
    ax.errorbar(
        bin_centers,
        np.percentile(histograms, 50, axis=0) + yerr,
        xerr=bin_widths / 2,
        yerr=yerr,
        fmt="o",
        color="purple",
        ecolor="#7a21dd",
        elinewidth=2,
        capsize=0,
        capthick=2,
        linestyle="None",
        label="Signal",
    )

    ax.plot(
        bin_centers,
        np.histogram(
            p_data,
            bins=bins,
        )[0],
        "o",
        color="black",
        label="Observed deviations",
    )
    # ax.errorbar(
    #     bin_centers,
    #     np.histogram(
    #         p_data,
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
        fontsize=24,
        horizontalalignment="left",
        verticalalignment="top",
        fontproperties="Tex Gyre Heros",
    )

    ax.set_xlim(0, (N_BINS + 1) * bin_width)

    fig.savefig("{}/{}.pdf".format(output_dir, file_name))
    fig.savefig("{}/{}.png".format(output_dir, file_name))
    fig.savefig("{}/{}.svg".format(output_dir, file_name))
