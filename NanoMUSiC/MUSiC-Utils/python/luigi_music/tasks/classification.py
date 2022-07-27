from collections import defaultdict
import json
import logging
import os, os.path
import shlex
import shutil
import fnmatch
import glob
import sys
import tarfile
import warnings
import re

import luigi
from luigi.contrib.external_program import ExternalProgramTask

import gridlib.ce

from shellhelpers import expand_path, tmp_chdir

from luigi_music.targets import GridTarget, GridTaskTarget
from luigi_music.util import chunker, cached_property
from .util import get_shared_parameters

from ectools.register import ecroot
from .grid import GridTask, GridpackPreperationTask, GridTaskFieldMixin
from .util import MyskimsManager

logger = logging.getLogger('luigi-interface')


class SkimmedFileTask( luigi.ExternalTask ):
    path = luigi.Parameter()
    site = luigi.Parameter()

    def output( self ):
        return GridTarget( path=self.path, site=self.site )

class _SampleClassificationMixin( luigi.WrapperTask, GridTaskFieldMixin ):
    remote = luigi.BoolParameter( default=False )
    extra_args = luigi.Parameter( default="" )
    forced_run_hash = luigi.Parameter( default="" )
    max_files = luigi.IntParameter( default=None )
    mean_job_runtime = luigi.IntParameter( default=10800, significant=False )
    myskims_path = luigi.Parameter( default = expand_path("$MUSIC_CONFIG/MC/myskims_ram.yaml") )
    #timing_dict_path = luigi.Parameter( default = "/net/scratch_cms3a/pook/MUSiC/MUSiCout/timing/timing_classification_tasks.json", significant=False )
    timing_dict_path = luigi.Parameter( default = "/net/scratch_cms3a/vigilante/timing_classification_tasks.json", significant=False )
    site = luigi.Parameter( default="T2_DE_RWTH" )
    #delegation_id = luigi.Parameter(default='', significant=False) #Yannik commeted it out (debug)

    chunk_size = luigi.IntParameter( default=10 )
    _timing_dict = None
    _input_remote_files = None

    #def ensure_delegation(self):
     #   if self.remote and not self.delegation_id:
      #      se = gridlib.se.get_se_instance( self.site )
       #     proxy = gridlib.ce.ProxyDelegator( se )
        #    self.delegation_id = proxy.get_delegation_id()
        #return self.delegation_id #Yannik commented it out (debug)

    @property
    def input_remote_files(self):
        if self._input_remote_files is None:
            myskims_tmp = MyskimsManager(self.myskims_path)
            self._input_remote_files = myskims_tmp.files( self.sample, site_order=( self.site, ) )
        return self._input_remote_files

    @property
    def timing_dict(self):
        if self._timing_dict is None:
            self._timing_dict = {}
            #if os.path.isfile("/net/scratch_cms3a/pook/MUSiC/MUSiCout/timing/timing_classification_tasks.json"):
            if os.path.isfile("/net/scratch_cms3a/vigilante/timing_classification_tasks.json"):
                #with open("/net/scratch_cms3a/pook/MUSiC/MUSiCout/timing/timing_classification_tasks.json","r") as jf:
                with open("/net/scratch_cms3a/vigilante/timing_classification_tasks.json","r") as jf:
                    self._timing_dict = json.load(jf)
            else:
                logger.info("timing dict not found at :" + self.timing_dict_path)
        return self._timing_dict

    def get_estimated_events_per_second(self, sample):
        if not self.sample in self.timing_dict:
            return 100.
        return self.timing_dict[sample]["mean"] + self.timing_dict[sample]["std"]

class SampleSetClassification( _SampleClassificationMixin ):
    ''' classify a set of samples as defined in myskims cfg '''

    def requires( self ):
        myskims_tmp = MyskimsManager(self.myskims_path)
        group_tasks = []
        for group in set([v for v in myskims_tmp.sample_groups.values()]):
            kwargs = get_shared_parameters(self, SampleGroupClassification)
            #kwargs['delegation_id'] = self.delegation_id #Yannik commented it out (debug)
            group_tasks.append( SampleGroupClassification(process_group = group,
                                                          **kwargs
                                                          ))
        return group_tasks

class _SampleGroupFieldMixin(object):
    # These parameters control where the gridjob is executed and stored
    # job type (mc, data, signal
    sample_type = luigi.Parameter( default="" )
    process_group = luigi.Parameter( default="" )

class _SampleGroupMixin( _SampleGroupFieldMixin ):

    def load_meta_info(self, myskims_obj):
        if not self.sample_type:
            self.sample_type = myskims_obj.get_group_type(self.process_group)

