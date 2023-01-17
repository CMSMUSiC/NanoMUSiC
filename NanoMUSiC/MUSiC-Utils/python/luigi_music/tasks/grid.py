import logging
import os, os.path
import re
import glob
import shutil
import sys
import time
import warnings
import urlparse
import random
import hashlib
import luigi
from collections import Counter

import gridlib.se
import gridlib.ce

from shellhelpers import expand_path, tmp_chdir

from luigi_music.targets import GridTarget, GridTaskTarget


from luigi_music.util import chunker, job_dir, exit_code_from_info

from .util import get_shared_parameters

logging.getLogger("gridlib.ce").addHandler(logging.StreamHandler(stream=sys.stderr))
logging.getLogger("gridlib.se").addHandler(logging.StreamHandler(stream=sys.stderr))

logger = logging.getLogger( 'luigi-interface' )



class RemoteError( RuntimeError ):
    pass

class GridBaseFieldMixin(object):
    site = luigi.Parameter(default="T2_DE_RWTH")

class GridpackBaseMixin(object):
    gridpack_base_name = luigi.Parameter( default="gridpack.tar.bz2" )
    gridpack_files = luigi.ListParameter( default=[] )
    gridpack_base = luigi.Parameter( default="." )
    delete_gridpack = luigi.BoolParameter(default=False, significant=False)

class GridBaseMixin( luigi.Task ):

    def __init__( self, *args, **kwargs ):
        super( GridBaseMixin, self ).__init__( *args, **kwargs )

        self._storage_element = None
        self._gridpack_manager = None

    @property
    def gridpack_name(self):
        head, tail = os.path.split(self.gridpack_base_name)
        tail = tail.replace( ".tar.bz2", self.get_files_hash() + ".tar.bz2" )
        gridpack_remote_name = os.path.join( head, tail )
        return gridpack_remote_name

    @property
    def storage_element(self):
        if self._storage_element is None:
            self._storage_element = gridlib.se.StorageElement(self.site)
        return self._storage_element

    @storage_element.setter
    def storage_element(self, storage_element):
        self._storage_element = storage_element

    @property
    def gridpack_manager(self):
        if self._gridpack_manager is None:
            self._gridpack_manager = gridlib.se.GridpackManager( self.storage_element )
        return self._gridpack_manager

    @gridpack_manager.setter
    def gridpack_manager(self, gridpack_manager):
        self._gridpack_manager = gridpack_manager


    def get_files_hash( self ):
        ''' Emulate behavior of gridlib gridpack creation and create multi file hash'''
        exp_base = os.path.expandvars( self.gridpack_base )
        exp_files = []
        for item in self.gridpack_files:
            item = os.path.expandvars( item )
            if not os.path.isabs( item ):
                item = os.path.join( exp_base, item )
            if os.path.exists( item ):
                if os.path.isdir( item ):
                    for root, dirnames, filenames in os.walk( item ):
                        exp_files += [os.path.join(root, f) for f in filenames]
                else:
                    exp_files.append( item )
        #based on:
        # http://stackoverflow.com/questions/34807537/generating-one-md5-sha1-checksum-of-multiple-files-in-python?rq=1
        if not exp_files:
            return "0"
        hash_obj = hashlib.md5(open(exp_files[0], 'rb').read())
        for fname in exp_files[1:]:
            hash_obj.update(open(fname, 'rb').read())
        checksum = hash_obj.hexdigest()
        return checksum

    def get_remote_path( self ):
        head, tail = os.path.split(self.gridpack_name)
        tail = tail.replace( ".tar.bz2", self.get_files_hash() + ".tar.bz2" )
        gridpack_remote_name = os.path.join( head, tail )
        cern_username = os.getenv("CERNUSERNAME")
        if cern_username is None:
            cern_username = self.gridpack_manager.username_cern
        gridpack_remote_path = os.path.join( "/store/",
                                             "user",
                                             cern_username,
                                             "luigi_gridpacks",
                                             gridpack_remote_name,
                                             )
        return gridpack_remote_path

    def create_local_gridpack(self):
        # add files to gridpack
        # environment variables are also expanded within gridpack_prepare
        self.gridpack_manager.gridpack_create( base_directory=self.gridpack_base,
                                               files=self.gridpack_files,
                                               gridpack_local=self.gridpack_name )


