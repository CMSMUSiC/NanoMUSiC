import os
import shutil
import abc
import json
import uuid
import glob
import csv
import fnmatch
import argparse
import numpy as np
import collections
import tarfile
import logging
import math
import dbtools
import numpy as np
import datetime
import time


from luigi_music.util import sdb

import luigi
from luigi.contrib.external_program import ExternalProgramTask

#3A Tools
import gridlib

# MUSiC-Utils
import roothelpers
from ectools.register import ecroot
from ectools.misc import rremove

from shellhelpers import expand_path, tmp_chdir


from .util import get_shared_parameters, _flatten, ECBaseMixin, ECIOMixin, ECFilterMixin
from .grid import GridTask
from luigi_music.targets import GridTarget,GridTaskTarget, SystSeedsTarget, ScanResultTarget, \
                                LocalScanTarget, ScanCorrectedResultTarget

logger = logging.getLogger('luigi-interface')

# Create a container class for the MCBin.
MCBin = collections.namedtuple('MCBin', ( 'mcEventsPerProcessGroup', 'mcStatUncertPerProcessGroup', 'lowerEdge', 'width', 'mcSysUncerts', 'unweightedEvents' ) )

class _BaseScanMixin( ECBaseMixin ):
    scan_key = luigi.Parameter(default='testscan')
    compare_scan_hash = luigi.Parameter(default='')
    filter_systematics = luigi.TupleParameter(default=[])
    shift_file_path = luigi.Parameter(default='shifts.json')
    nrounds = luigi.IntParameter(default=0)
    nrounds_signal = luigi.IntParameter(default=0)
    distribution = luigi.Parameter(default='SumPt')
    timing_dict_path = luigi.Parameter(default='', significant=False)

class _RemoteMixin(_BaseScanMixin):
    remote = luigi.BoolParameter(default=False, significant=False)
    site = luigi.Parameter(default='T2_DE_RWTH', significant=False)
    #delegation_id = luigi.Parameter(default='', significant=False)#Yannik commented it out (debug)
    min_remote_time = luigi.IntParameter(default=600, significant=False)

class _ScanMixin( _RemoteMixin ):

    runtime_per_job = luigi.IntParameter(default=4800, significant=False) # runtime in secs

    # scanner config
    minRegionWidth = luigi.IntParameter(default=3)
    coverageThreshold = luigi.FloatParameter(default=0.)
    regionYieldThreshold = luigi.FloatParameter(default=1e-6)
    sigmaThreshold = luigi.FloatParameter(default=0.6)
    integralScan = luigi.BoolParameter(default=False)
    skipLookupTable = luigi.BoolParameter(default=False)
    noLowStatsTreatment = luigi.BoolParameter(default=False)
    widthLowStatsRegions = luigi.IntParameter(default=4)
    thresholdLowStatsDominant = luigi.FloatParameter(default=0.9)
    mcStatUncertScaleFactor = luigi.FloatParameter(default=1.0)
    dicedSignalUncertScaleFactor = luigi.FloatParameter(default=1.0)
    dicedMCUncertScaleFactor = luigi.FloatParameter(default=1.0)
    _timing_dict = None

    #def ensure_delegation(self):
     #   if self.remote and not self.delegation_id:
      #      se = gridlib.se.get_se_instance( self.site )
       #     proxy = gridlib.ce.ProxyDelegator( se )
        #    self.delegation_id = proxy.get_delegation_id()
        #return self.delegation_id #Yannik commented it out (debug)

    @property
    def timing_dict(self):
        if not self._timing_dict:
            if os.path.exists(self.timing_dict_path):
                with open(self.timing_dict_path, 'r') as tf:
                    self._timing_dict = json.load(tf)
            else:
                logger.info("Unable to find timing file:" + self.timing_dict_path)
                self._timing_dict = {}
        return self._timing_dict

class _ScanDBMixin( sdb._ScanDBSessionMixin ):
    scan_db_path = luigi.Parameter(default="scandb.sqlite")
    connect_args = {}

    @abc.abstractproperty
    def connection_string(self):
        return None

class _ScanIOMixin( ECFilterMixin ):
    pass

class ScanDBInitTask(luigi.Task, _ScanDBMixin):

    def run(self):
        if not os.path.exists(self.scan_db_path):
            dbtools.init_database( self.engine )

    def output(self):
        return luigi.LocalTarget(path=self.scan_db_path)

