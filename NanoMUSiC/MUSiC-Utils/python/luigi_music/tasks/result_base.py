import os
import argparse
import logging

import sqlalchemy
import luigi

import dbtools

from ectools.register import ecroot
from ecresults.collector import ECCollector

from luigi_music.util import chunker

from .scan import _ScanMixin, _ScanIOMixin, _ScanDBMixin, ScanWriteBack, ScanTask

from .util import RemoteResultMixin, get_shared_parameters

logger = logging.getLogger('luigi-interface')

class BaseResultMixin(_ScanMixin, _ScanIOMixin, _ScanDBMixin):

    min_yield = luigi.FloatParameter(default=0.1)
    lumi = luigi.FloatParameter(default=0.0)

    @property
    def ec_plot_conf_args(self):
        args = self.ec_default_conf_args
        args += ['--cache-dir', self.remote_path]
        return args

class ScanDBMixin(object):
    class_type = luigi.Parameter( default="exclusive" )
    forced_data_scan_hash = luigi.Parameter( default="" )
    forced_mc_scan_hash = luigi.Parameter( default="" )
    forced_signal_scan_hash = luigi.Parameter( default="" )
    allow_hash_ambigouty = luigi.BoolParameter(default=False)

    @property
    def remote_scan_db_path(self):
        is_integral = False
        if hasattr(self, "integral_scan") and getattr(self, "integral_scan", False):
            is_integral = True
        return self.get_remote_scan_db_path(self.remote_path,
                                            self.class_type,
                                            self.distribution,
                                            is_integral=is_integral)

    @classmethod
    def get_remote_scan_db_path(cls, remote_path, class_type, distribution, is_integral=False):
        suffix = ""
        if is_integral:
            suffix = "_integral"
        db_name = "scan_{class_type}_{distribution}{suffix}.sqlite".format(class_type=class_type,
                                                                           distribution=distribution,
                                                                           suffix=suffix)
        return os.path.join(remote_path, db_name)

    def get_integral_requirements(self):
        kwargs_scan_task = get_shared_parameters(self, ScanWriteBack)
        kwargs_scan_task["filter_systematics"] = self.filter_systematics
        kwargs_scan_task["scan_db_path"] = self.remote_scan_db_path
        kwargs_scan_task["scan_key"] = self.campaign_id
        kwargs_scan_task["nrounds"] = 1
        scans = []
        logger.info("In require for scan task")
        for ec_name in self.ec_class_names:
            kwargs_scan_task_job = kwargs_scan_task.copy()
            kwargs_scan_task_job["ec_name"] = ec_name
            kwargs_scan_task_job["integralScan"] = True
            scans.append(ScanWriteBack(**kwargs_scan_task_job))
        return scans

    def set_scan_hashes(self, conf):
        if not os.path.exists(self.remote_scan_db_path):
             dbtools.init_database( self.engine )

        integral_scan = False
        if hasattr(self, "integral_scan"):
            integral_scan = self.integral_scan

        with dbtools.session_scope( self.engine ) as scan_session:
            # determine hashs from database if not set explicitly as forced hash
            print(self.forced_mc_scan_hash, self.forced_data_scan_hash, self.forced_signal_scan_hash)
            if not conf.mc_hash:
                if self.forced_mc_scan_hash:
                    conf.mc_hash = self.forced_mc_scan_hash
                else:
                    try:
                        conf.mc_hash = self._match_hash(scan_session, "pseudoScan", conf)
                    except dbtools.NoHashFoundException:
                        conf.mc_hash = self.get_scan_hash(conf.scan_id,
                                                        None,
                                                        self.distribution,
                                                        integralScan = integral_scan,
                                                        filter_systematics=self.filter_systematics )

            if not self.signal and not conf.data_hash:
                if self.forced_data_scan_hash:
                    self.data_hash = self.forced_data_scan_hash
                else:
                    try:
                        conf.data_hash = self._match_hash(scan_session, "dataScan", conf)
                    except dbtools.NoHashFoundException:
                        conf.data_hash = self.get_scan_hash(conf.scan_id,
                                                            self.data,
                                                            self.distribution,
                                                            integralScan = integral_scan,
                                                            filter_systematics=self.filter_systematics )

            if self.signal and not conf.signal_hash:
                if self.forced_signal_scan_hash:
                    conf.signal_hash = self.forced_signal_scan_hash
                else:
                    try:
                        conf.signal_hash = self._match_hash(scan_session, "signalScan", conf)
                    except dbtools.NoHashFoundException:
                        conf.signal_hash = self.get_scan_hash(conf.scan_id,
                                                              None,
                                                              self.distribution,
                                                              signal_path=self.signal,
                                                              integralScan = integral_scan,
                                                              filter_systematics=self.filter_systematics )

    def get_roi_requirements(self):
        kwargs_scan_task = get_shared_parameters(self, ScanTask)
        if hasattr(self, "ec_plot_conf_namespace"):
            conf = self.ec_plot_conf_namespace
        else:
            conf = self.conf_namespace

        self.scan_id = self.campaign_id
        self.set_scan_hashes(conf)

        kwargs_scan_task["scan_db_path"] = self.remote_scan_db_path
        kwargs_scan_task["filter_systematics"] = self.filter_systematics
        kwargs_scan_task["scan_key"] = self.campaign_id
        kwargs_scan_task["forced_mc_scan_hash"] = conf.mc_hash
        kwargs_scan_task["forced_data_scan_hash"] = conf.data_hash
        kwargs_scan_task["forced_signal_scan_hash"] = conf.signal_hash
        kwargs_scan_task["compare_scan_hash"] = conf.mc_hash
        kwargs_scan_task["remote"] = True
        if self.integral_scan:
            kwargs_scan_task["integralScan"] = True
        kwargs_scan_task["timing_dict_path"] = os.path.join(self.remote_dir,
                                                       "timing",
                                                       "timing_{class_type}_{distribution}.json".format(class_type=self.class_type,
                                                                                                        distribution=self.distribution))
        for class_type_info in ecroot.class_types():
            kwargs_scan_task[class_type_info.name.replace("-","_")] = False
        kwargs_scan_task[self.class_type.replace("-","_")] = True
        #~ return []
        return [ScanTask(**kwargs_scan_task)]

    def _match_hash(self, session, scan_type, conf):
        # first try to get hash by scan_id (robust against changes to hash)
        try:
            logger.info("Try to find hash for {} {} {}".format(conf.scan_id, scan_type, self.distribution))
            found_hash = dbtools.match_hash( conf.scan_id,
                                       session,
                                       scan_type=scan_type,
                                       allow_name=True,
                                       distribution=self.distribution).hash
            logger.info("Found hash {}".format(found_hash))
            return found_hash
        except dbtools.AmbigousHashException as e:
            scan_id = self.scan_id if hasattr(self, 'scan_id') else ""
            logger.warning("Found more than one hash for scan ID {}".format(scan_id))
            if self.allow_hash_ambigouty:
                logger.info("Ambigous hashes are allowed. Try to find scan with exact hash {}")
                return dbtools.match_hash( self.scan_hash,
                                           session,
                                           scan_type=scan_type,
                                           distribution=self.distribution).hash
            else:
                raise RuntimeError("Found more than one matching hash for scan ID" \
                    "provide a fixed scan_hash or use allow_hash_ambigouty option")




