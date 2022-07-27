import os
import csv
import random
import string
from array import array
import ROOT as ro

from ectools.register import ecroot

from colors import SYSTEMATIC_STYLES, DETAILED_SYSTEMATIC_STYLES

from .collector_base import ECCollectorConsumerPlot

class UncertaintyMap( ECCollectorConsumerPlot ):
    def __init__( self,
                  ec_records,
                  distribution,
                  class_type,
                  stat_type="median",
                  object_group_tag=None,
                  config=None,
                ):

        #cache
        self._uncert_maps = {}
        self._uncert_bin_maps = {}
        self._uncert_spikes = {}
        self._nclasses_dict = {}
        self._sys_names = None

        # call parent constructor
        super( UncertaintyMap, self ).__init__( ec_records,
                                                config=config,
                                                object_group_tag=object_group_tag,
                                                deviation_type=config.deviation_type)

        self.init_palette()
        # set class attributes
        self.class_type = class_type
        self.distribution = distribution
        self.stat_type = stat_type
        self.object_group_tag = object_group_tag
        self.max_jets = 9

    def init_palette(self):
        # default palette, looks cool
        stops = array('d', [0.00, 0.08, 0.15, 0.34, 0.50, 0.72, 0.87, 1.00])
        red = array('d', [250./255., 234./255., 237./255., 230./255., 212./255., 156./255., 99./255., 45./255.])
        green = array('d', [251./255., 238./255., 238./255., 168./255., 101./255.,  45./255.,  0./255.,  0./255.])
        blue = array('d', [244./255.,  85./255.,  11./255.,   8./255.,   9./255.,   3./255.,  1./255.,  1./255.])
        ro.TColor.CreateGradientColorTable(len(stops), stops, red, green, blue, 999)

    @classmethod
    def get_plot_name(cls,
                      config,
                      class_type='class_type',
                      object_group_tag="",
                      distribution="SumPt"):
        kwargs = {'class_type': class_type,
                  'map_type': config.map_type,
                  'norm' : config.uncert_map_norm,
                  'distribution' : distribution,
                  'object_group' : object_group_tag }
        if object_group_tag:
            name = "UncertMap_{object_group}_{class_type}_{map_type}_{distribution}_norm_{norm}".format(**kwargs)
        else:
            name = "UncertMap_{class_type}_{map_type}_{distribution}_norm_{norm}".format(**kwargs)
        if config.deviation_type:
            name += "_{}".format(config.deviation_type)

        if config.map_type == "perjet":
            name += "_{}".format(config.per_jet_syst)
        if config.cumulative:
            name += "_cumulative"
        return name

    @classmethod
    def get_subfolder(cls, config, object_group=None, **kwargs):
        if object_group:
            return "UncertMaps/{}".format(object_group)
        return "UncertMaps"

    def set_cache_dirty(self):
        super(UncertaintyMap, self).set_cache_dirty()
        self._sys_names = None
        for map_name in self._uncert_maps:
            if self._uncert_maps.get(map_name):
                self._uncert_maps.get(map_name).Delete()
            self._uncert_maps[map_name] = None
            self._uncert_bin_maps[map_name] = None
            self._uncert_spikes[map_name] = None

    @property
    def canvas( self ):
        if self._canvas is None:
            randid = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(4))
            self._canvas = ro.TCanvas( "uncert_map" + self.class_type + randid , "", self.config.canvas_width, self.config.canvas_height )
            self._canvas.UseCurrentStyle()
            self._canvas.SetLogy(0)
            self._canvas.SetBottomMargin( 0.25 )
            self._canvas.SetRightMargin( 0.15 )
            self._canvas.cd()
        return self._canvas

    @property
    def n_syst(self):
        return len(self.syst_names)

    @property
    def syst_names(self):
        if self._sys_names is None:
            syst_names = set()
            for c in self.ec_records:
                syst_names = syst_names.union(set([k for k in c['syst_stats'].keys() if not (k.startswith("xs") or k.startswith("qcdWeight")) ]))
            self._sys_names = list(syst_names)
        return self._sys_names

    def sort_syst_names(self, category_name):
        groups = ["Scale","Resolution",  "pdfup","pileup", "fake",]
        print category_name
        score = 0
        for i,obj in enumerate(ecroot.object_types()):
            if obj.name.replace("EB", "").replace("EE", "") in category_name:
                score += i* 100
        for i,group in enumerate(groups):
            if group in category_name:
                score += i * 10
        return score

    def plot(self):
        if self.config.map_type == "all":
            map_name = self.create_all_uncert_map()
        elif self.config.map_type == "perjet":
            map_name = self.create_jet_uncert_map(self.config.per_jet_syst)
        self.canvas.cd()
        self._uncert_maps[map_name].Draw("colz")
        self._uncert_maps[map_name].LabelsOption( 'v', 'x' )


    def _initialize_uncert_map(self,map_name, category_names):
        # Set up a TH2F
        n_cat = len(category_names)
        # add uncert name to global name register
        hist_title = "Uncert map %s of rel. uncert for bins with N_{MC} > 0.001" % self.stat_type
        self._uncert_maps[map_name] = ro.TH2F(map_name ,
                                              hist_title,
                                              n_cat,
                                              0,
                                              n_cat,
                                              20,
                                              0,
                                              1)

        self._uncert_maps[map_name].UseCurrentStyle()
        self._uncert_bin_maps[map_name] = {}
        category_names.sort(key=lambda x: self.sort_syst_names(x))
        # Set bin labels to category names and create bin to sys map
        for ibin,cat in enumerate(category_names):
            self._uncert_maps[map_name].GetXaxis().SetBinLabel( ibin +1 , self.get_syst_label(cat) )
            #~ self._uncert_maps[map_name].GetXaxis().ChangeLabel( ibin +1 , 0.5 )
            self._uncert_bin_maps[map_name][cat] = ibin + 1
        self._uncert_maps[map_name].GetXaxis().SetLabelFont( 42 )
        y_axis_title = "%s relative uncert" % self.stat_type
        z_axis_title = "fraction of classes"
        if self.config.uncert_map_norm == "total":
            z_axis_title += " w.r.t. all classes"
        if self.config.uncert_map_norm == "syst":
            z_axis_title += " w.r.t. all classes with this uncert"
        if self.config.cumulative:
            y_axis_title += "< y"
        self._uncert_maps[map_name].GetYaxis().SetTitle(y_axis_title)
        self._uncert_maps[map_name].GetZaxis().SetTitleOffset(1.2);
        self._uncert_maps[map_name].GetZaxis().SetTitle(z_axis_title)
        self._uncert_spikes[map_name] = {}

    def _fill_cat(self, map_name, category, systematic, class_info):
        if systematic not in class_info['syst_stats']:
            return False
        cat_bin = self._uncert_bin_maps[map_name][category]
        cat_bin_center = self._uncert_maps[map_name].GetXaxis().GetBinCenter(cat_bin)
        value = class_info['syst_stats'][systematic][self.stat_type]
        if value > self.config.uncert_spike_thresh:
            if not category in self._uncert_spikes[map_name]:
                self._uncert_spikes[map_name][category] = [ ]
            self._uncert_spikes[map_name][category].append((class_info['name'], value))
        if not self.config.cumulative:
            self._uncert_maps[map_name].Fill(cat_bin_center, value)
        else:
            for ibin in range(self._uncert_maps[map_name].GetNbinsY()):
                low = self._uncert_maps[map_name].GetYaxis().GetBinLowEdge(ibin)
                if value > low:
                    self._uncert_maps[map_name].Fill(cat_bin_center, low)
        return True

    def save_plots(self):
        for map_name in self._uncert_maps:
            self.canvas.cd()
            self._uncert_maps[map_name].Draw("colz")
            self._uncert_maps[map_name].LabelsOption( 'v', 'x' )
            self.save_plot(map_name, sub_path='UncertMaps')
            self.canvas.Clear()
            #~ self._canvas  = None

    def spikes2csv(self):
        for map_name in self._uncert_spikes:
            outpath = os.path.join(self.config.out,"csv", "UncertSpikes")
            if not os.path.isdir( outpath ):
                os.makedirs( outpath )
            with open(self.outfile_name(os.path.join(outpath, map_name), "csv"), "w") as csv_file:
                writer = csv.writer(csv_file)
                writer.writerow(["category", "name",  "%s uncert" % self.stat_type])
                for category, spike_list in self._uncert_spikes[map_name].items():
                    spike_list = sorted(spike_list, key=lambda x: x[1], reverse=True)
                    for ec_name, value in spike_list:
                        writer.writerow(( category, ec_name, value))


    def create_all_uncert_map(self):
        map_name = "all_uncert_map"
        if not self._uncert_maps.get(map_name):
            self._initialize_uncert_map(map_name, self.syst_names)
            # Fill values from all classes
            sys_counts = {}
            self._nclasses_dict[map_name] = 0
            for cl in self.ec_records:
                njets, nbjets = ecroot.n_jets_from_ecname(cl["name"])
                total_jets = njets + nbjets
                if total_jets >= self.max_jets:
                    continue
                self._nclasses_dict[map_name] += 1
                for sys in cl['syst_stats']:
                    if sys not in self.syst_names:
                        continue
                    success = self._fill_cat(map_name, sys, sys, cl)
                    if success:
                        sys_counts[sys] = sys_counts.get(sys, 0) + 1
            if self.config.uncert_map_norm == "syst":
                for sys in self.syst_names:
                    if sys not in self._uncert_bin_maps[map_name]:
                        continue
                    cat_bin = self._uncert_bin_maps[map_name][sys]
                    for uncert_bin in range(self._uncert_maps[map_name].GetNbinsY()):
                        global_bin = self._uncert_maps[map_name].GetBin(cat_bin, uncert_bin)
                        value = self._uncert_maps[map_name].GetBinContent(global_bin)
                        self._uncert_maps[map_name].SetBinContent(global_bin, value / sys_counts[sys])
            elif self.config.uncert_map_norm == "total":
                self._uncert_maps[map_name].Scale( 1. / self._nclasses_dict[map_name])
        return map_name

    def get_syst_label(self, syst_name):
        for syst, info in DETAILED_SYSTEMATIC_STYLES.items():
            if syst_name.startswith(syst):
                return info.label
        return syst_name

    def create_jet_uncert_map(self, syst="Jet_systScale"):
        map_name = "jet_uncert_map_%s" % syst

        if not self._uncert_maps.get(map_name):
            jet_categories = ["> %d Jet" %d for d in range(0, self.max_jets)]
            self._initialize_uncert_map(map_name, jet_categories)
            # Fill values from all classes
            n_classes_per_jet = {}
            self._nclasses_dict[map_name] = 0
            for cl in self.ec_records:
                if syst not in cl['syst_stats']:
                    continue
                njets, nbjets = ecroot.n_jets_from_ecname(cl["name"])
                total_jets = njets + nbjets
                if total_jets >= self.max_jets:
                    continue
                self._nclasses_dict[map_name] += 1
                if self.config.jets_cumulative:
                    for i in range(total_jets+1):
                        self._fill_cat(map_name, jet_categories[i], syst, cl)
                        n_classes_per_jet[i] = n_classes_per_jet.get(i, 0) + 1
                else:
                    self._fill_cat(map_name, jet_categories[total_jets], syst, cl)
                    n_classes_per_jet[total_jets] = n_classes_per_jet.get(total_jets, 0) + 1
            self._uncert_maps[map_name].Scale( 1. / self._nclasses_dict[map_name])
        return map_name

    def save_root(self):
        outfile = ro.TFile(os.path.join(self.config.out,
                                        self.outfile_name("uncert_maps","root")),
                           'RECREATE')
        outfile.cd()
        for map_name, hist in self._uncert_maps.items():
            hist.Write()
        outfile.Close()

    @classmethod
    def add_commandline_options( cls, parser, with_parent=True ):
        if with_parent:
            super(cls, UncertaintyMap).add_commandline_options(parser)
        group = parser.add_argument_group( title="Uncertainty summary plotting options" )
        group.add_argument( "--map-type", default="all", choices = ["all"," perjet"],
                            help="Type of map: all - overview of all uncerts, perjet - deistribution of uncertainty by jet multiplicity")
        group.add_argument( "--jets-cumulative", action="store_true",
                            help="Show jets cumulative in uncertainty summaries" )
        group.add_argument( "--per-jet-syst", default="Jet_systScale",
                            help="Uncertainty to use for perjet maps" )
        group.add_argument( "--uncert-spike-thresh", default=0.5, type=float,
                            help="Minimum relative uncert befor a class is considered a spike" )
        group.add_argument( "--uncert-map-norm", choices=["total","syst"], default="total",
                            help="Minimum relative uncert befor a class is considered a spike" )
        group.add_argument( "--deviation-type", choices=["","excess", "deficit"], default="",
                    help="Show map only for one possible deviation type" )

