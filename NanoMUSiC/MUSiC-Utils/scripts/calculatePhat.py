#!/bin/env python
from __future__ import print_function

import argparse
from collections import defaultdict, OrderedDict
import functools
import gzip
import json
import logging
import math
import multiprocessing
import os, os.path
import re
import sys
import traceback

import numpy as np
import matplotlib.pyplot as plt

import scipy.stats
import sqlalchemy
import pandas as pd

from ectools.register import ecroot, ecstyle
import dbtools

from ECFinalPlot import set_skip_reason

from shellhelpers import tmp_chdir


pd.options.mode.chained_assignment = None  # default='warn'
pd.options.display.width = 140

log = logging.getLogger("phat")



def protectmath( func ):
    @functools.wraps(func)
    def wrapper( *args, **kwargs ):
        try:
            return func( *args, **kwargs )
        except ArithmeticError, ValueError:
            log.exception("Math exception:")
            return np.nan
    return wrapper

def main():
    conf = parse_arguments()
    logging.basicConfig(format="%(asctime)-15s %(levelname)-6s: %(message)s", level=logging.DEBUG)
    log.info("Initialized logger.")

    engine = sqlalchemy.create_engine( 'sqlite:///' + conf.database, echo=False )

    store = Store(conf.cache_dir)

    total_results = OrderedDict()

    # Get name, real hash from DB
    conf.reference_hash, reference_name = get_meta(conf.reference_hash, engine)
    log.info("Working on reference: %s...", reference_name)

    # Lookup whether hash exists in cache
    if conf.reference_hash not in store:
        log.info("Not found in store, computing...")

        # Preprocess + analyze reference
        data_frame = preprocess(conf.reference_hash, engine, conf)
        reference_values = data_frame.ptilde

        store[conf.reference_hash] = run_stats_tests(data_frame, reference_values)

        # Store in cache
        store["reference_" + conf.reference_hash] = data_frame.ptilde

    # Read from cache
    log.info("Restoring reference from store...")
    total_results[reference_name] = store[conf.reference_hash]
    reference_values = store["reference_" + conf.reference_hash].ptilde

    for hash in conf.compare_hash:
        # Similar to reference: get name, real hash, lookup and insert in cache
        # if necessary, then read from cache.
        hash, name = get_meta(hash, engine)
        log.info("Working on %s...", hash)
        if hash not in store:
            log.info("Not found in store, computing...")
            data_frame = preprocess(hash, engine, conf)
            store[hash] = run_stats_tests(data_frame, reference_values)

        log.info("Restoring result from store...")
        total_results[name] = store[hash]

    log.info("Saving plots...")
    make_plots(total_results, alpha=conf.alpha, folder=conf.out)

    log.info("Done.")

def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""

    parser = argparse.ArgumentParser( description="Create MUSiC plots from ROOT files." )

    parser.add_argument( "--out", type=str, default="." )
    parser.add_argument( "--cache-dir", type=str, default="./cache" )
    parser.add_argument( "--min-yield", type=float, default=0.1 )
    parser.add_argument( "--max-p", type=float, default=1.0 )
    parser.add_argument( "--nrounds", type=int, default=1000000 )
    parser.add_argument( "--skip-filters", action="store_true" )
    parser.add_argument( "--alpha", type=float, default=0.05 )

    parser.add_argument( "--database", type=str, default=dbtools.default_db_name )
    parser.add_argument( "reference_hash", type=str )
    parser.add_argument( "compare_hash", type=str, nargs="*" )

    ecstyle.add_ec_standard_options( parser )

    return parser.parse_args()

class Store:
    def __init__(self, dir, compression=True):
        self.dir = dir
        self.compression = compression
        if not os.path.isdir(self.dir):
            os.makedirs(self.dir)

    def filename(self, name):
        name_clean = re.sub("[^A-Za-z0-9_-]", "_", name)
        filename = "cache_" + name_clean + ".csv"
        if self.compression:
            filename += ".gz"
        return os.path.join(self.dir, filename)

    def has(self, name):
        return os.path.exists(self.filename(name))

    def get(self, name):
        filename = self.filename(name)
        return pd.read_csv(filename, compression=("gzip" if self.compression else None), header=0)

    def put(self, name, data_frame):
        filename = self.filename(name)
        # For some reason, the "compression" argument is not recognized, so
        # we use the gzip module as our own compression layer.
        with gzip.open(filename, 'w') as archive:
            data_frame.to_csv(archive, header=True)

    def __contains__(self, name):
        return self.has(name)

    def __getitem__(self, name):
        return self.get(name)

    def __setitem__(self, name, value):
        self.put(name, value)

