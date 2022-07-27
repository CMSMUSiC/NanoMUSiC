import os
import logging
import itertools

import luigi

#~ from .scan import ScanTask, _ScanMixin, _ScanIOMixin, _ScanDBMixin
from .plot import BaseResultMixin, ECDistributionPlotTask, SimpsonsPlotTask, PtildePlotTask, \
    ECPlotTask, ECUncertaintyPlotTask, UncertaintyMapTask, DominantProcessPlotTask, PdistPlotTask, \
    ClassYieldsPlotTask, PtildeYieldMapPlotTask
from .tex import RoiScanMostSignificantTableTask, ECCountTexTask, SampleSummaryTableTask, \
                 ObjectGroupDefinitionTexTask, ObjectGroupSimpsonsPanelTex, RoiScanMostSignificantPlotPanelTask


from .util import get_shared_parameters, RemoteResultMixin
from .result_base import BaseResultMixin, RoIScanMixin

from ectools.register import ecstyle

logger = logging.getLogger('luigi-interface')


class TestTask(luigi.Task):
    outname = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(path=self.outname)

    def run(self):
        with open("testfile.txt", "w") as f:
            f.write("test")
        import time
        time.sleep(1)


class PlotResultMixin(BaseResultMixin):

    class_types = luigi.ListParameter(default=[])
    distributions = luigi.ListParameter(default=[])
    formats = luigi.ListParameter(default = ["pdf", "png", "root"])
    min_yield = luigi.FloatParameter(default=.1)
    requested_plots = luigi.ListParameter(default=[])
    requested_tex = luigi.ListParameter(default=[])
    filter_systematics = luigi.ListParameter(default=[])

    def add_simpsons_plots(self,tasks, out, class_type, distribution):
        kwargs = get_shared_parameters(self, SimpsonsPlotTask)
        kwargs['out'] = out
        kwargs['distribution'] = distribution
        kwargs["formats"] = self.formats
        kwargs['min_yield'] = self.min_yield
        kwargs['class_type'] = class_type
        kwargs["filter_systematics"] = self.filter_systematics
        kwargs['scan_db_path'] = SimpsonsPlotTask.get_remote_scan_db_path(
                                                              self.remote_path,
                                                              class_type,
                                                              distribution,
                                                              is_integral=True)
        # Create SimpsonsPlots
        for sort_option in ("p-value", "data_integral"):
            kwargs_data_integral = kwargs.copy()
            kwargs_data_integral["sort_option"] = sort_option
            task_name = "Simpsons-{}-{}".format(class_type, sort_option)
            tasks[task_name] = SimpsonsPlotTask(**kwargs_data_integral)

        # Add simpsons plot for object groups
        for obj_group_info in ecstyle.object_groups():
            kwargs_group = kwargs.copy()
            kwargs_group['object_group_tag'] = obj_group_info.name_tag
            kwargs_group["sort_option"] = "objects"
            # number of shown classes wil be reduced by available classes in
            # object group in plotter
            kwargs_group["n_ranking"] = 100
            task_name = "Simpsons-{}-{}-{}".format(class_type,
                                                     sort_option,
                                                     obj_group_info.name_tag)
            tasks[task_name] = SimpsonsPlotTask(**kwargs_group)

    def add_uncertainty_maps(self,
                            tasks,
                            map_type,
                            out,
                            class_type,
                            distribution,
                            per_jet_syst="Jet_systScale",
                            map_norm="total",
                            deviation_type=""):

        kwargs = get_shared_parameters(self, UncertaintyMapTask)
        kwargs['out'] = out
        kwargs["formats"] = self.formats
        kwargs['min_yield'] = self.min_yield
        kwargs['map_type'] = map_type
        kwargs['class_type'] = class_type
        kwargs['distribution'] = distribution
        kwargs['per_jet_syst'] = per_jet_syst
        kwargs['map_norm'] = map_norm
        kwargs["filter_systematics"] = self.filter_systematics
        task_name_base = "UncertMap_{}_{}_{}_{}".format(map_type,
                                                     per_jet_syst,
                                                     class_type,
                                                     distribution)
        if deviation_type:
            task_name_base += "_%s" % deviation_type
            kwargs['deviation_type'] = deviation_type
        tasks[task_name_base] = UncertaintyMapTask(**kwargs)
        kwargs_cum = kwargs.copy()
        kwargs_cum['cumulative'] = True
        task_name = task_name_base + "_cumulative"
        tasks[task_name] = UncertaintyMapTask(**kwargs_cum)
        for obj_group_info in ecstyle.object_groups():
            kwargs_group = kwargs.copy()
            kwargs_group['object_group_tag'] = obj_group_info.name_tag
            task_name = "{}_{}".format(task_name_base, obj_group_info.name_tag)
            tasks[task_name] = UncertaintyMapTask(**kwargs_group)
            kwargs_group_cumulative = kwargs_group.copy()
            kwargs_group_cumulative['cumulative'] = True
            task_name = task_name + "_cumulative"
            tasks[task_name] = UncertaintyMapTask(**kwargs_group_cumulative)

    def add_dominant_process_plots(self, tasks, out):
        kwargs = get_shared_parameters(self, DominantProcessPlotTask)
        kwargs['out'] = out
        kwargs["formats"] = self.formats
        kwargs['min_yield'] = 0.1
        task_name_base = "DominantProcesses"
        tasks[task_name_base] = DominantProcessPlotTask(**kwargs)

        kwargs_met = kwargs.copy()
        kwargs_met['distribution'] = "MET"
        task_name = task_name_base + "_MET"
        tasks[task_name] = DominantProcessPlotTask(**kwargs_met)

    def add_class_counts_tex(self, tasks, out):
        kwargs = get_shared_parameters(self, ECCountTexTask)
        kwargs['out'] = out
        kwargs['min_yield'] = self.min_yield
        task_name = "ClassCounts"
        tasks[task_name] = ECCountTexTask(**kwargs)
        # add number of classes for met
        task_name = "ClassCountsMET"
        kwargs_met = kwargs.copy()
        kwargs_met["distribution"] = "MET"
        tasks[task_name] = ECCountTexTask(**kwargs_met)

    def add_object_group_lists(self, tasks, out):
        kwargs = get_shared_parameters(self, ObjectGroupDefinitionTexTask)
        kwargs["out"] = out
        task_name = "ObjectGroupDefinitions"
        tasks[task_name] = ObjectGroupDefinitionTexTask(**kwargs)

    def add_pdist_plots(self, tasks, out, class_type, distribution):
        kwargs = get_shared_parameters(self, PdistPlotTask)
        kwargs['out'] = out
        kwargs["formats"] = self.formats
        kwargs['class_type'] = class_type
        kwargs['min_yield'] = self.min_yield
        kwargs['distribution'] = distribution
        task_name = "PdistPlot_{}".format(class_type)
        tasks[task_name] = PdistPlotTask(**kwargs)
        kwargs_met = kwargs.copy()
        kwargs['distribution'] = "MET"
        task_name = "{}_{}".format(task_name, "MET")
        tasks[task_name] = PdistPlotTask(**kwargs)

    def add_ptilde_plot(self,
                        tasks,
                        out,
                        class_type,
                        distribution,
                        deviation_type="",
                        forced_data_scan_hash=None,
                        forced_mc_scan_hash=None,
                        forced_signal_scan_hash=None,
                        integral_scan=False ):
        kwargs_scan = get_shared_parameters(self, PtildePlotTask)
        kwargs_scan["min_yield"] = self.min_yield
        kwargs_scan["out"] = out
        kwargs_scan["remote"] = True
        kwargs_scan["class_type"] = class_type
        kwargs_scan["formats"] = self.formats
        kwargs_scan["distribution"] = distribution
        kwargs_scan["deviation_type"] = deviation_type
        kwargs_scan["forced_data_scan_hash"] = forced_data_scan_hash
        kwargs_scan["forced_mc_scan_hash"] = forced_mc_scan_hash
        kwargs_scan["filter_systematics"] = self.filter_systematics
        kwargs_scan["forced_signal_scan_hash"] = forced_signal_scan_hash
        kwargs_scan["timing_dict_path"] = os.path.join(self.remote_dir,
                                                       "timing",
                                                       "timing_{class_type}_{distribution}.json".format(**kwargs_scan))
        kwargs_scan["integralScan"] = integral_scan
        task_name = "Scan-{class_type}-{distribution}-{deviation_type}-{integralScan}".format(class_type=class_type,
                                                                               distribution=distribution,
                                                                               deviation_type=deviation_type,
                                                                               integralScan=integral_scan)
        tasks[task_name] = PtildePlotTask(**kwargs_scan)

    def add_most_significant_table(self, tasks, out, class_type, distribution):

        kwargs_scan_table = get_shared_parameters(self, RoiScanMostSignificantTableTask)
        kwargs_scan_table["out"] = out
        kwargs_scan_table["class_type"] = class_type
        kwargs_scan_table["filter_systematics"] = self.filter_systematics
        kwargs_scan_table["distribution"] = distribution
        kwargs_scan_table["min_yield"] = self.min_yield
        task_name = "Scan-table-{class_type}-{distribution}".format(class_type=class_type,
                                                                    distribution=distribution)
        tasks[task_name] = RoiScanMostSignificantTableTask(**kwargs_scan_table)

    def add_most_significant_panel(self, tasks, out, class_type, distribution):

        kwargs_scan_panel = get_shared_parameters(self, RoiScanMostSignificantPlotPanelTask)
        kwargs_scan_panel["out"] = out
        kwargs_scan_panel["class_type"] = class_type
        kwargs_scan_panel["filter_systematics"] = self.filter_systematics
        kwargs_scan_panel["distribution"] = distribution
        kwargs_scan_panel["n_significant"] = 5
        task_name = "Scan-panel-{class_type}-{distribution}".format(class_type=class_type,
                                                                    distribution=distribution)
        tasks[task_name] = RoiScanMostSignificantPlotPanelTask(**kwargs_scan_panel)

    def add_class_yield_plots(self, tasks, out, class_type, distribution):
        kwargs_yield_plot = get_shared_parameters(self, ClassYieldsPlotTask)
        kwargs_yield_plot['class_type'] = class_type
        kwargs_yield_plot['min_yield'] = 0.
        task_name = "Class_yield_{}".format(class_type)
        tasks[task_name] = ClassYieldsPlotTask(**kwargs_yield_plot)

    def add_ptilde_class_yield_maps(self, tasks, out, class_type, distribution):
        kwargs_yield_plot = get_shared_parameters(self, ClassYieldsPlotTask)
        kwargs_yield_plot['class_type'] = class_type
        kwargs_yield_plot['min_yield'] = 0.01
        task_name = "Ptilde_yield_map_{}".format(class_type)
        tasks[task_name] = PtildeYieldMapPlotTask(**kwargs_yield_plot)


