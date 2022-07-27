###################################################
# Plot pileup distributions and resulting weight
#
# pook@physik.rwth-aachen.de

import argparse
import ROOT as ro

ro.gROOT.SetBatch()
ro.gErrorIgnoreLevel = ro.kWarning

ro.gStyle.SetOptStat( 0 )
ro.gStyle.SetOptTitle(0)

def main():
    options = read_command_line_options()
    pileup_plot = PileUpPlot(options)
    #~ pileup_plot.mc_hist
    pileup_plot.plot_distributions()
    pileup_plot.plot_weight()

class PileUpPlot(object):

    def __init__(self, options):
        self.options = options
        self.data_file = ro.TFile.Open(self.options.data_file)
        self.mc_file = ro.TFile.Open(self.options.mc_file)
        # cache
        self._canvas = None
        self._mc_hist = None
        self._data_hist = None
        self._data_hist_up = None
        self._data_hist_down = None
        self._weight_hist = None
        self._weight_hist_down = None
        self._weight_hist_up = None
        self._distribution_legend = None
        self._weight_legend = None

        self.canvas.cd()

    @property
    def canvas(self):
        if self._canvas is None:
            self._canvas = ro.TCanvas( 'Pile-up', 'Pile-up', 700, 650 )
            self._canvas.SetLeftMargin( 0.13 )
            self._canvas.SetRightMargin( 0.05 )
            self._canvas.SetTopMargin(0.06);
            self._canvas.SetBottomMargin(0.13);
            self._canvas.SetLogy( ro.kTRUE )

        return self._canvas

    @property
    def mc_hist(self):
        if self._mc_hist is None:
            self._mc_hist = self.mc_file.Get(self.options.mc_hist_name)
            self._prepare_hist(self._mc_hist, ro.kRed)
        return self._mc_hist

    @property
    def data_hist(self):
        if self._data_hist is None:
            self._data_hist = self.data_file.Get(self.options.data_hist_name)
            self._prepare_hist(self._data_hist, ro.kBlue)
            yMin = self._data_hist.GetMinimum() / 5.0
            yMax = self._data_hist.GetMaximum() * 10.0
            self._data_hist.GetYaxis().SetRangeUser( yMin, yMax )
        return self._data_hist

    @property
    def data_hist_up(self):
        if self._data_hist_up is None:
            histname = self.options.data_hist_name + self.options.data_up_suffix
            self._data_hist_up = self.data_file.Get(histname)
            self._prepare_hist(self._data_hist_up, ro.kCyan + 2 )
        return self._data_hist_up

    @property
    def data_hist_down(self):
        if self._data_hist_down is None:
            histname = self.options.data_hist_name + self.options.data_down_suffix
            self._data_hist_down = self.data_file.Get(histname)
            self._prepare_hist(self._data_hist_down, ro.kViolet - 3 )
        return self._data_hist_down

    @property
    def distribution_legend(self):
        if self._distribution_legend is None:
            self._distribution_legend = ro.TLegend(0.6, 0.7, 0.9, 0.9)
            self._distribution_legend.SetFillStyle( 0 )
            self._distribution_legend.SetTextSize( 0.04 * 0.8 )
            self._distribution_legend.SetBorderSize(0)
        return self._distribution_legend

    @property
    def weight_legend(self):
        if self._weight_legend is None:
            self._weight_legend = ro.TLegend(0.6, 0.7, 0.9, 0.9)
            self._weight_legend.SetFillStyle( 0 )
            self._weight_legend.SetTextSize( 0.04 * 0.8 )
            self._weight_legend.SetBorderSize(0)
        return self._weight_legend

    @property
    def weight_hist(self):
        if self._weight_hist is None:
            self._weight_hist = self.data_hist.Clone()
            self._prepare_weight_hist(self._weight_hist, ro.kBlue)
            yMin = 1e-3
            yMax = self._weight_hist.GetMaximum() * 100.0
            self._weight_hist.GetYaxis().SetRangeUser( yMin, yMax )
            self._weight_hist.GetXaxis().SetRangeUser( 0, 60 )
        return self._weight_hist

    @property
    def weight_hist_up(self):
        if self._weight_hist_up is None:
            self._weight_hist_up = self.data_hist_up.Clone()
            self._prepare_weight_hist(self._weight_hist_up, ro.kCyan + 2)
        return self._weight_hist_up

    @property
    def weight_hist_down(self):
        if self._weight_hist_down is None:
            self._weight_hist_down = self.data_hist_down.Clone()
            self._prepare_weight_hist(self._weight_hist_down, ro.kViolet - 3)
        return self._weight_hist_down

    def _prepare_weight_hist(self, data_hist, color):
        data_hist.Divide(self._mc_hist)
        self.style_hist(data_hist, 'w_{PU}', color)


    def _prepare_hist(self, hist, color):
        if not self.options.no_normalize:
            hist.Scale(1. / hist.Integral())
        self.style_hist(hist, "norm(1)", color)

    def plot_distributions(self):
        self.data_hist.Draw("hist")
        self.distribution_legend.AddEntry(self.data_hist,
                                  "Data <N> = %d" % int(self.data_hist.GetMean()),
                                  "l")
        self.mc_hist.Draw("hist same")
        self.distribution_legend.AddEntry(self.mc_hist,
                                          "Simulation <N> = %d" % int(self.mc_hist.GetMean()),
                                          "l")
        if not self.options.no_uncert:
            self.data_hist_up.Draw("hist same")
            self.distribution_legend.AddEntry(self.data_hist_up,
                                          "Data up variation",
                                          "l")
            self.data_hist_down.Draw("hist same")
            self.distribution_legend.AddEntry(self.data_hist_down,
                                          "Data down variation",
                                          "l")


        self.draw_tex_header()
        self.distribution_legend.Draw()
        self.canvas.Print("pileup_distributions.pdf")
        self.canvas.Clear()

    def plot_weight(self):
        self.canvas.Update()
        self.weight_hist.Draw("hist")
        self.weight_legend.AddEntry(self.weight_hist,"central weight", "l")
        if not self.options.no_uncert:
            self.weight_hist_up.Draw("same hist")
            self.weight_legend.AddEntry(self.weight_hist_up,"up weight", "l")
            self.weight_hist_down.Draw("same hist")
            self.weight_legend.AddEntry(self.weight_hist_down,"down weight", "l")
        self.draw_tex_header()
        self.weight_legend.Draw()
        self.canvas.Print("pileup_weight.pdf")
        self.canvas.Clear()

    def style_hist(self, hist, yTitle, color):
        hist.GetXaxis().SetTitle( 'interactions per event' )
        hist.GetXaxis().SetTitleSize( 0.05 )
        hist.GetXaxis().SetTitleFont( 62 )
        hist.GetXaxis().SetLabelOffset( 0.007 )
        hist.GetXaxis().SetLabelSize( 0.04 )
        hist.GetXaxis().SetLabelFont( 42 )

        hist.GetYaxis().SetTitle( yTitle )
        hist.GetYaxis().SetTitleOffset( 1.10 )
        hist.GetYaxis().SetTitleSize( 0.05 )
        hist.GetYaxis().SetTitleFont( 62 )
        hist.GetYaxis().SetLabelSize( 0.04 )
        hist.GetYaxis().SetLabelFont( 42 )

        hist.SetLineColor(color)
        hist.SetLineWidth(3)

    def draw_tex_header( self, title_outside=False ):

        # Lumi, sqrt(s)
        text = ""
        lumi = self.options.lumi
        if lumi > 0:
            # Lumi is stored as /pb, displayed as /fb
            text += "%.1f#kern[0.05]{fb^{-1}}" % ( lumi / 1000.0 )

        cme = self.options.cme
        if cme > 0:
            # Center of Momentum Energy is stored in GeV, displayed in TeV
            content =  "%g TeV" % int( cme )
            if text:
                text += " (%s)" % content
            else:
                text += content

        if text:
            self.dataset_info_element = ro.TLatex( self.canvas_right, self.text_position_y_info, text )
            self.dataset_info_element.SetTextAlign( 31 )
            self.dataset_info_element.SetNDC()
            self.dataset_info_element.SetTextSize( self.options.text_size_info )
            self.dataset_info_element.SetTextFont( 42 )
            self.dataset_info_element.SetName( "dataset_info_element" )
            self.dataset_info_element.Draw()

    @property
    def canvas_top( self ):
        return 1 - ro.gStyle.GetPadTopMargin()

    @property
    def canvas_bottom( self ):
        return ro.gStyle.GetPadBottomMargin()

    @property
    def canvas_left( self ):
        return ro.gStyle.GetPadLeftMargin()

    @property
    def canvas_right( self ):
        return 1 - ro.gStyle.GetPadRightMargin()

    @property
    def text_position_y_info( self ):
        return self.canvas_top + 0.06 #+ 0.01

def read_command_line_options():
    parser = argparse.ArgumentParser()
    parser.add_argument("mc_file", help="Path to pileup distributin for simulation")
    parser.add_argument("data_file", help="Path to pileup distributin for data")

    parser.add_argument("--mc-hist-name", default="pileup",
        help= "Name of the histogram stored in the input mc file")

    parser.add_argument("--data-hist-name", default="pileup",
        help= "Name of the histogram stored in the input data file")

    parser.add_argument("--data-up-suffix", default="Up",
        help= "Suffix to the histogram name to indicate a up variation")

    parser.add_argument("--data-down-suffix", default="Down",
        help= "Suffix to the histogram name to indicate a down variation")

    parser.add_argument("--no-normalize", action="store_true")

    parser.add_argument("--no-uncert", action="store_true",
        help="Do not plot uncertainties")

    parser.add_argument("--cme", help="Center of mass energy in TeV", default = 13.)

    parser.add_argument("--lumi", default = 35922.)

    parser.add_argument("--text_size_info", default = 0.04)

    return parser.parse_args()

if __name__=="__main__":
    main()