class ECCollectorMixin(BaseResultMixin):
    filter_systematics = luigi.ListParameter(default=[])

    @property
    def ec_plot_conf_namespace(self):
        parser = argparse.ArgumentParser()
        ecroot.add_ec_standard_options(parser)
        ECCollector.add_commandline_options(parser)
        conf = parser.parse_args(self.ec_plot_conf_args)
        conf.min_yield = self.min_yield
        conf.scan_db_path = self.remote_scan_db_path
        conf.scan_id = self.campaign_id
        conf.out = self.out
        conf.filter_systematics = self.filter_systematics
        # will be determined from database if forced_mc_scan_hash is None or empty
        conf.mc_hash = self.forced_mc_scan_hash
        conf.data_hash = self.forced_data_scan_hash
        conf.signal_hash = self.forced_signal_scan_hash
        return conf

class ECCollectorTask(luigi.Task, RemoteResultMixin, ECCollectorMixin, ScanDBMixin):
    ecs_per_chunk = luigi.IntParameter( default=100 )
    integral_scan = luigi.BoolParameter(default=True)

    @property
    def cache_file_name(self):
        return ECCollector.cache_file_name(self.ec_plot_conf_namespace,
                                           self.distribution)

    def requires(self):
        ec_class_names = self.get_class_names(distribution=self.distribution)
        tasks = []
        if not os.path.exists(self.remote_scan_db_path):
            self.scan_db_path = self.remote_scan_db_path
            logger.info("Initializing db" + self.remote_scan_db_path)
            dbtools.init_database( self.engine )

        for class_name_chunk in chunker(self.ecs_per_chunk, ec_class_names):
            kwargs = get_shared_parameters(self, ECCollectorChunkTask)
            kwargs["filter_systematics"] = self.filter_systematics
            kwargs['ec_class_names'] = class_name_chunk
            kwargs['scan_db_path'] = self.scan_db_path
            kwargs['integral_scan'] = self.integral_scan
            tasks.append(ECCollectorChunkTask(**kwargs))
        return tasks

    def run(self):
        all_records = []
        for cache_file in self.input():
            all_records += ECCollector.load_cache(cache_file.path)
        ECCollector.dump_to_cache(self.cache_file_name, all_records)
        # delete all chunked cache records if sucessful
        for cache_file in self.input():
            cache_file.remove()

    def output(self):
        return luigi.LocalTarget(path=self.cache_file_name)

