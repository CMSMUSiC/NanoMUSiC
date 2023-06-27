#! /usr/bin/env python2
import time
import sys
import os
import subprocess
import cPickle
import shutil
from collections import defaultdict
import multiprocessing
import logging
import glob
import re
import datetime
import traceback
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

def submitWorker(job):
    job.submit()
    return job

def runWorker(job):
    job.runLocal()
    return job

def resubmitWorker(job):
    job.resubmit()
    return job

def killWorker(job):
    job.kill()
    return job

def getCernUserName():
    try:
        username = os.environ["CERNUSERNAME"]
        return username
    except:
        try:
            from crabFunctions import CrabController
            crab = CrabController()
            return crab.checkusername()
        except:
            raise Exception("CERN user name could not be obtained. Please specify your CERN user name using the environment variable $CERNUSERNAME.")

def createAndUploadGridPack(localfiles, uploadurl, tarfile="gridpacktemp.tar.gz", uploadsite="srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2\?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/{username}/"):
    # create pack file
    command = ['tar', "zcvf", tarfile]
    if type(localfiles) == list:
        command.extend(localfiles)
    else:
        command.append(localfiles)
    process = subprocess.Popen(command, stdout=subprocess.PIPE,env=os.environ.copy())
    stdout, stderr = process.communicate()
    if process.returncode!=0:
        raise Exception ("Could not create tar file for grid pack: "+stdout+"\n"+stderr)
    return uploadGridPack(tarfile, uploadurl, uploadsite)

