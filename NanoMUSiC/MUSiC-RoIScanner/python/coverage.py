from __future__ import print_function, division

import collections
import itertools
import json
import math
import multiprocessing
import os, os.path
import sys
import time
import traceback

import numpy as np
import scipy
import scipy.stats
import scipy.integrate

import jmpl
import matplotlib as mpl
import matplotlib.pyplot as plt
import colormaps

import pCalc
from commandregister import CommandRegister


class CoverageResult:
    def __init__(self, **kwargs):
        # Parameters
        self.n_true = None
        self.rel_err = None
        self.p_claim = None
        self.prior = None
        self.test_case = None

        # Derived parameters
        self.asymmetry_correction = None
        self.p_claim_corrected = None

        # Results
        self.mean_n_exp = None
        self.mean_n_obs = None

        self.n_toys = None
        self.n_nottested = None
        self.n_invalid = None
        self.n_significant = None
        self.n_insignificant = None

        self._load_kwargs(kwargs)

        if self.n_significant is not None:
            assert( ( self.n_invalid + self.n_significant + \
                self.n_insignificant + self.n_nottested ) == self.n_toys )

    def _load_kwargs(self, kwargs):
        for name, value in kwargs.items():
            if hasattr( self, name ):
                setattr( self, name, value )

    def _p2Z(self, p):
        return scipy.stats.norm.isf( p )

    @property
    def p_true(self):
        # Reduct all cases where the p-value algorithm was not able to return a
        # meaningful p-value. We just pretend those experiments never happened.
        return self.n_significant / self.n_toys if self.n_toys else None

    @property
    def Z_true(self):
        return self._p2Z(self.p_true)

    @property
    def Z_claim(self):
        return self._p2Z(self.p_claim)

    @property
    def Z_claim_corrected(self):
        return self._p2Z(self.p_claim_corrected)

    # Two different approaches for coverage exist, either distance between
    # significances, or log10(p_true/p_claim). We report both.
    @property
    def coverage_Zdiff(self):
        return self.Z_true - self.Z_claim_corrected

    @property
    def coverage_pratio(self):
        return math.log10( self.p_true / self.p_claim_corrected ) if self.p_true > 0 else np.inf

    def _asdict(self):
        return vars(self)

    def print_report(self):
        # Report result
        print( "\tTest case:                        {:s}".format( self.test_case ) )
        print( "\tPrior:                            {:s}".format( self.prior ) )
        print( "\tToy experiments:                  {:d}".format( self.n_toys ) )
        print( "\tTested/not tested:                {:d}/{:d}".format( self.n_toys-self.n_nottested, self.n_nottested ) )
        print( "\tInvalid (p == 0):                 {:d}".format( self.n_invalid ) )
        print( "\tClaimed type-1 error rate:        {:.04f} (Z_claim  = {:.03f})".format( self.p_claim, self.Z_claim ) )
        print( "\tAsymetry correction factor:       {:.03f}".format( self.asymmetry_correction ) )
        print( "\tCorrected claimed values:         {:.04f} (Z_claim' = {:.03f})".format( self.p_claim_corrected, self.Z_claim_corrected ) )
        print( "\tExpected significant:             {:.02f}".format( self.n_toys*self.p_claim_corrected ) )
        print( "\tObserved significant:             {:d}".format( self.n_significant ) )
        print( "\tTrue type-1 error rate:           {:.04f} (Z_true   = {:.03f})".format( self.p_true, self.Z_true ) )
        print( "\tCoverage Z_true-Z_claim:          {:.3f}".format( self.coverage_Zdiff ) )
        print( "\tCoverage log10(p_true/p_claim):   {:.3f}".format( self.coverage_pratio ) )
        print( "\tMean exp.:                        {:.3f}".format( self.mean_n_exp ) )
        print( "\tMean obs.:                        {:.3f}".format( self.mean_n_obs ) )


