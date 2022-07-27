#!/usr/bin/env python
# Copyright [2016] <III Phys. Inst A, RWTH Aachen University>

# absolute imports
from __future__ import absolute_import

# standard library imports
import sys
import os
import time
import datetime
import logging
import subprocess
import textwrap
import functools
import math
import collections
import cPickle as pickle
import multiprocessing
import glob
import shutil
import re
import traceback
import copy
import uuid
import random

# local imports
import gridlib.se
import gridlib.util

# setup logging
log = logging.getLogger(__name__)


## Class to handle proxy delegation for ce job submission
class ProxyDelegator(object):

    ## Default constructor
    #
    # @param self Object pointer
    def __init__(self, storage_element):
        # initialize external variables
        self.storage_element = storage_element
        self.username_cern = gridlib.util.get_username_cern()
        logging.basicConfig(level=logging.DEBUG)
        # delegation related variables
        self.delegation_id = None


    ## Generate a delegation proxy and store its id
    #
    # This delegation id can and should be used for submitting a batch of jobs.
    #
    # @param self Object pointer
    def create_delegation_proxy(self):
        ce_id = self.storage_element.get_ce_id()
        host = ce_id.split('/')[0]
        delegation_id = '{0}_{1}_{2}'.format(
            self.username_cern,
            time.strftime('%Y-%m-%d_%H-%M-%S'),
            uuid.uuid1().hex)
        command = ['glite-ce-delegate-proxy', '-e', host, delegation_id]
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if process.returncode != 0:
            command_str = ' '.join(command)
            error_message = 'Failed to create delegation proxy:\n{0}\n{1}{2}'.format(command_str, stdout, stderr)
            log.error(error_message)
            raise OSError(process.returncode, error_message)
        # save delegation id if its creation was succesful
        self.delegation_id = delegation_id


    ## Return the delegation id
    #
    # If no delegation id exists, one is created before returning it.
    #
    # @param self Object pointer
    def get_delegation_id(self):
        if not self.delegation_id:
            self.create_delegation_proxy()
        return self.delegation_id


## Abstract class to store selected Task settings
#
# These information can be propagated to a Job, in case it is being added to a
# Task. The idea is to use inheritance to avoid code duplication. This class is
# not meant to be created by itself, only used conjunction with Task and Job
# instances.
class TaskSettings(object):

    ## Default constructor
    #
    # @param self Object pointer
    def __init__(self):
        # general
        self.directory = None
        # io
        self.inputfiles = []
        self.outputfiles = []
        # exe
        self.executable = None
        self.executable_upload = True
        # proxy delegation
        self.ce_id = None
        #self.delegation_id = None
        job_max_runtime = None


    ## Get a dict containing all relevant Task settings
    #
    # @param self Object pointer
    def get_settings(self):
        attributes = ['directory', 'inputfiles', 'outputfiles',
                      'executable', 'executable_upload',
                      #'ce_id', 'job_max_runtime', 'delegation_id']
                      'ce_id', 'job_max_runtime']
        settings_dict = dict()
        for attribute in attributes:
            settings_dict[attribute] = copy.copy(getattr(self, attribute))
        return settings_dict


    ## Load settings from a dictionary
    #
    # load_settings() is meant to be used to load settings from the settings
    # retrieved from the get_settings() function.
    #
    # @param self Object pointer
    # @param settings_dict Dictionary containing the TaskSettings settings
    def load_settings(self, settings_dict):
        for key in settings_dict:
            setattr(self, key, settings_dict[key])