class SingleBinScanTask(luigi.Task, _ScanMixin, _ScanIOMixin, _ScanDBMixin):

    def requires(self):
        if not os.path.exists(self.scan_db_path):
            dbtools.init_database( self.engine )

        tasks = []
        roothelpers.root_setup()
        from pprint import pprint
        kwargs = get_shared_parameters(self, ScanTask)
        for class_type_info in ecroot.class_types():
            kwargs_job = kwargs.copy()
            # make sure all other types are turned off
            for class_type_info_temp in ecroot.class_types():
                kwargs_job[class_type_info_temp.name.replace("-","_")] = False
            kwargs_job[class_type_info.name.replace("-","_")] = True
            kwargs_job["integralScan"] = True
            kwargs_job['filter_systematics'] = self.filter_systematics
            #~ pprint(kwargs_job)
            tasks.append(ScanTask(**kwargs_job))
        return tasks

    #~ def run(self):
        #~ for results in self.input():


    def output(self):
        return self.input()

class ScanTask( luigi.Task, _ScanMixin, _ScanIOMixin, _ScanDBMixin ):
    ''' Wrapper Task creates splitting and starts tasks '''
    forced_class_names = luigi.ListParameter(default=[])

    @property
    def resources(self):
        return {self.scan_db_path : 1}

    def get_class_names(self, root_file_path=None):
        if self.forced_class_names:
            names = self.forced_class_names
        else:
            names = super(ScanTask, self).get_class_names(root_file_path=root_file_path)
        if "MET" == self.distribution:
            names = [n for n in names if "MET" in n ]
        return names

    def requires( self ):
        #if self.remote:
            #self.ensure_delegation()   Yannik comment it out (debug)

        roothelpers.root_setup()
        jobs = {'ScanWriteBack':[]}

        for ec_name in self.get_class_names():
            if not ec_name:
                continue
            kwargs = get_shared_parameters(self, ScanWriteBack)
            kwargs['ec_name'] = ec_name
            #kwargs['delegation_id'] = self.delegation_id #Yannik commented it out (debug)
            kwargs['expected_round_runtime'] = self.timing_dict.get(ec_name, 2)
            kwargs['filter_systematics'] = self.filter_systematics
            if self.data:
                kwargs['nrounds'] = 1
            if self.signal:
                kwargs['nrounds'] = self.nrounds_signal
            jobs['ScanWriteBack'].append(ScanWriteBack(**kwargs))

        # For the final comparision we need an MC scan for comparision.
        # Add another scan task to the requirements for a MC scan if rounds
        # are expected
        if (self.data or self.signal) and self.nrounds:
            mc_scan_args = get_shared_parameters(self, ScanTask)
            mc_scan_args['filter_systematics'] = self.filter_systematics
            mc_scan_args['data'] = ''
            mc_scan_args['signal'] = ''
            jobs['ScanTask'] = ScanTask(**mc_scan_args)
        return jobs

    @property
    def state_file_path(self):
        return "state_%s" % self.scan_hash

    def calculate_compare_scores(self, session, compare_scan, destination_scan):
        query = session.query( dbtools.Result.event_class ).distinct() \
            .filter( dbtools.Result.hash==compare_scan.hash )

        event_classes = [ result[ 0 ] for result in query ]
        N_before_filtering = len( event_classes )

        logger.info( 'Processing %d / %d event classes.' % ( len( event_classes ),
                N_before_filtering ) )

        insert = dbtools.CorrectedResult.__table__.insert().prefix_with( 'OR REPLACE' )
        for i, event_class in enumerate( event_classes ):
            logger.info( 'Progress: %d / %d' % ( i+1, len( event_classes ) ) )

            query = session.query( dbtools.Result.round, dbtools.Result.score ) \
                    .filter( dbtools.Result.hash==compare_scan.hash, dbtools.Result.event_class==event_class )

            pseudo = list( query )
            if not pseudo:
                continue

            pseudo = np.array( pseudo )

            pseudo_p_values = pseudo[:,1]
            pseudo_round_indices = pseudo[:,0]

            if destination_scan == compare_scan:
                corrected_p_values = ecroot.get_pseudo_p_tilde_list( pseudo_p_values, correct_if_zero=False )
                corrected_tuples = zip( pseudo_round_indices, corrected_p_values )
            else:
                query = session.query( dbtools.Result ) \
                        .filter( dbtools.Result.hash==destination_scan.hash, dbtools.Result.event_class==event_class )

                if query.count() == 0:
                    logger.info( 'No results for EC %s, skipping...' % event_class )
                    continue

                corrected_tuples = [ ( result.round, ecroot.calc_p_tilde( pseudo_p_values, result.score, correct_if_zero=False ) ) for result in query ]


            mappings = [
                { 'hash': destination_scan.hash,
                  'event_class': event_class,
                  'round': round,
                  'compared_hash': compare_scan.hash,
                  'score': corrected_p_value,
                  'comp_count': len( pseudo_p_values ),
                  } for round, corrected_p_value in corrected_tuples
            ]
            assert mappings
            session.execute( insert, mappings )

            session.flush()
            session.commit()

    def run(self):
        '''Calculate p-tilde value comparing results for hash one with hash two
        (e.g. mc_hash mc_hash, mc_hash data_hash)'''

        # default case: compare mc with mc
        compare_hash = self.scan_hash

        # try to get data if no compare hash was defined
        if (self.data or self.signal) and not self.compare_scan_hash:
            # Use output from required mc pseudo scans if rounds were given
            if not self.nrounds:
                raise ValueError('Data scan without compare hash !')
            compare_hash =  self.get_scan_hash(self.scan_key,
                                               None,
                                               self.distribution,
                                               signal_path=self.signal,
                                               integralScan=self.integralScan,
                                               filter_systematics=self.filter_systematics)

        if self.compare_scan_hash:
            compare_hash = self.compare_scan_hash

        with self.session() as session:

            compare_scan = dbtools.match_hash( compare_hash, session )
            destination_scan = dbtools.match_hash( self.scan_hash, session )

            if compare_scan.distribution != destination_scan.distribution:
                logger.info( 'The distribution types \'%s\' and \'%s\' seem not to match.' % ( compare_scan.distribution, destination_scan.distribution ) )
                raise ValueError("Distribution types need to match!")
            logger.info("Calculating compare scores for hashs {}:{} {}:{}".format(compare_scan.scan_type,
                                                                                  compare_scan.hash,
                                                                                  destination_scan.scan_type,
                                                                                  destination_scan.hash))
            self.calculate_compare_scores(session, compare_scan, destination_scan)
        for o in self.output():
            if not o.exists():
                print("Did not find target for class %s %s %s " % (o.ec_name, o.hash, o.compared_hash))

    def complete(self):
        # return false if database is not yet initialized, saves time for many os exists checks
        if not os.path.exists(self.scan_db_path):
            return False
        # use standard way if database exists
        return super(ScanTask, self).complete()

    def output(self):
        compared_hash = self.compare_scan_hash
        nrounds = self.nrounds
        if self.signal:
            nrounds = self.nrounds_signal
        elif self.data:
            nrounds = 1
        if (self.data or self.signal) and not self.compare_scan_hash:
            if self.nrounds or self.nrounds_signal:
                task_input = self.input()
                if not self.compare_scan_hash:
                    compared_hash = self.get_scan_hash(self.scan_key,
                                                       None,
                                                       self.distribution,
                                                       signal_path=self.signal,
                                                       integralScan=self.integralScan,
                                                       filter_systematics=self.filter_systematics)

            elif not self.compare_scan_hash:
                raise ValueError("Need compare hash for data or nrounds for new scan")
        if not compared_hash:
            compared_hash = self.scan_hash
        outputs = []
        for ec_name in self.get_class_names():
            outputs.append(ScanCorrectedResultTarget(self.scan_db_path,
                                                 self.scan_hash,
                                                 compared_hash,
                                                 ec_name,
                                                 nrounds))
        return outputs

