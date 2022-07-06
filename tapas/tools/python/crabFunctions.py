#!/usr/bin/env python
## @package crabFunctions
# This module provides common functions for tasks with crab3
#
# This module provides common functions for tasks with crab3.
# You need no create a CrabController object in order to use the functions
import os,sys,glob
import re
import tarfile
import xml.etree.ElementTree as ET
import imp
import shutil
import json
import optparse
import subprocess
import logging
import datetime
import uuid
import time
from  httplib import HTTPException
from multiprocessing import Process, Queue
from CRABAPI.RawCommand import crabCommand
from CRABClient.UserUtilities import getConsoleLogLevel, setConsoleLogLevel
from CRABClient.ClientUtilities import LOGLEVEL_MUTE
from CRABClient.ClientExceptions import CachefileNotFoundException
setConsoleLogLevel(LOGLEVEL_MUTE)



import gridlib.se
import gridlib.gfal2context
import dbutilscms
import aachen3adb

LOG_CHOICES = [ 'ERROR', 'WARNING', 'INFO', 'DEBUG', 'CRAB' ]

generators = {
    'AG': 'alpgen',
    'AM': 'amcatnlo',
    'BM': 'blackmax',
    'CA': 'calchep',
    'CO': 'comphep',
    'HP': 'herwigpp',
    'HW': 'herwig',
    'JG': 'JHUGen',
    'MC': 'mcatnlo',
    'MG': 'madgraph',
    'P6': 'pythia6',
    'P8': 'pythia8',
    'PH': 'powheg',
    'SP': 'sherpa',
}

## The CrabController class
#
# This class can be used to manage Analyses using crab3