class SampleGroupClassification( _SampleClassificationMixin, _SampleGroupMixin ):
    ''' Classifiy all samples in one sample group within a myskims config '''

    def requires( self ):
        myskims_tmp = MyskimsManager(self.myskims_path)
        self.load_meta_info(myskims_tmp)
        sample_tasks = []
        kwargs = get_shared_parameters(self, SampleClassificationTask)
        #kwargs['delegation_id'] = self.delegation_id #Yannik commented it out (debug)
        for sample, group in myskims_tmp.sample_groups.items():
            if group != self.process_group:
                continue

            sample_tasks.append( SampleClassificationTask(
                                    sample=sample,
                                    **kwargs
                                ) )
        return sample_tasks

class SampleClassificationTask( _SampleClassificationMixin, _SampleGroupMixin ):
    ''' Classify a single sample either via remote or local'''
    sample = luigi.Parameter()

    @property
    def submission_dir( self ):
        args = []
        if self.sample_type:
            args.append( self.sample_type )
        if self.process_group:
            args.append( self.process_group )
        path = os.path.join( *args )
        return path

    def requires( self ):
        myskims_tmp = MyskimsManager(self.myskims_path)
        self.load_meta_info(myskims_tmp)
        args = shlex.split( myskims_tmp.args( self.sample ) )
        args.extend( shlex.split( self.extra_args ) )
        if self.forced_run_hash:
            run_hash = self.forced_run_hash
        else:
            run_hash = ecroot.calc_run_hash()
        args.extend(['--hash', run_hash])
        kwargs = get_shared_parameters(self, LocalClassificationTask)
        kwargs.update(dict(sample=self.sample,
            gridpack_files=myskims_tmp.settings["gridpackfiles"],
            gridpack_base=myskims_tmp.settings["gridpackbase"],
            executable=myskims_tmp.settings["executable"],
            submission_dir=self.submission_dir,
            job_args=args,
        ))

        tasks = []

        if self.remote:
            # make sure proxy delegation is set
            #self.ensure_delegation() Yannik commented it out (debug)

            # estimate the chunksize
            events_per_file = sum([int(n) for p,s,n in self.input_remote_files]) / len(self.input_remote_files)
            runtime_per_file = events_per_file / self.get_estimated_events_per_second(self.sample)
            chunk_size = self.mean_job_runtime / runtime_per_file
            #~ chunk_size *= self.mean_job_runtime
            chunk_size = max(1, int(chunk_size))
            logger.info("Sample name: %s " % (self.sample))
            logger.info("evt/file: %.1f | runtime/file: %.1f |etimated per evt: %.1f | chunksize: %d " % (events_per_file, runtime_per_file, self.get_estimated_events_per_second(self.sample), chunk_size))
            se = gridlib.se.get_se_instance( self.site )
            proxy = gridlib.ce.ProxyDelegator( se )
            kwargs_remote = get_shared_parameters(self, RemoteClassificationTask)
            #kwargs['delegation_id'] = self.delegation_id #Yannik commented it out (debug)
            kwargs.update(dict(
                task_name = self.sample,
                output_files=myskims_tmp.settings["outputfiles"],
                myskims_path=self.myskims_path,
                chunk_size=chunk_size,
                #delegation_id=self.delegation_id, #Yannik commented it out (debug)
            ))
            if "jobmaxruntime" in myskims_tmp.settings:
                kwargs["job_max_runtime"] = int(myskims_tmp.settings["jobmaxruntime"])
            kwargs_remote.update(kwargs)
            return RemoteClassificationTask(**kwargs_remote)

        else:
            if self.max_files is not None:
                files = files[ :self.max_files ]
            files = myskims_tmp.files( self.sample, site_order=( self.site, ) )

            for chunk in chunker( self.chunk_size, files ):
                task = LocalClassificationTask( chunk=chunk, **kwargs )
                tasks.append( task )

        logger.debug( "Sample '%s': %d .pxlio files to classify" % ( self.sample, len( tasks ) ) )

        return tasks

    def output( self ):
        return self.input()

class ClassificationFieldMixin(_SampleGroupFieldMixin):
    job_args = luigi.ListParameter( default=[] )
    submission_dir = luigi.Parameter("./", significant=False)

# abstract
class ClassificationMixin( _SampleGroupMixin, ClassificationFieldMixin ):
    ''' Generic Mixin for single sample Classification tasks '''
    sample = luigi.Parameter()
    _input_remote_files = None

    @property
    def input_remote_files(self):
        if self._input_remote_files is None:
            myskims_tmp = MyskimsManager(self.myskims_path)
            self._input_remote_files = myskims_tmp.files( self.sample, site_order=( self.site, ) )
        return self._input_remote_files

    def input_arguments( self ):
        args = []
        for site, path in self.chunk:
            args.append( GridTarget(path=path, site=site).path )
        return [ t.path for t in args ]

    @property
    def output_filename( self ):
        return "AnalysisOutput/EC_" + self.cleaned_taskname + ".root"

    @property
    def working_directory( self ):
        return os.path.join( self.submission_dir, self.sample )

    @property
    def cleaned_taskname(self):
        return re.sub("[e][x][t][1-9][_]", "", self.sample)

    @property
    def output_path( self ):
        return os.path.join( self.working_directory, "AnalysisOutput", self.output_filename )