_lookup_hits = 0
_lookup_misses = 0
@np.vectorize
def compute_p(n_obs, n_exp, sigma_exp, prior):
    global _lookup_hits, _lookup_misses

    p = -1
    if prior == 'lookup':
        p = pCalc.c_lookup_p_convolution( n_obs, n_exp, sigma_exp, debug=False )
        if p < 0:
            _lookup_misses += 1
        else:
            _lookup_hits += 1

    if p < 0:
        real_prior = pCalc.NORMAL_PRIOR if prior == 'normal' else pCalc.LOGNORMAL_PRIOR
        p = pCalc.c_compute_p_convolution(n_obs, n_exp, sigma_exp, real_prior, debugLevel=0 )
    return p

def coverage_mp_wrapper(args):
    try:
        try:
            return coverage(*args)
        except ValueError as e:
            if "lambda=0" in str(e):
                return CoverageResult()
            else:
                raise

    except Exception as e:
        traceback.print_exc()
        raise

def coverage( n_true=50.,
              rel_err=1.0,
              Z_claim=2.0,
              n_toys=100000,
              test_case='deficit',
              prior='lognormal',
              quiet=False ):

    # Possible test cases: 'excess', 'deficit', 'all'
    assert test_case in ('excess', 'deficit', 'all')
    assert prior in ('normal', 'lognormal', 'lookup')

    print( "Coverage Test for N = {:g}, err = {:g}% ...".format( n_true, rel_err*100 ) )

    # Random number generation

    # Select distributions based on the prior
    if prior == 'normal':
        exp_distribution = scipy.stats.truncnorm( -1.0/rel_err, np.inf, loc=n_true, scale=rel_err*n_true )
    elif prior in ('lookup', 'lognormal'):
        lnk = np.log1p( rel_err )
        exp_distribution = scipy.stats.lognorm( lnk, 0, n_true )

    if n_true == 0:
        raise ValueError("Cannot generate poisson distribution around lambda=0")

    # Generate random "pseudo-expectations" around n_true (with uncertainty,
    # see distributions above).
    n_exp = exp_distribution.rvs( size=n_toys )

    # Generate random "observations" around the n_true value
    n_obs = scipy.stats.poisson.rvs( n_true, size=n_toys )

    # We want to test for excesses and deficits separately, so we have to take
    # care of the asymmetry of the distribution.
    # We calculate the cdf (cumulative distribution function) or sf (survival
    # function) = 1-cdf for this distribution at n_true.
    # Since the distribution is normalized, this well be the factor itself
    if test_case == 'excess':
        correction_factor = exp_distribution.cdf( n_true )
    elif test_case == 'deficit':
        correction_factor = exp_distribution.sf( n_true )
    elif test_case == 'all':
        correction_factor = 1.0

    # Compute claimed values for asymetric distributions
    # First, convert significance Z to p-value
    p_claim = scipy.stats.norm.sf( Z_claim )
    # correct p-value
    p_claim_corrected = p_claim / ( 2*correction_factor )
    # Convert p-value back to Z-score (significance)
    Z_claim_corrected = scipy.stats.norm.isf( p_claim )

    # For debugging: values without correction
    #p_claim_corrected = p_claim
    #Z_claim_corrected = Z_claim

    # Filter only excesses/deficits

    # Generate [False, True, True, False, True, ...] vector containing info
    # whether this row should be tested
    if test_case == 'excess':
        should_test = n_obs > n_exp
    elif test_case == 'deficit':
        should_test = n_obs <= n_exp
    elif test_case == 'all':
        should_test = np.ones_like( n_exp, dtype=bool )

    tested_count = np.count_nonzero( should_test )

    # Warn on insufficient statistics
    if tested_count < 100:
        print( "Warning: only %d of %d pseudo-experiments will be tested." % ( tested_count, n_toys ), file=sys.stderr )

    # Select only the values that should be tested
    n_obs = n_obs[ should_test ]
    n_exp = n_exp[ should_test ]

    # Calculate p-values (vector operation on all n_obs/n_exp combinations at
    # once).
    # p-value: how often would we see a *deviation* as large (or larger) as the
    # observed one. Emphasis on "deviation", because it's not "how often do we
    # see an excess as large..." / "a deficit as large...".
    # Use music_p_mp to call multiprocessing wrapper

    if prior == 'normal':
        # "Old" error: rel_err on n_true (given)
        absolute_error = np.ones_like(n_exp)*n_true*rel_err
    elif prior in ('lognormal', 'lookup'):
        # "New" error: rel_err on n_exp (randomly drawn)
        absolute_error = n_exp*rel_err

    p = compute_p( n_obs, n_exp, absolute_error, prior )

    # Calculate true p-value and coverage
    invalid = ( p == 0 )
    significant = np.logical_and( p < p_claim, p != 0 )
    insignificant = np.logical_and( p >= p_claim, p != 0 )

    not_tested_count = n_toys - np.count_nonzero( should_test )
    invalid_count = np.count_nonzero( invalid )
    significant_count = np.count_nonzero( significant )
    insignificant_count = np.count_nonzero( insignificant )

    # Each pseudo-experiment is in exactly one of these groups:
    # * not tested (e.g. deficits when we're testing excesses)
    # * invalid (p == 0)
    # * significant (p < alpha)
    # * insignificant (p >= alpha)

    result = CoverageResult(
        n_true = n_true,
        rel_err = rel_err,
        asymmetry_correction = correction_factor,
        n_toys = n_toys,
        p_claim = p_claim,
        p_claim_corrected = p_claim_corrected,

        n_nottested = not_tested_count,
        n_invalid = invalid_count,
        n_significant = significant_count,
        n_insignificant = insignificant_count,

        mean_n_exp = n_exp.mean(),
        mean_n_obs = n_obs.mean(),
        prior = prior,
        test_case = test_case,
    )

    if not quiet:
        result.print_report()

        if False:
            plt.clf()

            X = n_exp
            #X = error
            Y = n_obs

            plt.scatter( X[ significant ], Y[ significant ], \
                s=2*scipy.stats.norm.isf( p[ significant ] ), color="red", \
                label="Significant", lw=0 )

            plt.scatter( X[ insignificant ], Y[ insignificant ], \
                s=2*scipy.stats.norm.isf( p[ insignificant ] ), color="green", \
                label="Insignificant", lw=0 )

            plt.xlabel( "Expected Number of Events" )
            #plt.xlabel( "Absolute Errors" )
            plt.ylabel( "Observed Number of Events" )
            plt.legend( loc=4 if test_case == 'excess' else 2 )

            amin = min(plt.xlim()[0], plt.ylim()[0])
            amax = min(plt.xlim()[1], plt.ylim()[1])
            plt.xlim(amin, amax)
            plt.ylim(amin, amax)
            plt.plot([amin, amax], [amin, amax], color="black", alpha=0.5)

            plt.savefig( "scatter.pdf" )

    return result

