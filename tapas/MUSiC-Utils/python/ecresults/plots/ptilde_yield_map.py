import os
import csv
import random
import string
import math
import numpy as np

import ROOT as ro

from ectools.register import ecroot

from colors import SYSTEMATIC_STYLES, DETAILED_SYSTEMATIC_STYLES

from .collector_base import ECCollectorConsumerPlot

class PtildeYieldMapPlot( ECCollectorConsumerPlot ):
    def __init__( self,
                  ec_records,
                  distribution,
                  class_type,
                  object_group_tag=None,
                  config=None,
                ):
        #cache
        self._ptilde_map = {}

        # call parent constructor
        super( PtildeYieldMapPlot, self ).__init__( ec_records,
                                                config=config,
                                                object_group_tag=object_group_tag)
        # set class attributes
        self.class_type = class_type

        self.distribution = distribution
        self.object_group_tag = object_group_tag
        self.max_jets = 9

        # Filter for object group if passed as argument
        if self.object_group_tag:
            ec_records = ec_records.filter_object_group(self.config,
                                                        object_group_tag)
        self.ec_records = ec_records

    @classmethod
    def get_plot_name(cls,
                      config,
                      class_type='class_type',
                      object_group_tag="",
                      distribution="SumPt"):
        kwargs = {'class_type': class_type,
                  'distribution' : distribution,
                  'object_group' : object_group_tag }
        if object_group_tag:
            name = "PtildeYieldMap_{object_group}_{class_type}_{distribution}".format(**kwargs)
        else:
            name = "PtildeYieldMap_{class_type}_{distribution}".format(**kwargs)
        return name

    @classmethod
    def get_subfolder(cls, config, object_group=None, **kwargs):
        if object_group:
            return "PtildeYieldMap/{}".format(object_group)
        return "PtildeYieldMap"

    def set_cache_dirty(self):
        super(PtildeYieldMapPlot, self).set_cache_dirty()
        self._ptilde_map = None

    @property
    def canvas( self ):
        if self._canvas is None:
            randid = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(4))
            self._canvas = ro.TCanvas( "ptile_yield_map" + self.class_type + randid , "", self.config.canvas_width, self.config.canvas_height )
            self._canvas.UseCurrentStyle()
            self._canvas.SetRightMargin( 0.15 )
            self._canvas.SetLogy(1)
            self._canvas.SetLogx(0)
            self._canvas.cd()
        return self._canvas

    #~ @property
    #~ def _yield_bin_edges(self):
        #~ # get maximum yield
        #~ by_yield = self.ec_records.sort_records('integral')
        #~ bin_edges = [0.001]
        #~ for i in range(int(np.log10(by_yield[0]['integral'])) + 1):
            #~ bin_edges.append(0.001 + 10**i)
        #~ print(min(bin_edges))
        print(bin_edges)
        #~ return np.array(bin_edges, dtype='float64')

    @property
    def _yield_bin_edges(self):
        ''' Property for plot bin edges.

            The x-axis spans several orders of magnitude and the bin edges are
            created to provide 10 bins per magnitude.
        '''
        min_yield = min([r['integral'] for r in self.ec_records])
        max_yield = max([r['integral'] for r in self.ec_records])
        bin_edges = np.array([min_yield, 0.1, 1 , 10, 10000, max_yield], dtype='float64')
        #~ bin_edges = np.linspace(min([r['integral'] for r in self.ec_records]), 1, 10, endpoint=False)

        #~ for i in range(int(np.log10(by_yield[0]['integral'])) + 1):
            #~ bin_edges = np.append(bin_edges, [0.001 + 10**i])
        #~ for i in range(1, int(np.log10(by_yield[0]['integral'])) + 1 ):
            #~ bin_edges = np.concatenate([bin_edges, np.linspace( 10**(i-1), 10**i, 10, endpoint=False)])

        print bin_edges
        return bin_edges

    @property
    def _log_ptilde_bin_edges(self):
        # get maximum yield
        by_p_min_tilde = self.ec_records.sort_records('p-tilde')
        by_p_min_tilde.reverse()
        bin_edges = []
        #~ print("max_ptilde", -1. * math.log10(by_p_min_tilde[0]['p-tilde']))
        max_ptilde = -1. * math.log10(1. / self.ec_records[0]["nrounds"]) + 1
        print("max_ptilde", max_ptilde)
        #~ print(np.array(-1. * np.linspace(-0.0001, by_p_tilde[0]['p-tilde'], 10), dtype='float64'))
        return np.array(np.linspace(0.0001, max_ptilde, 10, endpoint=False), dtype='float64')

    @property
    def ptilde_map(self):
        if self._ptilde_map is None:
            hist_title = "PtildeYieldMap{}".format(self.class_type)
            print(self._log_ptilde_bin_edges)
            print(self._yield_bin_edges)

            xbins = self._log_ptilde_bin_edges
            #~ xbins = np.array(np.linspace(0.001, 10, 10))
            ybins = self._yield_bin_edges
            #~ ybins = np.array(np.linspace(0.001, 10, 10))
            self._ptilde_map = ro.TH2F(hist_title ,
                                       "Ptilde by class yield {}".format(self.class_type),
                                       len(xbins)-1,
                                       xbins,
                                       len(ybins)-1,
                                       ybins)
            self._ptilde_map.UseCurrentStyle()
            self._ptilde_map.GetXaxis().SetTitle("-log_{10}(#tilde{p})")
            self._ptilde_map.GetYaxis().SetTitle("N_{Events} event class")
            self._ptilde_map.GetZaxis().SetTitle("Number of classes")
            for record in reversed(self.ec_records.sort_records("p-tilde")):
                p_tilde = -1. * math.log10(record['p-tilde'] if record['p-tilde'] > 0 else 1.0/record['nrounds'])
                try:
                    print("Fill record", record['name'], p_tilde, record['p-tilde'], record['integral'])
                except:
                    print("Domain Error for", record['p-tilde'])

                self._ptilde_map.Fill(p_tilde, record['integral'])
        return self._ptilde_map


    def plot(self):
        self.canvas.cd()
        self.ptilde_map.Draw("colz")


