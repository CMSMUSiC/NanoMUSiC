import operator
import ROOT as ro

from colors import PROCESS_GROUP_STYLES
from roothelpers import TextAlign, Font, root_latex_convert, get_dummy_hist
from cmsstyle import set_cms_root_style

from ectools.register import ecroot

#~ import ecresults
from .collector_base import ECCollectorConsumerPlot

from ecresults.util import ClassCountMixin

class ECDominantProcessPlot(ECCollectorConsumerPlot, ClassCountMixin):

    def __init__(self,
                 config,
                 ec_records,
                 distribution,
                 quantile=0.5,
                 object_group_tag=None ):
        # call parent constructor
        super( ECDominantProcessPlot, self ).__init__( ec_records,
                                                       config=config,
                                                       object_group_tag=object_group_tag)
        self.quantile = quantile
        self.distribution = distribution
        self.header_obs = []

        self.has_data = False

        # cached variables
        self._ec_type_records = None
        self._canvas = None
        self._legend = None
        self._mc_stack = None
        self._mc_hists = []
        self._data_hist = None
        self._total_frac_dict = None
        self._class_type_records = None
    def __del__(self):
        print("Cleaning up dominant process plot")
        if self._mc_stack:
            self._mc_stack.Delete()
        if self._data_hist:
            self._data_hist.Delete()
        for hist in self._mc_hists:
            hist.Delete()
        if self._canvas:
            self._canvas.Delete()

    @property
    def dominant_process_key(self):
        return self.get_dominant_process_key(self.quantile, self.distribution)

    @classmethod
    def get_subfolder(cls, config, object_group=None, **kwargs):
        if object_group:
            return "DominantProcesses/{}".format(object_group)
        return "DominantProcesses"

    @classmethod
    def get_plot_name(cls,
                      config,
                      quantile=0.5,
                      distribution="SumPt"):
        return cls.get_dominant_process_key(quantile, distribution)

    @property
    def canvas(self):
        if self._canvas is None:
            self._canvas = ro.TCanvas(self.dominant_process_key)
            self._canvas.UseCurrentStyle()
            self._canvas.SetLogy(0)
            self._canvas.SetLogx(0)
        return self._canvas

    @property
    def suffix(self):
        return self.ec_records.suffix

    @property
    def legend(self):
        if self._legend is None:
            self._legend = ro.TLegend(0.18, 0.75, 0.93, 0.91)
            self._legend.UseCurrentStyle()
            self._legend.SetNColumns(3)
            self._legend.SetFillStyle(0)
        return self._legend

    @property
    def mc_stack(self):
        if self._mc_stack is None:
            self._mc_stack = ro.THStack("stack" + self.suffix,
                               "Dominant processes by class type")
            self._add_mc_class_type_records(self.class_type_records)
        return self._mc_stack

    @property
    def data_hist(self):
        if self._data_hist is None:
            self._data_hist = ro.TH1F()
            self._add_data_class_type_records(self.class_type_records)
        return self._data_hist

    @data_hist.setter
    def data_hist(self, value):
        self._data_hist = value

    @property
    def class_type_records(self):
        if self._class_type_records is None:
            records = self.ec_records.get_class_type_dominant_process_stats(
                                                                self.config,
                                                                self.quantile)
            self._class_type_records = records
        return self._class_type_records

    def plot(self):
        self.canvas.cd()
        if self.has_data and self.config.dominant_process_show_data:
            self.mc_stack.SetMaximum( max([ infodict["nclasses{}".format(self.suffix)] for t,infodict in self.class_type_records.items() ] ) * 1.35 )
        else:
            self.mc_stack.SetMaximum( max([ infodict["nclasses{}".format(self.suffix)] for t,infodict in self.class_type_records.items() ] ) * 1.35 )

        self.dummy_hist = get_dummy_hist([self.mc_stack])
        self.dummy_hist.Draw()
        self.dummy_hist.GetXaxis().SetLabelSize(0.09)
        self.dummy_hist.GetYaxis().SetTitle( "number of classes" )
        self.dummy_hist.GetYaxis().SetTitleOffset(1.8)
        self.dummy_hist.GetXaxis().SetTickLength(0.)

        #~ self.mc_stack
        self.mc_stack.Draw("hist")
        self.mc_stack.GetXaxis().SetLabelSize(0.08)
        if self.has_data and self.config.dominant_process_show_data:
            self.data_hist.Draw("samePE")

        self.legend.Draw()
        ro.gPad.RedrawAxis()

        self._header_obs = ecroot.create_tex_header(self.canvas,
                                                    header_outside=True)
        for obs in self._header_obs:
            obs.Draw()

    @property
    def total_frac_dict(self):
        if self._total_frac_dict is None:
            self._total_frac_dict = self._get_total_frac_dict(self.class_type_records,
                                                              self.quantile,
                                                              self.distribution)
        return self._total_frac_dict

    @classmethod
    def _get_total_frac_dict(cls, class_type_records, quantile, distribution):
        total_frac_dict = {}
        dkey = cls.get_dominant_process_key(quantile, distribution)
        for key, infodict in class_type_records.items():
            max_frac = 0.
            max_proc = None
            for proc, frac in infodict[ dkey ].items():
                if not proc in total_frac_dict:
                    total_frac_dict[proc] = 0
                total_frac_dict[proc] += frac
                if frac > max_frac:
                    max_frac = frac
                    max_proc = proc
            if search_dominant in max_proc:
                print search_dominant, infodict['name']
        return total_frac_dict

    @classmethod
    def _fill_bins(cls, class_type_records, dkey, proc ,hist, suffix):
        hist.GetXaxis().SetLabelSize(0.12)
        for ibin, ( key, infodict ) in enumerate( class_type_records.items() ):
            hist.GetXaxis().SetBinLabel( ibin + 1 , key.replace("-only", "") )
            if not proc in infodict[ dkey ]:
                continue
            hist.SetBinContent(ibin+1, infodict["nclasses{}".format(suffix)] * infodict[ dkey ][proc] )
            hist.SetBinError(ibin+1, 0. )

    def _add_data_class_type_records(self, class_type_records):
        dkey = self.dominant_process_key
        for proc,frac in sorted(self.total_frac_dict.items(), key=operator.itemgetter(1), reverse = True):
            if proc != "data":
                continue
            hist = ro.TH1F(dkey+proc, dkey+proc, len(class_type_records), 0, len(class_type_records)+1)
            hist.SetLineColor( 1 )
            hist.SetLineWidth( 2 )
            self.legend.AddEntry( hist, "data", "PE" )
            self.has_data = True
            self._fill_bins(class_type_records, dkey, proc, hist, self.suffix)
            self.data_hist = hist

    def _add_mc_class_type_records(self, class_type_records):

        dkey = self.dominant_process_key

        # first compile list of process which are dominant in any class to
        # avoid adding empty processes to the legend
        dominant_processes = [ k for k,v in self.total_frac_dict.items() if v > 0 ]

        for proc,frac in sorted(self.total_frac_dict.items(), key=operator.itemgetter(1), reverse = True):
            if not proc in dominant_processes or proc == "data":
                continue
            self._mc_hists.append( ro.TH1F(dkey+proc,
                                   dkey+proc,
                                   len(class_type_records),
                                   0,
                                   len(class_type_records)+1))
            self._mc_hists[-1].UseCurrentStyle()
            self._mc_hists[-1].GetXaxis().SetLabelSize(0.08)
            #~ hist.GetYaxis().SetTitle( "number of classes" )
            self._mc_hists[-1].SetLineColor( PROCESS_GROUP_STYLES[proc].color )
            self._mc_hists[-1].SetFillColor( PROCESS_GROUP_STYLES[proc].color )
            self.legend.AddEntry( self._mc_hists[-1], root_latex_convert( PROCESS_GROUP_STYLES[proc].label ), "F" )

            self._fill_bins(class_type_records, dkey, proc, self._mc_hists[-1], self.suffix)
            self._mc_stack.Add(self._mc_hists[-1])

