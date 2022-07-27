from __future__ import print_function
import argparse
import subprocess
import hashlib
import json
import logging
import datetime
import time
import random
import os, os.path
import numpy as np

import luigi
from luigi.task import flatten
from luigi.configuration import get_config

import aachen3adb
from ramlib import ramutils
from shellhelpers import expand_path, tmp_chdir
from gridlib.util import memoize

from ectools.register import ecroot
from roothelpers import root_setup

from luigi_music.parameters.listparameters import StringListParameter
from luigi_music.targets import GridTarget
from luigi_music.util import cached_property

logger = logging.getLogger('luigi-interface')
root_setup()

def compute_dependencies( task, recursive=True, seen=set() ):
    """Recursively get task dependencies."""

    dependencies = list()

    required_tasks = flatten( task.requires() )
    for required in required_tasks:
        if required in dependencies:
            continue
        dependencies.append( required )

        if recursive:
            child_dependencies = compute_dependencies( required )

            if task in child_dependencies:
                raise ValueError("Cyclic dependencies.")

            dependencies.extend( child_dependencies )

    return dependencies

def get_shared_parameters(task, required_target_cls):
    passed_params = {}
    # list of (param_name, param_obj) tuple
    params = required_target_cls.get_params()
    def param_match(pn, po, pars):
        for n,o in pars:
            if pn == n and isinstance(po, type(o)):
                return True
        return False
    matched =[ pn for pn,po in task.get_params() if param_match(pn, po, params)]
    return { pn:po for pn,po in task.param_kwargs.items() if pn in matched}

def _flatten( item ):
    """Returns a serializable type from the given item."""

    # Namedtuples have the member _asdict.
    if hasattr( item, "_asdict" ):
        return _flatten( dict( item._asdict() ) )

    # Numpy types
    elif isinstance( item, np.ndarray ):
        return _flatten( item.tolist() )

    # Primitive types
    elif isinstance( item, ( int, float, str ) ):
        return item

    # Different kind of primitive containers.
    elif isinstance( item, dict ):
        return { k : _flatten(v) for k, v in item.items() }
    elif isinstance( item, list ):
        return list( _flatten(x) for x in item )
    elif isinstance( item, tuple ):
        return tuple( _flatten(x) for x in item )

    # Fallback solution.
    # When the warning is shown, somebody should implement a handler here!
    else:
        logging.getLogger().warning( "Item '%r' might not be JSON-serializable.", item )
        return item


@memoize
class MyskimsManager(object):
    def __init__( self, path = None ):
        self.clear()
        if path:
            self.load(path)

    def update( self, filename ):
        self.load( filename, merge=True )

    def reload( self ):
        self.clear()

        for filename in self.loaded_configs:
            self.load( filename, merge=True )

    def load( self, filename, merge=False ):

        settings, samplecfg = ramutils.load_config( filename )

        if not merge:
            self.clear()

        self.loaded_configs.add( filename )
        self.settings.update( settings )
        self.samplecfg.update( samplecfg )
        self._build_inverse_dicts()

    def clear( self ):
        self.settings = {}
        self.samplecfg = {}
        self.sample_groups = {}
        self.sample_configs = {}

        self.loaded_configs = set()

        self._db_cache = {}

    def _build_inverse_dicts( self ):
        self.sample_groups = {}
        self.sample_configs = {}
        for group, configs in self.samplecfg.iteritems():
            for sample, config in configs.iteritems():
                self.sample_groups[ sample ] = group
                self.sample_configs[ sample ] = config

    @cached_property
    def dblink( self ):
        return aachen3adb.ObjectDatabaseConnection()

    def group( self, sample ):
        return self.sample_groups[ sample ]

    @property
    def groups( self ):
        return self.samplecfg.keys()

    def is_data_group( self, group ):
        return "data" in group

    def is_signal_group( self, group ):
        return "signal" in group

    def is_mc_group( self, group ):
        return not self.is_data_group( group ) and not self.is_signal_group( group )

    def is_data_sample( self, sample ):
        return self.is_data_group( self.group( sample ) )

    def is_signal_sample( self, sample ):
        return self.is_signal_group( self.group( sample ) )

    def is_mc_sample( self, sample ):
        return self.is_mc_group( self.group( sample ) )

    def get_group_type( self, sample ):
        if self.is_data_group( sample ):
            return "data"
        if self.is_signal_group( sample ):
            return "signal"
        else:
            return "mc"

    def get_mc_groups( self ):
        return [ g for g in self.groups if self.is_mc_group(g)]

    def get_data_groups( self ):
        return [ g for g in self.groups if self.is_data_group(g)]

    def get_signal_groups( self ):
        return [ g for g in self.groups if self.is_signal_group(g)]

    def samples( self, group=None ):
        if group is None:
            return self.sample_groups.keys()
        else:
            return self.samplecfg[ group ].keys()

    def args( self, sample ):
        return self.sample_configs[ sample ]

    def files( self, sample, site_order=( "T2_DE_RWTH", "T2_DE_DESY" ) ):
        time.sleep(random.randint(1, 15) * 0.1)
        skim, sample = self._query_db( sample )

        site = None
        for test in site_order:
            if hasattr( skim, test ) and int( getattr( skim, test ) ) == 1:
                site = test
                break

        return [ (site, f.path, f.nevents) for f in skim.files ]

    def _query_db( self, sample_name ):
        key = sample_name
        if not key in self._db_cache:
            skim_criteria, sample_criteria = ramutils.get_selection_criteria( self.settings )
            sample_criteria["name"] = sample_name

            sampletype = "mc"
            if self.is_data_sample( sample_name ):
                sampletype = "data"

            a = ramutils.AnalysisSample(
                self.dblink,  # central link to database
                sampletype,  # data or mc
                "",  # title of group
                sample_name,  # identifier
                ""  # sample arguments
            )

            if not a.load_db_entry(sample_criteria, skim_criteria):
                raise ValueError( "Could not find sample '%s' in database." % sample_name )

            self._db_cache[ key ] = (a.skim, a.sample)

        return self._db_cache[ key ]