## Gridjob class
#
# This class manages running on the grid with the provided parameters
class Job(TaskSettings):

    ## Default constructor
    #
    # @param self Object pointer
    def __init__(self):
        # call inherited constructor
        super(Job, self).__init__()

        # job specific arguments
        self.arguments = []

        # various job specific information
        self.frontendstatus = ''
        self.infos = dict()
        self.job_id = None
        self.job_directory = None


    ## Returns the current status of the job
    #
    # @param self Object pointer
    @property
    def status(self):
        if 'Status' in self.infos:
            return str(self.infos['Status'])
        return 'NONE'


    ## Create the jdl file, substituting various arguments
    #
    # @param self Object pointer
    def create_jdl(self):
        # get absolute paths

        # create jdl header
        jdl = textwrap.dedent(
            '''\
            universe = grid
            Requirements = TARGET.FileSystemDomain != "hpc.itc.rwth-aachen.de"
            grid_resource = condor grid-ce-1-rwth.gridka.de grid-ce-1-rwth.gridka.de:9619
            use_x509userproxy = true 
            Executable = prologue.sh
            Log = log.txt
            Output = out.txt
            Error = err.txt
            request_memory = 20000
            +maxMemory = 20000
            '''
        )
        #Yannik changed the request_memory from 2000 to 3000 to 3500 to 4000 to 6000 to 8000 to 10000 (in MiB)
        #For scans 2000M is fine. For classify you should increase to 10000M
        #I do not know why for MUSIC we are not able to trasfer only the MusicOutDir.tar.gz correctly. So I have commented this two line.
        #should_transfer_files = YES
        #when_to_transfer_output = ON_EXIT
        #+xcount = 10
        #request_cpus = 10
        #
        #request_memory = 20 GB

        # files to be uploaded
        inputsandbox = ['./prologue.sh']
        if self.executable_upload:
            inputsandbox.append(os.path.abspath(self.executable))
        inputsandbox.extend([os.path.abspath(ifile) for ifile in self.inputfiles])
        #jdl += 'InputSandbox = {{"{0}"}};\n'.format('", "'.join(inputsandbox))
        jdl += 'transfer_input_files = {0}\n'.format(', '.join(inputsandbox))

        # files to be retrieved
        output = self.outputfiles
        if not isinstance(self.outputfiles, list):
            raise Exception('Outputfiles are not a list; check config file')
        #if output:
        #    jdl += 'transfer_output_files = {0}\n'.format(', '.join(output + self.outputfiles)) #Yannik commented it out
        #FOR MUSIC THE TWO LINE BELOW ARE NOT WORKING. Some problem to retrieve only the output needed.

        # arguments
        if self.executable_upload:
            executable = os.path.join('./', os.path.basename(self.executable))
        else:
            executable = os.path.join('./', self.executable)
        jdl += 'Arguments = {0}\n'.format(' '.join([str(self.node_id), executable] + self.arguments))

        # write jdl file and update status
        jdl += "Queue"
        self.jdlfilename = 'job{0}.jdl'.format(self.node_id)
        with open(self.jdlfilename, 'w') as jdl_file:
            jdl_file.write(jdl)
        self.frontendstatus = 'JDLWRITTEN'


    ## Submit the job to be computed
    #
    # @param self Object pointer
    def submit(self):
        # change into directory of job
        startdir = os.getcwd()
        if startdir != self.job_directory:
            os.chdir(self.job_directory)

        # ensure there is a proxy with sufficient time remaining
        gridlib.util.voms_proxy_ensure(time=3600)
        for i in xrange(50):
            # build submission command
            command = ['condor_submit','-verbose', self.jdlfilename] #Yannik add -verbose
            process = subprocess.Popen(command, stdout=subprocess.PIPE)
            stdout, stderr = process.communicate()

            # check for error messages in the output
            if 'FATAL' in stdout:
                # recoverable submission failures
                error_msgs = ['Submissions are disabled',
                              'jobRegister',
                              'Connection timed out',
                              'EOF detected during communication']
                # check for recoverable error message
                recoverable = ''
                for error_msg in error_msgs:
                    if error_msg in stdout:
                        recoverable = error_msg
                        break
                # retry submission if error is recoverable
                if recoverable:
                    print 'Submission server seems busy ({0}). Waiting...'.format(recoverable)
                    log.debug(stdout)
                    time.sleep(60 * (i + 1))
                    continue
                # non-recoverable failures
                log.error('Submission failed - Output:\n{0}'.format(stdout))
                self.frontendstatus = 'FAILED' #Every DONE-FAILED has been changed with FAILED #Yannik changed that
                break
            # ftp failure
            elif 'data_cb_read() - globus_ftp_client: the server responded with an error' in stdout:
                print 'Hickup in the ftp connection. Will try again. Waiting...'
                log.debug(stdout)
                time.sleep(60 * (i + 1))
                continue
            # general failure; not recoverable
            elif 'ERROR' in stdout or process.returncode is not 0:
                log.error('Submission failed - Output:\n{0}'.format(stdout))
                self.frontendstatus = 'FAILED'
                break

            # submission success
            self.frontendstatus = 'SENT'
            log.debug('Submission of one job is successful.')
            test_q_output = self.test_cond_status()#Yannik add
                        
            for line in stdout.splitlines(): 
                if 'Proc' in line and 'Id' not in line:             #Yannik add
                    tempID = line.split('c')
                    tr1 = tempID[len(tempID) -1]
                    tr2 = tr1.replace(':','')
                    self.job_id = tr2.strip() # Yannik added this line
                    log.debug('Submitted job with job id:  {0}'.format(self.job_id))
            break
        os.chdir(startdir)
        return process.returncode, stdout

    # ********** Yannik MAIN CHANGE **************
    def test_cond_status(self):
        command = ['condor_q', '-grid']
        log.debug("Getting status {0}".format(self.job_id))
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        
        return stdout
    # ****** END ****** 



    ## Updates the status of the Job
    #
    # This function is rarely used, since one can perform multiple status
    # requests at once.
    #
    # @param self Object pointer
    def update_status(self):
        # if the job is not active anymore, return
        if self.frontendstatus == 'RETRIEVED' or self.frontendstatus == 'PURGED' or self.job_id is None:
            log.debug("The job {0} has been RETRIEVED,PURGED or does not have a job id. I do not update the status".format(self.job_id))
            return
        # otherwise get status
        #command = ['glite-ce-job-status', self.job_id] #Yannik commented it out
        command = ['condor_q', '-grid', self.job_id]
        log.debug("Getting and updating status for job id {0}".format(self.job_id))
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if process.returncode is not 0:
            log.warning('Status retrieval failed for job id {0}\n{1}\n{2}'.format(self.job_id,
                                                                                  stdout,
                                                                                  stderr))
        # update job status
        self.infos = parse_status(stdout, self.job_id)


    ## Retrieve output of Job
    #
    # @param self Object pointer
    def retrieve_output(self):
        if self.job_id is None:
            return
        log.debug('Retrieving the output {0}'.format(self.job_id))
        #command = ['glite-ce-job-output', '--noint', '--dir', self.task.directory, self.job_id]
        command = ['condor_q', '-grid', self.job_id]
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if (process.returncode != 0) or 'This service does not support passive connections.' in stderr  or (not os.path.exists("{OUTDIR}/out.txt".format(OUTDIR=self.output_directory()))):
            log.warning('Output retrieval failed for job id {0}'.format(self.job_id))
        else:
            if len(stderr)>0:
                log.warning('Output retrieval failed for job id {0}:\n{1}'.format(self.job_id, stderr))
            else:
                #self.purge() #Yannik commented it out
                self.frontendstatus = 'RETRIEVED'


    ## Cancel the Job
    #
    # @param self Object pointer
    def cancel(self):
        if self.job_id is None:
            return
        log.debug('Canceling '+self.job_id)
        #command = ['glite-ce-job-cancel', '--noint', self.job_id]
        command = ['condor_rm', self.job_id]
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if process.returncode!=0:
            log.warning('Cancelling failed for job id '+self.job_id)
        else:
            self.frontendstatus = 'CANCELLED'


    ## Purge the Job
    #
    # @param self Object pointer
    def purge(self):
        if self.job_id is None:
            return
        log.debug('Purging '+self.job_id)
        #Yannik commented out all the following line and he added other lines
        #command = ['glite-ce-job-purge', '--noint', self.job_id]
        command = ['condor_rm', self.job_id]
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if process.returncode!=0:
            log.warning('Purging failed for job id '+self.job_id)
        else:
            self.frontendstatus = 'PURGED'
        #self.frontendstatus = 'PURGED'

    ## Cancel, then purge the Job and clear the info dict
    #
    # @param self Object pointer
    def kill(self):
        #self.cancel() #Yannik commented it put
        self.purge()
        self.infos = dict()


    ## Kill the Job if necessary, then submit it again
    #
    # @param self Object pointer
    def resubmit(self):
        #if self.status in ['PENDING', 'IDLE', 'RUNNING', 'REALLY-RUNNING', 'HELD']: #Yannik commented it out
            #self.cancel()
        #self.purge()
        self.infos = dict()
        self.submit()


    ## Get the output directory of the job
    #
    # @param self Object pointer
    @property
    def output_directory(self):
        return str(self.job_id).replace('https://', '').replace(':', '_').replace('/', '_')


    ## Get the job ID
    #
    # @param self Object pointer
    @property
    def jid(self):
        return str(self.job_id).split('/')[-1]

    ## Internal function to load a job text ouput file if it exists
    #
    # @param self Object pointer
    # @param filnemae Name of the output file e.g. out.txt
    def _load_job_output_file(self, filename):
        if not os.path.exists(os.path.join(self.directory, self.output_directory, filename)):
            print os.path.join(self.directory, self.output_directory, filename) + " does not exist"
            return None
        with open(os.path.join(self.directory, self.output_directory, filename), "r") as f:
            return f.read()

    ## Get the stdout for a retrieved job
    #
    # @param self Object pointer
    @property
    def stdout(self):
        return self._load_job_output_file("out.txt")

    ## Get the stderr for a retrieved job
    #
    # @param self Object pointer
    @property
    def stderr(self):
        return self._load_job_output_file("err.txt")

    ## Get the total runtime for this job
    #
    #@param self Object pointer
    # @param self Object pointer
    # @param self Object pointer
    def last_history_timestamp(self, key):
        if not self.infos['history']:
            return -1
        timestamps = sorted([int(t[1]) for t in self.infos['history'] if t[0].startswith(key)])
        if not timestamps:
            return -1
        return timestamps[-1]

    ## Get the total runtime for this job
    # @param self Object pointer
    @property
    def runtime(self):
        timestamp = self.last_history_timestamp("REALLY-RUNNING")
        if timestamp > 0:
            return time.time() - timestamp
        return 0