class _ScanJobMixin( _ScanDBMixin, _ScanMixin , _ScanIOMixin ):
    ec_name = luigi.Parameter()
    input_json_dir = luigi.Parameter( default='jsontemp', significant=False )
    expected_round_runtime = luigi.IntParameter(default=2)

    @property
    def input_json_name( self ):
        return self.ec_name + '_' + self.distribution + self.scan_hash + '.json'

    @property
    def input_json_path( self ):
        return os.path.join(self.input_json_dir, self.input_json_name)

class ECScanJob(luigi.Task, _ScanJobMixin):

    def requires(self):
        start_time = time.time()
        total_ec_time = self.expected_round_runtime * self.nrounds
        run_remote = False
        if self.remote and total_ec_time > self.min_remote_time and not self.integralScan:
            job_class = RemoteScanJob
            #self.ensure_delegation() Yannik commented it out (debug)
            run_remote = True
        else:
            job_class = LocalScanJob #Yannik commented it out
            #job_class = RemoteScanJob #Yannik added this line
            #run_remote = True #Yannik added this line

        job_kwargs = get_shared_parameters(self, job_class)
        job_kwargs['filter_systematics'] = self.filter_systematics
        job_kwargs['gridpack_base_name'] = 'gridpack' + self.ec_name + '_'+ self.distribution + '_' + self.scan_hash + '.tar.gz'
        if run_remote:
            shift_file_path = self.shift_file_path
            if self.signal:
                shift_file_path = "signal_shifts.json"
            seed_file_path = SeedCreationTask.get_output_path(shift_file_path,
                                                                self.scan_hash)
            job_kwargs['executable'] = 'python/cewrapper.py'
            job_kwargs['gridpack_files'] = ['python/cewrapper.py',
                                            'bin/scanClass',
                                            os.path.abspath(seed_file_path),
                                            os.path.abspath(self.input_json_path),
                                            'bin/lookuptable.bin']
            job_kwargs['gridpack_base'] = os.getenv('SCAN_BASE')
            job_kwargs['output_files'] = [job_class.get_generic_output_arch_name(self.input_json_name)]
            job_kwargs['job_args'] = ['-e', 'bin/scanClass',
                                      '--input-json', self.input_json_name,
                                      '-s', seed_file_path]
            job_kwargs['task_name'] = self.ec_name + '_' + self.scan_hash
            job_kwargs['chunk_size'] = 1
            #job_kwargs['delegation_id'] = self.delegation_id #Yannik commented it out (debug)

            job_kwargs['delete_gridpack'] = False

        else:
            job_kwargs['executable'] = os.path.join(os.getenv('SCAN_BASE'),
                                                    'bin/scanClass')
        return job_class(**job_kwargs)

    def output(self):
        output = self.input()
        return output

