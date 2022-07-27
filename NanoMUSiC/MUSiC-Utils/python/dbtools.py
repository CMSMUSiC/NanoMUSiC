from __future__ import print_function
import os, os.path
import sys
import sqlite3

from argparse import Namespace
from collections import defaultdict
from contextlib import contextmanager
import heapq
import json
import math

import numpy as np

from commandregister import CommandRegister
from ectools.register import ecroot, ecstats, ecstyle

import sqlalchemy

from sqlalchemy import Table, Column, ForeignKey, ForeignKeyConstraint, \
    Integer, String, Float, DateTime, Boolean, \
    MetaData, and_, or_, event

from sqlalchemy.engine import Engine
from sqlalchemy.orm import relationship
from sqlalchemy.ext.declarative import declarative_base

Base = declarative_base()
Session = sqlalchemy.orm.sessionmaker()
default_db_name = "scans.sqlite"

#@event.listens_for(Engine, "connect")
def set_sqlite_pragma(dbapi_connection, connection_record):
    pragmas = {
        "page_size": 4096,
        "cache_size": 10000,
        "automatic_index": True,
        #"locking_mode": "EXCLUSIVE",
        "synchronous": "NORMAL",
        "journal_mode": "WAL",
        "temp_store": "MEMORY"
    }

    assert( type(dbapi_connection) is sqlite3.Connection )
    cursor = dbapi_connection.cursor()
    for key, value in pragmas.iteritems():
        # watch out for SQL injection, dont let anyone from outside decide key or value
        cursor.execute( "PRAGMA %s=%s" % ( key, str(value ) ) )
    cursor.close()

# from http://docs.sqlalchemy.org/en/latest/orm/session_basics.htmlhttp://docs.sqlalchemy.org/en/latest/orm/session_basics.html
from contextlib import contextmanager
@contextmanager
def session_scope( engine ):
    """Provide a transactional scope around a series of operations."""
    Session.configure( bind=engine )
    session = Session()
    try:
        yield session
        session.commit()
    except:
        session.rollback()
        raise
    finally:
        session.close()


# Table definition
class Scan( Base ):
    __tablename__ = 'scans'

    hash = Column( String, primary_key=True )
    name = Column( String )
    date = Column( DateTime )
    distribution = Column( String )
    scan_type = Column( String )

    results = relationship( 'Result', back_populates='scan', cascade="all" )
    timings = relationship( 'Timing', back_populates='scan', cascade="all" )
    stats = relationship( 'Stat', back_populates='scan', cascade="all" )
    corrected_results = relationship( 'CorrectedResult', back_populates='scan', cascade="all", foreign_keys="[CorrectedResult.hash]" )


class Result( Base ):
    __tablename__ = 'results'

    hash = Column( String, ForeignKey( 'scans.hash' ), primary_key=True )
    event_class = Column( String, primary_key=True )
    round = Column( Integer, primary_key=True )
    roi_lower_edge = Column( Float )
    roi_width = Column( Float )
    mc_events = Column( Float )
    mc_uncert = Column( Float )
    data_events = Column( Float )
    ec_total_mc_events = Column( Float )
    ec_total_mc_uncert = Column( Float )
    ec_total_data_events = Column( Float )
    score = Column( Float )
    integral = Column( Boolean )
    skipped = Column( Boolean )

    scan = relationship( 'Scan', foreign_keys=hash )
    corrected = relationship( 'CorrectedResult', back_populates='result' )


class Timing( Base ):
    __tablename__ = 'timings'

    hash = Column( String, ForeignKey( 'scans.hash' ), primary_key=True )
    event_class = Column( String, primary_key=True )
    timing_type = Column( String, primary_key=True )
    first_round = Column( Integer, primary_key=True )
    count = Column( Integer )
    total = Column( Float )

    scan = relationship( 'Scan', foreign_keys=hash )


class Stat( Base ):
    __tablename__ = 'stats'

    hash = Column( String, ForeignKey( 'scans.hash' ), primary_key=True )
    event_class = Column( String, primary_key=True )
    stat_type = Column( String, primary_key=True )
    first_round = Column( Integer, primary_key=True )
    value = Column( Integer )

    scan = relationship( 'Scan', foreign_keys=hash )