def get_meta(hash_or_name, db_engine):
    with dbtools.session_scope(db_engine) as session:
        scan = dbtools.match_hash(hash_or_name, session)
        return scan.hash, scan.name

def preprocess(hash, db_engine, conf):
    log.debug("Calculating skip reasons for %s...", hash)

    if not conf.skip_filters:
        with dbtools.session_scope(db_engine) as session:
            _, good = set_skip_reason(session, hash, min_yield=conf.min_yield)

    if not conf.skip_filters:
        log.info("After skip: %d valid event classes", len(good))

        if not good:
            raise ValueError("No good results found.")

        filtered, _ = ecstyle.filter_class_names(good, conf)
        log.info("After filter: %d valid event classes", len(filtered))

    data_frame = read_data_frame(hash, db_engine, max_p=conf.max_p, max_round=conf.nrounds)

    if not conf.skip_filters:
        data_frame = data_frame[data_frame.event_class.isin(filtered)]

    return data_frame

def read_data_frame(hash, db_engine, max_p=0.95, max_round=None):
    log.info("Reading PANDAS data frame...")

    db_view = dbtools.reflect_view( 'v_ptilde', db_engine )

    with dbtools.session_scope(db_engine) as session:
        query = session.query(
                    db_view.c.event_class,
                    db_view.c.round,
                    db_view.c.ec_total_mc_events,
                    db_view.c.data_events,
                    db_view.c.ptilde,
                    db_view.c.comp_count,
                ).filter(db_view.c.hash==hash, db_view.c.ptilde<max_p)

        if max_round is not None:
            query = query.filter(db_view.c.round < max_round)

        sql = query.statement

        log.debug("Query: %s", sql)

        data_frame = pd.read_sql(sql, db_engine)
        if len( data_frame ) == 0:
            raise ValueError("Empty data frame (no p-tilde values?).")

        log.info("Fetched %d corrected scan results.", len(data_frame))

    return data_frame

def run_stats_tests(data_frame, reference):
    log.info("Calculate test statistics...")

    results = []
    for round, data in data_frame.groupby( "round" ):
        if round % 100 == 0:
            log.debug("Round %d...", round)

        # Count number of classes in the overflow bin BEFORE correction
        ptilde_overflow = np.count_nonzero( data.ptilde==0.0 )

        # Correct for p==0
        data.loc[data.ptilde==0.0, "ptilde"] = 1.0 / ( data.comp_count + 1. )

        # Calculate test statistics
        result = {
            "round": round,
            "KS_uniform": scipy.stats.kstest( data.ptilde, "uniform" )[0],
            "KS_referenced": ks_2samp_wrapper( reference, data.ptilde )[0],
            "CVM": cvm_test_uniform( data.ptilde ),
            "AD_uniform": anderson_test_uniform( data.ptilde ),
            "AD_referenced": ad_2samp_wrapper( reference, data.ptilde ),
            "ChiSq_uniform": chisq_test_uniform( data.ptilde ),
            "ChiSq_referenced": chisq_2samp_wrapper( reference, data.ptilde ),
            "Likelihood": likelihood_test_uniform( reference, data.ptilde ),
            "Simple": simple_test( data.ptilde, threshold=0.1 ),
            "N": len( data.ptilde ),
            "Overflow": ptilde_overflow,
        }

        results.append(result)

    # Convert list-of-dicts to dataframe with index "round"
    results_df = pd.DataFrame(results)
    results_df.set_index("round")

    summary = str(results_df.describe(percentiles=[0.05, 0.32, 0.5, 0.68, 0.95]))
    log.debug("=== RESULT SUMMARY ===\n" + summary)

    return results_df


