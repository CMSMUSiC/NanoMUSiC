import ROOT
from collections import namedtuple

from roothelpers import root_color2hex

HistStyle = namedtuple("HistStyle", ("label", "color", "colorhex", "fillstyle", "html"))
HistStyle.__new__.__defaults__ = ("Unknown", 33, 1001, None)

# Here the colors and labels for the MC background are defined.
# The namedtuple holds all information about a process group.
# The color value is expected to be parseable by Matplotlib, e.g. 'red', 'blue'
# or '#FF0000'.
# For convenience, the function root_color2hex( 123 ) is provided, which uses
# PyROOT to convert the color index to a hexadecimal string.


PROCESS_GROUP_STYLES = {
    "Upsilon": HistStyle(label=r"Upsilon", html=r"Upsilon", color=42, colorhex=""),
    "QCD": HistStyle(
        label=r"Multi-Jet", color=ROOT.TColor.GetColor("#4daf4a"), colorhex="#4daf4a"
    ),
    "QCD-BC": HistStyle(label=r"QCD-BCtoE", color=430, colorhex=""),
    "QCD-Mu": HistStyle(label=r"QCD-Mu", color=417, colorhex=""),
    "QCD-Mu10": HistStyle(label=r"QCD-Mu10", color=419, colorhex=""),
    "QCD-Mu15": HistStyle(label=r"QCD-Mu15", color=420, colorhex=""),
    "QCD-EM": HistStyle(label=r"QCD-EM", color=418, colorhex=""),
    "QCD-B": HistStyle(label=r"QCD-B", color=429, colorhex=""),
    "Gamma": HistStyle(
        label=r"$\gamma$ + Jets",
        html="&gamma+Jets",
        color=ROOT.TColor.GetColor("#337a33"),
        colorhex="#337a33",
    ),
    "DiPhoton": HistStyle(
        label=r"$\gamma \gamma$",
        html="&gamma&gamma",
        color=ROOT.TColor.GetColor("#b683eb"),
        colorhex="#b683eb",
    ),
    "GG": HistStyle(
        label=r"$\gamma \gamma$",
        html="&gamma&gamma",
        color=ROOT.TColor.GetColor("#b683eb"),
        colorhex="#b683eb",
    ),
    "TTbar": HistStyle(
        label=r"t $\bar{t}$",
        html="t&tmacr;",
        color=ROOT.TColor.GetColor("#ffdb1a"),
        colorhex="#ffdb1a",
    ),
    "TTbarV": HistStyle(
        label=r"t $\bar{t} + V$",
        html="tt&tmacr;+V",
        color=ROOT.TColor.GetColor("#ccaf14"),
        colorhex="#ccaf14",
    ),
    "Top": HistStyle(
        label=r"Single-Top", color=ROOT.TColor.GetColor("#e41a1c"), colorhex="#e41a1c"
    ),
    "TTTT": HistStyle(
        label=r"$t \bar{t} t \bar{t}$",
        html="tt&tmacr;",
        color=ROOT.TColor.GetColor("#99830f"),
        colorhex="#99830f",
    ),
    "TTG": HistStyle(
        label=r"t $\bar{t} + \gamma$",
        html="tt&tmacr;+&gamma",
        color=ROOT.TColor.GetColor("#ccaf14"),
        colorhex="#ccaf14",
    ),
    "TTGG": HistStyle(
        label=r"t $\bar{t} + \gamma \gamma$",
        html="tt&tmacr;+&gamma&gamma",
        color=ROOT.TColor.GetColor("#ccaf14"),
        colorhex="#ccaf14",
    ),
    "W": HistStyle(
        label=r"W + Jets", color=ROOT.TColor.GetColor("#514608"), colorhex="#514608"
    ),
    "WG": HistStyle(
        label=r"W + $\gamma$",
        html="W + &gamma",
        color=ROOT.TColor.GetColor("#0c3468"),
        colorhex="#0c3468",
    ),
    "WGStar": HistStyle(
        label=r"W + $\gamma*$", color=ROOT.TColor.GetColor("#0c3468"), colorhex="#a468"
    ),
    "WGG": HistStyle(
        label=r"W + $\gamma \gamma$",
        color=ROOT.TColor.GetColor("#244877"),
        colorhex="#244877",
    ),
    "WWG": HistStyle(
        label=r"WW + $\gamma$",
        color=ROOT.TColor.GetColor("#793e82"),
        colorhex="#793e82",
    ),
    "DrellYan": HistStyle(
        label=r"Drell-Yan", color=ROOT.TColor.GetColor("#ee7600"), colorhex="#ee7600"
    ),
    "tZQ": HistStyle(
        label=r"tZq", color=ROOT.TColor.GetColor("#e63032"), colorhex="#e63032"
    ),
    "TZQ": HistStyle(
        label=r"TZQ", color=ROOT.TColor.GetColor("#e63032"), colorhex="#e63032"
    ),
    "ZG": HistStyle(
        label=r"Z+$\gamma$", color=ROOT.TColor.GetColor("#c194ee"), colorhex="#c194ee"
    ),
    "DiBoson": HistStyle(
        label=r"Di-Boson", color=ROOT.TColor.GetColor("#984ee3"), colorhex="#984ee3"
    ),
    "TriBoson": HistStyle(
        label=r"Tri-Boson", color=ROOT.TColor.GetColor("#793E82"), colorhex="#793E82"
    ),
    "Multi-Boson": HistStyle(
        label=r"Multi-Boson", color=ROOT.TColor.GetColor("#984ee3"), colorhex="#984ee3"
    ),
    "WW": HistStyle(
        label=r"WW", color=ROOT.TColor.GetColor("#984ee3"), colorhex="#984ee3"
    ),
    "WZ": HistStyle(
        label=r"WZ", color=ROOT.TColor.GetColor("#793eb5"), colorhex="#793eb5"
    ),
    "ZZ": HistStyle(
        label=r"ZZ", color=ROOT.TColor.GetColor("#5b2e88"), colorhex="#5b2e88"
    ),
    "ZToInvisible": HistStyle(
        label=r"Z->$\nu\nu$", color=ROOT.TColor.GetColor("#f6ba7f"), colorhex="#f6ba7f"
    ),
    "TTW": HistStyle(
        label=r"$t\bar{t}$ + W",
        color=ROOT.TColor.GetColor("#ee7600"),
        colorhex="#ee7600",
    ),
    "TTWW": HistStyle(
        label=r"$t\bar{t}$ + WW",
        color=ROOT.TColor.GetColor("#ee7600"),
        colorhex="#ee7600",
    ),
    "WZG": HistStyle(
        label=r"WZ + $\gamma$",
        color=ROOT.TColor.GetColor("#86518e"),
        colorhex="#86518e",
    ),
    "ZToQQ": HistStyle(
        label=r"Z->qq", color=ROOT.TColor.GetColor("#ee7600"), colorhex="#ee7600"
    ),
    "TTZ": HistStyle(
        label=r"$t\bar{t}$ + Z",
        color=ROOT.TColor.GetColor("#ec5e60"),
        colorhex="#ec5e60",
    ),
    "TTZZ": HistStyle(
        label=r"$t\bar{t}$ + ZZ",
        color=ROOT.TColor.GetColor("#ec5e60"),
        colorhex="#ec5e60",
    ),
    "TG": HistStyle(
        label=r"t + $\gamma$", color=ROOT.TColor.GetColor("#9f1213"), colorhex="#9f1213"
    ),
    "tG": HistStyle(
        label=r"t + $\gamma$", color=ROOT.TColor.GetColor("#9f1213"), colorhex="#9f1213"
    ),
    "WWZ": HistStyle(
        label=r"WWZ", color=ROOT.TColor.GetColor("#6c3775"), colorhex="#6c3775"
    ),
    "WZZ": HistStyle(
        label=r"WZZ", color=ROOT.TColor.GetColor("#542b5b"), colorhex="#542b5b"
    ),
    "TTbarTTbar": HistStyle(
        label=r"$t \bar{t} t \bar{t}$",
        color=ROOT.TColor.GetColor("#99830f"),
        colorhex="#99830f",
    ),
    "HIG": HistStyle(
        label=r"Higgs", color=ROOT.TColor.GetColor("#48d1cc"), colorhex="#48d1cc"
    ),
    "ZZZ": HistStyle(
        label=r"ZZZ", color=ROOT.TColor.GetColor("#3c1f41"), colorhex="#3c1f41"
    ),
    "WWW": HistStyle(
        label=r"WWW", color=ROOT.TColor.GetColor("#793E82"), colorhex="#793E82"
    ),
    "mixed": HistStyle(label=r"Mixed", color=15),
    "Signal": HistStyle(label=r"Signal", color=2, fillstyle=0),
    "Wprime2000": HistStyle(
        label=r"W' 2000GeV (signal)", color=ROOT.TColor.GetColor("#cc0066"), fillstyle=0
    ),
    "Wprime3000": HistStyle(
        label=r"W' 3000GeV (signal)", color=ROOT.TColor.GetColor("#cc0066"), fillstyle=0
    ),
    "Wprime4000": HistStyle(
        label=r"W' 4000GeV (signal)", color=ROOT.TColor.GetColor("#cc0066"), fillstyle=0
    ),
    "Wprime5000": HistStyle(
        label=r"W' 5000GeV (signal)", color=ROOT.TColor.GetColor("#cc0066"), fillstyle=0
    ),
    "proc1": HistStyle(label=r"proc1", color=ROOT.TColor.GetColor("#0c3468")),
    "proc2": HistStyle(label=r"proc2", color=ROOT.TColor.GetColor("#e63032")),
    "proc3": HistStyle(label=r"proc3", color=ROOT.TColor.GetColor("#793e82")),
    "proc4": HistStyle(label=r"proc4", color=ROOT.TColor.GetColor("#0c3468")),
    "proc5": HistStyle(label=r"proc5", color=ROOT.TColor.GetColor("#e63032")),
    "proc6": HistStyle(label=r"proc6", color=ROOT.TColor.GetColor("#244877")),
    "proc7": HistStyle(label=r"proc7", color=ROOT.TColor.GetColor("#c194ee")),
    "proc8": HistStyle(label=r"proc8", color=ROOT.TColor.GetColor("#984ee3")),
    "proc9": HistStyle(label=r"proc9", color=ROOT.TColor.GetColor("#9f1213")),
}

