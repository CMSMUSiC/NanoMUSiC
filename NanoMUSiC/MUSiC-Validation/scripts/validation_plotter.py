#!/usr/bin/env python3
import os
import array

from binning import base_binning

music_base = os.environ["MUSIC_BASE"]

import ROOT

h = ROOT.TH1F("asd", "asd", 1,0,10)


ROOT.gStyle.SetOptStat(0)
ROOT.gROOT.SetBatch()  # don't pop up canvases
ROOT.TH1.SetDefaultSumw2()

# Set TDR styles
ROOT.gROOT.LoadMacro(f"{music_base}/NanoMUSiC/MUSiC-Validation/plotter/tdrstyle.C")
ROOT.gInterpreter.ProcessLine("setTDRStyle();")
ROOT.gROOT.LoadMacro(f"{music_base}/NanoMUSiC/MUSiC-Validation/plotter/CMS_lumi.C")

ROOT.gROOT.ProcessLine(
    "tdrStyle->SetPadRightMargin(0.05);"
)  # fix a margin issuie of the CMS macros

ROOT.gStyle.SetLegendBorderSize(
    0
)  # define standard border size for legends (no border)
ROOT.gStyle.SetLegendFillColor(0)  # legeneds backgrounds should be white
ROOT.gStyle.SetLegendTextSize(0.0)  # no title for legends
class Plotter:
    def __init__(
        self,
        outputs_reference: str,
        input_mc_files: str,
        input_data_files: str,
        outputs_dir: str,
    ) -> None:
        self.outputs_reference = outputs_reference
        self.input_mc = []
        for f in input_mc_files:
            self.input_mc.append(ROOT.TFile.Open(f))
        self.input_data = ROOT.TFile.Open(input_data_files)
        self.outputs_dir = outputs_dir

        # prepare outputs area
        os.system(f"rm -rf {self.outputs_dir}/{self.outputs_reference} > /dev/null")
        os.system(f"mkdir -p {self.outputs_dir}/{self.outputs_reference}")

    def print_canvas(self, canvas: ROOT.TCanvas, histogram_name: str):
        # canvas.Draw()  # print the canvas to the screen
        canvas.SaveAs(
            f"{self.outputs_dir}/{self.outputs_reference}/{histogram_name}.png"        )
        # canvas.Print(
        #     f"{self.outputs_dir}/{self.outputs_reference}/{histogram_name}.pdf", ".pdf"
        # )
        # canvas.Print(
        #     f"{self.outputs_dir}/{self.outputs_reference}/{histogram_name}.root",
        #     ".root",
        # )
        # canvas.Print(
        #     f"{self.outputs_dir}/{self.outputs_reference}/{histogram_name}.C", ".C"
        # )

    def plot(
        self,
        histogram_name: str,
        x_axis_label,
        run_lumi_tag,
        rebin_callable,
        change_last_bin,
        set_lower_bound=False,
    ):
        # get MC histograms
        histos_mc = []
        for h in self.input_mc:
            histos_mc.append(h.Get(histogram_name))
        
        # get Data histograms
        histo_data = self.input_data.Get(histogram_name)
        
        # rebin
        histo_data, histos_mc = rebin_callable(histo_data, histos_mc, change_last_bin)

        # configure data
        histo_data.SetMarkerStyle(20)  # set black dots as Data marker
        histo_data.SetMarkerSize(0.7)
        histo_data.SetMarkerColor(ROOT.kBlack)

        # configure MC
        h_stack = ROOT.THStack("h_temp_stack", f";{x_axis_label};Events")
        histo_mc_sum = histos_mc[0].Clone()
        histo_mc_sum.Reset()
        for h in histos_mc:
            h.SetLineWidth(0)
            h.SetFillColor(ROOT.kOrange - 2)
            h_stack.Add(
                h, "histsame"
            )
            histo_mc_sum.Add(h)  

        h_stack.SetMinimum(min(histo_mc_sum.GetMinimum(0.), histo_data.GetMinimum(0.)))
        h_stack.SetMaximum(max(histo_mc_sum.GetMaximum(), histo_data.GetMaximum())*1.1)

        # configure legend
        # define position of the legend
        x1_l = 0.8
        y1_l = 0.90
        dx_l = 0.30
        dy_l = 0.1
        x0_l = x1_l - dx_l
        y0_l = y1_l - dy_l

        legend = ROOT.TLegend(x0_l, y0_l, x1_l, y1_l)
        legend.AddEntry(histo_data, "Data", "PE")  # add Data histogram to legend
        for h in histos_mc:
            legend.AddEntry(
                h, "Drell-Yan (M_{LepLep} > 50 GeV)", "f"
            )  


        # calculate ratio
        c = ROOT.TCanvas("c")
        c.SetLogy()
        ratio = ROOT.TRatioPlot(histo_data, histo_mc_sum)
        ratio.Draw()
        upper_pad = ratio.GetUpperPad()

        upper_pad.cd()
        lp = upper_pad.GetListOfPrimitives() 
        for p in lp:
            lp.Remove(p)
        # h_stack.SetHistogram(ROOT.TH1F())
        # h_stack.GetHistogram().GetYaxis().SetLabelSize(0.)
        # h_stack.SetLabelSize(0.)
        # h_stack.Draw()
        # histo_data.Draw("same")
        # c.Update()


        # ratio=_ratio.GetCalculationOutputGraph()
        # ratio=histo_mc_sum
        # ratio.SetMinimum(-4)
        # ratio.SetMaximum(4)


        # configure ratio

        # #  canvas and pads
        # # create canvas and pads
        # c = ROOT.TCanvas("c")
        # upper_pad = ROOT.TPad("upper_pad", "upper_pad", 0, 0.2, 1, 1.0)
        # upper_pad.SetBottomMargin(0)  # joins upper and lower plot
        # # upper_pad.SetGridx()
        # upper_pad.Draw()
        


        # # Lower ratio plot is pad2
        # c.cd()  # returns to main canvas before defining pad2
        # lower_pad = ROOT.TPad("lower_pad", "lower_pad", 0, 0.05, 1, 0.2)
        # lower_pad.SetTopMargin(0)  # joins upper and lower plot
        # # lower_pad.SetBottomMargin(0.2)
        # # lower_pad.SetGridx()
        # lower_pad.Draw()

        
        # # draw upper
        # upper_pad.cd()
        # # h_stack.SetMinimum(0.0001)
        # h_stack.Draw()
        # histo_data.Draw("same")
        # # Do not draw the Y axis label on the upper plot and redraw a small
        # # axis instead, in order to avoid the first label (0) to be clipped.
        # upper_pad.SetLogy()
        
        # # draw legend
        # legend.Draw()

        # # draw lower
        # lower_pad.cd()
        # ratio.Draw("AP")

        # # print to file
        # upper_pad.Update()  # update the canvas with all modifications
        # lower_pad.Update()

        self.print_canvas(c, histogram_name)


        # this_plot.GetXaxis().SetLabelSize(0.025)
        # this_plot.GetUpYaxis().SetLabelSize(0.025)
        # this_plot.GetLowYaxis().SetLabelSize(0.025)

        # if set_lower_bound:
            # h_stack.SetMinimum(0.001)
        # h_stack.SetMaximum(h_stack.GetMaximum() * 100)
        # print(h_stack.GetMaximum())


        # ROOT.gROOT.ProcessLine(
        #     f'CMS_lumi(upper_pad, 0, 11, "Work in progress", "{run_lumi_tag}");',
        # )  # add information abou which period this Data corresponds

        # upper_pad.SetLogy()



