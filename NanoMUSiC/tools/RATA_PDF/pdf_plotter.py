##@package pdf_plotter
#Small script to plot the uncertainty bands calculated with RATA_PDF
#
#This script can be used to plot the uncertainty bands of different
#PDF sets which were calculated with the RATA_PDF framework.

#!/usr/bin/env python
import ROOT as r
from array import array
#from env import *
from math import sqrt, fabs

path = "/home/home1/institut_3a/erdweg/Desktop/Software/SirPlotAlot/"

tdrStyle = r.TStyle("tdrStyle","Style for P-TDR");
tdrStyle2 = r.TStyle("tdrStyle2","Style for P-TDR");

def setTDRStyle(logy):
    global tdrStyle
    
    # For the canvas:
    tdrStyle.SetCanvasBorderMode(0);
    tdrStyle.SetCanvasColor(r.kWhite);
    tdrStyle.SetCanvasDefH(600); #Height of canvas
    tdrStyle.SetCanvasDefW(600); #Width of canvas
    tdrStyle.SetCanvasDefX(0);   #POsition on screen
    tdrStyle.SetCanvasDefY(0);
    # For the Pad:
    tdrStyle.SetPadBorderMode(0);
    # tdrStyle.SetPadBorderSize(Width_t size = 1);
    tdrStyle.SetPadColor(r.kWhite);
    tdrStyle.SetPadGridX(False);
    tdrStyle.SetPadGridY(False);
    tdrStyle.SetGridColor(0);
    tdrStyle.SetGridStyle(3);
    tdrStyle.SetGridWidth(1);

    # For the frame:
    tdrStyle.SetFrameBorderMode(0);
    tdrStyle.SetFrameBorderSize(1);
    tdrStyle.SetFrameFillColor(0);
    tdrStyle.SetFrameFillStyle(0);
    tdrStyle.SetFrameLineColor(1);
    tdrStyle.SetFrameLineStyle(1);
    tdrStyle.SetFrameLineWidth(1);

    # For the histo:
    # tdrStyle.SetHistFillColor(1);
    # tdrStyle.SetHistFillStyle(0);
    tdrStyle.SetHistLineColor(1);
    tdrStyle.SetHistLineStyle(0);
    tdrStyle.SetHistLineWidth(1);
    # tdrStyle.SetLegoInnerR(Float_t rad = 0.5);
    # tdrStyle.SetNumberContours(Int_t number = 20);

    tdrStyle.SetEndErrorSize(2);
    #  tdrStyle.SetErrorMarker(20);
    tdrStyle.SetErrorX(0.);

    #tdrStyle.SetMarkerStyle(20);

    #For the fit/function:
    tdrStyle.SetOptFit(0);
    tdrStyle.SetFitFormat("5.4g");
    tdrStyle.SetFuncColor(2);
    tdrStyle.SetFuncStyle(1);
    tdrStyle.SetFuncWidth(1);

    #For the date:
    tdrStyle.SetOptDate(0);
    # tdrStyle.SetDateX(Float_t x = 0.01);
    # tdrStyle.SetDateY(Float_t y = 0.01);

    # For the statistics box:
    tdrStyle.SetOptFile(0);
    tdrStyle.SetOptStat("emr"); # To display the mean and RMS:   SetOptStat("mr");
    tdrStyle.SetStatColor(r.kWhite);
    tdrStyle.SetStatFont(42);
    tdrStyle.SetStatFontSize(0.025);
    tdrStyle.SetStatTextColor(1);
    tdrStyle.SetStatFormat("6.4g");
    tdrStyle.SetStatBorderSize(1);
    tdrStyle.SetStatH(0.1);
    tdrStyle.SetStatW(0.15);
    # tdrStyle.SetStatStyle(Style_t style = 1001);
    # tdrStyle.SetStatX(Float_t x = 0);
    # tdrStyle.SetStatY(Float_t y = 0);

    # Margins:
    tdrStyle.SetPadTopMargin(0.05);
    tdrStyle.SetPadBottomMargin(0.13);
    tdrStyle.SetPadLeftMargin(0.13);
    tdrStyle.SetPadRightMargin(0.05);




    # For the Global title:
    tdrStyle.SetOptTitle(0);
    tdrStyle.SetTitleFont(42);
    tdrStyle.SetTitleColor(1);
    tdrStyle.SetTitleTextColor(1);
    tdrStyle.SetTitleFillColor(10);
    tdrStyle.SetTitleFontSize(0.05);
    # tdrStyle.SetTitleH(0); # Set the height of the title box
    # tdrStyle.SetTitleW(0); # Set the width of the title box
    # tdrStyle.SetTitleX(0); # Set the position of the title box
    # tdrStyle.SetTitleY(0.985); # Set the position of the title box
    # tdrStyle.SetTitleStyle(Style_t style = 1001);
    # tdrStyle.SetTitleBorderSize(2);

    # For the axis titles:
    tdrStyle.SetTitleColor(1, "XYZ");
    tdrStyle.SetTitleFont(42, "XYZ");
    tdrStyle.SetTitleSize(0.06, "XYZ");
    # tdrStyle.SetTitleXSize(Float_t size = 0.02); # Another way to set the size?
    # tdrStyle.SetTitleYSize(Float_t size = 0.02);
    tdrStyle.SetTitleXOffset(0.9);
    tdrStyle.SetTitleYOffset(1.05);
    # tdrStyle.SetTitleOffset(1.1, "Y"); # Another way to set the Offset

    # For the axis labels:
    tdrStyle.SetLabelColor(1, "XYZ");
    tdrStyle.SetLabelFont(42, "XYZ");
    tdrStyle.SetLabelOffset(0.007, "XYZ");
    tdrStyle.SetLabelSize(0.05, "XYZ");

    # For the axis:
    tdrStyle.SetAxisColor(1, "XYZ");
    tdrStyle.SetStripDecimals(r.kTRUE);
    tdrStyle.SetTickLength(0.03, "XYZ");
    tdrStyle.SetNdivisions(508, "XYZ");
    tdrStyle.SetPadTickX(1);  # To get tick marks on the opposite side of the frame
    tdrStyle.SetPadTickY(1);

    # Change for log plots:
    tdrStyle.SetOptLogx(0);
    tdrStyle.SetOptLogy(logy);
    tdrStyle.SetOptLogz(0);

    # Postscript options:
    tdrStyle.SetPaperSize(20.,20.);

    tdrStyle.SetPalette(1);
    
    NRGBs = 5;
    NCont = 255;

    stops = [ 0.00, 0.34, 0.61, 0.84, 1.00 ]
    red   = [ 0.00, 0.00, 0.87, 1.00, 0.51 ]
    green = [ 0.00, 0.81, 1.00, 0.20, 0.00 ]
    blue  = [ 0.51, 1.00, 0.12, 0.00, 0.00 ]
    r.TColor.CreateGradientColorTable(NRGBs, array("d",stops), array("d", red), array("d",green ), array("d", blue), NCont);
    #TColor.CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    #TColor.CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    tdrStyle.SetNumberContours(NCont);
    #gROOT.ForceStyle();
    tdrStyle.cd();
    
    

