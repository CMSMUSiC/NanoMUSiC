import os
import logging
import argparse
import time
import random

import luigi
from luigi.contrib.external_program import ExternalProgramTask
from ectools.register import ecroot

from ecresults.tex import ECCountTex, ECObjectGroupList, \
                          ECObjectGroupSimpsonsPanel, RoiScanMostSignificantTable, \
                          RoiScanMostSignificantPlotPanel, SampleSummaryTable

from ecresults.collector import ECCollector

from shellhelpers import expand_path

from .result_base import ECCollectorTask, ECCollectorMixin, RoIScanMixin, BaseResultMixin, ScanDBMixin
from .util import get_shared_parameters, RemoteResultMixin,ECBaseMixin, ECIOMixin
from .plot import SimpsonsPlotTask, ECPlotTask


logger = logging.getLogger('luigi-interface')

class TexMixin(object):

    @classmethod
    def tex_dir(cls, out):
        return os.path.join(out, "tex")

    @classmethod
    def ensure_out_dir(cls, out):
        path = cls.tex_dir(out)
        if not os.path.exists(path):
            # add some jitter and check again to prevent multiple jobs
            # trying to create the same folder on (slow) storage
            time.sleep(0.01 * random.randint(0, 100))
            if not os.path.exists(path):
                os.makedirs( path )
    @property
    def conf_namespace(self):
        parser = argparse.ArgumentParser()
        ecroot.add_ec_standard_options(parser)
        ECCollector.add_commandline_options(parser)
        conf = parser.parse_args(self.ec_plot_conf_args)
        return conf

class ECCountTexTask(RemoteResultMixin,
                     ECCollectorMixin,
                     TexMixin,
                     BaseResultMixin,
                     ScanDBMixin,
                     luigi.Task,):
    object_group_tag = luigi.Parameter("")
    integral_scan = luigi.BoolParameter(default=True)

    @property
    def conf_namespace(self):
        conf = super(ECCountTexTask, self).conf_namespace
        conf.scan_db_path = self.remote_scan_db_path
        conf.scan_id = self.campaign_id
        conf.out = self.out
        return conf

    def requires(self):
        kwargs_collector = get_shared_parameters(self, ECCollectorTask)
        kwargs_collector['distribution'] = self.distribution
        return ECCollectorTask(**kwargs_collector)

    def run(self):
        self.ensure_out_dir(self.out)
        config=self.ec_plot_conf_namespace
        cache_file_name = ECCollector.cache_file_name(self.ec_plot_conf_namespace,
                                                      self.distribution)
        ec_records = ECCollector.load_cache(cache_file_name)
        table = ECCountTex(config,
                           ec_records,
                           self.distribution,
                           self.object_group_tag)
        table.write_table()

    def output(self):
        return luigi.LocalTarget(path=ECCountTex.get_outfile_name(self.conf_namespace.out,
                                                                  self.distribution,
                                                                  self.object_group_tag,))

class ObjectGroupDefinitionTexTask(luigi.Task, RemoteResultMixin, TexMixin):
    out = luigi.Parameter("")


    @property
    def table_config(self):
        if self._config is None:
            self._config = TexTableConfig()
            self._config._add_raw_flag("description")
        return self._config

    def run(self):
        self.ensure_out_dir(self.out)
        table = ECObjectGroupList(self.out.replace(".tex", ""))
        table.write_table()

    def output(self):
        return luigi.LocalTarget(path=ECObjectGroupList.get_outfile_name(self.out))