class CorrectedResult( Base ):
    __tablename__ = 'corrected_results'

    hash = Column( String, ForeignKey( 'scans.hash' ), primary_key=True )
    event_class = Column( String, primary_key=True )
    round = Column( Integer, primary_key=True )
    compared_hash = Column( String, ForeignKey( 'scans.hash' ) )
    score = Column( Float )
    comp_count = Column( Integer )

    scan = relationship( 'Scan', foreign_keys=( hash, ) )
    compared_scan = relationship( 'Scan', foreign_keys=( compared_hash, ) )
    result = relationship( 'Result', foreign_keys=( hash, event_class, round ) )

    __table_args__ = (
        ForeignKeyConstraint( [ 'hash', 'event_class', 'round' ],
            [ 'results.hash', 'results.event_class', 'results.round' ] ),
    )


def init_database( engine ):
    Base.metadata.create_all( engine )

    with session_scope( engine ) as session:
        # Here we do some maintenance on the database:
        # Dropping views and recreating them. Thus we are always working
        # on the most up-to-date definition of the views, as executed below.


        # Although there seems to be a solution of generating views in an
        # SQLAlchemy-style ( http://stackoverflow.com/a/9769411/489345 ),
        # I decided (for now) that it's more readable to use raw SQL in this
        # case.
        session.execute( "DROP VIEW IF EXISTS v_timing_overview" )
        session.execute( """CREATE VIEW v_timing_overview AS
            SELECT distribution,
                   event_class,
                   Sum(sum_types) / Count(*) AS avg_hashes
            FROM  (SELECT hash,
                          distribution,
                          event_class,
                          Sum(avg) AS sum_types
                   FROM   (SELECT hash,
                                  event_class,
                                  timing_type,
                                  Sum(total) / Sum(count) AS avg
                           FROM   timings
                           GROUP  BY hash,
                                     event_class,
                                     timing_type)
                          JOIN scans using (hash)
                   GROUP  BY hash,
                             event_class)
            GROUP  BY distribution,
                      event_class
            """ )

        session.execute( "DROP VIEW IF EXISTS v_hash_overview" )
        session.execute( """CREATE VIEW v_hash_overview AS
            SELECT
                hash,
                name,
                date,
                distribution,
                scan_type,
                COUNT(DISTINCT event_class) AS event_class_count,
                COUNT(DISTINCT round) AS round_count
            FROM results
            JOIN scans USING (hash)
            GROUP BY hash
            """ )

        session.execute( "DROP VIEW IF EXISTS v_ptilde" )
        session.execute( """CREATE VIEW IF NOT EXISTS v_ptilde AS
            SELECT
                hash,
                name,
                distribution,
                event_class,
                round,
                roi_lower_edge,
                roi_width,
                mc_events,
                mc_uncert,
                data_events,
                ec_total_mc_events,
                ec_total_data_events,
                results.score AS p,
                corrected_results.score AS ptilde,
                comp_count,
                integral,
                skipped
            FROM corrected_results
            JOIN scans USING (hash)
            JOIN results USING (hash, event_class, round)
            """ )

# http://docs.sqlalchemy.org/en/latest/core/reflection.html#reflecting-views
def reflect_view( viewname, engine ):
    metadata = MetaData( bind=engine )
    view = Table( viewname, metadata, autoload=True )
    return view

def copy_database( source_engine, destination_engine, chunksize=10000, ignore_existing=True ):
    with session_scope( source_engine ) as source_session:
        with session_scope( destination_engine ) as destination_session:
            for name, table in Base.metadata.tables.items():
                print( "Copying table %s..." % table )

                table.create( destination_engine, checkfirst=True )

                insert = table.insert()
                if ignore_existing:
                    insert = insert.prefix_with( "OR IGNORE" )

                N = source_session.query( table ).count()

                offset = 0
                while offset < N:
                    print( "Progress: %d / %d" % ( offset, N ) )

                    query = source_session.query( table ) \
                            .offset( offset ).limit( chunksize )
                    offset += chunksize

                    rows = [ row._asdict() for row in query ]
                    if not rows:
                        break

                    destination_session.execute( insert, rows )
                    destination_session.flush()
                    destination_session.commit()

class AmbigousHashException(Exception):
    pass

class NoHashFoundException(ValueError):
    pass

