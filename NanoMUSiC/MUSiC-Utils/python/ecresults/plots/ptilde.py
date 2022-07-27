#!/bin/env python
from __future__ import print_function

import argparse
from array import array
import math
import os
import sys
import time
import collections
import pickle

import scipy.stats
import ROOT
import numpy as np
import sqlalchemy


start = time.time()
from ectools.register import ecroot
import dbtools
from cmsstyle import set_cms_root_style

from .general import GeneralPlotsBase

z_to_p = scipy.stats.norm.sf

class PtildePlot( GeneralPlotsBase ):
    z_score_to_log_p_dict = {
                              1 : -1.* np.log10( 0.158655 ),
                              2 : -1.* np.log10( 0.02275 ),
                              3 : -1.* np.log10( 0.00135 ),
                              4 : -1.* np.log10( 3.2e-05 ),
                              5 : -1.* np.log10( 0.00001 ),
                            }

    ## one scale factor per bin
    band_scale_dict  = { 'SumPt' : { 'exclusive' : [(1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0)],
                                    'inclusive' :  [(1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0)],
                                    'jet-inclusive' :[(1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0)]},
                         'InvMass' : { 'exclusive' :[(1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0)],
                                    'inclusive' :  [(1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0)],
                                    'jet-inclusive' :[(1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0)]},
                        'MET' : { 'exclusive' :[(1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0)],
                                    'inclusive' :  [(1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0)],
                                    'jet-inclusive' :[(1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0),
                                                    (1.0, 1.0)]},}
    def __init__( self,
                  config,
                  event_classes,
                  session=None,
                  class_type="mixed",
                  distribution=None ):

        self.mc_scan = session.query( dbtools.Scan ).get( config.mc_hash )
        self.data_scan = session.query( dbtools.Scan ).get( config.data_hash ) if config.data_hash else None
        self.signal_scan = session.query( dbtools.Scan ).get( config.signal_hash ) if config.signal_hash else None

        #assert(self.mc_scan.distribution == self.data_scan.distribution)
        if config.signal_hash :
            assert(self.mc_scan.distribution == self.signal_scan.distribution)
        else :
            assert(self.mc_scan.distribution == self.data_scan.distribution)
        super( PtildePlot, self ).__init__(
                  config,
                  lumi=config.lumi,
                  cme=13000 )
        set_cms_root_style()
        # Read configurations
        self.session = session
        #self.scan_type = dbtools.get_scan_type(self.config.data_hash, self.session)
        if config.signal_hash :
            self.scan_type = dbtools.get_scan_type(self.config.signal_hash, self.session)
        else :
            self.scan_type = dbtools.get_scan_type(self.config.data_hash, self.session)
        self.event_classes = list(event_classes)
        self.class_type = class_type
        self.distribution = self.mc_scan.distribution
        # calculate maximal value for -log10( p-tilde) based on nscans
        self.max_log_p_tilde = -np.log10( 1. / self.config.nscans )

        #sar change
        self.nbins = 9
        #self.nbins = 90
        self.binwidth = self.max_log_p_tilde / self.nbins

        self.xlabel = "-log_{10}(#tilde{p})"
        self.ylabel = "number of classes"

        self.control_plot_file = ROOT.TFile( os.path.join( self.config.out, "p_value_control.root" ), 'RECREATE' )

        self.all_mc_hist_control = ROOT.TH1F( "all_mc_control", "MC vs. MC",
                                             self.nbins + 1 ,
                                             0.,
                                             ( self.nbins + 1 ) * self.binwidth )


        # The p-tilde values are stored in lists of lists.
        # The outer list contains one entry per event class, the inner list
        # one entry per round.
        # This format has developed historically and we could replace it
        # by something better if the database idea is further pursued.
        log_signal_p_tilde_lists = None
        if not self.config.data_only:
            if not self.config.use_cache or not os.path.exists("ec_pseudo_p_lists_cache.pkl"):
                log_pseudo_p_tilde_lists = dbtools.collect_p_tilde_list(self.config.mc_hash,
                                                                    self.event_classes,
                                                                    session,
                                                                    assert_nscans=not self.config.filter_p_for_deviation_type)
                if self.config.use_cache:
                    with open("ec_pseudo_p_lists_cache.pkl", "wb") as f:
                        pickle.dump(log_pseudo_p_tilde_lists, f)
            else:
                print("Using class names from cache")
                with open("ec_pseudo_p_lists_cache.pkl", "r") as f:
                    log_pseudo_p_tilde_lists = pickle.load(f)

                assert( len( log_pseudo_p_tilde_lists ) == len( self.event_classes ) )
                if not self.config.filter_p_for_deviation_type:
                    assert( len( log_pseudo_p_tilde_lists[0] ) == self.config.nscans,
                       "Did you forget to run calc-p value?" )
        else:
            log_pseudo_p_tilde_lists = None

        #if not self.config.mc_only:
        if self.config.data_hash and not self.config.mc_only:
            log_data_p_tilde_lists = dbtools.collect_p_tilde_list(self.config.data_hash,
                                                                  self.event_classes,
                                                                  session)
            if self.config.print_included_classes:
                flat_data_list = [l[0] for l in log_data_p_tilde_lists]
                sorted_indices = np.argsort(flat_data_list)
                for i in sorted_indices:
                    print(flat_data_list[i], self.event_classes[i])
            assert( len( log_data_p_tilde_lists ) == len( self.event_classes ) )
        #elif self.config.signal_hash and not self.config.mc_only:
        elif self.config.signal_hash and not self.config.mc_only:
            if not self.config.use_cache or not os.path.exists("ec_signal_p_lists_cache.pkl"):
                log_signal_p_tilde_lists = dbtools.collect_p_tilde_list(self.config.signal_hash,
                                                                    self.event_classes,
                                                                    session,
                                                                    assert_nscans=not self.config.filter_p_for_deviation_type)
                if self.config.use_cache:
                    with open("ec_signal_p_lists_cache.pkl", "wb") as f:
                        pickle.dump(log_signal_p_tilde_lists, f)
            else:
                print("Using class names from cache")
                with open("ec_signal_p_lists_cache.pkl", "r") as f:
                    log_signal_p_tilde_lists = pickle.load(f)
            log_data_p_tilde_lists = None
        else:
            log_data_p_tilde_lists = None

        self.n_classes = len( self.event_classes )

        self.data_vs_mc_hist = None

        self.initalize_legend()

        self.hasData = False
        #if log_data_p_tilde_lists and not self.config.mc_only:
        if (log_data_p_tilde_lists or log_signal_p_tilde_lists) and not self.config.mc_only:
            log_obs_p_tilde_list = None
            if log_data_p_tilde_lists and log_signal_p_tilde_lists:
                raise ValueError("Signal and data hash requested at the same time")
            elif log_data_p_tilde_lists:
                log_obs_p_tilde_list = log_data_p_tilde_lists
            elif log_signal_p_tilde_lists:
                log_obs_p_tilde_list = log_signal_p_tilde_lists
            if not log_obs_p_tilde_list:
                raise ValueError("Neither data nor signal requested")

            if self.config.filter_p_for_deviation_type:
                self.create_obs_vs_mc_hist_per_deviation_type( log_obs_p_tilde_list )
            else:
                self.create_obs_vs_mc_hist( log_obs_p_tilde_list )
            self.hasData = True

        if not self.config.data_only:
            if self.config.filter_p_for_deviation_type:
                self.create_mc_vs_mc_hist_per_deviation_type( log_pseudo_p_tilde_lists )
            else:
                self.create_mc_vs_mc_hist( log_pseudo_p_tilde_lists )
            if not self.config.filter_p_for_deviation_type:
                self.control_plot_file.cd()
                self.all_mc_hist.Write()

        self.create_uniform_distribution()

        #~ self.create_overflow_hists( )
        self.control_plot_file.cd()
        self.control_plot_file.Close()

        self.z_score_lines = []
        self.z_score_labels = []


    @property
    def canvas(self):
        if self._canvas is None:
            self._canvas = ROOT.TCanvas( "p-tilde_%s" % self.distribution,
                                      "#tilde{p} distribution",
                                      self.config.canvas_width,
                                      int( self.config.canvas_height * 0.8 ) )
            self._canvas.UseCurrentStyle()
            self._canvas.SetLogy()
        return self._canvas

    @classmethod
    def get_subfolder(cls, config, distribution="dist"):
        return os.path.join( "Ptilde", distribution )

    @classmethod
    def get_plot_name(cls, config, distribution="Dist", class_type="type", **kwargs):
        plot_name = "Ptilde_{distribution}_{class_type}".format(distribution=distribution,
                                                          class_type=class_type)
        if config.filter_deviation_type:
            plot_name += "_" + config.filter_deviation_type
        if config.integralScan:
            plot_name += "_integralScan"
        return plot_name

    @classmethod
    def ec_names_from_db(cls, scan_hash, session):
        ec_names_query = session.query( dbtools.Result.event_class.label("event_class")) \
                  .filter_by( hash=scan_hash ) \
                  .filter_by( round=0 )
        return [r.event_class for r in ec_names_query]

    def plot( self ):
        ymax = self.uniform_hist.GetBinContent( 1 ) * 8
        ymin = 1. / (2 * len(self.event_classes))

        self.canvas.cd()
        if not self.config.data_only:
            self.total_mc_vs_mc_hist.GetYaxis().SetRangeUser(ymin , ymax)
            self.total_mc_vs_mc_hist.GetXaxis().SetTitle( self.xlabel )
            self.total_mc_vs_mc_hist.GetYaxis().SetTitle( self.ylabel )
            self.total_mc_vs_mc_hist.SetFillStyle( 0 )
            self.total_mc_vs_mc_hist.SetMarkerSize( 2 )
            #~ if self.config.plot_mc_mean:
                #~ print("In Draw mean")
            self.total_mc_vs_mc_hist.Print()
            self.total_mc_vs_mc_hist.Draw()
            if not self.config.filter_p_for_deviation_type:
                self.ci_outer_band.Draw( "2same" )
                self.ci_inner_band.Draw( "2same" )
            self.total_mc_vs_mc_hist.Draw( "same" )
        if self.config.plot_uniform:
            self.uniform_hist.Draw( "same" )

        if not self.config.data_only and not self.config.no_plot_mc_median:
            self.total_mc_vs_mc_hist_median.Draw( "same" )

        if self.hasData and not self.config.mc_only:
            self.total_data_vs_mc_graph.GetXaxis().SetTitle( self.xlabel )
            self.total_data_vs_mc_graph.GetYaxis().SetTitle( self.ylabel )
            self.total_data_vs_mc_graph.SetMaximum( ymax )
            self.total_data_vs_mc_graph.SetMinimum( ymin )
            self.total_data_vs_mc_graph.Draw( "P same" )
            print ("Pulls for {} {}".format(self.distribution, self.class_type))
            for ibin in range(self.total_mc_vs_mc_hist.GetNbinsX() + 1):
                mc_median = self.total_mc_vs_mc_hist_median.GetBinContent(ibin)
                data_median = self.total_data_vs_mc_hist_median.GetBinContent(ibin)
                if mc_median:
                    median_pull = (mc_median - data_median) / mc_median
                else:
                    median_pull = 0.
                mc_mean = self.total_mc_vs_mc_hist.GetBinContent(ibin)
                data_mean = self.total_data_vs_mc_hist.GetBinContent(ibin)
                if mc_mean:
                    mean_pull = (mc_mean - data_mean) / mc_mean
                else:
                    mean_pull = 0.
                print ("ibin: {},Mean: MC: {} Data: {} {} | Median: MC: {} Data: {} {}".format(ibin,
                                                                                               mc_mean,
                                                                                               data_mean,
                                                                                               mean_pull,
                                                                                               mc_median,
                                                                                               data_median,
                                                                                               median_pull))

        self.draw_z_score_lines()
        self.legend.Draw()
        self.header_obs = ecroot.create_tex_header(self.canvas,
                                              header_title=self.config.header_title,
                                              header_subtitle=self.config.header_subtitle,
                                              header_outside=True,
                                              lumi=self.lumi,
                                              cme=self.cme,
                                            )

        for obs in self.header_obs:
            obs.Draw()

    def create_obs_vs_mc_hist(self, log_obs_p_tilde_lists):
        hist_list = []

        root_file = ROOT.TFile( os.path.join( self.config.out, "p_value_signal_val.root" ), 'RECREATE' ) #to be changed
        self.all_data_hist = ROOT.TH1F( "all_signal", "all_signal",
                                     self.nbins + 1 ,
                                     0.,
                                     ( self.nbins + 1 ) * self.binwidth )


        n_signal_scans = len( log_obs_p_tilde_lists[ 0 ] )
        for iscan in range( n_signal_scans ):
            hist_list.append( ROOT.TH1F( "signal_p_tilde_hist_%d" % iscan,
                                       "Signal Pseudo Round %d" % iscan,
                                       self.nbins + 1 ,
                                       0.,
                                       ( self.nbins + 1 ) * self.binwidth ) )
            for p_tilde_list in log_obs_p_tilde_lists:
                hist_list[ iscan ].Fill( p_tilde_list[ iscan ] )
                self.all_data_hist.Fill( p_tilde_list[ iscan ] )

        for hist in hist_list:
            hist.Write()

        self.total_data_vs_mc_hist = ROOT.TH1F( "signal_p_tilde_hist", "Total Data/Signal Hist",
                                            self.nbins + 1 ,
                                            0.,
                                            ( self.nbins + 1 ) * self.binwidth )
        self.total_data_vs_mc_hist_median = ROOT.TH1F( "signal_p_tilde_hist_median", "Median Data/Signal Hist",
                                            self.nbins + 1 ,
                                            0.,
                                            ( self.nbins + 1 ) * self.binwidth )
        nbins = self.total_data_vs_mc_hist.GetNbinsX()

        n_classes                  = array( "f" )
        n_classes_median           = array( "f" )
        n_classes_inner_up         = array( "f" )
        n_classes_inner_down       = array( "f" )
        n_classes_outer_up         = array( "f" )
        n_classes_outer_down       = array( "f" )
        zeros                      = array( "f" )
        log10_p_value_center       = array( "f" )
        log10_p_value_half_width   = array( "f" )
        ci_tail_inner = ( 1. - self.config.inner_band_ci ) / 2
        ci_tail_outer = ( 1. - self.config.outer_band_ci ) / 2

        per_bin_file = ROOT.TFile( os.path.join( self.config.out, "per_bin_distributions.root" ), "RECREATE" )
        for ibin in range( nbins ):
            bin_value_list = []
            # Fill the mean in all histograms per bin
            for hist in hist_list:
                bin_value_list.append( hist.GetBinContent( ibin + 1 ) )
            mean_n_classes = np.mean( bin_value_list )
            median_n_classes = np.median( bin_value_list )
            n_classes.append( mean_n_classes )
            n_classes_median.append( median_n_classes )
            self.total_data_vs_mc_hist.SetBinContent( ibin + 1, mean_n_classes )
            self.total_data_vs_mc_hist.SetBinError( ibin + 1, 0 )
            self.total_data_vs_mc_hist_median.SetBinContent( ibin + 1, median_n_classes )
            self.total_data_vs_mc_hist_median.SetBinError( ibin + 1, 0 )
            if len( bin_value_list ) > 1:
                inner_up_value = max( 0, self.get_percentile( bin_value_list, 1. - ci_tail_inner ) - median_n_classes )
                inner_down_value = median_n_classes - self.get_percentile( bin_value_list, ci_tail_inner )
                outer_up_value = max( 0, self.get_percentile( bin_value_list, 1. - ci_tail_outer ) - median_n_classes )
                outer_down_value = median_n_classes - self.get_percentile( bin_value_list, ci_tail_outer )

                n_classes_inner_up.append( inner_up_value )
                n_classes_inner_down.append( inner_down_value )
                n_classes_outer_up.append( outer_up_value )
                n_classes_outer_down.append( outer_down_value )

            else:
                n_classes_inner_up.append( 0. )
                n_classes_inner_down.append( 0. )
                n_classes_outer_up.append( 0. )
                n_classes_outer_down.append( 0. )
            log10_p_value_center.append( self.total_data_vs_mc_hist.GetBinCenter( ibin + 1 ) )
            log10_p_value_half_width.append( self.total_data_vs_mc_hist.GetBinWidth( ibin + 1 ) / 2. )
            zeros.append( 0. )
            per_bin_hist = ROOT.TH1F( "bin" + str( ibin + 1 ), "bin" + str( ibin + 1 ), 1000, 0, 1000 )
            for i, val in enumerate( bin_value_list ):
                per_bin_hist.Fill( val )
            per_bin_hist.Write()
        per_bin_file.Close()
        self.total_data_vs_mc_graph = ROOT.TGraphAsymmErrors( nbins,
                                                      log10_p_value_center,
                                                      n_classes_median,
                                                      log10_p_value_half_width,
                                                      log10_p_value_half_width,
                                                      n_classes_inner_down,
                                                      n_classes_inner_up )
        self.total_data_vs_mc_graph.UseCurrentStyle()
        self.total_data_vs_mc_graph.SetMarkerSize( 0 )
        self.total_data_vs_mc_graph.SetLineWidth( 3 )
        self.total_data_vs_mc_graph.SetLineColor( 2 )
        self.ci_inner_band = ROOT.TGraphAsymmErrors( nbins,
                                                    log10_p_value_center,
                                                    n_classes_median,
                                                    log10_p_value_half_width,
                                                    log10_p_value_half_width,
                                                    n_classes_inner_down,
                                                    n_classes_inner_up )
        self.ci_outer_band = ROOT.TGraphAsymmErrors( nbins,
                                                    log10_p_value_center,
                                                    n_classes_median,
                                                    log10_p_value_half_width,
                                                    log10_p_value_half_width,
                                                    n_classes_outer_down,
                                                    n_classes_outer_up )
        if self.scan_type == "signalScan":
            self.legend.AddEntry( self.total_data_vs_mc_graph,
                                  "Median signal rounds", "LP" )
        elif self.scan_type == "dataScan":
            self.legend.AddEntry( self.total_data_vs_mc_graph,
                                  "observed deviations", "LP" )
        root_file.Close()


    def create_class_control_plots( self, event_class ):
        class_control_hist = ROOT.TH1F( "pseudo_p_tilde_hist_%s" % event_class,
                                       "MC vs. MC %s" % event_class,
                                       self.nbins + 1 ,
                                       0.,
                                       ( self.nbins + 1 ) * self.binwidth )

        query = self.session.query( dbtools.CorrectedResult ) \
            .filter_by( hash=self.config.mc_hash, event_class=event_class )

        for result in query:
            log10 = math.log10(result.score if result.score > 0 else 1.0/result.comp_count)
            class_control_hist.Fill(log10)

        self.all_mc_hist_control.Add( class_control_hist )

        class_control_hist.Write()


    def create_overflow_hists(self):
        """ creates hists consisting only of the overflow bin with red color """
        if self.hasData:
            self.data_vs_mc_overflow_hist = self.data_vs_mc_hist.Clone()
            self.data_vs_mc_overflow_hist.SetFillColor( ROOT.kRed )

        if self.config.data_only:
            nbins = self.data_vs_mc_overflow_hist.GetNbinsX()
        else:
            self.mc_vs_mc_overflow_hist = self.total_mc_vs_mc_hist.Clone()
            self.mc_vs_mc_overflow_hist.SetFillColor( 46 )
            nbins = self.mc_vs_mc_overflow_hist.GetNbinsX()
        for ibin in range( nbins ):
            if not self.config.data_only:
                if self.mc_vs_mc_overflow_hist.GetBinLowEdge( ibin + 1 ) < self.max_log_p_tilde:
                    self.mc_vs_mc_overflow_hist.SetBinContent( ibin + 1, 0. )
            if self.hasData:
                if self.data_vs_mc_overflow_hist.GetBinLowEdge( ibin + 1 ) < self.max_log_p_tilde:
                    self.data_vs_mc_overflow_hist.SetBinContent( ibin + 1, 0. )


    def get_percentile( self, list, ci ):
        counter = 0
        list.sort()
        for i, val in enumerate( list ):
            counter += val
            if float( i ) / len( list ) > ci:
                return val


    def create_obs_vs_mc_hist_per_deviation_type(self, log_obs_p_tilde_lists):

        self.total_data_vs_mc_hist = ROOT.TH1F( "signal_p_tilde_hist", "Total Data/Signal Hist",
                                            self.nbins + 1 ,
                                            0.,
                                            ( self.nbins + 1 ) * self.binwidth )
        self.total_data_vs_mc_hist_median = ROOT.TH1F( "signal_p_tilde_hist_median", "Median Data/Signal Hist",
                                            self.nbins + 1 ,
                                            0.,
                                            ( self.nbins + 1 ) * self.binwidth )
        nbins = self.total_data_vs_mc_hist.GetNbinsX()

        for class_list in log_obs_p_tilde_lists:
            print ("Obs", "mean", np.mean( class_list ), "median", np.median( class_list))
            self.total_data_vs_mc_hist.Fill(np.mean( class_list ))
            self.total_data_vs_mc_hist_median.Fill(np.median( class_list ))

        n_classes                  = array( "f" )
        n_classes_median           = array( "f" )
        n_classes_inner_up         = array( "f" )
        n_classes_inner_down       = array( "f" )
        log10_p_value_center       = array( "f" )
        log10_p_value_half_width   = array( "f" )

        for ibin in range( nbins+1 ):
            n_classes.append(self.total_data_vs_mc_hist.GetBinContent(ibin))
            n_classes_median.append(self.total_data_vs_mc_hist_median.GetBinContent(ibin))
            n_classes_inner_up.append(0)
            n_classes_inner_down.append(0)
            log10_p_value_center.append( self.total_data_vs_mc_hist.GetBinCenter( ibin + 1 ) )

            log10_p_value_half_width.append( self.total_data_vs_mc_hist.GetBinWidth( ibin + 1 ) / 2. )
        self.total_data_vs_mc_graph = ROOT.TGraphAsymmErrors( nbins,
                                                      log10_p_value_center,
                                                      n_classes_median,
                                                      log10_p_value_half_width,
                                                      log10_p_value_half_width,
                                                      n_classes_inner_down,
                                                      n_classes_inner_up )
        self.total_data_vs_mc_graph.UseCurrentStyle()
        self.total_data_vs_mc_graph.SetMarkerSize( 0 )
        self.total_data_vs_mc_graph.SetLineWidth( 3 )
        self.total_data_vs_mc_graph.SetLineColor( 2 )

        if self.scan_type == "signalScan":
            self.legend.AddEntry( self.total_data_vs_mc_graph,
                                  "Median signal rounds", "LP" )
        elif self.scan_type == "dataScan":
            self.legend.AddEntry( self.total_data_vs_mc_graph,
                                  "observed deviations", "LP" )


    def create_mc_vs_mc_hist_per_deviation_type(self, log_pseudo_p_tilde_lists):
        self.total_mc_vs_mc_hist = ROOT.TH1F( "pseudo_p_tilde_hist", "Total MC Hist",
                                    self.nbins + 1 ,
                                    0.,
                                    ( self.nbins + 1 ) * self.binwidth )

        self.total_mc_vs_mc_hist_median = ROOT.TH1F( "pseudo_p_tilde_hist_median", "Median MC Hist",
                                            self.nbins + 1 ,
                                            0.,
                                            ( self.nbins + 1 ) * self.binwidth )

        nbins = self.total_mc_vs_mc_hist.GetNbinsX()

        for class_list in log_pseudo_p_tilde_lists:
            print ("EXP", "mean", np.mean( class_list ), "median", np.median( class_list))
            self.total_mc_vs_mc_hist.Fill(np.mean( class_list ))
            self.total_mc_vs_mc_hist_median.Fill(np.median( class_list ))

        self.total_mc_vs_mc_hist.UseCurrentStyle()
        self.total_mc_vs_mc_hist.SetLineColor( 432 )
        self.total_mc_vs_mc_hist.SetLineWidth( 4 )

        self.total_mc_vs_mc_hist_median.UseCurrentStyle()
        self.total_mc_vs_mc_hist_median.SetLineColor( 431 )
        self.total_mc_vs_mc_hist_median.SetLineWidth( 4 )
        self.total_mc_vs_mc_hist_median.SetLineStyle( 3 )

        self.legend.AddEntry( self.total_mc_vs_mc_hist,
                              "mean SM-pseudo rounds", "L" )
        self.legend.AddEntry( self.total_mc_vs_mc_hist_median,
                              "median SM-pseudo rounds", "L" )

    def create_mc_vs_mc_hist( self, log_pseudo_p_tilde_lists ):
        hist_list = []
        root_file = ROOT.TFile( os.path.join( self.config.out, "p_value_val.root" ), 'RECREATE' )
        self.all_mc_hist = ROOT.TH1F( "all_mc", "all_mc",
                                     self.nbins + 1 ,
                                     0.,
                                     ( self.nbins + 1 ) * self.binwidth )
        test = len( log_pseudo_p_tilde_lists[ 0 ] )
        for iscan in range( len( log_pseudo_p_tilde_lists[ 0 ] ) ):
            hist_list.append( ROOT.TH1F( "pseudo_p_tilde_hist_%d" % iscan,
                                       "BG Pseudo Round %d" % iscan,
                                       self.nbins + 1 ,
                                       0.,
                                       ( self.nbins + 1 ) * self.binwidth ) )
            for p_tilde_list in log_pseudo_p_tilde_lists:
                hist_list[ iscan ].Fill( p_tilde_list[ iscan ] )
                self.all_mc_hist.Fill( p_tilde_list[ iscan ] )
        for hist in hist_list:
            hist.Write()
        self.total_mc_vs_mc_hist = ROOT.TH1F( "pseudo_p_tilde_hist", "Total MC Hist",
                                            self.nbins + 1 ,
                                            0.,
                                            ( self.nbins + 1 ) * self.binwidth )

        self.total_mc_vs_mc_hist_median = ROOT.TH1F( "pseudo_p_tilde_hist_median", "Median MC Hist",
                                            self.nbins + 1 ,
                                            0.,
                                            ( self.nbins + 1 ) * self.binwidth )
        nbins = self.total_mc_vs_mc_hist.GetNbinsX()

        n_classes                  = array( "f" )
        n_classes_median           = array( "f" )
        n_classes_inner_up         = array( "f" )
        n_classes_inner_down       = array( "f" )
        n_classes_outer_up         = array( "f" )
        n_classes_outer_down       = array( "f" )
        zeros                      = array( "f" )
        log10_p_value_center       = array( "f" )
        log10_p_value_half_width   = array( "f" )
        ci_tail_inner = ( 1. - self.config.inner_band_ci ) / 2
        ci_tail_outer = ( 1. - self.config.outer_band_ci ) / 2

        per_bin_file = ROOT.TFile( os.path.join( self.config.out, "per_bin_distributions.root" ), "RECREATE" )

        for ibin in range( nbins ):
            bin_value_list = []
            # Fill the mean in all histograms per bin
            for hist in hist_list:
                bin_value_list.append( hist.GetBinContent( ibin + 1 ) )
            mean_n_classes = np.mean( bin_value_list )
            median_n_classes = np.median( bin_value_list )
            self.total_mc_vs_mc_hist.SetBinContent( ibin + 1, mean_n_classes )
            self.total_mc_vs_mc_hist.SetBinError( ibin + 1, 0 )
            self.total_mc_vs_mc_hist_median.SetBinContent( ibin + 1, median_n_classes )
            self.total_mc_vs_mc_hist_median.SetBinError( ibin + 1, 0 )
            n_classes.append( mean_n_classes )
            n_classes_median.append( median_n_classes )

            inner_up_value_raw = max( 0, self.get_percentile( bin_value_list, 1. - ci_tail_inner ) - median_n_classes )
            inner_down_value_raw = median_n_classes - self.get_percentile( bin_value_list, ci_tail_inner )
            outer_up_value_raw = max( 0, self.get_percentile( bin_value_list, 1. - ci_tail_outer ) - median_n_classes )
            outer_down_value_raw = median_n_classes - self.get_percentile( bin_value_list, ci_tail_outer )

            if self.config.scale_bands:
                inner_up_value = inner_up_value_raw * self.band_scale_dict[self.distribution][self.class_type][ibin][0]
                inner_down_value = inner_down_value_raw * self.band_scale_dict[self.distribution][self.class_type][ibin][1]
                outer_up_value = outer_up_value_raw * self.band_scale_dict[self.distribution][self.class_type][ibin][0]
                outer_down_value = outer_down_value_raw * self.band_scale_dict[self.distribution][self.class_type][ibin][1]
            else:
                inner_up_value = inner_up_value_raw
                inner_down_value = inner_down_value_raw
                outer_up_value = outer_up_value_raw
                outer_down_value = outer_down_value_raw
            n_classes_inner_up.append( inner_up_value )
            n_classes_inner_down.append( inner_down_value )
            n_classes_outer_up.append( outer_up_value )
            n_classes_outer_down.append( outer_down_value )


            log10_p_value_center.append( self.total_mc_vs_mc_hist.GetBinCenter( ibin + 1 ) )
            log10_p_value_half_width.append( self.total_mc_vs_mc_hist.GetBinWidth( ibin + 1 ) / 2. )
            zeros.append( 0. )
            per_bin_hist = ROOT.TH1F( "bin" + str( ibin + 1 ), "bin" + str( ibin + 1 ), 1000, 0, 1000 )
            for i, val in enumerate( bin_value_list ):
                per_bin_hist.Fill( val )
            per_bin_hist.Write()
        per_bin_file.Close()
        self.total_mc_vs_mc_hist.UseCurrentStyle()
        self.ci_inner_band = ROOT.TGraphAsymmErrors( nbins,
                                                   log10_p_value_center,
                                                   n_classes_median,
                                                   log10_p_value_half_width,
                                                   log10_p_value_half_width,
                                                   n_classes_inner_down,
                                                   n_classes_inner_up )
        self.ci_outer_band = ROOT.TGraphAsymmErrors( nbins,
                                                   log10_p_value_center,
                                                   n_classes_median,
                                                   log10_p_value_half_width,
                                                   log10_p_value_half_width,
                                                   n_classes_outer_down,
                                                   n_classes_outer_up )

        self.ci_inner_band.UseCurrentStyle()
        self.ci_outer_band.UseCurrentStyle()

        self.total_mc_vs_mc_hist.SetLineColor( 432 )
        self.total_mc_vs_mc_hist.SetLineWidth( 4 )
        self.total_mc_vs_mc_hist_median.SetLineColor( 431 )
        self.total_mc_vs_mc_hist_median.SetLineWidth( 4 )
        self.total_mc_vs_mc_hist_median.SetLineStyle( 3 )
        self.ci_inner_band.SetFillColorAlpha( 56, 0.3 )
        self.ci_outer_band.SetFillColorAlpha( 67, 0.3 )

        #~ if self.config.plot_mc_mean:
        self.legend.AddEntry( self.total_mc_vs_mc_hist,
                              "mean SM-pseudo rounds", "L" )
        #~ if self.config.no_plot_mc_median:
        self.legend.AddEntry( self.total_mc_vs_mc_hist_median,
                              "median SM-pseudo rounds", "L" )
        self.legend.AddEntry( self.ci_inner_band,
                              "%d%% of SM-pseudo rounds" % int( 100 * self.config.inner_band_ci ), "F" )
        self.legend.AddEntry( self.ci_outer_band,
                              "%d%% of SM-pseudo rounds" % int( 100 * self.config.outer_band_ci ), "F" )
        root_file.Close()

    def create_uniform_distribution( self ):
        self.uniform_hist = ROOT.TH1F( "uniform", "a uniform unicorn",
                                     self.nbins + 1 ,
                                     0.,
                                     ( self.nbins + 1 ) * self.binwidth )

        self.uniform_hist.SetLineWidth( 3 )
        self.uniform_hist.SetLineStyle( 9 )
        self.uniform_hist.SetLineColor( 417 )
        for ibin in range( 1, self.nbins + 2 ):
            self.uniform_hist.SetBinContent( ibin, ( -10**( -self.uniform_hist.GetXaxis().GetBinUpEdge( ibin ) ) + 10**( -self.uniform_hist.GetBinLowEdge( ibin ) ) ) * self.n_classes )
        if self.config.plot_uniform:
            self.legend.AddEntry( self.uniform_hist, "uniform distribution", "L" )

    def init_text_sizes( self ):
        super( PtildePlot, self ).init_text_sizes()
        #self.text_position_x_header = 0.65

    def initalize_legend( self ):
        distributions = ecroot.distribution_types()
        for distribution in distributions:
            if distribution.name == self.distribution:
                margin = 0.004
                class_type_info = ecroot.get_class_type_info_by_type_name(self.class_type)
                legend_title = class_type_info.root_tag + "classes, " + distribution.root_tag + " distributions"
                self.legend = ROOT.TLegend( self.canvas_left + margin,
                                            self.canvas_bottom + margin,
                                            0.48 + 0.4 * (1. - (1400. - self.config.canvas_width) / 1400.),
                                            0.45 ,
                                            legend_title, "NDC" )
                self.legend.SetTextSize( 0.04 )
                self.legend.SetFillStyle( 1001 )
                self.legend.SetLineColor( 0 )
                return
        else:
            raise ValueError( "Could not find label for distribution %s" % self.distribution )

    def draw_z_score_lines( self ):
        for z in range( 1, int( self.max_log_p_tilde ) ):
            log10p = -math.log10( z_to_p( z ) )

            lines, labels = ecroot.create_line_markers( self,
                                        log10p,
                                        "#color[4]{%d#sigma}" % z,
                                        0.85 )

            # set some styling
            for line in lines:
                line.SetLineColor( 4 )
                line.SetLineStyle( 2 )
                line.SetLineWidth( 3 )
                line.Draw()
            self.z_score_lines.extend( lines )

            for label in labels:
                label.SetTextColor( 4 )
                label.Draw()
            self.z_score_labels.extend( labels )

    @classmethod
    def set_skip_reason(cls,
                        session,
                        data_hash,
                        mc_hash,
                        nscans=None,
                        min_yield=0,
                        ec_names=[],
                        filter_deviation_type=None, # deviation type either "deficit" or "excess"
                        skip_no_data=False):
        # determine reasons why a certain event class would be skipped

        all_ecs_query = session.query( dbtools.Result.event_class.label( "event_class" ),  dbtools.Result.skipped.label( "skipped" )) \
                       .filter_by( hash=mc_hash, round=0 ) \
                       .group_by( dbtools.Result.event_class )
        print("Setting skip reasons")
        # Note: here we're skipping *MC* event classes based on the number of correction rounds
        if nscans is not None:
            all_ecs_corrected_query = session.query( dbtools.CorrectedResult.event_class.label( "event_class" ) ) \
                       .filter_by( hash=mc_hash, comp_count = nscans ) \
                       .group_by( dbtools.CorrectedResult.event_class )

            all_ecs_corrected = set(r.event_class for r in all_ecs_corrected_query if r.event_class in ec_names)
            not_corrected = set(ec_names) - all_ecs_corrected
            ecs_missing_scans = not_corrected
        else:
            ecs_missing_scans = []
        ecs_missing_scans = []


        query = all_ecs_query.filter_by( skipped=1 )
        skipped_scans = [ row.event_class for row in query if row.event_class in ec_names]
        # Note: here we're skipping *data* event classes based on the number of SM/BG events
        # in the event class.
        query = all_ecs_query.filter( dbtools.Result.ec_total_mc_events < min_yield )

        ecs_too_low_mc = [ row.event_class for row in query if row.event_class in ec_names]

        query_with_data = all_ecs_query.filter( dbtools.Result.ec_total_mc_events < min_yield, dbtools.Result.ec_total_data_events > 0 )

        ecs_without_data = []
        if skip_no_data:
            #~ query_with_data_all = all_ecs_query.filter(  dbtools.Result.data_events > 0 )
            query_with_data_all = session.query( dbtools.Result.event_class.label( "event_class" )) \
                       .filter_by( hash=data_hash, round=0 ) \
                       .filter(  dbtools.Result.data_events > 0 ) \
                       .group_by( dbtools.Result.event_class )
            all_ecs_with_data = [ row.event_class for row in query_with_data_all]
            ecs_without_data = list(set(ec_names) - set(all_ecs_with_data))


        ecs_too_low_mc_but_data = [ row.event_class for row in query_with_data]

        ecs_wrong_deviation_type = []
        if filter_deviation_type:

            if filter_deviation_type == "excess":
                print("Filtering for %s" % filter_deviation_type)
                query = session.query( dbtools.Result.event_class.label( "event_class" ) ) \
                       .filter( dbtools.Result.data_events < dbtools.Result.mc_events) \
                       .filter_by( hash=data_hash ) \
                       .group_by( dbtools.Result.event_class )
            elif filter_deviation_type == "deficit":
                print( "Filtering for %s" % filter_deviation_type)
                query = session.query( dbtools.Result.event_class.label( "event_class" ) ) \
                       .filter( dbtools.Result.data_events > dbtools.Result.mc_events) \
                       .filter_by( hash=data_hash ) \
                       .group_by( dbtools.Result.event_class )
            ecs_wrong_deviation_type = [ row.event_class for row in query if row.event_class in ec_names]
            print(ecs_wrong_deviation_type)

        result = {}
        for ec in ecs_missing_scans:
            result[ec] = "missing scan"
        for ec in ecs_too_low_mc:
            result[ec] = "too small MC yield"
        for ec in skipped_scans:
            result[ec] = "Scan skipped in scanner"
        for ec in ecs_wrong_deviation_type:
            result[ec] = "Scan not an %s" % filter_deviation_type
        for ec in ecs_without_data:
            result[ec] = "Scan without data in RoI"

        for ec in ecs_too_low_mc_but_data:
            print("Warning: Event Class %s has too low MC yield, but contains data." % ec)
            #result[ec] = "too small MC yield, but data"

        all_ecs = set( r[0] for r in all_ecs_query if r[0] in ec_names)
        vetoed = set( result.keys() )
        not_vetoed = all_ecs - vetoed

        print( "Done: %d / %d vetoed" % ( len( vetoed ), len( all_ecs ) ) )

        return result, not_vetoed


    @classmethod
    def add_commandline_options( cls, parser ):
        group = GeneralPlotsBase.add_commandline_options( parser )

        group.add_argument( "--database", help="Result database to analyze.", type=str, default=dbtools.default_db_name )

        #~ group.add_argument( "--scan-id", help="Id for the scan hashs are matched automatically.", type=str, required=True )
        group.add_argument( "--mc-hash", help="Hash of BG-only scan. Alternative way if fixed hash should be used", type=str )
        group.add_argument( "--data-hash", help="Hash of data scan. Alternative way if fixed hash should be used", type=str )
        group.add_argument( "--signal-hash", help="Hash of signal scan. Alternative way if fixed hash should be used", type=str )
        group.add_argument( '--data-only', action="store_true", help="Plot only Data vs. MC distribution" )
        group.add_argument( '--mc-only', action="store_true", help="Plot only MC vs. MC distribution" )
        group.add_argument( '--no-plot-mc-median', action="store_true", help="Plot the median line for MC vs. MC" )
        group.add_argument( '--plot-uniform', action="store_true", help="Plot the expected uniform distribution MC vs. MC" )
        group.add_argument( '--plot-mc-mean', action="store_true", help="Plot the median line for MC vs. MC" )
        group.add_argument( '--n-ranking', default=15, type=int, help="Number of most significant classes to list" )
        group.add_argument( '--inner-band-ci', type=float, default=0.68 )
        group.add_argument( '--outer-band-ci', type=float, default=0.95 )
        group.add_argument( '--scale-bands', action="store_true", help="Use the hardcoded value maps to scale the binwise scale the uncert bands" )
        group.add_argument( '--filter-deviation-type', choices=["excess", "deficit"], default=None )
        group.add_argument( '--filter-no-data', action="store_true", help="Skip all classes without data in the RoI" )
        group.add_argument( '--filter-p-for-deviation-type', action="store_true" )
        group.add_argument( '--print-included-classes', action="store_true", help="Print all included classes and their p-value" )
        group.add_argument( '--nscans', type=int, default=0 )
        group.add_argument( '--integralScan', action="store_true" )
        group.add_argument( '--use-cache', action="store_true" )
        ecroot.add_ec_standard_options( group )

        return group