class ClassificationDistributionPlotsTask(luigi.WrapperTask, RemoteResultMixin, PlotResultMixin):


    def requires(self):
        # Plots for single ECs
        tasks = {}
        kwargs_ec_plot = get_shared_parameters(self, ECDistributionPlotTask)
        kwargs_ec_plot['out'] = self.remote_path
        kwargs_ec_plot['min_yield'] = 0.1

        if "distribution" in self.requested_plots:
            tasks["ECPlotTask"] = ECDistributionPlotTask(**kwargs_ec_plot)

        # Plot uncertainties for all ECs
        if "uncert" in self.requested_plots:
            kwargs_ec_uncert_plot = kwargs_ec_plot.copy()
            kwargs_ec_uncert_plot['is_uncertainty_plot'] = True
            tasks["ECUncertaintyPlotTask"] = ECDistributionPlotTask(**kwargs_ec_uncert_plot)

        return tasks

class ClassificationAllClasstypePlotsTask(luigi.WrapperTask, RemoteResultMixin, PlotResultMixin):

    def requires(self):
        # Plots for single ECs
        tasks = {}
        if "dominant-process" in self.requested_plots:
            self.add_dominant_process_plots(tasks, self.remote_path)
        if "class-count" in self.requested_tex:
            print("Adding class count")
            self.add_class_counts_tex(tasks, self.remote_path)
        if "object-group" in self.requested_tex:
            self.add_object_group_lists(tasks, self.remote_path)
        return tasks