def match_hash( hash,
                session,
                multiple=False,
                allow_name=True,
                scan_type=None,
                distribution=None ):
    filters = [ Scan.hash.like( hash + '%' ) ]
    criteria = []
    if allow_name:
        filters.append( Scan.name == hash )
    if scan_type is not None:
        criteria.append(Scan.scan_type == scan_type)
    if distribution:
        criteria.append(Scan.distribution == distribution)
    query = session.query(Scan).filter(and_(or_(*filters), *criteria))

    results = query.all()
    if not results:
        raise NoHashFoundException( "No matching hash found for query '%s'" % hash )
    if multiple:
        return results
    else:
        if len( results ) > 1:
            raise AmbigousHashException( "Ambiguous hash used for query without multiple option.")
        return results[ 0 ]



def collect_p_tilde_list_class(hash, event_class, session, log10=True):
    query = session.query( CorrectedResult.score.label("score"),
                       CorrectedResult.comp_count.label("comp_count") ) \
                    .filter_by( hash=hash, event_class=event_class )

    pseudo_values = np.array([ r.score if r.score > 0 else 1.0/r.comp_count for r in query ])

    if log10:
        log_p_tilde_values = -np.log10(pseudo_values)
    else:
        log_p_tilde_values = list(pseudo_values)
    return list(log_p_tilde_values)

def get_scan_type(hash , session):
    query = session.query( Scan.scan_type.label("scan_type") ).filter( Scan.hash == hash)
    results = query.all()
    return results[0].scan_type

def collect_p_tilde_list(hash, event_classes, session, assert_nscans=True):
    log_p_tilde_lists = []
    print("Collecting pseudo ptilde values for hash '%s'..." % hash)
    check_len = None
    for event_class in event_classes:
        pseudo_values = collect_p_tilde_list_class(hash,
                                                   event_class,
                                                   session)
        if not pseudo_values:
            continue
        if check_len is None or not assert_nscans:
            check_len = len(pseudo_values)
        else:
            try:
                assert( check_len == len( pseudo_values ) )
            except AssertionError:
                print (event_class, check_len, len( pseudo_values ))
                sys.exit(1)
        pseudo_values[0] - 1
        try:
            assert( all(p >= 0 for p in pseudo_values) )
        except AssertionError:
            print("Negative values in pseudo values list")
            print(pseudo_values[0], pseudo_values[0] - 1)
            for i,p in enumerate(pseudo_values):
                if p < 0:
                    print (i, p)
            sys.exit(1)
        log_p_tilde_lists.append( pseudo_values )

    return log_p_tilde_lists

def open_database(filename, create=False, **kwargs):
    if not os.path.exists(filename) and not create:
        raise IOError("Database file %s does not exist." % filename)

    return sqlalchemy.create_engine( 'sqlite:///' + filename, **kwargs )


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

