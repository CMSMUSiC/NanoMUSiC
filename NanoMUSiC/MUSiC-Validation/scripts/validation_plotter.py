#!/usr/bin/env python3
import os
import glob
import argparse
import toml
from pprint import pprint


from plotter import Plotter
from binning import *


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


def leplep_plots(
    latex_name: str,
    outputs_reference: str,
    input_mc: dict[str, str],
    input_data: list[str],
):
    lumi = 58.83
    z_LepLep_X = Plotter(
        outputs_reference,
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


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--config",
        required=True,
        help='Task configuration (TOML) file, produced by "analysis_config_builder.py"',
    )

    args = parser.parse_args()

    if not args.config or not os.path.exists(args.config):
        raise RuntimeError(
            f"ERROR: Could parse arguments. Config (-c/--config) path ({args.config}) is invalid."
        )

    return args


# Globals (sorry, me from the future ...)
years = ["2016APV", "2016", "2017", "2018"]
analyses_names = [
    "z_to_mu_mu_x",
    "z_to_mu_mu_x_Z_mass",
    "z_to_ele_ele_x",
    "z_to_ele_ele_x_Z_mass",
]


def make_plotter_args(
    analysis_config: dict,
    latex_name: str,
    analysis_name: str,
):
    plotter_args = {
        "latex_name": latex_name,
        "outputs_reference": analysis_name,
        "input_mc": {},
        "input_data": [],
    }

    plotter_args["input_data"] = [
        f
        for f in glob.glob(f"validation_outputs/*/{analysis_name}_Data*.root")
        if not ("cutflow_" in f)
    ]

    for sample in analysis_config:
        if (
            sample != "Lumi"
            and sample != "Global"
            and not (analysis_config[sample]["is_data"])
        ):
            for year in years:
                if f"das_name_{year}" in analysis_config[sample].keys():
                    if (
                        analysis_config[sample]["ProcessGroup"]
                        in plotter_args["input_mc"]
                    ):
                        plotter_args["input_mc"][
                            analysis_config[sample]["ProcessGroup"]
                        ].append(
                            f"validation_outputs/{year}/{analysis_name}_{sample}_{year}.root"
                        )
                    else:
                        plotter_args["input_mc"][
                            analysis_config[sample]["ProcessGroup"]
                        ] = [
                            f"validation_outputs/{year}/{analysis_name}_{sample}_{year}.root"
                        ]

    return plotter_args


def main():
    args = parse_args()

    # load analysi_config
    analysis_config: dict = toml.load(args.config)
    leplep_plots(
        **make_plotter_args(
            analysis_config, latex_name="\mu", analysis_name="z_to_mu_mu_x"
        )
    )

    # leplep_plots(
    #     "\mu",
    #     "z_MuMu_X",
    #     {
    #         "DYJetsToLL_M-50_13TeV": "validation_outputs_BKP/DYJetsToLL_M-50_13TeV_AM_z_to_mu_mu_x.root"
    #     },
    #     ["validation_outputs_BKP/SingleMuon_z_to_mu_mu_x.root"],
    # )


if __name__ == "__main__":
    main()