class ScanWriteBack(luigi.Task, _ScanJobMixin):

    priority = 200

    @property
    def resources(self):
        return {self.scan_db_path : 1}

    def requires(self):
        job_kwargs = get_shared_parameters(self, ECScanJob)
        scan_kwargs = get_shared_parameters(self, ScanDBInitTask)
        job_kwargs['filter_systematics'] = self.filter_systematics
        return {'ScanDBInitTask' : ScanDBInitTask(**scan_kwargs),
                'ECScanJob' : ECScanJob(**job_kwargs)}

    def run(self):
        scan_job_result = self.input()['ECScanJob']
        with self.session() as session:
            logger.info(scan_job_result.files)
            for path in [ f for f in scan_job_result.files if f.endswith('.tar.gz')]:
                for info, file in self.archive_ls( path, '*_info.json' ):
                    print( 'Working JSON:', path, '>', info.name )
                    for mapper, mappings in self._handle_json(file):
                        self.insert_in_db(mapper, mappings, session)

                no_output_found = True
                for info, file in self.archive_ls( path, '*_output.csv' ):
                    mapper, mappings = self._handle_csv(file)
                    print( 'Working CSV:', path, '>', info.name )
                    self.insert_in_db(mapper, mappings, session)
                    no_output_found = False
                if no_output_found:
                    logger.error("Empty archive for job, deleting")
                    os.remove(path)
            session.flush()
            session.commit()


    def insert_in_db(self, mapper, mappings, session):

        #session.bulk_update_mappings( mapper, mappings )
        # almost equivalent (to commented out code, because bulk_update_mappings
        # is only available from sqlalchemy version 1.0
        insert = mapper.__table__.insert().prefix_with( 'OR REPLACE' )
        try:
            session.execute( insert, mappings )
        except Exception as e:
            print('Error inserting: ', mappings)
            raise

        session.flush()

    def _handle_csv( self, file ):
        reader = csv.DictReader( file, delimiter=',' )
        return dbtools.Result, list( reader )

    def _handle_json( self, file, name=None ):
        data = json.load( file )
        scan = dict( hash=data[ 'hash' ],
            date= datetime.datetime.now(),
            distribution=data[ 'distribution' ],
            scan_type=data[ 'ScanType' ],
            name=self.scan_key )

        yield dbtools.Scan, [ scan ]

        for timing_type, timing_values in data[ 'timing' ].iteritems():
            timing = dict( hash=data[ 'hash' ],
                event_class=data[ 'name' ],
                timing_type=timing_type,
                first_round=data[ 'firstRound' ],
                count=timing_values[ 'count' ],
                total=timing_values[ 'total' ] )

            if timing[ 'count'] > 2**32:
                print('Warning: timing count too large for DB: %d' % timing[ 'count' ])
                timing[ 'count' ] = None

            yield dbtools.Timing, [ timing ]

        for stat_type, stat_value in data[ 'stats' ].iteritems():
            stat = dict( hash=data[ 'hash' ],
                event_class=data[ 'name' ],
                stat_type=stat_type,
                first_round=data[ 'firstRound' ],
                value=stat_value )

            if stat[ 'value'] > 2**32:
                print('Warning: stat value too large for DB: %d' % stat[ 'value' ])
                stat[ 'value' ] = None

            yield dbtools.Stat, [ stat ]

    @staticmethod
    def datetime_from_uuid( u ):
        # magic values etc from http://stackoverflow.com/a/3795750/489345
        return datetime.datetime.fromtimestamp( ( u.time - 0x01b21dd213814000L )/1e7 )

    @classmethod
    def archive_ls( cls, filename, pattern=None ):
        with tarfile.open( filename, 'r' ) as archive:
            for tarinfo in archive:
                if not pattern or fnmatch.fnmatch( tarinfo.name, pattern ):
                    yield tarinfo, archive.extractfile( tarinfo )

    def output(self):
        return ScanResultTarget(self.scan_db_path,
                                self.scan_hash,
                                self.ec_name,
                                self.distribution,
                                0,
                                self.nrounds)