colors = [1, 2, 3, 4, 6, 7, 8, 9, 11]
markers = [20, 21, 22, 23, 24, 25, 26, 27, 28]
styles = [1, 2, 3, 4, 5, 6, 7, 8, 9]

#this function is not used at the moment but you never know if you want to make a nice color plot
def set_palette(name, ncontours=999):
    """Set a color palette from a given RGB list
    stops, red, green and blue should all be lists of the same length
    see set_decent_colors for an example"""

    if name == "gray" or name == "grayscale":
        stops = [0.00, 0.34, 0.61, 0.84, 1.00]
        red   = [1.00, 0.84, 0.61, 0.34, 0.00]
        green = [1.00, 0.84, 0.61, 0.34, 0.00]
        blue  = [1.00, 0.84, 0.61, 0.34, 0.00]
    # elif name == "whatever":
        # (define more palettes)
    else:
        # default palette, looks cool
        stops = [0.00, 0.34, 0.61, 0.84, 1.00]
        red   = [0.00, 0.00, 0.87, 1.00, 0.51]
        green = [0.00, 0.81, 1.00, 0.20, 0.00]
        blue  = [0.51, 1.00, 0.12, 0.00, 0.00]

    s = array('d', stops)
    r = array('d', red)
    g = array('d', green)
    b = array('d', blue)

    npoints = len(s)
    r.TColor.CreateGradientColorTable(npoints, s, r, g, b, ncontours)
    r.gStyle.SetNumberContours(ncontours)