class GridpackPreperationTask( GridBaseMixin, GridBaseFieldMixin, GridpackBaseMixin ):
    resources = {'grid_upload' : 1}
    def run( self ):
        logger.debug( "Uploading gridpack to " + self.get_remote_path() )
        self.create_local_gridpack()
        self.gridpack_manager.gridpack_upload( self.gridpack_name,
                                               self.get_remote_path() )
        #~ # delete gridpack if it was successfully transmited to the GRID
        if self.delete_gridpack and self.output().exists():
            os.remove(self.gridpack_name)

    def output( self ):
        return GridTarget( path=self.get_remote_path(), site=self.site )

class GridJobFieldMixin(GridBaseFieldMixin, GridpackBaseMixin):

    submission_dir = luigi.Parameter("./", significant=False)
    chunk_size = luigi.IntParameter( default=15 )
    update_interval = luigi.FloatParameter( default=120.0, significant=False )
    cmssw_version = luigi.Parameter( default=os.environ["CMSSW_VERSION"] )

    # Maximum runtime in minutes!!
    job_max_runtime = luigi.IntParameter( default=None, significant=False )
    #delegation_id = luigi.Parameter( default=None, significant=False ) #Yannik commented it out (debug)



class GridJobMixin(GridBaseMixin, GridJobFieldMixin, GridpackBaseMixin):
    task_name = luigi.Parameter()
    executable = luigi.Parameter()
    job_args = luigi.ListParameter( default=[] )
    output_files = luigi.ListParameter( default=[] )
    ce_task = None

    def __init__( self, *args, **kwargs ):
        super( GridJobMixin, self ).__init__( *args, **kwargs )
        self.task_dir = os.path.join( ".", self.task_name )

    #def create_delegation_id(self):
     #   se = gridlib.se.get_se_instance( self.site )
      #  proxy = gridlib.ce.ProxyDelegator( se )
       # self.delegation_id = proxy.get_delegation_id() #Yannik commented it (out)

    def get_gridpacks(self):
        ''' Add all gridpacks found from gridpack preparation dependencies

            if self input is:
                - a single item interpret it as a GridpackPreparationTask
                - a list interpret all items as GridpackPreparationTask outputs
                - a dict check "GridpackPreparations" for list of GridpackPreparationTask output
                  or "GridpackPreparation" for a single GridpackPreparationTask output
        '''
        gridpacks = []
        if type(self.input()) == list:
            for dt in self.input():
                gridpacks.append(dt.path)
            return gridpacks
        if type(self.input()) == dict:
            if "GridpackPreparations" in self.input():
                for dt in self.input()["GridpackPreparations"]:
                    gridpacks.append(dt.path)
            if "GridpackPreparation" in self.input():
                gridpacks.append(self.input()["GridpackPreparation"].path)
            return gridpacks
        gridpacks.append(self.input().path)
        return gridpacks

class GridTaskFieldMixin(GridJobFieldMixin):
    max_failed_fraction = luigi.FloatParameter(default=0.0, significant=False)

class GridTaskMixin(GridJobMixin, GridTaskFieldMixin):
    pass

class GridTaskSubmission(GridTaskMixin):
    resources = {'grid_submission' : 1}
    input_arguments = luigi.ListParameter( default=[] )
    priority = 250
    def requires(self):
        kwargs = get_shared_parameters(self, GridpackPreperationTask)
        return {"GridpackPreparation": GridpackPreperationTask(**kwargs)}

    def run(self):
        #if not self.delegation_id: #Yannik commented it out (debug)
        #    self.create_delegation_id()
        if not os.path.exists(self.submission_dir):
            os.makedirs( self.submission_dir )
        with tmp_chdir( self.submission_dir ):
            if self.job_max_runtime is not None:
                logger.debug( "Job max runtime set to %d minutes, will try to use short queues..." % self.job_max_runtime )

            self.ce_task = gridlib.ce.Task(
                name=self.task_name,
                cmssw_version=self.cmssw_version,
                storage_element=self.storage_element,
                job_max_runtime=self.job_max_runtime,
                #delegation_id=self.delegation_id, #Yannik commented it out (debug)
            )

            self.ce_task.executable_upload = False
            self.ce_task.executable = expand_path( self.executable )
            gridpacks = self.get_gridpacks()
            logger.info("Gridpacks found for this task " + ",".join(gridpacks))
            for gridpack in gridpacks:
                self.ce_task.add_gridpack( gridpack )
            # commented out because our input files are fetched differently
            # self.ce_task.inputfiles = [ os.path.basename(t.path) for t in self.input() ]
            self.ce_task.outputfiles = [ os.path.basename(p) for p in self.output_files ]

            for chunk in chunker( self.chunk_size, self.input_arguments ):
                job = self.ce_task.create_job()
                job.arguments = list(self.job_args + chunk)

            logger.info( "Submitting task '{}'.".format( self.ce_task.name ) )
            time.sleep(random.randint(0,10))
            self.ce_task.submit()

            if not self.ce_task.frontendstatus in ("SUBMITTED" ,"RUNNING"):
                raise RemoteError("Task submission failed.")

    def output(self):
        return luigi.LocalTarget( os.path.join(self.submission_dir, self.task_name,"task.pkl") )


