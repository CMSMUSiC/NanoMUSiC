import ROOT
import math
import sys
import os


def configure_root() -> None:
    ROOT.gErrorIgnoreLevel = 6000

    ROOT.gSystem.AddIncludePath(
        "-I{}/NanoMUSiC/Classification/include".format(os.getenv("MUSIC_BASE"))
    )
    ROOT.gSystem.AddIncludePath(
        "-I{}/NanoMUSiC/MUSiC/include".format(os.getenv("MUSIC_BASE"))
    )

    if ROOT.gSystem.Load("libDistribution.so") != 0:
        sys.exit(
            'ERROR: Could not load Distribution shared library. Did you "ninja install"?'
        )

    ROOT.PyConfig.IgnoreCommandLineOptions = True
    ROOT.TH1.AddDirectory(False)
    ROOT.TDirectory.AddDirectory(False)
    ROOT.gROOT.SetBatch(True)
    ROOT.EnableThreadSafety()


def to_root_latex(class_name, latex_syntax=False):
    has_suffix = False
    is_first_object = True

    muon_part = ""
    electron_part = ""
    tau_part = ""
    photon_part = ""
    bjet_part = ""
    jet_part = ""
    met_part = ""
    suffix = ""

    for i, p in enumerate(class_name.split("_")):
        if i > 0:
            if "Muon" in p:
                if str(p[0]) != str(0):
                    muon_part = str(p[0]) + r"#mu"
                    is_first_object = False

            if "Electron" in p:
                if str(p[0]) != str(0):
                    if is_first_object:
                        electron_part = str(p[0]) + r"e"
                        is_first_object = False
                    else:
                        electron_part = r" + " + str(p[0]) + r"e"

            if "Tau" in p:
                if str(p[0]) != str(0):
                    if is_first_object:
                        tau_part = str(p[0]) + r"#tau"
                        is_first_object = False
                    else:
                        tau_part = r" + " + str(p[0]) + r"#tau"

            if "Photon" in p:
                if str(p[0]) != str(0):
                    if is_first_object:
                        photon_part = str(p[0]) + r"#gamma"
                        is_first_object = False
                    else:
                        photon_part = r" + " + str(p[0]) + r"#gamma"

            if "bJet" in p:
                if str(p[0]) != str(0):
                    bjet_part = r" + " + str(p[0]) + r"bjet"

            if p[1:] == "Jet" and p[0] != "b" and p[0] != "N":
                if str(p[0]) != str(0):
                    jet_part = r" + " + str(p[0]) + r"jet"

            if "MET" in p:
                if str(p[0]) != str(0):
                    met_part = r" + " + r"p_{T}^{miss}"

            if r"X" in p:
                suffix = r" " + r"incl."
                has_suffix = True

            if r"NJet" in p:
                suffix = r" " + r"jet inc."
                has_suffix = True

    if not has_suffix:
        suffix = " excl."

    output = (
        muon_part
        + electron_part
        + tau_part
        + photon_part
        + bjet_part
        + jet_part
        + met_part
    )

    if latex_syntax:
        output = "$" + output.replace("#", "\\") + "$"

    output = output + suffix

    return output


def make_shifts(shifts):
    cpp_shift = ROOT.std.unordered_map(ROOT.std.string, ROOT.std.vector(ROOT.double))()
    n_shifts = -1
    for s in shifts:
        cpp_shift[s] = ROOT.std.vector(ROOT.double)(shifts[s])
        if n_shifts != len(shifts[s]) and n_shifts >= 0:
            sys.exit(
                "ERROR: Could not parse shifts. All systematic variations should have the number of shifts."
            )
        else:
            n_shifts = len(shifts[s])

    return cpp_shift, n_shifts


def change_exponent(number: float, modifier):
    # Extract the base-10 exponent and mantissa
    mantissa = number / (10 ** math.floor(math.log10(number)))
    exponent = math.floor(math.log10(number))

    # Double the exponent
    exponent_doubled = modifier(exponent)

    # Calculate the new number
    new_number = mantissa * (10**exponent_doubled)

    return new_number
