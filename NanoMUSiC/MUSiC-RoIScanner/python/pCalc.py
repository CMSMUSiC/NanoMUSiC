#!/bin/env python

from __future__ import print_function, division
import ctypes
import math
import os, os.path
import numpy as np
import scipy.stats

# Load shared object libraries
src_path = os.path.join( os.environ["SCAN_BASE"], "src" )
default_lookup_path = os.path.join( os.environ["SCAN_BASE"], "bin", "lookuptable.bin" )

cc_library = ctypes.cdll.LoadLibrary( os.path.join( src_path, "ConvolutionComputer.so" ) )
_compute_p_convolution = cc_library.compute_p_convolution
_compute_p_convolution.argtypes = [ ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_int ]
_compute_p_convolution.restype = ctypes.c_double

cl_library = ctypes.cdll.LoadLibrary( os.path.join( src_path, "ConvolutionLookup.so" ) )
_lookup_p_convolution = cl_library.lookup_p_convolution
_lookup_p_convolution.argtypes = [ ctypes.c_double, ctypes.c_double, ctypes.c_double ]
_lookup_p_convolution.restype = ctypes.c_double

# Constants
# We have to copy this from ConvolutionComputer.hh
NORMAL_PRIOR = 1
LOGNORMAL_PRIOR = 2

DEFAULT_PRIOR = NORMAL_PRIOR

# Python implementation
def c_compute_p_convolution( N_obs, N_SM, error_parameter, prior=DEFAULT_PRIOR, debugLevel=1 ):
    return _compute_p_convolution( N_obs, N_SM, error_parameter, prior, debugLevel )

np_compute_p_convolution = np.vectorize( c_compute_p_convolution, excluded=["prior", "debugLevel"] )

def py_compute_p_convolution( n_obs, n_exp, sigma_exp, prior=DEFAULT_PRIOR ):
    if prior == NORMAL_PRIOR:
        prior_func = lambda theta: np.exp( -np.power( (theta - n_exp)/sigma_exp, 2 )/ 2. )
    elif prior == LOGNORMAL_PRIOR:
        lnk = np.log1p( sigma_exp / n_exp )
        prior_func = lambda theta: scipy.stats.lognorm.pdf( theta, lnk, 0, n_exp )
        # equivalent:
        #prior_func = lambda theta: np.exp( -np.power( np.log( theta / n_exp ) / lnk, 2 )/2. ) / ( math.sqrt( 2.*math.pi ) * lnk * theta )
    else:
        raise ValueError( "Invalid prior" )

    excess_func = lambda theta: prior_func( theta ) * scipy.stats.poisson.sf( n_obs - 1, theta )
    deficit_func = lambda theta: prior_func( theta ) * scipy.stats.poisson.cdf( n_obs, theta )
    integration_args = dict(
        epsrel=1e-4,
        epsabs=1e-15,
        limit=10000,
    )

    # 8 sigma gives precision of 1e-16
    lower_bound = max( 0, n_exp - 8.*sigma_exp )
    upper_bound = n_exp + 8.*sigma_exp

    if n_obs > n_exp:
        p = scipy.integrate.quad( excess_func, lower_bound, upper_bound, **integration_args )[ 0 ]
    else:
        p = scipy.integrate.quad( deficit_func, lower_bound, upper_bound, **integration_args )[ 0 ]

    if prior == NORMAL_PRIOR:
        p /= scipy.integrate.quad( prior_func, lower_bound, upper_bound, **integration_args )[ 0 ]

    return p

def c_lookup_p_convolution( n_obs, n_exp, sigma_exp, debug=False, filename=default_lookup_path ):
    return _lookup_p_convolution( n_obs, n_exp, sigma_exp, debug, filename )

np_lookup_p_convolution = np.vectorize( c_lookup_p_convolution, excluded=["debug", "filename"] )

def c_combined_p_convolution( n_obs, n_exp, sigma_exp, prior, debug=False ):
    p = c_lookup_p_convolution( n_obs, n_exp, sigma_exp, debug )
    if p < 0:
        p = c_compute_p_convolution( n_obs, n_exp, sigma_exp, prior, debugLevel=2 if debug else 0 )
    return p

def parse_args():
    import argparse
    parser = argparse.ArgumentParser( description="A simple tool to calculate p-values with a hybrid bayesian / frequentist approach" )
    parser.add_argument( "--batch", action="store_true", help="only print p-value and return" )
    parser.add_argument( "--lognormal", action="store_true", help="use log-normal prior instead of normal" )
    parser.add_argument( "--obs", required=True, type=int, help="observed event yield in this region" )
    parser.add_argument( "--exp", required=True, type=float, help="expected event yield in this region" )
    parser.add_argument( "--sigma-exp", required=True, type=float, help="total uncertainty on expected event yield" )
    parser.add_argument( "--debug", type=int, default=1, help="debuglevel: 0 = errors only, 1 = warnings, 2 = info, 3 = debug" )
    parser.add_argument( "--python", action="store_true", help="use python implementation" )
    return parser.parse_args()

def main():
    args = parse_args()

    prior = LOGNORMAL_PRIOR if args.lognormal else NORMAL_PRIOR

    if args.python:
        p = py_compute_p_convolution( args.obs, args.exp, args.sigma_exp, prior )
    else:
        p = c_compute_p_convolution( args.obs, args.exp, args.sigma_exp, prior, args.debug )

    if args.batch:
        print( p )
        return

    print( "Input:" )
    print( "\tExpected: ( %g +- %g )" % ( args.exp, args.sigma_exp ) )
    print( "\tObserved: %d" % args.obs )
    print( "Output:" )
    print( "\tp-value: \t%g" % p )

    Z = scipy.stats.norm.isf( p )
    print( "\tZ-value: \t%g" % Z )

if __name__=="__main__":
    main()
