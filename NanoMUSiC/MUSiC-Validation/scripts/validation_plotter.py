#!/usr/bin/env python3
import os
import uproot
import hist
import numpy as np
import matplotlib as mpl

mpl.use("Agg")
import matplotlib.pyplot as plt
import mplhep as hep

hep.style.use("CMS")

from binning import base_binning, rebin_hist

music_base = os.environ["MUSIC_BASE"]


class Plotter:
    def __init__(
        self,
        outputs_reference: str,
        input_mc_files: dict[str, dict],
        input_data_files: str,
        outputs_dir: str,
    ) -> None:
        self.outputs_reference = outputs_reference
        self.input_mc_files = input_mc_files
        self.input_data_files = input_data_files
        self.outputs_dir = outputs_dir

        # prepare outputs area
        os.system(f"rm -rf {self.outputs_dir}/{self.outputs_reference} > /dev/null")
        os.system(f"mkdir -p {self.outputs_dir}/{self.outputs_reference}")

    def print_canvas(self, fig, histogram_name):
        fig.savefig(f"{self.outputs_dir}/{self.outputs_reference}/{histogram_name}.png")
        fig.savefig(f"{self.outputs_dir}/{self.outputs_reference}/{histogram_name}.pdf")

    def plot(
        self,
        histogram_name: str,
        x_axis_label,
        run_lumi_tag,
        lumi,
        rebin_callable,
        change_last_bin,
    ):
        # get MC histograms
        histos_mc = {}
        for sample in self.input_mc_files:
            histos_mc[sample] = uproot.open(
                f"{self.input_mc_files[sample]}:{histogram_name}"
            ).to_hist()

        # get Data histogram
        histo_data = uproot.open(f"{self.input_data_files}:{histogram_name}").to_hist()

        # setup figure
        fig = plt.figure()
        grid = fig.add_gridspec(2, 1, hspace=0, height_ratios=[3, 1])
        ax1 = fig.add_subplot(grid[0])
        ax2 = fig.add_subplot(grid[1], sharex=ax1)
        plt.setp(ax1.get_xticklabels(), visible=False)

        # rebin
        histo_data, histos_mc = rebin_callable(histo_data, histos_mc, change_last_bin)

        total_mc_histo = histos_mc[(list(histos_mc.keys()))[0]].copy(deep=True)
        for sample in histos_mc:
            if sample != list(histos_mc.keys())[0]:
                total_mc_histo += histos_mc[sample]

        # plot ratio
        histo_data.plot_ratio(total_mc_histo, ax_dict={"main_ax": ax1, "ratio_ax": ax2})

        # the top histogram has to be cleared, in order to print per sample plot
        ax1.clear()

        # print CMS labels
        hep.cms.label(run_lumi_tag, data=True, lumi=lumi, ax=ax1)

        # plot distributions on top subplot
        _histos_mc = []
        _histos_mc_samples = []
        for sample in histos_mc:
            _histos_mc.append(histos_mc[sample])
            _histos_mc_samples.append(sample)
        hep.histplot(
            _histos_mc,
            yerr=False,
            stack=True,
            label=_histos_mc_samples,
            ax=ax1,
            histtype="fill",
            linewidth=0,
        )

        # plot MC uncertanties
        # TODO: make variations assymmetric. One could have two ax.bar (up and down)
        mc_errors = np.sqrt(total_mc_histo.variances()) * 2.0
        ax1.bar(
            # x
            total_mc_histo.axes[0].centers,
            # height
            mc_errors,
            # width
            total_mc_histo.axes[0].edges[1:] - total_mc_histo.axes[0].edges[:-1],
            # bottom
            total_mc_histo.values() - mc_errors / 2.0,
            # kwargs
            fill=True,
            linewidth=0,
            edgecolor="gray",
            color="gray",
            alpha=0.4,
            hatch="///",
            # label="Stats. and Syst. Uncert.",
            label="Stats. Uncert.",
        )

        # plot ratio error bars
        up_mc_errors = np.zeros_like(total_mc_histo.values())
        dw_mc_errors = np.zeros_like(total_mc_histo.values())
        _mc_errors = mc_errors / 2.0
        with np.errstate(all="ignore"):
            up_mc_errors = (
                total_mc_histo.values() + _mc_errors
            ) / total_mc_histo.values()
            dw_mc_errors = (
                total_mc_histo.values() - _mc_errors
            ) / total_mc_histo.values()

        # Set 0 and inf to nan to hide during plotting
        # up_mc_errors[up_mc_errors == 0] = np.nan
        up_mc_errors[np.isinf(up_mc_errors)] = np.nan
        # down_mc_errors[down_mc_errors == 0] = np.nan
        dw_mc_errors[np.isinf(dw_mc_errors)] = np.nan
        # print(up_mc_errors, dw_mc_errors)

        up_mc_errors = np.nan_to_num(up_mc_errors)
        dw_mc_errors = np.nan_to_num(dw_mc_errors)

        ax2.bar(
            # x
            total_mc_histo.axes[0].centers,
            # height
            (up_mc_errors - dw_mc_errors),
            # width
            total_mc_histo.axes[0].edges[1:] - total_mc_histo.axes[0].edges[:-1],
            # bottom
            dw_mc_errors,
            # kwargs
            fill=True,
            linewidth=0,
            edgecolor="gray",
            color="gray",
            alpha=0.3,
        )

        # plot data
        hep.histplot(
            histo_data,
            yerr=True,
            stack=False,
            label="Data",
            ax=ax1,
            histtype="errorbar",
            color="black",
            marker=".",
            markersize=10.0,
            elinewidth=1,
        )

        ax1.set_yscale("log")
        plt.setp(ax1.get_yticklabels()[0], visible=False)

        # legend
        ax1.legend(loc="upper right")

        y_min, y_max = ax1.get_ylim()
        minval_data = np.min(histo_data.values()[histo_data.values() > 0]) * 0.7
        minval_mc = np.min(total_mc_histo.values()[total_mc_histo.values() > 0]) * 0.7
        if ((y_max - y_min) / y_min) < 1000.0:
            ax1.set_ylim(y_max / 1000, y_max * 100)
        else:
            ax1.set_ylim(min(minval_data, minval_mc), y_max)

        # set axes names
        ax1.set_ylabel("Events")
        # ax2.set_ylabel(r"$\frac{Data}{Simulation}$", verticalalignment="center")
        ax2.set_ylabel(r"Data/MC", loc="center")
        ax2.set_xlabel(x_axis_label, loc="right")

        fig.tight_layout()

        self.print_canvas(fig, histogram_name)