##################################################
# Task class and support functions

## Job lock wrapper
#
# This function is wrapped around other functions. It allows for locking of the
# specific job.
#
# @param funct The function to wrap around
def lock(funct):
    def func_wrapper(instance, *args, **kwargs):
        if instance._is_locked():
            log.warning('Task is locked. Will not execute function {0}'.format(funct.__name__))
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


## Run a function of Job instance
#
# This is used as a wrapper for multiprocessing with class instances.
#
# @param func_name Name of the function to execute
# @param job Job instance of which to run the function
def job_apply(func_name, job):
    getattr(job, func_name)()
    return job


## Parse the output of the glite-ce-job-output command
#
# @param stdout Output of the command
def parse_output(stdout):
    successful_job_ids = []
    for line in stdout.splitlines():
        if 'https' not in line: continue
        regex = re.compile('\[(https.*?)\]')
        r = regex.search(line)
        try:
            job_id = r.groups()[0]
        except IndexError:
            continue
        if 'output will be stored' in line:
            successful_job_ids.append(job_id)
    return successful_job_ids


## Parse the output of glite-ce-job-status to a dict
#
# @param Output of the command
def parse_status(stdout, j):
    result=dict()
    #Yannik main changes (default below)
    job_id = None
    ln2= list()
    ln1= ''
    timestamp = ''
    status = None
    tmpk= dict()
    i=0
    for j in job_ids: #Yannik added this for
        tmpk[j] = dir_tmp[i]
        if str(j) not in stdout:
            phrase = str(j) + '  dummyowner  ' + '   COMPLETED   '  + ' condor  '
            stdout += phrase + '\n'
        i +=1
    
    for line in stdout.splitlines():
        #Yannik MAIN change
        if 'Schedd' in line:
            timestamp = line.split('@ ', 1)[1]
            tmstr= timestamp.replace("/", "-")
            timesst=time.mktime(datetime.datetime.strptime(tmstr, "%m-%d-%y %H:%M:%S").timetuple())
            #date_time_obj = datetime.datetime.strptime(tmstr, '%m-%d-%y %H:%M:%S')
            continue
        elif 'condor' in line:
            ln1=line.split('condor')[0]
            ln2=ln1.split()
            job_id=ln2[0]
        else:
            continue
    if len(ln2) < 3:
        status = 'IDLE'
    elif len(ln2)==3:
        status = ln2[2]
    else:
        print '######## ERROR: status lenght is less then 0 ############'
        # END
        
    if status == 'COMPLETED':
        print("************************* before FAILED Line 576 in ce.py ***************************") #Yannik add this
        print tmpk[job_id]
        if not os.path.isfile(os.path.join(tmpk[job_id], '/*Output*' )):
            status = 'FAILED'    
        
        
    result[job_id] = dict()
    result[job_id]['history'] = list()
    result[job_id]['history'].append([status, timesst])
    result[job_id]['Status'] = status
    return result
    #END Yannik changes
    
    # The part below is DEFAULt before Yannik changes
    '''
    for line in stdout.splitlines():
        try:
            key, value = line.split('=',1)
        except ValueError:
            continue
        key, value = key.strip('\t* '), value.strip()[1:-1]
        result[key] = value
    return result
    '''

## Parse the output of glite-ce-job-status to a dict for multiple Jobs
#
# @param stdout Output of the command
def parse_status_multiple(stdout):
    result = dict()
    job_id = None
    for line in stdout.splitlines():
        try:
            key, value = line.split('=',1)
        except ValueError:
            continue
        key, value = key.strip('\t* '), value.strip()[1:-1]
        if key=='Job_Id':
            job_id = value
            result[job_id]=dict()
        result[job_id][key] = value
    return result


