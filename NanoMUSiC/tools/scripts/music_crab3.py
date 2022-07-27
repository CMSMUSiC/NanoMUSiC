#!/usr/bin/env python

## @package music_crab3
# MUSiC wrapper for crab3
#
# music_crab is a wrapper script that has been developed
# to submit the large number of tasks (more or less) automatically,
# needed to get all data and MC. The script automatically creates CRAB
# config files according to the dataset file provided.
# crab3 rebuilds the previous crab2 functionality and relies on the work
# in the previous version
# @author Tobias Pook

import datetime
import os, csv
import sys
import shutil
import time
import re
import logging
import argparse
import glob
import subprocess
import imp
import pickle
import fnmatch

#custom libs
from crabFunctions import *
import aachen3adb
import dbutilscms
import loggingFunctions
from gridlib import util
# some general definitions
COMMENT_CHAR = '#'

# create global logger
log = logging.getLogger( 'music_crab' )

# define some module-wide switches
runOnMC = False
runOnData = False
runOnGen = False

import FWCore.ParameterSet.Config as cms

def main():
    # get controller object for contacts with crab
    #~ controller =  CrabController(logger = log)
    controller =  CrabController(debug = 0)
    # Parse user input from command line
    options = commandline_parsing( controller )
    # Setup logging for music_crab
    loggingFunctions.setupLogging(log,
                                 options.debug,
                                 log_file_name = 'music_crab_' + options.isodatetime + '.log')
    # adjust options for CrabController
    controller.dry_run = options.dry_run
    log.info("Starting music_crab3")

    log.info( "Current user: " + gridlib.util.get_username_cern() )

    # first check if user has permission to write to selected site
    if not controller.checkwrite():sys.exit(1)

    # create a connection to aix3adb if necessary
    dblink = aachen3adb.ObjectDatabaseConnection(key=options.db_key,
                                                 certificate=options.db_cert)

    tasks = []
    start = time.time()

    for sampleFileName in options.submit_configs:
        # Read additional information from music_crab config file
        #get SampleFile from command line argument
        SampleFileInfoDict = readSampleFile( sampleFileName ,options)
        SampleDict =  SampleFileInfoDict['sampledict']
        SampleFileInfoDict['sampleFileName'] = sampleFileName

        # Check if the current commit is tagged or tag it otherwise
        if not options.noTag:
            try:
                gitTag = createTag( options )
                SampleFileInfoDict.update({'gitTag':gitTag})
            except Exception, e:
                log.error( " can not find tag" )
                log.error( e )
                sys.exit( 3 )
        else:
            SampleFileInfoDict.update( { 'gitTag':options.overrideTag } )

        log.info("Using skimmer repo tag: " + SampleFileInfoDict['gitTag'])

        # extract the global tag for the used config file
        globalTag =  getGlobalTag(options)
        log.info("Using global tag: %s" % globalTag)
        SampleFileInfoDict.update({'globalTag':globalTag})

        # create a connection to aix3adb if necessary
        dblink = aachen3adb.ObjectDatabaseConnection(key=options.db_key,
                                                     certificate=options.db_cert)

        # create crab config files and submit tasks
        for i,key in enumerate(SampleDict.keys()):
            # read in relevant information from sample dict
            log.info("Working on {0}".format(key))
            if runOnData:
                ( name, datasetpath, lumi_mask, lumisPerJob) = SampleDict[key]
                json_file = lumi_mask
            else:
                ( name, datasetpath ) = SampleDict[key]
                json_file = None

            # Create and write a crab config file with gathered information
            CrabConfig = createCrabConfig(SampleFileInfoDict,
                                          SampleDict[key],
                                          options)
            log.info("Created crab config object")
            try:
                configFileName = writeCrabConfig(key,CrabConfig,options)
            except IOError as e:
                log.exception( "Unable to write crab config" )
            # initalize crabTask from crabFunctions lib
            if options.dry_run:
                continue
            task = CrabTask( key ,
                             initUpdate = False,
                             dblink = dblink,
                             globalTag = globalTag,
                             datasetpath = datasetpath,
                             skimmer_version = SampleFileInfoDict['gitTag'],
                             json_file = json_file )
            task.crab_existing_skim = False
            task.crab_not_in_db_yet = False
            task.crab_submitted = False
            # this task already exists and can be skipped
            if task.inDB and not options.force:
                task.crab_existing_skim = True
                tasks.append(task)
                continue

            #task seems to exist but not in db yet
            if task.isCreated and not options.force:
                task.crab_not_in_db_yet = True
                tasks.append(task)
                continue

            # Delete folder and submit if --force option is used
            if os.path.isdir( task.crabFolder ) and options.force:
                shutil.rmtree( task.crabFolder )
            if not os.path.isdir( task.crabFolder ):
                log.info('Submitting the task: %s' %key)
                task.submit()
            else:
                log.info('Existing CRAB folder for tasks: %s not '\
                         'submitted (use force to submit anyway)' % key)
                tasks.append(task)
                task.crab_not_in_db_yet = True
                continue
            if i == 0:
                start = time.time()
            task.crab_submitted = True
            tasks.append(task)

    # Check results and show summary
    existing = [t for t in tasks if task.crab_existing_skim]
    if existing:
        log.info( "Some samples are skipped due to existing skims" )
        log.info( '{:<12} {:<12} {:<12} {:<90} '.format("Skim ID", "owner", "finished", "name"))
        for task in existing:
            log.info( '{:<12} {:<12} {:<12} {:<90} '.format( task.dbSkim.id,
                                                             task.dbSkim.owner,
                                                             task.dbSkim.finished,
                                                             task.name ) )

    db_missing = [t for t in tasks if task.crab_not_in_db_yet]
    if db_missing:
        log.info( "Some samples had folders but are not in the aachen3a-db. Either they are not finished or not retrieved to db." )
        log.info( "Trying to update the state of the task" )
        for task in db_missing:
            log.info( '{:<12} {:<90} '.format( task.state,
                                               task.name ) )
    
    
    # make sure at least 30 seconds passed since the first job was submitted
    timedelta = time.time() - start
    if timedelta < 30:
        timedelta = time.time() - start
        log.info( "Give crab at least %d seconds before we first check tasks" % int(timedelta))
        time.sleep(timedelta)

    submitted = [t for t in tasks if task.crab_submitted]
    if submitted:
        log.info( '{:<12} {:<90} '.format( "Skim ID", "Sample name" ) )
        for task in submitted:
            task.update()
            log.info( '{:<12} {:<90} '.format( task.dbSkim.id, task.name ) )
    