def no_rebinning(histo_data, histo_mc, change_last_bin=False):
    return histo_data, histo_mc


def rebin_energy_like(histo_data, histos_mc, change_last_bin=False):
    """
    Will rebin energy-like histograms. Last bin is reduce to encopass data and a more coarse binning is applied.
    """
    new_binning = base_binning
    if change_last_bin:
        last_data_point = -1
        for idx_bin in range(1300 - 1, 0, -1):
            if histo_data[idx_bin].value != 0:
                last_data_point = histo_data.axes[0].centers[idx_bin]
                break

        for idx, _ in enumerate(base_binning[:-1]):
            if (
                base_binning[idx] < last_data_point
                and base_binning[idx + 1] > last_data_point
            ):
                new_binning = base_binning[: idx + 2]
                break

    rebinned_data = rebin_hist(histo_data, new_binning)
    rebinned_mc = {}
    for h in histos_mc:
        rebinned_mc[h] = rebin_hist(histos_mc[h], new_binning)
    return rebinned_data, rebinned_mc


def leplep_plots(latex_name, name, input_mc, input_data):
    lumi = 58.83
    z_LepLep_X = Plotter(
        name,
        input_mc,
        input_data,
        "validation_plots",
    )

    z_LepLep_X.plot(
        "h_invariant_mass",
        "$M_{inv}$",
        "Work in progress",
        lumi,
        rebin_energy_like,
        True,
    )
    z_LepLep_X.plot(
        "h_sum_pt",
        "$\Sigma p_{T}$",
        "Work in progress",
        lumi,
        rebin_energy_like,
        True,
    )
    z_LepLep_X.plot("h_met", "MET", "Work in progress", lumi, rebin_energy_like, True)
    z_LepLep_X.plot(
        "h_lepton_1_pt",
        f"$p_{{T}}^{{lead-{latex_name}}}$",
        "Work in progress",
        lumi,
        rebin_energy_like,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_2_pt",
        f"$p_{{T}}^{{sublead-{latex_name}}}$",
        "Work in progress",
        lumi,
        rebin_energy_like,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_1_eta",
        f"$\eta^{{lead-{latex_name}}}$",
        "Work in progress",
        lumi,
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_2_eta",
        f"$\eta^{{sublead-{latex_name}}}$",
        "Work in progress",
        lumi,
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_1_phi",
        f"$\phi^{{lead-{latex_name}}}$",
        "Work in progress",
        lumi,
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_2_phi",
        f"$\phi^{{sublead-{latex_name}}}$",
        "Work in progress",
        lumi,
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_1_jet_1_dPhi",
        f"$\Delta \phi({latex_name}, jet_{{lead}})$",
        "Work in progress",
        lumi,
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_1_jet_1_dR",
        f"$\Delta R({latex_name}, jet_{{lead}})$",
        "Work in progress",
        lumi,
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_jet_multiplicity",
        "N Jets",
        "Work in progress",
        lumi,
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_bjet_multiplicity",
        "N BJets",
        "Work in progress",
        lumi,
        no_rebinning,
        True,
    )


def main():
    leplep_plots(
        "\mu",
        "z_MuMu_X",
        {
            "DYJetsToLL_M-50_13TeV": "validation_outputs_BKP/DYJetsToLL_M-50_13TeV_AM_z_to_mu_mu_x.root"
        },
        "validation_outputs_BKP/SingleMuon_z_to_mu_mu_x.root",
    )


if __name__ == "__main__":
    main()
