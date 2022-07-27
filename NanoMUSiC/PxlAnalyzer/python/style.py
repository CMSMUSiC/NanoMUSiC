import ROOT

def set_root_style():
    #For the canvas:
    ROOT.gStyle.SetCanvasBorderMode(0)
    ROOT.gStyle.SetCanvasColor(ROOT.kWhite)
    ROOT.gStyle.SetCanvasDefH(600)
    ROOT.gStyle.SetCanvasDefW(600)
    ROOT.gStyle.SetCanvasDefX(0)
    ROOT.gStyle.SetCanvasDefY(0)

#For the Pad:
    ROOT.gStyle.SetPadBorderMode(0)
    ROOT.gStyle.SetPadColor(ROOT.kWhite)
    ROOT.gStyle.SetPadGridX(False)
    ROOT.gStyle.SetPadGridY(False)
    ROOT.gStyle.SetGridColor(0)
    ROOT.gStyle.SetGridStyle(3)
    ROOT.gStyle.SetGridWidth(1)

# For the frame:
    ROOT.gStyle.SetFrameBorderMode(0)
    ROOT.gStyle.SetFrameBorderSize(1)
    ROOT.gStyle.SetFrameFillColor(0)
    ROOT.gStyle.SetFrameFillStyle(0)
    ROOT.gStyle.SetFrameLineColor(1)
    ROOT.gStyle.SetFrameLineStyle(1)
    ROOT.gStyle.SetFrameLineWidth(1)

    # For the histo:
    ROOT.gStyle.SetHistLineColor(1)
    ROOT.gStyle.SetHistLineStyle(0)
    ROOT.gStyle.SetHistLineWidth(1)

    ROOT.gStyle.SetEndErrorSize(2)

    ROOT.gStyle.SetMarkerStyle(20)
    ROOT.gStyle.SetMarkerSize(0.5)

#For the fit/function:
    ROOT.gStyle.SetOptFit(1)
    ROOT.gStyle.SetFitFormat("5.4g")
    ROOT.gStyle.SetFuncColor(2)
    ROOT.gStyle.SetFuncStyle(1)
    ROOT.gStyle.SetFuncWidth(1)

#For the date:
    ROOT.gStyle.SetOptDate(0)

# For the statistics box:
    ROOT.gStyle.SetOptFile(0)
    ROOT.gStyle.SetOptStat(0) # To display the mean and RMS:  SetOptStat("mr")
    ROOT.gStyle.SetStatColor(ROOT.kWhite)
    ROOT.gStyle.SetStatFont(42)
    ROOT.gStyle.SetStatFontSize(0.025)
    ROOT.gStyle.SetStatTextColor(1)
    ROOT.gStyle.SetStatFormat("6.4g")
    ROOT.gStyle.SetStatBorderSize(1)
    ROOT.gStyle.SetStatH(0.1)
    ROOT.gStyle.SetStatW(0.15)

#Margins
    ROOT.gStyle.SetPadTopMargin(0.07)
    ROOT.gStyle.SetPadBottomMargin(0.12)
    ROOT.gStyle.SetPadRightMargin(0.035)

#Global title
    #ROOT.gStyle.SetOptTitle(0)
    ROOT.gStyle.SetTitleFont(42)
    ROOT.gStyle.SetTitleColor(1)
    ROOT.gStyle.SetTitleTextColor(1)
    ROOT.gStyle.SetTitleFillColor(10)
    ROOT.gStyle.SetTitleFontSize(0.05)

# For the axis titles:
    ROOT.gStyle.SetTitleColor(1, "XYZ")
    ROOT.gStyle.SetTitleFont(42, "XYZ")
    ROOT.gStyle.SetTitleSize(0.05, "XYZ")
    ROOT.gStyle.SetTitleXOffset(1)
    ROOT.gStyle.SetTitleYOffset(0.98)


# For the axis labels:
    ROOT.gStyle.SetLabelColor(1, "XYZ")
    ROOT.gStyle.SetLabelFont(42, "XYZ")
    ROOT.gStyle.SetLabelOffset(0.007, "XYZ")
    ROOT.gStyle.SetLabelSize(0.05, "XYZ")

# For the axis:
    ROOT.gStyle.SetAxisColor(1, "XYZ")
    ROOT.gStyle.SetStripDecimals(True)
    ROOT.gStyle.SetTickLength(0.02, "XYZ")
    ROOT.gStyle.SetNdivisions(510, "XYZ")
    ROOT.gStyle.SetPadTickX(1)  # To get tick marks on the opposite side of the frame
    ROOT.gStyle.SetPadTickY(1)

# Change for log plots:
    ROOT.gStyle.SetOptLogx(0)
    ROOT.gStyle.SetOptLogy(0)
    ROOT.gStyle.SetOptLogz(0)