class Storage:
    def __init__(self, name):
        self.path = name

    def _assert_dir_exists(self):
        if not os.path.exists( self.path ):
            os.makedirs( self.path )

    def has(self, name):
        path = os.path.join( self.path, name )
        return os.path.exists( path )

    def save(self, name, value):
        self._assert_dir_exists()

        _, ext = os.path.splitext(name)
        ext = ext.lower()

        path = os.path.join( self.path, name )

        if ext == ".json":
            import json
            with open( path, 'w' ) as file:
                json.dump( value, file, indent=2 )
        elif ext == ".txt":
            np.savetxt( path, value )
        elif ext == ".npy":
            np.save( path, value )
        elif ext == ".pkl":
            import cPickle as pickle
            with open( path, 'wb' ) as file:
                pickle.dump( value, file )
        else:
            raise ValueError("Unknown extension '%s'" % ext)


    def load(self, name):
        _, ext = os.path.splitext(name)
        ext = ext.lower()

        path = os.path.join( self.path, name )

        if ext == ".json":
            import json
            with open( path, 'r' ) as file:
                return json.load( file )
        elif ext == ".txt":
            return np.loadtxt( path )
        elif ext == ".npy":
            return np.load( path )
        elif ext == ".pkl":
            import cPickle as pickle
            with open( path, 'rb' ) as file:
                return pickle.load( file )
        else:
            raise ValueError("Unknown extension '%s'" % ext)



