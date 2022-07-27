#!/bin/env python

from __future__ import print_function

# Built-In libraries.
import os
import sys
import time
import shutil
import math
import configargparser
import ConfigParser
import multiprocessing
import subprocess
import collections
import tarfile
import json
import copy
import logging
import traceback
import uuid
from contextlib import contextmanager
from pprint import pprint

import pickle

# 3rd-party libraries.
import numpy as np
import ROOT

# Custom libraries.
from gridlib import ce,se,util
from ramlib import ramutils #TAPAS

from BaseSubmitter import BaseSubmitter
from ectools.misc import rremove
from ectools.register import ecroot

# MUSiC-Utils
import roothelpers

logger = logging.getLogger("scan-submitter")

# Not allowing JSON nans because rapidjson (C++) cannot handle them.
JSON_DEFAULTS = dict( allow_nan=False, indent=2 )

def main( remote=True ):
    """Main function of the submit script."""
    # Get list of command line arguments.
    # Add unset options from config.
    options = commandline_and_config_parsing()

    roothelpers.root_setup()

    logger = logging.getLogger("main")

    # This makes references relative to the config file easier.
    if options.config:
        path = os.path.realpath( os.path.dirname( options.config ) )
        os.chdir( path )

    """ Create job inputs from event classes and submit them either local or remote """
    if not os.path.isfile( options.mc ):
        logger.error( "Input file %s does not exist.", options.mc )
        return

    if options.data and not os.path.isfile( options.data ):
        logger.error( "Data file %s does not exist.", options.data )
        return

    if options.signal and not os.path.isfile( options.signal ):
        logger.error( "Signal file %s does not exist.", options.signal )
        return

    # check if output dir exists if we run local
    if not options.remote and not os.path.isdir( options.out ):
        logger.info( "Creating local output directory" )
        os.makedirs( options.out )

    logger.info( "Reading event classes from root file %s.", options.mc )

    # Open the ROOT file and loop over all objects
    mc_file = ROOT.TFile.Open( options.mc )

    if options.data:
        data_file = ROOT.TFile.Open( options.data )
        options.Nrounds = 1
    else:
        data_file = None

    if options.signal:
        signal_file = ROOT.TFile.Open( options.signal )
        signal_names = [ key.GetName() for key in signal_file.GetListOfKeys() ]
        mc_names = [ key.GetName() for key in mc_file.GetListOfKeys() ]

        for mc_name in mc_names:
            if not mc_name in signal_names:
                err_msg = "Class %s only in mc file but missing in signal file" % mc_name
                logger.error ( err_msg )
                err_msg = "Maybe you forgot to merge background and signal "
                err_msg += "or merged with --filter option ?"
                logger.error ( err_msg )
    else:
        signal_file = None

    # Get a list of all class names.
    options.veto.append( "Rec_Empty*" )
    ec_names = ecroot.prepare_class_names( options, mc_file=mc_file )[ 0 ]

    # convert to list and sort class names
    ec_names = list(ec_names)
    ec_names.sort(key = lambda s: len(s))

    hash = uuid.uuid1().hex
    logger.info( "Submission hash: %r", hash )

    if options.gridpackname:
        options.gridpackname = options.gridpackname.replace( "HASH", hash )

    if not options.shifts_json:
        # create shifts.json
        options.shifts_json = os.path.join(os.getcwd(), "shifts.json" )
        systematics = collect_systematics( ec_names, mc_file, signal_file )
        shifts = create_systematic_shifts( systematics, options.Nrounds )
        with open( options.shifts_json, 'w' ) as file:
            json.dump( _flatten( shifts ), file, **JSON_DEFAULTS )

    submitter = Submitter( executable = options.executable,
                           gridpackname = options.gridpackname,
                           site = options.site,
                           scanner_base = options.scanner_base,
                           shifts_json = options.shifts_json,
                           remote = options.remote,
                           local_outdir = options.out )

    if options.continue_run and not options.remote:
        continue_local( options, submitter )
    else:
        create_and_submit( options,
                           ec_names,
                           mc_file,
                           data_file,
                           signal_file,
                           hash,
                           submitter )

    logger.info( "Finished." )