class SeedCreationTask(luigi.Task, _ScanMixin):
    priority = 350
    systematic_names = luigi.TupleParameter()

    @staticmethod
    def get_output_path(shift_file_path, scan_hash):
        base, ext = os.path.splitext(os.path.basename(shift_file_path))
        return os.path.join(os.path.dirname(shift_file_path),
                            base + scan_hash + ext)

    def output(self):
        return SystSeedsTarget(self.get_output_path(self.shift_file_path, self.scan_hash),
                               self.systematic_names,
                               self.nrounds)

    def run(self):
        with open(self.get_output_path(self.shift_file_path, self.scan_hash), 'w') as jf:
            json.dump(self.create_systematic_shifts(), jf, indent=4)

    def create_systematic_shifts(self):
        shifts = np.random.normal( loc=0.0,
                                   scale=1.0,
                                   size=( len( self.systematic_names ),
                                   self.nrounds ) )
        shifts = [list(s) for s in shifts]
        dictionary = dict( zip( self.systematic_names, shifts ) )
        return dictionary

class _ScanChunkMixin( _ScanJobMixin ):
    first_round = luigi.IntParameter(default=0)
    last_round = luigi.IntParameter(default=0)
    chunk_id = luigi.IntParameter(default=0)
    gridpack_base_name = luigi.Parameter( default='Scangridpack.tar.bz2' )

    @staticmethod
    def get_output_arch_name( input_json_name, chunk_id ):
        outpath = os.path.basename( input_json_name )
        return outpath.replace('.json', '%d_Output.tar.gz' % chunk_id)

    @staticmethod
    def get_generic_output_arch_name( input_json_name ):
        outpath = os.path.basename( input_json_name )
        return outpath.replace('.json', '*_Output.tar.gz')#Yannik commented out. This is DEFAULT LINE

    def get_raw_systematic_names(self):
        if self.signal:
            cache_name = "raw_syst_names_signal" + self.scan_hash + ".json"
        else:
            cache_name = "raw_syst_names" + self.scan_hash + ".json"
        raw_syst_names = []
        if not os.path.exists(cache_name):
            logger.info("Getting initial list of raw syst names")
            roothelpers.root_setup()
            if self.signal:
                empty_ec = self.get_ec(self.signal_root_file, 'Rec_Empty+X')
            else:
                empty_ec = self.get_ec(self.mc_root_file, 'Rec_Empty+X')
            raw_syst_names = empty_ec.getSystematicNames()
            broken_systs = ("scale_factor_stat", "*/*uninitialized*")
            logger.info(raw_syst_names)
            syst_names = set()
            for ec_systematic_name in raw_syst_names:
                ec_systematic_name = rremove(ec_systematic_name, 'Up')
                ec_systematic_name = rremove(ec_systematic_name, 'Down')
                vetoed =False
                for veto in self.filter_systematics:
                    if veto in ec_systematic_name:
                        vetoed =True
                        break
                if ecroot._skip_systematic("*", ec_systematic_name, self.filter_systematics):
                    continue
                if vetoed:
                    continue
                if "scale_factor_stat" in ec_systematic_name:
                    continue
                syst_names.add(ec_systematic_name)

            with open(cache_name, "w") as jf:
                json.dump([s for s in list(syst_names) if not any(veto in s for veto in broken_systs)], jf)
            empty_ec.Delete()
            logger.info("Dumped initial list of raw syst names")
        else:
            with open(cache_name, "r") as jf:
                raw_syst_names = json.load(jf)
        return raw_syst_names

    def get_chunk_requirements(self, is_remote=False):

        seed_kwargs = get_shared_parameters(self, SeedCreationTask)

        raw_syst_names = self.get_raw_systematic_names()

        syst_names = set()
        for ec_systematic_name in raw_syst_names:
            ec_systematic_name = rremove(ec_systematic_name, 'Up')
            ec_systematic_name = rremove(ec_systematic_name, 'Down')
            vetoed =False
            for veto in self.filter_systematics:
                if veto in ec_systematic_name:
                    vetoed =True
                    break
            if ecroot._skip_systematic("*", ec_systematic_name, self.filter_systematics):
                continue
            if vetoed:
                continue
            if "scale_factor_stat" in ec_systematic_name:
                continue
            syst_names.add(ec_systematic_name)
        if self.signal:
            seed_kwargs["shift_file_path"] = "signal_shifts.json"
        seed_kwargs['filter_systematics'] = self.filter_systematics
        seed_task = SeedCreationTask(systematic_names=list(syst_names),
                                     **seed_kwargs)
        arch_kwargs = get_shared_parameters(self, ECScanInputCreationTask)
        arch_kwargs['is_remote'] = is_remote
        arch_kwargs['filter_systematics'] = self.filter_systematics
        return {'SeedCreationTask':seed_task,
                'ECScanInputCreationTask' : ECScanInputCreationTask(**arch_kwargs)}