SYSTEMATIC_STYLES = {
    "charge": HistStyle(label="Charge", color=5),
    "Scale": HistStyle(label="Scale", color=85),
    "Resolution": HistStyle(label="Resolution", color=3),
    "xs": HistStyle(label="XSec", color=871),
    "alphas": HistStyle(label="#alpha_{s}", html=r"&alpha s", color=96),
    "fake": HistStyle(label="Mis ID", color=6),
    "pdf": HistStyle(label="PDF", color=7),
    "luminosity": HistStyle(label="Luminosity", color=4),
    "scale_factor": HistStyle(label="ID Efficiency", color=867),
    "stat": HistStyle(label="MC Statistics", color=99),
    "pileup": HistStyle(label="Pileup", color=9),
    "qcdWeight": HistStyle(label="Renorm. & Fact.", color=834),
    "BJetsSF2A": HistStyle(label="BJet SF", color=ROOT.TColor.GetColor("#9f1213")),
    "total": HistStyle(label="Total", color=1),
}

DETAILED_SYSTEMATIC_STYLES = {
    "charge": HistStyle(label="Charge", color=5),
    "Ele_systScale": HistStyle(label="Ele Scale", color=80),
    "Muon_systScale": HistStyle(label="Muon Scale", color=82),
    "Gamma_systScale": HistStyle(label="Gamma Scale", color=83),
    "Jet_systScale": HistStyle(label="Jet Scale", color=84),
    "Tau_systScale": HistStyle(label="Tau Scale", color=86),
    "slimmedMETs_systScale": HistStyle(label="MET Scale", color=88),
    "Ele_systResolution": HistStyle(label="Ele Resolution", color=419),
    "Muon_systResolution": HistStyle(label="Muon Resolution", color=414),
    "Jet_systResolution": HistStyle(label="Jet Resolution", color=409),
    "xs": HistStyle(label="XSec", color=93),
    "alphas": HistStyle(label="#alpha_{s}", color=871),
    "fake": HistStyle(label="Mis ID", color=910),
    "pdf": HistStyle(label="PDF", color=877),
    "luminosity": HistStyle(label="Luminosity", color=4),
    "scale_factor": HistStyle(label="ID Efficiency", color=393),
    "stat": HistStyle(label="MC Statistics", color=99),
    "pileup": HistStyle(label="Pileup", color=9),
    "qcdWeight": HistStyle(label="Renorm. & Fact.", color=834),
    "BJetsSF2A": HistStyle(label="BJet SF", color=ROOT.TColor.GetColor("#9f1213")),
    "total": HistStyle(label="Total", color=1),
}