class GridTask(GridTaskMixin):
    resources = {'grid_monitor' : 1}
    priority = 100
    running_status = ['REGISTERED',
                      'PENDING',
                      'IDLE',
                      'RUNNING',
                      'REALLY-RUNNING']
    check_resubmit_status = ['RETRIEVED', 'COMPLETED', 'RESUB']#Yannik change DONE in COMPLETED and removed DONE-OK
    check_stuck_running_status = ['RUNNING', 'REALLY-RUNNING']

    def __init__( self, *args, **kwargs ):
        super( GridTask, self ).__init__( *args, **kwargs )

        self._last_status = None
        self.running = True
        self.job_stats = Counter()
        self.running_watchdict = {}
        self.really_running_watchdict = {}
        self.resubmit_watchdict = {}

    def requires(self):
        kwargs = get_shared_parameters(self, GridTaskSubmission)
        kwargs["input_arguments"] = self.input_arguments()
        return {"GridTaskSubmission": GridTaskSubmission(**kwargs)}


    def input_arguments( self ):
        raise NotImplementedError

    @property
    def success(self):
        return self.output().exists()

    def get_status(self):
        """ Update status """
        try:
            self.ce_task.get_status()
        except TypeError as e:
            shutil.rmtree(self.task_name)
        n_running = len([True for job in self.ce_task.jobs if job.status in self.running_status])
        self.running = n_running > 0

        #~ logger.info("%d %.1f %.2f" % (n_running, 1. * n_running / len(self.ce_task.jobs), (1. - -self.max_failed_fraction)))
        new_stats = Counter(job.status for job in self.ce_task.jobs)
        if new_stats != self.job_stats:
            self.ce_task.retrieve_output(retry=2)
            self.job_stats = new_stats
            self.ce_task.save()
            self.update_status_info()

        if not self.running:
            self.ce_task.save()
            return

        job_resubmit_ids = []
        for i in range(len(self.ce_task.jobs)):
            if self.ce_task.jobs[i].status in self.check_stuck_running_status \
               and self.prevent_stuck_running(self.ce_task.jobs[i]):
                # "RUNNING" is a "special" state between IDLE and REALLY-RUNNING
                job_resubmit_ids.append(i)
            if self.ce_task.jobs[i].status in self.check_resubmit_status:
                if self.check_resubmit(self.ce_task.jobs[i]):
                    job_resubmit_ids.append(i)
        self.ce_task.save()

        # get status again after some waiting if we resubmitted jobs
        if job_resubmit_ids:
            self.resubmit_jobs(job_resubmit_ids)


    def resubmit_jobs(self, job_ids):
        msg = "Resubmitting jobs for %s: %s " % (self.task_name, ' '.join(["%s" % d for d in job_ids]))
        logger.info(msg)
        print " ************ grid.py ****** "
        print msg #Yannik add this line
        self.ce_task._unlock()
        self.set_status_message( msg )
        self.ce_task.resubmit(job_ids)
        self.ce_task.save()
        time.sleep(120)
        #self.ce_task = gridlib.ce.Task.load(self.task_name, delegation_id=self.delegation_id) #Yannik commented it out (debug)
        self.ce_task = gridlib.ce.Task.load(self.task_name)
        self.get_status()

    def update_status_info(self):
        msg = "Status change (%s): " % self.ce_task.frontendstatus
        stat_msg = "   ".join(["%s:%d" % (k,v) for k,v in self.job_stats.items()])
        msg += " " + self.task_name
        logger.info(msg + ":  "  + stat_msg)
        #~ try:
            #~ self.set_status_message( "|".join( [self.ce_task.frontendstatus, stat_msg] ) )
        #~ except:
            #~ logger.warning("unable to update status message in grid task")

    @property
    def any_job_successful(self):
        return any(exit_code_from_info( ce_job.infos ) == 0 for ce_job in self.ce_task.jobs)

    def check_resubmit(self, ce_job):
        print "I enter check_resubmit"
        exit_code = exit_code_from_info( ce_job.infos )
        if exit_code < 0:
            return False
        out_files = ""
        task_folder = os.path.join(self.submission_dir,self.task_name)
        #for output_file in self.output_files:
            #out_files += glob.glob(os.path.join(self.task_name,
             #                     job_dir(ce_job),
             #                     output_file))
        t1=False #Yannik starts to add from here
        t2=False
        out_files =os.path.join(self.task_name,job_dir(ce_job))
        list_grid=os.listdir(out_files)
        for j in list_grid:
            if "Out" in j:
                t1=True
                break
                
        
        #for x in out_files:
        #    if "Out" in x:
        #        t1=True
        #        break
        #for y in self.output_files:
        #    if "Out" in y:
        #        t2=True
        #        break 
        if t1: #or not self.any_job_successful:#DEFAULT
            print "Entro:   if t1 or not self.any_job_successful   "
            return False 
        
        if len(out_files) == len(self.output_files): #or not self.any_job_successful: #DEFAULT # Yannik commented   THIS IS THE DEFAULLT LINE! AT THE MOMENT IS  WORKING ONLY FOR THE SCAN
            print "Entro:   if len(out_files) == len(self.output_files) or not self.any_job_successful        "
            print self.output_files
            print out_files
            print "Restituisco False"
            return False
        #if ( t1 and t2 ) or not self.any_job_successful:
            #print "****** I will resub something. grid.py LINE 365 ********"#Yannik added this line
            #return False
        print "**** NOT CORRECT OUTPUT FILES ARE FOUND in grid.py. To resubmit. OUTPUT FILES FIND ARE: *****" #Yannik added this lines
        print(ce_job)
        
        print "*************************************************************"
        return True

    def prevent_stuck_running(self, ce_job):
        if ce_job.status == 'RUNNING': #Yannik changed from REALLY-RUNNING
            if self.job_max_runtime is None:
                return False
            if ce_job.runtime / 60. > self.job_max_runtime:
                return True
        else:
            if not ce_job.job_id in self.running_watchdict:
                self.running_watchdict[ce_job.job_id] = time.time()
            if (time.time() - self.running_watchdict[ce_job.job_id]) > 1200:
                del self.running_watchdict[ce_job.job_id]
                return True
            return False

    def run(self):
        pkl_path = os.path.join( self.task_name, "task.pkl" )
        if not os.path.exists(pkl_path):
            raise IOError("No task.pkl found for task %s" % self.task_name)
