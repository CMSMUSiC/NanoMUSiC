from __future__ import print_function

import argparse
import json
import cPickle
import textwrap
import math

from types import ModuleType, ClassType
import sys
import os, os.path

def fake_modules():
    fake_module( "Analyzer", [ "EC_result", "scan_result" ] )
    fake_module( "DiceAndScan", [ "scan_result"] )
    fake_module( "Scan", ["scan_result"])
    fake_module( "MISConfig", ["dummy"] )

def fake_module( name, classes=() ):
    module = ModuleType( name )
    for clsname in classes:
        cls = ClassType( clsname, (object, ), {} )
        setattr( module, clsname, cls )
    sys.modules[ name ] = module
fake_modules()

def toCamelCase( text ):
    out = ""
    i = 0
    while i < len(text):
        c = text[i]
        if c == '_' and i+1 < len(text):
            out += text[i+1].upper()
            i += 2
        else:
            out += c
            i += 1
    return out

def main():
    parser = argparse.ArgumentParser( description="Converter for 8TeV-MUSiC pickle files (*.pkl) to 13TeV-MUSiC JSON job descriptions." )
    parser.add_argument( "pkl", type=str, nargs="+", help="Pickle file(s) to analyze." )
    parser.add_argument( "--config", type=str, default=None, help="Pickle file containing the 8 TeV scanner configuration (usually called 'config.pkl')." )
    parser.add_argument( "--out", type=str, default="./", help="Directory for JSON job files." )
    args = parser.parse_args()

    for filename in args.pkl:
        convert( filename, args.out, configfile=args.config )

def convert( filename, result_directory, configfile=None ):
    with open( filename, 'rb' ) as file:
        source = cPickle.load( file )

    destination = {}
    destination["name"] = source.name

    if configfile:
        with open( configfile, 'rb' ) as file:
            config = cPickle.load( file )
        for name in dir( config ):
            if not name.startswith("_"):
                destination[toCamelCase(name)] = getattr(config, name)
        destination["minRegionWidth"] = int( destination["minRegionWidth"] )
    #destination["minRegionWidth"] = 3
    N_bins = len( source.bg_means )
    mcbins = []
    for bin_index in range( N_bins ):
        lower_edge = source.bg_bins[ bin_index ]
        width = source.bg_bins[ bin_index + 1 ] - lower_edge
        means = source.bg_means[ bin_index ]

        uncerts = {}
        for uncert in source.bg_errorLists[ bin_index ]:
            name, _, value = uncert.partition( "=" )
            value = float( value )
            if name in ("JES","Uncl", "MuoES", "EleES", "GamES" ): continue
            uncerts[ name ] = value
        #print( uncerts )

        for uncert in source.bg_errorDetails[ bin_index ]:
            name, _, value = uncert.partition( "=" )
            value = float( value )
            uncerts[ name ] = value
        #print( uncerts )
        statistical_uncertainty = uncerts.pop( "MC", 0 )

        mcbins.append({
            "MCerror": statistical_uncertainty,
            "width": width,
            "Nevents": means,
            "lowerEdge": lower_edge,
            "systematics": uncerts,
            "unweightedEvents": {},
        })

    destination["MCBins"] = mcbins
    destination["DataBins"] = source.data_means
    if destination["DataBins"] is None:
        destination["DataBins"] = [0.0] * N_bins

    name = os.path.splitext( os.path.basename( filename ) )[0]
    outfilename = os.path.join( result_directory, name + ".json" )
    with open( outfilename, 'wb' ) as file:
        json.dump( destination, file, indent=2 )

    result = {}
    result["JsonFile"] = outfilename
    results = []
    first_bin = mcbins[source.data_res.first_bin]
    last_bin = mcbins[source.data_res.last_bin]
    #print( dir(source.data_res) )
    results.append({
        "data": source.data_res.data_events,
        "mc": source.data_res.MC_events,
        "errors": { "combined": source.data_res.MC_error },
        "lowerEdge": first_bin["lowerEdge"],
        "width": last_bin["lowerEdge"] + last_bin["width"] - first_bin["lowerEdge"],
        "CompareScore": source.data_res.p,
    })
    result["ScanResults"] = results

    resultfilename = os.path.join( result_directory, name + "_result.txt" )
    with open( resultfilename, 'wb' ) as file:
        json.dump( result, file, indent=2 )

    return outfilename, resultfilename

if __name__=="__main__":
    main()
