#!/bin/env python

from __future__ import print_function

import json
import argparse

import matplotlib as mpl
mpl.use( "Qt4Agg" )

import numpy as np
import matplotlib.pyplot as plt

from cmsstyle import *

def plot( args ):
    p_values = []

    for filename in args.json:
        with open( filename, 'r' ) as file:
            data = json.load( file )
        for item in data["ScanResults"]:
            p_values.append( item["CompareScore"] )

    p_values = np.array( p_values )
    p_values = - np.log10( p_values )

    range = (0, 8)
    bins = range[1] - range[0]

    print( p_values )
    hist, edges = np.histogram( p_values, range=range, bins=bins, density=False )
    widths = np.diff( edges )
    edges = edges[:-1]

    print( edges, hist)

    figure = plt.figure()
    axis = create_custom_subplot( figure, CMSAxes, 111 )
    axis.bar( edges, hist, width=widths, color="red" )
    axis.set_xlabel( "-log10(p)" )
    axis.set_ylabel( "# of dicing rounds" )
    axis.set_xlim(*range)
    axis.set_ylim(0, hist.max() * 1.1)
    plt.show()

def parse_arguments():
    parser = argparse.ArgumentParser( description="Plot a p-tilde distribution from given output.json file(s)" )
    parser.add_argument( "json", nargs="+", help="output.json file", type=str )
    args = parser.parse_args()
    return args

if __name__=="__main__":
    args = parse_arguments()
    plot( args )
