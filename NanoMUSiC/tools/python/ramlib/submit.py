#!/usr/bin/env python
# Copyright [2015] <III Phys. Inst A, RWTH Aachen University>

# standard library imports
import sys
import os
import glob
import yaml
import subprocess
import logging

# setup logging
log = logging.getLogger(__name__)

# 3a imports
import gridlib.se
import gridlib.ce
import gridlib.util

# ramlib imports
import ramutils

## Submitter class managing the job submission to the grid
#
# This class is creating jobs from the provided config and the information
# stored in the database. It also composes the files necessary to run the job,
# including the gridpack, and forwards them to the gridlib.
class Submitter(gridlib.ce.ProxyDelegator, gridlib.se.GridpackManager):
    ## The constructor
    #
    # @param self Object pointer
    # @param args Command line arguments
    def __init__(self, args):

        # call inherited constructor
        super(Submitter, self).__init__(gridlib.se.StorageElement(args.site))
        # GridpackManager has identical constructor; not calling that

        # most important inherited variables/methods:
        # ProxyDelegator:
        # - self.storage_element
        # - self.get_delegation_id()
        # GridpackManager:
        # - self.gridpack_prepare(...)

        # load the settings and samples
        self.settings, self.samplecfg = ramutils.load_config(args.config)

        # set command line options
        self.outputdirectory = args.directory
        self.testmode = args.test

        # sample information from aix3adb
        self.samplelist = []


    ## Class representation as a string
    #
    # @param self Object pointer
    def __str__(self):
        s = ('<Submitter instance>\n'
             + str(len(self.samplelist))
             + ' stored samples:\n')
        for sample in self.samplelist:
            s += sample.__str__()
        return s


    ## Prepare the gridpack
    #
    # @param self Object pointer
    def prepare_gridpack(self):
        print 'Preparing gridpack ...'
        self.gridpack_prepare(self.settings['gridpackbase'],
                              self.settings['gridpackfiles'],
                              self.settings['gridpacklocal'],
                              os.path.join('store/user', self.username_cern,
                                           self.settings['gridpackremote']))


    ## Prepare jobs using the aix3a database
    #
    # @param self Object pointer
    def prepare_jobs(self):
        self.samplelist = ramutils.get_samples(self.settings, self.samplecfg, self.testmode)


    ## Returns command line arguments defining the chunk size
    #
    # There are 3 cases to be considered in this function:
    #  - Testmode: Run over 100 events of a single file
    #  - Events per job: Split one file into jobs with fixed numbers of events
    #  - Files per job: Run over all events of a batch of files
    #
    # @param self Object pointer
    # @param sample AnalysisSample for which to determine arguments and chunks
    def build_arguments_and_chunks(self, sample):
        # retrieve file paths from skim
        files = sample.get_files()
        if not files:
            log.warning('No files listed in DB - Searching for files on dcache ...')
            # build path to date subdirectories
            path = os.path.join('store/user', sample.skim.owner, 'PxlSkim',
                                sample.skim.skimmer_version, sample.sample.name,
                                sample.skim.datasetpath.split('/')[1],
                                sample.skim.skimmer_globaltag)
            # find match for day of skim creation
            time_skim = int(sample.skim.createdat.translate(None, '-: ')[2::])
            for item in self.storage_element.ls(path):
                # we start at highest date; the first skimmed files older than
                # our db entry should belong to it
                if time_skim - int(item.translate(None, '_')) <= 0:
                    path = os.path.join(path, item)
                    continue
            files = self.storage_element.find(path, '*.pxlio')
            log.warning('Found {0} files in {1}'.format(len(files), path))

        # prefix lfn paths with the file access protocol to get pfn paths
        files = [self.storage_element.get_site_path(f) for f in files]
        # determine chunk splitting
        if self.testmode:
            arguments = [self.settings['maxeventsoption'], '100']
            chunks = [[files[0]]]
        elif self.settings['eventsperjob'] and not self.settings['filesperjob']:
            # determine chunks jobs by events per job
            # TODO(radziej)
            arguments = [self.settings['maxeventsoption'],
                         self.settings['eventsperjob'],
                         self.settings['skipeventsoption']]
            sys.exit(1)
        elif self.settings['filesperjob'] and not self.settings['eventsperjob']:
            # determine chunks jobs by files per job
            arguments = []
            # split files into bunches of size 'filesperjob' - last bunch might be smaller
            chunks = [files[x:x+int(self.settings['filesperjob'])]
                      for x in range(0, len(files), int(self.settings['filesperjob']))]
        else:
            raise Exception('Can\'t build arguments: Choose either events or files per job')
        return arguments, chunks


    ## Submit jobs to the grid
    #
    # @param self Object pointer
    def submit_jobs(self):
        for anasample in self.samplelist:
            # create task based on class instance information
            taskname = anasample.get_taskname()
            print 'Preparing task', taskname
            task = gridlib.ce.Task(
                name = taskname,
                directory = os.path.join(self.outputdirectory, anasample.samplegroup, taskname),
                scram_arch = self.settings['scram_arch'] if 'scram_arch' in self.settings else None,
                cmssw_version = self.settings['cmssw'] if self.settings['cmssw'] else None,
                storage_element = self.storage_element,
                job_max_runtime = 120 if self.testmode else None,
                #delegation_id = self.get_delegation_id()
            )

            # executable is part of the gridpack, no need for separate upload
            task.executable = os.path.expandvars(self.settings['executable'])
            task.executable_upload = False
            task.outputfiles.extend(self.settings['outputfiles'])
            # task.inputfiles.extend(self.inputfiles.split())
            task.add_gridpack(os.path.join('store/user', self.username_cern,
                                           self.settings['gridpackremote']))
            for dcache_outputfile in self.settings['outputfilesdcache']:
                task.output_to_dcache(dcache_outputfile)

            # get information on analysis job
            arguments, chunks = self.build_arguments_and_chunks(anasample)
            print 'Number of jobs', len(chunks)
            for chunk in chunks:
                # get preconfigured job from task
                job = task.create_job()
                # sample specific arguments
                job.arguments.append(anasample.samplearguments)
                # arguments and file chunks for job splitting
                job.arguments.extend(arguments)
                job.arguments.extend(chunk)
            # submit jobs
            print 'Submitting ...'
            task.submit(6)
            print 'Done'


if __name__=='__main__':
    print 'This is a library, dont run it.'
    sys.exit(0)
