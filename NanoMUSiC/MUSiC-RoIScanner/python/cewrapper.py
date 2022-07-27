#!/bin/env python
import argparse
import os
import sys
import json
import glob
import tarfile
import tempfile
import shutil
import subprocess

temp_output_directory = tempfile.mkdtemp( prefix="scan_temp_", dir=os.getcwd() )

def pack( arch_name, folder, filters ):
    with tarfile.open( arch_name, 'w|gz') as archive:
        last_wd = os.getcwd()
        os.chdir( folder )
        for filter in filters:
            for filename in glob.iglob( filter ):
                if os.path.isfile( filename ):
                    archive.add( filename )
        os.chdir( last_wd )

def process_ec(args, input_json_path, first_round, nrounds):

    arguments = [ args.executable,
          "-j", input_json_path,
          "-o", temp_output_directory,
          "-s", args.shifts,
          "-l", str(first_round),
          "-n", str(nrounds)
        ]
    arguments = map( str, arguments )
    print "CEWRAPPER calling:", " ".join( arguments )
    subprocess.call( arguments, stderr=sys.stderr, stdout=sys.stdout )


parser = argparse.ArgumentParser( description='Main script to run MUSiC scans' )
parser.add_argument( '-o', '--out-archive', type=str,
    help="Arch which will be filled with json files containing scan results" )
parser.add_argument( '-e', '--executable', type=str,
    help="Path to the executable" )
group = parser.add_mutually_exclusive_group()
splitting_group = group.add_argument_group(title="splitting arguments", description=None)

group.add_argument( '--splitting',  default="splitting.json", type=str,
    help="Path to splitting json" )
splitting_group.add_argument( '--input-json', type=str,
    help="Path to splitting json" )
splitting_group.add_argument( '--first-round',  default=-1, type=int,
    help="Path to splitting json" )
splitting_group.add_argument( '--nrounds',  default=1, type=int,
    help="Path to splitting json" )
parser.add_argument( '-s', '--shifts', default="shifts.json", type=str,
    help="Path to systematics shift json" )

args = parser.parse_args()

if args.first_round >= 0:
    process_ec(args, args.input_json, args.first_round, args.nrounds)
else:
    # run main scan: an input file can contain multiple event classes
    # with the current splitting mechanism
    with open( args.splitting, "r" ) as file:
        split_infos = json.load( file )
    for ec_name, split_info in split_infos.items():
        name_base = ec_name + "_" + split_info[ "distribution" ]
        in_json_name = name_base + "_Input.json"
        process_ec(args,
                   in_json_name,
                   int( split_info[ "first_round" ] ),
                   int( split_info[ "num_rounds" ] ) )

pack( args.out_archive, temp_output_directory,
        [ "*_output.csv", "*_info.json", "*_regions.root" ] )

shutil.rmtree( temp_output_directory )