## Parse the output of glite-ce-job-status -L1 to a dict
#
# Invoking the command with the L1 flag includes the status history.
#
# @param stdout Output of the command
def parse_status_multiple_l1(stdout,job_ids, dir_tmp): #Yannik improve the function adding job_ids 
    
    #Yannik MAIN change
    
    #**** END *******
    result = dict()
    job_id = None
    ln2= list()
    ln1= ''
    timestamp = ''
    status = None
    tmpk= dict()
    tmpddir=[]
    extcd="1"
    tmpcom = 0
    i=0
    log_path= ''
    err_path= ''
    out_path= ''
    tpt = ''
    
    for j in job_ids: #Yannik added this for
        tmpk[j] = dir_tmp[i]
        if str(j) not in stdout:
            phrase = str(j) + '  dummyowner  ' + '   COMPLETED   '  + ' condor  '
            stdout += phrase + '\n'
        i +=1
                
    for line in stdout.splitlines():
       
        
        #Yannik MAIN change
        if 'Schedd' in line:
            timestamp = line.split('@ ', 1)[1]
            tmstr= timestamp.replace("/", "-")
            timesst=time.mktime(datetime.datetime.strptime(tmstr, "%m-%d-%y %H:%M:%S").timetuple())
            #date_time_obj = datetime.datetime.strptime(tmstr, '%m-%d-%y %H:%M:%S')
            continue
        elif 'condor' in line:
            ln1=line.split('condor')[0]
            ln2=ln1.split()
            job_id=ln2[0]
        else:
            continue
        if len(ln2) < 3 or ln2[2] == 'SUSPENDED':
            status = 'IDLE'
        elif len(ln2)==3:
            status = ln2[2]
        else:
            print '######## ERROR: status lenght is less then 0 ############'
        # END
        if status == 'COMPLETED' or status == 'FAILED' or status == "RESUB" or status == "HOLD" or status == "HELD":
            
            status= 'RESUB'
            extcd="1"
            tmpddir = os.listdir(tmpk[job_id])
            tpt = tmpk[job_id]
            log_path = os.path.join(tpt, 'log.txt')
            err_path = os.path.join(tpt, 'err.txt')
            out_path = os.path.join(tpt, 'out.txt')
            
            for f in tmpddir:
                if 'Output' in f or 'MusicOutDir' in f:
                    status = 'COMPLETED'
                    extcd="0"
                    break

            if extcd != "0":
                for f in sorted(tmpddir):
                    if "err.txt" in f:
                        with open(err_path, 'r') as h:
                            term="exception"
                            bflag=False
                            for line in h:
                                line.strip().split('/n')
                           #     print line
                                if term in line:
                                    bflag=True
                                    break
                            if bflag:
                                print("Again problem to copy the gridpack.Too many copies required. Put this job in list to RESUBMIT!!!!!!")
                                status= "RESUB"
                                extcd="1"
                                break

                            
                            if "Aborted" in h.read() or "aborted" in h.read():# or "Killed" in h.read() or "ERROR" in h.read() or "Error" in h.read():
                                if "return value 0" not in h.read():
                                    status = "RESUB"
                                    extcd="1"
                                    break
                                


                    if "out.txt" in f:
                        with open(out_path, 'r') as g:
                            if "Segmentation violation" in g.read():
                                print("music.cc has SEGMENTATION VAIOLATION for this job. Put this JOB on FAILED. Check music.cc or discard this sample.")
                                print(tmpk[job_id])
                                status= "FAILED"
                                break            
                            if "running music failed"  in g.read():
                                print("music.cc has FAILED for this job. Check the reason below.")
                                print(tmpk[job_id])
                                if "err.txt" in f:
                                    with open(err_path, 'r') as h:
                                        if "org.dcache.ftp.client.exception" in h.read():
                                            #print("Again problem with the dcach. Put this job in list to RESUBMIT!!!!!!")
                                            #print(tmpk[job_id])
                                            status= "RESUB"
                                            extcd="1"
                                            break
                                print("Reason unknown. At the moment Put this JOB on RESUBMIT. Check music.cc or discard this sample.")
                                status= "RESUB"
                                extcd="1"
                                break

                    if "log.txt" in f:
                        with open(log_path, 'r') as g:
                            if "by user" in g.read():
                                status = "RESUB"
                                extcd="1"
                                break
                            if "ResidentSetSize" in g.read():
                                status = "FAILED"
                                #print("The job is FAILED due to less RequestMemory. Please increase RequestMemory into ce.py, or reduce the fileperjobs in your .yaml file")
                                #print(tmpk[job_id])
                                break
                            if "aborted" in g.read() or "return value 1" in g.read() or "return value 2" in g.read():
                                status = "RESUB"
                                extcd="1"
                                break
                            elif "return value 0" in g.read():
                                status = "RUNNING"
                                break
                            else:
                                status = "FAILED"
                                break
                    
        result[job_id] = dict()
        result[job_id]['history'] = list()
        result[job_id]['history'].append([status, timesst])
        result[job_id]['Status'] = status
        result[job_id]['ExitCode'] = extcd
    
        # Yannik commented the following part out
    '''
        try:
            key, value = line.split('=', 1)
        except ValueError:
            continue
        if 'Command' in key: continue
        key, value = key.strip('\t* '), value.strip()[1:-1]
        if key == 'JobID':
            job_id = value
            result[job_id] = dict()
            result[job_id]['history'] = list()
        elif key == 'Status':
            status = value.split()[0][0:-1]
            timestamp = value.split()[-1][1:]
            result[job_id]['history'].append([status, timestamp])
            result[job_id]['Status'] = status
        else:
            result[job_id][key] = value
    '''
        # **** END ****
    return result


## Yield successive n-sized chunks from list
#
# Last chunk may be smaller, if the list cannot be divded by n
#
# @param l List to divide
# @param n Size of chunks
def chunks(l, n):
    for i in xrange(0, len(l), n):
        yield l[i:i+n]