def no_rebinning(histo_data: ROOT.TH1F, histo_mc: ROOT.TH1F, change_last_bin=False):
    return histo_data, histo_mc


def rebin_energy_like(
    histo_data: ROOT.TH1F, histos_mc: ROOT.TH1F, change_last_bin=False
):
    """
    Will rebin energy-like histograms. Last bin is reduce to encopass data and a more coarse binning is applied.
    """
    new_binning = base_binning
    if change_last_bin:
        last_data_point = -1
        for idx_bin in range(1300, 0, -1):
            if histo_data.GetBinContent(idx_bin) != 0:
                last_data_point = histo_data.GetBinCenter(idx_bin)
                break

        for idx, _ in enumerate(base_binning[:-1]):
            if (
                base_binning[idx] < last_data_point
                and base_binning[idx + 1] > last_data_point
            ):
                new_binning = base_binning[: idx + 2]
                break

    rebinned_data = histo_data.Rebin(
        len(new_binning) - 1, "histo_data", array.array("d", new_binning)
    )
    rebinned_mc = []
    for h in histos_mc:
        rebinned_mc.append(
            h.Rebin(len(new_binning) - 1, "histo_mc", array.array("d", new_binning))
        )

    return rebinned_data, rebinned_mc


def leplep_plots(lepton, root_latex_name, name, input_mc, input_data):
    z_LepLep_X = Plotter(
        name,
        input_mc,
        input_data,
        "validation_plots",
    )
    z_LepLep_X.plot(
        "h_invariant_mass",
        "M_{inv}",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        rebin_energy_like,
        True,
    )
    z_LepLep_X.plot(
        "h_sum_pt",
        "#Sigma p_{T}",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        rebin_energy_like,
        True,
    )
    z_LepLep_X.plot(
        "h_met", "MET", "Run2018 [59.83 fb^{-1} (13 TeV)]", rebin_energy_like, True
    )
    z_LepLep_X.plot(
        "h_lepton_1_pt",
        f"p_{{T}}^{{lead {root_latex_name}}}",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        rebin_energy_like,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_2_pt",
        f"p_{{T}}^{{sublead {root_latex_name}}}",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        rebin_energy_like,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_1_eta",
        f"#eta^{{lead {root_latex_name}}}",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_2_eta",
        f"#eta^{{sublead {root_latex_name}}}",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_1_phi",
        f"#phi^{{lead {root_latex_name}}}",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        no_rebinning,
        True,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_2_phi",
        f"#phi^{{sublead {root_latex_name}}}",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        no_rebinning,
        True,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_1_jet_1_dPhi",
        f"#Delta #phi({root_latex_name}, jet_{{lead}})",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_lepton_1_jet_1_dR",
        f"#Delta R({root_latex_name}, jet_{{lead}})",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_jet_multiplicity",
        "N Jets",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        no_rebinning,
        True,
    )
    z_LepLep_X.plot(
        "h_bjet_multiplicity",
        "N BJets",
        "Run2018 [59.83 fb^{-1} (13 TeV)]",
        no_rebinning,
        True,
    )


def main():
    leplep_plots(
        "mu",
        "#mu",
        "z_MuMu_X",
        ["validation_outputs_BKP/DYJetsToLL_M-50_13TeV_AM_z_to_mu_mu_x.root"],
        "validation_outputs_BKP/SingleMuon_z_to_mu_mu_x.root",
    )
    # leplep_plots(
    #     "e",
    #     "e",
    #     "z_EleEle_X",
    #     ["validation_outputs_BKP/DYJetsToLL_M-50_13TeV_AM_z_to_ele_ele_x.root"],
    #     "validation_outputs_BKP/SingleMuon_z_to_ele_ele_x.root",
    # )


if __name__ == "__main__":
    main()
