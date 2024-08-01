import os
from typing import Any

import matplotlib.pyplot as plt
import mplhep as hep

hep.style.use("CMS")

import json
from rich.progress import track
from colors import PROCESS_GROUP_STYLES


Lumi = {"Run2": "138", "2016": "36.3", "2017": "41.5", "2018": "59.8"}


def get_ec_name(eventclass):
    ec = eventclass.split("_")
    ec_name = ""
    for p in ec:
        if "Muon" in p and p[0] != "0":
            ec_name += p[0] + r"$\mu$"
        if "Electron" in p and p[0] != "0":
            ec_name += "+" + p[0] + r"$e$"
        if "Tau" in p and p[0] != "0":
            ec_name += "+" + p[0] + r"$\tau$"
        if "Photon" in p and p[0] != "0":
            ec_name += "+" + p[0] + r"$\gamma$"
        if "bJet" in p and p[0] != "0":
            ec_name += "+" + p[0] + r"$bjet$"
        if "Jet" in p and p[0] != "0":
            ec_name += "+" + p[0] + r"$jet$"
        if "MET" in p and p[0] != "0":
            ec_name += "+" + p[0] + r"$met$"

    ec_name = ec_name.strip("+")

    if "+X" in eventclass:
        ec_name += " inc"
    elif "+NJet" in eventclass:
        ec_name += " njet"
    else:
        ec_name += " exc"

    return ec_name


def plot_classes(
    event_classes: list[Any], year: str, ec_data_json: dict[str, Any]
) -> None:
    set_of_labels = set()
    fig, ax = plt.subplots(figsize=(16 * 1.8, 9 * 1.8 + 6))
    # # process_groups = ["QCD", "Gamma","DiPhoton","GG","TTbar","TTbarV","Top","TTTT","TTG","TTGG","W","WG","WGStar","WGG","WWG","DrellYan",
    # #                     "tZQ","TZQ","ZG","DiBoson","TriBoson","Multi-Boson","WW","WZ","ZZ","ZToInvisible","TTW","TTWW","WZG","ZToQQ","TTZ",
    # #                     "TTZZ","TG","tG","WWZ","WZZ","TTbarTTbar","HIG","ZZZ","WWW"]

    ordered_pg = {}
    for ec in event_classes:
        for pg in ec_data_json[year][ec]["mc"]:
            if pg in ordered_pg:
                ordered_pg[pg] += ec_data_json[year][ec]["mc"][pg]
            else:
                ordered_pg[pg] = ec_data_json[year][ec]["mc"][pg]
    ordered_pg = sorted(ordered_pg.items(), key=lambda item: item[1], reverse=False)

    for ec in track(event_classes):
        bottom = 0

        for pg, d_count in ordered_pg:
            if pg not in ec_data_json[year][ec]["mc"]:
                ec_data_json[year][ec]["mc"][pg] = 0
            if not pg in set_of_labels:
                p = ax.bar(
                    get_ec_name(ec),
                    ec_data_json[year][ec]["mc"][pg],
                    label=pg,
                    bottom=bottom,
                    color=PROCESS_GROUP_STYLES[pg].colorhex,
                )
                set_of_labels.add(pg)
            else:
                p = ax.bar(
                    get_ec_name(ec),
                    ec_data_json[year][ec]["mc"][pg],
                    bottom=bottom,
                    color=PROCESS_GROUP_STYLES[pg].colorhex,
                )
            bottom += ec_data_json[year][ec]["mc"][pg]

        mc_uncert = ec_data_json[year][ec]["mc_uncert"]
        p_value = ec_data_json[year][ec]["p_value"]
        p = ax.bar(
            get_ec_name(ec),
            2 * mc_uncert,
            bottom=(bottom - mc_uncert),
            color="grey",
            alpha=0.01,
            hatch="//",
        )
        bottom = bottom + mc_uncert
        if p_value != None and p_value >= 0.1:
            plt.text(
                get_ec_name(ec),
                bottom * (1 + 0.05),
                f"p = {p_value:.2f}",
                ha="center",
                va="bottom",
                rotation=90,
            )
        elif p_value != None and p_value < 0.1:
            plt.text(
                get_ec_name(ec),
                bottom * (1 + 0.05),
                f"p = {p_value:.1e}",
                ha="center",
                va="bottom",
                rotation=90,
            )
        else:
            plt.text(
                get_ec_name(ec),
                bottom * (1 + 0.05),
                "p = " + str(p_value),
                ha="center",
                va="bottom",
                rotation=90,
            )
        data_uncert = ec_data_json[year][ec]["data_uncert"]
        if "Data" in set_of_labels:
            ax.errorbar(
                p[0].xy[0] + p[0].get_width() / 2,
                ec_data_json[year][ec]["data_count"],
                xerr=0.0,
                yerr=data_uncert,
                fmt="o",
                color="k",
            )
        else:
            ax.errorbar(
                p[0].xy[0] + p[0].get_width() / 2,
                ec_data_json[year][ec]["data_count"],
                xerr=0.0,
                yerr=data_uncert,
                fmt="o",
                color="k",
                label="Data",
            )
            set_of_labels.add("Data")

    legend_elements, labels = plt.gca().get_legend_handles_labels()
    labels.reverse()
    legend_elements.reverse()

    ax.legend(handles=legend_elements, labels=labels, loc="upper right", ncol=5)
    plt.xticks(rotation=90, va="top")
    plt.yscale("log")
    ylim_bottom, ylim_top = ax.get_ylim()
    plt.ylim(ylim_bottom, 3 * ylim_top)
    hep.cms.label("Work in progress", data=True, lumi=Lumi[year], year=year)
    ax.set_ylabel("Events per class")
    plt.tight_layout()


