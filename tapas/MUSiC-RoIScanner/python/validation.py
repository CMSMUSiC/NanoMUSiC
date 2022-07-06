from __future__ import print_function

import traceback
import argparse
import tempfile
import os, os.path
import subprocess
import json
import pprint
import sys
import shutil
from multiprocessing import Pool, cpu_count

from import8TeV import convert as convert_json

def main():
    args = parse_arguments()
    cores = min( args.cores, len( args.pkl ) )

    results = []
    for pkl in args.pkl:
        #~ try:
        results.append( validate( pkl ) )
        #~ except:
            #~ print( "can not process %s" % pkl)
    print(dict((x, results.count(x)) for x in results))
    #~ print( "Using %d workers." % cores )
    #~ pool = Pool( cores )
    #~ result = pool.map_async( _validate, args.pkl ).get( 999999 )
    #~ print( result )
    #~ pool.close()
    #~ pool.join()
    #~ pool.terminate()

def _validate( filename ):
    try:
        validate( filename )
    except Exception as e:
        traceback.print_exc( file=sys.stderr )
        raise

def validate( filename ):
    scan_base = os.environ[ "SCAN_BASE" ]
    name = os.path.splitext( os.path.basename( filename ) )[0]
    result_directory = os.path.join( ".", "tmp", name )
    if not os.path.isdir( result_directory ):
        os.makedirs( result_directory )

    print( "START %s" % filename )

    config = os.path.join( os.path.dirname( filename ), "config.pkl" )
    if not os.path.isfile( config ):
        config = None

    json_file, desired_result_file = convert_json( filename, result_directory, config )
    result_file = os.path.join( result_directory, "output.json" )

    # first check if all bins are empty
    filled_bin = False
    print( json_file )
    with open( json_file, "rb" ) as js_file:
        js = json.loads( js_file.read() )
        for i,mcbin in enumerate( js[ "MCBins" ] ):
            if float(mcbin["Nevents"]) > 0.:
                filled_bin = True
                break

    if not filled_bin: return "EmptyBins"

    executable = os.path.join( scan_base, "bin", "scanClass" )
    DEVNULL = open( os.devnull, 'wb' )
    subprocess.check_call( [ executable, "-j", json_file, "-o", result_file ], stdout=DEVNULL, stderr=DEVNULL )

    with open( desired_result_file, 'r' ) as file:
        desired_result = json.load( file )
    with open( result_file, 'r' ) as file:
        result = json.load( file )

    assert( desired_result["JsonFile"] == result["JsonFile"] )

    fail_reason = None

    desired_scan_result = desired_result["ScanResults"][0]
    desired_p = desired_scan_result["CompareScore"]
    if len( result["ScanResults"] ) == 0:
        if( (desired_p - 1.) < 0.00001 ): return "noToOne"
        fail_reason = "no result"
    elif desired_p ==0.: fail_reason ="8TeVZero"
    else:

        scan_result = result["ScanResults"][0]


        p = scan_result["CompareScore"]
        rel_p_diff = abs(p-desired_p) / desired_p

        if rel_p_diff > 0.02:
            fail_reason = "p mismatch"

        if desired_scan_result["lowerEdge"] != scan_result["lowerEdge"]:
            fail_reason = "RoI mismatch"
        if desired_scan_result["width"] != scan_result["width"]:
            fail_reason = "RoI mismatch"

    if fail_reason is None:
        message  = "TEST OK:   " + filename
        shutil.rmtree( result_directory )
        return "Ok"
    else:
        new_result_directory = result_directory + ".fail"
        shutil.move( result_directory, new_result_directory )
        message  = "TEST FAIL: " + filename + "\n"
        message += "    Temporary files stored in " + new_result_directory + "\n"
        message += "    Reason: " + fail_reason + "\n"
    print( message )
    sys.stdout.flush()
    return fail_reason

def parse_arguments():
    parser = argparse.ArgumentParser( description="Validate 8-TeV scan results with the 'new' 13-TeV scanner." )
    parser.add_argument( "pkl", nargs="+", type=str, help="Pickle file(s) to validate." )
    parser.add_argument( "--cores", type=int, default=cpu_count(), help="Number of CPU cores to use for validation." )
    return parser.parse_args()

if __name__=="__main__":
    main()
