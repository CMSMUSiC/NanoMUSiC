#!/usr/bin/env python

from __future__ import print_function

import multiprocessing
import multiprocessing.dummy
import ConfigParser
import argparse
import subprocess
import os.path
import os
import sys
import logging

def parseArguments():
    parser = argparse.ArgumentParser( description='Create config for merger out of music config.' )
    parser.add_argument( '--inpath', '-i', required=True, help='path to output of first merge step' )
    parser.add_argument( '--config', '-c', default='merge_final.cfg', help='input config file for merger' )
    parser.add_argument( '--outpath', '-o', required=True, help='output name' )
    parser.add_argument( '--cpus', type=int, default=0, help='Number of CPUs used for merging.' )
    parser.add_argument( '--debug', metavar='LEVEL', default='INFO',
                       help = 'Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default: %(default)s]' )
    return parser.parse_args()

def main():
    options = parseArguments()

    numeric_level = getattr( logging, options.debug.upper(), None )
    logging.basicConfig( stream=sys.stderr, level=numeric_level, format="%(asctime)-15s %(name)s (%(threadName)s) %(levelname)s: %(message)s ")

    logging.info( "Starting..." )
    logging.debug( "Command line arguments: %s", options )

    config = ConfigParser.ConfigParser()
    config.optionxform = str
    config.read( options.config )

    merge_files_tmp = config.defaults().keys()
    merge_files = []

    for merge_file in merge_files_tmp:
        merge_files.append( os.path.join( options.inpath, merge_file ) )

    logging.debug( "Input files: %s", merge_files )

    if not "MUSIC_UTILS" in os.environ:
        raise EnvironmentError( "'MUSIC_UTILS' environment variable not set." )

    arguments = []
    program_options = [ "--merge=all", "-j", str( options.cpus ), "--final", "--data=False" ]

    outputFile = os.path.join( os.path.abspath( options.outpath ) )

    program_name = os.path.join( os.environ[ "MUSIC_UTILS" ], "scripts", "ECMerger.py" )
    program_name = os.path.expanduser( program_name )

    args = [ "python" ] + [ program_name, '-o', outputFile ] + merge_files + program_options

    arguments.append( args )

    logging.debug( "Resulting arguments: %s", arguments )

    command = " ".join( args )
    logging.getLogger().info( "Calling: '%s'", command )
    subprocess.call( args, stdout=sys.stdout, stderr=sys.stderr )

    logging.info( "Finished." )

if __name__=="__main__":
    main()