class ECScanInputCreationTask(luigi.Task, _ScanChunkMixin):
    is_remote = luigi.BoolParameter(default=False, significant=False)

    @property
    def priority(self):
        if self.is_remote:
            return 300
        else:
            return 120

    def run(self):
        d = {
            'gridpack_name' : self.gridpack_base_name,
            'minRegionWidth' : self.minRegionWidth,
            'coverageThreshold' : self.coverageThreshold,
            'regionYieldThreshold' : self.regionYieldThreshold,
            'sigmaThreshold' : self.sigmaThreshold,
            'integralScan' : self.integralScan,
            'skipLookupTable' : self.skipLookupTable,
            'noLowStatsTreatment' : self.noLowStatsTreatment,
            'widthLowStatsRegions' : self.widthLowStatsRegions,
            'thresholdLowStatsDominant' : self.thresholdLowStatsDominant,
            'mcStatUncertScaleFactor' : self.mcStatUncertScaleFactor,
            'dicedMCUncertScaleFactor' : self.dicedMCUncertScaleFactor,
            'dicedSignalUncertScaleFactor' : self.dicedSignalUncertScaleFactor,
            'hash': self.scan_hash,
            'name': self.ec_name,
            'distribution': self.distribution,
        }
        roothelpers.root_setup()
        if not self.mc_root_file:
            raise AttributeError('MC root file needed for scan')

        mc_ec = ecroot.read_ec_object( self.mc_root_file, self.ec_name)
        d['MCBins'] = _flatten( self._extract_mc_bins( mc_ec,
                                                       self.distribution,
                                                       self.filter_systematics,
                                                       self.scan_key ))
        if self.data:
            data_ec = ecroot.read_ec_object( self.data_root_file, self.ec_name)
            empty = False
            if not data_ec:
                data_ec = mc_ec
                empty = True
            hist = ecroot.total_event_yield( data_ec, self.distribution )
            d['DataBins'] = _flatten(roothelpers.root_map_hist( hist.GetBinContent, hist, empty ))
        elif self.signal:
            signal_ec = ecroot.read_ec_object( self.signal_root_file, self.ec_name)
            #merged_ec = mc_ec.Copy()
            #if signal_ec:
            #    # merge signal and background to build the s+b hypothesis
            #    all_processes = merged_ec.getGlobalProcessList()
            #    # build a root set object to interact with TEventClass object
            #    processesToMerge = r.set( 'string' )()
            #    for proc in all_processes:
            #        processesToMerge.insert( processesToMerge.begin(), str( proc ) )
            #    merged_ec.addEventClass( signal_ec, processesToMerge, False )
            #d['SignalBins'] = _flatten( self._extract_mc_bins( merged_ec,
            #                                                   self.distribution,
            #                                                   self.filter_systematics ))
            d['SignalBins'] = _flatten( self._extract_mc_bins( signal_ec,
                                                               self.distribution,
                                                               self.filter_systematics,
                                                               self.scan_key ))
            d['FirstRound'] = 0
            d['NumRounds'] = self.nrounds_signal
        else:
            d['FirstRound'] = self.first_round
            d['NumRounds'] = self.last_round - self.first_round
        self.output().makedirs()
        with open(self.output().path, 'wb') as jf:
            json.dump(d, jf, indent=2)

    @classmethod
    def _extract_mc_bins( cls, event_class, distribution, filter_systematics=None, scan_key="" ):
        if filter_systematics is None:
            filter_systematics = []
        mc_hists = ecroot.event_yield_by_process_group( event_class, distribution, aggregation_level='none' )
        uncertainty_hists = ecroot.combined_uncertainty_by_systematic_name( event_class,
                                                                         distribution,
                                                                         symmetrize=False,
                                                                         filter_systematics=filter_systematics )
        unweighted_hists = cls._extract_histograms( event_class, distribution )
        retval = cls._hists2bins( mc_hists, uncertainty_hists, unweighted_hists, scan_key)

        for hist in mc_hists.values():
            hist.Delete()
        for hist in uncertainty_hists.values():
            hist.Delete()
        for hists in unweighted_hists.values():
            hists['hist'].Delete()

        return retval

    @classmethod
    def _extract_histograms(cls,  event_class, distribution ):
        '''Read histogram and corresponding error histograms.'''

        # Initalize dict for unweighted number of events.
        # Is filled with dicts { 'hist': unweighted histogram, 'xs': crossection }
        # for each process_group.
        unweighted_hists = {}

        # Add up hists and errors for all processes
        for process_name in list( event_class.getProcessList() ):
            unweighted_hists[ process_name ] = {
                'hist' : event_class.getHistoPointerUnweighted( process_name, distribution ).Clone(),
                'xs'   : event_class.getCrossSection( process_name )
            }

        return unweighted_hists

    @classmethod
    def _hists2bins( self, mc_hists, uncertainty_hists, unweighted_hists, scan_key="" ):
        assert len( mc_hists ) > 0

        bins = []
        Nbins = mc_hists.values()[0].GetNbinsX()

        for ibin in range( 1, Nbins+1 ):
            # Obtain lowerEdge and width from 'some' contribution (doesn't matter which)
            lowerEdge = mc_hists.values()[0].GetBinLowEdge( ibin )
            width = mc_hists.values()[0].GetBinWidth( ibin )

            mcEventsPerProcessGroup = collections.OrderedDict()
            mcStatUncertPerProcessGroup = collections.OrderedDict()
            for process_name, process_hist in mc_hists.items():
                assert process_hist.GetBinLowEdge( ibin ) == lowerEdge
                assert process_hist.GetBinWidth( ibin ) == width
                mcEventsPerProcessGroup[ process_name ] = process_hist.GetBinContent( ibin )
                mcStatUncertPerProcessGroup[ process_name ] = process_hist.GetBinError( ibin )
            if "ReduceAllSyst" in scan_key:
                logger.info("Reducing all systs to 50%")
            mcSysUncerts = collections.OrderedDict()
            # Add uncertainties.
            for systematic_name, systematic_hist in uncertainty_hists.items():
                assert systematic_hist.GetBinLowEdge( ibin ) == lowerEdge
                assert systematic_hist.GetBinWidth( ibin ) == width
                if "Jet_systScale" in systematic_name and  ("reduceJetScale" in scan_key or "reduceJetUncerts" in scan_key):
                    logger.info("Reducing jet scale by 0.5")
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin ) * 0.5
                elif "Jet_systResolution" in systematic_name and  ("reduceJetRes" in scan_key or "reduceJetUncerts" in scan_key):
                    logger.info("Reducing jet res by 0.5")
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin ) * 0.5
                elif "reduceAllSyst" in scan_key:
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin ) * 0.5
                elif "reduceLOXS" in scan_key and systematic_name.endswith("LO"):
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin ) * 0.5
                elif "reduceMETuncert" in scan_key and "slimmedMETs_systScale" in systematic_name:
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin ) * 0.5
                elif "reduceQCDWeight" in scan_key and systematic_name.startswith("qcdWeight"):
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin ) * 0.5
                elif "reducePDFuncert" in scan_key and systematic_name.endswith("pdfuponly"):
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin ) * 0.5
                elif "reduceFakeUncert" in scan_key and "fake" in systematic_name:
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin ) * 0.5
                elif "reduceAllScaleUncerts" in scan_key and "systScale" in systematic_name:
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin ) * 0.5
                else:
                    mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin )

            unweightedEvents = collections.OrderedDict()
            # Add unweighted event numbers and cross sections.
            for process_name, unweighted_hist in unweighted_hists.items():
                assert unweighted_hist[ 'hist' ].GetBinLowEdge( ibin ) == lowerEdge
                assert unweighted_hist[ 'hist' ].GetBinWidth( ibin ) == width
                unweightedEvents[ process_name ] = {
                    'Nevents':  unweighted_hist[ 'hist' ].GetBinContent( ibin ),
                    'xs' :      unweighted_hist[ 'xs' ]
                }

            bins.append( MCBin(
                mcEventsPerProcessGroup = mcEventsPerProcessGroup,
                mcStatUncertPerProcessGroup = mcStatUncertPerProcessGroup,
                lowerEdge = lowerEdge,
                width = width,
                mcSysUncerts = mcSysUncerts,
                unweightedEvents = unweightedEvents,
            ) )

        return bins

    def output(self):
        return luigi.LocalTarget( path=self.input_json_path )

