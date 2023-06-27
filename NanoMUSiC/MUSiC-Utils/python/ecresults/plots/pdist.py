import os
import ROOT as ro

from cmsstyle import exponent_notation, convert_tlatex_to_tbox
from .collector_base import ECCollectorConsumerPlot
from ectools.register import ecroot


class PdistPlot(ECCollectorConsumerPlot):

    def __init__( self, ec_records, class_type, config=None, object_group_tag=None):
        # call parent constructor
        super( PdistPlot, self ).__init__( ec_records,
                                           config=config,
                                           object_group_tag=object_group_tag,
                                           class_type=class_type)

        # check if we show p values and therefore need to differentiate between skipped scans
        if os.path.exists(self.config.scan_db_path) and self.config.scan_id and (self.config.show_p_value or self.config.sort_option == "p-value"):
            self.ec_records = self.ec_records.filter_skipped(invert=self.config.show_skipped_classes)

        # check if we use a requirement for the minimal p-value
        if self.config.min_p_value:
            self.ec_records = self.ec_records.filter_gt("p-value",
                                                        self.config.min_p_value,
                                                        allow_equal=True)

        self._legend = None
        self._uniform_hist = None

    def set_cache_dirty(self):
        super(PdistPlot, self).set_cache_dirty()
        self._p_hist = None
        self._legend = None
        self._uniform_hist = None

    @property
    def legend(self):
        if self._legend is None:
            self._legend = ro.TLegend(0.18, 0.75, 0.93, 0.91)
            self._legend.UseCurrentStyle()
            self._legend.SetNColumns(2)
            self._legend.SetFillStyle(0)
        return self._legend

    @property
    def p_hist(self):
        if self._p_hist is None:
            self._p_hist = ro.TH1F( "pdist",
                                   "p distribution;p;number of classes",
                                   self.config.nbins,
                                   0,
                                   1 + 0.01 )
            self._p_hist.UseCurrentStyle()
            self._p_hist.SetLineColor( 2 )
            self._p_hist.SetLineWidth( 3 )
            n = 0
            for record in self.ec_records:
                self._p_hist.Fill(record["p-value"])
                n += 1
            print ("Filled %d in p_hist" % n)
        return self._p_hist

    @property
    def uniform_hist(self):
        if self._uniform_hist is None:
            self._uniform_hist = ro.TH1F( "pdist",
                                   "p distribution;p;number of classes",
                                   self.config.nbins,
                                   0,
                                   0.6 )
            self._uniform_hist.UseCurrentStyle()
            self._uniform_hist.SetLineColor( 417 )
            self._uniform_hist.SetLineWidth( 3 )
            self._uniform_hist.SetLineStyle( 9 )
            print (self.class_type, len(self.ec_records), len(self.ec_records) / self.config.nbins)
            filled = 0
            bin_value = len(self.ec_records) / self.config.nbins
            for ibin in range(self.config.nbins + 1):
                self._uniform_hist.SetBinContent(ibin, bin_value)
                filled += bin_value
            if filled - self.p_hist.Integral() > 0.001:
                print ("Filled does not match integral", filled, self.p_hist.Integral())
        return self._uniform_hist

    def plot(self):
        self.canvas.cd()
        self.canvas.SetLogy(0)
        self.p_hist.Draw("hist")
        self.uniform_hist.Draw("hist same")
        self.legend.AddEntry( self.p_hist, "observed", "L" )
        self.legend.AddEntry( self.uniform_hist, "uniform distribution", "L" )

    @classmethod
    def get_plot_name(cls, config, class_type='class_type', distribution="",object_group_tag=""):
        kwargs = {'class_type': class_type,
                  'object_group' : object_group_tag,
                  'distribution' : distribution }
        if object_group_tag:
            name = "Pdist_{object_group}_{class_type}_{distribution}_{sort_option}".format(**kwargs)
        else:
            name = "Pdist_{class_type}_{distribution}".format(**kwargs)

        return name

    @classmethod
    def get_subfolder(cls, config, object_group=None, **kwargs):
        if object_group:
            return "Pdist/{}".format(object_group)
        return "Pdist"


    @classmethod
    def add_commandline_options( cls, parser, with_parent=True ):
        super(cls, PdistPlot).add_commandline_options(parser, with_parent=with_parent)
        group = parser.add_argument_group( title="p-value distribution plotting options" )
        group.add_argument( "--nbins", default=10,
            help="Number of bins to use in p-value distribution" )
