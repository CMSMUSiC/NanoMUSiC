from scan_results import ScanResults
from typing import Callable


class ClassSelection:
    def __init__(self, title: str, filter_func: Callable[[ScanResults], bool]):
        self.title = title
        self.filter_func = filter_func


class_selections: dict[str, ClassSelection] = {
    "no_selection": ClassSelection("", lambda _: True),
}

#### --------------------- at least one lepton


def at_least_1_muon_no_leptons(result: ScanResults) -> bool:
    if (
        "Muon" in result.class_name
        and "Electron" not in result.class_name
        and "Tau" not in result.class_name
    ):
        return True
    return False


class_selections["at_least_1_muon_no_other_leptons"] = ClassSelection(
    r"$N_{\mu} \geq 1$ - no other leptons",
    at_least_1_muon_no_leptons,
)


def at_least_1_electron_no_leptons(result: ScanResults) -> bool:
    if (
        "Muon" not in result.class_name
        and "Electron" in result.class_name
        and "Tau" not in result.class_name
    ):
        return True
    return False


class_selections["at_least_1_electron_no_other_leptons"] = ClassSelection(
    r"$N_{e} \geq 1$ - no other leptons",
    at_least_1_electron_no_leptons,
)


def at_least_1_tau_no_leptons(result: ScanResults) -> bool:
    if (
        "Muon" not in result.class_name
        and "Electron" not in result.class_name
        and "Tau" in result.class_name
    ):
        return True
    return False


class_selections["at_least_1_tau_no_other_leptons"] = ClassSelection(
    r"$N_{\tau} \geq 1$ - no other leptons",
    at_least_1_tau_no_leptons,
)

#### --------------------- at least one lepton - no photon


def at_least_1_muon_no_leptons_no_photon(result: ScanResults) -> bool:
    if (
        "Muon" in result.class_name
        and "Electron" not in result.class_name
        and "Tau" not in result.class_name
        and "Photon" not in result.class_name
    ):
        return True
    return False


class_selections["at_least_1_muon_no_other_leptons_no_photon"] = ClassSelection(
    r"$N_{\mu} \geq 1$ - no $e$, $\tau$ or $\gamma$",
    at_least_1_muon_no_leptons_no_photon,
)


def at_least_1_electron_no_leptons_no_photon(result: ScanResults) -> bool:
    if (
        "Muon" not in result.class_name
        and "Electron" in result.class_name
        and "Tau" not in result.class_name
        and "Photon" not in result.class_name
    ):
        return True
    return False


class_selections["at_least_1_electron_no_other_leptons_no_photon"] = ClassSelection(
    r"$N_{e} \geq 1$ - no $\mu$, $\tau$ or $\gamma$",
    at_least_1_electron_no_leptons_no_photon,
)


def at_least_1_tau_no_leptons_no_photon(result: ScanResults) -> bool:
    if (
        "Muon" not in result.class_name
        and "Electron" not in result.class_name
        and "Tau" in result.class_name
        and "Photon" not in result.class_name
    ):
        return True
    return False


class_selections["at_least_1_tau_no_other_leptons_no_photon"] = ClassSelection(
    r"$N_{\tau} \geq 1$ - no $\mu$, $e$ or $\gamma$",
    at_least_1_tau_no_leptons_no_photon,
)

#### --------------------- exactly 2 leptons


def exactly_2_muon_no_leptons(result: ScanResults) -> bool:
    if (
        "2Muon" in result.class_name
        and "Electron" not in result.class_name
        and "Tau" not in result.class_name
    ):
        return True
    return False


class_selections["exactly_2_muon_no_other_leptons"] = ClassSelection(
    r"$N_{\mu} = 2$ - no other leptons",
    exactly_2_muon_no_leptons,
)


def exactly_2_electron_no_leptons(result: ScanResults) -> bool:
    if (
        "Muon" not in result.class_name
        and "2Electron" in result.class_name
        and "Tau" not in result.class_name
    ):
        return True
    return False


class_selections["exactly_2_electron_no_other_leptons"] = ClassSelection(
    r"$N_{e} = 2$ - no other leptons",
    exactly_2_electron_no_leptons,
)


def exactly_2_tau_no_leptons(result: ScanResults) -> bool:
    if (
        "Muon" not in result.class_name
        and "Electron" not in result.class_name
        and "2Tau" in result.class_name
    ):
        return True
    return False


class_selections["exactly_2_tau_no_other_leptons"] = ClassSelection(
    r"$N_{\tau} = 2$ - no other leptons",
    exactly_2_tau_no_leptons,
)

#### --------------------- only light leptons


def only_light_leptons(result: ScanResults) -> bool:
    if (
        ("Muon" in result.class_name or "Electron" in result.class_name)
        and "Tau" not in result.class_name
        and "Photon" not in result.class_name
        and "bJet" not in result.class_name
        and "Jet" not in result.class_name
        and "MET" not in result.class_name
    ):
        return True
    return False


class_selections["only_light_leptons"] = ClassSelection(
    r"only light leptons",
    only_light_leptons,
)


def light_leptons_no_photon_no_tau(result: ScanResults) -> bool:
    if (
        ("Muon" in result.class_name or "Electron" in result.class_name)
        and "Tau" not in result.class_name
        and "Photon" not in result.class_name
    ):
        return True
    return False


class_selections["light_leptons_no_photon_no_tau"] = ClassSelection(
    r"no $\tau$ or $\gamma$",
    light_leptons_no_photon_no_tau,
)

#### --------------------- only photons


def only_photons(result: ScanResults) -> bool:
    if (
        "Muon" not in result.class_name
        and "Electron" not in result.class_name
        and "Tau" not in result.class_name
        and "Photon" in result.class_name
        and "bJet" not in result.class_name
        and "Jet" not in result.class_name
        and "MET" not in result.class_name
    ):
        return True
    return False


class_selections["only_photons"] = ClassSelection(
    r"only photons - no other object",
    only_photons,
)


def only_photons_no_leptons(result: ScanResults) -> bool:
    if (
        "Muon" not in result.class_name
        and "Electron" not in result.class_name
        and "Tau" not in result.class_name
        and "Photon" in result.class_name
    ):
        return True
    return False


class_selections["only_photons_no_leptons"] = ClassSelection(
    r"only photons - no leptons",
    only_photons_no_leptons,
)