class ECCollectorChunkTask(luigi.Task, RemoteResultMixin, ECCollectorMixin, ScanDBMixin):
    ec_class_names = luigi.ListParameter(default=[])
    integral_scan = luigi.BoolParameter(default=True)

    @property
    def cache_file_name(self):
        return ECCollector.cache_file_name(self.ec_plot_conf_namespace,
                                           self.distribution,
                                           self.ec_class_names)

    def requires(self):
        if self.integral_scan:
            return self.get_integral_requirements()
        return self.get_roi_requirements()

    def run(self):
        records = []
        mc_root_file = self.mc_root_file
        data_root_file = self.data_root_file
        n_classes = len(self.ec_class_names)
        self.scan_key = self.campaign_id
        scan_hash = self.scan_hash
        for i, ec_name in enumerate(self.ec_class_names):
            mc_ec = self.get_ec(mc_root_file, ec_name)
            data_ec = self.get_ec(data_root_file, ec_name)
            record = ECCollector.get_record(self.ec_plot_conf_namespace,
                                            self.distribution,
                                            i=i,
                                            N=n_classes,
                                            mc_ec=mc_ec,
                                            data_ec=data_ec,
                                            status_line=False,
                                            scan_hash=scan_hash)
            if record:
                records.append(record)
            #self.set_status_message( "Processed %d / %d (last: %s)" % (i, n_classes, ec_name) )
            if mc_ec:
                mc_ec.Delete()
            if data_ec:
                data_ec.Delete()
        if mc_root_file:
            mc_root_file.Close()
        if data_root_file:
            data_root_file.Close()
        ECCollector.dump_to_cache(self.cache_file_name, records)

    def output(self):
        return luigi.LocalTarget(path=self.cache_file_name)

class RoIScanMixin(ScanDBMixin, BaseResultMixin):
    integral_scan = luigi.BoolParameter(default=False)

    def requires(self):
        return self.get_roi_requirements()

