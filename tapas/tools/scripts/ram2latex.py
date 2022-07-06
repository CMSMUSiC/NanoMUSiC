#!/bin/env python

import os,sys
import argparse
import logging
import argparse
import subprocess

try:
    from table2latex.textable import TexTable
except ImportError:
    print "table2latex not installed, checkout https://github.com/tobias-pook/table2latex"

# setup logging
log = logging.getLogger("ram2latex")

def setupLogging( args ):
    #setup logging
    format = '%(levelname)s: %(message)s'
    logging.basicConfig( level=logging._levelNames[ args.debug ], format=format)
    log.setLevel(logging._levelNames[ args.debug ])
    formatter = logging.Formatter( format )

def commandline_parsing():
    parser = argparse.ArgumentParser( formatter_class=argparse.RawTextHelpFormatter,
        description="Create latex input files with tables from database" )
    parser.add_argument( '--debug', metavar='LEVEL', default='INFO',
        choices=[ 'ERROR', 'WARNING', 'INFO', 'DEBUG' ],
        help='Set the debug level. default: %(default)s' )
    parser.add_argument( '-i','--ram-config', metavar='RAM_YAML',
        help='A ram yaml input file', required = True)
    parser.add_argument( '-o','--out', metavar='FILE', default = 'summary.tex',
        help='Name of output text file')
    parser.add_argument( '-t','--config',
        help='Path to a texLib table infig file. Options in config overwrite CLI')
    parser.add_argument( '--dump-name', metavar='FILE', default = 'ram_dump.csv',
        help='Name of the dumped csv file which is read from aix3adb')

    table_group = parser.add_argument_group( title="Table options",
        description= "Options used to determine construction of tables" )

    table_group.add_argument( '--chunksize', default = 900,
        help= 'Number of rows before new header printed / page break')
    table_group.add_argument( '--tablestyle', default = 'longtable',
        choices = ['longtable', 'table'],
        help= 'Number of rows before new header printed / page break')
    table_group.add_argument( '--landscape', action='store_true',
        help= 'Produce table in landscape mode' )
    table_group.add_argument( '--hide-groups', action='store_true',
        help='Hide group in table. Still used for ordering if chosen')
    args = parser.parse_args()
    return args

def main():

    args = commandline_parsing()
    setupLogging( args )
    if not os.environ.get('TOOLS3A'):
        log.error("TAPAS tools repo not sourced, exiting")
        sys.exit(1)

    # run aix3adbManager to create complete output from database
    #~ cmd = 'aix3adbManager.py '
    cmd = 'aachen3adbManager.py '
    cmd += '--full-output '
    cmd += '--latest-only '
    cmd += '--ram-config %s ' % args.ram_config
    cmd += '-o %s ' % args.dump_name
    cmd += '--debug %s ' % args.debug
    print cmd
    p = subprocess.Popen( cmd,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,
                          shell=True )
    (string_out,string_err) = p.communicate()
    print string_out
    if p.returncode != 0:
        log.error( "Unable to run aachen3adbManager.py" )
        log.error( cmd )
        log.error( string_out )
        log.error( string_err )
        sys.exit(1)
    # create table object
    chosen_column_keys = [   'sa_name',
                    'sa_crosssection',
                    ]
    print vars(args)
    table = TexTable(**vars(args))
    # read in csv file from database dump
    table.read_csv(args.dump_name)
    # create and write the latex table
    table.write_tex_file()
    table.write_pdf_file()

if __name__=="__main__":
    main()
