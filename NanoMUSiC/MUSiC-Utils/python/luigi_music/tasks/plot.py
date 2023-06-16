import logging
import argparse
import os, os.path
import sys
import traceback
import sqlalchemy

from ectools.register import ecroot
from ecresults.collector import ECCollector
from ecresults.plots import ECDistributionPlot, ECUncertaintyPlot,\
                            SimpsonsPlot, UncertaintyMap, ECDominantProcessPlot,\
                            PtildePlot, PdistPlot, ClassYieldsPlot, PtildeYieldMapPlot

import roothelpers
import dbtools

import luigi

from shellhelpers import expand_path
from luigi_music.util import chunker
from luigi_music.parameters.listparameters import StringListParameter

from .util import get_shared_parameters, RemoteResultMixin
from .merge import FullClassificationTask
from .result_base import BaseResultMixin, ECCollectorTask, ECCollectorMixin, \
                         ScanDBMixin ,RoIScanMixin

logger = logging.getLogger('luigi-interface')


class ECDistributionPlotMixin(RemoteResultMixin, BaseResultMixin):
    forced_data_scan_hash = luigi.Parameter(default='')
    forced_mc_scan_hash = luigi.Parameter(default='')
    forced_signal_scan_hash = luigi.Parameter(default='')
    filter_systematics = luigi.ListParameter(default=[])
    formats = luigi.ListParameter(default = ["pdf", "root"])
    scan_required = luigi.BoolParameter(default=False)

class ECDistributionPlotTask(luigi.WrapperTask, ECDistributionPlotMixin):
    ecs_per_chunk = luigi.IntParameter( default=50 )
    is_uncertainty_plot = luigi.BoolParameter( default=False )

    def requires(self):
        ec_class_names = self.get_class_names()
        tasks = []
        for class_name_chunk in chunker(self.ecs_per_chunk, ec_class_names):
            kwargs = get_shared_parameters(self, self.plot_task)
            kwargs['ec_class_names'] = class_name_chunk
            task = self.plot_task(**kwargs)
            if task.output():
                tasks.append(task)
        return tasks

    @property
    def plot_task(self):
        if self.is_uncertainty_plot:
            return ECUncertaintyPlotTask
        return ECPlotTask


class ECBasePlotTask( ECDistributionPlotMixin, luigi.Task):

    ec_class_names = luigi.ListParameter(default=[])

    @property
    def plot_class(self):
        raise NotImplementedError("plot_class not implemented. ECBasePlotTask should only be used a parent class!")

    @property
    def ec_plot_conf_namespace(self):
        parser = argparse.ArgumentParser()
        ecroot.add_ec_standard_options(parser)
        self.plot_class.add_commandline_options(parser)
        conf = parser.parse_args(self.ec_plot_conf_args)
        conf.out = self.out
        conf.formats = self.formats
        conf.filter_systematics = self.filter_systematics
        return conf

    def run(self):
        roothelpers.root_setup()
        if os.path.exists(self.scan_db_path):
            engine = sqlalchemy.create_engine( 'sqlite:///' + self.scan_db_path,
                                                echo=False )
            with dbtools.session_scope( engine ) as scan_session:
                self._make_plots(scan_session)
        else:
            self._make_plots()

    def _make_plots(self, session=None):
        raise NotImplementedError("_make_plots not implemented. ECBasePlotTask should only be used a parent class!")


