#!/bin/env python
""" This program reads scanner output jsons and saves them back into EventClasses
    or shows a summary of scan results in an EventClass instead        """
from __future__ import print_function

import argparse
from collections import defaultdict
import copy
import csv
import datetime
import fnmatch
import json
import multiprocessing
import os
import re
import signal
import tarfile
import time
import uuid

import sqlalchemy

# from MUSiC-Utils
import dbtools
import mpchain

from ectools.register import ecroot

def main():
    """main function of the project."""

    conf = parse_arguments()

    # Processes:
    # * SQL worker
    # * Directory watcher
    # * Archive Crawler(s)
    group = mpchain.Chain( "processes" ) \
        .layer( directory_worker, jobs=1, kwargs=dict( directories=conf.resultdir,
            patterns=("*_Output.tar.gz", "*_info.json", "*_output.csv"), watch=conf.watch ), source=True ) \
        .layer( file_worker, jobs=4, args=(conf, ) ) \
        .layer( sql_worker, jobs=1, kwargs=dict( database=conf.database ), queue_size=500 )

    def handler( signum, frame ):
        if signum == signal.SIGUSR1:
            print( "=" * 80 )
            print( "STATUS REPORT" )
            group.print_status()
            print( "=" * 80 )

    signal.signal( signal.SIGUSR1, handler )

    group.start()
    group.join()


def directory_worker( directories, patterns, watch=False ):
    seen = set()
    while True:
        for path in recursive_ls( directories, patterns ):
            if path not in seen:
                yield path
                seen.add( path )
        if not watch:
            break

def file_worker( archives, conf ):
    def _handle_json( file, name=None ):
        data = json.load( file )
        scan = dict( hash=data[ "hash" ],
            date=datetime_from_uuid( uuid.UUID( data[ "hash" ] ) ),
            distribution=data[ "distribution" ],
            scan_type=data[ "ScanType" ],
            name=name )

        yield dbtools.Scan, [ scan ]

        for timing_type, timing_values in data[ "timing" ].iteritems():
            timing = dict( hash=data[ "hash" ],
                event_class=data[ "name" ],
                timing_type=timing_type,
                first_round=data[ "firstRound" ],
                count=timing_values[ "count" ],
                total=timing_values[ "total" ] )

            if timing[ "count"] > 2**32:
                print("Warning: timing count too large for DB: %d" % timing[ "count" ])
                timing[ "count" ] = None

            yield dbtools.Timing, [ timing ]

        for stat_type, stat_value in data[ "stats" ].iteritems():
            stat = dict( hash=data[ "hash" ],
                event_class=data[ "name" ],
                stat_type=stat_type,
                first_round=data[ "firstRound" ],
                value=stat_value )

            if stat[ "value"] > 2**32:
                print("Warning: stat value too large for DB: %d" % stat[ "value" ])
                stat[ "value" ] = None

            yield dbtools.Stat, [ stat ]

    def _handle_csv( file ):
        reader = csv.DictReader( file, delimiter="," )
        yield dbtools.Result, list( reader )


    for path in archives:
        print("Working on", path)
        dir = os.path.dirname( path )

        if path.endswith(".tar.gz"):
            for info, file in archive_ls( path, "*_info.json" ):
                print( "Working JSON:", path, ">", info.name )
                for item in _handle_json( file, name=dir ):
                    yield item

            for info, file in archive_ls( path, "*_output.csv" ):
                print( "Working CSV:", path, ">", info.name )
                for item in _handle_csv( file ):
                    yield item

        elif path.endswith("_info.json"):
            with open( path, 'r' ) as file:
                for item in _handle_json( file ):
                    yield item

        elif path.endswith("_output.csv"):
            with open( path, 'r' ) as file:
                for item in _handle_csv( file ):
                    yield item

        else:
            print("Unexpected file:", path)

        # Use "None" as commit token
        yield None

def sql_worker( items, database ):
    engine = dbtools.open_database( database, create=True )
    dbtools.init_database( engine )

    with dbtools.session_scope( engine ) as session:
        for item in items:
            if item is not None:
                mapper, mappings = item

                #session.bulk_update_mappings( mapper, mappings )
                # almost equivalent (to commented out code, because bulk_update_mappings
                # is only available from sqlalchemy version 1.0
                insert = mapper.__table__.insert().prefix_with( "OR REPLACE" )
                try:
                    session.execute( insert, mappings )
                except Exception as e:
                    print("Error inserting: ", mappings)
                    raise

                session.flush()
            else:
                session.commit()

        session.flush()
        session.commit()

def recursive_ls( roots, patterns=None ):
    if isinstance( roots, str ):
        roots = [ roots ]

    for root in roots:
        for dirpath, dirnames, filenames in os.walk( root ):
            if patterns:
                results = set()
                for pattern in patterns:
                    results.update( fnmatch.filter( filenames, pattern ) )
                filenames = list( results )

            for filename in filenames:
                yield os.path.join( dirpath, filename )

def archive_ls( filename, pattern=None ):
    with tarfile.open( filename, 'r' ) as archive:
        for tarinfo in archive:
            if not pattern or fnmatch.fnmatch( tarinfo.name, pattern ):
                yield tarinfo, archive.extractfile( tarinfo )

def datetime_from_uuid( u ):
    # magic values etc from http://stackoverflow.com/a/3795750/489345
    return datetime.datetime.fromtimestamp( ( u.time - 0x01b21dd213814000L )/1e7 )

def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""

    parser = argparse.ArgumentParser( description="Write back scan results to database" )

    general_group = parser.add_argument_group(title="General options")

    general_group.add_argument( "--resultdir",
                                help="Directory which contains scanner output jsons which should be written to ec",
                                type=str, default=os.getcwd(), nargs="+" )
    general_group.add_argument( "--database", help="Path to SQLite database to be used, will be created if necessary",
                                type=str, default=dbtools.default_db_name )
    general_group.add_argument( "--integral", action="store_true",
                                help="Switch to turn on integralScans being written to database")
    general_group.add_argument( "--clear", action="store_true",
                                help="Remove existing scan results before writing new ones." )
    general_group.add_argument( "--watch", action="store_true",
                                help="Continuosly watch folders for changes and update database accordingly." )


    return parser.parse_args()

if __name__=="__main__":
    main()