class ObjectGroupSimpsonsPanelTex(luigi.Task, RemoteResultMixin, BaseResultMixin):
    out = luigi.Parameter("")
    class_type = luigi.Parameter("")

    def requires(self):
        tasks = {}
        kwargs = get_shared_parameters(self, SimpsonsPlotTask)
        kwargs['out'] = self.out
        kwargs['min_yield'] = self.min_yield
        # we only need the pdf output
        kwargs['formats'] = ["pdf"]
        kwargs["filter_systematics"] = self.filter_systematics
        kwargs['scan_db_path'] = SimpsonsPlotTask.get_remote_scan_db_path(
                                                      self.out,
                                                      self.class_type,
                                                      "SumPt",
                                                      is_integral=True)
        # Require SimpsonsPlot for each object group
        for obj_group_info in ecroot.object_groups():
            kwargs_group = kwargs.copy()
            kwargs_group['object_group_tag'] = obj_group_info.name_tag
            kwargs_group["sort_option"] = "objects"
            # number of shown classes wil be reduced by available classes in
            # object group in plotter
            kwargs_group["n_ranking"] = 100
            task_name = "Simpsons-{}".format(obj_group_info.name_tag)
            tasks[task_name] = SimpsonsPlotTask(**kwargs_group)
        return tasks

    def run(self):
        panel = ECObjectGroupSimpsonsPanel(self.out, self.class_type)
        for task_name, plots in self.input().items():
            group_tag = task_name.split("-")[1]
            plot_path = None
            for p in plots:
                if p.path.endswith(".pdf"):
                    plot_path = p.path
                    path = os.path.relpath(plot_path, self.remote_path)
                    panel.add_simpsons_plot(group_tag, path)
        panel.write_tex()

    def output(self):
        return luigi.LocalTarget(path=ECObjectGroupSimpsonsPanel.get_outfile_name(self.out, self.class_type))

#~ class RoiScanMostSignificantTableTask(ExternalProgramTask,
                             #~ RemoteResultMixin,
                             #~ RoIScanMixin,
                             #~ TexMixin):
    #~ executable = luigi.Parameter( default="sdb2latex.py" )
    #~ n_significant = luigi.IntParameter( default=25 )

    #~ @property
    #~ def table_path(self):
        #~ return os.path.join(self.tex_dir(self.out), "most_significant_{}".format(self.class_type))

    #~ def program_args( self ):
        #~ args = [ self.executable ]
        #~ args += self.input_arguments()
        #~ return [ os.path.expandvars( arg ) for arg in args ]

    #~ def input_arguments( self ):
        #~ return ["--database", self.remote_scan_db_path,
                #~ "-n", str(self.n_significant),
                #~ "--name", self.campaign_id,
                #~ "-o", self.table_path ]

    #~ def output(self):
        #~ return luigi.LocalTarget(path=self.table_path + "_{}.tex".format(self.distribution))


class SampleSummaryTableInputTask(ExternalProgramTask):
    out = luigi.Parameter("")
    myskims_path = luigi.Parameter( default = expand_path("$MUSIC_CONFIG/MC/myskims_ram.yaml") )

    dump_name = luigi.Parameter("ram_dump.csv")

    def program_args( self ):
        args = ['aachen3adbManager.py']
        args.append('--full-output')
        args.append('--latest-only')
        args.append('--ram-config')
        args.append(self.myskims_path)
        args.append('-o')
        args.append(self.output().path)

        return args

    def output(self):
        return luigi.LocalTarget(path=os.path.join(self.out, self.dump_name))

class SampleSummaryTableTask(luigi.Task, RemoteResultMixin, TexMixin):
    out = luigi.Parameter("")
    myskims_path = luigi.Parameter( default = expand_path("$MUSIC_CONFIG/MC/myskims_ram.yaml") )
    table_type = luigi.Parameter( default = "summary" ) # refernce is the alternative

    def requires(self):
        kwargs = get_shared_parameters(self, SampleSummaryTableInputTask)
        return SampleSummaryTableInputTask(**kwargs)

    def run(self):
        self.ensure_out_dir(self.out)
        table = SampleSummaryTable(self.out, self.input().path, table_type=self.table_type)
        table.write_table()

    def output(self):
        return luigi.LocalTarget(path=SampleSummaryTable.get_outfile_name(self.out, table_type=self.table_type))