class RemoteScanJob(GridTask, _ScanChunkMixin):
    priority = 110
    def requires(self):
        req = super(RemoteScanJob, self).requires()
        req["GridTaskSubmission"].submission_dir = self.scan_submission_dir
        req.update(self.get_chunk_requirements(is_remote=True))
        return req

    def input_arguments( self ):

        ec_runtime = self.timing_dict.get(self.ec_name, 2.5)
        total_ec_time = ec_runtime * self.nrounds
        start_round = 0
        remaining_runtime = self.runtime_per_job
        rounds_per_job = min( self.nrounds, int(math.ceil( float( self.runtime_per_job ) / ec_runtime)))
        chunks = []
        chunk_id = 0
        while True:
            chunk_args = []
            out_arch = self.get_output_arch_name(self.input_json_name, chunk_id)
            additional_rounds =  min(rounds_per_job, self.nrounds - start_round)
            chunk_args += ['-o', out_arch]
            chunk_args += ['--first-round', str(start_round)]
            chunk_args += ['--nrounds', str(additional_rounds)]
            chunks.append(' '.join(chunk_args))

            start_round += additional_rounds
            remaining_runtime -= additional_rounds * ec_runtime
            chunk_id +=1
            if start_round >= self.nrounds:
                break
        return chunks

    @property
    def scan_submission_dir( self ):
        return os.path.join( 'Scan_{}_{}'.format(self.distribution, self.scan_key))

    def run(self):
        if not os.path.exists( self.scan_submission_dir ):
            os.makedirs( self.submission_dir )

        with tmp_chdir( self.scan_submission_dir ):
            super( RemoteScanJob, self ).run()

    def output(self):
        if self.scan_submission_dir in os.getcwd():
            path = self.task_name
        else:
            path = os.path.join(self.scan_submission_dir, self.task_name)

        return GridTaskTarget(path,
                              max_failed=self.max_failed_fraction,
                              target_files=self.output_files)