def createCrabConfig(SampleFileInfoDict, sampleinfo,options):
    global runOnMC
    global runOnData
    global runOnGen
    # Parse user input
    from crabConfigParser import CrabConfigParser
    config = CrabConfigParser()
    if runOnData:
        (name,sample,lumi_mask,lumisPerJob) = sampleinfo
    else:
        (name,sample) = sampleinfo

    ### General section
    config.add_section('General')
    config.set( 'General', 'requestName', name )
    if options.workingArea:
        config.set( 'General', 'workArea', options.workingArea )
    if options.transferOutputs:
        config.set( 'General', 'transferOutputs', 'True')
    if options.nolog:
        config.set( 'General', 'transferLogs', 'False' )
    else:
        config.set( 'General', 'transferLogs', 'True' )

    ### JobType section
    config.add_section('JobType')
    config.set( 'JobType', 'pluginName', 'Analysis' )
    config.set( 'JobType', 'psetName', SampleFileInfoDict['pset'] )
    config.set( 'JobType', 'allowUndistributedCMSSW', 'True' )


    if options.failureLimit:
        try:
            config.set( 'JobType', 'failureLimit', "%.2f"%float(options.failureLimit) )
        except:
            log.error('No failureLimit set. failureLimit needs float')
    # add name, datasetpath, globalTag (optional) and custom Params (optional)
    # arguments starting with '-' or '--' are not allowed, because they
    # are interpreted as cmsRun options
    paramlist = ['name=%s'%name,'datasetpath=%s'%sample]
    #~ if options.globalTag:
        #~ paramlist.extend(['--psglobalTag',options.globalTag])
    paramlist.extend(['globalTag=%s'%SampleFileInfoDict['globalTag']])
    if options.pyCfgParams:
        paramlist.append(options.pyCfgParams)
    config.set( 'JobType', 'pyCfgParams', paramlist )
    if options.inputFiles:
        config.set( 'JobType', 'inputFiles', options.inputFiles )
    if options.outputFiles:
        config.set( 'JobType', 'outputFiles', options.outputFiles )
    else:
        config.set( 'JobType', 'outputFiles', [name+".pxlio"]  )
    if options.allowUndistributedCMSSW:
        config.set( 'JobType', 'allowUndistributedCMSSW', 'True' )
    if options.maxmemory:
        try:
            config.set( 'JobType', 'maxmemory', "%d"%int(options.maxmemory ) )
        except:
            log.error('Option maxmemory not used. maxmemory needs integer')
    if options.maxJobRuntimeMin:
        try:
            config.set( 'JobType', 'maxJobRuntimeMin', "%d"%int(options.maxJobRuntimeMin ) )
        except:
            log.error('Option maxJobRuntimeMin not used. maxJobRuntimeMin needs integer')
    if options.numcores:
        try:
            config.set( 'JobType', 'numcores', "%d"%int(options.numcores ) )
        except:
            log.error('Option numcores not used. numcores needs integer')
            
    ### Dirty way to get L1 PrefiringMaps #LOR TRY
    files_to_copy=['L1PrefiringMaps.root']    
    for era in files_to_copy:         
        shutil.copyfile(os.path.abspath(os.environ['CMSSW_BASE']+"/src/PxlSkimmer/Skimming/data/"+era), os.path.abspath(os.path.join(options.workingArea,era)))     
    config.set( 'JobType','inputFiles', files_to_copy)

    ### Data section
    config.add_section('Data')
    config.set( 'Data', 'inputDataset', sample )
    config.set( 'Data', 'inputDBS', options.inputDBS)
    config.set( 'Data', 'publication', 'False' )
    config.set( 'Data','publishDBS','phys03')

    if runOnData:
        config.set( 'Data', 'splitting', 'EventAwareLumiBased' )
        config.set( 'Data', 'unitsPerJob', str(options.eventsPerJob) )
        if ".json" in lumi_mask:
            config.set( 'Data', 'lumiMask', os.path.join(options.lumi_dir , lumi_mask) )
        else:
            config.set( 'Data', 'lumiMask', SampleFileInfoDict['DCSOnly_json'] )
            config.set( 'Data', 'runRange', lumi_mask )
    else:
        config.set( 'Data', 'splitting', 'FileBased' )
        dasclient = dbutilscms.DasClient()
        DatasetSummary = dasclient.get_dataset_summary(sample, options.inputDBS)
        SampleFileInfoDict.update({'dasInfos':DatasetSummary})
        try:
            #~ print DatasetSummary
            filesPerJob =  int((float(options.eventsPerJob) * int(DatasetSummary['nfiles'])) /  int(DatasetSummary['nevents']) )
            if filesPerJob < 1:
                filesPerJob = 1
        except:
            log.error("events per job needs an integer")
            sys.exit(1)
        #LOR TRY TO INCLUDE Automatic splitting
        if options.splitOption == "Automatic":
            config.set( 'Data', 'splitting', 'Automatic' )
            config.set( 'Data', 'unitsPerJob', '180')
        else:
            config.set( 'Data', 'splitting', 'FileBased' )
            config.set( 'Data', 'unitsPerJob', '%d'%filesPerJob)

    if options.outLFNDirBase:
        outdir = os.path.join( '/store/user/', options.user, options.outLFNDirBase )
    else:
        outdir = os.path.join( '/store/user/', options.user, options.name, SampleFileInfoDict['gitTag'], name )
    config.set( 'Data', 'outLFNDirBase', outdir )

    ## set default for now, will change later
    config.set( 'Data', 'outputDatasetTag', SampleFileInfoDict['globalTag'] )
    if options.publish:
        config.set( 'Data', 'publication', 'True')
        # seems to be the only valid choice at the moment
        config.set( 'Data', 'publishDBS', 'phys03')

    if options.ignoreLocality:
        config.set( 'Data', 'ignoreLocality', 'True')

    ### Site section
    config.add_section('Site')
    # config.set( 'Site', 'storageSite', 'T2_DE_RWTH' )
    config.set( 'Site', 'storageSite', 'T2_CH_CERN' )

    if options.whitelist:
        whitelists = options.whitelist.split(',')
        config.set( 'Site', 'whitelist', whitelists )

    if options.blacklist:
        blacklists = options.blacklist.split(',')
        config.set( 'Site', 'blacklist', blacklists )


    ### User section
    config.add_section('User')
    #config.set( 'User', 'voGroup', options.proxy.voGroup )#LOR COMM MAY 2021

    return config