def most_occupied_classes(
    input_file_path: str,
    output_dir: str,
    year: str = "Run2",
    num_classes: int = 20,
    most_occupied: bool = True,
    most_occupied_objects: bool = True,
    most_occupied_exclusive: bool = True,
) -> None:
    print("\n\n[ MUSiC p-value - Plotter ]\n")

    with open(input_file_path) as input_file:
        ec_data_json = json.load(input_file)

    os.system("rm -rf {}".format(output_dir))
    os.system("mkdir -p {}".format(output_dir))

    def select_most_occupied_class(event_class: str):
        return ec_data_json[year][event_class]["data_count"]

    def select_most_occupied_muon_class(event_class: str):
        if r"$\mu$" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        else:
            return -1

    def select_most_occupied_electron_class(event_class: str):
        if r"$e$" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        else:
            return -1

    def select_most_occupied_tau_class(event_class: str):
        if r"$\tau$" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        else:
            return -1

    def select_most_occupied_photon_class(event_class: str):
        if r"$\gamma$" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        else:
            return -1

    def select_most_occupied_exc_class(event_class: str):
        if "exc" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        else:
            return -1

    if most_occupied:
        print(f"Processing {num_classes} most occupied event classes for {year} ...")

        selected_ec = sorted(
            ec_data_json[year].keys(), key=select_most_occupied_class, reverse=True
        )
        plot_classes((selected_ec)[0:num_classes], year, ec_data_json)
        plt.savefig("{}/pval_plot.png".format(output_dir))
        plt.savefig("{}/pval_plot.pdf".format(output_dir))
        plt.savefig("{}/pval_plot.svg".format(output_dir))

    if most_occupied_objects:
        print(
            f"Processing {num_classes} most occupied event classes with at least one muon for {year} ..."
        )

        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=select_most_occupied_muon_class,
            reverse=True,
        )
        plot_classes((selected_ec)[0:num_classes], year, ec_data_json)
        plt.savefig("{}/pval_plot_muon.png".format(output_dir))
        plt.savefig("{}/pval_plot_muon.pdf".format(output_dir))
        plt.savefig("{}/pval_plot_muon.svg".format(output_dir))

        print(
            f"Processing {num_classes} most occupied event classes with at least one electron for {year} ..."
        )
        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=select_most_occupied_electron_class,
            reverse=True,
        )
        plot_classes((selected_ec)[0:num_classes], year, ec_data_json)
        plt.savefig("{}/pval_plot_electron.png".format(output_dir))
        plt.savefig("{}/pval_plot_electron.pdf".format(output_dir))
        plt.savefig("{}/pval_plot_electron.svg".format(output_dir))

        print(
            f"Processing {num_classes} most occupied event classes with at least one tau for {year} ..."
        )
        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=select_most_occupied_tau_class,
            reverse=True,
        )
        plot_classes((selected_ec)[0:num_classes], year, ec_data_json)
        plt.savefig("{}/pval_plot_tau.png".format(output_dir))
        plt.savefig("{}/pval_plot_tau.pdf".format(output_dir))
        plt.savefig("{}/pval_plot_tau.svg".format(output_dir))

        print(
            f"Processing {num_classes} most occupied event classes with at least one photon for {year} ..."
        )
        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=select_most_occupied_photon_class,
            reverse=True,
        )
        plot_classes((selected_ec)[0:num_classes], year, ec_data_json)
        plt.savefig("{}/pval_plot_photon.png".format(output_dir))
        plt.savefig("{}/pval_plot_photon.pdf".format(output_dir))
        plt.savefig("{}/pval_plot_photon.svg".format(output_dir))

    if most_occupied_exclusive:
        print(
            f"Processing {num_classes} most occupied exclusive event classes for {year} ..."
        )
        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=select_most_occupied_exc_class,
            reverse=True,
        )
        plot_classes((selected_ec)[0:num_classes], year, ec_data_json)
        plt.savefig("{}/pval_plot_excl.png".format(output_dir))
        plt.savefig("{}/pval_plot_excl.pdf".format(output_dir))
        plt.savefig("{}/pval_plot_excl.svg".format(output_dir))

    print("Copying index.php ...")
    os.system(
        r"find ___OUTPUT_DIR___/ -type d -exec cp $MUSIC_BASE/NanoMUSiC/Plotter/assets/index.php {} \;".replace(
            "___OUTPUT_DIR___", output_dir
        )
    )