class ECBaseDistributionPlotTask(ECBasePlotTask, ScanDBMixin):

    def _make_plots(self, session=None):
        mc_root_file = self.mc_root_file
        data_root_file = None
        if self.data:
            data_root_file = self.data_root_file
        signal_root_file = None
        if self.signal:
            logger.info("signal " + self.signal)
            signal_root_file = self.signal_root_file

        for ec_name in self.ec_class_names:
            mc_ec = self.get_ec(mc_root_file, ec_name)
            data_ec = self.get_ec(data_root_file, ec_name)
            signal_ec = self.get_ec(signal_root_file, ec_name)
            self._make_plot(ec_name, mc_ec, data_ec, signal_ec, session)
            # if mc_ec:
            #     mc_ec.Delete()
            # if data_ec:
            #     data_ec.Delete()
        mc_root_file.Close()
        if data_root_file:
            data_root_file.Close()
        if signal_root_file:
            signal_root_file.Close()

    def _make_plot(self, ec_name, mc_ec, data_ec, signal_ec, session=None):
        # the_ec points to at least one existing TEventClass, thus can be
        # used to obtain the event class name, etc.
        the_ec = data_ec if data_ec else mc_ec
        for distribution in ecroot.get_ec_distribution_types( the_ec ):
            job_name = ec_name + " (" + distribution + ")"
            config = self.ec_plot_conf_namespace
            # skip if a scan result is expected but missing
            if self.scan_required and self.plot_class.check_scan_missing(session,
                                                                         config,
                                                                         ec_name,
                                                                         distribution):

                continue
            # Create the plot
            try:
                plot = self.plot_class(
                                        config,
                                        distribution,
                                        data_ec=data_ec,
                                        mc_ec=mc_ec,
                                        signal_ec=signal_ec,
                                        session=session
                                        )
            except Exception as e:
                logger.error("Failed to produce plot for {}".format(ec_name))
                traceback.print_exc()
                continue
            if plot.skip:
                logger.error( " {:>40s} | {:<40s}".format( job_name, "skipped:" +  plot.skipreason ) )
            else:
                outname = plot.get_plot_name( config,
                                              ec_name=ec_name,
                                              distribution=distribution )
                sub_path = plot.get_subfolder(config,
                                              ec_name=ec_name,
                                              distribution=distribution)
                plot.save_plot( outname, sub_path )
            plot.cleanup()

    def _add_output(self, plots, conf, session=None):
        for ec_name in self.ec_class_names:
            #~ logger.info("Check if scan is mising")
            #~ logger.info("Scan was found")
            mc_ec = self.get_ec(self.mc_root_file, ec_name)
            data_ec = self.get_ec(self.data_root_file, ec_name)
            the_ec = data_ec if data_ec else mc_ec
            for distribution in ecroot.get_ec_distribution_types( the_ec ):
                if not the_ec:
                    raise ValueError("EC is none for event class ec_name")
                if self.plot_class.check_scan_missing(session, conf, ec_name, distribution):
                    continue
                sub_path = self.plot_class.get_subfolder(conf,
                                                         ec_name=ec_name,
                                                         distribution=distribution)
                outname = self.plot_class.get_plot_name( conf,
                                                         ec_name=ec_name,
                                                         distribution=distribution )
                for plot_path in self.plot_class.get_output_files(conf, outname, sub_path):
                    plots.append(luigi.LocalTarget(path=plot_path))
            # if mc_ec:
            #     mc_ec.Delete()
            # if data_ec:
            #     data_ec.Delete()

    def output(self):
        plots = []
        roothelpers.root_setup()
        conf = self.ec_plot_conf_namespace
        if self.scan_required and os.path.exists(self.scan_db_path):
            engine = sqlalchemy.create_engine( 'sqlite:///' + self.scan_db_path,
                                                echo=False )
            with dbtools.session_scope( engine ) as scan_session:
                logger.info("Adding plots with session")
                self._add_output(plots, conf, scan_session)
        else:
            self._add_output(plots, conf)
        if not plots:
            logger.info("Found no plots for " + " ".join(self.ec_class_names))
        return plots