class CrabController():

    ## The constructor.
    # @type self: CrabController
    # @param self: The object pointer.
    # @type self: A logging logger instance
    # @param self: A previously defined logger. Crab log messages will use this logger as their parent logger.
    def __init__(self,
                 debug=0,
                 logger = None,
                 workingArea = None,
                 username = None,
                 proxy=None):
        self.debug = debug
        if proxy:
            self.proxy = proxy
        else:
            self.proxy = gridlib.util.VomsProxy()
        if workingArea is not None:
            self.workingArea = workingArea
        else:
            self.workingArea = os.getcwd()
        self.dry_run = False
        if username is not None:
            self.username = username
        else:
            self.username = None

        if logger is not None:
            self.logger = logger.getChild("CrabController")
        else:
            # add instance logger as logger to root
            self.logger = logging.getLogger("CrabController")
            # check if handlers are present for root logger
            # we assume that the default logging is not configured
            # if handler is present
            if len(logging.getLogger().handlers) < 1 :
                ch = logging.FileHandler('crabController.log',
                                          mode='a',
                                          encoding=None,
                                          delay=False)
                ch.setLevel(logging.DEBUG)
                # create formatter
                myformat = "%(asctime)s - "
                myformat += "%(name)s - "
                myformat += "%(levelname)s -"
                myformat += "%(message)s'"
                formatter = logging.Formatter( myformat )
                # add formatter to ch
                ch.setFormatter(formatter)
                self.logger.addHandler(ch)
        self.crab_q = Queue()

    ## Check if crab can write to specified site
    #
    # @type self: CrabController
    # @param self: The object pointer.
    # @type site string
    # @param site The Site symbol [default:T2_DE_RWTH]
    # @type path string
    # @param path lfn path to check write permission in. see twiki WorkBookCRAB3Tutorial
    # @return boolean which is True if user can write to site and False otherwise
    def checkwrite(self,site='T2_DE_RWTH',path='noPath'):
        if self.username is None:
            self.username = self.checkusername()
        cmd = ['checkwrite', '--site', site]
        self._add_proxy_options(cmd)
        if self.proxy.voGroup:
            cmd += ["--voGroup", self.proxy.voGroup]
        if self.proxy.voRole:
            cmd += ["--voRole", self.proxy.voRole]
        if not 'noPath' in path:
            cmd += [ '--lfn', path]
        try:
            self.logger.info( "Checking if user can write to " \
            " /store/user/%s on site %s with voGroup %s"%(self.username,
                                                          site,
                                                          self.proxy.voGroup) )
            res = self.callCrabCommand(cmd)
            if res['status'] == 'SUCCESS':
                self.logger.info("Checkwrite was sucessfully called.")
                return True
            else:
                errmsg = "The crab checkwrite command failed for site: %s" % site
                self.logger.error( errmsg )
                self.logger.debug(res)
                return False
        except:
            self.logger.error( 'Unable to perform crab checkwrite')
            self.logger.debug(cmd)
            return False

    ## Check if crab can write to specified site
    #
    # @param self: The object pointer.
    # @type name string
    # @param name The crab3 config file name or task name
    def submit(self, name):
        cmd = ['submit']
        if self.dry_run:
            cmd += ['--dryrun']
            self.logger.info('Dry-run: You may check the created config and sandbox')
        else:
            cmd += ['--wait']
        self._add_proxy_options(cmd)
        if not name.startswith('crab_'):
            name = 'crab_' + name
        if not name.endswith('_cfg.py'):
            name += '_cfg.py'
        cmd += [name]
        self.logger.debug("Calling submit with command: %s" % " ".join(cmd))
        res = self.callCrabCommand(cmd)
        self.logger.info("crab sumbit called for task %s" % name)
        return res

    ## Resubmit all failed tasks in job or specified list of jobs in task
    #
    # @type self: CrabController
    # @param self: The object pointer.
    # @type name string
    # @param name The crab3 request name, a.k.a the sample name
    # @type joblist list of strings
    # @param joblist The crab3 request name, a.k.a the sample name
    def resubmit(self,name,joblist = None):
        if self.dry_run:
            self.logger.info('Dry-run: Created config file. ')
            return {}
        if False:
            pass
        else:
            cmd = ['resubmit',
                   '--wait',
                   os.path.join(self.workingArea,
                                self._prepareFoldername(name))
                    ]
        self._add_proxy_options(cmd)
        res = self.callCrabCommand( cmd )
        self.logger.info("crab resumbit called for task %s"%name)
        return res

    ## Returns the hn name for a user with valid proxy
    #
    # @type self: CrabController
    # @param self: The object pointer.
    # @returns users hypernews name
    def checkusername(self):
        self.logger.info("Checking username")

        try:
            username = os.environ["CERNUSERNAME"]
            return username
        except:
            pass
        cmd = ['checkusername']
        self._add_proxy_options(cmd)
        self.logger.debug("With command %s" % ' '.join(cmd))
        res = crabCommand(cmd)
        try:
            self.username = res['username']
            return res['username']
        except:
            return "noHNname"

    ## Check crab status
    #
    # @type self: CrabController
    # @param self: The object pointer.
    # @type name string
    # @param name The crab3 request name, a.k.a the sample name
    def status(self,name):
        if self.dry_run:
            self.logger.info("Dry-run: Created config file."\
                "crab command would have been: %s"%cmd)
        else:
            cmd = ['status', '--long']
            self._add_proxy_options(cmd)
            crabfolder = os.path.join(self.workingArea,
                          self._prepareFoldername(name))
            cmd += [crabfolder]
            try:
                self.logger.debug("Getting status with command: %s" % ' '.join(cmd))
                res = self.callCrabCommand(cmd)
                if 'taskFailureMsg' in res and 'jobs' in res:
                    return (res['status'],
                            res['jobs'],
                            res['taskFailureMsg'])
                elif 'jobs' in res and 'taskFailureMsg' not in res:
                    return res['status'], res['jobs'],None
                elif 'jobs' not in res and 'taskFailureMsg' in res:
                    return res['status'], {},res['taskFailureMsg']
                else:
                     return res['status'],{},None
            except Exception as e:
                print e
                self.logger.error("Can not run crab status request")
                return "NOSTATE",{},None

    ## Call crab command in a new process and return result dict
    #
    # @param self The object pointer
    # @param crabArgs A list of arguments for crab beginning with the command
    def callCrabCommand( self, crabArgs ):
        crabCommandProcessArgs = (self.crab_q, crabArgs)
        p = Process(target=crabCommandProcess,
                    args=(crabCommandProcessArgs))
        p.start()
        res = self.crab_q.get()
        p.join()
        return res

    ## Call crab getlog
    #
    # @param self: The object pointer.
    # @param name Nae of the crab task
    # @param A comma seperated list of jobids and ranges as a string
    def getlog(self, name, jobid_str = "", checksum=False):
        foldername = self._prepareFoldername( name)
        cmd = ['getlog', '%s' % foldername ]
        cmd += ["--dump"]
        if jobid_str:
                cmd += [ '--jobids', jobid_str ]
        if checksum:
            chk_str = "yes"
        else:
            chk_str = "no"
        cmd += ['--checksum', chk_str]
        cmd += ['--wait', "1500"]
        self._add_proxy_options(cmd)
        try:
            res = self.callCrabCommand( cmd )
            return res
        except:
            self.logger.error("Error calling crab getlog for %s" %foldername)
            return {}

    ## Call crab report command and return path to lumiSummary
    #
    # @param self The object pointer
    # @param name The crab3 request name, a.k.a the sample name
    def report(self, name):
        foldername = self._prepareFoldername( name)
        cmd = ['report']
        self._add_proxy_options(cmd)
        cmd += [foldername]
        self.logger.debug("Calling crab report with command: %s " % ' '.join(cmd))
        try:
            return self.callCrabCommand(cmd)
        except:
            self.logger.error("Error calling crab report for %s" % foldername)
            return {}


    ## Read a crab config and return python object
    #
    # @param self: The object pointer.
    # @param name The sample name (crab request name)
    def readCrabConfig( self, name ):
        try:
            pset = 'crab_%s_cfg.py' % name
            with open( pset, 'r') as cfgfile:
                cfo = imp.load_source("pycfg", pset, cfgfile )
                config = cfo.config
                del cfo
            return config
        except:
            return None

    ## Return list of all crab folders in workin area (default cwd)
    #
    # @param self The object pointer
    #
    @property
    def crabFolders(self):
        results = []
        dirlist = [x for x in os.listdir(self.workingArea)
                    if (x.startswith('crab_')
                    and os.path.isdir(os.path.join(self.workingArea,x)))]
        return dirlist

    ## Add crab_ to Foldername if needed
    #
    # @param self the object pointer
    # @param name name of folder or crab task
    def _prepareFoldername(self, name):
        if name.startswith("crab_"):
            crabfolder = '%s'%name
        else:
            crabfolder = "crab_%s "%name
        return crabfolder.strip()

    ## Add standard proxy options to crab command
    #
    # @param self the object pointer
    # @param cmd the crab command as a list
    def _add_proxy_options(self, cmd):
        #~ pass
        cmd += ["--proxy", self.proxy.path]
    ## Populates an existing optparse parser or returns a new one with options for crab functions
    #
    # This functions populates a previously created (or new) instance of a
    # optparse parser object with options needed by crab functions.
    # It is possible to add three kinds of options:
    # - options where a error should be raised if the option was previously defined
    # - options where previous definitions should be kept
    # - options where previous definitions should be overriden
    # @type Optparse parser instance
    # @param parser A previously created parser oject which should be extenden [default: new instance]
    # @return A new or extenden optparse parser instance
    def commandlineOptions(self, parser):
        # we first need to call parse_args with a dummy string at the beginning to
        # check for existing options later
        args = parser.parse_args("")

        # The following block shows how variables should be added, where
        # conflicts are possible if the option is overridden by this function
        # they raise a value error
        #try:
        #    parser.add_option( '--someOption',metavar='DIR',default=None,
        #                       help='Dummy option for future integration')
        #except OptionConflictError as e:
        #    conditionalLog(crablog,"There are conflicts extending the optparse options object",'error')
        #    conditionalLog(crablog,e.strerror,'error')

        # options where it is checked if they exists and new options are added
        # otherwise
        if not hasattr(args, 'dry_run'):
            parser.add_argument( '--dry-run', action='store_true', default=False,
                help='Do everything except calling CRAB or registering'\
                     ' samples to the database.' )
        if not hasattr(args, 'workingArea'):
            parser.add_argument( '--workingArea',
                metavar='DIR',
                default=os.getcwd(),
                help='The area (full or relative path) where to create'\
                     'the CRAB project directory. If the area doesn\'t '\
                     'exist, CRAB will try to create it using the mkdir'\
                     'command (without -p option).' \
                     'Defaults to thecurrent working directory.' )


        # Some options can be added without expected trouble with other parser
        # parts, simply because it is quite fixed what they represent.
        # those variables should be added here and will throw no exception if
        # they already exist in the parser
        #parser.set_conflict_handler('resolve')
        #parser.add_option( '--someOption',metavar='DIR',default=None,
        #                       help='Dummy option for future integration')

        return parser