def writeCrabConfig(name,config,options):
    if options.workingArea:
        runPath = options.workingArea
        if not runPath.strip()[-1] == "/":
            runPath+="/"
    else:
        runPath ="./"
    # filename = '%s/crab_%s_cfg.py'%(runPath,name)
    filename = os.path.join( runPath, 'crab_%s_cfg.py' % name )
    #try:
    if "/" in name:
        raise ValueError( 'Sample contains "/" which is not allowed' )
    config.writeCrabConfig(filename)
    log.info( 'created crab config file %s'%filename )

    return filename

def readSampleFile(filename,options):
    global runOnMC
    global runOnData
    global runOnGen
    outdict = {}
    sampledict = {}
    afterConfig = False
    existing = []
    #check if only samples matching a certain pattern should be added
    if options.only:
    # 'PATTERNS' should be a comma separated list of strings.
    # Remove all empty patterns and those containing only whitespaces.
        options.only = options.only.split( ',' )
        options.only = filter( lambda x: x.strip(), options.only )
        log.debug( "Only submitting samples matching patterns '%s'." % ','.join( options.only ) )

    with open(filename,'rb') as sample_file:
        for line in sample_file:
            line = line.strip()
            if not line or line.startswith( COMMENT_CHAR ): continue
            if COMMENT_CHAR in line:
                line, comment = line.split( COMMENT_CHAR, 1 )
            if line.startswith( 'generator' ):
                generator = line.split( '=' )[1].strip()
                outdict.update({'generator':generator})
                runOnMC = True
            if line.startswith( 'maxJobRuntimeMin' ):
                generator = line.split( '=' )[1].strip()
                outdict.update({'maxJobRuntimeMin':options.maxJobRuntimeMin})
            if line.startswith( 'CME' ):
                energy = line.split( '=' )[1].strip()
                outdict.update({'energy':energy})
            if line.startswith( 'DCSOnly' ):
                DCSOnly_json = line.split( '=' )[1].strip()
                outdict.update({'DCSOnly_json':DCSOnly_json})
                # set a default
                outdict.update({'defaultUnitsPerJob':"20"})
            if line.startswith( 'defaultUnitsPerJob' ):
                defaultLumisPerJob= line.split( '=' )[1].strip()
                outdict.update({'defaultLumisPerJob':defaultLumisPerJob})
            if line.startswith( 'isData' ):
                runOnData = bool(line.split( '=' )[1].strip())
                runOnMC = not (runOnData)
            if line.startswith( 'config' ):
                (junk,pset) = line.split( '=' )
                pset = os.path.join( options.config_dir, pset.strip() )
                outdict.update({'pset':pset})
                afterConfig = True
            if afterConfig and not "config" in line:

                skip = False
                if options.only:
                    for pattern in options.only:
                        if fnmatch.fnmatchcase( line, pattern ):
                    # One of the patterns does match, no need to continue.
                            break
                        else:
                            # Found none matching, skip submission.
                            skip = True
                if skip:
                    log.debug( "Skipping sample '%s' (not matching any patterns)." % line )
                    continue
                #lumi-mask and lumis-per-job can be specified in the command line
                if ';' in line:
                    split_line = line.split( ';' )
                    first_part = split_line[ 0 ]
                    lumi_mask = split_line[ 1 ].strip()
                    if len( split_line ) > 2:
                        lumisPerJob = int( split_line[ 2 ] )
                    else:
                        lumisPerJob = options.eventsPerJob
                else:
                    first_part = line
                    lumi_mask = None
                try:
                    ( sample, datasetpath ) = first_part.split( ':' )
                except:
                    log.error("could not parse line: '%s'"%(first_part))
                    sys.exit(1)

                if runOnData:
                    sampledict.update({sample : ( sample, datasetpath, lumi_mask, lumisPerJob )})
                else:
                    sampledict.update({sample : ( sample, datasetpath )})

    # add sampledict to outdict
    outdict.update({'sampledict':sampledict})
    # overwrite pset if --config option is used
    if options.config:
        pset = options.config
        outdict.update({'pset':pset})

    if 'pset' in outdict:
        return outdict
    else:
        log.error( 'No CMSSW config file specified!' )
        log.error( 'Either add it to the sample file or add it to the command line.' )
        sys.exit(1)