def uploadGridPack(tarfile, uploadurl, uploadsite="srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2\?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/{username}/", force=False):
    replacedict=dict()
    replacedict["createdate"]=datetime.datetime.now().strftime('%Y-%m-%d')
    replacedict["createdatetime"]=datetime.datetime.now().strftime('%Y-%m-%d-%H-%M-%S')
    replacedict['username']=getCernUserName()

    resultuploadurl=uploadurl.format(**replacedict)

    #check for an existing gridpack
    cmd = ["srmls",(uploadsite).format(**replacedict)+resultuploadurl]
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    process.communicate()
    if process.returncode == 0:
        if not force:
            print "File %s exists on dcache!" % ((uploadsite).format(**replacedict)+resultuploadurl)
            print 'Remove? [Y/N]'
            input = raw_input('-->')
            if input == 'y' or input == 'Y':
                force = True
        if force:
            deleteCommand = ["srmrm",(uploadsite).format(**replacedict)+resultuploadurl]
            process = subprocess.Popen(deleteCommand, stdout=subprocess.PIPE)
            process.communicate()

    # upload pack file
    command = ["srmcp", "file:///"+tarfile, (uploadsite).format(**replacedict)+resultuploadurl ]
    process = subprocess.Popen(command, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    if process.returncode!=0:
        raise Exception ("Could not upload grid pack: "+stdout+"\n"+stderr)
    return resultuploadurl


class Job:
    def __init__(self):
        self.inputfiles, self.outputfiles, self.arguments , self.executable = [], [], [], None
        self.frontEndStatus = ""
        self.jobid=None
    @property
    def status(self):
        try:
            status=str(self.infos["Status"])
        except:
            status="None"
        return status
    def writeJdl(self):

        if self.executable is None: self.executable = self.task.executable
        if (self.task.uploadexecutable):
            relPathExecutable = "./" + os.path.basename(self.executable)
        else:
            relPathExecutable = "./" + self.executable
        self.executable = os.path.abspath( self.executable )
        self.inputfiles = [os.path.abspath( ifile ) for ifile in self.inputfiles ]
        jdl = (
            '[Type = "Job";\n'
            'VirtualOrganisation = "dcms";\n'
            'AllowZippedISB = true;\n'
            #'Requirements = (RegExp("rwth-aachen.de", other.GlueCEUniqueId)) && (RegExp("cream", other.GlueCEUniqueId)) && !(RegExp("short", other.GlueCEUniqueId));\n'
            'ShallowRetryCount = 10;\n'
            'RetryCount = 3;\n'
            'MyProxyServer = "";\n'
            'executable = "prologue.sh";\n'
            'StdOutput = "out.txt";\n'
            'StdError  = "err.txt";\n'
            'outputsandboxbasedesturi="gsiftp://localhost";\n'
            )
        standardinput_files = ["./prologue.sh"]
        if self.task.uploadexecutable:
            standardinput.append(self.executable)
        jdl += 'InputSandbox = { "' + ('", "'.join(standardinput+self.inputfiles+self.task.inputfiles)) + '"};\n'
        stds=["out.txt", "err.txt"]
        if not isinstance(self.outputfiles,list) or not isinstance(self.task.outputfiles,list):
            raise Exception("You passed a non list object as outputfile argument! Make a list!")
        jdl += 'OutputSandbox = { "' + ('", "'.join(stds+self.outputfiles+self.task.outputfiles)) + '"};\n'
        jdl += 'Arguments = "' + (' '.join([str(self.nodeid), relPathExecutable] + self.arguments)) + '";\n'
        jdl += "]"
        self.jdlfilename = "job"+str(self.nodeid)+".jdl"
        jdl_file = open(self.jdlfilename, 'w')
        jdl_file.write(jdl)
        jdl_file.close()
        self.frontEndStatus = "JDLWRITTEN"
    def submit(self):
        startdir = os.getcwd()
        if startdir!=self.task.directory:
            os.chdir(self.task.directory)

        #get a proxy for at least 4 days
        checkAndRenewVomsProxy(604800)
        for i in range(50):
            #command = ['glite-ce-job-submit', '-a', '-r', 'ce201.cern.ch:8443/cream-lsf-grid_cms', self.jdlfilename]
            command = ['glite-ce-job-submit', '-a', '-r', self.task.ceId, self.jdlfilename]
            process = subprocess.Popen(command, stdout=subprocess.PIPE)
            stdout, stderr = process.communicate()
            if "FATAL" in stdout and "Submissions are disabled!" in stdout:
                log.debug(stdout)
                print "Submission server seems busy (Submissions are disabled). Waiting..."
                time.sleep(60*(i+1))
                continue
            if "FATAL - jobRegister" in stdout:
                log.debug(stdout)
                print "Submission server seems busy (jobRegister). Waiting..."
                time.sleep(60*(i+1))
                continue
            if "FATAL" in stdout and "Connection timed out" in stdout:
                log.debug(stdout)
                print "Submission server seems busy (Connection timed out). Waiting..."
                time.sleep(60*(i+1))
                continue
            if "FATAL - EOF detected during communication" in stdout:
                log.debug(stdout)
                print "Submission server seems busy (EOF detected during communication). Waiting..."
                time.sleep(60*(i+1))
                continue
            if "data_cb_read() - globus_ftp_client: the server responded with an error" in stdout:
                log.debug(stdout)
                print "Hickup in the ftp connection. Will try again. Waiting..."
                time.sleep(60*(i+1))
                continue
            if "FATAL" in stdout or "ERROR" in stdout or process.returncode != 0:
                log.error('Submission failed.')
                log.error('Output:\n' + stdout)
                self.error = stdout
                break
            self.frontEndStatus = "SENT"
            log.debug('Submission successful.')
            for line in stdout.splitlines():
                if "https://" in line:
                    self.jobid = line.strip()
                    log.debug("Submitted job "+self.jobid)
            break
        os.chdir(startdir)
        return process.returncode, stdout

    def runLocal(self):
        #import stat
        from string import Template
        startdir = os.getcwd()
        self.task.directory=self.task.directory.replace("bak/","")
        if startdir!=self.task.directory:
            os.chdir(self.task.directory)
        jobFileName="job_local_%d"%self.nodeid
        #if not os.path.exists(jobFileName):
        os.mkdir(jobFileName)
        #else:
            #try:
                #os.mkdir(os.path.join(self.directory,"bak"))
            #except OSError:
                #pass
            #try:
                #shutil.move(jobFileName, os.path.join(self.directory, "bak"))
            #except OSError:
                #pass
        jdl_file = open(self.jdlfilename, 'r')
        inputfiles=[]
        for line in jdl_file:
            if "InputSandbox" in line:
                line=line.strip()
                line=line.split("=")[1]
                line=line.replace("};","").replace("{","").replace('"',"").replace('./',"").replace(" ","")
                inputfiles=line.split(",")
        for file in inputfiles:
            shutil.copy(file,jobFileName)
        os.chdir(jobFileName)
        for file in glob.glob("*.sh"):
            os.system("chmod u+x %s"%file)
            #st = os.stat(file)
            #os.chmod(file, st.st_mode | stat.S_IEXEC)
        d = dict(
                VO_CMS_SW_DIR='/cvmfs/cms.cern.ch/'
            )
        file=open("prologue.sh","r")
        text=file.read()
        file.close()
        newText=Template(text).safe_substitute(d)
        fileNew=open("prologue.sh","w+")
        fileNew.write(newText)
        fileNew.close()

        localargs=(' '.join(["./prologue.sh","%d"%self.nodeid,"./"+os.path.basename(self.executable)] + self.arguments))
        if "grid-dcap." in localargs:
            localargs=localargs.replace("grid-dcap.","grid-dcap-extern.")
        else:
            localargs=localargs.replace("/pnfs","dcap://grid-dcap-extern.physik.rwth-aachen.de/pnfs")

        errFile=open("err.txt","w")
        outFile=open("out.txt","w")
        #print "run "+localargs
        process = subprocess.Popen(localargs, stdout=outFile, stderr=errFile, shell=True )
        #stdout, stderr = process.communicate()
        process.communicate()
        outFile.close()
        errFile.close()
        self.jobid=jobFileName
        return


    def getStatus(self):
        if self.frontEndStatus == "RETRIEVED" or self.frontEndStatus == "PURGED" or self.jobid is None:
            return
        command = ["glite-ce-job-status", self.jobid]
        log.debug("Getting status "+self.jobid)
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if process.returncode!=0:
            log.warning('Status retrieval failed for job id '+self.jobid)
            log.info(stdout)
            log.info(stderr)
        self.infos = parseStatus(stdout)
    def getOutput(self):
        if self.jobid is None:
            return
        log.debug("Getting output "+self.jobid)
        command = ["glite-ce-job-output", "--noint", "--dir", self.task.directory, self.jobid]
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if process.returncode!=0 or "This service does not support passive connections." in stderr:
            log.warning('Output retrieval failed for job id '+self.jobid)
        else:
            if len(stderr)>0:
                log.warning('Output retrieval failed for job id '+self.jobid)
                log.warning('with error: '+stderr)
            else:
                self.purge()
                self.frontEndStatus = "RETRIEVED"
    def cancel(self):
        if self.jobid is None:
            return
        log.debug("Canceling "+self.jobid)
        command = ["glite-ce-job-cancel", "--noint", self.jobid]
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if process.returncode!=0:
            log.warning('Cancelling failed for job id '+self.jobid)
        else:
            self.frontEndStatus = "CANCELLED"
    def purge(self):
        if self.jobid is None:
            return
        log.debug("Purging "+self.jobid)
        command = ["glite-ce-job-purge", "--noint", self.jobid]
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if process.returncode!=0:
            log.warning('Purging failed for job id '+self.jobid)
        else:
            self.frontEndStatus = "PURGED"
    def kill(self):
        self.cancel()
        self.purge()
        self.infos = dict()

    def resubmit(self):
        if self.status in ["PENDING", "IDLE", "RUNNING", "REALLY-RUNNING", "HELD"]:
            self.cancel()
        self.purge()
        self.infos = dict()
        self.submit()
    @property
    def outputSubDirectory(self):
        return str(self.jobid).replace("https://","").replace(":","_").replace("/","_")
    @property
    def jid(self):
        return str(self.jobid).split("/")[-1]

def lock(funct):
    def func_wrapper(instance, *args, **kwargs):
        if instance._isLocked():
            log.warning("Task is locked. Will not execute function %s", funct.__name__)
            return
        instance._lock()
        try:
            result = funct(instance, *args, **kwargs)
        except Exception as e:
            logging.error(traceback.format_exc())
            instance._unlock()
            raise
        instance._unlock()
        return result
    return func_wrapper


class Task:
    @classmethod
    def load(cls, directory):
        f = open(os.path.join(directory, "task.pkl"),'rb')
        obj = cPickle.load(f)
        f.close()
        obj.directory = os.path.abspath(directory)
        obj.mode = "OPEN"

        # for downward compatibility with old task.pkl files. This can be removed in the future
        if not 'ceId' in obj.__dict__:
            obj.ceId = 'grid-ce.physik.rwth-aachen.de:8443/cream-pbs-cms'
        return obj
    def __init__(self, name, directory = None, mode="RECREATE", scramArch=True, cmsswVersion=None, ceId='grid-ce.physik.rwth-aachen.de:8443/cream-pbs-cms'):
        self.name = name
        self.directory=directory
        if self.directory is None:
            self.directory = name
        self.directory = os.path.abspath(self.directory)
        self.jdlfilename = name+".jdl"
        self.inputfiles, self.outputfiles, self.jobs, self.executable = [], [], [], None
        self.mode = mode
        if scramArch is True:
            self.scramArch = os.environ.get('SCRAM_ARCH')
        else:
            self.scramArch = scramArch
        if cmsswVersion is True:
            self.cmsswVersion = os.environ.get('CMSSW_VERSION')
        else:
            self.cmsswVersion = cmsswVersion
        self.ceId = ceId
        self.jobs = []
        self.frontEndStatus=""
        self.stageOutDCache, self.gridPacks = [], []
        self.uploadexecutable = True
        self.replacedict = {'username': '${CESUBMITUSERNAME}', 'nodeid': '${CESUBMITNODEID}', 'createdate': '${CESUBMITCREATEDATE}', 'createdatetime': '${CESUBMITCREATEDATETIME}', 'taskname': "${CESUBMITTASKNAME}", 'runid': "${CESUBMITRUNID}"}
    def save(self):
        log.debug('Save task %s',self.name)
        f = open(os.path.join(self.directory, "task.pkl"), 'wb')
        cPickle.dump(self, f)
        f.close()
        f = open(os.path.join(self.directory, "jobids.txt"), 'w')
        for job in self.jobs:
            try:
                f.write(str(job.jobid)+"\n")
            except (AttributeError, TypeError):
                f.write("None\n")
        f.close()
    def addJob(self, job):
        job.task = self
        self.jobs.append(job)
    def submit(self, processes=0, local=False):
        log.info('Submit task %s',self.name)
        self.inputfiles = [os.path.abspath( ifile ) for ifile in self.inputfiles ]
        if len(self.jobs)==0:
            log.error('No jobs in task %s',self.name)
            return
        # create directory
        self.createdir()
        startdir = os.getcwd()
        os.chdir(self.directory)
        log.debug('Make prologue %s',self.name)
        self.makePrologue()
        checkAndRenewVomsProxy(604800)
        #enumerate jobs and create jdl files
        log.debug('Create %d jdl file',len(self.jobs))
        for i in range(len(self.jobs)):
            self.jobs[i].nodeid = i
            self.jobs[i].writeJdl()
        #multiprocessing
        if not local:
            self._dosubmit(range(len(self.jobs)), processes, submitWorker)
        else:
            self._dosubmit(range(len(self.jobs)), processes, runWorker)
        self.frontEndStatus="SUBMITTED"
        os.chdir(startdir)
        self.save()
    def _dosubmit(self, nodeids, processes, worker):
        jobs = [j for j in self.jobs if j.nodeid in nodeids]
        if processes:
            pool = multiprocessing.Pool(processes)
            result = pool.map_async(worker, jobs)
            result.wait()
            pool.close()
            #pool.join()
            while pool._cache:
                time.sleep(1)
            res = result.get()
            for job in res:  #because the task and jobs have been pickled, the references have to be restored
                job.task = self
                self.jobs[job.nodeid]=job
        else:
            for job in jobs:
                worker(job)

    def _lock(self):
        open(self.directory+"/.lock","a").close()

    def _unlock(self):
        try:
            os.remove(self.directory+"/.lock")
        except:
            return

    def _isLocked(self):
        return os.path.exists(self.directory+"/.lock")

    @lock
    def resubmit(self, nodeids, processes=0):
        if len(nodeids)==0:
            return
        log.info('Resubmit (some) jobs of task %s (%d jobs)',self.name,len(nodeids))
        self._dosubmit(nodeids, processes, resubmitWorker)
        self.frontEndStatus = "SUBMITTED"
        self.save()
        self.cleanUp()

    def resubmitLocal(self, nodeids, processes=0):
        log.debug('Finish up local (some) jobs of task %s',self.name)
        self._dosubmit(nodeids, processes, runWorker)
        self.frontEndStatus = "Local"
        self.save()
        self.cleanUp()

    def kill(self, nodeids, processes=0):
        log.debug('Kill (some) jobs of task %s',self.name)
        self._dosubmit(nodeids, processes, killWorker)
        self.save()
        self.cleanUp()
    def copyResultsToDCache(self, resultfile, uploadurl="{createdate}/{taskname}/{resultfileprefix}-{nodeid}_{runid}.{resultfilesuffix}", uploadsite="srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2\?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/{username}/"):
        self.stageOutDCache.append((resultfile, uploadsite+uploadurl))
    def addGridPack(self, uploadurl, extractdir="./", uploadsite="srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2\?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/{username}/"):
        self.gridPacks.append((uploadsite+uploadurl, extractdir))
    def makePrologue(self):
        executable = (
        '#!/bin/sh -e\n'
        +'echo Job started: $(date)\n'
        +'CESUBMITTASKNAME='+self.name+'\n'
        +'CESUBMITCREATEDATE='+datetime.datetime.now().strftime('%Y-%m-%d')+'\n'
        +'CESUBMITCREATEDATETIME='+datetime.datetime.now().strftime('%Y-%m-%d-%H-%M-%S')+'\n'
        +'CESUBMITUSERNAME='+getCernUserName()+'\n'
        +'CESUBMITNODEID=$1\n'
        +'rchars=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789; CESUBMITRUNID=; for i in {1..4}; do CESUBMITRUNID=$CESUBMITRUNID${rchars:$(($RANDOM % 62)):1}; done\n'
        +'shift\n'
        +'RUNAREA=$(pwd)\n'
        +'echo Running in: $RUNAREA\n'
        +'echo Running on: $HOSTNAME\n'
        )
        if self.cmsswVersion is not None:
            executable += (
            'echo Setting SCRAM_ARCH to ' + self.scramArch +'\n'
            +'export SCRAM_ARCH=' +self.scramArch + '\n'
            +'export BUILD_ARCH=$SCRAM_ARCH\n'
            +'source $VO_CMS_SW_DIR/cmsset_default.sh\n'
            +'scram project CMSSW ' + self.cmsswVersion + '\n'
            +'cd ' + self.cmsswVersion + '\n'
            +'eval $(scramv1 ru -sh)\n'
            +'cd $RUNAREA\n'
            )
        buildcmssw=False
        if self.gridPacks:
            executable += 'echo Retrieving grid packs\n'
            for inurl, outpath in self.gridPacks:
                gridpackname = os.path.basename(inurl)
                if "$CMSSW_BASE" in outpath:
                    buildcmssw=True
                command = "srmcp {0} file:///{1}".format(inurl, gridpackname)
                command = command.format(**self.replacedict)
                executable+= (
                command+"\n"
                +'mkdir -p '+outpath+"\n"
                +'tar xf {0} -C '.format(gridpackname)+outpath+'\n'
                +'rm {0}\n'.format(gridpackname)
                )
        if buildcmssw:
            executable+=(
            'cd $CMSSW_BASE\n'
            +'scram b\n'
            +'cd $RUNAREA\n'
            )
        executable += (
        'env\n'
        +'echo Current directory $PWD\n'
        +'echo Directory content:\n'
        +'ls -R\n'
        +'echo $@\n'
        +'chmod u+x $1\n'
        +'echo Executing $@\n'
        +'echo ================= Start output =================\n'
        +'$@\n'
        +'echo ================== End output ==================\n'
        +'echo Current directory $PWD\n'
        +'echo Directory content:\n'
        +'ls\n'
        )
        if self.stageOutDCache:
            executable += 'echo Copying files to dcache\n'
            for infile, outurl in self.stageOutDCache:
                command = "srmcp file:///{infile} {outurl}".format(infile=infile, outurl=outurl)
                infileparts=os.path.basename(infile).rsplit(".", 1)
                command = command.format(resultfileprefix=infileparts[0], resultfilesuffix=infileparts[-1], **self.replacedict)
                executable+=command+"\n"

        executable += (
        'echo Job ended: $(date)\n'
        )
        f = open("prologue.sh","w")
        f.write(executable)
        f.close()
    def createdir(self):
        if os.path.exists(self.directory) and self.mode!="RECREATE":
            raise Exception('Directory ' + self.directory + ' already exists')
        elif os.path.exists(self.directory):
            shutil.rmtree(self.directory)
        os.makedirs(self.directory)
    def _getStatusMultiple(self, joblist=None, retry=0):
        if joblist is None:
            totaljobs = [job for job in self.jobs if job.frontEndStatus not in ["RETRIEVED", "PURGED"] and job.status not in ["COMPLETED"] ] #Yannik changed from DONE-OK
        else:
            totaljobs = [job for job in joblist if job.frontEndStatus not in ["RETRIEVED", "PURGED"] and job.status not in ["COMPLETED"]  ] #Yannik changed from DONE-OK
        jobpackages = list(chunks(totaljobs, 50))
        njobs = 0
        listOfJobsToRetry=set()
        for jobs in jobpackages:
            jobids = [job.jobid for job in jobs if job.jobid is not None]
            if not jobids: return 0
            command = ["glite-ce-job-status", "-L1"] + jobids
            process = subprocess.Popen(command, stdout=subprocess.PIPE)
            stdout, stderr = process.communicate()
            if process.returncode!=0:
                log.warning('Status retrieval failed for task '+self.name)
                log.info(stdout)
                log.info(stderr)
            result = parseStatusMultipleL1(stdout)
            realjobs=0
            for job in jobs:
                try:
                    infos = result[job.jobid]
                    if len(infos)>0:
                        job.infos = infos
                        realjobs+=1
                    else:
                        log.warning('Failed to get status of job %s of task %s',job.jobid, self.name)
                        listOfJobsToRetry.add(job)
                except KeyError as keyerr:
                    log.warning('Failed to get status of job %s of task %s',job.jobid, self.name)
                    if job.jobid is not None:
                        listOfJobsToRetry.add(job)
            log.debug('Got status for %d jobs',realjobs)
            njobs+=len(jobids)
            if len(listOfJobsToRetry)>0:
                jobpackages = list(chunks(list(listOfJobsToRetry), max(int(  min(len(listOfJobsToRetry),50) /(retry+2)),1)) )
                for jobs in jobpackages:
                    self._getStatusMultiple(joblist=jobs,retry=retry+1)
        return njobs

    @lock
    def getStatus(self):
        log.debug('Get status of task %s',self.name)
        numberOfJobs = self._getStatusMultiple()
        oldfestatus = self.frontEndStatus
        retrieved, done, running, purged = True, True, False, True
        for job in self.jobs:
            if job.frontEndStatus!="RETRIEVED":
                retrieved=False
            if job.frontEndStatus!="PURGED":
                purged = False
            if "RUNNING" in job.status:
                running=True
                break
            if "COMPLETED" not in job.status: #Yannik changed it from DONE
                done = False
        if running: self.frontEndStatus="RUNNING"
        elif retrieved: self.frontEndStatus="RETRIEVED"
        elif done: self.frontEndStatus="COMPLETED" #Yannik changed from DONE
        elif purged: self.frontEndStatus="PURGED"
        if numberOfJobs > 0 or oldfestatus != self.frontEndStatus:
            self.save()
        return self.frontEndStatus

    @lock
    def getOutput(self, connections=1 , joblist=None, retry=0):

        if joblist is None:
            jobs = [job for job in self.jobs if job.status=="COMPLETED" and job.frontEndStatus not in ["RETRIEVED", "PURGED"]] #Yannik changed from DONE-OK
        else:
            jobs = [job for job in joblist if job.status=="COMPLETED" and job.frontEndStatus not in ["RETRIEVED", "PURGED"]] #Yannik changed from DONE-OK
        if not jobs:
            return
        jobpackages = list(chunks(jobs, 50))
        log.info('Get output of %s jobs of task %s',str(len(jobs)), self.name)
        retryjobs=set()
        for jobpackage in jobpackages:
            jobids = [job.jobid for job in jobpackage if job.jobid is not None]
            command = ["glite-ce-job-output", "-s", str(connections), "--noint", "--dir", self.directory] + jobids
            process = subprocess.Popen(command, stdout=subprocess.PIPE)
            stdout, stderr = process.communicate()
            if process.returncode!=0:
                log.warning('Output retrieval failed for task '+self.name)
                log.info(stdout)
                log.info(stderr)
                for job in jobpackage:
                    retryjobs.add(job)
            else:
                succesfulljobids = parseGetOutput(stdout)
                log.info('Retrieved %s jobs for %s', str(len(succesfulljobids)),self.name)
                for job in jobpackage:
                    if job.jobid in succesfulljobids:
                        job.purge()
                        job.frontEndStatus = "RETRIEVED"
                        log.debug('Successfully retrieved job %s', job.jobid)
                    else:
                        job.frontEndStatus = "FAILED2RETRIEVE"
                        log.warning('Failed to retrieve job %s', job.jobid)
        if len(retryjobs)>0:
            jobpackages = list(chunks(list(retryjobs), max(int( min(len(retryjobs),50) /(retry+2)),1) ) )
            for ijoblist in jobpackages:
                self._unlock()
                self.getOutput(connections=1 ,joblist=ijoblist,retry=retry+1)


        self.save()

    def jobStatusNumbers(self):
        jobStatusNumbers=defaultdict(int)
        good, bad = 0, 0
        for job in self.jobs:
            try:
                jobStatusNumbers[job.status]+=1
                jobStatusNumbers[job.frontEndStatus]+=1
                if job.status in ["ABORTED", "FAILED"]: #Yannik changed from DONE-FAILED
                    bad += 1
                if job.status == "COMPLETED": #Yannik changed from DONE-OK
                    if "ExitCode" in job.infos:
                        if job.infos["ExitCode"] == "0":
                            good += 1
                        else:
                            bad += 1
            except AttributeError:
                pass
        jobStatusNumbers["total"]=len(self.jobs)
        jobStatusNumbers["good"] = good
        jobStatusNumbers["bad"] = bad
        return jobStatusNumbers
    def cleanUp(self):
        log.debug('Cleaning up task %s',self.name)
        subdirs=[job.outputSubDirectory for job in self.jobs if job.jobid is not None]
        for checkdir in glob.glob(os.path.join(self.directory,"*")):
            if os.path.isdir(checkdir) and os.path.basename(checkdir)!="bak":
                if os.path.basename(checkdir) not in subdirs:
                    try:
                        os.mkdir(os.path.join(self.directory,"bak"))
                    except OSError:
                        pass
                    shutil.move(checkdir, os.path.join(self.directory, "bak"))



def parseGetOutput(stdout):
    successfuljobidlist=[]
    for line in stdout.splitlines():
        if "https" not in line: continue
        regex = re.compile("\[(https.*?)\]")
        r = regex.search(line)
        try:
            jobid = r.groups()[0]
        except IndexError:
            continue
        if "output will be stored" in line:
            successfuljobidlist.append(jobid)
    return successfuljobidlist

def parseStatus(stdout):
    # parses the output of glite-ce-job-status to a dict
    result=dict()
    for line in stdout.splitlines():
        try:
            key, value = line.split("=",1)
        except ValueError:
            continue
        key, value = key.strip("\t* "), value.strip()[1:-1]
        result[key] = value
    return result

def parseStatusMultiple(stdout):
    # parses the output of glite-ce-job-status to a dict for multiple jobids
    result = dict()
    jobid = None
    for line in stdout.splitlines():
        try:
            key, value = line.split("=",1)
        except ValueError:
            continue
        key, value = key.strip("\t* "), value.strip()[1:-1]
        if key=="JobID":
            jobid = value
            result[jobid]=dict()
        result[jobid][key] = value
    return result

def parseStatusMultipleL1(stdout):
    # parses the output of glite-ce-job-status -L1 to a dict, this includes the status history
    result = dict()
    jobid = None
    for line in stdout.splitlines():
        try:
            key, value = line.split("=",1)
        except ValueError:
            continue
        if "Command" in key: continue
        key, value = key.strip("\t* "), value.strip()[1:-1]
        if key=="JobID":
            jobid = value
            result[jobid]=dict()
            result[jobid]["history"]=list()
        if key=="Status":
            status=value.split()[0][0:-1]
            timestamp=value.split()[-1][1:]
            result[jobid]["history"].append( (status, timestamp,) )
            result[jobid]["Status"] = status
        else:
            result[jobid][key] = value
    return result

def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in range(0, len(l), n):
        yield l[i:i+n]

class ProxyError( Exception ):
    pass

def timeLeftVomsProxy():
    log.debug('Check time left of VomsProxy')
    """Return the time left for the proxy."""
    proc = subprocess.Popen( ['voms-proxy-info', '-timeleft' ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
    output = proc.communicate()[0]
    log.debug('Check time left of VomsProxy [Done]')
    if proc.returncode != 0:
        return False
    else:
        return int( output )

def checkVomsProxy( time=86400 ):
    """Returns True if the proxy is valid longer than time, False otherwise."""
    timeleft = timeLeftVomsProxy()
    return timeleft > time

def renewVomsProxy( voms='cms:/cms/dcms', passphrase=None ):
    """Make a new proxy with a lifetime of one week."""
    if passphrase:
        p = subprocess.Popen(['voms-proxy-init', '--voms', voms, '--valid', '192:00'], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
        stdout = p.communicate(input=passphrase+'\n')[0]
        retcode = p.returncode
        if not retcode == 0:
            raise ProxyError( 'Proxy initialization command failed: '+stdout )
    else:
        retcode = subprocess.call( ['voms-proxy-init', '--voms', voms, '--valid', '192:00'] )
    if not retcode == 0:
        raise ProxyError( 'Proxy initialization command failed.')

def checkAndRenewVomsProxy( time=604800, voms='cms:/cms/dcms', passphrase=None ):
    log.debug('Check and renew VomsProxy')
    """Check if the proxy is valid longer than time and renew if needed."""
    if not checkVomsProxy( time ):
        renewVomsProxy(passphrase=passphrase)
        if not checkVomsProxy( time ):
            raise ProxyError( 'Proxy still not valid long enough!' )
    log.debug('Check and renew VomsProxy [Done]')