## Task managing multiple jobs
class Task(TaskSettings):

    ## Default constructor
    def __init__(self, name, directory=None, mode='RECREATE',
                 scram_arch=None, cmssw_version=None,
                 storage_element=None,
                 job_max_runtime=None):
                 #delegation_id=None):
        # call inherited constructor; initializes empty variables
        super(Task, self).__init__()

        # Task specific settings
        # general
        self.name = name
        self.directory = os.path.abspath(directory if directory else name)

        # environment
        self.mode = mode
        self.scram_arch = scram_arch if scram_arch is not None else os.environ.get('SCRAM_ARCH')
        self.cmssw_version = cmssw_version if cmssw_version else os.environ.get('CMSSW_VERSION')
        # storage element is lazy loaded if not set
        self._storage_element = storage_element
        # 2880min = 48h, default for standard queues
        self.job_max_runtime = job_max_runtime if job_max_runtime else 2880
        #self.delegation_id = delegation_id
        self.jobs = []
        self.gridpacks = []
        self.output_dcache = []
        # status
        self.frontendstatus = ''


    ## Save the Task object as a pkl file
    #
    # @param self Object pointer
    def save(self):
        log.debug('Save task {0}'.format(self.name))
        # writing Task object as pkl
        with open(os.path.join(self.directory, 'task.pkl'), 'wb') as f:
            pickle.dump(self, f)
        # writing job ids to txt file
        with open(os.path.join(self.directory, 'job_ids.txt'), 'w') as f:
            for job in self.jobs:
                try:
                    f.write(str(job.job_id)+"\n")
                except (AttributeError, TypeError):
                    f.write("None\n")

    @property
    def storage_element(self):
        if self._storage_element is None:
            self._storage_element = gridlib.se.StorageElement('T2_DE_RWTH')
        return self._storage_element

    @storage_element.setter
    def storage_element(self, storage_element):
        self._storage_element = storage_element
    ## Construct a Task object based on an existing pkl file
    #
    # @param cls The class itself
    # @param directory Directory with the task.pkl file to be loaded
    @classmethod
    def load(cls, directory):
        with open(os.path.join(directory, 'task.pkl'), 'rb') as f:
            obj = pickle.load(f)
        obj.directory = os.path.abspath(directory)
        obj.mode = 'OPEN'
        return obj


    ## Create a new Job for this Task and return it
    #
    # The Jobs created by this function are automatically assigned the a CE ID
    # and have their Task's settings loaded.
    #
    # @param self Object pointer
    def create_job(self):
        # load ce_id from storage_element, if it has not been set (manually)
        #if not self.ce_id:
        #    self.ce_id = self.storage_element.get_ce_id(self.job_max_runtime)
        # create Job and propagate TaskSettings
        job = Job()
        job.load_settings(self.get_settings())
        # return latest of Task's Jobs
        self.jobs.append(job)
        return self.jobs[-1]


    ## Add a Job to the Task's list of jobs
    #
    # If load_settings is True, adding a Job to a Task also sets certain options
    # for the job (the one specified by TaskSettings). If one wants to change
    # these options, the changes should be done after adding the Job to the
    # Task.
    #
    # @param self Object pointer
    # @param job Job object which to add
    # @param load_settings If True, the Tasks settings are loaded into the Job
    def add_job(self, job, load_settings=False):
        # load ce_id from storage_element, if it has not been set (manually)
        #if not self.ce_id:
        #    self.ce_id = self.storage_element.get_ce_id(self.job_max_runtime)
        # propgate TaskSettings if requested
        if load_settings:
            job.load_settings(self.get_settings())
        self.jobs.append(job)


    ## Prepare and submit all jobs in the Class' joblist
    #
    # @param self Object pointer
    # @param processes Number of processes
    def submit(self, processes=0):
        log.info('Start to submit task {0}'.format(self.name))
        if not self.jobs:
            log.error('No jobs in task {0}'.format(self.name))
            return

        # setup working directory
        startdir = os.getcwd()
        self.create_working_dir()

        # ensure there is a proxy with sufficient time remaining
        gridlib.util.voms_proxy_ensure()

        # number jobs and create jdl files
        log.debug('Create {0} jdl file'.format(len(self.jobs)))
        for i in xrange(len(self.jobs)):
            self.jobs[i].node_id = i
            # create job specific folder
            os.mkdir(os.path.join(self.directory, 'grid-' + str(i)))
            self.jobs[i].job_directory = os.path.join(self.directory,'grid-' +  str(i))
            os.chdir(self.jobs[i].job_directory)
            # create prologue.sh
            log.debug('Make prologue {0}'.format(self.name))
            self.create_prologue()
            self.jobs[i].create_jdl()

        # run with specified number of processes
        self._apply_function(range(len(self.jobs)), processes, 'submit')

        # update status, return to startdir and save
        self.frontendstatus = 'SUBMITTED'
        os.chdir(startdir)
        log.debug('Task {0}  hase ben SUBMITTED'.format(self.name))
        self.save()


    ## Apply a function to the Task's jobs
    #
    # If the requested processes are above 1, multiprocessing is used.
    #
    # @param self Object pointer
    # @param node_ids IDs for each job to apply the funcion to
    # @param processes Number of processes used for applying functions
    # @param function Function which to apply to the jobs
    def _apply_function(self, node_ids, processes, func_name):
        jobs = [j for j in self.jobs if j.node_id in node_ids]
        func = functools.partial(job_apply, func_name)
        # do multiprocessing
        if processes > 1:
            pool = multiprocessing.Pool(processes)
            result = pool.map_async(func, jobs).get(0xFFFF)
            # result.wait()
            pool.close()
            pool.join()
            # while pool._cache:
            #     time.sleep(1)
            # argument jobs are pickled/copied -> replace jobs with returned jobs
            for job in result:
                self.jobs[job.node_id] = job
        # do sequential processing
        else:
            for job in jobs:
                job_apply(func_name, job)


    ## Create a lock file, indicating that this job being processed
    #
    # @param self Object pointer
    def _lock(self):
        open(os.path.join(self.directory, '.lock'), 'a').close()


    ## Unlock the job by removing the lock file
    #
    # @param self Object pointer
    def _unlock(self):
        try:
            os.remove(os.path.join(self.directory, '.lock'))
        except:
            return


    ## True if job locked, else False
    #
    # @param self Object pointer
    def _is_locked(self):
        return os.path.exists(self.directory+'/.lock')


    ## Resubmit jobs
    #
    # @param self Object pointer
    # @param node_ids Node IDs which to resubmit
    # @param processes Number of processes; Multiprocessing for values above 1
    @lock
    def resubmit(self, node_ids, processes=0):
        if len(node_ids) is 0:
            return
        log.info('Resubmit (some) jobs of task {0} ({1} jobs)'.format(self.name, len(node_ids)))
        self._apply_function(node_ids, processes, 'resubmit')
        self.frontendstatus = 'SUBMITTED'
        self.save()
        #self.cleanup() #Yannik commented it out


    ## Kill jobs
    #
    # @param self Object pointer
    # @param node_ids Node IDs which to kill
    # @param processes Number of processes; Multiprocessing for values above 1
    def kill(self, node_ids, processes=0):
        log.debug('Kill (some) jobs of task {0}'.format(self.name))
        self._apply_function(node_ids, processes, 'kill')
        self.save()
        #self.cleanup()#Yannik commented it out


    ## Set copy command for moving output to dcache
    #
    # Since the resultfileprefix and suffix are replaced/formatted within ce.py,
    # one has to escape curly brackets. To do that, a single curly bracket have
    # to become two, e.g. '{{hello}}' becomes '{hello}'.
    #
    # @param self Object pointer
    # @param outputfile Outputfile which to copy
    # @param lfn_path Logical file name path on the upload site
    def output_to_dcache(self,
                         outputfile,
                         lfn_path='store/user/${{CESUBMITUSERNAME}}/${{CESUBMITCREATEDATE}}/${{CESUBMITTASKNAME}}/{resultfileprefix}-${{CESUBMITNODEID}}_${{CESUBMITRUNID}}.{resultfilesuffix}',
                         srm_prefix=None):
        # upload to a specific site
        if srm_prefix:
            path = os.path.join(srm_prefix, lfn_path)
        # upload to default site
        else:
            path = self.storage_element.get_srm_path(lfn_path)
        self.output_dcache.append([outputfile, path])


    ## Add the gridpack to the task
    #
    # @param self Object pointer
    # @param lfn_path Logical file name of/path to the file on the grid
    # @param extract_dir Directory to which to extract the gridpack, once retrieved
    # @param username Name of the user in whose grid directory the path is
    def add_gridpack(self, lfn_path, extract_dir='./'):
        self.gridpacks.append([self.storage_element.get_srm_path(lfn_path), extract_dir])



    ## Create the proglue shell script
    #
    # @param self Object pointer
    def create_prologue(self):
        # general job information header
        executable = textwrap.dedent(
            '''\
            #!/bin/sh -e
            set -o pipefail
            echo Job started: $(date)
            CESUBMITTASKNAME={0}
            CESUBMITCREATEDATE={1}
            CESUBMITCREATEDATETIME={2}
            CESUBMITUSERNAME={3}
            CESUBMITNODEID=$1
            rchars=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789; CESUBMITRUNID=; for i in {{1..4}}; do CESUBMITRUNID=$CESUBMITRUNID${{rchars:$(($RANDOM % 62)):1}}; done
            shift
            RUNAREA=$(pwd)
            echo Running in: $RUNAREA
            echo Running on: $HOSTNAME
            export LCG_GFAL_INFOSYS="grid-bdii.desy.de:2170,lcg-bdii.cern.ch:2170"
            export MYPROXY_SERVER="grid-px0.desy.de"
            export SITE_NAME="RWTH-Aachen"
            export VO_CMS_DEFAULT_SE="grid-srm.physik.rwth-aachen.de"
            export VO_CMS_SW_DIR="/cvmfs/cms.cern.ch"
            export VO_DTEAM_DEFAULT_SE="grid-srm.physik.rwth-aachen.de"
            export VO_OPS_DEFAULT_SE="grid-srm.physik.rwth-aachen.de"
            export X509_CERT_DIR="/etc/grid-security/certificates"
            export http_proxy="http://grid-squid.physik.rwth-aachen.de:3128"
            export no_proxy=".rwth-aachen.de"
            lsb_release  -i -r
            '''.format(self.name,
                       time.strftime('%Y-%m-%d'),
                       time.strftime('%Y-%m-%d-%H-%M-%S'),
                       gridlib.util.get_username_cern())
        )
        # add cmssw sourcing procedure
        ####TEMP FIXING FOR GRID FROM LCG_GFAL_INFOSYS TO no_proxy
        if self.cmssw_version:
            executable += textwrap.dedent(
                '''\
                echo Setting SCRAM_ARCH to {0}
                
                export SCRAM_ARCH={0}
                export BUILD_ARCH=$SCRAM_ARCH
                echo VO_CMS_SW_DIR $VO_CMS_SW_DIR
                source $VO_CMS_SW_DIR/cmsset_default.sh
                
                scram project CMSSW {1}
                cd {1}
                eval $(scramv1 ru -sh)
                cd $RUNAREA
                
                '''.format(self.scram_arch,
                           self.cmssw_version)
            )
        cmssw_build=False
        # add gridpack retrieval and extraction
        #LOR MADE SOME CHANGES BELOW ONLY FOR CLASSIFICATION. REMOVE THEM FOR SCANS:
        #I added /$TMP/ before {1} to tar xf and rm lines.
        #tar xf /$TMP/{1} -C {2}
                   # rm /$TMP/{1}
        #
        if self.gridpacks:
            executable += 'echo Retrieving grid packs\n'
            executable += 'echo Temp $TMP\n'
            executable += 'echo Vo_CMS_SW_DIR $VO_CMS_SW_DIR\n'
            for inurl, outpath in self.gridpacks:
                #LOR TRY TO HACK THE ERROR WITH ACCESSING THE GRIDPAKS ON DCACHE
                if("Data" in self.name or "data" in self.name):
                    j=random.randint(1,200)
                    m=str(j)
                    inurl=inurl.replace("/gridpack","/testData")
                    head, tail = inurl.split("/testData")
                    head= head +"/testData" + m + ".tar.bz2"
                    inurl = head
                #   print(inurl)
                else: #MC
                    j=random.randint(1,200)
                    m=str(j)
                    p1=inurl
                    inurl=inurl.replace("/gridpack","/test")
                    head, tail = inurl.split("/test")
                    head= head +"/test" + m + ".tar.bz2"
                    inurl = head
                    #midcmd="srmls " + head
                    #if(os.system(midcmd) != 0):
                    fnlcmd = "srmcp " + p1 + " " + head                    
                    os.system(fnlcmd)
                #    print(inurl)
                #END HACKING
                executable += textwrap.dedent(
                    '''\
                    #unset LD_LIBRARY_PATH;
                    #source /cvmfs/grid.cern.ch/umd-c7ui-latest/etc/profile.d/setup-c7-ui-example.sh;
                    srmcp {0} file://$TMP/{1}
                    #srmcp {0} {1}
                    #gfal-copy {0} {1}
                    mkdir -p {2}
                    #tar xf {1} -C {2}
                    #rm {1}
                    tar xf /$TMP/{1} -C {2}
                    rm /$TMP/{1}                                    
                    '''.format(inurl, os.path.basename(inurl), outpath)
                )
                if 'CMSSW_BASE' in outpath:
                    cmssw_build = True
        if cmssw_build:
            executable += textwrap.dedent(
                '''\
                cd $CMSSW_BASE
                scram b
                cd $RUNAREA
                '''
            )
        executable += textwrap.dedent(
            '''\
            env
            echo Current directory $PWD
            echo Directory content:
            ls
            echo $@
            chmod u+x $1
            echo Executing $@
            echo ================= Start output =================
            { $@ 2>&1 1>&3 3>&- | tail -c 2M; } 3>&1 1>&2 | tail -c 3M
            echo ================== End output ==================
            echo Current directory $PWD
            echo Directory content:
            ls
            '''
        )
        # add copy procedure for output to dcache
        if self.output_dcache:
            executable += 'echo Copying files to dcache\n'
            for infile, outurl in self.output_dcache:
                # command = 'gfal-copy file:{0} {1}'.format(infile, outurl)
                command = 'srmcp file://./{0} {1}'.format(infile, outurl)
                infileparts = os.path.basename(infile).rsplit('.', 1)
                command = command.format(resultfileprefix=infileparts[0], resultfilesuffix=infileparts[-1])
                executable += command + '\n'

        # finalize and write the prologue.sh file
        executable += 'echo Job ended: $(date)\n'
        f = open('prologue.sh', 'w')
        f.write(executable)
        f.close()


    ## Create the tasks working directory
    #
    # @param self Object pointer
    def create_working_dir(self):
        if os.path.exists(self.directory) and self.mode != 'RECREATE':
            raise Exception('Directory {0} already exists'.format(self.directory))
        elif os.path.exists(self.directory):
            shutil.rmtree(self.directory)
        os.makedirs(self.directory)


    ## Get the status for all jobs of the Task
    #
    # @param self Object pointer
    # @param joblist List of jobs for which to get their status
    # @param retry Number of retries
    def _update_status_of_jobs(self, joblist=None, retry=0):
        # gather jobs which are not yet finished
        active_jobs = []
        for job in joblist if joblist is not None else self.jobs:
            if job.frontendstatus not in ['RETRIEVED', 'PURGED'] and job.status not in ['COMPLETED']:# Every DONE-OK is changed with DONE# Yannik changed
                active_jobs.append(job)

        # loop over chunks of jobs
        job_chunks = list(chunks(active_jobs, 50))
        updated_jobs = 0
        retry_jobs = []
        for job_chunk in job_chunks:
            # ensure there are job_chunk to check
            job_ids = [job.job_id for job in job_chunk if job.job_id is not None]
            dir_temp = [job.job_directory for job in job_chunk if job.job_id is not None] #Yannik added it
            if not job_ids:
                return 0
            # execute job-status command
            
            #command = ['glite-ce-job-status', '-L1'] #Yannik commented it out
            command = ['condor_q', '-grid']
            command.extend(job_ids)
            process = subprocess.Popen(command,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE)
            stdout, stderr = process.communicate()
            # if status retrieval failed, add jobs to retry queue
            if process.returncode is not 0:
                retry_jobs.extend(job_chunk)
                log.warning('Retrying status retrieval for task {0}:\n{1}\n{2}'.format(
                    self.name,
                    stdout,
                    stderr
                ))
            else:
                result = parse_status_multiple_l1(stdout,job_ids, dir_temp)
                # update infos for each Job
                valid_jobs = 0
                for job in job_chunk:
                    if job.job_id in result and len(result[job.job_id]) > 0:
                        job.infos = result[job.job_id]
                        valid_jobs += 1
                    else:
                        log.warning('Failed to get status of job {0} of task {1}',job.job_id, self.name)
                        if job.job_id is not None:
                            retry_jobs.append(job)
                log.debug('Got status for {0} jobs'.format(valid_jobs))
                updated_jobs += len(job_ids)

        # retry failed Jobs if retry limit has not been reached
        if retry_jobs:
            if retry < 5:
                # retry limit not reached
                # reduce the chunk size, ensure its still an integer number and retry
                job_chunks = list(chunks(list(retry_jobs), int(math.ceil(min(len(retry_jobs), 50) / (retry + 2)))))
                for job_chunk in job_chunks:
                    updated_jobs += self._update_status_of_jobs(job_chunk, retry + 1)

            else:
                # retry limit reached, log and skip jobs
                log.info('Job status retrieval not successful after 5 attempts. Will not retry.')
                log.debug('Failed to retrieve status for job IDs: {0}'.format(
                    ' '.join([job.job_id for job in retry_jobs])
                ))

        return updated_jobs


    ## Get the status of this Task
    #
    # The status of a Task is based on the status of its jobs
    #
    # @param self Object pointer
    @lock
    def get_status(self):
        log.debug('Get status of task {0}'.format(self.name))
        num_jobs = self._update_status_of_jobs()
        oldfestatus = self.frontendstatus
        retrieved, done, purged = True, True, True
        running = False
        # loop over all jobs to determine the overall status from the Jobs' status
        for job in self.jobs:
            # if Jobs are not yet retrieved, purged or done
            # the entire Task is not either
            if job.frontendstatus != 'RETRIEVED':
                retrieved = False
            if job.frontendstatus != 'PURGED':
                purged = False
            if "COMPLETED" not in job.status: #Yannik change from status DONE
                done = False
            # if Jobs are still running, the entire Task is still running
            if 'RUNNING' in job.status or 'RESUB' in job.status:
                running = True
                break
        # update frontentstatus to result of loop
        if running: self.frontendstatus='RUNNING'
        elif retrieved: self.frontendstatus='RETRIEVED'
        elif done: self.frontendstatus='COMPLETED' #Yannik changed from status DONE
        elif purged: self.frontendstatus='PURGED'
        # update and save Task if something has changed
        if num_jobs > 0 or oldfestatus != self.frontendstatus:
            self.save()
        return self.frontendstatus


    ## Retrieve the output
    #
    # @param self Object pointer
    # @param connections Number of connections
    # @param joblist Jobs which to retrieve; Defaults to all not yet retrieved jobs
    # @param retry Number of retry attempts
    @lock
    def retrieve_output(self, connections=1, joblist=None, retry=0):
        # gather finished jobs, but not yet retrieved jobs
        a = False
        done_jobs = []
        for job in joblist if joblist is not None else self.jobs:
            #if job.status == 'DONE-OK' and job.frontendstatus not in ['RETRIEVED', 'PURGED']:
            if 'ExitCode' in job.infos:
                if job.infos['ExitCode'] == '0':
                    a = True
            if a and job.status == 'COMPLETED' and job.frontendstatus not in ['RETRIEVED', 'PURGED']: #Yannik add this and commented 1 line above
                done_jobs.append(job)
        if not done_jobs:
            return

        # loop over chunks of jobs
        job_chunks = list(chunks(done_jobs, 50))
        log.info('Get output of {0} jobs of task {1}'.format(len(done_jobs), self.name))
        retry_jobs = []
        for job_chunk in job_chunks:
            job_ids = [job.job_id for job in job_chunk if job.job_id is not None]
            #command = ['glite-ce-job-output', '-s', str(connections), '--noint', '--dir', self.directory] + job_ids #Yannik commented it out
            command = ['condor_q', '-grid']
            command.extend(job_ids) # Yannik added this line
            process = subprocess.Popen(command, stdout=subprocess.PIPE)
            stdout, stderr = process.communicate()

            # retry if retrieval failed
            if process.returncode != 0:
                log.warning('Output retrieval failed for task {0}:\n{1}\n{2}'.format(self.name, stdout, stderr))
                for job in job_chunk:
                    retry_jobs.append(job)
                continue

            # process output
            # DOING A STUPID THING HERE. TO WORK WITH CONDOR. NEEDS TO BE IMPLEMENTED BETTER !!!!
            # The problem is that with glite you should retrieve stuff. With CONDOR no more. So iti is better to delete where this retrieve (and others) are called
            #successful_job_ids = parse_output(stdout) #Yannik commented it out
            successful_job_ids = job_ids
            
            log.info('Retrieved {0} jobs for {1}'.format(len(successful_job_ids), self.name))
            for job in job_chunk:
                if job.job_id in successful_job_ids:
                    #job.purge() #Yannik commented it out
                    job.frontendstatus = 'RETRIEVED'
                    log.debug('Successfully retrieved job {0}'.format(job.job_id))
                else:
                    job.frontendstatus = 'FAILED2RETRIEVE'
                    log.warning('Failed to retrieve job {0}'.format(job.job_id))
        if retry_jobs:
            if retry < 5:
                # reduce the chunk size, ensure its still an integer number and retry
                job_chunks = list(chunks(list(retry_jobs), math.ceil(min(len(retryjobs), 50) / (retry + 2))))
                for job_chunk in job_chunks:
                    self._unlock()
                    self.retrieve_output(connections = 1, joblist = job_chunk, retry = retry + 1)
            else:
                # retry limit reached, log and skip jobs
                log.info('Output retrieval not successful after 5 attempts. Will not retry.')
                log.debug('Failed to retrieve output for job IDs: {0}'.format(
                    ' '.join([job.job_id for job in retry_jobs])
                ))
        self.save()

    ## Get a dictionary of job status counts
    #
    # @param self Object pointer
    def job_status_dict(self):
        # using a default dict since keysets are homogeneous
        status_dict = collections.defaultdict(int)
        good, bad = 0, 0

        for job in self.jobs:
            try:
                # increment each individual status
                status_dict[job.status] += 1
                status_dict[job.frontendstatus] += 1
                # increment failed status
                if job.status in ['ABORTED', 'FAILED']: #Yannik changed from DONE-FAILED
                    bad += 1
                # determine success and increment
                if job.status == 'COMPLETED': #Yannik changed from DONE
                    if 'ExitCode' in job.infos:
                        if job.infos['ExitCode'] == '0':
                            good += 1
                        else:
                            bad += 1
            except AttributeError:
                pass

        status_dict['total'] = len(self.jobs)
        status_dict['good'] = good
        status_dict['bad'] = bad
        return status_dict


    ## Clean up the Task directory
    #
    # @param self Object pointer
    def cleanup(self):
        log.debug('Cleaning up task {0}'.format(self.name))
        job_dirs = [job.output_directory for job in self.jobs if job.job_id is not None]
        # ensure there is a backup directory
        if not os.path.exists(os.path.join(self.directory, 'bak')):
            os.mkdir(os.path.join(self.directory, 'bak'))
        # move all directories not belonging to tasks to bak
        for item in glob.glob(os.path.join(self.directory, '*')):
            if os.path.isdir(item) and os.path.basename(item) != 'bak':
                if os.path.basename(item) not in job_dirs:
                    shutil.move(item, os.path.join(self.directory, 'bak'))



