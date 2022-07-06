#!/bin/env python

# General python includes
import os
from configobj import ConfigObj
try:
    from collections import OrderedDict
except ImportError:
    from ordered import OrderedDict

# rootpy includes
from rootpy.io import root_open

# Include the plotting classes
from DukePlotALot import *
import style_class as sc

# Include the histogram storage class
from plotlib import HistStorage,getColorList,getDictValue,HistStorageContainer

# Global variables
root_directory = os.getcwd() + '/temp_root_files/'
xs_file_name = 'xs_test.cfg'
lumi = 1

# Main method to call all test functions
def main():

    Creat_standard_plot()

    return 42

# Function to create a standard plot with:
# - Stacked histograms as background
# - Line histograms as signal
# - Data points for the data
# - Normalize the simulation according to a cross section file
# Also apply basic style criteria
def Creat_standard_plot():
    # First, lets create some dummy histograms to have something to plot
    create_test_histos()

    # Second, lets create a dummy cross section file
    create_xs_file()

    # Read in the created cross section file
    xs = ConfigObj(xs_file_name)

    # Create the HistStorage object to store and manipulate the histograms
    bghists = HistStorage(xs, lumi, path = root_directory)

    # Create the list of background samples
    # Each entry in the dict can contain more than one sample
    # The samples for each key will later be merged for the plot
    bglist = OrderedDict()
    bglist["Background 1"] = ['BG1']
    bglist["Background 2"] = ['BG2']

    # Create a dict to define the color which should later be used in the plot
    colorList={}
    colorList["Background 1"]="pink"
    colorList["Background 2"]="green"

    # Add the background histograms (and their colors) to the HistStorage
    bghists.addFileList(bglist)
    bghists.colorList = colorList

    # Now do the same for the signal histograms
    sghist = HistStorage(xs, lumi, path = root_directory)

    sgName = "Signal"
    # You can set additional sample specific weights
    sghist.additionalWeight = {"Signal":1.0}
    # You can also add all histograms which contain just a tag
    sghist.addAllFiles(tag = "SG", joinName = sgName)
    sghist.colorList = {sgName :"darkred"}

    # Create a HistContainer for the data, therefore set the data flag :-)
    dat_hist = HistStorage(xs, lumi, path = root_directory, isData = True)
    dat_hist.addFile("Dat")

    # Create a list of histograms that should be plotted
    hists = ["h1_dummy_hist"]

    # Add all HistStorage objects to the HistStorageContainer
    histContainer = HistStorageContainer(bg = bghists, data = dat_hist, sg = sghist)

    # Define histogram specific binnings
    binning={
            "_pt":10,
            "_MT":range(200,300,20)+range(300,400,50)+range(400,1600,100)+range(1600,2000,200),
            "_met_et":30,
    }

    # Define histogram specific x-ranges
    xranges={
            "_pt":[70,1400],
            "_met_et":[120,1000],
    }

    # List of histograms that should be made cumulative
    cumulative=[]

    # Set the general style options
    bghists.initStyle(style = "bg")
    sghist.initStyle(style = "sg")

    # Create the style container object, which defines the style of each plot
    hist_style = sc.style_container(lumi = lumi, style = 'CMS', useRoot = False)

    # Loop over all histograms that should be plotted and load the histogram
    for hist in hists:
        histContainer.getHist(hist)

        # Apply a binning if specified
        binf = getDictValue(hist, binning)
        if binf is not None:
            if isinstance(binf, list):
                histContainer.rebin(vector = binf)
            else:
                histContainer.rebin(width = binf)

        # Make histograms cumulative if specified
        if getDictValue(hist, cumulative):
            histContainer.makeCumulative()

        # Make the actual plotting object
        test = plotter(hist = histContainer.getBGList(),
                       sig = histContainer.getSGList(),
                       style = hist_style,
                       cmsPositon = "upper left")

        # Add the data histogram
        test.Add_data(histContainer.getData())

        # Set the x-axis range if specified
        mxrange = getDictValue(hist, xranges)
        if mxrange is not None:
            test.Set_axis(xmin = mxrange[0], xmax = mxrange[1], ymin = 1.01e-1, ymax = 0.5e4)
        else:
            test.Set_axis(ymin = 1., ymax = 1e5)

        if True:
            test.Add_plot('Signi',pos=0, height=15)
            test.Add_plot('Ratio',pos=1, height=15)
            test.Add_plot('DiffRatio',pos=2, height=15)

        if True:
            tfile = root_open(root_directory + "/sys.root", "READ")
            sys_hist = tfile.Get('systematics')
            test.Add_error_hist([sys_hist], band_center = 'ref', stacking = 'No')

        # Set the name of the plot
        name = hist.replace("/", "")

        # Create and save the final plot
        test.make_plot('%s.pdf'%(name))

# Function to create a dummy cross section file, to normalize every histogram
def create_xs_file():
    ofile = open(xs_file_name, 'w')
    ofile.write('[BG1]\n')
    ofile.write('xs=     1\n')
    ofile.write('weight= 1\n')
    ofile.write('Nev=    1000\n')
    ofile.write('\n')
    ofile.write('[BG2]\n')
    ofile.write('xs=     1\n')
    ofile.write('weight= 1\n')
    ofile.write('Nev=    1000\n')
    ofile.write('\n')
    ofile.write('[SG1]\n')
    ofile.write('xs=     1\n')
    ofile.write('weight= 1\n')
    ofile.write('Nev=    1000\n')
    ofile.close()

# Method to create dummy histograms to test the plotting functions
def create_test_histos():
    # set the random seed
    ROOT.gRandom.SetSeed(42)
    np.random.seed(42)

    # signal distribution
    signal = 126 + 10 * np.random.randn(1000)
    signal_obs = 126 + 10 * np.random.randn(1000)

    # create histograms
    h1 = Hist(30, 40, 200, title='Background 1', markersize=0)
    h1.GetXaxis().SetTitle('Mass (GeV)')
    h1.GetYaxis().SetTitle('Events')
    h2 = h1.Clone(title='Signal')
    h3 = h1.Clone(title='Data')
    h4 = h1.Clone(title='Systematics')
    h5 = h1.Clone(title='Background 2')

    # fill the histograms with our distributions
    h1.FillRandom('landau', 10000)
    h4.FillRandom('landau', 10000)
    h5.FillRandom('landau', 100000)
    h4.Scale(1.1)
    h4.Add(h1,-1)
    h4.Divide(h1)
    map(h2.Fill, signal)
    h3.FillRandom('landau', 10000)
    h3.FillRandom('landau', 100000)
    map(h3.Fill, signal_obs)

    # Check if the output directory exists, if not create it
    if not os.path.isdir(root_directory):
        os.mkdir(root_directory)

    # Write the different output files
    tfile = root_open(root_directory + "/BG1.root", "RECREATE")
    h1.Write('h1_dummy_hist')
    tfile.close()

    tfile = root_open(root_directory + "/SG1.root", "RECREATE")
    h2.Write('h1_dummy_hist')
    tfile.close()

    tfile = root_open(root_directory + "/Dat.root", "RECREATE")
    h3.Write('h1_dummy_hist')
    tfile.close()

    tfile = root_open(root_directory + "/sys.root", "RECREATE")
    h4.Write('systematics')
    tfile.close()

    tfile = root_open(root_directory + "/BG2.root", "RECREATE")
    h5.Write('h1_dummy_hist')
    tfile.close()


main()
