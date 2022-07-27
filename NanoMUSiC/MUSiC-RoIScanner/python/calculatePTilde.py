from __future__ import print_function

import argparse
import sqlite3
import numpy as np
import time

import sqlalchemy

import dbtools
from ectools import calc_p_tilde, get_pseudo_p_tilde_list, super_filter

def main():
    conf = parse_arguments()

    engine = sqlalchemy.create_engine( 'sqlite:///' + conf.database, echo=False )

    with dbtools.session_scope( engine ) as session:
        compare_scan = session.query( dbtools.Scan ).get( conf.compare_hash )
        if not compare_scan:
            raise ValueError( "Compare scan with hash %s not found." % conf.compare_hash )

        if compare_scan.scan_type != 'pseudoScan':
            print( "The scan type is '%s' which seems not to be a standard BG/BG scan." % compare_scan.scan_type )
            answer = prompt( "Do you want to proceed anyways?" )
            if not answer:
                return

        for destination_hash in conf.destination_hash:
            destination_scan = session.query( dbtools.Scan ).get( destination_hash )
            if not destination_scan:
                raise ValueError( "Destination scan with hash %s not found." % destination_hash )

            if compare_scan.distribution != destination_scan.distribution:
                print( "The distribution types '%s' and '%s' seem not to match." % ( compare_scan.distribution, destination_scan.distribution ) )
                answer = prompt( "Do you want to proceed anyways?" )
                if not answer:
                    return

            query = session.query( dbtools.Result.event_class ).distinct() \
                    .filter( dbtools.Result.hash==compare_scan.hash )

            event_classes = [ result[ 0 ] for result in query ]
            N_before_filtering = len( event_classes )

            event_classes = super_filter( event_classes, inclusion_patterns=conf.filter,
                    exclusion_patterns=conf.veto )

            print( "Processing %d / %d event classes." % ( len( event_classes ),
                    N_before_filtering ) )

            insert = dbtools.CorrectedResult.__table__.insert().prefix_with( "OR REPLACE" )

            for event_class in event_classes:
                start_time = time.time()

                query = session.query( dbtools.Result.round, dbtools.Result.score ) \
                        .filter( dbtools.Result.hash==compare_scan.hash, dbtools.Result.event_class==event_class )

                pseudo = list( query )
                if not pseudo:
                    continue

                pseudo = np.array( pseudo )

                pseudo_p_values = pseudo[:,1]
                pseudo_round_indices = pseudo[:,0]

                if destination_scan == compare_scan:
                    corrected_p_values = get_pseudo_p_tilde_list( pseudo_p_values, correct_if_zero=False )
                    corrected_tuples = zip( pseudo_round_indices, corrected_p_values )
                else:
                    query = session.query( dbtools.Result ) \
                            .filter( dbtools.Result.hash==destination_scan.hash, dbtools.Result.event_class==event_class )
                    corrected_tuples = [ ( result.round, calc_p_tilde( pseudo_p_values, result.score, correct_if_zero=False ) ) for result in query ]

                    if not corrected_tuples:
                        print( "No results for EC %s, skipping..." % event_class )
                        continue

                mappings = [
                    { "hash": destination_scan.hash,
                      "event_class": event_class,
                      "round": round,
                      "compared_hash": compare_scan.hash,
                      "score": corrected_p_value,
                      "comp_count": len( pseudo_p_values ),
                      } for round, corrected_p_value in corrected_tuples
                ]
                assert mappings

                session.execute( insert, mappings )

                session.flush()
                session.commit()

                end_time = time.time()
                print( event_class, ( end_time - start_time ) )


def prompt( text ):
    while True:
        print( text, end=" [yes/no]: " )
        yes = { "yes", "y" }
        no = { "no", "n" }
        choice = raw_input().lower().strip()
        if choice in yes:
            return True
        elif choice in no:
            return False
        else:
            print( "Please respond with 'yes' or 'no'." )

def parse_arguments():
    parser = argparse.ArgumentParser( description="Calculate p-tilde values on results from a database" )

    parser.add_argument( "--filter", help="Only process event classes that match this filter",
                        type=str, default=('*',), nargs="*" )
    parser.add_argument( "--veto", help="Veto event classes matching this pattern",
                        type=str, default=tuple(), nargs="*" )
    parser.add_argument( "--database", help="Path to SQLite database to work on",
                        type=str, default=dbtools.default_db_name )
    parser.add_argument( "compare_hash", help="Hash to compare to (usually BG MC)",
                        type=str )
    parser.add_argument( "destination_hash", help="p-values that will be corrected",
                        type=str, nargs="+" )
    return parser.parse_args()

if __name__=="__main__":
    main()
