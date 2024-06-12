from __future__ import print_function

import math
import argparse
from collections import OrderedDict
import os, os.path
import json

import numpy as np
import matplotlib as mpl
mpl.rc("mathtext", rm="sans", default="regular")
import matplotlib.pyplot as plt
import ROOT

from ectools.register import ecstyle

ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.TH1.AddDirectory( False )
ROOT.TDirectory.AddDirectory( False )

REASONS = {
    -99: "Minimum region width",
    -98: "Data without MC",
    -97: "MC without Data",
    -96: "Empty region",
    -95: "Unsignificant",
    -94: "Empty bin added",
    -93: "NH process missing",
    -92: "NH process fluctuates",
    -91: u"Rel. stat. uncert \u2265 60%", #unicode-symbol for >=
    -90: "Negative total MC",
    -89: "Relative Uncertainty",
    -88: "NH process negative",
    -87: "too much negative contribution",
}

class CategoryPlotter:
    def __init__( self, categories, **kwargs ):
        ncolors = 256

        if "cmap" in kwargs:
            self.cmap = kwargs["cmap"]
        else:
            colors = [
                    "#a6cee3",
                    "#1f78b4",
                    "#b2df8a",
                    "#33a02c",
                    "#fb9a99",
                    "#e31a1c",
                    "#fdbf6f",
                    "#f2ef6f",
                    "#ff7f00",
                    "#cab2d6",
                    "#6a3d9a",
                    "#ffff99",
                    "#b15928",
            ]
            name = 'my_colors'
            cmap = mpl.colors.ListedColormap( colors, name=name )
            plt.register_cmap(cmap=cmap)
            self.cmap = name
            ncolors = len( colors )

        self.categories_sorted = OrderedDict( sorted( categories.items(), key=lambda x: x[0] ) )
        boundaries = np.array( self.categories_sorted.keys() ) - 0.5
        boundaries = np.append( boundaries, [ boundaries[-1] + 1 ] )
        self.norm = mpl.colors.BoundaryNorm( boundaries, ncolors=ncolors )
        self.plot = None

        self.pcolor = self.pcolormesh

    def pcolormesh( self, X, Y, C, **kwargs ):
        self.plot = plt.pcolor( X, Y, C, cmap=self.cmap, norm=self.norm, **kwargs )
        return self.plot

    def colorbar( self ):
        if self.plot is None:
            raise ValueError("Must first plot.")
        cbar = plt.colorbar( self.plot )
        cbar.set_ticks( self.categories_sorted.keys() )
        cbar.ax.set_yticklabels( self.categories_sorted.values() )
        return cbar

    def legend( self, values=None, **kwargs ):
        mapper = mpl.cm.ScalarMappable( cmap=self.cmap, norm=self.norm )
        colors = mapper.to_rgba( self.categories_sorted.keys() )
        patches = []
        names = []
        for color, tuple in zip( colors, self.categories_sorted.items() ):
            number, name = tuple
            if values is not None:
                count = np.count_nonzero( values == number )
                if count == 0:
                    continue
                #else:
                #    name += " (%d)" % count

            patches.append( mpl.patches.Patch( color=color ) )
            names.append( name )

        return plt.legend( patches, names, **kwargs )

def th2_to_array( hist ):
    nbins_x = hist.GetXaxis().GetNbins()
    nbins_y = hist.GetYaxis().GetNbins()
    array = np.empty( (nbins_x, nbins_y), dtype=float )
    X = np.empty( (nbins_x, nbins_y), dtype=float )
    Y = np.empty_like( X )
    for x in range( nbins_x ):
        for y in range( nbins_y ):
            array[x, y] = hist.GetBinContent( x+1, y+1 )
            X[x, y] = hist.GetXaxis().GetBinLowEdge( x+1 )
            Y[x, y] = hist.GetYaxis().GetBinLowEdge( y+1 )
    return X, Y, array

def add_cms_text(text='Work in Progress'):
    margin = 0.02

    plt.text(
        0,
        1 + margin,
        text,
        horizontalalignment='left',
        verticalalignment='baseline',
        transform=plt.gca().transAxes,
        fontdict=dict(weight='normal',size='x-large'),
    )

def add_classname(text):
    plt.text(
        1.00,
        1.02,
        text,
        horizontalalignment='right',
        verticalalignment='baseline',
        transform=plt.gca().transAxes,
        fontdict=dict(weight='normal',size='medium'),
    )

def generate_smooth_logscale(min, max, round=True):
    assert min > 0
    assert max > 0
    if round:
        max = math.pow( 10, math.ceil( math.log10( max ) ) )
        min = math.pow( 10, math.floor( math.log10( min ) ) )
    assert min < max
    marks = []
    value = min
    while value < max:
        marks.append( value )
        marks.append( value*2 )
        marks.append( value*5 )
        value *= 10
    marks.append(max)
    return marks