if __name__ == '__main__':
    print 'This is a library'

    ## BEGIN Enable logging ##
    # logging.basicConfig(level=logging.INFO)
    ## END ##

    ## BEGIN Proxy delegation tests ##
    # se = gridlib.se.StorageElement('T2_DE_DESY')
    # proxydel = ProxyDelegator(se)
    # print 'Delegation id:', proxydel.get_delegation_id()
    ## END ##

    ## BEGIN TaskSettings tests ##
    # taskset = TaskSettings()
    # taskset.name = 'pookerface'
    # settings = taskset.get_settings()
    # print '         Task settings:', taskset.get_settings()
    # settings['executable'] = 'pookerface.exe'
    # taskset.load_settings(settings)
    # print 'Modified task settings:', taskset.get_settings()
    ## END ##


    # Try Task submission also with T2_DE_DESY and the following lfn path
    # 'store/user/vkutzner/PxlSkim/CMSSW_7_4_v3.1/DM_PseudoscalarWHLL_Mphi-200_Mchi-100_gSM-1p0_gDM-1p0_13TeV_JG/DM_PseudoscalarWHLL_Mphi-200_Mchi-100_gSM-1p0_gDM-1p0_13TeV-JHUGen/74X_mcRun2_asymptotic_realisticBS_v1/160202_155122/0000/DM_PseudoscalarWHLL_Mphi-200_Mchi-100_gSM-1p0_gDM-1p0_13TeV_JG_3.pxlio'
    ## BEGIN Task submission test ##
    # se = gridlib.se.StorageElement('T2_DE_RWTH')
    # task = Task('pookerface', storage_element = se)
    # task.add_gridpack('gridpacks/gridpack.tar.bz2')
    # task.executable = 'gridwrapper.sh'
    # task.executable_upload = False
    # task.outputfiles.append('AnalysisOutput.tar.bz2')

    # for lfn in ['store/user/radziej/PxlSkim/CMSSW_7_6_v1.0/ZToMuMu_M_120_200_13TeV_PH/ZToMuMu_NNPDF30_13TeV-powheg_M_120_200/76X_mcRun2_asymptotic_v12/160120_111540/0000/ZToMuMu_M_120_200_13TeV_PH_4.pxlio',
    #             'store/user/radziej/PxlSkim/CMSSW_7_6_v1.0/ZToMuMu_M_120_200_13TeV_PH/ZToMuMu_NNPDF30_13TeV-powheg_M_120_200/76X_mcRun2_asymptotic_v12/160120_111540/0000/ZToMuMu_M_120_200_13TeV_PH_1.pxlio']:
    #     # get a preconfigured job from the task
    #     job = task.create_job()
    #     # sample specific arguments + files / events per job option + files
    #     job.arguments.append('LEDge/cfg/mc.cfg')
    #     job.arguments.append(se.get_site_path(lfn))

    # task.submit()
    # print 'submitted'
    # time.sleep(10)
    # print 'Status ', task.get_status()
    # task_submission_test_rwth()
    # task_monitoring_test_rwth()
    # gr8 success!
    ## END ##


    ## BEGIN Task monitoring test ##
    # task = Task.load('pookerface')
    # print 'Status:', task.get_status()
    # task.retrieve_output()
    ## END ##

    sys.exit(0)
