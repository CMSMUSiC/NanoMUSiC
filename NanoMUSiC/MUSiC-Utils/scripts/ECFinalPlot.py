#!/bin/env python
from __future__ import print_function

import argparse
from array import array
import math
import os
import sys
import time
import collections

import scipy.stats
import ROOT
import numpy as np
import sqlalchemy


start = time.time()
from ectools.register import ecroot
from ecresults.plots import PtildePlot
import dbtools

from cmsstyle import set_cms_root_style

ROOT.gROOT.SetBatch()

set_cms_root_style()

end = time.time()
print("%.2f seconds for imports" % (end - start))
z_to_p = scipy.stats.norm.sf

def dump_latex( conf, pdicts, n, distribution, class_type="mixed" ):

    if distribution == "InvMass":
        distribution_long = "\\mass"
    elif distribution == "SumPt":
        distribution_long = "\\sumpt"
    elif distribution == "MET":
        distribution_long = "\\MET"
    else:
        distribution_long = "unknown"
    dist_types = ecroot.distribution_types()
    distribution_type = [ d for d in dist_types if d.name== distribution ]
    if distribution_type:
        distribution_long = distribution_type[0].latex_tag
    else:
        distribution_long = "unknown"

    filename = os.path.join( conf.out,
        "most_significant_%s_%s.tex" % ( distribution, class_type )
    )
    with open( filename, 'w' ) as fout:
        # table definition
        fout.write( "\\begin{table}\n" )
        # caption
        fout.write( "   \\caption{Overview of the " )
        fout.write( str( n ) )
        fout.write( " most significant classes in the " )
        fout.write( distribution_long + " " + class_type + "distributions.}\n" )
        #label
        fout.write( "   \\label{tab:ptilde_list_" + distribution.lower())
        fout.write( "_"+ class_type + "}\n" )
        fout.write( "   \\begin{tabular}{p{5.5cm}| l l | l l l l l l}\n" )
        fout.write( "      \\hline\\hline\n" )
        # first header line
        fout.write( "      Event Class & \\multicolumn{2}{c|}{Global} &" )
        fout.write( " \\multicolumn{6}{c}{Region} \\\\\n" )
        # second header line
        fout.write( "       & $n_{\\text{Data}}$ &" )
        fout.write( "$n_{\\text{MC}}$ & ")
        fout.write( "$n_{\\text{Data}}$ & ")
        fout.write( "$n_{\\text{MC}}$ & " )
        fout.write( "low & " )
        fout.write( "high & " )
        fout.write( "\\ptilde & " )
        fout.write( "$\log_{10} \ptilde$ \\\\\n" )
        fout.write( "      \\hline\n" )
        # third header line
        fout.write( "       & & & & & ($\GeV$) & ($\GeV$) & & \\\\" )
        # add lines
        for pdict in pdicts:
            for rounddict in pdict[ "DataRounds" ]:
                rowlist = []
                rowlist.append( ecroot.latex_ec_name( pdict[ 'Name' ], "latex" ) )
                rowlist.append( "%.0f" % rounddict[ 'DataEventsClass' ] )
                rowlist.append( "%.2g" % pdict[ 'MCEventsClass' ] )
                rowlist.append( "%1.0g" % rounddict[ 'DataEventsRegion' ] )
                rowlist.append( "%.2g" % rounddict[ 'MCEventsRegion' ] )
                rowlist.append( "%d" % rounddict[ 'LowerEdge' ] )
                rowlist.append( "%d" % ( rounddict[ 'LowerEdge' ] + rounddict[ 'Width' ] ) )
                rowlist.append( "%.2g" % rounddict[ 'PTildeValue' ] )
                rowlist.append(  "%.2g" % ( -1. * np.log10( rounddict[ 'PTildeValue' ] ) ) )
                fout.write( "      ")
                fout.write( " & ".join(rowlist) )
                fout.write("\\\\\n")
        fout.write( "      \\hline\\hline\n" )
        fout.write( "   \\end{tabular}\n" )
        fout.write( "\\end{table}\n" )