## Function to read a histogram from a given file in the memory
#
# In this function the .root file is opened and the histogram
# is read in the memory. At the moment also a rebinning with
# the factor 20 is done.
# @param[in] file String of the file name from which the histogram should be read
# @param[in] hist String of the histogram name to be read
# @param[out] ohist TH1F histogram that was read
def reader(file,hist):
    infile = r.TFile(file,"READ")
    ohist = r.TH1F()
    ohist = infile.Get(hist)
    ohist.SetDirectory(0)
    infile.Close()
    ohist.Rebin(20)
    #ohist.Scale(1/20.)
    return ohist

## Main function
#
# In this function the different histograms are read added and plotted
# together.
def main():
    ## Read in the different necessary histograms
    h_main = reader("/user/erdweg/out/new_res/WprimeToMuNu_M-2000.root","results/h1_mtsys_pdf_CT10_mean")
    h_ct_up = reader("/user/erdweg/out/new_res/WprimeToMuNu_M-2000.root","results/h1_mtsys_pdf_CT10_up")
    h_ct_down = reader("/user/erdweg/out/new_res/WprimeToMuNu_M-2000.root","results/h1_mtsys_pdf_CT10_down")
    h_mstw_up = reader("/user/erdweg/out/new_res/WprimeToMuNu_M-2000.root","results/h1_mtsys_pdf_MSTW_up")
    h_mstw_down = reader("/user/erdweg/out/new_res/WprimeToMuNu_M-2000.root","results/h1_mtsys_pdf_MSTW_down")
    h_nnpdf_up = reader("/user/erdweg/out/new_res/WprimeToMuNu_M-2000.root","results/h1_mtsys_pdf_NNPDF_up")
    h_nnpdf_down = reader("/user/erdweg/out/new_res/WprimeToMuNu_M-2000.root","results/h1_mtsys_pdf_NNPDF_down")

    ## Calculate the relative uncertainty bands for the CT pdf set
    #
    # Calculate the difference between the normal and reweighted histograms
    # and translate them into a relative uncertainty
    h_ct_up.Add(h_main,-1)
    h_ct_up.Divide(h_main)
    h_ct_up.SetLineColor(r.kGreen)
    h_ct_up.SetLineStyle(1)
    h_ct_up.SetLineWidth(1)

    h_ct_down.Add(h_main,-1)
    h_ct_down.Divide(h_main)
    h_ct_down.SetLineColor(r.kGreen)
    h_ct_down.SetLineStyle(1)
    h_ct_down.SetLineWidth(1)

    h_ct_mean = h_ct_down.Clone("h_ct_mean")
    h_ct_mean.SetLineColor(r.kGreen)
    h_ct_mean.SetLineStyle(2)
    h_ct_mean.SetLineWidth(1)

    ## Calculate the mean distributions for the CT pdf set
    for i in range(1,h_ct_down.GetNbinsX()):
        h_ct_mean.SetBinContent(i,(h_ct_down.GetBinContent(i) + h_ct_up.GetBinContent(i))/2.)
    g_ct = r.TGraph(2*h_ct_up.GetNbinsX())
    n_bin = 1
    for i in range(1,h_ct_up.GetNbinsX()):
        g_ct.SetPoint(i,h_ct_up.GetBinCenter(i),h_ct_up.GetBinContent(i))
        n_bin = i
    for i in range(1,h_ct_down.GetNbinsX()):
        g_ct.SetPoint(i+n_bin,h_ct_down.GetBinCenter(h_ct_down.GetNbinsX()+1-i),h_ct_down.GetBinContent(h_ct_down.GetNbinsX()+1-i))

    ## Set the plotting style for the CT pdf set
    g_ct.SetFillColor(r.kGreen)
    g_ct.SetFillStyle(3002)
    g_ct.SetLineColor(r.kGreen)
    g_ct.SetLineWidth(1)

    ## Calculate the relative uncertainty bands for the MSTW pdf set
    #
    # Calculate the difference between the normal and reweighted histograms
    # and translate them into a relative uncertainty
    h_mstw_up.Add(h_main,-1)
    h_mstw_up.Divide(h_main)
    h_mstw_up.SetLineColor(r.kBlue)
    h_mstw_up.SetLineStyle(1)
    h_mstw_up.SetLineWidth(1)

    h_mstw_down.Add(h_main,-1)
    h_mstw_down.Divide(h_main)
    h_mstw_down.SetLineColor(r.kBlue)
    h_mstw_down.SetLineStyle(1)
    h_mstw_down.SetLineWidth(1)

    h_mstw_mean = h_mstw_up.Clone("h_mstw_mean")
    h_mstw_mean.SetLineColor(r.kBlue)
    h_mstw_mean.SetLineStyle(2)
    h_mstw_mean.SetLineWidth(1)

    ## Calculate the mean distributions for the MSTW pdf set
    for i in range(1,h_mstw_down.GetNbinsX()):
        h_mstw_mean.SetBinContent(i,(h_mstw_down.GetBinContent(i) + h_mstw_up.GetBinContent(i))/2.)
    g_mstw = r.TGraph(2*h_mstw_up.GetNbinsX())
    n_bin = 1
    for i in range(1,h_mstw_up.GetNbinsX()):
        g_mstw.SetPoint(i,h_mstw_up.GetBinCenter(i),h_mstw_up.GetBinContent(i))
        n_bin = i
    for i in range(1,h_mstw_down.GetNbinsX()):
        g_mstw.SetPoint(i+n_bin,h_mstw_down.GetBinCenter(h_mstw_down.GetNbinsX()+1-i),h_mstw_down.GetBinContent(h_mstw_down.GetNbinsX()+1-i))

    ## Set the plotting style for the MSTW pdf set
    g_mstw.SetFillColor(r.kBlue)
    g_mstw.SetFillStyle(3002)
    g_mstw.SetLineColor(r.kBlue)
    g_mstw.SetLineWidth(1)

    ## Calculate the relative uncertainty bands for the NNPDF pdf set
    #
    # Calculate the difference between the normal and reweighted histograms
    # and translate them into a relative uncertainty
    h_nnpdf_up.Add(h_main,-1)
    h_nnpdf_up.Divide(h_main)
    h_nnpdf_up.SetLineColor(r.kTeal)
    h_nnpdf_up.SetLineStyle(1)
    h_nnpdf_up.SetLineWidth(1)

    h_nnpdf_down.Add(h_main,-1)
    h_nnpdf_down.Divide(h_main)
    h_nnpdf_down.SetLineColor(r.kTeal)
    h_nnpdf_down.SetLineStyle(1)
    h_nnpdf_down.SetLineWidth(1)

    h_nnpdf_mean = h_nnpdf_up.Clone("h_nnpdf_mean")
    h_nnpdf_mean.SetLineColor(r.kTeal)
    h_nnpdf_mean.SetLineStyle(2)
    h_nnpdf_mean.SetLineWidth(1)

    ## Calculate the mean distributions for the NNPDF pdf set
    for i in range(1,h_nnpdf_up.GetNbinsX()):
        h_nnpdf_mean.SetBinContent(i,(h_nnpdf_down.GetBinContent(i) + h_nnpdf_up.GetBinContent(i))/2.)
    g_nnpdf = r.TGraph(2*h_nnpdf_up.GetNbinsX())
    n_bin = 1
    for i in range(1,h_nnpdf_up.GetNbinsX()):
        g_nnpdf.SetPoint(i,h_nnpdf_up.GetBinCenter(i),h_nnpdf_up.GetBinContent(i))
        n_bin = i
    for i in range(1,h_nnpdf_down.GetNbinsX()):
        g_nnpdf.SetPoint(i+n_bin,h_nnpdf_down.GetBinCenter(h_nnpdf_down.GetNbinsX()+1-i),h_nnpdf_down.GetBinContent(h_nnpdf_down.GetNbinsX()+1-i))

    ## Set the plotting style for the NNPDF pdf set
    g_nnpdf.SetFillColor(r.kTeal)
    g_nnpdf.SetFillStyle(3002)
    g_nnpdf.SetLineColor(r.kTeal)
    g_nnpdf.SetLineWidth(1)

    ## Calculate the relative uncertainty bands for all pdf set
    #
    # Calculate the difference between the normal and reweighted histograms
    # and translate them into a relative uncertainty
    h_main.Add(h_main,-1)
    h_main.SetLineColor(r.kBlack)
    h_main.SetLineStyle(1)
    h_main.SetLineWidth(1)

    ## Set the plotting style for the complete envelope
    U = h_main.Clone("U")
    U.SetLineColor(r.kRed)
    U.SetLineStyle(1)
    U.SetLineWidth(2)
    L = h_main.Clone("L")
    L.SetLineColor(r.kRed)
    L.SetLineStyle(1)
    L.SetLineWidth(2)
    M = h_main.Clone("M")
    M.SetLineColor(r.kRed)
    M.SetLineStyle(2)
    M.SetLineWidth(2)

    ## Calculate the PDF set envelope
    for i in range(1,h_main.GetNbinsX()):
        U.SetBinContent(i,max(h_ct_up.GetBinContent(i),h_mstw_up.GetBinContent(i),h_nnpdf_up.GetBinContent(i)))
        L.SetBinContent(i,min(h_ct_down.GetBinContent(i),h_mstw_down.GetBinContent(i),h_nnpdf_down.GetBinContent(i)))
        M.SetBinContent(i,(U.GetBinContent(i) + L.GetBinContent(i))/2.)

    ## Create the Legend for the plot
    leg = r.TLegend(0.164573,0.165803,0.364322,0.415803,"")
    leg.SetFillColor(r.kWhite)
    leg.SetLineColor(r.kWhite)
    leg.AddEntry(g_ct,"CT10","lf")
    leg.AddEntry(g_mstw,"MSTW","lf")
    leg.AddEntry(g_nnpdf,"NNPDF","lf")
    leg.AddEntry(U,"envelope","l")
    leg.AddEntry(M,"mean","l")

    ## Set the style for the plot and create the canvas
    setTDRStyle(0)
    c1 = r.TCanvas("c1","",800,800)

    ## Draw all histograms
    h_main.GetYaxis().SetRangeUser(-0.35,0.35)
    h_main.GetXaxis().SetRangeUser(220,3000)
    h_main.GetYaxis().SetTitle("(pdf - raw) / raw")
    h_main.GetXaxis().SetTitle("M_{T} (GeV)")
    h_main.SetStats(0)
    h_main.Draw("hist")
    g_ct.Draw("F same")
    g_mstw.Draw("F same")
    g_nnpdf.Draw("F same")
    h_ct_up.Draw("hist same")
    h_ct_down.Draw("hist same")
    h_ct_mean.Draw("hist same")
    h_mstw_up.Draw("hist same")
    h_mstw_down.Draw("hist same")
    h_mstw_mean.Draw("hist same")
    h_nnpdf_up.Draw("hist same")
    h_nnpdf_down.Draw("hist same")
    h_nnpdf_mean.Draw("hist same")
    U.Draw("hist same")
    L.Draw("hist same")
    M.Draw("hist same")
    h_main.Draw("hist same")
    leg.Draw("same")

    ## Add the textheader
    cmspre = r.TLatex()
    cmspre.SetNDC()
    cmspre.SetTextSize(0.04)
    cmspre.DrawLatex( 0.13,.965,"CMS Private Work")

    intlumi = r.TLatex()
    intlumi.SetNDC()
    intlumi.SetTextAlign(12)
    intlumi.SetTextSize(0.03)
    intlumi.DrawLatex(0.45,0.97,"#mu + #slash{E}_{T}      #scale[0.7]{#int} L dt = 20 fb^{-1}")

    ecm = r.TLatex()
    ecm.SetNDC()
    ecm.SetTextAlign(12)
    ecm.SetTextSize(0.03)
    ecm.DrawLatex(0.83,0.975,"#sqrt{s} = 8 TeV")

    ## Redraw the axis of the plot
    r.gPad.RedrawAxis()

    ## Wait for user input
    raw_input("done")

    ## Save the results as .root and as .pdf file
    c1.SaveAs("pdf_sys.root")
    c1.SaveAs("pdf_sys.png")

## Function to call the main method
#
if __name__ == '__main__':
    main()