#        self.ce_task = gridlib.ce.Task.load(self.task_name, delegation_id=self.delegation_id) #Yannik commented it out (debug)
        self.ce_task = gridlib.ce.Task.load(self.task_name)
        if self.ce_task.name != self.task_name:
            raise ValueError("Cannot reattach to grid task (incompatible)")
        self.get_status()
        #if the job is not initally running first check if the task is complete
        # otherwise check if we should resubmit jobs which are retrieved without output
        if not self.running:
            job_resubmit_ids = []
            for i in range(len(self.ce_task.jobs)):
                if self.check_resubmit(self.ce_task.jobs[i]):
                    job_resubmit_ids.append(i)
            if job_resubmit_ids:
                print "****** I will resub something. grid.py LINE 400 ********"#Yannik added this line
                self.resubmit_jobs(job_resubmit_ids)
                self.get_status()
                self.ce_task.save()
                self.get_status()
        while self.running:
            time.sleep( self.update_interval )
            #~ logger.info("updating status %s" % " ".join(("%s: %d" % (k,c) for k,c in dict(Counter(job.status for job in self.ce_task.jobs)).items())) )
            self.get_status()
            self.ce_task.save()
        self.ce_task.retrieve_output(retry=2)
        self.get_status()
        self.ce_task.save()

    def output(self):
        return GridTaskTarget(os.path.join(self.submission_dir,self.task_name),
                              max_failed=self.max_failed_fraction,
                              target_files=self.output_files)