## Function to run crab command in a new process
#
# Some CRAB commands (e.g. submit) create broken cmssw process objects
# when they are created in multiple calls of crabCommand via CRAB API
# Running them in a new process is a workaround, see
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3FAQ#Multiple_submission_fails_with_a
def crabCommandProcess(q,crabCommandArgs):
    # give crab3 the chance for one server glitch
    i=0
    while True:
        i+=1
        try:
            res = crabCommand(*crabCommandArgs)
            break
        except HTTPException as e:
            # crab error will try again
            import time
            time.sleep(5)
        except CachefileNotFoundException as e:
            # crab error CachefileNotFoundException will try again
            #print e
            #print "end error ---------------"
            #print crabCommandArgs
            res={ 'status':"CachefileNotFoundError",'jobs':{}}
            break
        except:
            pass
        if crabCommandArgs[0] == "submit" and os.path.exists(crabCommandArgs[-1]):
            shutil.rmtree( crabCommandArgs[-1] )
        if i>5:
            res={ 'status':"CrabError",'jobs':{}}
            break
    q.put( res )


# Class to handle logging in a picklable way
class LoggerMixin():
    @property
    def log(self):
        component = "{}.{}".format(type(self).__module__,
                                   type(self).__name__)
        return logging.getLogger(component)