def create_and_submit( options,
                       ec_names,
                       mc_file,
                       data_file,
                       signal_file,
                       hash,
                       submitter ):
    logger = logging.getLogger("create_and_submit")

    if options.timing_json:
        logger.debug( "Loading timing JSONs from '%s'" % options.timing_json )
        with open( options.timing_json ,"rb") as timing_json:
            class_timing_dict = json.load( timing_json)
        default_time_per_round = np.percentile( class_timing_dict.values(), 80 )
        logger.info("default_time_per_round")
        logger.info(default_time_per_round)
    else:
        class_timing_dict = {}
        default_time_per_round = 3.

    default_width = {
        "SumPt" : 3,
        "InvMass" : 1,
        "MET" : 3 }

    if options.minRegionWidth < 1:
        options.minRegionWidth = default_width[ options.distribution ]

    scanner_config = {
        "minRegionWidth" : options.minRegionWidth,
        "coverageThreshold" : options.coverageThreshold,
        "sigmaThreshold" : options.sigmaThreshold,
        "integralScan" : options.integralScan,
        "skipLookupTable": options.skipLookupTable,
        "noLowStatsTreatment" : options.noLowStatsTreatment,
        "widthLowStatsRegions" : options.widthLowStatsRegions,
        "thresholdLowStatsDominant" : options.thresholdLowStatsDominant,
    }

    if options.seed:
        scanner_config["poissonSeed"] = options.seed

    job_factory = JobFactory( hash=hash,
            distribution=options.distribution,
            filter_systematics=options.filter_systematics,
            config=scanner_config )

    progress = ramutils.Progress( maximum=len( ec_names ), text='Submitting scan jobs for classes' )
    buncher = JsonBuncher( options, submitter, class_timing_dict, hash )

    timing_dict = collections.defaultdict( list )
    for iclass, name in enumerate( ec_names ) :

        key = mc_file.GetKey( name )
        logger.info( "Working event class '%s.", name )

        event_class = key.ReadObj()

        if options.distribution not in event_class.getDistTypes():
            logger.debug( "Distribution %s not valid for EC %s, skipping...", options.distribution, name )
            event_class.Delete()
            continue

        if event_class.getTotalEvents() <= 0:
            logger.warning( "Negative integral (<= 0) in event class '%s', skipping..." % name )
            event_class.Delete()
            continue

        # use default value if no timing information found for a class
        if not name in class_timing_dict:
            class_timing_dict[ name ] = default_time_per_round

        # determine runtime per dicing round (not! per dicing)
        runtime = class_timing_dict[ name ]

        start = time.time()

        # dataScan
        if data_file:
            logger.debug( "Matching data event class..." )
            data_key = data_file.GetKey( name )

            if data_key:
                data_event_class = data_key.ReadObj()
            else:
                logger.debug( "Event class '%s' not found in data.", name )
                data_event_class = None

            job = job_factory.prepare_data_job( event_class, data_event_class, empty = ( data_event_class is None ) )
            timing_dict["add_data_round"].append( time.time() - start )
            buncher.add_class( job, runtime )

            if data_key:
                data_event_class.Delete()

        # signalScan
        elif signal_file:
            logger.debug( "Matching signal event class..." )
            signal_key = signal_file.GetKey( name )
            if signal_key:
                signal_event_class = signal_key.ReadObj()

                job = job_factory.prepare_signal_job( event_class, signal_event_class )

                logger.debug( "Creating job copies for %d signal dicing rounds.", options.Nrounds )
                buncher.add_class( job, runtime )

                signal_event_class.Delete()

                timing_dict["add_signal_round"].append( time.time() - start )

            else:
                logger.debug( "Event class '%s' not found in signal sample.", name )

        # BG-only scan
        else:
            logger.debug( "Initializing BG job template..." )

            job = job_factory.prepare_mc_only_job( event_class )

            logger.debug( "Creating job copies for %d BG dicing rounds.", options.Nrounds )
            buncher.add_class( job, runtime )
            timing_dict["add_mc_round"].append( time.time() - start )

        progress.advance_to( iclass + 1 )
        progress.draw()

        event_class.Delete()

    # make sure that everything has been added to the submission queue
    buncher.submit()

    progress.end()

    # Close file now, no references exist anymore.
    mc_file.Close()
    if data_file:
        data_file.Close()

    with open( "scanner_timing.pkl", "wb" ) as pklfile:
        pickle.dump( timing_dict, pklfile )

    # send termination signal to workers

    # cross-check: find classes that are present only in data (not MC)
    data_ec_names = []
    if data_file:
        data_ec_names = ecroot.prepare_class_names( options, data_file=data_file )[ 0 ]

    for ec_name in data_ec_names:
        if not ec_name in ec_names:
            logger.warning( "Event class with data and without mc: %s" % ec_name )