class RoiScanMostSignificantTableTask(RemoteResultMixin,
                                      RoIScanMixin,
                                      TexMixin,
                                      ScanDBMixin,
                                      luigi.Task):
    n_significant = luigi.IntParameter( default=25 )
    integral_scan = luigi.BoolParameter(default=False)

    @property
    def conf_namespace(self):
        conf = super(RoiScanMostSignificantTableTask, self).conf_namespace
        conf.scan_db_path = self.remote_scan_db_path
        conf.scan_id = self.campaign_id
        conf.out = self.out
        conf.mc_hash = self.forced_mc_scan_hash
        conf.data_hash = self.forced_data_scan_hash
        conf.signal_hash = self.forced_signal_scan_hash
        return conf

    def run(self):
        self.ensure_out_dir(self.out)
        data_hash = self.conf_namespace.data_hash
        if not data_hash:
            data_hash = self.get_scan_hash(self.campaign_id,
                                           self.data,
                                           self.distribution,
                                           filter_systematics=self.filter_systematics)

        table = RoiScanMostSignificantTable(self.out,
                                            self.class_type,
                                            self.distribution,
                                            self.remote_scan_db_path,
                                            self.n_significant,
                                            scan_hash=data_hash)
        table.write_table()

    def output(self):
        return luigi.LocalTarget(path=RoiScanMostSignificantTable.get_outfile_name(self.out,
                                                                                   self.class_type,
                                                                                   self.distribution))


class RoiScanMostSignificantPlotPanelTask(
                                 RoIScanMixin,
                                 RemoteResultMixin,
                                 TexMixin,
                                 luigi.Task):

    n_significant = luigi.IntParameter( default=25 )
    plot_width = luigi.FloatParameter( default=0.7 )
    integral_scan = luigi.BoolParameter(default=False)

    @property
    def conf_namespace(self):
        conf = super(RoiScanMostSignificantPlotPanelTask, self).conf_namespace
        conf.scan_db_path = self.remote_scan_db_path
        conf.scan_id = self.campaign_id
        conf.out = self.out
        conf.mc_hash = None
        conf.data_hash = None
        conf.signal_hash = None
        return conf



    def run(self):

        data_hash = self.conf_namespace.data_hash
        if not data_hash:
            data_hash = self.get_scan_hash(self.campaign_id,
                                           self.data,
                                           self.distribution,
                                           filter_systematics=self.filter_systematics)

        # get plots from most significant table
        rows = RoiScanMostSignificantTable.get_rows(self.remote_scan_db_path,
                                                    self.n_significant,
                                                    self.class_type,
                                                    self.distribution,
                                                    scan_name=self.campaign_id,
                                                    scan_hash=data_hash)
        kwargs_ec_plot = get_shared_parameters(self, ECPlotTask)
        kwargs_ec_plot["ec_class_names"] = []
        kwargs_ec_plot["formats"] = ["pdf"]
        kwargs_ec_plot["class_type"] = self.class_type
        kwargs_ec_plot["scan_db_path"] = self.remote_scan_db_path
        panel = RoiScanMostSignificantPlotPanel(self.out,
                                                self.class_type,
                                                self.distribution,
                                                plot_width=self.plot_width)
        plots = []
        # first check if all plots exist for this panel
        for i,row in enumerate(rows):
            kwargs = kwargs_ec_plot.copy()
            kwargs["ec_class_names"] = [row['event_class']]
            kwargs["scan_required"] = True
            plot = ECPlotTask(**kwargs)
            # make sure the plots for the panel exist
            plot_path = [f.path for f in plot.output() if "{}.pdf".format(self.distribution) in f.path][0]
            plot_path = os.path.relpath(plot_path, self.remote_path)
            panel.add_class_plot(i+1, row['event_class'], self.distribution, plot_path)
            plots.append(plot)

        missing_plots = [p for p in plots if not p.complete()]

        if missing_plots:
            yield missing_plots
        panel.write_tex()

    def output(self):
        return luigi.LocalTarget(path=RoiScanMostSignificantPlotPanel.get_outfile_name(self.out,
                                                                                   self.class_type,
                                                                                   self.distribution))