class LocalClassificationTask( ClassificationMixin, ExternalProgramTask ):
    ''' Task for classification of a single sample on the local machine

        This task is not intended to be required directly.
        Use SampleClassificationTask without --remote option instead.
    '''
    gridpack_files = luigi.ListParameter( default=[] )
    gridpack_base = luigi.Parameter( default="." )
    chunk = luigi.ListParameter( default=[] )
    executable = luigi.Parameter( default="./EventClassFactory/ce_wrapper.sh" )

    def simulate_gridpack( self, folder ):
        folder = os.path.abspath( folder )

        logger.debug( "Simulating gridpack at '%s'..." % folder )

        with tmp_chdir( expand_path( self.gridpack_base ) ):
            for src in self.gridpack_files:
                src = expand_path( src )
                if os.path.isabs( src ):
                    dst = os.path.join( folder, os.path.basename( src ) )
                else:
                    dst = os.path.join( folder, src )
                self._copy( src, dst )

    def _copy( self, src, dst ):
        if os.path.exists( dst ):
            logger.debug( "'%s' already exists, skipping..." % os.path.abspath( dst ) )
            return

        logger.debug( "Copying: '%s' => '%s'" % ( os.path.abspath( src ), os.path.abspath( dst ) ) )

        parent = os.path.dirname( dst )
        if not os.path.exists( parent ):
            os.makedirs( parent )

        if os.path.isdir( src ):
            shutil.copytree( src, dst )
        elif os.path.isfile( src ):
            shutil.copy( src, dst )
        else:
            raise IOError( "File not found: '%s'" % src )

    def program_args( self ):
        args = [ "bash", self.executable ]
        args += list(self.job_args)
        args += self.input_arguments()
        return [ os.path.expandvars( arg ) for arg in args ]

    def input_arguments( self ):
        # on lx3acms1.physik.rwth-aachen.de (nickname "MUSiC machine"),
        # we use the direct dcap path instead of -external
        #if os.environ.get("HOSTNAME", "").lower().startswith("lx3acms1"): Yannik changed this
        if os.environ.get("HOSTNAME", "").lower().startswith("lx3acms2"):
            f = lambda i: i.site_path
        else:
            f = lambda i: i.external_path
        files = [ GridTarget( path=path, site=site ) for site, path in self.chunk ]
        return [ f(i) for i in files ]

    def run( self ):
        if not os.path.exists(self.working_directory):
            os.makedirs(self.working_directory)
        self.simulate_gridpack( self.working_directory )

        with tmp_chdir( self.working_directory ):
            super( LocalClassificationTask, self ).run()

    def output( self ):
        return luigi.LocalTarget( self.output_path )

class RemoteClassificationTask( ClassificationMixin, GridTask ):
    ''' Task for classification of a single sample on the local machine

        This task is not intended to be required directly.
        Use SampleClassificationTask with --remote option instead
    '''
    myskims_path = luigi.Parameter( default = expand_path("$MUSIC_CONFIG/MC/myskims_ram.yaml") )
    site = luigi.Parameter( default="T2_DE_RWTH" )
    chunk = luigi.ListParameter( default=[] )

    def input_arguments( self ):
        args = [ GridTarget( path=path, site=self.site ) for site, path, nevents in self.input_remote_files ]
        return [ i.site_path for i in args ]

    def run(self):

        if not os.path.exists( self.submission_dir ):
            os.makedirs( self.submission_dir )


        with tmp_chdir( self.submission_dir ):
            super( RemoteClassificationTask, self ).run()

            ec_found = False

            archive_filenames = []
            for root, dirnames, filenames in os.walk(self.working_directory):
                if root.startswith('bak'):
                    continue
                for output_file_name in self.output_files:
                    for filename in fnmatch.filter(filenames, output_file_name):
                        archive_filenames.append(os.path.join(root, filename))

            #~ if len(self.ce_task.jobs) > len(archive_filenames) and self.max_failed_fraction < 0.00001:
                #~ raise RuntimeError("Job arch missing for some jobs and no failed jobs allowed")
            for archive_filename in archive_filenames:
                if os.path.exists( archive_filename ):
                    arch_dir = os.path.dirname(archive_filename)
                    with tarfile.open( archive_filename, mode='r' ) as archive:
                        archive.extractall( arch_dir )
                    # Delete archive if we find a file
                    if glob.glob(os.path.join(arch_dir, "AnalysisOutput", "EC_*.root")):
                        os.remove(archive_filename)
                        ec_found = True
                    else:
                        logger.info("No EC for arch %s " % archive_filename)



    def output(self):
        if self.submission_dir in os.getcwd():
            path = self.task_name
        else:
            path = os.path.join(self.submission_dir, self.task_name)
        return GridTaskTarget(path,
                              max_failed=self.max_failed_fraction,
                              target_files=[self.output_filename])
    @property
    def working_directory( self ):
        return os.path.join( self.task_name )
