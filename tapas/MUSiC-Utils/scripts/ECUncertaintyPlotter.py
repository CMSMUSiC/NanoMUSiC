#!/bin/env python

from __future__ import print_function

import os.path
import configargparser
import re
import traceback
import ROOT as ro


from ectools.register import ecroot
from ecresults.plots import ECUncertaintyPlot
ro.gROOT.SetBatch()

ro.gErrorIgnoreLevel = ro.kWarning



def plot_ec( conf, i=0, N=0, data_ec=None, mc_ec=None, config=None, default_lumi=2.3, relative=True, **kwargs ):
    # the_ec points to at least one existing TEventClass, thus can be
    # used to obtain the event class name, etc.
    the_ec = mc_ec

    # Calculate the filename: From the Event Class name, all illegal
    # characters are replaced by '_'.
    ec_name = the_ec.GetName()
    filename_matcher = re.compile( '[^a-zA-Z0-9_]' )
    clean_ec_name = filename_matcher.sub( '_', ec_name )

    if i == 0:
        head = " {:>11s} | {:>40s} | {:<40s}".format( "count", "Event Class", "Filename" )
        print( head )
        print( "="*( len( head ) ) )

    for distribution_type in ecroot.distribution_types():
        distribution = distribution_type.name
        if distribution == "MET" and not "MET" in ec_name:
            continue

        try:
            # Create the plot
            plot = ECUncertaintyPlot(
                config,
                distribution,
                data_ec=data_ec,
                mc_ec=mc_ec,
            )
            if not plot.skip:
                outname = plot.get_plot_name( config,
                                              ec_name=ec_name,
                                              distribution=distribution )
                sub_path = plot.get_subfolder(config,
                                              ec_name=ec_name,
                                              distribution=distribution)
                plot.save_plot( outname, sub_path )

        except Exception as e:
            print( " {:>4d} / {:>4d} | {:>40s} | {:<40s}".format(
                i+1,
                N,
                ec_name,
                "< NO PLOT ('%s') >" % str(e) )
            )
            traceback.print_exc()
        else:
            if plot.skip:
                print( " {:>4d} / {:>4d} | {:>40s} | {:<40s}".format( i+1, N, ec_name, "skipped:" + plot.skipreason ) )
            else:
                print( " {:>4d} / {:>4d} | {:>40s} | {:<40s}".format( i+1, N, ec_name, conf.out ) )

def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""

    parser = configargparser.ArgumentParser( description="Create MUSiC uncertainty plots from ROOT files." )

    general_group = parser.add_argument_group( title="General options" )

    ecroot.add_ec_standard_options( general_group )

    ECUncertaintyPlot.add_commandline_options( parser )

    return parser.parse_args()

def main():
    """main function of the project."""

    print( r"""
                                ____________      __      __
                               / ____/ ____/___  / /___  / /_
                              / __/ / /   / __ \/ / __ \/ __/
                             / /___/ /___/ /_/ / / /_/ / /_
                            /_____/\____/ .___/_/\____/\__/
                                       /_/

                                    /'i:zi plo:t/
    """ )

    conf = parse_arguments()

    ecroot.ec_loop( conf, plot_ec, config=conf, relative=conf.relative )

if __name__=="__main__":
    main()