class LocalScanJob(ExternalProgramTask, _ScanChunkMixin):
    executable = luigi.Parameter( default='scanClass', significant=False )
    shift_path = luigi.Parameter(default='shifts.json', significant=False)
    priority = 110

    def requires(self):
        self.get_chunk_requirements(is_remote=False)
        return self.get_chunk_requirements()

    def program_args(self):
        return [ self.executable,
                  '-j', self.input()['ECScanInputCreationTask'].path,
                  '-s', self.input()['SeedCreationTask'].path,
                  '-l', str(self.first_round),
                  '-o', self.task_id,
                  '-n', str(self.nrounds)
                ]

    @property
    def always_log_stderr(self):
        '''
        When True, stderr will be logged even if program execution succeeded
        '''
        return False

    @property
    def local_output_path(self):
        if self.integralScan:
            return "localOutput_{}_{}".format("integral", self.scan_hash)
        else:
            return "localOutput_{}_{}".format(self.distribution, self.scan_hash)

    def run(self):
        if not os.path.exists(self.task_id):
            os.mkdir(self.task_id)
        if not os.path.exists(self.local_output_path):
            os.mkdir(self.local_output_path)
        super(LocalScanJob, self).run()
        out_archname = os.path.join(self.local_output_path, self.get_output_arch_name( self.input_json_name,
                                                       self.chunk_id ))
        self.pack( out_archname, self.task_id,
            [ '*_output.csv', '*_info.json', '*_regions.root' ] )
        shutil.rmtree(self.task_id)

    def output(self):
        path = os.path.join(self.local_output_path, self.get_output_arch_name( self.input_json_name,
                                                       self.chunk_id ))
        return LocalScanTarget(path=path)

    @staticmethod
    def pack( arch_name, folder, filters ):
        with tarfile.open( arch_name, 'w|gz') as archive:
            last_wd = os.getcwd()
            os.chdir( folder )
            for filter in filters:
                for filename in glob.iglob( filter ):
                    if os.path.isfile( filename ):
                        archive.add( filename )
            os.chdir( last_wd )
