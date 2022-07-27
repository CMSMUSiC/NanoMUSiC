from __future__ import print_function

import argparse
from collections import defaultdict
import json

import sqlalchemy

import dbtools

def main():
    """main function of the project."""

    conf = parse_arguments()

    engine = sqlalchemy.create_engine( 'sqlite:///' + conf.database, echo=False )
    dbtools.init_database( engine )

    with dbtools.session_scope( engine ) as session:
        generate_timing_dict( session, conf.json )


def generate_timing_dict( session, timing_dictname ):
        """ Generate "old-school" timing.json """

        view = dbtools.reflect_view( 'v_timing_overview', session.get_bind() )

        results = session.execute( view.select() )

        timings = defaultdict( dict )
        for distribution, event_class, avg_total in results:
            timings[ distribution ][ event_class ] = avg_total

        for distribution, distribution_timing in timings.items():
            with open( timing_dictname + "_" + distribution + ".json", 'w' ) as file:
                json.dump( distribution_timing, file, indent=2 )


def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""

    parser = argparse.ArgumentParser( description="Generate timing-json from database" )

    parser.add_argument( "--database", help="Path to SQLite database to be used",
                                type=str, default=dbtools.default_db_name )
    parser.add_argument( "--json", default="timing",
                                help="Filename for timing information")

    return parser.parse_args()

if __name__=="__main__":
    main()