def make_plots(total_results, alpha=0.95, folder="."):
    with tmp_chdir(folder, create=True):
        plot_results( total_results, "KS_uniform", xlabel="Kolmogorov-Smirnov d", range=(0, 1.0), alpha=alpha )
        plot_results( total_results, "KS_referenced", xlabel="Kolmogorov-Smirnov d", range=(0, 1.0), alpha=alpha )
        plot_results( total_results, "CVM", xlabel="Cramer-von-Mises T", range=(0, 500.), alpha=alpha )
        plot_results( total_results, "AD_uniform", xlabel="Anderson-Darling A", range=(0, 30.), alpha=alpha )
        plot_results( total_results,"AD_referenced", xlabel="Anderson-Darling A", range=(-5, 1000 ), alpha=alpha )
        plot_results( total_results, "ChiSq_uniform", xlabel="Chi-Squared per DoF", range=(0, 3), alpha=alpha )
        plot_results( total_results, "ChiSq_referenced", xlabel="Chi-Squared per DoF", range=(0, 3), alpha=alpha )
        plot_results( total_results, "Likelihood", xlabel="-log10( Likelihood )", range=(0, 400), alpha=alpha )
        plot_results( total_results, "N", xlabel="Number of Event Classes", range=( 0, 2000 ), alpha=alpha )
        plot_results( total_results, "Simple", xlabel="Simple Test", range=( 0, 1.0 ), alpha=alpha )
        plot_results( total_results, "Overflow", xlabel="Number of Event Classes in Overflow Bin", range=( 0, 20 ), alpha=alpha )

# PLOTTING
def plot_results( total_results, test_name, xlabel=None, alpha=0.95, **kwargs ):
    log.info("Plotting %s", test_name)
    try:
        plt.clf()

        threshold = None
        distribution = None
        for key, data_frame in total_results.iteritems():
            values = data_frame[ test_name ]

            if len( values ) == 1:
                # for data, we only have ONE value
                plt.axvline( values[ 0 ], color="blue", ls="-", label=key )
            else:
                key_parts = key.split(",")
                key_parts = [ k.replace("_", " ") \
                               .replace("M-", "M=") \
                               .replace("SM", "Standard Model") \
                               .replace("signal", "") \
                               .replace("bJets", "") \
                               for k in key_parts ]

                key_parts = filter( None, key_parts )

                if distribution is None:
                    for dist_type in ecstyle.distribution_types():
                        if dist_type.name in key_parts:
                            distribution = dist_type.mpl_tag

                dist_type_names = [dist_type.name for dist_type in ecstyle.distribution_types()]
                key_parts = [ k for k in key_parts if k not in dist_type_names ]

                label = " ".join(key_parts)

                if threshold is None:
                    threshold = np.percentile(values, 100.*(1.-alpha))
                    log.debug("Threshold at %.1f %%: %g", 100.*alpha, threshold)
                    plt.axvline( threshold, color="gray", ls="--", label="%.1f %% percentile" % (100.*alpha))
                else:
                    beta = float(np.count_nonzero(values<threshold))/len(values)
                    power = 1.-beta
                    log.info("Power: %.1f %%", 100.*power)
                    label += " (power: %.1f %%)" % (100.*power)

                plt.hist( values, histtype="step", bins=30, label=label, normed=True, **kwargs )

        if xlabel:
            plt.xlabel( xlabel )

        plt.ylabel( "Number of Rounds" )
        plt.ylim( 0, plt.ylim()[ 1 ] )
        plt.xlim( 0, 1 )

        if distribution:
            plt.title( "Global Results for %s" % distribution )

        if "range" in kwargs:
            plt.xlim( *kwargs["range"] )

        #lgd = plt.legend( bbox_to_anchor=(0.5, -0.1), loc='upper center', ncol=1, frameon=False )
        lgd = plt.legend( loc='upper right' )

        plt.savefig( test_name + "_results.pdf", bbox_extra_artists=(lgd, ), bbox_inches="tight" )
    except Exception as e:
        log.exception("Exception while plotting.")


def steps( X, Y ):
    x = np.repeat( X, 2 )
    y = np.hstack( ( [ 0 ], np.repeat( Y, 2 ), [ 0 ] ) )
    return x, y

def plot_cum( reference, vals, title, filename ):
    plt.clf()
    #k = ks_test_uniform( vals, plot=True )
    ks_2samp_wrapper( reference, vals, plot=True )
    #plt.plot( [0, 1], [0, 1], color="red" )
    plt.xlim( -0.1, 1.1 )
    plt.ylim( -0.1, 1.1 )
    plt.xlabel( "p" )
    plt.ylabel( "Cumulative distribution" )
    plt.title( title )
    lgd = plt.legend( bbox_to_anchor=(0.5, -0.1), loc='upper center', ncol=1, frameon=False )
    plt.savefig( filename, bbox_extra_artists=(lgd, ), bbox_inches="tight" )