class ECPlotTask(ECBaseDistributionPlotTask):
    @property
    def ec_plot_conf_namespace(self):
        conf = super(ECPlotTask, self).ec_plot_conf_namespace
        if os.path.exists(self.scan_db_path):
            conf.data_hash = None
            conf.mc_hash = None
            conf.signal_hash = None
            if self.scan_required:
                conf.scan_db_path = self.scan_db_path
                conf.scan_id = self.campaign_id
                self.set_scan_hashes(conf)
                conf.scan_id = conf.data_hash
        return conf

    @property
    def plot_class(self):
        return ECDistributionPlot


class ECUncertaintyPlotTask(ECBaseDistributionPlotTask):

    @property
    def plot_class(self):
        return ECUncertaintyPlot

class ECCollectorConsumerPlotMixin(ECBasePlotTask, ECCollectorMixin, ScanDBMixin):

    @property
    def plot_class(self):
        raise NotImplementedError("No plot class implemented for ECCollectorConsumerPlotMixin child class")

    @property
    def plot_name(self):
        raise NotImplementedError("No plot_name function implemented for ECCollectorConsumerPlotMixin child class")

    @property
    def sub_path(self):
        raise NotImplementedError("No sub_path function implemented for ECCollectorConsumerPlotMixin child class")

    def init_plot(self, ec_records, conf):
        raise NotImplementedError("No init_plot function implemented for ECCollectorConsumerPlotMixin child class")

    @property
    def ec_plot_conf_namespace(self):
        parser = argparse.ArgumentParser()
        ecroot.add_ec_standard_options(parser)
        ECCollector.add_commandline_options(parser)
        self.plot_class.add_commandline_options(parser, with_parent=False)
        conf = parser.parse_args(self.ec_plot_conf_args)
        conf.scan_db_path = self.remote_scan_db_path
        conf.distribution = self.distribution
        conf.min_yield = self.min_yield
        conf.scan_id = self.campaign_id
        conf.out = self.out
        conf.filter_systematics = self.filter_systematics
        conf.formats = self.formats
        # will be determined from database if forced_mc_scan_hash is None or empty
        conf.mc_hash = self.forced_mc_scan_hash
        conf.data_hash = self.forced_data_scan_hash
        conf.signal_hash = self.forced_signal_scan_hash
        return conf

    def requires(self):
        kwargs_collector = get_shared_parameters(self, ECCollectorTask)
        kwargs_collector['distribution'] = self.distribution
        kwargs_collector['integral_scan'] = self.integral_scan
        return ECCollectorTask(**kwargs_collector)

    @classmethod
    def get_ec_records(cls, conf, distribution, scan_hash=None):
        return ECCollector.get_records(conf,
                                       distribution,
                                       scan_hash=scan_hash)

    def run(self):
        conf = self.ec_plot_conf_namespace
        self._make_plot(conf)

    def _make_plot(self, conf):

        self.scan_id = self.campaign_id
        ec_records = self.get_ec_records(self.ec_plot_conf_namespace,
                                         self.distribution,
                                         scan_hash=self.scan_hash)
        if hasattr(self, "class_type"):
            ec_records = ec_records.filter_in("class_type", [self.class_type])
        ec_records = ec_records.filter_gt("integral", self.min_yield)
        plot = self.init_plot( ec_records, self.ec_plot_conf_namespace)
        if plot.is_empty:
            raise RuntimeError("No record found for requested plot")
        plot.plot()
        plot.save_plot( self.plot_name, self.sub_path )
        plot.cleanup()

    def output(self):
        plots = []
        files = self.plot_class.get_output_files(self.ec_plot_conf_namespace,
                                                 self.plot_name,
                                                 self.sub_path)
        for plot_path in files:
            plots.append(luigi.LocalTarget(path=plot_path))
        return plots


