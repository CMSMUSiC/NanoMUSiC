import os
import json
import logging
import argparse

import luigi
from luigi.contrib.external_program import ExternalProgramTask

from luigi_music.tasks import SampleClassificationTask, ClassificationFieldMixin

from shellhelpers import expand_path

from ectools.register import ecroot
from roothelpers import root_setup
from .util import MyskimsManager, get_shared_parameters, _flatten, RemoteResultMixin
from .grid import GridTaskFieldMixin

logger = logging.getLogger('luigi-interface')


class MergeTask( ExternalProgramTask ):
    '''Generic merge task to run the actual merger script'''
    merger_binary = luigi.Parameter( default=expand_path( "$MUSIC_UTILS/scripts/ECMerger.py" ) )
    final = luigi.BoolParameter( default=False )
    keep = luigi.BoolParameter( default=False )
    debug_level = luigi.Parameter( default="INFO", significant=False )

    job_count = luigi.IntParameter( default=1, significant=False )
    num_files = luigi.IntParameter( default=200, significant=False )

    def program_args( self ):
        args = [ self.merger_binary,
            "-o", self.output().path,
            "--debug", self.debug_level,
            "--merge=all",
        ]

        if self.keep:
            args.append( "--keep" )
        if self.final:
            args.append( "--final" )
        if self.job_count > 1:
            args.extend( [ "-j", self.job_count ] )
        if self.num_files > 0:
            args.extend( [ "--num-files", self.num_files ] )

        args += self.input_files

        return args

    @property
    def input_files(self):
        raise NotImplementedError

class MergeMixin(ClassificationFieldMixin, GridTaskFieldMixin):
    myskims_path = luigi.Parameter( default = expand_path("$MUSIC_CONFIG/MC/myskims_ram.yaml") )
    myskims_path_data = luigi.Parameter( default = expand_path("$MUSIC_CONFIG/Data/myskims_ram.yaml") )
    timing_dict_path = luigi.Parameter( default = "", significant=False )
    #delegation_id = luigi.Parameter(default='', significant=False)#Yannik commented it out (debug)

class FullClassificationTask(luigi.Task, RemoteResultMixin, MergeMixin):
    def requires(self):
        tasks = {}

        kwargs = get_shared_parameters(self, SampleSetMergeTask)
        kwargs["campaign_id"] = self.campaign_id
        #kwargs["delegation_id"] = self.delegation_id#Yannik commented it out (debug)
        kwargs["remote_dir"] = self.remote_dir
        tasks["BGMergeTask"] = CopyMergedToRemoteTask(**kwargs)
        kwargs_data = kwargs.copy()
        kwargs_data['myskims_path'] = self.myskims_path_data
        tasks["DataMergeTask"] = CopyMergedToRemoteTask(**kwargs_data)
        return tasks

    def output( self ):
        return self.input()

class MCClassificationTask(luigi.Task, RemoteResultMixin, MergeMixin):
    def requires(self):
        tasks = {}

        kwargs = get_shared_parameters(self, SampleSetMergeTask)
        kwargs["campaign_id"] = self.campaign_id
        #kwargs["delegation_id"] = self.delegation_id#Yannik commented it out (debug)
        kwargs["remote_dir"] = self.remote_dir
        tasks["BGMergeTask"] = CopyMergedToRemoteTask(**kwargs)
        return tasks

    def output( self ):
        return self.input()

class CopyMergedToRemoteTask(luigi.Task, RemoteResultMixin, MergeMixin):

    @property
    def bg_local_path(self):
        if 'BGMergeTask' in self.input():
            return self.input()['BGMergeTask'].path
        return None

    @property
    def data_local_path(self):
        if 'DataMergeTask' in self.input():
            return self.input()['DataMergeTask'].path
        return None

    def requires(self):
        kwargs = get_shared_parameters(self, SampleSetMergeTask)
        #kwargs['delegation_id'] = self.delegation_id#Yannik commented it out (debug)
        return SampleSetMergeTask(**kwargs)

    def run(self):
        if not os.path.exists(self.remote_path):
            luigi.local_target.LocalFileSystem().mkdir(self.remote_path)

        if self.bg_local_path:
            remote_location = os.path.join(self.remote_path,os.path.basename(self.bg_local_path))
            if not os.path.exists(remote_location):
                logger.info("Copy BG result file to remote")
                luigi.local_target.LocalFileSystem().copy(self.bg_local_path, remote_location)
        if self.data_local_path:
            remote_location = os.path.join(self.remote_path, os.path.basename(self.data_local_path))
            if not os.path.exists(remote_location):
                logger.info("Copy Data result file to remote")
                luigi.local_target.LocalFileSystem().copy(self.data_local_path, remote_location)

    def output(self):
        if self.bg_local_path:
            return luigi.LocalTarget(path=os.path.join(self.remote_path,os.path.basename(self.bg_local_path)))
        if self.data_local_path:
            return luigi.LocalTarget(path=os.path.join(self.remote_path,os.path.basename(self.data_local_path)))


