#!/bin/env python
""" This program reads scanner output jsons and saves them back into EventClasses
    or shows a summary of scan results in an EventClass instead        """
from __future__ import print_function

import os
import json
import glob
import re
from collections import namedtuple

import configargparser
from ectools.register import ecroot

def main():
    """main function of the project."""

    conf = parse_arguments()

    ecroot.ec_loop( conf, show_summary )

def print_scan_result( result, note="" ):
    print( " {scanIndex:>5d} || {lowerEdge:>10.0f} | {width:>10.0f} || {nData:>10.2g} | {nMC:>10.2g} | {uncert:>10.2g} || {p:>10.2g} | {note:s} ".format( note=note, **( result._asdict() ) ) )

def print_header( *entry_list ):
    print( " {:>5s} || {:>10s} | {:>10s} || {:>10s} | {:>10s} | {:>10s} || {:>10s} | ".format( *entry_list ) )

ScanResult = namedtuple("ScanResult", ["scanIndex", "lowerEdge", "width", "nData", "nMC", "uncert", "p"])

def load_all_results( ec, distribution, pseudo=False, max_scans=10000 ):
    result = []
    if pseudo:
        n_sm_pseudo_scans = ec.getNpseudoExp( distribution )
        if max_scans:
            n_sm_pseudo_scans = min( n_sm_pseudo_scans, max_scans )
        for iscan in range( min( n_sm_pseudo_scans, max_scans) ):
            result.append( ScanResult( iscan,
                                ec.getLowerEdgePseudoEntry( distribution, iscan ),
                                ec.getWidthPseudoEntry( distribution, iscan ),
                                ec.getNDataPseudoEntry( distribution, iscan ),
                                ec.getNMCPseudoEntry( distribution, iscan ),
                                ec.getTotalUncertPseudoEntry( distribution, iscan ),
                                ec.getComparePseudoEntry( distribution, iscan ) ) )
    else:
        n_data_scans = ec.getNsignalRounds( distribution )
        if max_scans:
            n_data_scans = min( n_data_scans, max_scans )
        for iscan in range( min( n_data_scans, max_scans) ):
            result.append( ScanResult(  iscan,
                                ec.getLowerEdgeEntry( distribution, iscan ),
                                ec.getWidthEntry( distribution, iscan ),
                                ec.getNDataEntry( distribution, iscan ),
                                ec.getNMCEntry( distribution, iscan ),
                                ec.getTotalUncertEntry( distribution, iscan ),
                                ec.getCompareEntry( distribution, iscan ) ) )
    return result

def print_all_results( results, sorted=True ):
    print_header( "nScan", "lowerEdge", "width", "nData", "nMC", "tot uncrt", "p-value" )
    if sorted:
        results.sort( key=lambda result: result.p )
        for i, result in enumerate( results ):
            if i == 0:
                note = "Min"
            elif i == len( results ) - 1:
                note = "Max"
            elif int( len( results ) / 2 ) == i:
                note = "Median"
            else:
                note = ""
            print_scan_result( result, note=note )
    else:
        for result in results:
            print_scan_result( result )

def show_summary( conf, i=0, N=0, data_ec=None, mc_ec=None, cparser=None, **kwargs ):
    # the_ec points to at least one existing TEventClass, thus can be
    # used to obtain the event class name, etc.
    the_ec = data_ec if data_ec else mc_ec

    for distribution in ecroot.get_ec_distribution_types( the_ec ):
        pseudo_scans = load_all_results( the_ec, distribution, pseudo=True, max_scans=conf.max_scans )
        data_scans = load_all_results( the_ec, distribution, pseudo=False, max_scans=conf.max_scans )

        if not data_scans and not pseudo_scans:
            continue

        print( "Summmary for %s, distribution %s " % ( the_ec.GetName(), distribution ) )
        if pseudo_scans:
            print( "Standard-Model Pseudoexperiments: " )
            print_all_results( pseudo_scans )
            print( "\n" )

        if data_scans:
            print( "Signal Pseudoexperiments / Data: " )
            print_all_results( data_scans )
            print( "\n" )

def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""

    parser = configargparser.ArgumentParser( description="Create MUSiC plots from ROOT files." )

    general_group = parser.add_argument_group(title="General options")

    general_group.add_argument('--dataScan', action="store_true", help="Show results for dataScan")
    general_group.add_argument('--max-scans', default= 1e8,type=int, help="Maximum nober of pseudo scant to show per class")

    ecroot.add_ec_standard_options( general_group )

    args = parser.parse_args()
    return args

if __name__=="__main__":
    main()