def make_control_plot( filename, args ):
    plt.clf()
    file = ROOT.TFile( filename )
    if not file:
        raise IOError( "Could not open ROOT file." )
    try:
        histo = file.Get( "regions" )
        if not histo:
            raise ValueError( "Could not find control plot in ROOT file." )

        X, Y, C = th2_to_array( histo )
        nonzero_x, nonzero_y = C.nonzero()

        xmax = nonzero_x.max() + 2
        ymax = nonzero_y.max() + 2

        if args.max is not None:
            xmax = ( X <= args.max ).nonzero()[0].max() + 2
            ymax = ( Y <= args.max ).nonzero()[1].max() + 2

        xmin = 0
        ymin = 0

        if args.min is not None:
            xmin = ( X >= args.min ).nonzero()[0].min() - 2
            ymin = ( Y >= args.min ).nonzero()[1].min() - 2

        assert xmin < xmax
        assert ymin < ymax

        C = C[xmin:xmax,xmin:ymax]
        X = X[xmin:xmax,xmin:ymax]
        Y = Y[xmin:xmax,xmin:ymax]

        pValues = np.ma.masked_array( C, C <= 0 )

        if pValues.count() > 0:
            pValues = np.power( 10, -pValues )
            minp = pValues.min() if args.minp is None else args.minp
            maxp = 1 if args.maxp is None else args.maxp

            norm = mpl.colors.LogNorm(minp, maxp)
            pValuesPlot = plt.pcolormesh(X, Y, pValues, norm=norm, cmap="gray", rasterized=True )

            cbar = plt.colorbar( pValuesPlot )
            cbar.set_label( "p-value" )

            ticks = np.array( generate_smooth_logscale(minp, maxp) )
            ticks = ticks[ np.logical_and( ticks > minp, ticks < maxp ) ]
            cbar.ax.get_yaxis().set_ticks(norm(ticks))
            sticks = list("%g" % tick for tick in ticks)
            cbar.ax.get_yaxis().set_ticklabels(sticks)

            cbar.solids.set_rasterized( True )
            #cbar.set_label( r"$- \mathrm{log}_{10}(p)$" + ": %d regions" % ( np.count_nonzero( C > 0 ) ) )

        if args.min is not None:
            plt.xlim( args.min, plt.xlim()[1] )
            plt.ylim( args.min, plt.ylim()[1] )

        if args.max is not None:
            plt.xlim( plt.xlim()[0], args.max )
            plt.ylim( plt.ylim()[0], args.max )

        plotter = CategoryPlotter( REASONS )

        values = np.ma.masked_array( C, C >= 0 )
        plotter.pcolormesh( X, Y, values, rasterized=True )
        legend = plotter.legend( loc=4, values=values )
        legend.get_frame().set_linewidth(0)

        plt.xlabel( "Lower Edge [GeV]" )
        plt.xticks(rotation=90)
        plt.ylabel( "Upper Edge [GeV]" )
        plt.grid()

        if os.path.basename(filename).startswith("Rec_"):
            #Rec_1Ele_3Gamma_2Jet+NJets_SumPt_Output_
            parts = os.path.basename(filename).split( "_" )
            nameparts = parts[ : parts.index( "regions.root" ) - 1 ]
            name = "_".join( nameparts )
            latex = ecstyle.latex_ec_name( name )
            add_classname( "$" + latex + "$" )

        add_cms_text()

        if os.path.isfile( "output.json" ):
            with open( "output.json" ) as jsonfile:
                obj = json.load( jsonfile )
            result = obj[ "ScanResults" ][ 0 ]

            lower = result[ "lowerEdge" ]
            plt.axvline( lower, ymin=-0.1, color="red", linestyle="--" )

            upper = result[ "lowerEdge" ] + result[ "width" ]
            plt.axhline( upper, xmin=-0.1, color="red", linestyle="--" )

        locator = mpl.ticker.MultipleLocator(50)
        plt.gca().xaxis.set_minor_locator( locator )
        plt.gca().yaxis.set_minor_locator( locator )
        locator = mpl.ticker.MultipleLocator(200)
        plt.gca().xaxis.set_major_locator( locator )
        plt.gca().yaxis.set_major_locator( locator )
        plt.tick_params( axis="both", which="major", length=5 )
        plt.tick_params( axis="both", which="minor", length=3 )

        for format in ("pdf", "png"):
            newname = os.path.splitext(filename)[0] + "." + format
            if args.out is not None:
                newname = os.path.basename( newname )
                newname = os.path.join( args.out, newname )
            plt.savefig( newname, bbox_inches="tight" )

            print("Controlplot created at '%s'." % newname)

    finally:
        file.Close()


if __name__=="__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument( "--out", type=str, default=None, help="Output directory" )
    parser.add_argument( "--min", type=float, default=None, help="Lowest bin shown" )
    parser.add_argument( "--max", type=float, default=None, help="Highest bin shown" )
    parser.add_argument( "--minp", type=float, default=None, help="Minimal p to show on the colorbar." )
    parser.add_argument( "--maxp", type=float, default=None, help="Maximal p to show on the colorbar." )
    parser.add_argument( "filename", type=str, nargs="+" )
    args = parser.parse_args()
    for filename in args.filename:
        make_control_plot( filename, args )