class SampleSetMergeTask( luigi.Task, MergeMixin ):
    ''' Merge task for all tasks in a sample set (myskims) '''
    priority = 200
    def requires( self ):
        myskims_tmp = MyskimsManager(self.myskims_path)
        tasks = {}
        if myskims_tmp.get_mc_groups():
            kwargs = get_shared_parameters(self, BGMergeTask)
            #kwargs['delegation_id'] = self.delegation_id#Yannik commented it out (debug)
            tasks['BGMergeTask'] = BGMergeTask(myskims_path=self.myskims_path)
        if myskims_tmp.get_data_groups():
            kwargs = get_shared_parameters(self, DataMergeTask)
            kwargs['process_group'] = 'data'
            #kwargs['delegation_id'] = self.delegation_id #Yannik commented it out (debug)
            tasks['DataMergeTask'] = DataMergeTask(**kwargs)
        return tasks

    def output( self ):
        return self.input()

class SampleGroupMergeTask( MergeTask, MergeMixin ):
    ''' Merge task for all process groups within one process group in mysikms file'''
    process_group = luigi.Parameter()
    final = True

    @property
    def input_files(self):
        def choose_input(target):
            if hasattr(target, "files"):
                return target.files
            else:
                return target.path
        return luigi.task.flatten( [ choose_input(taskTarget) for taskTarget in luigi.task.flatten( self.input() ) ] )

    def requires( self ):
        myskims_tmp = MyskimsManager(self.myskims_path)
        kwargs = get_shared_parameters(self, SampleClassificationTask)
        tasks = []
        for s in myskims_tmp.samples( group=self.process_group ):
            kwargs["sample"] = s
            #kwargs['delegation_id'] = self.delegation_id #Yannik commented it out (debug)
            tasks.append(SampleClassificationTask(**kwargs)) #
        return tasks

    def output( self ):
        myskims_tmp = MyskimsManager(self.myskims_path)
        grouptype = myskims_tmp.get_group_type(self.process_group) +"_merged"
        if not os.path.exists(grouptype):
            os.mkdir(grouptype)
        filename = os.path.join(grouptype, self.process_group + ".root")
        return luigi.LocalTarget( filename )


class AllSampleGroupsMergeTask( luigi.WrapperTask, MergeMixin ):

    def requires( self ):
        kwargs = get_shared_parameters(self, SampleGroupMergeTask)
        myskims_tmp = MyskimsManager(self.myskims_path)
        tasks = []
        for g in myskims_tmp.get_mc_groups():
            kwargs["process_group"] = g
            #kwargs['delegation_id'] = self.delegation_id # Yannik commented it out (debug)
            tasks.append(SampleGroupMergeTask( **kwargs))
        return tasks

    def output( self ):
        return self.input()

class BGMergeTask( MergeTask, MergeMixin ):
    '''' Merge all outputs from SampleGroupMergeTask into one total output file'''
    out = luigi.Parameter( default="bg.root" )
    final = False
    num_files = 0

    def requires( self ):
        kwargs = get_shared_parameters(self, AllSampleGroupsMergeTask)
        #kwargs['delegation_id'] = self.delegation_id #Yannik commented it out (debug)
        return AllSampleGroupsMergeTask( **kwargs )

    @property
    def input_files(self):
        return luigi.task.flatten( [ localTarget.path for localTarget in luigi.task.flatten( self.input() ) ] )

    def output( self ):
        return luigi.LocalTarget( self.out )


class SignalMergeTask( SampleGroupMergeTask ):
    pass

class SignalStackTask( MergeTask ):
    signal_process_group = luigi.Parameter()
    final = False
    num_files = 0

    def requires( self ):
        return ( BGMergeTask(), SignalMergeTask( process_group=self.signal_process_group ) )

    def output( self ):
        filename = self.signal_process_group + "_stacked.root"
        return luigi.LocalTarget( filename )

class DataMergeTask( SampleGroupMergeTask ):
    def output(self):
        return luigi.LocalTarget( path="data.root" )

if __name__=="__main__":
    luigi.run( main_task_cls=BGMergeTask )