def calculate_grid( ns, sigmas, dirname, Z_claim=2.0, test_case='excess', n_toys=10000,
                    prior=pCalc.NORMAL_PRIOR, workers=1 ):
    params = {
        'N': ns.tolist(),
        'sigma': sigmas.tolist(),
        'Z': Z_claim,
        'test_case': test_case,
        'ntoys': n_toys,
        'prior': prior,
    }

    storage = Storage( dirname )
    storage.save( "params.json", params )

    ns_rep, sigmas_rep = zip(*itertools.product(ns, sigmas))
    assert(len(ns_rep) == len(ns)*len(sigmas))
    assert(len(sigmas_rep) == len(ns)*len(sigmas))

    args = zip( ns_rep, sigmas_rep, itertools.repeat(Z_claim), itertools.repeat(n_toys), itertools.repeat(test_case), itertools.repeat(prior), itertools.repeat(True) )
    assert(len(args) == len(ns)*len(sigmas))

    if workers > 1:
        pool = multiprocessing.Pool( min(workers, len(args)) )
        # http://stackoverflow.com/a/1408476/489345
        all_results = pool.map_async(coverage_mp_wrapper, args, 10).get(9999999)
        pool.close()
        pool.join()
    else:
        all_results = map(coverage_mp_wrapper, args)

    storage.save( "results.json", [ t._asdict() for t in all_results ] )

    grid = np.zeros((len(ns), len(sigmas)))
    grid.fill( np.NAN )

    for i, n in enumerate(ns):
        for j, sigma in enumerate(sigmas):
            index = i*len(sigmas) + j
            grid[i][j] = all_results[index].coverage_Zdiff

    storage.save( "grid.txt", grid )