class SimpsonsPlotTask(ECCollectorConsumerPlotMixin):

    class_type = luigi.Parameter( default="exclusive" )
    sort_option = luigi.Parameter( default="data_integral" )
    n_ranking = luigi.IntParameter( default=25 )
    detailed_groups = luigi.Parameter( default="strong" )
    object_group_tag = luigi.Parameter( default="" )
    yield_skipped_plot = luigi.BoolParameter( default=True )
    is_skipped_plot = luigi.BoolParameter( default=False )
    integral_scan = luigi.BoolParameter(default=True)

    @property
    def plot_class(self):
        return SimpsonsPlot

    @property
    def plot_name(self):
        return self.plot_class.get_plot_name( self.ec_plot_conf_namespace,
                                              class_type=self.class_type,
                                              object_group_tag=self.object_group_tag,
                                            )

    @property
    def sub_path(self):
        return self.plot_class.get_subfolder(self.ec_plot_conf_namespace,
                                             object_group=self.object_group_tag)
    @property
    def ec_plot_conf_namespace(self):
        conf = super(SimpsonsPlotTask, self).ec_plot_conf_namespace
        conf.n_ranking = self.n_ranking
        conf.sort_option = self.sort_option
        conf.scan_db_path = self.remote_scan_db_path
        conf.scan_id = self.campaign_id
        conf.detailed_groups = self.detailed_groups
        conf.show_skipped_classes = self.is_skipped_plot
        conf.object_group_tag = self.object_group_tag
        return conf

    def run(self):
        conf = self.ec_plot_conf_namespace
        self.set_scan_hashes(conf)
        self._make_plot(conf)
        # create a plot with skipped classses if requested
        if self.yield_skipped_plot and not self.is_skipped_plot:
            # first check if there are skipped records
            # for the requested class type, distribution, object_group variant
            conf.show_skipped_classes = True
            ec_records = self.get_ec_records(conf, self.distribution)
            skipped_plot = self.init_plot(ec_records, conf)
            if not skipped_plot.is_empty:
                kwargs_skipped = get_shared_parameters(self, SimpsonsPlotTask)
                kwargs_skipped["is_skipped_plot"] = True
                yield SimpsonsPlotTask(**kwargs_skipped)

    def init_plot(self, ec_records, conf):
        return self.plot_class( ec_records,
                                self.class_type,
                                config=conf,
                                object_group_tag=self.object_group_tag)


class DominantProcessPlotTask(ECCollectorConsumerPlotMixin):
    quantile = luigi.FloatParameter(default=0.5)
    object_group_tag = luigi.Parameter( default="" )
    integral_scan = luigi.BoolParameter(default=True)

    @property
    def plot_class(self):
        return ECDominantProcessPlot

    @property
    def ec_plot_conf_namespace(self):
        conf = super(DominantProcessPlotTask, self).ec_plot_conf_namespace
        conf.scan_db_path = None
        conf.scan_id = None
        return conf

    @property
    def plot_name(self):
        return self.plot_class.get_plot_name( self.ec_plot_conf_namespace,
                                              quantile=self.quantile,
                                              distribution=self.distribution,
                                            )
    @property
    def sub_path(self):
        return self.plot_class.get_subfolder(self.ec_plot_conf_namespace,
                                             object_group=self.object_group_tag)

    def init_plot(self, ec_records, conf):
        return self.plot_class(conf,
                               ec_records,
                               self.distribution,
                               quantile=self.quantile,
                               object_group_tag=self.object_group_tag)