class ClassificationPerClasstypePlotsTask(luigi.WrapperTask, RemoteResultMixin, PlotResultMixin):
    class_type = luigi.Parameter()
    distribution = luigi.Parameter()

    def requires(self):

        tasks = {}
        if "simpsons" in self.requested_plots:
            logger.info("In ClassificationFinalizationTask " + " ".join(self.filter_systematics))
            self.add_simpsons_plots(tasks,
                                    self.remote_path,
                                    self.class_type,
                                    self.distribution)
                    # Add table containing a list of all object groups
            kwargs_panel = get_shared_parameters(self, ObjectGroupSimpsonsPanelTex)
            kwargs_panel['out'] = self.remote_path
            task_name = "AN-SimpsonsPanel"
            tasks[task_name] = ObjectGroupSimpsonsPanelTex(**kwargs_panel)
            self.add_object_group_lists(tasks, self.remote_path)

        if "uncertainty-map" in self.requested_plots:
            for deviation_type in ("", "excess", "deficit"):
                self.add_uncertainty_maps(tasks,
                                          "all",
                                          self.remote_path,
                                          self.class_type,
                                          self.distribution,
                                          deviation_type=deviation_type,
                                          )
                self.add_uncertainty_maps(tasks,
                                          "perjet",
                                          self.remote_path,
                                          self.class_type,
                                          self.distribution,
                                          map_norm="syst",
                                          deviation_type=deviation_type,
                                          )

        if "pdist" in self.requested_plots:
            self.add_pdist_plots(tasks, self.remote_path, self.class_type, self.distribution)
        if "class-yield" in self.requested_plots:
            self.add_class_yield_plots(tasks, self.remote_path, self.class_type, self.distribution)
        return tasks