def plot_grid( coverage, params, logx=False, logy=False, vmin=None, vmax=None, cmap="viridis", colorbar=True ):
    N, S = np.array(params["N"]), np.array(params["sigma"])
    test_case = params.get( "test_case", 'excess' )
    prior = params.get( "prior", "?" )
    Z = params[ "Z" ]
    ntoys = int( params[ "ntoys" ] )


    prior_name = {
        'lognormal': "Log-Normal Prior",
        'normal': "Normal Prior",
        'lookup': "Log-Normal Prior via LUT",
    }[ prior ]

    case_name = {
        'excess': 'Excesses Only',
        'deficit': 'Deficits Only',
        'all': 'All Regions',
    }[ test_case ]

    def _close(a,b):
        return abs(a-b)/a < 1e-4

    if not _close( N[-1]-N[-2], N[1]-N[0] ):
        logx = True
    if not _close( S[-1]-S[-2], S[1]-S[0] ):
        logy = True

    assert len(N) == coverage.shape[0], (len(N), coverage.shape[0])
    assert len(S) == coverage.shape[1], (len(S), coverage.shape[1])

    if not logx:
        dN = (N[1]-N[0])/2.
        cell_N = np.append( N-dN, ( N[-1]+dN ) )
    else:
        fN = math.sqrt( N[1]/N[0] )
        cell_N = np.append( N/fN, ( N[-1]*fN, ) )

    if not logy:
        dS = (S[1]-S[0])/2.
        cell_S = np.append( S-dS, ( S[-1]+dS ) )
    else:
        fS = math.sqrt( S[1]/S[0] )
        cell_S = np.append( S/fS, ( S[-1]*fS, ) )

    coverage = np.ma.masked_invalid( coverage )

    cmap = plt.get_cmap(cmap)
    cmap.set_bad(color="w", alpha=0.0)
    pcol = plt.pcolormesh( cell_N, cell_S, coverage.T, cmap=cmap, vmin=vmin, vmax=vmax, rasterized=True )
    #pcol = plt.pcolor( cell_N, cell_S, coverage.T, cmap=cmap, vmin=vmin, vmax=vmax, rasterized=True )
    pcol.set_edgecolor('face')
    pcol.set_linewidth(0)

    plt.grid()
    #pcol.set_zorder(100)
    #plt.contourf( N, S, coverage.T, 50, cmap=cmap, vmin=vmin, vmax=vmax )

    # if test_case != 'excess':
        # # for deficits, mark "forbidden-area", where calculated p-value is
        # # less than the Z we probed
        # mesh_N, mesh_S = np.meshgrid( N, S )
        # mesh_sigma = mesh_S*mesh_N
        # p = compute_p( 0, mesh_N+1*mesh_sigma, mesh_sigma, prior )
        # forbidden = p > scipy.stats.norm.isf( Z )
        # forbidden_display = np.ma.masked_array( np.ones_like(forbidden), mask=forbidden )
        # plt.pcolor( cell_N, cell_S, forbidden_display, cmap="jet", vmin=0, vmax=1, hatch="/", color="none", edgecolor="red" )

    def _nice_ticks( x, maxlen=20 ):
        while len(x) > maxlen:
            x = x[::5]
        return x

    xticks = _nice_ticks( N )
    yticks = _nice_ticks( S )

    plt.xticks( xticks, rotation="vertical" )
    plt.yticks( yticks )

    def _remove_frame( ax=None ):
        if ax is None:
            ax = plt.gca()
        ax.spines['top'].set_visible( False )
        ax.spines['right'].set_visible( False )
        ax.spines['bottom'].set_visible( False )
        ax.spines['left'].set_visible( False )

    #_remove_frame()

    plt.xlim( cell_N.min(), cell_N.max() )
    plt.ylim( cell_S.min(), cell_S.max() )

    if logx:
        plt.xscale( "log", nonposx="clip" )
    if logy:
        plt.yscale( "log", nonposy="clip" )

    plt.xlabel( "True N" )
    plt.ylabel( "Relative Uncertainty" )

    text =  prior_name + "\n" + \
            case_name + "\n" + \
            r"$n_\mathrm{toys} = %d$" + "\n" + \
            r"$Z = %.1f$"
    text = text % ( ntoys , Z )

    text_kwargs = dict(
        bbox=dict( boxstyle="square", facecolor="white", edgecolor="white", alpha=0.5 ),
        transform=plt.gca().transAxes,
        fontsize=14)

    if test_case == 'excess':
        plt.text(0.95, 0.95, text,
            verticalalignment='top',
            horizontalalignment='right',
            **text_kwargs)
    else:
        plt.text(0.05, 0.05, text,
            verticalalignment='bottom',
            horizontalalignment='left',
            **text_kwargs)

    #plt.title( "Coverage for %dk Toy Experiments" % ( int(float(ntoys)/1000) ) )

    if colorbar:
        def _coverage2logpfrac(c):
            n = scipy.stats.norm.sf( c + Z )
            d = scipy.stats.norm.sf( Z )
            return math.log10(n/d)

        def _format_ytick(val, pos):
            mapped_val = val*(vmax-vmin) + vmin
            return "%.01f" % _coverage2logpfrac(mapped_val)

        cb = plt.colorbar( pcol, pad=0.05 )
        cb.solids.set_edgecolor("face")
        #cb.set_label( "Coverage" )
        _remove_frame(cb.ax)

        pos = cb.ax.get_position()
        cb.ax.set_aspect('auto')

        ax2 = cb.ax.twinx()
        #ax2.set_ylim([ _coverage2logpfrac(vmin), _coverage2logpfrac(vmax) ])
        ax2.set_yticks( cb.ax.get_yticks() )
        ax2.yaxis.set_major_formatter( mpl.ticker.FuncFormatter( _format_ytick ) )

        pos.x0 += 0.08
        cb.ax.set_position(pos)
        ax2.set_position(pos)

        cb.ax.set_ylabel( r"$Z_\mathrm{true} - Z_\mathrm{claim}$", labelpad=-75.0 )
        ax2.set_ylabel( r"$log(p_\mathrm{true} / p_\mathrm{claim})$" )