def continue_local( options, submitter ):
    """Continue running on all unfunished remote jobs locally"""
    import glob

    logger = logging.getLogger("continue_local")
    folders = glob.glob( os.path.join( os.getcwd(), "jsonArch*") )
    folders = [ folder for folder in folders if os.path.isdir( folder ) ]

    for folder in folders:
        task = ce.Task.load( folder )
        task.get_status()
        if not task.frontendstatus == 'RETRIEVED':
            last_folder = os.path.basename(os.path.normpath(folder))
            for job in task.jobs:
                job.kill()
                job.purge()
            shutil.move( folder, folder + "_bk")

            filename = last_folder + ".tar.gz"

            # Locally execute archive
            submitter.submit( filename )

def collect_systematics( ec_names, mc_file, signal_file=None ):
    systematic_names = set()
    for ec_name in ec_names:
        for file in ( mc_file, signal_file ):
            if not file:
                continue
            key = file.GetKey( ec_name )
            event_class = key.ReadObj()
            ec_systematic_names = event_class.getSystematicNames()

            for ec_systematic_name in ec_systematic_names:
                ec_systematic_name = rremove( ec_systematic_name, "Up" )
                ec_systematic_name = rremove( ec_systematic_name, "Down" )
                systematic_names.add( ec_systematic_name )

            event_class.Delete()

    return systematic_names

def create_systematic_shifts( systematic_names, count=1 ):
    shifts = np.random.normal( loc=0.0, scale=1.0, size=( len( systematic_names ), count ) )
    dictionary = dict( zip( systematic_names, shifts ) )
    return dictionary

class JsonBuncher( object ):
    def __init__( self, options, submitter, timing_dict, hash ):
        self.options = options
        self.timing_dict = timing_dict
        self.hash = hash

        self._current_arch_runtime = 0.
        self._archive_count = 0
        self._dirty = False
        self._submitter = submitter
        self._split_dict = {}

        self._json_files = collections.defaultdict( list )

    def __del__( self ):
        if self._dirty:
            logger.warning( "%s destroyed before all jobs have been submitted.", self.__class__.__name__ )

    def submit( self ):
        """ Finalize current bunch: pack current json files into a tar-archive. """
        if self._dirty:
            # Construct archive
            archive_filename = "%s_%s_%d.tar.gz" % ( self.options.remote_json_arch, self.hash, self._archive_count )

            json_files = self._json_files[ self._archive_count ]

            if len( json_files ) > 0:
                logger.debug( "Creating JSON archive '%s' containing %d JSON file(s).",
                    archive_filename, len( json_files ) )

                if not self.options.continue_run:
                    self._create_json_archive( archive_filename, json_files )

                # Add the archive to the submission queue
                self._submitter.submit( archive_filename )

                logger.debug( "Added archive '%s' to submission queue.", archive_filename )

                self._archive_count += 1
            else:
                logger.debug( "Skipping submission, no json files provided." )

            # Increase/reset internal counters and flags
            self._current_arch_runtime = 0.
            self._json_files[ self._archive_count ] = []
            self._split_dict = {}

            self._dirty = False


    def add_class(self, job_template, runtime):
        start_round = 0
        json_file = self.dump_job_json( job_template )

        while start_round < self.options.Nrounds:
            remaining_rounds_in_arch = math.ceil( float( self.remaining_runtime ) / runtime )
            num_rounds = int( min( remaining_rounds_in_arch, self.options.Nrounds - start_round ) )
            first_round = int( start_round )

            if not json_file in self._json_files:
                self._json_files[ self._archive_count ].append( json_file )
            job_runtime = runtime * num_rounds

            self._split_dict[ job_template.ec_name ] = {"first_round":first_round,
                                                        "num_rounds":num_rounds,
                                                        "distribution":self.options.distribution}
            logger.debug( "Adding job for '%s' containing %d rounds,"\
                               "expected runtime: %d seconds.", job_template.ec_name,
                                                                 num_rounds,
                                                                 job_runtime )

            self._current_arch_runtime += job_runtime

            start_round += num_rounds

            self._dirty = True

            if self.remaining_runtime <= 0:
                logger.debug( "Reached max archive runtime: %g sec >= %g sec, creating archive.", \
                    self._current_arch_runtime, self.options.runtime_per_arch )
                self.submit()
            else:
                logger.debug( "Maximal archive runtime (%g sec) not reached yet (currently %g sec).", \
                    self.options.runtime_per_arch, self._current_arch_runtime )

        assert( start_round == self.options.Nrounds )

    def dump_job_json( self, job ):
        logger.debug( "Dumping job as JSON file(s) to local directory." )

        if not os.path.isdir(  "jsontemp" ):
            os.makedirs( "jsontemp" )
            logger.debug( "Directory jsontemp has been created." )

        json_filename = job.ec_name + "_" + job.distribution + "_Input.json"
        local_json_filename = os.path.join( "jsontemp", json_filename )
        job.dump_to_file( local_json_filename )

        return json_filename

    @property
    def archive_count( self ):
        return self._archive_count

    @property
    def runtime( self ):
        return self._current_arch_runtime

    @property
    def remaining_runtime( self ):
        return self.options.runtime_per_arch - self._current_arch_runtime

    def _create_json_archive( self, archive_name, file_list ):
        splitting_name = "splitting.json"
        splitting_path = os.path.join("jsontemp",splitting_name)
        with open( splitting_path , 'w' ) as jfile:
            json.dump( self._split_dict, jfile, **JSON_DEFAULTS )
        with tarfile.open( archive_name , 'w:gz') as mytar:
            for filename in file_list:
                local_json_path = os.path.join( "jsontemp", filename )
                mytar.add( local_json_path, filename )
                #os.remove( local_json_path )
            mytar.add( splitting_path, splitting_name )