class FullScanTask(luigi.WrapperTask, RemoteResultMixin, PlotResultMixin, RoIScanMixin):

    def requires(self):
        tasks = {}
        for deviation_type in ("", "excess", "deficit"):
                self.add_ptilde_plot(tasks,
                                     self.remote_path,
                                     self.class_type,
                                     self.distribution,
                                     deviation_type=deviation_type,
                                     forced_data_scan_hash=self.forced_data_scan_hash,
                                     forced_mc_scan_hash=self.forced_mc_scan_hash,
                                     forced_signal_scan_hash=self.forced_signal_scan_hash,
                                     integral_scan=self.integral_scan,
                                     )
        self.add_most_significant_table(tasks,
                                        self.remote_path,
                                        self.class_type,
                                        self.distribution)
        self.add_most_significant_panel(tasks,
                                        self.remote_path,
                                        self.class_type,
                                        self.distribution)

        return tasks

class ScanFinaliaztionTask(luigi.Task, RemoteResultMixin, PlotResultMixin, RoIScanMixin):

    def requires(self):
        tasks = {}
        self.add_ptilde_class_yield_maps(tasks,
                                         self.remote_path,
                                         self.class_type,
                                         self.distribution)
        kwargs_scan_task = get_shared_parameters(self, FullScanTask)
        tasks["FullScanTask"] = FullScanTask(**kwargs_scan_task)
        return tasks

    def run(self):
        kwargs_ec_plot = self.get_kwargs()
        yield ECDistributionPlotTask(**kwargs_ec_plot)

    def get_kwargs(self):
        kwargs_ec_plot = get_shared_parameters(self, ECDistributionPlotTask)
        kwargs_ec_plot['out'] = self.remote_path
        kwargs_ec_plot['min_yield'] = 0.1
        kwargs_ec_plot['forced_data_scan_hash'] = self.forced_data_scan_hash
        kwargs_ec_plot['forced_mc_scan_hash'] = self.forced_mc_scan_hash
        kwargs_ec_plot['filter_systematics'] = self.filter_systematics
        kwargs_ec_plot['forced_signal_scan_hash'] = self.forced_signal_scan_hash
        kwargs_ec_plot['scan_db_path'] = self.remote_scan_db_path
        kwargs_ec_plot['formats'] = self.formats
        kwargs_ec_plot['scan_required'] = True
        return kwargs_ec_plot

    def output(self):
        kwargs_ec_plot = self.get_kwargs()
        return  ECDistributionPlotTask(**kwargs_ec_plot).output()

