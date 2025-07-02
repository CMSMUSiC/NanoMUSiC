import math
import os
import sys
from enum import Enum
from typing import Any

import matplotlib.pyplot as plt
import mplhep as hep

hep.style.use("CMS")
import matplotlib as mpl

mpl.use("Agg")

# Load style sheet
plt.style.use(hep.style.CMS)  # or ATLAS/LHCb2

import json

from colors import PROCESS_GROUP_STYLES
from rich.progress import track
from tools import change_exponent

Lumi = {"Run2": "138", "2016": "36.3", "2017": "41.5", "2018": "59.8"}


def get_ec_name(eventclass: Any) -> str:
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
            ec_name += "+" + p[0] + r"$b-jet$"
        if "Jet" in p and "bJet" not in p and "NJet" not in p and p[0] != "0":
            ec_name += "+" + p[0] + r"$jet$"
        if "MET" in p and p[0] != "0":
            ec_name += "+" + r"$MET$"

    ec_name = ec_name.strip("+")

    if "+X" in eventclass:
        ec_name += " + X"
    elif "+NJet" in eventclass:
        ec_name += " + NJet"
    else:
        ec_name += " exc."

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

    fig, ax = plt.subplots(figsize=((16 * 2) * plot_size.value, (9 * 1.8 + 6)))

    ordered_pg = {}
    for ec in event_classes:
        for pg in ec_data_json[year][ec]["mc"]:
            if pg in ordered_pg:
                ordered_pg[pg] += ec_data_json[year][ec]["mc"][pg]
            else:
                ordered_pg[pg] = ec_data_json[year][ec]["mc"][pg]
    ordered_pg = sorted(ordered_pg.items(), key=lambda item: item[1], reverse=False)

    p_val_texts = []
    for ec in event_classes:
        bottom = 0

        for pg, _ in ordered_pg:
            if pg not in ec_data_json[year][ec]["mc"]:
                ec_data_json[year][ec]["mc"][pg] = 0
            if not pg in set_of_labels:
                p = ax.bar(
                    get_ec_name(ec),
                    ec_data_json[year][ec]["mc"][pg],
                    label=PROCESS_GROUP_STYLES[pg].label,
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
            edgecolor="black",
            facecolor="None",
            linewidth=0,
        )
        data_uncert = ec_data_json[year][ec]["data_uncert"]
        if "Data" in set_of_labels:
            ax.errorbar(
                p[0].xy[0] + p[0].get_width() / 2,
                ec_data_json[year][ec]["data_count"],
                xerr=p[0].get_width() / 2,
                yerr=data_uncert,
                fmt="o",
                color="k",
            )
        else:
            ax.errorbar(
                p[0].xy[0] + p[0].get_width() / 2,
                ec_data_json[year][ec]["data_count"],
                xerr=p[0].get_width() / 2,
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

        p_val_text = None
        if p_value != None and p_value >= 0.01:
            p_val_text = plt.text(
                get_ec_name(ec),
                bottom * 1.1,
                "p = " + f"{p_value:.2f}",
                ha="center",
                usetex=True,
                va="bottom",
                rotation=90,
                fontsize=40,
            )
        elif p_value != None and p_value < 0.01:
            p_val_text = plt.text(
                get_ec_name(ec),
                bottom * 1.1,
                "p = " + f"{p_value:.1e}",
                ha="center",
                usetex=True,
                va="bottom",
                rotation=90,
                fontsize=40,
            )
        else:
            p_val_text = plt.text(
                get_ec_name(ec),
                bottom * 1.1,
                "p = " + str(p_value),
                ha="center",
                usetex=True,
                va="bottom",
                rotation=90,
                fontsize=40,
            )

        p_val_texts.append(p_val_text)

    # Draw to compute actual height
    fig.canvas.draw()
    p_val_max_y = 0
    p_val_min_y = float("+inf")
    for text in p_val_texts:
        p_val_max_y = max(
            p_val_max_y, text.get_window_extent().transformed(ax.transAxes).y1
        )
        p_val_min_y = min(
            p_val_min_y, text.get_window_extent().transformed(ax.transAxes).y0
        )

    legend_elements, labels = plt.gca().get_legend_handles_labels()
    labels.reverse()
    legend_elements.reverse()

    _ = ax.legend(
        handles=legend_elements,
        labels=labels,
        loc="upper left",
        ncol=2,
        # fontsize="x-large",
        bbox_to_anchor=(1.01, 1.0),  # x = just outside (1.05), y = top (1.0)
        borderaxespad=0.0,
    )
    plt.xticks(rotation=90, va="top", usetex=False)
    plt.yscale("log")
    ylim_bottom, ylim_top = ax.get_ylim()
    plt.ylim(ylim_bottom, change_exponent(ylim_top, lambda x: x * 1.4))

    description = f"{len(event_classes)} most {title_modifier[0]}"
    if title_modifier[1] != "":
        description += f": {title_modifier[1]}"

    _ = hep.cms.label(
        f"Work in progress - {description}",
        loc=3,
        fontsize=50,
        data=True,
        lumi=Lumi[year],
        year=year,
    )

    ax.set_ylabel("Events per class", fontsize=50)

    plt.tick_params(axis="both", labelsize=50, pad=10)

    ax.set_ylim(min(ax.get_ylim()[0], p_val_min_y * 1.1), p_val_max_y * 10**3.5)

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
) -> tuple[str, str, int]:
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

    def select_most_occupied_no_photon_class(event_class: str):
        if r"$\gamma$" not in get_ec_name(event_class):
            return ec_data_json[year][event_class]["data_count"]
        return -1

    def select_most_occupied_no_photon_exc_class(event_class: str):
        if "exc" in get_ec_name(event_class) and r"$\gamma$" not in get_ec_name(
            event_class
        ):
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

    def select_most_discrepant_no_photon_class(event_class: str):
        if (
            r"$\gamma$" not in get_ec_name(event_class)
            and ec_data_json[year][event_class]["data_count"] >= data_threshold
            and get_total_mc(ec_data_json[year][event_class]) >= mc_threshold
        ):
            return ec_data_json[year][event_class]["p_value"]
        return sys.float_info.max

    def select_most_discrepant_no_photon_exc_class(event_class: str):
        if (
            "exc" in get_ec_name(event_class)
            and r"$\gamma$" not in get_ec_name(event_class)
            and ec_data_json[year][event_class]["data_count"] >= data_threshold
            and get_total_mc(ec_data_json[year][event_class]) >= mc_threshold
        ):
            return ec_data_json[year][event_class]["p_value"]
        return sys.float_info.max

    def custom_plot(class_selector, is_reverse, description, file_name_modifier):
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
            (plot_type.value, description),
        )
        plt.savefig(
            f"{output_dir}/pval_plot_{file_name_modifier}_{year}_{num_classes}.pdf"
        )
        plt.close()

    if plot_all:
        is_reverse, class_selector = True, select_most_occupied_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_class
        custom_plot(class_selector, is_reverse, "", "all")

    if plot_per_objects:
        # muon
        is_reverse, class_selector = True, select_most_occupied_muon_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_muon_class
        custom_plot(class_selector, is_reverse, "at least 1 muon", "muon")

        # electron
        is_reverse, class_selector = True, select_most_occupied_electron_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_electron_class
        custom_plot(class_selector, is_reverse, "at least 1 electron", "electron")

        # tau
        is_reverse, class_selector = True, select_most_occupied_tau_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_tau_class
        custom_plot(class_selector, is_reverse, "at least 1 tau", "tau")

        # photon
        is_reverse, class_selector = True, select_most_occupied_photon_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_photon_class
        custom_plot(class_selector, is_reverse, "at least 1 photon", "photon")

        # no photon
        is_reverse, class_selector = True, select_most_occupied_no_photon_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_no_photon_class

        custom_plot(class_selector, is_reverse, "no photons", "no_photon")
    if plot_exclusive:
        is_reverse, class_selector = True, select_most_occupied_exc_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = False, select_most_discrepant_exc_class
        custom_plot(class_selector, is_reverse, "exc. classes", "exc")

        # no photon - exc
        is_reverse, class_selector = True, select_most_occupied_no_photon_exc_class
        if plot_type == IntegralPValuePlotType.MostDiscrepant:
            is_reverse, class_selector = (
                False,
                select_most_discrepant_no_photon_exc_class,
            )
        custom_plot(
            class_selector, is_reverse, "no photons - exc. classes", "no_photon_exc"
        )

    match plot_type:
        case IntegralPValuePlotType.MostOccupied:
            return "most occupied", year, num_classes
        case IntegralPValuePlotType.MostDiscrepant:
            return "most discrepant", year, num_classes
        case _:
            raise ValueError("Invalid Integral PValue Plot Type")