def getGlobalTag(options):
    someCondition = False
    if options.globalTag is not None:
        globalTag =  options.globalTag
    elif someCondition:
        log.info("this is a place where Global tags will be defined during the run")
    else:
        if options.DefaultGlobalTag:
            #find default globalTag
            from Configuration.AlCa.autoCond import autoCond
            if runOnData:
                #~ globalTag = cms.string( autoCond[ 'com10' ] )
                globalTag =  autoCond[ 'com10' ]
            else:
                globalTag = autoCond[ 'startup' ]
        else:
            log.error( "Global tag not specified, aborting! Specify global tag or run with --DefaultGlobalTag (not trustworthy). " )
            exit(1)
    return globalTag

def createTag( options ):
    # Save the current working directory to get back here later.
    workdir = os.getcwd()

    def gitCheckRepo( skimmer_dir ):
        os.chdir( skimmer_dir )

        cmd = [ 'git', 'diff-index', '--name-only', 'HEAD' , '--' ]
        proc = subprocess.Popen( cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
        output = proc.communicate()[0]
        retcode = int( proc.returncode )

        if retcode != 0:
            log.warning( "Failed command: " + ' '.join( cmd ) )
            log.debug( 'Full output:\n' + output )
            return False
        else:
            if output:
                error  = "Repository in '%s' has uncommitted changes.\n" % skimmer_dir
                error += "It is strongly recommended that you commit your changes and rerun this script.\n"
                error += "If you know what you are doing, you can use the '--noTag' option to submit anyway!"
                log.error( error )
                return False
            return True

    if not gitCheckRepo( options.ana_dir ):
        raise RuntimeError( "git repository in '%s' dirty!" % options.ana_dir )

    # Call git to see if the commit is already tagged.
    cmd = [ 'git', 'log', '-1', '--pretty=%h%d', '--decorate=full' ]
    log.debug( 'Checking commit for tags: ' + ' '.join( cmd ) )
    proc = subprocess.Popen( cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
    output = proc.communicate()[0]

    # There should only be one line.
    line = output.splitlines()[0]

    success = False
    if not 'tags' in line:
        os.chdir( workdir )
        raise RuntimeError( "No tag found for repo in %s (did you checkout a branch and not a tag)?" % options.ana_dir )
    else:
        commit, line = line.split( ' ', 1 )
        info = line.split( ',' )
        head = info[0].strip( '() ' )
        branch = info[-1].strip( '() ' )

        tags = []

        for part in info:
            if 'tags/' in part:
                tags.append( part.strip( '() ' ).split( '/' )[-1] )
    if len(tags) < 1:
        log.error( "Found no tag for repo, exiting." )
        log.error( "Create tag or use --noTag option and try again" )
        raise RuntimeError( "No tag found for sample" % options.ana_dir )
    tag = tags[0]
    os.chdir( workdir )

    log.info( "Using Skimmer version located in '%s'." % options.ana_dir )

    if success:
        log.info( "Using Skimmer version tagged with '%s'." % tag )

    return tag

def commandline_parsing( parsingController ):
    ##parse user input
    ####################################
    # The following options
    # were already present in muic_crab
    ####################################
    skimmer_dir = os.path.join( os.environ[ 'CMSSW_BASE' ], 'src/PxlSkimmer/Skimming' )
    lumi_dir    = os.getcwd()
    config_dir  = os.path.join( skimmer_dir, 'test' )
    parser = argparse.ArgumentParser(description='Submit MUSiCSkimmer jobs for all submit configs')

    music_crabOpts = parser.add_argument_group('General music options')
    music_crabOpts.add_argument('-c',
                                '--config',
                                metavar='FILE',
                                help='Use FILE as CMSSW config file, instead of the one declared in DATASET_FILE.' \
                                     ' Correspond to crab3 JobType.psetName' )
    music_crabOpts.add_argument( '--ana-dir', metavar='ANADIR', default=skimmer_dir,
                       help='Directory containing the analysis. If set, ANADIR is used '\
                            'as the base directory for CONFDIR and LUMIDIR. [default: '\
                            '%(default)s]' )
    music_crabOpts.add_argument( '--config-dir', metavar='CONFDIR', default=config_dir,
                       help='Directory containing CMSSW configs. Overwrites input from '\
                            'ANADIR. [default: %(default)s]' )
    music_crabOpts.add_argument( '--lumi-dir', metavar='LUMIDIR', default=lumi_dir,
                       help='Directory containing luminosity-masks. Overwrites input '\
                            'from ANADIR. [default: %(default)s]' )
    music_crabOpts.add_argument( '-o', '--only', metavar='PATTERNS', default=None,
                       help='Only submit samples matching PATTERNS (bash-like ' \
                            'patterns only, comma separated values. ' \
                            'E.g. --only QCD* ). [default: %(default)s]' )
    music_crabOpts.add_argument( '-S', '--submit', action='store_true', default=False,
                       help='Force the submission of jobs, even if a CRAB task with the given process name already exists.' \
                            ' [default: %(default)s]' )
    music_crabOpts.add_argument( '--dry-run', action='store_true', default=False,
                        help='Do everything except calling CRAB or registering samples to the database.' )
    music_crabOpts.add_argument( '--debug', metavar='LEVEL', default='INFO', choices=loggingFunctions.LOG_CHOICES,
                        help='Set the debug level. Allowed values: ' + ', '.join( loggingFunctions.LOG_CHOICES ))
    music_crabOpts.add_argument( '--noTag', action='store_true', default=False,
                        help="Do not create a tag in the skimmer repository. [default: %(default)s]" )
    music_crabOpts.add_argument( '--overrideTag', default="noTag",
                        help="Same as noTag but with custom string replacement for the tag name. [default: %(default)s]" )

    music_crabOpts.add_argument( '-D', '--no-db', action='store_true',
                       help="Register all datasets at the database: 'https://cern.ch/aix3adb/'. [default: %(default)s]" )
    music_crabOpts.add_argument('--db-key', default='~/private/CERN_UserCertificate.key',
                       help="Key for authentication to 'https://cms-project-aachen3a-db.web.cern.ch'. [default: %(default)s]" )
    music_crabOpts.add_argument( '--db-cert', default='~/private/CERN_UserCertificate.pem',
                       help="Cert for authentication to 'https://cms-project-aachen3a-db.web.cern.ch'. [default: %(default)s]" )

    #///////////////////////////////
    #// new options since crab3
    #//////////////////////////////

    # new feature alternative username
    music_crabOpts.add_argument( '-u', '--user', metavar='USERNAME',
                                 help='Alternative username [default: HN-username]' )
    music_crabOpts.add_argument( '-g','--globalTag',
                                help='Override globalTag from pset')
    music_crabOpts.add_argument( '--DefaultGlobalTag',
                                 action='store_true',
                                 default=False,
        help='Allow submission without stating globalTag (use default)')
    music_crabOpts.add_argument( '--force',
                                 action='store_true',
                                 default=False,
                                 help='Delete existing crab folder and resubmit tasks')
    ###########################################
    # new  options for General section in pset
    ##########################################
    generalOpts = parser.add_argument_group("\n SECTION General - Options for crab3 config section General ")
    generalOpts.add_argument( '--workingArea',metavar='DIR',default=os.getcwd(),help='The area (full or relative path) where to create the CRAB project directory. '
                             'If the area doesn\'t exist, CRAB will try to create it using the mkdir command' \
                             ' (without -p option). Defaults to the current working directory.'       )
    generalOpts.add_argument( '-t', '--transferOutputs', action='store_true',default=True,
                                help='Whether to transfer the output to the storage site'
                                     'or leave it at the runtime site. (Not transferring the output might'\
                                     ' be useful for example to avoid filling up the storage area with'\
                                     ' useless files when the user is just doing some test.) ' )
    generalOpts.add_argument( '--nolog', action='store_true',default=False,
                                help='Whether or not to copy the cmsRun stdout /'\
                                     'stderr to the storage site. If set to False, the last 1 MB'\
                                     ' of each job are still available through the monitoring in '\
                                     'the job logs files and the full logs can be retrieved from the runtime site with')
    generalOpts.add_argument( '--failureLimit',
                                help='The number of jobs that may fail permanently before the entire task is cancelled. '\
                                      'Defaults to 10%% of the jobs in the task. ')
    ########################################
    # new options for JobType in pset
    ########################################
    jobTypeOpts = parser.add_argument_group( "\n SECTION JobType - Options for crab3 config section JobType ")
    jobTypeOpts.add_argument('--pyCfgParams',default =None,
        help="List of parameters to pass to the CMSSW parameter-set configuration file, as explained here. For example, if set to "\
             "[\'myOption\',\'param1=value1\',\'param2=value2\'], then the jobs will execute cmsRun JobType.psetName myOption param1=value1 param2=value2. ")
    jobTypeOpts.add_argument('--inputFiles',help='List of private input files needed by the jobs.', nargs='+', type=str)
    jobTypeOpts.add_argument('--outputFiles',help='List of output files that need to be collected, besides those already specified in the output'\
                                                ' modules or TFileService of the CMSSW parameter-set configuration file.  ')
    jobTypeOpts.add_argument( '--allowUndistributedCMSSW', action='store_true', default=False,
        help='Allow using a CMSSW release potentially not available at sites. [default: %(default)s]' )
    jobTypeOpts.add_argument('--maxmemory',help=' Maximum amount of memory (in MB) a job is allowed to use. ')
    jobTypeOpts.add_argument('--maxJobRuntimeMin',help="Overwrite the maxJobRuntimeMin if present in samplefile [default: 72] (set by crab)" )
    jobTypeOpts.add_argument('--numcores', help="Number of requested cores per job. [default: 1]" )
    jobTypeOpts.add_argument('--priority', help='Task priority among the user\'s own tasks. Higher priority tasks will be processed before lower priority.'\
                                                    ' Two tasks of equal priority will have their jobs start in an undefined order. The first five jobs in a'\
                                                    ' task are given a priority boost of 10. [default  10] ' )
    jobTypeOpts.add_argument('-n','--name', default="PxlSkim" ,
        help="Name for this analysis run (E.g. Skim Campaign Name) [default: %(default)s]")
    jobTypeOpts.add_argument('--publish',default = False,help="Switch to turn on publication of a processed sample [default: %(default)s]")

    ####################################
    # new options for Data in pset
    ####################################
    dataOpts = parser.add_argument_group( "\n SECTION Data - Options for crab3 config section Data")
    dataOpts.add_argument('--splitOption', default="FileBased",help="Job splitting option. You can choose Automatic or FileBased. [default: FileBased]")
    dataOpts.add_argument('--eventsPerJob', default=120000,help="Number of Events per Job for MC [default: %(default)s]")
    dataOpts.add_argument( '-d', '--inputDBS', metavar='inputDBS',default='global',
        help='Set DBS instance URL to use (e.g. for privately produced samples published in a local DBS).' )

    ####################################
    # new options for Site in pset
    ####################################
    siteOpts = parser.add_argument_group( "\n SECTION Site - Options for crab3 config section Site ")
    siteOpts.add_argument( '--outLFNDirBase', metavar='OUTLFNDIRBASE', default=None,
                       help="Set dCache directory for crab output to '/store/user/USERNAME/"\
                            "OUTLFNDIRBASE'. [default: 'store/user/USERNAME/PxlSkim/git-tag/']" )
    siteOpts.add_argument( '-w', '--whitelist', metavar='SITES', help="Whitelist SITES in a comma separated list, e.g. 'T2_DE_RWTH,T2_US_Purdue'." )
    siteOpts.add_argument( '-b', '--blacklist', metavar='SITES', help='Blacklist SITES in addition to T0,T1 separated by comma, e.g. T2_DE_RWTH,T2_US_Purdue  ' )
    siteOpts.add_argument('--ignoreLocality',action='store_true',default=False,help="Set to True to allow jobs to run at any site,"
                                                        "regardless of whether the dataset is located at that site or not. "\
                                                        "Remote file access is done using Xrootd. The parameters Site.whitelist"\
                                                        " and Site.blacklist are still respected. This parameter is useful to allow "\
                                                        "jobs to run on other sites when for example a dataset is available on only one "\
                                                        "or a few sites which are very busy with jobs. It is strongly recommended "\
                                                        "to provide a whitelist of sites physically close to the input dataset's host "\
                                                        "site. This helps reduce file access latency. [default: %(default)s]" )

    # we need to add the parser options from other modules
    #get crab command line options
    parsingController.commandlineOptions(parser)
    # add positional argument for crab config files
    parser.add_argument('submit_configs', nargs='+',
                        help='List of music_crab3 configs as produced with e.g. parseSampleList.py or dataset.py')
    # add ask passphrase for proxy validation
    parser.add_argument( '-p','--ask-passphrase', help='Prompt user for password to renew proxy')
    options  = parser.parse_args()
    now = datetime.datetime.now()
    isodatetime = now.strftime( "%Y-%m-%d_%H.%M.%S" )
    options.isodatetime = isodatetime

    # check if user has valid proxy
    import getpass
    proxy = gridlib.util.VomsProxy()
    expired = False
    min_lifetime = 300
    if proxy.timeleft < min_lifetime:
        print("Your proxy is valid for only %d seconds.")
        expired = True
    if options.ask_passphrase or expired:
        proxy.passphrase = getpass.getpass('Please enter your GRID pass phrase for proxy renewal:')
        proxy.ensure(min_lifetime)
    options.proxy = proxy

    #get current user HNname
    if not options.user:
        options.user = parsingController.checkusername()

    # use no-Tag option if overrideTag option is used
    if options.overrideTag != "noTag":
        options.noTag = True

    # Set CONFDIR and LUMIDIR relative to ANADIR if ANADIR is set
    # but the other two are not.
    if not options.ana_dir == skimmer_dir:
        # ANADIR was set (it is not at its default value).
        if options.lumi_dir == lumi_dir:
            # LUMIDIR was not set (it is at its default value).
            options.lumi_dir = os.path.join( options.ana_dir, 'test/lumi' )
        if options.config_dir == config_dir:
            # CONFDIR was not set (it is at its default value).
            options.config_dir = os.path.join( options.ana_dir, 'test/configs' )

    return options

if __name__ == '__main__':
    main()