class GridJob( GridJobMixin):
    ''' Abstract target for single grid jobs '''

    existing = luigi.Parameter( default=None, significant=False )
    resubmit = luigi.BoolParameter( default=False, significant=False )

    cmssw_version = luigi.Parameter( default=os.environ["CMSSW_VERSION"] )

    # Maximum runtime in minutes!!
    job_max_runtime = luigi.IntParameter( default=None, significant=False )
    #delegation_id = luigi.Parameter( default=None, significant=False ) #Yannik commented it out (debug)

    executable = luigi.Parameter()

    state_change_time = None

    def input_arguments( self ):
        raise NotImplementedError

    def __init__( self, *args, **kwargs ):
        super( GridTask, self ).__init__( *args, **kwargs )

        self.task_name = "luigi_" + self.task_id
        self.task_dir = os.path.join( ".", self.task_name )

        self.ce_task = None

        self._last_status = None

    @property
    def ce_job( self ):
        return self.ce_task.jobs[0]

    @property
    def status( self ):
        if self.ce_task is None:
            return None

        status = self.ce_job.status

        # get_status() returns None if job is locked (file .lock exists)
        # in this case, just return the last known status
        if status is None:
            return self._last_status

        status = status.strip().upper()
        self._last_status = status

        return status

    @property
    def exit_code( self ):
        return exit_code_from_info( self.ce_job.infos )

    @property
    def running( self ):
        return self.status in ( "REGISTERED", "PENDING", "IDLE", "RUNNING", "REALLY-RUNNING" )

    @property
    def done( self ):
        return not self.running

    @property
    def success( self ):
        return ( self.status == "COMPLETED" ) and ( self.exit_code == 0 ) #Yannik changed DONE-OK

    @property
    def out_dir( self ):
        return os.path.join( self.task_dir, "out" )

    def run( self ):
        self.ce_task = None
        time.sleep(random.randint(0,10))
        # "Discover" existing pkl directory
        if os.path.exists( os.path.join( self.task_name, "task.pkl" ) ):
            logger.info( "Found directory '{}', will try to reattach.".format( self.task_name ) )
            self.existing = self.task_name

        # Reattach to existing task instance (loaded from Pickle file)
        if self.existing:
            logger.info( "Reattaching to '{}'...".format( self.existing ) )

            self.ce_task = gridlib.ce.Task.load( self.existing )
            if self.ce_task.name != self.task_name:
                raise ValueError( "Cannot reattach to grid task (incompatible)" )

            if self.resubmit:
                node_ids = [job.node_id for job in self.ce_task.jobs]
                time.sleep(random.randint(0,10))
                self.ce_task.resubmit( node_ids )

        # else: compose and submit a totally new job
        else:
            if self.job_max_runtime is not None:
                logger.debug( "Job max runtime set to %d minutes, will try to use short queues..." % self.job_max_runtime )

            self.ce_task = gridlib.ce.Task(
                name=self.task_name,
                cmssw_version=self.cmssw_version,
                storage_element=self.storage_element,
                job_max_runtime=self.job_max_runtime,
                #delegation_id=self.delegation_id,#Yannik commented it out (debug)
            )

            self.ce_task.executable_upload = False
            self.ce_task.executable = expand_path( self.executable )
            gridpacks = self.get_gridpacks()
            for gridpack in gridpacks:
                self.ce_task.add_gridpack( gridpack )
            # commented out because our input files are fetched differently
            #self.ce_task.inputfiles = [ os.path.basename(t.path) for t in self.input() ]
            self.ce_task.outputfiles = [ os.path.basename(p) for p in self.output_files ]

            job = self.ce_task.create_job()
            job.arguments = self.job_args

            logger.info( "Submitting task '{}'.".format( self.ce_task.name ) )
            time.sleep(random.randint(0,10))
            self.ce_task.submit()

            if self.ce_job.frontendstatus == 'FAILED': #Yannik changed from DONE-FAILED
                raise RemoteError("Task submission failed.")

        self.ce_task.get_status()
        self.state_change_time = time.time()
        # Main loop: watch task status and wait for completion
        logger.info( "Initial status '{}'.".format( self.status ) )

        if not self.running:
            logger.warn("Job is not running after submission, submission may have failed.")

        last_status = None
        while not self.done:
            time.sleep( self.update_interval )
            self.ce_task.get_status()
            if self.status == "RUNNING" and (self.state_change_time - time.time()) > 1200:
                raise RuntimeError("Task stuck running, failing")
            if last_status != self.status:
                self.state_change_time = time.time()
                last_status = self.status
                self.set_status_message( last_status )
                logger.info( "Task status changed to '{}'".format( last_status ) )
        # After completion, report, retrieve output and check for errors
        self.set_status_message( self.status )
        logger.info( "Task done, final status: {}, exit code: {}".format( self.status, self.exit_code ) )

        self.ce_task.retrieve_output()

        # move to fixed name
        job_dir = job_dir(self.ce_job)
        if os.path.isdir(job_dir):
            shutil.move(job_dir, self.out_dir)
        else:
            logger.warn( "Cannot find job directory at '%s'" % job_dir)

        if not self.success:
            raise RemoteError( "Task completed with status {} and exit code {}.".format( self.status, self.exit_code ) )


if __name__=="__main__":
    luigi.run( main_task_cls=Test )