class UncertaintyMapTask(ECCollectorConsumerPlotMixin):
    class_type = luigi.Parameter( default="exclusive" )
    object_group_tag = luigi.Parameter( default="" )
    map_type = luigi.Parameter( default="all" )
    per_jet_syst = luigi.Parameter(default="Jet_systScale")
    map_norm = luigi.Parameter(default="total")
    cumulative = luigi.BoolParameter(default=False)
    integral_scan = luigi.BoolParameter(default=True)
    deviation_type = luigi.Parameter(default="")

    @property
    def plot_class(self):
        return UncertaintyMap

    @property
    def plot_name(self):
        return self.plot_class.get_plot_name( self.ec_plot_conf_namespace,
                                              class_type=self.class_type,
                                              object_group_tag=self.object_group_tag,
                                              distribution=self.distribution)

    @property
    def sub_path(self):
        return self.plot_class.get_subfolder(   self.ec_plot_conf_namespace,
                                             object_group=self.object_group_tag)

    @property
    def ec_plot_conf_namespace(self):
        conf = super(UncertaintyMapTask, self).ec_plot_conf_namespace
        conf.cumulative = self.cumulative
        conf.map_type = self.map_type
        conf.uncert_map_norm = self.map_norm
        conf.per_jet_syst = self.per_jet_syst
        conf.deviation_type = self.deviation_type
        conf.object_group_tag = self.object_group_tag
        return conf

    def init_plot(self, ec_records, conf):
        return self.plot_class( ec_records,
                                distribution=self.distribution,
                                class_type=self.class_type,
                                config=conf,
                                object_group_tag=self.object_group_tag)


class PdistPlotTask(ECCollectorConsumerPlotMixin):
    class_type = luigi.Parameter( default="exclusive" )
    object_group_tag = luigi.Parameter( default="" )
    integral_scan = luigi.BoolParameter(default=True)

    @property
    def plot_class(self):
        return PdistPlot

    @property
    def plot_name(self):
        return self.plot_class.get_plot_name( self.ec_plot_conf_namespace,
                                              class_type=self.class_type,
                                              distribution=self.distribution,
                                              object_group_tag=self.object_group_tag)

    @property
    def sub_path(self):
        return self.plot_class.get_subfolder(self.ec_plot_conf_namespace,
                                             object_group=self.object_group_tag)

    def run(self):
        conf = self.ec_plot_conf_namespace
        self.set_scan_hashes(conf)
        self._make_plot(conf)

    def init_plot(self, ec_records, conf):
        return self.plot_class( ec_records,
                                self.class_type,
                                config=conf,
                                object_group_tag=self.object_group_tag)


class ClassYieldsPlotTask(ECCollectorConsumerPlotMixin):
    class_type = luigi.Parameter( default="exclusive" )
    object_group_tag = luigi.Parameter( default="" )
    integral_scan = luigi.BoolParameter(default=True)

    @property
    def ec_plot_conf_namespace(self):
        conf = super(ClassYieldsPlotTask, self).ec_plot_conf_namespace
        conf.canvas_width = 800
        conf.canvas_height = 800
        return conf

    @property
    def plot_class(self):
        return ClassYieldsPlot

    @property
    def plot_name(self):
        return self.plot_class.get_plot_name( self.ec_plot_conf_namespace,
                                              distribution=self.distribution,
                                              class_type=self.class_type,
                                              object_group_tag=self.object_group_tag)

    @property
    def sub_path(self):
        return self.plot_class.get_subfolder(self.ec_plot_conf_namespace)

    def init_plot(self, ec_records, conf):
        return self.plot_class( ec_records,
                                self.class_type,
                                config=conf,
                                distribution=self.distribution,
                                object_group_tag=self.object_group_tag)

class PtildeYieldMapPlotTask(ECCollectorConsumerPlotMixin):
    class_type = luigi.Parameter( default="exclusive" )
    object_group_tag = luigi.Parameter( default="" )
    integral_scan = luigi.BoolParameter(default=False)

    @property
    def plot_class(self):
        return PtildeYieldMapPlot

    @property
    def plot_name(self):
        return self.plot_class.get_plot_name( self.ec_plot_conf_namespace,
                                              class_type=self.class_type,
                                              object_group_tag=self.object_group_tag,
                                              distribution=self.distribution,
                                            )
    @property
    def sub_path(self):
        return self.plot_class.get_subfolder(self.ec_plot_conf_namespace,
                                             object_group=self.object_group_tag)

    def requires(self):
        # overwrite implementation in parent class to require RoI scan
        kwargs_collector = get_shared_parameters(self, ECCollectorTask)
        kwargs_collector['distribution'] = self.distribution
        kwargs_collector['integral_scan'] = False
        return ECCollectorTask(**kwargs_collector)

    def run(self):
        conf = self.ec_plot_conf_namespace
        self.set_scan_hashes(conf)
        self._make_plot(conf)

    def init_plot(self, ec_records, conf):
        return self.plot_class( ec_records,
                                self.distribution,
                                self.class_type,
                                config=conf,
                                object_group_tag=self.object_group_tag)