class CoverageComputer:
    cmd = CommandRegister()

    @cmd.command
    def calc( self, n_true, rel_err, Z_claim=2.0, n_toys=10000, test_case='deficit', prior='lognormal' ):
        n_true = float( n_true )
        rel_err = float( rel_err )

        coverage(n_true, rel_err, Z_claim=Z_claim, n_toys=n_toys, test_case=test_case, prior=prior )

    @cmd.command
    def plot( self, names=tuple(), logx=False, logy=False, fmt=("png", "pdf"), cmap="viridis_r", vmin=-2.0, vmax=2.0, no_colorbar=False ):
        for dirname in names:

            storage = Storage( dirname )
            if not storage.has( "grid.txt" ):
                continue

            params = storage.load( "params.json" )
            coverage = storage.load( "grid.txt" )

            plt.clf()
            plot_grid( coverage, params, logx=logx, logy=logy, vmin=vmin, vmax=vmax, cmap=cmap, colorbar=(not no_colorbar) )
            self._save_plot( dirname, fmts=fmt )

            # results = [ CoverageResult( **r ) for r in storage.load( "results.json" ) ]

            # reducer = lambda x: x is not None and not np.isnan(x) and x != 0
            # p_claims = filter( reducer, ( r.p_claim_corrected for r in results ) )
            # p_trues = filter( reducer, ( r.p_true for r in results ) )
            # if p_trues:
                # plt.clf()
                # plt.hist( p_trues, bins=50 )
                # plt.axvline( p_claims[0], ls="--" )
                # plt.xlabel("True p-value")
                # plt.ylabel("Number of cells ")
                # self._save_plot( dirname, "_p_comp", fmts=fmt )
            # else:
                # print( "No interesting data found." )

    def _save_plot( self, dirname, suffix="", fmts=("pdf", ) ):
        for format in fmts:
            plotname = os.path.basename( dirname ) + suffix + "." + format
            plt.savefig( plotname, bbox_inches="tight", dpi=720 )
            print( "Plot created at", plotname )

    @cmd.command
    def grid( self, dirname, logn=False, logsigma=False, nmin=0., nmax=100.,
        sigmamin=0.0, sigmamax=1.0, nn=20, nsigma=20, Z_claim=3.0, workers=1,
        test_case='excess', n_toys=1000000, prior='lognormal' ):

        if logn:
            ns = np.logspace( math.log10(nmin), math.log10(nmax), nn+1 )
        else:
            ns = np.linspace( nmin, nmax, nn+1 )

        if logsigma:
            sigmas = np.logspace( math.log10(sigmamin), math.log10(sigmamax), nsigma+1 )
        else:
            sigmas = np.linspace( sigmamin, sigmamax, nsigma+1 )

        calculate_grid( ns, sigmas, dirname, test_case=test_case, Z_claim=Z_claim,
            n_toys=n_toys, prior=prior, workers=workers )

    @cmd.command
    def benchmark( self, prior='lognormal', n=10000, N=5, n_obs=10., n_exp=8., rel_err=0.2 ):
        durations = []
        for step_N in range(N):

            start = time.clock()
            for step_n in range(n):
                compute_p( n_obs, n_exp, rel_err, prior )
            end = time.clock()

            duration = end-start
            print(step_N, duration)

            durations.append(duration)

        min_for_n = min(durations)
        time_per_calculation = min_for_n / n
        print( "Time per calculation: %g us" % (1e6*time_per_calculation) )

if __name__=="__main__":
    CoverageComputer.cmd.run(CoverageComputer)
    print( "Lookup Hits:  ", _lookup_hits )
    print( "Lookup Misses:", _lookup_misses )