class ECBaseMixin(object):
    mc = luigi.Parameter('')
    data = luigi.Parameter(default='')
    signal = luigi.Parameter(default='')
    forced_data_scan_hash = luigi.Parameter(default='')
    forced_mc_scan_hash = luigi.Parameter(default='')
    forced_signal_scan_hash = luigi.Parameter(default='')

    @staticmethod
    def get_scan_hash(scan_key,
                      data_path,
                      distribution,
                      signal_path=None,
                      integralScan=False,
                      filter_systematics=None):
        hash_components = [scan_key]
        hash_components.append(distribution)
        if data_path:
            hash_components.append("withData")
        if integralScan:
            hash_components.append("IntegralScan")
        if signal_path:
            hash_components.append("withSignal")
        if filter_systematics:
            hash_components += filter_systematics
        hash_components.append(ECBaseMixin.get_git_revision_short_hash())
        return hashlib.sha1(''.join(hash_components)).hexdigest()

    @staticmethod
    def get_git_revision_short_hash():
        with tmp_chdir(os.getenv('SCAN_BASE')):
            return subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).strip()

    @property
    def scan_hash(self):
        if self.data and self.forced_data_scan_hash:
            return self.forced_data_scan_hash
        if not self.data and self.signal and self.forced_signal_scan_hash:
            return self.forced_signal_scan_hash
        if not self.data and not self.signal and self.forced_mc_scan_hash:
            return self.forced_mc_scan_hash
        integral_scan = getattr(self, "integral_scan", False) or getattr(self, "integralScan", False)
        return self.get_scan_hash(self.scan_key,
                                  self.data, self.distribution,
                                  signal_path=self.signal,
                                  integralScan=integral_scan)