class Submitter( object ):
    ''' This class is used to submit jobs'''
    def __init__( self,
            executable,
            gridpackname,
            site,
            scanner_base,
            shifts_json,
            remote = False,
            local_outdir = "output",
        ):

        self.logger = logging.getLogger( "Submitter" )

        self.executable = executable
        self.scanner_base = scanner_base
        self.shifts_json = shifts_json
        self.local_outdir = local_outdir
        self.remote = remote

        self.user = util.get_username_cern()

        if remote:
            self.logger.debug( "Preparing executable gridpack..." )

            gp_manager = BaseSubmitter( site )
            gridpack_remote = gp_manager.gridpack_prepare( scanner_base,
                                                ["python/cewrapper.py",
                                                 "bin/scanClass",
                                                 shifts_json,
                                                 "bin/lookuptable.bin"],
                                                 gridpackname )

            #self.logger.debug( "Creating BaseSubmitter, delegation id = %s" % str( gp_manager.delegation_id ) ) #Yannik commented it out
            print("************************ Base submitted delegation_ID scanSubmitter.py L508 ***************************") #Yannik add this
            #self.remote_submitter = BaseSubmitter( site,
             #                                      delegation_id=gp_manager.delegation_id,
              #                                     executable=executable,                         #Yannik commented it out
               #                                    executable_upload=False,
                #                                   gridpacks = [ gridpack_remote ] )
            self.remote_submitter = BaseSubmitter( site,
                                                   executable=executable, 
                                                   executable_upload=False,                        #Yannik add this
                                                   gridpacks = [ gridpack_remote ] )        
        else:
            self.remote_submitter = None


    def submit( self, json_arch, dryrun=False ):
        """ Wrapper function for both local and remote submission """
        name = os.path.basename( json_arch ).replace( ".tar.gz", "" )
        if self.remote:
            self.logger.debug( "Submitting archive %s as task %s" % ( json_arch, name ) )
            return self.submit_remote( name, json_arch, dryrun=False )
        elif not dryrun:
            self.logger.debug( "Executing archive %s as task %s" % ( json_arch, name ) )
            return self.submit_local( name, json_arch )

    def submit_remote( self, name, json_arch, dryrun=False, retry=10 ):
        print("************************ Submit Remote scanSubmitter.py L529 ***************************") #Yannik add this
        """Create and submit a task using the 'cesubmit' library."""
        for tries in range( retry ):
            remote_path = os.path.join('store', 'user', self.user, "ScannerInput/" + json_arch)
            try:
                self.remote_submitter.gridpack_upload( os.path.abspath( json_arch ), remote_path )
            except Exception as e:
                # Duration: 5 sec, 10 sec, 20 sec, ...
                # Total duration over 10 tries: ca 85 minutes
                wait = int(5 * 2**tries)
                self.logger.exception("Error uploading json arch %s. Will try again %d times in %d seconds..." % ( json_arch, retry - tries, wait) )
                time.sleep(wait)
            else:
                break
        else:
            pass
            # nobreak

        cetask = self.remote_submitter.create_task( name )
        cetask.add_gridpack( os.path.join( 'store', 'user', self.user, "ScannerInput" , json_arch ) )

        # make path relative
        output_name = self.output_arch_name( json_arch )

        cejob = cetask.create_job()
        cejob.arguments = [ "-o", output_name,
                            "-e", "bin/scanClass"]
        cejob.outputfiles.append( output_name )

        self.logger.debug( "CE job %r appended to CE task %r.", cejob, cetask )

        self.logger.info( "Submitting CE task %r...", cetask )

        # TODO: use command line option here
        cetask.submit( processes=4 )

        self.logger.info( "Task %r successfully submitted.", cetask )
        if cetask.frontendstatus == "SUBMITTED":
            return 0
        else:
            return 1

    def submit_local( self, name, json_arch ):
        self.logger.debug( "Submitting local: %s" % name )

        temp_arch_base = json_arch.replace( ".tar.gz", "")
        out_archname = self.output_arch_name( json_arch )

        # backup current working directory, will be changed and restored eventually
        outer_cwd = os.getcwd()

        # create temporary run folder for archive
        if not os.path.exists( temp_arch_base ):
            os.mkdir( temp_arch_base )

        os.chdir( temp_arch_base )

        # extract input json to obtain all input files
        with tarfile.open( os.path.join( outer_cwd, json_arch ), "r:gz" ) as tar_arch:
            tar_arch.extractall()

        # construct command line and execute
        arguments = [ os.path.join( self.scanner_base, self.executable ),
                      "-o", out_archname,
                      "-e", os.path.join( self.scanner_base, "bin/scanClass" ),
                      "-s", os.path.join( outer_cwd, os.path.basename( self.shifts_json ) ),
                      "--splitting",  "splitting.json" ]

        command = " ".join( arguments )
        self.logger.debug( "Executing: '%s'", command )

        subprocess.check_call( arguments )

        if os.path.isabs( self.local_outdir ):
            local_outdir = self.local_outdir
        else:
            local_outdir = os.path.join( outer_cwd, self.local_outdir )

        shutil.copyfile( out_archname, os.path.join( local_outdir, out_archname) )

        # restore original working directory
        os.chdir( outer_cwd )

        # delete temporary files (as on the remote submit, only the archive survives)
        shutil.rmtree( temp_arch_base )

    def output_arch_name( self, input_arch_name ):
        return os.path.basename( input_arch_name ).replace(".tar.gz", "_Output.tar.gz" )


