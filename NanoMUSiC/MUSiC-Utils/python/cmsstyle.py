import math
import ROOT as ro
from roothelpers import *

# global style object
tdrStyle = ro.TStyle("tdrStyle","Style for P-TDR")

def set_cms_root_style():
    global tdrStyle

    # For the canvas:
    tdrStyle.SetCanvasBorderMode(0);
    tdrStyle.SetCanvasColor(ro.kWhite);
    tdrStyle.SetCanvasDefH(600); #Height of canvas
    tdrStyle.SetCanvasDefW(600); #Width of canvas
    tdrStyle.SetCanvasDefX(0);   #POsition on screen
    tdrStyle.SetCanvasDefY(0);
    # For the Pad:
    tdrStyle.SetPadBorderMode(0);
    # tdrStyle.SetPadBorderSize(Width_t size = 1);
    tdrStyle.SetPadColor(ro.kWhite);
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
    tdrStyle.SetHistLineStyle(1);
    tdrStyle.SetHistLineWidth(0);
    tdrStyle.SetMarkerStyle(20);
    # tdrStyle.SetLegoInnerR(Float_t rad = 0.5);
    # tdrStyle.SetNumberContours(Int_t number = 20);

    tdrStyle.SetEndErrorSize(2);
    #  tdrStyle.SetErrorMarker(20);
    #tdrStyle.SetErrorX(0.);

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
    #~ tdrStyle.SetOptStat("emr"); # To display the mean and RMS:   SetOptStat("mr");
    tdrStyle.SetOptStat(00); # To display the mean and RMS:   SetOptStat("mr");
    tdrStyle.SetStatColor(ro.kWhite);
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
    #~ tdrStyle.SetPadRightMargin(0.15);




    # For the Global title:
    tdrStyle.SetOptTitle(0);
    #~ tdrStyle.SetTitleFont(42);
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
    tdrStyle.SetTitleSize(0.035, "XYZ");
    # tdrStyle.SetTitleXSize(Float_t size = 0.02); # Another way to set the size?
    # tdrStyle.SetTitleYSize(Float_t size = 0.02);
    tdrStyle.SetTitleXOffset(1.2);
    tdrStyle.SetTitleYOffset(1.6);
    # tdrStyle.SetTitleOffset(1.1, "Y"); # Another way to set the Offset

    # For the axis labels:
    tdrStyle.SetLabelColor(1, "XYZ");
    tdrStyle.SetLabelFont(42, "XYZ");
    tdrStyle.SetLabelOffset(0.007, "XYZ");
    #~ tdrStyle.SetLabelSize(0.03, "XYZ");
    tdrStyle.SetLabelSize(0.04, "XYZ");

    # For the axis:
    tdrStyle.SetAxisColor(1, "XYZ");
    tdrStyle.SetStripDecimals(ro.kTRUE);
    tdrStyle.SetTickLength(0.03, "XYZ");
    #tdrStyle.SetNdivisions(508, "XYZ");
    tdrStyle.SetPadTickX(1);  # To get tick marks on the opposite side of the frame
    tdrStyle.SetPadTickY(1);

    # For the Legend:
    tdrStyle.SetLegendFont(42)
    tdrStyle.SetLegendFillColor(ro.kWhite)
    tdrStyle.SetLegendBorderSize( 0 )
    # Change for log plots:
    tdrStyle.SetOptLogx(0);
    tdrStyle.SetOptLogy(1);
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
    #TColor.CreateGradientColorTable(NRGBs, array("d",stops), array("d", red), array("d",green ), array("d", blue), NCont);
    #TColor.CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    #TColor.CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    #~ dibosonC = 1756
    #~ topC = 1757
    #~ dyC = 1758
    #~ DYcolor = ro.TColor(dyC, , 0.2, 0.3)
    tdrStyle.SetNumberContours(NCont);
    #gROOT.ForceStyle();
    tdrStyle.cd();
    return NCont

def exponent_notation( number ):
    """Convert a number to text-exponent notation, e.g. 5.2e-2, 3.1e0."""
    if number == 0:
        return "0e0"
    sign = -1 if number < 0 else 1
    number = abs( number )
    magnitude = int( math.floor( math.log10( number ) ) )
    number /= math.pow( 10, magnitude )
    return "%s%.1fe%d" % ( "-" if sign==-1 else "", number, magnitude )


def create_custom_subplot( figure, Class, *args, **kwargs):
    """Instantiate the custom class Class (inheriting from matplotlib.axes.Axes)
    and attach it to the given figure. *args and **kwargs are passed to the
    new subplot, just as it would be with add_subplot( *args, **kwargs ).
    """
    axes = matplotlib.axes.subplot_class_factory( axes_class=Class )( figure, *args, **kwargs )
    figure.add_subplot( axes )
    return axes


def cms_matplotlib_config():
    """Adjust the global Matplotlib configuration to match CMS style."""

    # use sans-serif font for \mathrm{foo}
    matplotlib.rcParams[ 'mathtext.rm' ]        = 'sans'

    # increase the default font size (default: 12)
    matplotlib.rcParams[ 'font.size' ]          = 16.0

    # use the same font for math as for normal text
    matplotlib.rcParams[ 'mathtext.default' ]   = 'regular'

    # use Arial as sans-serif font (suggested by CMS)
    matplotlib.rcParams[ 'font.sans-serif' ]    = 'Arial'