## Class for a single CrabRequest
#
# This class represents one crab3 task/request
class CrabTask( LoggerMixin ):

    ## The object constructor
    #
    # @type self: CrabTask
    # @param self: The object pointer.
    # @type taskname: String
    # @param taskname: The object pointer.
    # @type initUpdate: Boolean
    # @param initUpdate: Flag if crab status should be called when an instance is created
    def __init__(self, taskname ,
                       crabController = None ,
                       initUpdate = True,
                       debuglevel = "ERROR",
                       dblink = None,
                       localDir = "",
                       outlfn = "" ,
                       globalTag = None,
                       skimmer_version = None,
                       json_file = None,
                       datasetpath=None,
                       proxy=None,
                       max_crab_chunk = 101,
                       site="T2_DE_RWTH"):
        self.name = taskname
        self.uuid = uuid.uuid4()
        self.jobs = {}
        self.localDir = localDir
        self.outlfn = outlfn
        self.isUpdating = False
        self.taskId = -1
        self.max_crab_chunk = max_crab_chunk
        #variables for statistics
        self.nJobs = 0
        self.state = "NOSTATE"
        self.maxjobnumber = 0
        self.nUnsubmitted   = 0
        self.nIdle = 0
        self.nRunning = 0
        self.nTransferring    = 0
        self.nCooloff    = 0
        self.nFailed    = 0
        self.nFinished    = 0
        self.nComplete    = 0
        self.failureReason = None
        self.lastUpdate = datetime.datetime.now().strftime( "%Y-%m-%d_%H.%M.%S" )

        self._isData = None
        self.dblink = dblink
        self.resubmitCount = 0
        self.site = site
        self._se = None
        self.debug = False

        self.finalFiles = []
        self.totalEvents = 0
        # crab config as a python object should only be used via .config
        self._crabConfig = None
        # dict with lummi summary info
        self.report = {}
        self._inDB = None
        self.dbSkim = aachen3adb.DataSkim(dbcon=dblink) if self.isData else aachen3adb.MCSkim(dbcon=dblink)
        self.dbSample = aachen3adb.DataSample(dbcon=dblink) if self.isData else aachen3adb.MCSample(dbcon=dblink)
        self._crabFolder = None

        self._globalTag_default = globalTag
        self._skimmer_version_default = skimmer_version
        self._json_file_default = json_file
        self._datasetpath_default = datasetpath
        self._timestamp = None
        self._pickle_prepared = False
        if proxy:
            self.proxy = proxy
        else:
            self.proxy = gridlib.util.VomsProxy()
        #start with first updates
        if initUpdate:
            self.update()
            self.updateFromDB()
            self.updateJobStats()


    ## Property function to find out if task runs on data
    #
    # @param self: CrabTask The object pointer.
    @property
    def isData( self ):
        if self._isData is None:
            try:
                test = self.crabConfig.Data.lumiMask
                self._isData = True
            except:
                if self.name.startswith( "Data_" ):
                    self._isData = True
                else:
                    self._isData = False
        return self._isData


    @property
    def se ( self ):
        if not self._se:
            self._se = gridlib.se.StorageElement( self.site )
        return self._se
    ## Function to access crab config object or read it if unititalized
    #
    # @param self: CrabTask The object pointer.
    @property
    def crabConfig( self ):
        if self._crabConfig is None:
            crab = CrabController(proxy=self.proxy)
            self._crabConfig = crab.readCrabConfig( self.name )
        return self._crabConfig

    @property
    def globalTag( self ):
        try:
            return self.crabConfig.Data.outputDatasetTag
        except:
            pass
        try:
            return  self.dbSkim.globaltag
        except:
            pass
        return self._globalTag_default

    @property
    def skimmer_version( self ):
        try:
            outlfn = self.crabConfig.Data.outLFNDirBase.split('/store/user/')[1]
            return outlfn.split("/")[2]
        except:
            pass
        try:
            return self.dbSkim.version
        except:
            pass
        return self._skimmer_version_default

    @property
    def datasetpath( self ):
        try:
            return self.crabConfig.Data.inputDataset
        except:
            pass
        try:
            return self.dbSkim.datasetpath
        except:
            pass
        return self._datasetpath_default

    @property
    def json_file( self ):
        if not self.isData: return None
        try:
            return self.crabConfig.Data.lumiMask.split("/")[-1]
        except:
            pass
        try:
            return self.dbSkim.run_json
        except:
            pass
        return self._json_file_default

    @property
    def inDB( self ):
        if self.state == "FINAL":
            self._inDB = True
        if self.dbSkim.id:
            return True

        if self.updateFromDB():
            try:
                if self.dbSkim.id:
                    self._inDB = True
            except:
                self._inDB = False
        else:
            self._inDB = False
        return self._inDB

    @property
    def isFinal( self ):
        if self.state == "FINAL":
            return True
        if not self.inDB:
            return False
        else:
            return self.dbSkim.finished is not None

    @property
    def crabFolder( self ):
        if not self._crabFolder is None:
            return self._crabFolder
        crab = CrabController(proxy=self.proxy)
        if os.path.exists(os.path.join(self.crabConfig.General.workArea,
                                       crab._prepareFoldername(self.name))):
            self._crabFolder = os.path.join(self.crabConfig.General.workArea,
                                            crab._prepareFoldername(self.name))
            return self._crabFolder
        alternative_path = os.path.join(os.getcwd(),
                                        crab._prepareFoldername( self.name ) )
        if os.path.exists( alternative_path ):
            self._crabFolder = alternative_path
            return self._crabFolder
        self.log.debug( "No crab folder for task")
        return ""

    @property
    def isCreated( self ):
        if not self.crabConfig:
            return False
        if not self.crabFolder:
            return False
        return True

    @property
    def remotePath(self):
        return os.path.join(self.crabConfig.Data.outLFNDirBase,
                            self.datasetpath.split('/')[1],
                            self.globalTag)

    @property
    def logPaths(self):
        # Find logs
        return glob.glob( os.path.join( self.crabFolder,
                                            "results",
                                            "cmsRun_*.log.tar.gz"))

    def _logPath2jobNumber(self, logPath):
        try:
            jid = int(logPath.split("/")[-1].split("_")[1].split(".")[0])
            return jid
        except ValueError:
            self.log.warning("unable to parse jID for logPath %s" % logPath)
        return -1

    # Property for a list of finished job ids in task
    @property
    def finished_job_ids(self):
        return [ int(k) for k,v in self.jobs.items() if v["State"] == "finished" ]

    @property
    def transfered_log_ids(self):
        return [self._logPath2jobNumber(p) for p in self.logPaths]

    @property
    def finalized_job_ids(self):
        return [ int(f.path.split(".pxlio")[0].split("_")[-1]) for f in self.dbSkim.files ]

    @property
    def timestamp(self):
        if not self._timestamp:
            crab = CrabController(proxy=self.proxy)
            res = crab.callCrabCommand(['getoutput',
                                        '--dump',
                                        self.crabFolder])
            try:
                self._timestamp = res['lfn'][0].split('/')[-3]
            except:
                self.log.error('Unable to parse info from "getoutput --dump"')
                self.log.error(res)
                raise('Timestamp not defined for task see error log message')
        return self._timestamp

    ## Function to resubmit failed jobs in tasks
    #
    # @param self: CrabTask The object pointer.
    def resubmit_failed( self ):
        failedJobIds = []
        controller =  CrabController(proxy=self.proxy)
        for jobkey in self.jobs.keys():
            job = self.jobs[jobkey]
            if job['State'] == 'failed':
                failedJobIds.append( job['JobIds'][-1] )
        controller.resubmit( self.name, joblist = failedJobIds )
        self.lastUpdate = datetime.datetime.now().strftime( "%Y-%m-%d_%H.%M.%S" )

    ## Function to update Task in associated Jobs
    #
    # @param self: CrabTask The object pointer.
    def update(self, updateDB = True):
        #~ self.lock.acquire()
        self.log.debug( "Start update for task %s" % self.name )
        self.isUpdating = True
        controller =  CrabController(proxy=self.proxy)
        self.state = "UPDATING"
        # check if we should drop this sample due to missing info
        if ((self.globalTag is None)
            or (self.skimmer_version is None)
            or (self.isData and self.json_file is None)):
            self.state = "DROP"
            self.log.debug( "Dropping: One of the following is missing:"\
                "globalTag, skimmer_version or json_file in case of data")
            self.log.debug( "GlobalTag: %s skimmer_version: %s  json_file: %s"%(
                (self.globalTag is None),
                (self.skimmer_version is None),
                (self.isData and self.json_file is None)))
        if self.isFinal: # Final tasks can be skipped for crab status updates
            self.state = "FINAL"
            self.log.debug( "Already final" )
            if self.inDB:
                self.nJobs = len(self.dbSkim.files)
                self.nFinished = len (self.dbSkim.files)
        elif self.isCreated: # Update samples if task is created yet
            self.log.debug( "Try to get status for task" )
            # cache number of old files to see if more outputs are ready
            finished_before = self.nFinished
            # update task status from crab
            self.state , self.jobs,self.failureReason = controller.status(self.name)
            self.log.debug( "Found state: %s" % self.state )
            # handle failed tasks
            if self.state=="FAILED":
                #try it once more
                time.sleep(2)
                self.state , self.jobs,self.failureReason = controller.status(self.name)
            #update the job statistics from crab status result
            self.nJobs = len(self.jobs.keys())
            self.updateJobStats()
            #start fetching logs if more jobs are finished which are not finalized yet
            if updateDB and finished_before < self.nFinished and \
                len(set(self.finished_job_ids) - set(self.finalized_job_ids)):
                self.log.debug("Found some new finished jobs getting and updating files")
                self.transferLogs()
                self.findFinishedFiles()
                self.updateDB()

            if (updateDB
                and not self.inDB
                and self.state in ( "QUEUED",
                                    "SUBMITTED",
                                    "COMPLETED",
                                    "FINISHED",
                                    "COMPLETED")):
                self.log.debug( "Trying to inject/update entries to DB" )
                self.updateDB()
            if self.state == "NOSTATE" and self.crabFolder and self.inDB:
                self.log.debug( "Trying to resubmit because of NOSTATE" )
                if self.resubmitCount < 3: self.handleNoState()
        # Final solution inf state not yet found
        if self.state == "UPDATING":
            if not self.inDB:
                self.state = "NOTFOUND"
            else:
                self.state = "CREATED:%s" % self.dbSkim.owner
        self.isUpdating = False
        self.lastUpdate = datetime.datetime.now().strftime( "%Y-%m-%d_%H.%M.%S" )
        #~ self.lock.release()

    def submit(self):
        crab = CrabController(proxy=self.proxy)
        try:
            crab.submit( self.name )
        except HTTPException:
            # Try resubmit once.
            #For this check if folder has been created
            # and kill task just to be sure
            try:
                crab.callCrabCommand(["kill", crabFoldername])
            except:
                pass
            if not os.path.isdir( crabFoldername ):
                shutil.rmtree( crabFoldername )
            crab.submit( os.path.join(self.crabFolder, self.name + "_cfg.py") )

    ## Function to handle Task which received NOSTATE status
    #
    # @param self: CrabTask The object pointer.
    def handleNoState( self ):
        crab = CrabController(proxy=self.proxy)
        if ("The CRAB3 server backend could not resubmit your task"\
            "because the Grid scheduler answered with an error."
            in task.failureReason):
            # move folder and try it again
            cmd = 'mv %s bak_%s' %(crab._prepareFoldername( self.name ),
                                   crab._prepareFoldername( self.name ))
            p = subprocess.Popen(cmd,stdout=subprocess.PIPE, shell=True)
            (out,err) = p.communicate()
            self.state = "SHEDERR"
            configName = '%s_cfg.py' % (crab._prepareFoldername( self.name ))
            crab.submit(configName)

        elif task.failureReason is not None:
            self.state = "ERRHANDLE"
            crab.resubmit(self.name)
        self.resubmitCount += 1

    ## Update state from database
    #
    def updateFromDB( self ):
        self.log.debug( "Checking if sample is in db ")
         # check if we have a db link
        if self.dblink is None:
            self.log.debug( "No dblink for task ")
            return False

        # fill search criteria for skim and samples
        skimCriteria = {}
        sampleCriteria = {}

        sampleCriteria[ "name" ] = self.name
        skimCriteria["version"] = self.skimmer_version
        skimCriteria["globaltag"] = self.globalTag
        skimCriteria["datasetpath"] = self.datasetpath
        if self.isData:
            skimCriteria["run_json"] = self.json_file

        self.log.debug( "Trying to query mc")
        self.log.debug( "skimCriteria: %s" % str( skimCriteria ) )
        self.log.debug( "sampleCriteria: %s" % str( sampleCriteria ) )
        self.log.debug(self.dblink.key)
        self.log.debug(self.dblink.certificate)
        searchResult = self.dblink.get_objects(sampleCriteria,
                                               skimCriteria,
                                               latest=True,
                                               data=self.isData)
        if searchResult:
            self.dbSample = searchResult[0]
            self.dbSkim = self.dbSample.skims[0]
            self.log.debug("Found Sample / Skim in DB")
            return True
        else: # check if the sample already exists
            self.log.debug("Trying to get only sample")
            searchResult = self.dblink.get_objects(sampleCriteria,
                                                   {},
                                                   data=self.isData)
            if searchResult:
                self.dbSample = searchResult[0]
                self.log.debug("only sample found")
                return False
        self.log.debug(" Sample + Skim not found in DB")
        return False

    def updateDB(self):
        # Do not update db if task is not created yet
        if not self.isCreated:
            self.log.debug("Crab task not created yet stoping update")
            return False

        self.log.debug("Filling Skim Fields for update")
        self.dbSample.name = self.name
        self.dbSample.energy = 13

        # Fill common fields
        self.dbSkim.datasetpath = self.crabConfig.Data.inputDataset
        outlfn = self.crabConfig.Data.outLFNDirBase.split('/store/user/')[1]
        self.dbSkim.updater = outlfn.split("/")[0]
        self.dbSkim.version = self.skimmer_version
        self.dbSkim.cmssw = os.getenv( 'CMSSW_VERSION' )
        self.dbSkim.globaltag = self.globalTag
        self.dbSkim.nevents = str( self.totalEvents )
        self.dbSkim.add_set_item("sites",self.site)
        now = datetime.datetime.now()
        if not self.dbSkim.id:
            self.dbSkim.owner = outlfn.split("/")[0]
            self.dbSkim.created = now
        if self.isFinal:
            self.dbSkim.finished = now


        if self.isData:
            # input json file
            self.dbSkim.run_json = self.crabConfig.Data.lumiMask.split("/")[-1]
            if self.isFinal:
                # add first and last run to db
                minRun, maxRun = self.getJsonBorders()
                self.log.debug( "Parsed json borders")
                # copy lumi summary json of processed lumi sections to SE and save path in DB
                if minRun != 0 and maxRun != 0:
                    lumi_summary_se_path = self.copyLumiSummaryToSE()
                    if lumi_summary_se_path:
                        self.log.debug( "copied lummi summary to SE")
                        self.dbSkim.processed_json = lumi_summary_se_path

                self.dbSkim.run_first = minRun
                self.dbSkim.run_last = maxRun
            else:
                self.log.debug("Data not final yet")
                self.log.debug("state %s" % self.state)
        elif not self.isData:
            mcmutil = dbutilscms.McMUtilities()
            mcmutil.readURL(self.crabConfig.Data.inputDataset)
            # Do not update meta infos if already set to preserve manual changes
            if not self.dbSample.generator:
                self.dbSample.generator = generators[self.name.split("_")[-1]]
            if not self.dbSample.crosssection:
                self.dbSample.crosssection = mcmutil.getCrossSection()
                if self.dbSample.generator in ("pythia6", "pythia8", "madgraph"):
                    self.dbSample.crosssection_order = "LO"
                else:
                    self.dbSample.crosssection_order = "NLO"
                self.dbSample.crosssection_ref = 'McM'
            if not self.dbSample.filterefficiency:
                self.dbSample.filterefficiency = mcmutil.getFilterEfficiency()
                self.dbSample.filterefficiency_ref = 'McM'
            if not self.dbSample.kfactor:
                self.dbSample.kfactor = 1.
                self.dbSample.kfactor_ref = None
            self.dbSample.energy = mcmutil.getEnergy()

        self.dbSample.skims = [self.dbSkim]
        self.dbSample.save()
        self.log.debug("Saved sample and skim")
        return True

    ## Function to update JobStatistics
    #
    # @param self: The object pointer.
    # @param dCacheFilelist: A list of files on the dCache
    def updateJobStats(self,dCacheFileList = None):
        jobKeys = sorted(self.jobs.keys())
        try:
            intJobkeys = [int(x) for x in jobKeys]
        except:
            print "error parsing job numers to int"

        stateDict = {'unsubmitted':0,
                     'idle':0,
                     'running':0,
                     'transferring':0,
                     'cooloff':0,
                     'failed':0,
                     'finished':0}
        nComplete = 0

        # loop through jobs
        for key in jobKeys:
            job = self.jobs[key]
             #check if all completed files are on decache
            for statekey in stateDict.keys():
                if statekey in job['State']:
                    stateDict[statekey]+=1
                    # check if finished fails are found on dCache if dCacheFilelist is given
                    if dCacheFileList is not None:
                        outputFilename = "%s_%s"%( self.name, key)
                        if ('finished' in statekey
                            and any(outputFilename in s for s in dCacheFileList)):
                            nComplete +=1

        for state in stateDict:
            attrname = "n" + state.capitalize()
            setattr(self, attrname, stateDict[state])
        self.nComplete = nComplete

    ## Function to read log info from log.tar.gz
    #
    # @param self: The object pointer.
    # @param logArchName: path to the compressed log file
    # @return a dictionary with parsed info
    def readLogArch(self, logArchName):
        JobNumber = logArchName.split("/")[-1].split("_")[1].split(".")[0]
        log = {'readEvents' : 0}
        with tarfile.open( logArchName, "r") as tar:
            try:
                nEvents = 0
                JobXmlFile = tar.extractfile('FrameworkJobReport-%s.xml' % JobNumber)
                root = ET.fromstring( JobXmlFile.read() )
                for child in root:
                    if child.tag == 'InputFile':
                        for subchild in child:
                            if subchild.tag == 'EventsRead':
                                nEvents += int(subchild.text)
                                log.update({'readEvents' : nEvents})
                                break
            except:
                print "Can not parse / read %s" % logArchName
        return log

    ## Return seperate list of job ids in equal chunks of finished jobs
    #
    #
    def getJobChunks( self, jids ):
        chunks = []
        # get a list of job ids for all finished jobs
        sorted_jids = sorted( [int(jid) for jid in jids] )
        no_gap = True
        first_jid = sorted_jids[0]
        chunk = []

        def chunk_append( chunk, jid, i, first_jid ):
            if jid == first_jid:
                chunk.append("%d" % jid)
                return 1
            else:
                chunk.append("%d-%d" % ( first_jid, jid) )
                return jid - first_jid

        n_added = 0
        for i,jid in enumerate( sorted_jids ):
            jid = int( jid )
            if i ==  len( sorted_jids ) -1:
                #self.log.debug("Working on last finished job %d" % jid)
                n_added += chunk_append( chunk, jid, i, first_jid)
                continue
            if abs( jid - sorted_jids[i+1]) > 1:
                n_added += chunk_append(chunk, jid, i, first_jid)
                first_jid = sorted_jids[i+1]
            if n_added + jid - first_jid > self.max_crab_chunk:
                chunk_append(chunk, jid, i, first_jid)
                chunks.append(chunk)
                chunk = []
                n_added = 0
                first_jid = sorted_jids[i+1]
        # add remaining entries
        chunks.append( chunk )
        return chunks

    ## Copy logs to local folder and return list of local paths
    #
    #@param self The object pointer
    def transferLogs(self):
        crab = CrabController(proxy=self.proxy)

        lfns = []
        # try to get logs for jobs whioch are finished but nut yet transfered
        jids = set(self.finished_job_ids) - set(self.transfered_log_ids)
        self.log.debug("logs to transfer")
        self.log.debug(jids)
        if not jids:
            return
        for chunk in self.getJobChunks(jids):
            res = crab.getlog( self.name, jobid_str = ",".join(chunk) )
            if not "lfn" in res:
                self.log.error("LFN key missing in crab response")
                self.log.debug(res)
                continue
            if not res["lfn"]:
                self.log.error("No files found in getlog output")
            try:
                lfns += res["lfn"]
            except:
                log.debug(res)
        logpath = os.path.join(self.crabFolder,"results")
        if not os.path.exists(logpath):
            os.makedirs(logpath)
        existing_logs = glob.glob(os.path.join(logpath, "cmsRun_*.log.tar.gz"))
        existing_logs = [os.path.basename(p) for p in existing_logs]
        for lfn in lfns:

            self.log.debug('se.fetch("%s", "%s", force=True)' % (lfn, logpath))
            if os.path.basename(lfn) in existing_logs:
                self.log.debug("skip existing")
                continue
            if "/failed/" in lfn:
                self.log.debug("skip failed logs")
                continue
            try:
                self.se.fetch( lfn,
                               os.path.join(logpath,os.path.basename(lfn)),
                               force = True )

            except gridlib.gfal2context.gfal2.GError:
                self.log.error("unable to copy log file " + logpath )
            self.log.debug("transfered %s " % os.path.basename(lfn))
        return

    def findFinishedFiles(self, finalizing = False):
        # get the files expected to be transferred to the se
        crab = CrabController(proxy=self.proxy)
        remote_files = []
        self.log.debug( " Getting list of output files which are not finalized yet")
        jids = set(self.finished_job_ids) - set(self.finalized_job_ids)
        if not jids:
            self.log.debug("All finished files are already finaized in skim")
            return True
        for chunk in self.getJobChunks(jids):
            res = crab.callCrabCommand( ["getoutput",
                                         "--dump",
                                         crab._prepareFoldername(self.name),
                                         "--jobids", ",".join(chunk)] )
            try:
                remote_files += res['lfn']
            except:
                self.log.debug(res)
        # Get all directories containing files
        remote_dirs = set([os.path.dirname(f) for f in remote_files if not "/failed/" in f])
        file_stats = []
        remote_files_temp = remote_files[:]
        # use stat from gridlib to find files and sizes.
        # Getting all files in folder with stat option is faster than
        # single file access
        for rdir in remote_dirs:
            self.log.debug("rdir %s" % rdir)
            dir_entries = self.se.ls(str(rdir), stat=True)
            for dir_entry in dir_entries:
                if not os.path.join(rdir,dir_entry.name) in remote_files:
                    continue
                remote_files.remove(os.path.join(rdir,dir_entry.name))
                file_stats.append({'name' : dir_entry.name,
                                   'path' : os.path.join(rdir,dir_entry.name),
                                   'size': dir_entry.stat.st_size})
        remote_files = remote_files_temp

        totalEvents = 0
        finalFiles = []
        for logPath in self.logPaths:
            JobNumber = str(self._logPath2jobNumber(logPath))
            try:
                log = self.readLogArch( logPath )
            except:
                msg = "Failed to read log arch %s" % logPath
                os.remove( logPath )
                if self.isData:
                    self.log.error(msg)
                    return False
                self.log.warning(msg)
            self.log.debug("searching for %s_%s.pxlio" % ( self.name, JobNumber ))
            matched_stat = [ stat for stat in file_stats if "%s_%s.pxlio" % ( self.name, JobNumber ) in stat['name'] ]
            if matched_stat and log['readEvents'] > 0:
                self.log.debug("Found")
                fdict = {'path':matched_stat[0]['path'],
                         'nevents':log['readEvents'],
                         'size':matched_stat[0]['size']}
                fkwargs = {'dbcon': self.dblink, 'field_dict':fdict}
                if self.isData:
                    dbfile = aachen3adb.DataFile(**fkwargs)
                else:
                    dbfile = aachen3adb.MCFile(**fkwargs)
                self.dbSkim.files.add(dbfile)
                totalEvents += log['readEvents']
            else:
                self.log.debug("Not Found")

        self.totalEvents = totalEvents
        self.log.debug(" Matched %d files with %d events to log " % (len(self.dbSkim.files),
                                                              totalEvents))

        # The rest of the function performs consistency checks of the result
        # and return False in case any check fails

        # check consistency for number of reported and found files
        if self.nFinished != len( self.dbSkim.files ):
            msg = "Crab reports output files which are not found on storage element !"
            msg += " %d in crab getoutput %d found with gfal" % ( self.nFinished,  len(self.dbSkim.files))
            if finalizing:
                self.log.error(msg)
                self.log.error("Stopping finalization")
                return False
            else:
                self.log.warning(msg)

        self.log.debug( "Found %d files" % len( file_stats ))

        # check consistency of completed jobs reported by crab and found files
        if self.nFinished != len( self.dbSkim.files ):
            msg = "Number of files/logs %d from crab does not match complete %d jobs"
            if finalizing:
                self.log.error( msg % (len( remote_files ), self.nComplete) )
            else:
                self.log.warning( msg % (len( remote_files ), self.nComplete) )
            return False

        return True

    ## Function to finalize task in TAPAS workflow
    #
    # Read config files, MCM and other sources and add sample to aix3adb.
    # This includes reading the number of processed events from crab logs,
    # match them to files in SE and populate the db files field.
    def finalizeTask(self):
        self.log.debug( "Starting finalize procedure " )
        self.log.debug( "Check if we update an already finalized sample"
                        "and need to get the crab status again" )
        if self.state == "FINAL":
            self.dbSkim.finished = None
            self.update(updateDB = False)

        self.log.debug( " Getting not yet transferred log files from dCache " )
        self.transferLogs()

        self.log.debug( " Getting final set of found files with strict consistency checks" )
        # The findFinishedFiles procedure will return False if any
        # consistency check fails
        if not self.findFinishedFiles(finalizing=True):
            return

        if self.isData:
            crab = CrabController(proxy=self.proxy)
            self.report = crab.report(self.name)
            # check if missing lumis are reported and stop finalize in this case
            if self.report and 'notProcessedLumis' in self.report and self.report['notProcessedLumis']:
                self.log.error('Not all lumis processed for datajob stopping finalization')
                self.state = 'LUMIMISS'
                return
            # compare number of events from log to report output
            if self.report and 'numEventsRead' in self.report and self.totalEvents != int(self.report['numEventsRead']):
                self.log.warning('Number of events from logs (%d) does not match numEventsRead from crab report (%d)' % (self.totalEvents, int(self.report['numEventsRead'])))

        self.state = "FINAL"
        self.updateDB()
        with open('finalSample','a') as outfile:
            outfile.write("%s:%s\n" % ( self.name,  self.crabConfig.Data.inputDataset))

    def copyLumiSummaryToSE( self ):
        try:
            se_path = os.path.join(self.crabConfig.Data.outLFNDirBase, 'processedLumis.json')
            self.se.cp( self.se.get_local_path(os.path.join(self.crabFolder, 'results', 'processedLumis.json')),
                        self.se.get_srm_path(se_path),
                        force=True )
            return se_path
        except:
            self.log.error('Unable to upload processedLumis.json to SE')
            return ""

    def getJsonBorders( self ):
        run_min, run_max = 0,0
        try:
            if 'processedLumis' in self.report:
                runs = [ int( run ) for run in self.report['processedLumis'] ]
            try:
                run_min, run_max = min(runs), max(runs)
            except:
                self.log.error( "Unable to parse run numbers in lummi summary json" )
                self.log.debug(self.report)
        except:
            self.log.error( "Unable to access local lumi summary from crab report" )
        return run_min, run_max

    def pickle_prepare(self):
        if not self._pickle_prepared:
            self.dbSkim = self.dbSkim.to_dict(keep_none=True)
            self.dbSample = self.dbSample.to_dict(keep_none=True)
            self.dblink = None
            self.se = None
            self._pickle_prepared = True

    def pickle_aftercare(self, dblink):
        if not self._pickle_prepared:
            return
        if self.isData:
            self.dbSample = aachen3adb.DataSample(field_dict=self.dbSample,
                                                dbcon=dblink)
            self.dbSkim = aachen3adb.DataSkim(field_dict=self.dbSkim,
                                              dbcon=dblink)
        else:
            self.dbSample = aachen3adb.MCSample(field_dict=self.dbSample,
                                                dbcon=dblink)
            self.dbSkim = aachen3adb.MCSkim(field_dict=self.dbSkim,
                                            dbcon=dblink)
        self.dblink = dblink
        self.se = gridlib.se.StorageElement( self.site )
        self.dbSample.skims = [self.dbSkim]
        self._pickle_prepared = False