class JobFactory( object ):
    __safe_for_unpickling__ = True

    def __init__( self, hash, distribution=None, config=None, filter_systematics=None ):
        self.hash = hash
        self.distribution = distribution
        self.filter_systematics = filter_systematics
        self.config = config

    def __repr__( self ):
        return "<%s>" % (self.__class__.__name__)

    def prepare_mc_only_job( self, mc_event_class, **kwargs ):
        dicing_job = DicingJob( mc_event_class.GetName(), self.distribution, hash=self.hash, config=self.config, **kwargs )
        dicing_job.fill_mc_bins( mc_event_class, self.filter_systematics )
        return dicing_job

    def prepare_signal_job( self, mc_event_class, signal_event_class, **kwargs ):
        signal_job = SignalJob( mc_event_class.GetName(), self.distribution, hash=self.hash, config=self.config, **kwargs )
        signal_job.fill_mc_bins( mc_event_class, self.filter_systematics )
        signal_job.fill_signal_bins( signal_event_class )
        return signal_job

    def prepare_data_job( self, mc_event_class, data_event_class, empty=False, **kwargs ):
        data_job = DataJob( mc_event_class.GetName(), self.distribution, hash=self.hash, config=self.config, **kwargs )
        data_job.fill_mc_bins( mc_event_class, self.filter_systematics )
        if not empty:
            data_job.fill_data_bins( data_event_class )
        else:
            data_job.fill_data_bins( mc_event_class, True )
        return data_job

