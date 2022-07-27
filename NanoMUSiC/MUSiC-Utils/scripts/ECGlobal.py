#!/bin/env python

from __future__ import print_function

import os
import json
import ConfigParser as configparser
import configargparser
import ROOT as ro
import numpy as np
import random
import itertools

from ectools.register import ecroot

from colors import PROCESS_GROUP_STYLES, SYSTEMATIC_STYLES, DETAILED_SYSTEMATIC_STYLES, HistStyle
from roothelpers import root_hist2arrays, TextAlign, Font
from cmsstyle import *

from ecresults.plots import SimpsonsPlot, SimpsonsRatioPlot, ECDominantProcessPlot, UncertaintyMap, ClassYieldsPlot
from ecresults.collector import ECCollector
from ecresults.util import ECRecordList

ro.gROOT.SetBatch()

ro.gErrorIgnoreLevel = ro.kWarning


def parse_config( filename=None ):
    parser = configparser.ConfigParser()
    if filename:
        parser.read( filename )
    if not parser.has_section( 'GENERAL' ):
        parser.add_section( 'GENERAL' )
    return parser

import random

def plot_p_value_dist(conf, hist_entries, suffix):
    hist = ro.TH1F("p_values_hist_" + suffix, "p_values_hist_" + suffix, 20, 0.00000001, 0.5)
    set_cms_root_style()
    c = ro.TCanvas("testll")
    c.UseCurrentStyle()
    c.SetLogy(1)
    c.SetLogx(0)
    for entry in hist_entries:
        hist.Fill(float(entry["p-value"]))
    hist.SetLineWidth(2)
    hist.Draw("hist")
    c.Print(os.path.join( conf.out, "p_values_%s.pdf" % suffix))


def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""

    parser = configargparser.ArgumentParser( description="Aggregate infos about a dataset" )

    general_group = parser.add_argument_group( title="General options" )

    ecroot.add_ec_standard_options( general_group )

    general_group.add_argument( "--class-count-summary", action = "store_true",
        help="Produce dominant process summary" )
    general_group.add_argument( "--class-yields", action = "store_true",
        help="Produce yield distribution plot" )
    general_group.add_argument( "--dominant-process-quantiles", type=float, nargs="+", default = [0., 0.5],
        help="Quantile when a process is considered dominant in a class (otherwise 'mixed' is used)" )
    general_group.add_argument( "--dominant_process_show_data", action="store_true",
        help="Show number of data classes in dominant process statistic plots")
    general_group.add_argument( "--uncert-summary", action = "store_true",
        help="Prodcue summary plots for dominant uncertainties" )
    general_group.add_argument( "--p-value-hist", action = "store_true",
        help="Prodcue a hist of p-values" )
    general_group.add_argument( "--simpsons", action = "store_true",
        help="Create Simpsons plots" )
    general_group.add_argument( "--simpsons-ratio", action = "store_true",
        help="Create Simpsons ratio plots" )
    general_group.add_argument( "--uncert-map", action = "store_true",
        help="Create uncertainty map plot" )

    object_group_choices = [None] + [e.name_tag for e in ecroot.object_groups()]
    general_group.add_argument( "--object-group-tag", default=None,
        choices=object_group_choices,
        help="Create the chosen outputs only for a specific object group" )

    UncertaintyMap.add_commandline_options( parser, with_parent=False )
    SimpsonsPlot.add_commandline_options( parser, with_parent=False )
    ECCollector.add_commandline_options( parser )

    args = parser.parse_args()
    if not os.path.exists(args.out):
        os.mkdir(args.out)

    if not args.mc:
        raise ValueError("mc argument is required for ECGlobal")
    return args


def plot_uncert_map(config, class_type, ec_records):
    plot = UncertaintyMap(ec_records,
                          class_type,
                          'median',
                          config=config,
                          object_group_tag=config.object_group_tag)
    plot.create_all_uncert_map()
    plot.create_jet_uncert_map()
    outname = plot.get_plot_name( config,
                                  class_type=class_type,
                                  object_group_tag=config.object_group_tag,
                                  distribution=config.distribution)
    sub_path = plot.get_subfolder(config)
    plot.plot()
    plot.save_plot( outname, sub_path )
    plot.cleanup()

def plot_simpsons(config, class_type, ec_records):
    plot_classes = []
    if config.simpsons:
        plot_classes.append(SimpsonsPlot)
    if config.simpsons_ratio:
        plot_classes.append(SimpsonsRatioPlot)

    for plot_class in plot_classes:
        plot = plot_class(ec_records,
                            class_type,
                            config=config,
                            object_group_tag=config.object_group_tag)
        plot.plot()
        outname = plot.get_plot_name( config, class_type=class_type)
        sub_path = plot.get_subfolder(config)
        plot.save_plot( outname, sub_path )
        plot.cleanup()