def plot_pdf( vals, title, filename ):
    plt.clf()
    plt.hist( vals, bins=20, range=(0, 1), histtype="step", color="black", normed=True )
    plt.xlim( 0, 1 )
    plt.xlabel( "p" )
    plt.ylabel( "Probability density distribution" )
    plt.title( title )
    plt.savefig( filename, bbox_inches="tight" )

def plot_cumulative( vals, *args, **kwargs ):
    vals = sorted( vals )
    X = np.empty( 2*len( vals ) )
    Y = np.empty_like( X )
    for i, p in enumerate( vals ):
        for offset in (0, 1):
            f = float( i + offset )/len( vals )
            X[2*i+offset] = p
            Y[2*i+offset] = f
    plt.plot(X, Y, *args, **kwargs)

# STATISTICAL TESTS

@protectmath
def simple_test( vals, threshold=0.1 ):
    N = len( vals )
    n = np.count_nonzero( vals < threshold )
    return float(n)/N
    #dist = scipy.stats.binom( N, threshold )
    #p = dist.pmf( n )
    #print( "Total:", N, "Exp:", threshold*N, " Obs:", n, "=> p=", p )
    #return -math.log10( p )

@protectmath
def likelihood_test_uniform( reference, vals ):
    nbins = 20
    bin_size = 1.0/nbins

    N = len(vals)
    expected = N*bin_size

    bins, bin_edges = np.histogram( vals, bins=nbins, range=(0,1), density=False )
    reference_bins, bin_edges = np.histogram( reference, bins=nbins, range=(0,1), density=True )

    binom = scipy.stats.binom

    p_lower = binom.cdf( N, bins[bins<=expected], reference_bins[bins<=expected] )
    p_higher = binom.sf( N, bins[bins>expected], reference_bins[bins>expected] )
    p_total = np.concatenate( ( p_lower, p_higher ) )

    log_p_total = -np.log10( p_total )

    # clamp to 1e-50
    log_p_total[log_p_total > 50] = 50

    return log_p_total.sum()

@protectmath
def chisq_test_uniform( vals ):
    nbins = 10
    hist, bin_edges = np.histogram( vals, bins=nbins, range=(0,1), density=True )
    return scipy.stats.chisquare( hist )[0] / nbins

@protectmath
def anderson_test_uniform( vals ):
    sorted_copy = np.sort( vals )
    n = len( vals )
    i = np.arange( 1, n+1 )
    lns = np.log( sorted_copy ) + np.log( 1. - sorted_copy[::-1] )
    arr = ( 2.*i - 1. ) * lns
    S = arr.sum() / n
    A = math.sqrt( -n - S )
    return A

@protectmath
def cvm_test_uniform( vals ):
    # Cramer-Von-Mises test
    sorted_copy = np.sort( vals )
    n = len( vals )
    i = np.arange( 1, n+1 )
    arr = np.power( ( 2.*i - 1. )/n - sorted_copy, 2. ) * (-1.*np.log( sorted_copy ))
    T = 1./(12.*n) + arr.sum()
    return T


def _2samp_wrapper( a, b, accuracy=0.1 ):
    if len( b ) > len( a ):
        a, b = b, a
    # a now contains the longer array
    skip = int( len( a )/len( b ) * accuracy )
    if skip > 0:
        a = a[::skip]
    return a, b

@protectmath
def ad_2samp_wrapper( a, b, accuracy=0.5 ):
    a, b = _2samp_wrapper( a, b, accuracy )
    return scipy.stats.anderson_ksamp( (a, b) )[ 0 ]

@protectmath
def ks_2samp_wrapper( a, b, accuracy=0.5, plot=False ):
    a, b = _2samp_wrapper( a, b, accuracy )
    if plot:
        plot_cumulative( a, label="Reference" )
        plot_cumulative( b, label="p-Values" )
    return scipy.stats.ks_2samp( a, b )

@protectmath
def chisq_2samp_wrapper( a, b, accuracy=0.5 ):
    nbins = 10
    hist_a, bin_edges_a = np.histogram( a, bins=nbins, range=(0,1), density=True )
    hist_b, bin_edges_b = np.histogram( b, bins=nbins, range=(0,1), density=True )
    assert (bin_edges_a == bin_edges_b).all()
    return scipy.stats.chisquare( hist_b, f_exp=hist_a )[0] / nbins

if __name__=="__main__":
    main()