class PtildePlotTask(RoIScanMixin, ECBasePlotTask):
    deviation_type = luigi.Parameter(default="")

    @property
    def plot_class(self):
        return PtildePlot

    @property
    def ec_plot_conf_namespace(self):
        parser = argparse.ArgumentParser()
        self.plot_class.add_commandline_options(parser)
        conf = parser.parse_args(self.ec_plot_conf_args)
        conf.out = self.out
        conf.formats = self.formats
        conf.scan_db_path = self.remote_scan_db_path
        conf.scan_id = self.campaign_id
        conf.distribution = self.distribution
        conf.nscans = self.nrounds
        # will be determined from database if forced_mc_scan_hash is None or empty
        conf.mc_hash = self.forced_mc_scan_hash
        conf.data_hash = self.forced_data_scan_hash
        conf.signal_hash = self.forced_signal_scan_hash
        conf.lumi = self.lumi
        conf.filter_systematics = self.filter_systematics
        if self.deviation_type:
            conf.filter_deviation_type = self.deviation_type
        if self.integral_scan:
            conf.integralScan = True
        return conf

    def run(self):

        conf = self.ec_plot_conf_namespace
        engine = None
        if os.path.exists(self.remote_scan_db_path):
            engine = sqlalchemy.create_engine( 'sqlite:///' + self.remote_scan_db_path,
                                                echo=False )
        else:
            raise IOError("Unable to find scan file {path}".format(path=self.remote_scan_db_path))
        self.scan_id = self.campaign_id
        with dbtools.session_scope( engine ) as scan_session:
            # determine hashs from database if not set explicitly as forced hash
            self.set_scan_hashes(conf)
            self._make_ptilde_plot(conf, scan_session)

    def _make_ptilde_plot(self, conf, scan_session):

        # First select all valid classes
        all_class_names = self.plot_class.ec_names_from_db(conf.mc_hash,
                                                           scan_session)
        selected, dropped = ecroot.filter_class_names(set(all_class_names), conf)
        # set skip reasons
        skipped, not_skipped = self.plot_class.set_skip_reason( scan_session,
                                                                data_hash=conf.data_hash,
                                                                mc_hash=conf.mc_hash,
                                                                nscans=conf.nscans,
                                                                min_yield=conf.min_yield,
                                                                ec_names=selected,
                                                                filter_deviation_type=self.deviation_type )
        plot = self.plot_class( conf,
                                not_skipped,
                                session=scan_session,
                                class_type=self.class_type )

        plot.plot()
        outname = plot.get_plot_name( conf,
                                      distribution=self.distribution,
                                      class_type=self.class_type )
        sub_path = plot.get_subfolder(conf, distribution=self.distribution)
        plot.save_plot( outname, sub_path )


    def output(self):
        plots = []
        if self.ec_plot_conf_namespace.filter_deviation_type:
            logger.info("filter_deviation_type " + self.ec_plot_conf_namespace.filter_deviation_type)
        sub_path = self.plot_class.get_subfolder(self.ec_plot_conf_namespace, distribution=self.distribution)
        outname = self.plot_class.get_plot_name( self.ec_plot_conf_namespace,
                                                 distribution=self.distribution,
                                                 class_type=self.class_type )
        for plot_path in self.plot_class.get_output_files(self.ec_plot_conf_namespace, outname, sub_path):
            plots.append(luigi.LocalTarget(path=plot_path))
        return plots
