#!/usr/bin/env python

import logging
import optparse
import os
import re
import sys

log = logging.getLogger( 'cleanProcessNames' )

def main():

    usage = '%prog PROCESSFILES'
    description = "This script checks PROCESSFILES, created e.g. by parseSampleList.py, for process names that apear multiple times in these files. PROCESSFILES can be a space-separated list. In the respective file, lines with duplicate process names will be marked at the beginning of the line by the string '-DUPLICATE-' unless the option -n is set."

    parser = optparse.OptionParser( usage = usage, description = description, version = '%prog 0' )
    parser.add_option(       '--debug', metavar = 'LEVEL', default = 'INFO',
                       help = 'Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default: %default]' )
    parser.add_option( '-n', '--nomark',action='store_true', default=False,
                       help = "Don't change the PROCESSFILES by writing 'DUPLICATE:' at the beginning of lines with duplicate process names." )
    ( options, args ) = parser.parse_args()

    if not args:
        parser.error( 'Needs PROCESSFILES to work!' )

    # Set loggig format and level
    #
    format = '%(name)s [%(levelname)s] at %(asctime)s: %(message)s'
    logging.basicConfig( filename = 'log_cleanProcessNames.txt', filemode = 'w', level = logging._levelNames[ options.debug ], format = format, datefmt = '%F %H:%M:%S' )
    console = logging.StreamHandler()
    console.setLevel( logging._levelNames[ options.debug ] )
    log.addHandler( console )

    log.info( 'Checking for duplicates ....' )
    log.debug( 'Checking files: ' + ' '.join( args ) )

    procs_and_files = []
    procs_list = []
    for file in args:
        mc_file = open( file, 'r' )
        for i, line in enumerate( mc_file.readlines(), 1 ):
            if not line.startswith( 'config' ) and not line.startswith( '#' ) and not line.startswith( 'generator' ) and line.strip():
                proc = line.strip().split( ':' )[0]
                procs_and_files.append( [ proc, file, i ] )
                procs_list.append( proc )

    dupl_procs = set( sample for sample in procs_list if procs_list.count( sample ) > 1 )
    if dupl_procs:
        log.warning( 'These process names appear multiple times:' )
        for item in sorted( dupl_procs ):
            for i in range( len( procs_and_files ) ):
                if item == procs_and_files[i][0]:
                    print_duplicates = item + '  --->  ' + procs_and_files[i][1] + ' in line no.: ' + str( procs_and_files[i][2] )
                    log.warning( print_duplicates )
        log.warning( 'You *must* adjust their names by hand!!' )
    else:
        log.info( "You're safe: No duplicates were found." )

    if not options.nomark:
        for file in args:
            mc_file_r = open( file, 'r' )
            mc_file_w = open( file + '_tmp', 'w' )
            for line in mc_file_r.readlines():
                if line.strip().split( ':' )[0] in dupl_procs:
                    line = '# -DUPLICATE- ' + line.strip()
                    print >> mc_file_w, line
                else:
                    print >> mc_file_w, line,
            mc_file_r.close()
            mc_file_w.close()
            os.rename( file + '_tmp', file )


if __name__ == '__main__':
    main()