class Sdb:
    cmd = CommandRegister()

    def __init__( self, database=default_db_name, dbtype = 'sqlite:///' ):
        self.engine = sqlalchemy.create_engine( dbtype + database, echo=False )

    @cmd.command
    def init( self ):
        """ (Re)initialize a database according to  """
        init_database( self.engine )

    @cmd.command
    def list( self, header=True ):
        """ List all distinct scan hashes in database. """
        with self.session() as session:
            if header:
                print( "{:<32s} {:<9s} {:<15s} {:s}".format( "Hash", "Distr.", "Type", "Name" ) )
            for result in session.query( Scan ).order_by( Scan.name ):
                print( "{hash:<32s} {distribution:<9s} {scan_type:<15s} {name:s}".format( **result.__dict__ ) )

    @cmd.command
    def list_compared( self ):
        """ List all distinct p-tilde comparisons stored in the database. """
        with self.session() as session:
            query = session.query( CorrectedResult.compared_hash, CorrectedResult.hash ) \
                           .distinct( CorrectedResult.compared_hash, CorrectedResult.hash )

            retval = []
            for cmp, dst in query:
                cmp_scan = session.query( Scan ).get( cmp )
                dst_scan = session.query( Scan ).get( dst )

                print(dst_scan.name, dst_scan.hash, cmp_scan.name, cmp_scan.hash)
                retval.append( ( cmp_scan.hash, dst_scan.hash ) )

            return retval

    @cmd.command
    def list_ec( self, ec_name, hash, round=0 ):
        """ List all distinct p-tilde comparisons for a single class stored in the database. """
        with self.session() as session:
            result = session.query( Result.event_class, Result.score) \
                             .filter( Result.hash==hash,
                                      Result.event_class==ec_name,
                                      Result.round==round ).first()
            corrected_results = session.query( CorrectedResult.compared_hash,
                                              CorrectedResult.hash,
                                              CorrectedResult.comp_count,
                                              CorrectedResult.score ) \
                                              .filter( CorrectedResult.hash==hash,
                                                       CorrectedResult.round==round,
                                                       CorrectedResult.event_class == ec_name ).all()
            #~ query = session.query( CorrectedResult.compared_hash, CorrectedResult.hash ) \
                           #~ .filter_by( Result.hash=hash, Result.round=round )
            for corrected_result in corrected_results:
                print(result.event_class,
                      result.score,
                      corrected_result.score,
                      corrected_result.compared_hash,
                      corrected_result.comp_count)

            return 0

    @cmd.command
    def rename( self, hash, name ):
        """ Set name for scan hash (used e.g. in plot legends). """
        with self.session() as session:
            scan = match_hash( hash, session )
            scan.name = name
            session.flush()
            session.commit()

    @cmd.command
    def auto_name( self ):
        """ Auto calculate names from ECScanResultWrite names """
        is_valid = lambda p: (p not in (".", "..")) and ("jsonArch" not in p) and ("grid-ce.physik.rwth-aachen.de" not in p)

        with self.session() as session:
            query = session.query( Scan )
            for scan in query:
                if '/' not in scan.name:
                    continue
                parts = scan.name.split("/")
                parts = filter(is_valid, parts)
                new_name = ",".join( parts )
                print("Rename: %s (%s) => %s" % (scan.name, scan.hash, new_name))
                scan.name = new_name
            session.flush()
            session.commit()

    @cmd.command
    def overview( self ):
        """ Show number of event classes and rounds for scans. """
        with self.session() as session:
            view = reflect_view( "v_hash_overview", session.get_bind() )
            print( "{:<50s} {:<12s} {:<12s}".format( "Name", "EC count", "Round count" ) )
            for result in session.query( view ):
                print( "{name:<50s} {event_class_count:<12d} {round_count:<12d}".format( **result.__dict__ ) )


    @cmd.command
    def timing( self, filename="timing" ):
        """ Generate timing dictionaries from database. """
        with self.session() as session:
            view = reflect_view( 'v_timing_overview', session.get_bind() )

            results = session.execute( view.select() )

            timings = defaultdict( dict )
            for distribution, event_class, avg_total in results:
                timings[ distribution ][ event_class ] = avg_total

            for distribution, distribution_timing in timings.items():
                with open( filename + "_" + distribution + ".json", 'w' ) as file:
                    json.dump( distribution_timing, file, indent=2 )

    @cmd.command
    def slow( self, distribution="SumPt", n=10 ):
        """ Show slowest event classes. """
        with self.session() as session:
            view = reflect_view( 'v_timing_overview', session.get_bind() )
            query = session.query( view.c.event_class, view.c.avg_hashes ) \
                           .filter( view.c.distribution==distribution ) \
                           .order_by( sqlalchemy.desc( view.c.avg_hashes ) ) \
                           .limit( n )
            print( "{:<50s} {:<12s}".format( "Event Class", "Time per Round" ) )
            for result in query:
                print( "{:<50s} {:<12.3f}".format( *result ) )


    @cmd.command
    def drop( self, hash ):
        """ Drop scan (and its results) from the database. """
        with self.session() as session:
            scan = match_hash( hash, session )
            confirmed = prompt( "Do you really want to drop scan \"{name:s}\" ({hash:s}) " \
                              "and all its results from the database? This cannot be undone.".format( name=scan.name, hash=scan.hash) )
            if not confirmed:
                return

            session.delete( scan )
            session.flush()
            session.commit()

    @cmd.command
    def drop_ptilde( self, hash ):
        """ Drop corrected results for scan from the database based on compare score hash. """
        with self.session() as session:
            scan = match_hash( hash, session )
            confirmed = prompt( "Do you really want to drop CorrectedResults for scan \"{name:s}\" ({hash:s}) ".format( name=scan.name, hash=scan.hash) )
            if not confirmed:
                return

            query = session.query(CorrectedResult).filter(CorrectedResult.compared_hash == hash)
            for cr in list(query):
                session.delete(cr)
            session.flush()
            session.commit()

    @cmd.command
    def optimize( self ):
        """ Analyze the database content for faster queries. """
        # This allows the query planner to know whether to create auto indices, etc.
        self.engine.execute( "ANALYZE" )

    @cmd.command
    def vacuum( self ):
        """ Rebuild database for smaller file size and increased performance. Takes a long time. """
        # The database must not be accessed at the same time.
        # It will be rebuilt from scratch, keeping its content.
        self.engine.execute( "VACUUM" )

    @cmd.command
    def significant( self, hash,
                     n=10,
                     mode="median",
                     corrected=True,
                     table_style=None,
                     return_rows=False,
                     offset=0,
                     include_skipped=0,
                     invert_order=False,
                     ):
        """ Show most significant classes. """
        with self.session() as session:
            scan = match_hash( hash, session )

            def _get_results( event_class, table, order, offset=0, include_skipped=0 ):
                query = session.query( table ) \
                        .filter_by( hash=scan.hash, event_class=event_class, skipped=include_skipped ) \
                        .order_by( *order ) \
                        .offset(offset)

                N = query.count()

                if N == 0:
                    return None

                if mode=="median":
                    offset = int( N/2. )
                elif mode=="max":
                    offset = N-1
                elif mode=="min":
                    offset = 0
                else:
                    raise ValueError( "Illegal mode: %r" % mode )

                result = query.offset( offset ).limit( 1 ).first()

                return result

            if corrected:
                view = reflect_view( 'v_ptilde', session.get_bind() )
                result_func = lambda ec: _get_results( ec, view, ( view.c.ptilde, view.c.p ) )
                sort_func = lambda result: ( result.ptilde,
                                             result.p,
                                             result.roi_width
                                            - abs(result.data_events-result.mc_events)/result.mc_uncert )

            else:
                result_func = lambda ec: _get_results( ec, Result, ( Result.score, ) )
                sort_func = lambda result: ( result.score,
                                             result.roi_width,
                                           - abs(result.data_events-result.mc_events)/result.mc_uncert )

            event_classes_query = session.query( Result.event_class ) \
                .filter_by( hash=scan.hash ).distinct()

            results = filter( None, ( result_func(ec) for ec, in event_classes_query ) )
            if not results:
                print("No results found.")
                return

            if invert_order:
                top = heapq.nlargest( n, results, key=sort_func )
            else:
                top = heapq.nsmallest( n, results, key=sort_func )

            top.sort( key=sort_func )

            if return_rows:
                header = ["event_class", "round", "roi_start", "roi_end", "mc_events", "sys_uncert", "data_events", "p", "p-tilde" ]
            else:
                header = ["Event Class", "Round", "RoI Start", "RoI End", "MC Events", "Sys. Uncert.", "Data Events", "p", "p-tilde" ]

            if corrected:
                header += ["p-tilde"]
                rows = [ ( r.event_class, r.round, int(r.roi_lower_edge), int(r.roi_lower_edge+r.roi_width), r.mc_events, r.mc_uncert, int( r.data_events ), r.p, r.ptilde, ) \
                            for r in top ]
            else:
                rows = [ ( r.event_class, r.round, int(r.roi_lower_edge), int(r.roi_lower_edge+r.roi_width), r.mc_events, r.mc_uncert, int( r.data_events ), r.score ) \
                            for r in top ]

            if return_rows:
                return rows, header

            if table_style is None:
                # fields: EC, Round, Start, End, MC +- Uncert, Data, p
                header_fmt = "{:<40s} {:>7s} {:>11s} {:>11s} {:>13s} +- {:<13s} {:>11s} {:>9s}"
                row_fmt =    "{:<40s} {:>7d} {:>11.0f} {:>11.0f} {:>13.02g} +- {:<13.02g} {:>11d} {:>9.02g}"

                if corrected:
                    # add format fields for ptilde
                    header_fmt += " {:>9s}"
                    row_fmt += " {:>9.02g}"

                print( header_fmt.format( *header ) )
                for row in rows:
                    print( row_fmt.format( *row ) )

            else:
                if table_style == "html":
                    try:
                        from tabulate import tabulate
                    except ImportError:
                        raise ImportError("Module 'tabulate' is required for pretty tables. "
                                          "Install it using 'pip --user install tabulate'!")


                        html_func = lambda name: ecstyle.latex_ec_name( name, style="html" )
                        rows = [ tuple( [html_func(r[0])] + list(r[1:]) ) for r in rows ]
                    print( tabulate(rows, header, tablefmt=table_style ) )


    @cmd.command
    def calc_ptilde( self,
                     compare_hash,
                     destination_hash,
                     filter=("*", ),
                     veto=(),
                     filter_deviation_type="" ):
        """Calculate p-tilde value comparing results for hash one with hash two
           (e.g. mc_hash mc_hash, mc_hash data_hash)"""
        with self.session() as session:
            compare_scan = match_hash( compare_hash, session )

            if compare_scan.scan_type != 'pseudoScan':
                print( "The scan type is '%s' which seems not to be a standard BG/BG scan." % compare_scan.scan_type )
                answer = prompt( "Do you want to proceed anyways?" )
                if not answer:
                    return

            destination_scan = match_hash( destination_hash, session )

            if compare_scan.distribution != destination_scan.distribution:
                print( "The distribution types '%s' and '%s' seem not to match." % ( compare_scan.distribution, destination_scan.distribution ) )
                answer = prompt( "Do you want to proceed anyways?" )
                if not answer:
                    return

            query = session.query( Result.event_class ).distinct() \
                    .filter( Result.hash==compare_scan.hash )

            event_classes = [ result[ 0 ] for result in query ]
            N_before_filtering = len( event_classes )

            print( "Processing %d / %d event classes." % ( len( event_classes ),
                    N_before_filtering ) )

            insert = CorrectedResult.__table__.insert().prefix_with( "OR REPLACE" )

            for i, event_class in enumerate( event_classes ):
                print( "Progress: %d / %d" % ( i, len( event_classes ) ) )

                filter_args = [Result.hash==compare_scan.hash, Result.event_class==event_class]
                if filter_deviation_type:
                    if filter_deviation_type == "excess":
                        filter_args.append(Result.mc_events < Result.data_events)
                    if filter_deviation_type == "deficit":
                        filter_args.append(Result.mc_events > Result.data_events)

                query = session.query( Result.round, Result.score ) \
                        .filter( *filter_args )

                pseudo = list( query )
                if not pseudo:
                    continue

                pseudo = np.array( pseudo )

                pseudo_p_values = pseudo[:,1]
                pseudo_round_indices = pseudo[:,0]

                if destination_scan == compare_scan:
                    corrected_p_values = ecroot.get_pseudo_p_tilde_list( pseudo_p_values, correct_if_zero=False )
                    corrected_tuples = zip( pseudo_round_indices, corrected_p_values )
                else:
                    query = session.query( Result ) \
                            .filter( Result.hash==destination_scan.hash, Result.event_class==event_class )

                    if query.count() == 0:
                        print( "No results for EC %s, skipping..." % event_class )
                        continue

                    corrected_tuples = [ ( result.round, ecroot.calc_p_tilde( pseudo_p_values, result.score, correct_if_zero=False ) ) for result in query ]


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

    @cmd.command
    def auto_calc_ptilde( self, dry=False ):
        """ Iterate through all scans and calculate p-tilde where necessary,
            comparing pseudoScans to signalScans. Distributions must match
            and bJet must be either in both names or none (this is specific
            for JLs master thesis. """

        done = self.list_compared()

        with self.session() as session:
            pseudo_scans = session.query( Scan ) \
                            .filter( Scan.scan_type == "pseudoScan" )
            signal_scans = session.query( Scan ) \
                            .filter( Scan.scan_type == "signalScan" )

            # fetch results
            pseudo_scans = list( pseudo_scans )
            signal_scans = list( signal_scans )

            compare_scans = pseudo_scans
            destination_scans = pseudo_scans + signal_scans

            for compare_scan in compare_scans:
                for destination_scan in destination_scans:
                    if ( compare_scan.hash, destination_scan.hash ) in done:
                        continue

                    if compare_scan.distribution == destination_scan.distribution:
                        if ("bJets" in compare_scan.name and "bJets" not in destination_scan.name) or ("bJets" in destination_scan.name and "bJets" not in compare_scan.name):
                            continue

                        print("Comparing %s to %s..." % ( destination_scan.name, compare_scan.name ))
                        if not dry:
                            self.calc_ptilde( compare_scan.hash, destination_scan.hash )


    @cmd.command
    def lookup_ptilde( self, hash, event_class, p ):
        """ Calculate p-tilde from a single given p-value, a hash and an EC.
            The hash and EC are used for fetching the compare-scan from the DB.
            All fetched p-values are then compared to the given p, in order
            to calculate a p-tilde value. """
        p = float( p )

        with self.session() as session:
            scan = match_hash( hash, session )

            if scan.scan_type != "pseudoScan":
                print( "Warning: You are comparing to scan of type '%s'." % scan.scanType )
                print( "p-tilde values are usually calculated from pseudoScans." )

            query = session.query( Result ) \
                           .filter_by( hash=scan.hash, event_class=event_class )

            N = query.count()
            if N == 0:
                print( "No scans found for this hash/event-class combination." )
                return

            n_smaller = query.filter( Result.score <= p ).count()
            if n_smaller == 0:
                print( "p-tilde is less than 1/%d = %g" % ( N, 1.0/N ) )
            else:
                print( "p-tilde: %d / %d = %g" % ( n_smaller, N, float(n_smaller) / N ) )

    @cmd.command
    def compare_results( self, hash1, hash2, max=1.0 ):
        """ Compare the (uncorrected) results of two runs, EC by EC, round by round.
            Ignore p-values above max. """

        with self.session() as session:
            scan1 = match_hash( hash1, session )
            scan2 = match_hash( hash2, session )

            result1 = sqlalchemy.orm.aliased( Result )
            result2 = sqlalchemy.orm.aliased( Result )

            query = session.query( result1,
                                   result2,
                                   ( sqlalchemy.func.abs( result1.score-result2.score )/sqlalchemy.func.min( result1.score, result2.score )).label('rel_diff') ) \
                .join( ( result2, sqlalchemy.and_( result1.event_class==result2.event_class, result1.round==result2.round ) ) ) \
                .filter( result1.hash==scan1.hash, result2.hash==scan2.hash,
                         sqlalchemy.or_( result1.roi_lower_edge!=result2.roi_lower_edge,
                                        result1.roi_width!=result2.roi_width ),
                         sqlalchemy.or_( result1.score<max, result2.score<max ),
                         result1.skipped==False, result2.skipped==False,
                        ) \
                .order_by( 'rel_diff' )

            header = ("Event Class", "Round", "MC", "Uncert.", "Data", "p", "MC", "Uncert.", "Data", "p", "Rel. Diff (%)" )
            rows = [ ( r1.event_class, int(r1.round), r1.mc_events, r1.mc_uncert, int(r1.data_events), r1.score, \
                        r2.mc_events, r2.mc_uncert, int(r2.data_events), r2.score, rdiff ) for r1, r2, rdiff in query ]

            header_fmt = "{:<40s} {:>6s} | {:>9s} +- {:<9s} {:>6s} {:>9s} | {:>9s} +- {:<9s} {:>6s} {:>9s} | {:>9s}"
            row_fmt =    "{:<40s} {:>6d} | {:>9.02g} +- {:<9.02g} {:>6d} {:>9.04g} | {:>9.02g} +- {:<9.02g} {:>6d} {:>9.02g} | {:>9.04g}"
            print( header_fmt.format( *header ) )

            for row in rows:
                print( row_fmt.format( *row ) )

    @cmd.command
    def stats( self, hash ):
        """ Analyze content of stats-table (e.g. LUT hits/misses). """

        with self.session() as session:
            scan = match_hash( hash, session )

            # SELECT name, timing_type, SUM(total), SUM(count) FROM timings JOIN scans USING (hash) GROUP BY hash, timing_type
            # SELECT name, stat_type, SUM(value) FROM stats JOIN scans USING (hash) GROUP BY hash, stat_type

            timing_query = session.query( sqlalchemy.func.sum( Timing.total ) ) \
                                  .group_by( Timing.hash, Timing.timing_type ) \
                                  .filter( Timing.hash == scan.hash,
                                           Timing.timing_type == 'roiFinding' )

            time = timing_query.first()[0]
            print( "Total RoI finding time: %.1f sec = %.1f min = %.1f h" % ( time, time/60.0, time/3600.0 ) )

            stat_query = session.query( Stat.stat_type, sqlalchemy.func.sum( Stat.value ) ) \
                                  .group_by( Stat.hash, Stat.stat_type ) \
                                  .filter( Stat.hash == scan.hash )

            results = dict( stat_query )

            print("== RAW STATS ==")
            for key in sorted(results.keys()):
                value = int(results[key])
                print("{:>30s}: {:>12d}".format(key, value))
            print("===============")

            try:
                hit = results["lut: hit"]
                miss = results["lut: miss"]
                hit_ratio = float(hit)/(hit+miss)
                print( "LUT Hit percentage: %.1f %%" % ( 100.*hit_ratio ) )
            except KeyError:
                # no LUT info
                pass

    @cmd.command
    def p_hist( self, event_class, hashes=[], corrected=False ):
        """ Create a histogram of p/p-tilde values for a given event_class. """

        assert hashes

        with self.session() as session:
            for hash in hashes:
                scan = match_hash( hash, session )
                if corrected:
                    query = session.query( CorrectedResult.score ) \
                        .filter( CorrectedResult.hash==hash, CorrectedResult.event_class==event_class )
                else:
                    query = session.query( Result.score ) \
                        .filter( Result.hash==hash, Result.event_class==event_class )
                values = [ result[0] for result in query ]
                assert values

                N = len(values)
                range = (0, 1 - math.pow(0.01, 1./(N-1)))
                print(range)

                plt.hist( values, histtype="step", bins=20, label=scan.name, normed=True, range=range )

                p = np.linspace( range[0], range[1], 1000 )
                y = N * np.power( 1-p, N-1 )
                plt.plot( p, y, '-' )

            plt.title( event_class )
            plt.xlim( *range )

            if corrected:
                plt.xlabel( "p-tilde" )
            else:
                plt.xlabel( "p" )
            plt.ylabel( "Number of Rounds" )
            #lgd = plt.legend( bbox_to_anchor=(0.5, -0.1), loc='upper center', ncol=1, frameon=False )
            #assert lgd
            plt.savefig( "plot.pdf", bbox_inches="tight" ) # bbox_extra_artists=(lgd, ), )

    @cmd.command
    def check( self, full=False ):
        """ Perform sanity check on the written results.
            --full also checks for orphaned entries, this is usually not necessary.
        """
        something_found = False

        with session_scope( self.engine ) as session:
            for hash,name in session.query( Scan.hash, Scan.name ):
                print( "Checking scan \"%s\"..." % name )
                query = session.query( Result.event_class,
                                            sqlalchemy.func.min( Result.round),
                                            sqlalchemy.func.max( Result.round ),
                                            sqlalchemy.func.count( Result.round )) \
                                   .filter( Result.hash==hash ) \
                                   .group_by( Result.hash, Result.event_class )
                for ec,min,max,count in query:
                    if min != 0:
                        something_found = True
                        print( "> Range: minimal round for event class %s is %d." % ( ec, min ) )
                    if count != (max-min)+1:
                        something_found = True
                        print( "> Range: not entire round range covered on event class %s (%d rounds between %d and %d)." % ( ec, count, min, max ) )

            if full:
                known_hashes = session.query( Result.hash ).distinct()
                known_hashes = set( result[ 0 ] for result in known_hashes )
                query = session.query( Result.hash, sqlalchemy.func.count( Result.score ) ) \
                            .group_by( Result.hash )


                print( "Checking for orphaned entries..." )
                for hash, count in query:
                    if not hash in known_hashes:
                        print("> Orphan found: %d results for unknown hash %s" % ( count, hash ) )

        if not something_found:
            print("Check finished without any problems")
        else:
            print("Some problems found, see message above")

    @cmd.command
    def merge( self, other_database, destination_database="" ):
        other_sdb = Sdb( other_database )

        if destination_database:
            destination_sdb = Sdb( destination_database )
            destination_sdb.init()
            copy_database( self.engine, destination_sdb.engine )

            copy_database( other_sdb.engine, destination_sdb.engine )
        else:
            copy_database( other_sdb.engine, self.engine )

    @cmd.command
    def scan_info( self, name=None, hash=None):
        if name is None and hash is None:
            raise ValueError("Either scan name or hash needs to be set")
        filters = []
        if name is not None:
            filters.append(Scan.name == name)
        if hash is not None:
            filters.append(Scan.hash == hash)
        with session_scope( self.engine ) as session:
            query = session.query( Scan.hash.label("hash"),
                           Scan.name.label("name"),
                           Scan.distribution.label("distribution"),
                           Scan.scan_type.label("scan_type") )\
                           .filter( *filters)
        return [r for r in query]


    @contextmanager
    def session( self ):
        with session_scope( self.engine ) as session:
            yield session