class AnalysisNoteTask(luigi.WrapperTask, RemoteResultMixin, PlotResultMixin):

    def requires(self):
        tasks = {}
        out = os.path.join(self.remote_path, "generated_content")
        # only pdfs needed for Analysis note
        self.formats = ["pdf"]
        # Add a list of specific tasks
        kwargs_ec_plot = get_shared_parameters(self, ECPlotTask)
        kwargs_ec_plot['out'] = out
        kwargs_ec_plot['formats'] = self.formats
        kwargs_ec_plot["ec_class_names"] = ["Rec_2Ele+X", "Rec_2Muon+X", "Rec_2Ele_1MET+X"]
        task_name = "AN-ec-plots"
        tasks[task_name] = ECPlotTask(**kwargs_ec_plot)
        kwargs_ec_uncert_plot = kwargs_ec_plot.copy()
        task_name = "AN-ec-plots-uncert"
        tasks[task_name] = ECUncertaintyPlotTask(**kwargs_ec_uncert_plot)
        for class_type in self.class_types:
            for distribution in self.distributions:
                self.add_simpsons_plots(tasks, out, class_type, distribution)
                self.add_uncertainty_maps(tasks,
                                          "all",
                                           out,
                                           class_type,
                                           distribution)
                self.add_uncertainty_maps(tasks,
                                          "perjet",
                                          out,
                                          class_type,
                                          distribution,
                                          map_norm="syst")
                self.add_pdist_plots(tasks, out, class_type, distribution)
                self.add_ptilde_plot(tasks, out, class_type, distribution)
                self.add_most_significant_table(tasks, out, class_type, distribution)
                self.add_most_significant_panel(tasks, out, class_type, distribution)
        # Add a tex panel including all object group simpsons plots
        kwargs_panel = get_shared_parameters(self, ObjectGroupSimpsonsPanelTex)
        kwargs_panel['out'] = out
        task_name = "AN-SimpsonsPanel"
        tasks[task_name] = ObjectGroupSimpsonsPanelTex(**kwargs_panel)
        self.add_dominant_process_plots(tasks, out)

        # Add class counts (dominant processes etc.)
        self.add_class_counts_tex(tasks, out)
        # Add table containing a list of all object groups
        self.add_object_group_lists(tasks, out)

        # Add a sample summary tex table
        kwargs_sample_summary = get_shared_parameters(self, SampleSummaryTableTask)
        kwargs_sample_summary["out"] = out
        kwargs_sample_summary["table_type"] = "summary"
        task_name = "AN-Sample-Summary-Table"
        tasks[task_name] = SampleSummaryTableTask(**kwargs_sample_summary)
        # Add a summary table for all sample references
        kwargs_sample_summary = get_shared_parameters(self, SampleSummaryTableTask)
        kwargs_sample_summary["out"] = out
        kwargs_sample_summary["table_type"] = "reference"
        task_name = "AN-Sample-Summary-Table"
        tasks[task_name] = SampleSummaryTableTask(**kwargs_sample_summary)
        return tasks