## Class holds job statistics for several Crab tasks
#
# This class saves and updates statistics from a given list of CrabTask objects.
class TaskStats:

    ## The object constructor
    #
    # @type self: TaskStats
    # @param self: The object pointer.
    # @type tasklist: List of CrabTask objects
    # @param tasklist: (Optional) List of CrabTasks for which statistics should be calculated
    def __init__(self, tasklist = None):
        if tasklist is not None:
            self.updateStats(tasklist)
        else:
            self.clearStats()

    ## This function updates the statistics for a given tasklist
    #
    # @type self: TaskStats
    # @param self: The object pointer.
    # @type tasklist: List of CrabTask objects
    # @param tasklist: List of CrabTasks for which statistics should be calculated
    def updateStats(self,tasklist):
        self.clearStats()
        self.nTasks = len(tasklist)
        for task in tasklist:
            if not task.isUpdating:
                self.nUnsubmitted   += task.nUnsubmitted
                self.nIdle += task.nIdle
                self.nRunning += task.nRunning
                self.nTransferring    += task.nTransferring
                self.nCooloff    += task.nCooloff
                self.nFailed    += task.nFailed
                self.nFinished    += task.nFinished
                self.nComplete    += task.nComplete

    ## This function sets all counts to zero
    #
    # @type self: TaskStats
    # @param self: The object pointer.
    def clearStats(self):
        self.nTasks = 0
        self.nUnsubmitted   = 0
        self.nIdle = 0
        self.nRunning = 0
        self.nTransferring    = 0
        self.nCooloff    = 0
        self.nFailed    = 0
        self.nFinished    = 0
        self.nComplete    = 0

def setupLogging( logger, loglevel, log_file_name=""):
    pyloglevel = loglevel
    if loglevel == 'CRAB':
        pyloglevel = 'ERROR'
    #setup logging
    date = '%F %H:%M:%S'
    format = '%(levelname)s (%(name)s) [%(asctime)s]: %(message)s'
    logging.basicConfig( level=logging._levelNames[ pyloglevel ], format=format, datefmt=date )
    logger.setLevel(logging._levelNames[ pyloglevel ])
    formatter = logging.Formatter( format )
    if log_file_name:
        hdlr = logging.FileHandler( log_file_name, mode='w' )
        hdlr.setFormatter( formatter )
        logger.addHandler( hdlr )
    if not loglevel == 'CRAB':
        logging.getLogger('CRAB3').propagate = False  # Avoid any CRAB message to propagate up to the handlers of the root logger.
        setConsoleLogLevel(LOGLEVEL_MUTE)