def plot_class_yield(config, class_type, ec_records):
    plot = ClassYieldsPlot(ec_records,
                           class_type,
                           distribution=config.distribution,
                           config=config,
                           object_group_tag=config.object_group_tag,
                           min_yield=config.min_yield)
    outname = plot.get_plot_name( config, config.distribution, class_type=class_type)
    sub_path = plot.get_subfolder(config)
    plot.save_plot( outname, sub_path )
    plot.cleanup()

def plot_per_distribution(func, conf):
    for distribution in ("SumPt", "MET"):
        ec_records = ECCollector.get_records(conf, distribution)
        func(conf, distribution, ec_records)


def plot_dominant_processes(conf, distribution, ec_records):
    for quantile in conf.dominant_process_quantiles:
        plot = ECDominantProcessPlot(conf,
                                     ec_records,
                                     distribution,
                                     quantile=quantile)
        plot.plot()
        outname = plot.get_plot_name( conf,
                                      quantile=quantile,
                                      distribution=distribution)
        sub_path = plot.get_subfolder(conf)
        plot.save_plot(outname, sub_path)
        del plot
        print("Produced dominant process plot for %s" % distribution)


def main():
    """main function of the project."""

    print( r"""
        ____________      __      __          __
       / ____/ ____/___ _/ /___  / /_  ____ _/ /
      / __/ / /   / __ `/ / __ \/ __ \/ __ `/ /
     / /___/ /___/ /_/ / / /_/ / /_/ / /_/ / /
    /_____/\____/\__, /_/\____/_.___/\__,_/_/
                /____/

                                    /'i:zi glo:bal/
    """ )

    conf = parse_arguments()
    cparser = parse_config( conf.config )

    # yield plots need all classes to show the effect of a min yield cut
    tmp_min_yield = conf.min_yield
    if conf.min_yield and conf.class_yields:
        conf.min_yield = 0.

    ec_records = ECCollector.get_records(conf, conf.distribution)
    conf.min_yield = tmp_min_yield
    if not len(ec_records):
        print("No entries found for given criteria")
        return

    stats = {'deficit' :0 , 'excess' : 0}
    for r in ec_records:
        if r['name'].endswith('+X') or r['name'].endswith('+NJets'):
            continue
        if r['scan_skipped']:
            continue
        if r['integral'] > r['data_integral']:
            stats['deficit'] +=1
        else:
            stats['excess'] +=1

    print (stats)

    if conf.class_yields:

        ec_records.record_class_type_loop(plot_class_yield, conf)
        # filter min yield for the following plots
        ec_records = ECRecordList([r for r in ec_records if r['integral'] > conf.min_yield])

    if conf.uncert_map:
        ec_records.record_class_type_loop(plot_uncert_map, conf)

    if conf.simpsons or conf.simpsons_ratio:
        ec_records.record_class_type_loop(plot_simpsons, conf)

    if conf.class_count_summary:
        plot_per_distribution(plot_dominant_processes, conf)


def update_nclasses_tex(conf, scan_type_dict):
    periods = ["fifteen", "sixteen"]
    short_class_types = {'exclusive' : 'excl',
                        'inclusive' : 'incl',
                        'jet-inclusive' : 'jetincl'}

    data_type_dict = {"data" : "_data", "mc": ""}

    count_dict = {}
    for period, class_type, data_type in itertools.product(periods, short_class_types, data_type_dict):
        for suffix in ("", "_met"):
            n_classes = scan_type_dict[class_type]['nclasses{}{}'.format(data_type_dict[data_type] ,suffix)]
            key = 'n{}{}{}{}'.format(short_class_types[class_type],
                                     suffix.replace("_",""),
                                     data_type,
                                     period)
            count_dict[key] = n_classes

    # first read in existing values
    outpath = os.path.join(conf.out, "class_counts.tex")
    original_lines = []
    if os.path.exists(outpath):
        with open(outpath, 'r') as existing_file:
            original_lines = existing_file.read().split('\n')
    with open(outpath, 'w') as new_file:
        # write new infos to file
        for key, count in count_dict.items():

            new_file.write('\\newcommand{\\%s}{%d}\n' % (key, count))
            original_lines = [l for l in original_lines if not key in l]
        # write non updated info to file
        for line in original_lines:
            new_file.write(line + '\n')

if __name__=="__main__":
    main()
