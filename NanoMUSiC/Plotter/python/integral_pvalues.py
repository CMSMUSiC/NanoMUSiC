import os
import sys
from enum import Enum
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


class PlotSize(float, Enum):
    Small = 0.5
    Medium = 1.0
    Large = 2.5


def plot_classes(
    event_classes: list[Any],
    year: str,
    ec_data_json: dict[str, Any],
    plot_size: PlotSize,
    title_modifier: tuple[str, str],
) -> None:
    set_of_labels = set()
    _, ax = plt.subplots(figsize=((16 * 1.8) * plot_size.value, (9 * 1.8 + 6)))

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

        for pg, _ in ordered_pg:
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
        plt.rcParams["hatch.color"] = "grey"
        p = ax.bar(
            get_ec_name(ec),
            2 * mc_uncert,
            bottom=(bottom - mc_uncert),
            alpha=0.6,
            color="None",
            hatch="//",
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
        bottom = (
            max(
                ec_data_json[year][ec]["data_count"] + data_uncert,
                bottom + mc_uncert,
                plt.gca().get_ylim()[0],
            )
            * 1.1
        )
        if p_value != None and p_value >= 0.1:
            plt.text(
                get_ec_name(ec),
                bottom,
                "$p_{int}$  = " + f"{p_value:.2f}",
                ha="center",
                usetex=True,
                va="bottom",
                rotation=90,
            )
        elif p_value != None and p_value < 0.1:
            plt.text(
                get_ec_name(ec),
                bottom,
                "$p_{int}$ = " + f"{p_value:.1e}",
                ha="center",
                usetex=True,
                va="bottom",
                rotation=90,
            )
        else:
            plt.text(
                get_ec_name(ec),
                bottom,
                "$p_{int}$  = " + str(p_value),
                ha="center",
                usetex=True,
                va="bottom",
                rotation=90,
            )

    legend_elements, labels = plt.gca().get_legend_handles_labels()
    labels.reverse()
    legend_elements.reverse()

    ax.legend(handles=legend_elements, labels=labels, loc="upper right", ncol=5)
    plt.xticks(rotation=90, va="top")
    plt.yscale("log")
    ylim_bottom, ylim_top = ax.get_ylim()
    plt.ylim(ylim_bottom, 15 * ylim_top)
    hep.cms.label("Preliminary", data=True, lumi=Lumi[year], year=year)
    ax.set_ylabel("Events")

    plt.text(
        0.02,
        0.98,
        f"{len(event_classes)} most {title_modifier[0]}: {title_modifier[1]}",
        transform=plt.gca().transAxes,
        fontsize=36,
        fontweight="bold",
        verticalalignment="top",
        horizontalalignment="left",
    )

    plt.tight_layout()


class IntegralPValuePlotType(str, Enum):
    MostOccupied = "occupied"
    MostDiscrepant = "discrepant"


def get_total_mc(counts):
    total = 0
    for process in counts["mc"]:
        total += counts["mc"][process]

    return total


def integral_pvalues_summary(
    input_file_path: str,
    output_dir: str,
    year: str = "Run2",
    num_classes: int = 20,
    plot_all: bool = True,
    plot_per_objects: bool = True,
    plot_exclusive: bool = True,
    plot_type: IntegralPValuePlotType = IntegralPValuePlotType.MostOccupied,
) -> None:
    plot_size = PlotSize.Medium
    if num_classes >= 100:
        plot_size = PlotSize.Large

    with open(input_file_path) as input_file:
        ec_data_json = json.load(input_file)

    mc_threshold = 0.1
    data_threshold = 1

    def select_most_occupied_class(event_class: str):
        return ec_data_json[year][event_class]["data_count"]

    def select_most_occupied_muon_class(event_class: str):
        if r"$\mu$" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        return -1

    def select_most_occupied_electron_class(event_class: str):
        if r"$e$" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        return -1

    def select_most_occupied_tau_class(event_class: str):
        if r"$\tau$" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        return -1

    def select_most_occupied_photon_class(event_class: str):
        if r"$\gamma$" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        return -1

    def select_most_occupied_exc_class(event_class: str):
        if "exc" in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        return -1

    def select_most_discrepant_class(event_class: str):
        if (
            ec_data_json[year][event_class]["data_count"] >= data_threshold
            and get_total_mc(ec_data_json[year][event_class]) >= mc_threshold
        ):
            return ec_data_json[year][event_class]["p_value"]
        return sys.float_info.max

    def select_most_discrepant_muon_class(event_class: str):
        if (
            r"$\mu$" in get_ec_name(event_class)
            and ec_data_json[year][event_class]["data_count"] >= data_threshold
            and get_total_mc(ec_data_json[year][event_class]) >= mc_threshold
        ):
            return ec_data_json[year][event_class]["p_value"]
        return sys.float_info.max

    def select_most_discrepant_electron_class(event_class: str):
        if (
            r"$e$" in get_ec_name(event_class)
            and ec_data_json[year][event_class]["data_count"] >= data_threshold
            and get_total_mc(ec_data_json[year][event_class]) >= mc_threshold
        ):
            return ec_data_json[year][event_class]["p_value"]
        return sys.float_info.max

    def select_most_discrepant_tau_class(event_class: str):
        if (
            r"$\tau$" in get_ec_name(event_class)
            and ec_data_json[year][event_class]["data_count"] >= data_threshold
            and get_total_mc(ec_data_json[year][event_class]) >= mc_threshold
        ):
            return ec_data_json[year][event_class]["p_value"]
        return sys.float_info.max

    def select_most_discrepant_photon_class(event_class: str):
        if (
            r"$\gamma$" in get_ec_name(event_class)
            and ec_data_json[year][event_class]["data_count"] >= data_threshold
            and get_total_mc(ec_data_json[year][event_class]) >= mc_threshold
        ):
            return ec_data_json[year][event_class]["p_value"]
        return sys.float_info.max

    def select_most_discrepant_exc_class(event_class: str):
        if (
            "exc" in get_ec_name(event_class)
            and ec_data_json[year][event_class]["data_count"] >= data_threshold
            and get_total_mc(ec_data_json[year][event_class]) >= mc_threshold
        ):
            return ec_data_json[year][event_class]["p_value"]
        return sys.float_info.max

    if plot_all:
        print(
            f"Processing {num_classes} most {plot_type.value} event classes for {year} ..."
        )

        is_reverse, class_selector = True, select_most_occupied_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_class

        selected_ec = sorted(
            ec_data_json[year].keys(), key=class_selector, reverse=is_reverse
        )

        plot_classes(
            (selected_ec)[0:num_classes],
            year,
            ec_data_json,
            plot_size,
            (
                plot_type.value,
                "all event classes",
            ),
        )
        plt.savefig("{}/pval_plot_{}_{}.png".format(output_dir, year, num_classes))
        plt.savefig("{}/pval_plot_{}_{}.pdf".format(output_dir, year, num_classes))
        plt.savefig("{}/pval_plot_{}_{}.svg".format(output_dir, year, num_classes))
        plt.close()

    if plot_per_objects:
        print(
            f"Processing {num_classes} most {plot_type.value} event classes with at least one muon for {year} ..."
        )

        is_reverse, class_selector = True, select_most_occupied_muon_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_muon_class

        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=class_selector,
            reverse=is_reverse,
        )
        plot_classes(
            (selected_ec)[0:num_classes],
            year,
            ec_data_json,
            plot_size,
            (plot_type.value, "with at least 1 muon"),
        )
        plt.savefig("{}/pval_plot_muon_{}_{}.png".format(output_dir, year, num_classes))
        plt.savefig("{}/pval_plot_muon_{}_{}.pdf".format(output_dir, year, num_classes))
        plt.savefig("{}/pval_plot_muon_{}_{}.svg".format(output_dir, year, num_classes))
        plt.close()

        print(
            f"Processing {num_classes} most {plot_type.value} event classes with at least one electron for {year} ..."
        )

        is_reverse, class_selector = True, select_most_occupied_electron_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_electron_class

        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=select_most_occupied_electron_class,
            reverse=is_reverse,
        )
        plot_classes(
            (selected_ec)[0:num_classes],
            year,
            ec_data_json,
            plot_size,
            (
                plot_type.value,
                "with at least 1 electron",
            ),
        )
        plt.savefig(
            "{}/pval_plot_electron_{}_{}.png".format(output_dir, year, num_classes)
        )
        plt.savefig(
            "{}/pval_plot_electron_{}_{}.pdf".format(output_dir, year, num_classes)
        )
        plt.savefig(
            "{}/pval_plot_electron_{}_{}.svg".format(output_dir, year, num_classes)
        )
        plt.close()

        print(
            f"Processing {num_classes} most {plot_type.value} event classes with at least one tau for {year} ..."
        )

        is_reverse, class_selector = True, select_most_occupied_tau_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_tau_class

        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=class_selector,
            reverse=is_reverse,
        )
        plot_classes(
            (selected_ec)[0:num_classes],
            year,
            ec_data_json,
            plot_size,
            (
                plot_type.value,
                "with at least 1 tau",
            ),
        )
        plt.savefig("{}/pval_plot_tau_{}_{}.png".format(output_dir, year, num_classes))
        plt.savefig("{}/pval_plot_tau_{}_{}.pdf".format(output_dir, year, num_classes))
        plt.savefig("{}/pval_plot_tau_{}_{}.svg".format(output_dir, year, num_classes))
        plt.close()

        print(
            f"Processing {num_classes} most {plot_type.value} event classes with at least one photon for {year} ..."
        )
        is_reverse, class_selector = True, select_most_occupied_photon_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_photon_class

        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=class_selector,
            reverse=is_reverse,
        )
        plot_classes(
            (selected_ec)[0:num_classes],
            year,
            ec_data_json,
            plot_size,
            (
                plot_type.value,
                "with at least 1 photon",
            ),
        )
        plt.savefig(
            "{}/pval_plot_photon_{}_{}.png".format(output_dir, year, num_classes)
        )
        plt.savefig(
            "{}/pval_plot_photon_{}_{}.pdf".format(output_dir, year, num_classes)
        )
        plt.savefig(
            "{}/pval_plot_photon_{}_{}.svg".format(output_dir, year, num_classes)
        )
        plt.close()

    if plot_exclusive:
        print(
            f"Processing {num_classes} most {plot_type.value} exclusive event classes for {year} ..."
        )
        is_reverse, class_selector = True, select_most_occupied_exc_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_exc_class

        selected_ec = sorted(
            ec_data_json[year].keys(),
            key=class_selector,
            reverse=is_reverse,
        )
        plot_classes(
            (selected_ec)[0:num_classes],
            year,
            ec_data_json,
            plot_size,
            (
                plot_type.value,
                "exclusive classes",
            ),
        )
        plt.savefig("{}/pval_plot_excl_{}_{}.png".format(output_dir, year, num_classes))
        plt.savefig("{}/pval_plot_excl_{}_{}.pdf".format(output_dir, year, num_classes))
        plt.savefig("{}/pval_plot_excl_{}_{}.svg".format(output_dir, year, num_classes))
        plt.close()

    print("Copying index.php ...")
    os.system(
        r"find ___OUTPUT_DIR___/ -type d -exec cp $MUSIC_BASE/NanoMUSiC/Plotter/assets/index.php {} \;".replace(
            "___OUTPUT_DIR___", output_dir
        )
    )