class Job( object ):
    def __init__( self, name, distribution, hash=None, config=None ):
        self.ec_name = name
        self.distribution = distribution
        self.mc_bins = []
        self.hash = hash
        if config is None:
            config = {}
        self.scanner_config = config

    def __repr__( self ):
        return "<%s '%s'>" % (self.__class__.__name__, self.ec_name)

    def dump_to_file( self, filename ):
        with open( filename , 'w' ) as file:
            json.dump( self._asdict(), file, **JSON_DEFAULTS )

    def _asdict( self ):
        d = {
            "hash": self.hash,
            "name": self.ec_name,
            "distribution": self.distribution,
            "MCBins": _flatten( self.mc_bins ),
        }
        d.update( self.scanner_config )
        return d

    def add_scanner_config( self, name, value ):
        self.scanner_config[ name ] = value

    def fill_mc_bins( self, event_class, filter_systematics=None ):
        self.mc_bins = self._extract_mc_bins( event_class, self.distribution, filter_systematics )

    @staticmethod
    def _extract_mc_bins( event_class, distribution, filter_systematics=None ):
        if filter_systematics is None:
            filter_systematics = []

        mc_hists = ecroot.event_yield_by_process_group( event_class, distribution, aggregation_level="none" )
        uncertainty_hists = ecroot.combined_uncertainty_by_systematic_name( event_class,
                                                                     distribution,
                                                                     symmetrize=False,
                                                                     filter_systematics=filter_systematics )
        unweighted_hists = Job._extract_histograms( event_class, distribution )
        retval = Job._hists2bins( mc_hists, uncertainty_hists, unweighted_hists )

        for hist in mc_hists.values():
            hist.Delete()
        for hist in uncertainty_hists.values():
            hist.Delete()
        for hists in unweighted_hists.values():
            hists["hist"].Delete()

        return retval

    @staticmethod
    def _extract_histograms( event_class, distribution ):
        """Read histogram and corresponding error histograms."""

        # Initalize dict for unweighted number of events.
        # Is filled with dicts { "hist": unweighted histogram, "xs": crossection }
        # for each process_group.
        unweighted_hists = {}

        # Add up hists and errors for all processes
        for process_name in list( event_class.getProcessList() ):
            unweighted_hists[ process_name ] = {
                "hist" : event_class.getHistoPointerUnweighted( process_name, distribution ).Clone(),
                "xs"   : event_class.getCrossSection( process_name )
            }

        return unweighted_hists

    @staticmethod
    def _hists2bins( mc_hists, uncertainty_hists, unweighted_hists ):
        assert len( mc_hists ) > 0

        bins = []
        Nbins = mc_hists.values()[0].GetNbinsX()

        for ibin in range( 1, Nbins+1 ):
            # Obtain lowerEdge and width from "some" contribution (doesn't matter which)
            lowerEdge = mc_hists.values()[0].GetBinLowEdge( ibin )
            width = mc_hists.values()[0].GetBinWidth( ibin )

            mcEventsPerProcessGroup = collections.OrderedDict()
            mcStatUncertPerProcessGroup = collections.OrderedDict()
            for process_name, process_hist in mc_hists.items():
                assert process_hist.GetBinLowEdge( ibin ) == lowerEdge
                assert process_hist.GetBinWidth( ibin ) == width
                mcEventsPerProcessGroup[ process_name ] = process_hist.GetBinContent( ibin )
                mcStatUncertPerProcessGroup[ process_name ] = process_hist.GetBinError( ibin )

            mcSysUncerts = collections.OrderedDict()
            # Add uncertainties.
            for systematic_name, systematic_hist in uncertainty_hists.items():
                assert systematic_hist.GetBinLowEdge( ibin ) == lowerEdge
                assert systematic_hist.GetBinWidth( ibin ) == width
                mcSysUncerts[ systematic_name ] = systematic_hist.GetBinContent( ibin )

            unweightedEvents = collections.OrderedDict()
            # Add unweighted event numbers and cross sections.
            for process_name, unweighted_hist in unweighted_hists.items():
                assert unweighted_hist[ "hist" ].GetBinLowEdge( ibin ) == lowerEdge
                assert unweighted_hist[ "hist" ].GetBinWidth( ibin ) == width
                unweightedEvents[ process_name ] = {
                    "Nevents":  unweighted_hist[ "hist" ].GetBinContent( ibin ),
                    "xs" :      unweighted_hist[ "xs" ]
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


class DataJob(Job):
    def __init__( self, *args, **kwargs ):
        super( DataJob, self ).__init__( *args, **kwargs )
        self.data_bins = []

    def fill_data_bins( self, event_class, empty = False ):
        ''' Fill data bins vector. use empty option with mc class if
            event class does not exist in data.'''
        hist = ecroot.total_event_yield( event_class, self.distribution )
        self.data_bins = roothelpers.root_map_hist( hist.GetBinContent, hist, empty )

    def _asdict( self ):
        d = super( DataJob, self )._asdict()
        d["DataBins"] = _flatten( self.data_bins )
        return d

# Is initially called only once per event class, but then copied for multiple
# dicing rounds. Each dicing round copy has a different set of seeds,
# but the copies of all event classes and one dicing round have the same.
class DicingJob( Job ):
    def __init__( self, *args, **kwargs ):
        super( DicingJob, self ).__init__( *args, **kwargs )
        self.first_round = 0
        self.num_rounds = 0

    def _asdict( self ):
        d = super( DicingJob, self )._asdict()
        d["FirstRound"] = self.first_round
        d["NumRounds"] = self.num_rounds
        return d

class SignalJob( DicingJob ):
    def __init__(self, *args, **kwargs ):
        super( SignalJob, self ).__init__( *args, **kwargs )
        self.signal_bins = []

    def fill_signal_bins( self, event_class, filter_systematics=None ):
        self.signal_bins = self._extract_mc_bins( event_class, self.distribution, filter_systematics )

    def _asdict( self ):
        d = super( SignalJob, self )._asdict()
        d["SignalBins"] = _flatten( self.signal_bins )
        return d

# Create a container class for the MCBin.
MCBin = collections.namedtuple("MCBin", ( 'mcEventsPerProcessGroup', 'mcStatUncertPerProcessGroup', 'lowerEdge', 'width', 'mcSysUncerts', 'unweightedEvents' ) )

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

def setup_logging( verbose_count ):
    levels = {
        0: logging.WARNING,
        1: logging.INFO,
        2: logging.DEBUG,
    }
    level = levels[min(verbose_count, len(levels)-1)]

    formatter = logging.Formatter("%(asctime)-15s %(name)s [%(processName)-12.12s] [%(levelname)-5.5s]:  %(message)s")

    file_handler = logging.FileHandler("submission.log")
    file_handler.setFormatter(formatter)

    stream_handler = logging.StreamHandler()
    stream_handler.setFormatter(formatter)

    root_logger = logging.getLogger()
    root_logger.setLevel(level)

    root_logger.addHandler(file_handler)
    root_logger.addHandler(stream_handler)

def commandline_and_config_parsing():
    # Desired resolution order:
    # 1. if provided as command line argument, USE IT
    # 2. elif provided in config file, USE IT
    # 3. else: use default
    logger = logging.getLogger()

    parser = configargparser.ArgumentParser(description='Main script to run MUSiC scans')
    submit_group = parser.add_argument_group(title="Submit Options")
    ecroot.add_ec_standard_options( submit_group )

    submit_group.add_argument( '--signal', type=str, default=None,
        help="Signal ROOT file to study (must contain complete signal: BG + BSM model)." )

    submit_group.add_argument( '-v', '--verbose', action="count", dest='verbosity', default=0,
        help="Output verbosity (repeat -v for more verbosity)." )
    # TODO replace "jsontemp" in code with this option (-j used for number of jobs)
    #parser.add_argument( '--json-out', dest="jsondir", type=str, default='json', help="Directory for JSON files (relative to the main directory)" )
    submit_group.add_argument( '--scanner-base', default= os.path.expandvars("$SCAN_BASE"), type=str,
        help="Base path for the scanner" )
    submit_group.add_argument( '--executable', default="python/cewrapper.py", type=str, help="RoI-scan executable." )
    submit_group.add_argument( '--out', default = "./output", type=str,
        help="Directory where output jsons are saved if scan is performed on local machine" )
    submit_group.add_argument( '--remote', action='store_true',
        help="Run jobs on GRID CE")
    submit_group.add_argument( '--continue-run', action='store_true',
        help="Continue previous run on GRID CE locally. Only non-retrieved tasks will be processed locally. Other jobs are killed and purged" )
    submit_group.add_argument( '--remote-json-arch', type=str, default="jsonArch",
        help="Path of the json archive stored in user folder on SE" )
    submit_group.add_argument( '--runtime-per-arch', type=float, default= 1800.,
        help="Runtime per CE job in minutes ( determines size of arch) default: 1800" )
    submit_group.add_argument( '--timing-json', type=str, help="Name of a json file containing timing information" )
    submit_group.add_argument( '--shifts-json', type=str, help="Name of a json file containing systematic shifts" )
    submit_group.add_argument( '--site', type=str, default="T2_DE_RWTH", help="Run on remote CE computing grid. If not provided, the scan is ran on the local computer." )
    submit_group.add_argument( '--gridpackname', type=str, default="scannerGridpack_HASH.tar.bz2", help="Name of the gridpack archive" )
    # deprecated until implemented in gridlib
    #submit_group.add_argument( '--dry', action='store_true', help="Only create json files (or job descriptions with --remote).")
    # TODO implement option to continue submission
    #parser.add_argument( '--continue-submit', action='store_true', help="Continue previous run")

    scan_group = parser.add_argument_group(title="Scanner Options")
    scan_group.add_argument( '-N', '--Nrounds', default=0, type=int, help='Number of dicing rounds')
    scan_group.add_argument('--distribution', choices=("SumPt", "InvMass", "MET"), default="SumPt",
        help="Name of the distribution to scan.")
    scan_group.add_argument( '--minRegionWidth', default=0, type=int, help='Minimum number of bins per region' )
    scan_group.add_argument( '--coverageThreshold', default=0., type=float, help='Maxmial relative MC uncertainty in regions without data.' )
    scan_group.add_argument( '--sigmaThreshold', default=0., type=float, help='Minimal deviation to calculate a p-value' )
    scan_group.add_argument( '--integralScan', action='store_true', help='Perform integral scan instead of full region construction' )
    scan_group.add_argument( '--skipLookupTable', action='store_true', help='Do not use the lookup table for p-value calculation.' )
    scan_group.add_argument( '--noLowStatsTreatment', default=False, help='Skip low statistics treatment', action="store_true" )
    scan_group.add_argument( '--thresholdLowStatsUncert', default=0.6, type=float,
        help='Threshold for relative statistical uncertainty in region under investigation')
    scan_group.add_argument( '--widthLowStatsRegions', default=4, type=int,
        help='Number of bins to use as width for neighborhood regions in low statisticts treatment')
    scan_group.add_argument( '--thresholdLowStatsDominant', default=0.95, type=float,
        help='Threshold fo quantile used to consider if a neighborhood process is dominant')
    scan_group.add_argument( '--seed', default=None, type=int,
        help='Fix seed for poisson dicing' )
    scan_group.add_argument( '-s', '--filter_systematics', default = [], nargs="+",
        help="Systematics that should be filtered from json" )

    # Parse again. This time for real.
    args = parser.parse_args()

    setup_logging( args.verbosity )

    if args.continue_run and not args.remote:
        logging.warning( "Continuing previous remote submission, please make sure to use the same runtime-per-arch")

    if args.continue_run and not args.shifts_json:
        logging.error( "Option shift-json is required for continue_run" )
        sys.exit( 1 )

    # Done.
    logging.debug( "Command line and config file parsing done: %s.", args )
    return args

if __name__ == '__main__':
    main()
