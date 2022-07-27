#!/usr/bin/env python

import multiprocessing
import multiprocessing.dummy
import ConfigParser
import argparse
import subprocess
import os.path
import os
import sys
import logging

def call( args ):
    command = " ".join( args )
    logging.getLogger().info( "Calling: '%s'", command )
    subprocess.call( args, stdout=sys.stdout, stderr=sys.stderr )

def parseArguments():
    parser = argparse.ArgumentParser( description='Create config for merger out of music config.' )
    parser.add_argument( '--inpath', '-i', required=True, help='path to grid output' )
    parser.add_argument( '--outpath', '-o', required=True, help='output path' )
    parser.add_argument( '--config', '-c', help='name of output config', default='merge_final.cfg' )
    parser.add_argument( '--dry', help='only create config file', action='store_true' )
    parser.add_argument( '--cpus', type=int, default=0, help='Number of CPUs used for merging.' )
    parser.add_argument( '--debug', metavar='LEVEL', default='INFO',
                       help = 'Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default: %(default)s]' )
    return parser.parse_args()

def write_config( dict, config_name ):
    parser = ConfigParser.RawConfigParser()
    parser.optionxform = str
    for name, path in dict.iteritems():
        parser.set( "DEFAULT", name + '.root', 1 )
    file_out = open( config_name, 'w' )
    parser.write( file_out )

def main():
    options = parseArguments()
    
    numeric_level = getattr( logging, options.debug.upper(), None )
    logging.basicConfig( stream=sys.stderr, level=numeric_level, format="%(asctime)-15s %(name)s (%(threadName)s) %(levelname)s: %(message)s ")

    logging.info( "Starting..." )
    logging.debug( "Command line arguments: %s", options )

    if options.dry:
        logging.info( "Running in dry mode. Do not merge but create config" )

    merge_list = {}
    name = ''
    for path, dirnames, filename in os.walk( os.path.abspath( options.inpath ) ):
        found = False
        for dirname in dirnames:
            if "CREAM" in dirname and "bak" not in path:
                name = path.split( '/' )[ -1 ].split( "-skimid" )[ 0 ]
                if '_ext' in name:
                    name_sample = name.split( '_ext' )[ 0 ] + name.split( '_ext' )[ 1 ][ 1: ]
                else:
                    name_sample = name
                if not name_sample in merge_list:
                    merge_list[ name_sample ] = [ path ]
                else:
                    merge_list[ name_sample ].append( path )
                break

    logging.debug( "Paths: %s", merge_list )

    if not "MUSIC_UTILS" in os.environ:
        raise EnvironmentError( "'MUSIC_UTILS' environment variable not set." )

    arguments = []
    for name, paths in merge_list.iteritems():
        program_options = [ "--merge=all", "-j", "16", "--final", "--data=False", "--num-files=200" ]

        inputFiles = paths
        outputFile = os.path.join( os.path.abspath( options.outpath ), name + ".root" )

        program_name = os.path.join( os.environ[ "MUSIC_UTILS" ], "scripts", "ECMerger.py" )
        program_name = os.path.expanduser( program_name )

        args = [ program_name, '-o', outputFile ] + inputFiles + program_options

        arguments.append( args )

    logging.debug( "Resulting arguments: %s", arguments )

    processes = multiprocessing.cpu_count()
    if options.cpus:
        processes = options.cpus
    processes = min( processes, len( arguments ) )

    if not options.dry:
        logging.info( "Running %d jobs on %d workers." % ( len(arguments), processes ) )

        pool = multiprocessing.dummy.Pool( processes=processes )  
        pool.map( call, arguments )
        pool.close()
        try:
            logging.debug( "Waiting for processes to finish..." )
            pool.join()
        except KeyboardInterrupt:
            logging.info( "Ctrl+C pressed, terminating..." )
            pool.terminate()

    logging.info( "Writing config file" )
    write_config( merge_list, options.config )

    logging.info( "Finished." )

if __name__=="__main__":
    main()