class ECIOMixin(object):
    remote_dir = luigi.Parameter( default = "" )
    out = luigi.Parameter(default = "")
    exclusive = luigi.BoolParameter(default=False)
    inclusive = luigi.BoolParameter(default=False)
    jet_inclusive = luigi.BoolParameter(default=False)
    _mc_root_file = None
    _data_root_file = None
    _signal_root_file = None
    min_yield = luigi.FloatParameter(default=0.01)

    @property
    def ec_conf_namespace(self):
        ''' Create a argarse like namespace object to emulate ectools configs'''
        parser = argparse.ArgumentParser()
        ecroot.add_ec_standard_options(parser)
        args = self.ec_default_conf_args
        args += self.ec_default_filter_conf_args
        args += ['--jobs', '1']
        return parser.parse_args(args)

    @property
    def ec_default_filter_conf_args(self):
        args = []
        for class_type_info in ecroot.class_types():
            filter_short = class_type_info.name.replace("-","_")
            if hasattr(self, filter_short) and getattr(self, filter_short):
                args.append("--filter-%s" % class_type_info.name)
        return args

    @property
    def ec_minimal_conf_args(self):
        args = []
        if self.mc:
            args += ['--mc', self.mc]
        if self.data:
            args += ['--data', self.data]
        if self.signal:
            args += ['--signal', self.signal]

        return args

    @property
    def ec_default_conf_args(self):
        args = self.ec_minimal_conf_args

        # Set filter functions depending on class type
        for class_type_info in ecroot.class_types():
            class_type = class_type_info.name
            if(getattr(self, class_type.replace("-","_"))):
                args += ['--filter-{class_type}'.format(class_type=class_type)]
        return args

    @property
    def ec_default_conf_namespace(self):
        ''' Create a argarse like namespace object to emulate ectools configs'''
        parser = argparse.ArgumentParser()
        ecroot.add_ec_standard_options(parser)
        args = self.ec_default_conf_args
        conf = parser.parse_args(args)
        return conf

    @property
    def ec_minimal_conf_namespace(self):
        ''' Create a argarse like namespace object to emulate ectools configs'''
        parser = argparse.ArgumentParser()
        ecroot.add_ec_standard_options(parser)
        args = self.ec_minimal_conf_args
        conf = parser.parse_args(args)
        return conf

    @property
    def mc_root_file(self):
        return ecroot.read_root_file(self.mc)

    @property
    def data_root_file(self):
        return ecroot.read_root_file(self.data)

    @property
    def signal_root_file(self):
        return ecroot.read_root_file(self.signal)

    @classmethod
    def get_ec(cls, root_file, ec_name):
        return ecroot.read_ec_object(root_file, ec_name)

    def get_class_names(self, root_file_path=None, distribution="SumPt"):
        '''
            get a list of all class names and total yields and cache the result

            the file specified in the mc parameter will be used if no file is passed
        '''
        if root_file_path is None:
            root_file = self.mc_root_file
        else:
            root_file = ecroot.read_root_file(root_file_path)

        run_hash = ecroot.get_run_hash_from_file(root_file)
        cache_name = os.path.join("ec_names_" + run_hash + ".json")
        ec_names = {}
        root_file.Close()
        conf = self.ec_minimal_conf_namespace
        if not os.path.exists(cache_name):
            logger.info("Getting initial list of class names for class with hash %s, may take some time..." % run_hash)
            ec_names_raw = ecroot.ec_loop(conf,
                                          ecroot.get_class_yields,
                                          root_setup=False,
                                          distribution=distribution)
            ec_names_dict = {r[0]:r[1] for r in ec_names_raw if r[0]}
            with open(cache_name, "w") as jf:
                logger.info("Dumped new list of class names")
                json.dump(ec_names_dict, jf)
        else:
            with open(cache_name, "r") as jf:
                ec_names_dict = json.load(jf)
        ec_names = [name for name, integral in ec_names_dict.items() if integral > self.min_yield]

        if distribution == "MET":
            ec_names = [n for n in ec_names if "MET" in n]
        keep, veto = ecroot.filter_class_names(set(ec_names),
                                               self.ec_conf_namespace)
        return keep


class ECFilterMixin(ECIOMixin):
    ecfilter = luigi.ListParameter(default=[])
    ecveto = luigi.ListParameter(default=[])

    def get_class_names(self, root_file_path=None, distribution="SumPt"):
        names = super(ECFilterMixin, self).get_class_names(root_file_path=root_file_path)
        names_good, vetoed = ecroot.filter_class_names(names, self.ec_default_conf_namespace)
        return names_good

    @property
    def ec_default_conf_args(self):
        args = super(ECFilterMixin, self).ec_default_conf_args
        if self.ecfilter:
            args += ['--filter']
            args += self.ecfilter
        if self.ecveto:
            args += ['--veto']
            args += self.ecveto
        return args

class RemoteResultMixin(object):
    remote_dir = luigi.Parameter( default = "." )
    campaign_id = luigi.Parameter( default = "campaign" )

    @property
    def remote_path(self):
        return self.get_remote_path(self.remote_dir, self.campaign_id)

    @classmethod
    def get_remote_path(cls, remote_dir, campaign_id):
        # check if a dir for this hash exists already
        dirs = [os.path.join(remote_dir, o) for o in os.listdir(remote_dir)
            if os.path.isdir(os.path.join(remote_dir,o)) and campaign_id in o]
        if dirs:
            # check if several path fulfill requirement and only take closest match
            if len(dirs) > 1:
                dirs = sorted(dirs, key=len)
            return dirs[0]
        else:
            datestamp = datetime.date.strftime( datetime.date.today(), "%Y_%m_%d")
            return os.path.join(remote_dir, campaign_id + "_" + datestamp )


if __name__=="__main__":
    print(myskims.groups(), myskims.get_mc_groups())
