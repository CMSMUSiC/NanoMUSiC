#!/bin/env python
import os
import argparse
import sqlalchemy
import dbtools
import traceback
import re
import ROOT as ro
import configargparser

from ectools.register import ecroot
from ecresults.plots import ECDistributionPlot

ro.gROOT.SetBatch(True)

ro.gErrorIgnoreLevel = ro.kWarning

def plot_ec( conf, i=0, N=0, data_ec=None, mc_ec=None, signal_ec=None, config=None, default_lumi=-1., **kwargs ):
    # the_ec points to at least one existing TEventClass, thus can be
    # used to obtain the event class name, etc.
    the_ec = data_ec if data_ec else mc_ec

    # Calculate the filename: From the Event Class name, all illegal
    # characters are replaced by '_'.
    ec_name = the_ec.GetName()
    filename_matcher = re.compile( '[^a-zA-Z0-9_]' )
    clean_ec_name = filename_matcher.sub( '_', ec_name )
    if i == 0:
        head = " {:>11s} | {:>40s} | {:<40s}".format( "count", "Event Class", "Filename" )
        print( head )
        print( "="*( len( head ) ) )

    def make_plots(session=None):
        for distribution in ecroot.get_ec_distribution_types( the_ec ):
            job_name = ec_name + " (" + distribution + ")"
            if ECDistributionPlot.check_scan_missing( session, config, ec_name, distribution=distribution):
                continue
            try:
                # Create the plot
                plot = ECDistributionPlot(
                                            config,
                                            distribution,
                                            data_ec=data_ec,
                                            mc_ec=mc_ec,
                                            signal_ec=signal_ec,
                                            session=session
                                         )
            except Exception as e:
                print( " {:>4d} / {:>4d} | {:>40s} | {:<40s}".format(
                    i+1,
                    N,
                    job_name,
                    "< NO PLOT ('%s') >" % str( e ) )
                )
                traceback.print_exc()
            else:
                if plot.skip:
                    print( " {:>4d} / {:>4d} | {:>40s} | {:<40s}".format( i+1, N, job_name, "skipped:" +  plot.skipreason ) )
                    try:
                        plot.cleanup()
                    except:
                        pass
                else:
                    outname = plot.get_plot_name( config,
                                                  ec_name=ec_name,
                                                  distribution=distribution )
                    sub_path = plot.get_subfolder(config, ec_name=ec_name, distribution=distribution)
                    plot.save_plot( outname, sub_path )
                    plot.cleanup()
                    print( " {:>4d} / {:>4d} | {:>40s} | {:<40s}".format( i+1, N, job_name, conf.out ) )
                del plot

    if os.path.exists(conf.scan_db_path):
        engine = sqlalchemy.create_engine( 'sqlite:///' + conf.scan_db_path, echo=False )
        with dbtools.session_scope( engine ) as scan_session:
            make_plots(scan_session)
    else:
        make_plots()

    #if conf.jobs > 1 and mc_ec: mc_ec.Delete()
    #if conf.jobs > 1 and data_ec: data_ec.Delete()

def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""

    parser = configargparser.ArgumentParser( description="Create MUSiC plots from ROOT files." )

    general_group = parser.add_argument_group( title="General options" )

    ecroot.add_ec_standard_options( general_group )

    ECDistributionPlot.add_commandline_options( parser )
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

                                    /'i:zi 'plo:t/
    """ )

    conf = parse_arguments()
    if conf.scan_id and not os.path.exists(conf.scan_db_path):
        raise ValueError("DB does not exists")

    ecroot.ec_loop( conf, plot_ec, config=conf )

if __name__=="__main__":
    main()