def class_type_actions(conf,
                       class_type_name,
                       ec_names,
                       distribution=None,
                       session=None,
                       **kwargs):
    for temp_class_type in ecroot.class_types():
        class_type_option = "filter_" + temp_class_type.name.replace("-", "_")
        if temp_class_type.name == class_type_name:
            setattr(conf, class_type_option, True)
        else:
            setattr(conf, class_type_option, False)
    ec_names, dropped = ecroot.filter_class_names( set(ec_names), conf )

    if not ec_names:
        return

    with open(os.path.join(conf.out, "all_classes_%s.txt" % class_type_name), "wb") as out:
        for ec_name in sorted(list(ec_names)):
            out.write(ec_name + "\n")

    if conf.nscans == 0:
        start = time.time()
        nscans_query = session.query( dbtools.Result.event_class.label("event_class"),
                        sqlalchemy.func.count( dbtools.Result.round ).label("round_count") ) \
                       .filter_by( hash=conf.mc_hash ) \
                       .group_by( dbtools.Result.event_class )

        nscans_list = [ r.round_count for r in nscans_query ]
        nscans_list = [ n for n in nscans_list if n > 0 ]
        end = time.time()
        print("%.2f seconds for nscans filter" % (end - start))
        # there might be bugs for few classes
        if nscans_list:
            conf.nscans = int( np.percentile( nscans_list, 80 ) )
        # conf.nscans = 0

        # give some input
        print( "There are %d classes with less than %d pseudo experiments." % ( sum( nscan < conf.nscans for nscan in nscans_list ), conf.nscans ) )
        print( "There are %d classes with more than %d pseudo experiments." % ( sum( nscan > conf.nscans for nscan in nscans_list ), conf.nscans ) )

    # reset nscans if p-tilde value calculation considers only one deviation type
    if conf.filter_p_for_deviation_type:
        nscans_skip = None
    else:
        nscans_skip = conf.nscans
        print( "Using %d pseudo experiments for all classes.\n" % conf.nscans )
    print( "Calculating skip reasons" )

    start = time.time()
    # set skip reasons
    skipped, not_skipped = PtildePlot.set_skip_reason( session,
                                            data_hash=conf.data_hash,
                                            mc_hash=conf.mc_hash,
                                            nscans=nscans_skip,
                                            min_yield=conf.min_yield,
                                            ec_names=ec_names,
                                            filter_deviation_type=conf.filter_deviation_type,
                                            skip_no_data = conf.filter_no_data )

    with open(os.path.join(conf.out, "skipped_classes_%s.txt" % class_type_name), "wb") as out:
        for ec_name in sorted(skipped.keys()):
            out.write(ec_name + " " + skipped[ec_name] + "\n")

    with open(os.path.join(conf.out, "kept_classes_%s.txt" % class_type_name), "wb") as out:
        for ec_name in sorted(list(not_skipped)):
            out.write(ec_name + "\n")

    end = time.time()
    print("%.2f seconds to set skip reasons" % (end - start))
    print( "%d classes are not included (skipped)" % len( skipped ) )
    skip_counter = collections.Counter(skipped.values())
    for k, c in skip_counter.items():
        print(k, c)
    print( "%d classes are not skipped" % len( not_skipped ) )

    start = time.time()
    plot = PtildePlot( conf,
                       not_skipped,
                       session=session,
                       distribution=distribution,
                       class_type=class_type_name)

    plot.plot()
    outname = plot.get_plot_name( conf,
                                  distribution=distribution,
                                  class_type=class_type_name)
    sub_path = plot.get_subfolder(conf, distribution=distribution)
    plot.save_plot( outname, sub_path )
    end = time.time()
    print("%.2f seconds to create ptilde distribution plot for %s" % (end - start, plot.class_type))


def main():
    """main function of the project."""

    print( r""" ECFinalPlot - Final ptilde distribution plotter
    """ )

    parser = argparse.ArgumentParser( "Create MUSiC p-tilde distribution plot" )
    PtildePlot.add_commandline_options( parser )
    conf = parser.parse_args()

    if not any((conf.scan_id, conf.data_hash, conf.mc_hash)):
        raise RuntimeError("One of scan_id, data_hash, mc_hash needs to be defined")

    start = time.time()
    first_start = start
    engine = sqlalchemy.create_engine( 'sqlite:///' + conf.scan_db_path, echo=False )
    v_ptilde = dbtools.reflect_view( 'v_ptilde', engine )
    end = time.time()
    print("%.2f seconds to build db view" % (end - start))
    with dbtools.session_scope( engine ) as session:

        if conf.mc_hash:
            mc_scan = dbtools.match_hash(conf.mc_hash,
                                         session,
                                         allow_name=False,
                                         scan_type="pseudoScan")
        else:
            mc_scan = dbtools.match_hash( conf.scan_id, session, scan_type="pseudoScan", allow_name=True )

        conf.mc_hash = mc_scan.hash
        print("MC hash:", conf.mc_hash)

        if not conf.signal_hash:
            if conf.data_hash:
                data_scan = dbtools.match_hash(conf.data_hash,
                                   session,
                                   allow_name=False)
            else:
                data_scan = dbtools.match_hash( conf.scan_id, session, scan_type="dataScan", allow_name=True )
            conf.data_hash = data_scan.hash
            print("Data hash:", conf.data_hash)
            assert(data_scan.distribution == mc_scan.distribution)
        else:
            if conf.signal_hash:
                signal_scan = dbtools.match_hash(conf.signal_hash,
                                             session,
                                             scan_type="signalScan",
                                             allow_name=False)
            else:
                signal_scan = dbtools.match_hash( conf.scan_id, session, scan_type="signalScan" )
            conf.signal_hash = signal_scan.hash
            print("Signal hash:", conf.data_hash)
            assert(signal_scan.distribution == mc_scan.distribution)


        print("Scan hash for distribution: " + mc_scan.distribution)
        # Create output path, otherwise savefig() will fail.
        if not os.path.isdir( conf.out ):
            os.makedirs( conf.out )
        print("Receiving all ec_names from database")
        start = time.time()
        # get list of class names
        ec_names_query = session.query( dbtools.Result.event_class.label("event_class")) \
                          .filter_by( hash=conf.mc_hash ) \
                          .filter_by( round=0 )

        ec_names = [r.event_class for r in ec_names_query]
        end = time.time()
        print( "%d classes found in database" % len( ec_names ) )
        ecroot.ec_class_type_loop(class_type_actions,
                                  conf,
                                  ec_names,
                                  distribution=mc_scan.distribution,
                                  session=session )
    print("%.2f secondstotal runtime" % (end - first_start))
if __name__=="__main__":
    main()
