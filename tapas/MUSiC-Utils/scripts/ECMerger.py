#!/usr/bin/env python

from collections import defaultdict
from datetime import datetime
import fnmatch
import logging
import glob
import math
import multiprocessing
import optparse
import os
import resource
import sys


music_path = os.getenv( 'MUSIC_BASE' )
if music_path == None:
    print 'FATAL: MUSIC_BASE not set'
    sys.exit(1)


class FileOpenError( Exception ):
    def __init__( self, filename ):
        self.filename = filename
    def __str__( self ):
        return repr( "Could not open file: '%s'" % self.filename )


log = logging.getLogger( 'ECMerger' )

################################################################################
#################################### MAIN ######################################
################################################################################


def main():
    usage = '%prog [options] INPUTPATH(S)'
    desc = "Merge ROOTFILES located at INPUTPATH(S) containing " \
           "TEventClasses. " \
           "If '--jes' is specified, INPUTPATHS can be either three " \
           "ROOTFILES (JES UP, JES DOWN, 'normal') or one or more " \
           "directories, containing MUSiC classification output. " \
           "Otherwise, INPUTPATH(S) can be JES-merged ROOTFILES " \
           "(EC_Final.root), or directories containing MUSiC classification " \
           "output (including EC_Final.root). " \
           "The EventClasses must be normalised to the same lumi. " \
           "If there is more than one file containing EventClasses from " \
           "the same process, you are prompted to choose if you want to " \
           "merge these processes or ignore the duplicates."

    data_choices = [ 'auto', 'True', 'False' ]
    merge_choice = [ 'ignore', 'all' ]

    parser = optparse.OptionParser( usage=usage,
                                    description=desc,
                                    version='%prog version 4'
                                    )
    parser.add_option( '-o', '--output', metavar = 'OUTPUT', default = './Output.root',
                       help = 'Set name of the output rootfile. [default = %default]' )
    parser.add_option(       '--debug', metavar = 'LEVEL', default = 'INFO',
                       help = 'Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default: %default]' )
    parser.add_option( '-j', '--jobs',
                       metavar ='NJOBS',
                       type ='int',
                       default = 1,
                       help = 'Use up to NJOBS jobs when merging. ' \
                              '[default = %default, max = number of cores]'
                       )
    parser.add_option( '-d', '--data', metavar = 'DATA', type = 'choice', choices = data_choices, default = data_choices[0],
                       help = 'Merging data or not? [choices = ' + ', '.join( data_choices ) + '; default = %default]' )
    parser.add_option( '-a', '--merge',
                       metavar = 'MERGETYPE',
                       type = 'choice',
                       choices = merge_choice,
                       default = None,
                       help = "Decide how to automatically merge found " \
                              "duplicate processes in given INPUTPATHS. " \
                              "MERGETYPE can be any of '%s'. " \
                              "'all' means, all EventClasses found in " \
                              "duplicate processes will be merged into the " \
                              "final ROOTFILE (rescaling all histograms to " \
                              "the total number of events for every " \
                              "process). " \
                              "'ignore' means, only EventClasses from the " \
                              "first found process (of every given type) " \
                              "will be merged into the final ROOTFILE. " \
                              "Use this option with caution, if you have " \
                              "duplicate processes in your INPUTPATHS, the " \
                              "output might be wrong! " \
                              "If no MERGETYPE is specified, you will be " \
                              "prompted in case two or more files with " \
                              "identical process names are found in the " \
                              "given INPUTPATHS. " \
                              "When using in connection with '--jobs', only " \
                              "a maximum of 10 jobs will be invoked when " \
                              "merging in parallel. " \
                              "[no default]" % ', '.join( merge_choice )
                       )
    parser.add_option( '-n', '--num-files', metavar = 'NUMFILES', type = 'int', default = 500,
                       help = 'Never keep more than NUMFILES files open at ' \
                              'the same time. (There cannot be too many ' \
                              '(ROOT)FILES open at the same time, thus merge ' \
                              'in subsets before final merge). ' \
                              '[default = %default]' )
    parser.add_option(       '--keep', action = 'store_true', default = False,
                       help = 'When merging in several steps, keep the ' \
                              'intermediate output (helpful for debugging ' \
                              'etc.). [default = %default]' )
    parser.add_option(       '--final', action = 'store_true', default = False,
                       help = 'Treat merge as final, do not store PDF ' \
                              'histograms in the merged histogram. ' \
                              'Does nothing on data. [default = %default]' )

    ( options, paths ) = parser.parse_args()

    # Set logging details
    #
    date = '%F %H:%M:%S'
    format = '%(levelname)s from %(name)s at %(asctime)s: %(message)s'
    logging.basicConfig( level = logging._levelNames[ options.debug ], format = format, datefmt = date )
    log_file_name = options.output.split( '.root' )[ 0 ] + '.log'
    file_handler = logging.FileHandler( log_file_name )
    console_handler = logging.StreamHandler()
    log.addHandler( file_handler )
    log.addHandler( console_handler )

    if len( paths ) < 1:
        parser.error( "At least one INPUTPATH must be specified!" )

    root_file_paths = []
    for path in paths:
        if not os.path.exists( path ):
            parser.error( "Specified input path '%s' does not exist!" % path )
            continue
        if path.endswith( ".root" ):
            root_file_paths.append( path )
        for root, dirnames, filenames in os.walk( path ):
            for filename in fnmatch.filter( filenames, '*.root' ):
                if filename.startswith( "EC" ): root_file_paths.append( os.path.join( root, filename ) )

    if options.data == 'True' or options.data == '1':  options.data = True
    if options.data == 'False' or options.data == '0': options.data = False
    if options.data == 'auto':
        data = set( [ 'Data' in path or 'data' in path for path in paths ] )

        if len( data ) > 1:
            parser.error( "Could not determine if files to be merged are " \
                          "data or not. Please check and specify with help " \
                          "of '--data' once you know what you are doing!"
                          )

        if len( data ) == 1:
            if True in data:
                options.data = True
                log.debug( 'Running on data.' )
            else:
                options.data = False
                log.debug( 'Running on MC.' )


    # OK, we have to import ROOT shit after the parser, otherwise the --help option
    # from the parser is handed to the root interpreter, D'Oh!
    #
    import ROOT as r
    # Make ROOT global, so don't have to pass it to the functions.
    global r
    r.gROOT.ProcessLine( '#include <set>' )
    r.gSystem.Load( os.path.join( music_path, 'lib/libTEventClass.so' ) )
    r.TH1.AddDirectory( False )
    r.TDirectory.AddDirectory( False )

    #if len( root_file_paths ) < 2:
    #    parser.error( 'At least two ROOTFILES or directories containing ' \
    #                  'ROOTFILES needed!'
    #                  )

    numRootFiles = len( root_file_paths )
    log.info( "Merging %i ROOTFILES." % numRootFiles )

    # Do we have to merge in several steps?
    if numRootFiles > options.num_files:
        multi = options.jobs > 1
        if options.merge == None:
            if multi:
                multi = False
                log.warning( "Cannot run merge in parallel, no MERGETYPE " \
                             "specified. " \
                             "Please use '--merge' and specify a " \
                             "MERGETYPE in order to run in parallel."
                             )
            else:
                log.info( "No MERGETYPE specified. " \
                          "You can use '--merge' to specify a MERGETYPE. " \
                          "This will speed up the merge!"
                          )

        if multi:
            # How many processes shall run at the same time?
            # Never do more than 10 jobs at a time, since the final merge
            # may become too large then.
            numProcesses = min( 10, options.jobs )
            numCPU = multiprocessing.cpu_count()

            # Should never be more than number of CPUs.
            if numCPU < numProcesses:
                numProcesses = numCPU

            # + 1 makes sure, that there are never too much files per chunk.
            # maxFilesPerChunk * numProcesses should always
            # be < options.num_files
            maxFilesPerChunk = options.num_files / ( numProcesses + 1 )
        else:
            maxFilesPerChunk = options.num_files

        # Split the ROOFILES to be merged into chunks not larger than
        # options.num_files / numProcesses.
        # On Linux, 'ulimit -n' gives you the max. number of
        # file(descriptors)s a user can open. A typical number is 1024.
        # Thus, by default we set the maximum number of ROOTFILES to be
        # merged at once to ~500, what should not be too much.
        root_file_paths_chunks = [ root_file_paths[ x:x + maxFilesPerChunk ]
                                   for x in xrange( 0,
                                                    len( root_file_paths ),
                                                    maxFilesPerChunk
                                                    ) ]
        numChunks = len( root_file_paths_chunks )

        log.info( "%i ROOTFILES, merging in %i steps." %
                  ( numRootFiles, numChunks )
                  )

        output_files = []
        arguments = []
        for iChunk, chunk in enumerate( root_file_paths_chunks, 1 ):
            outputName = updateOutputName( options.output, iChunk, numChunks )
            output_files.append( outputName )

            if multi:
                arguments.append( ( options,
                                    chunk,
                                    outputName,
                                    False,
                                    multi
                                    ) )
            else:
                log.info( 'Step %i of %i' % ( iChunk, numChunks ) )
                merge( options, chunk, outputName )

        # Cross check.
        if len( output_files ) != numChunks:
            log.error( 'Something went wrong! Number of output ' \
                       'files (%i) != number of merging steps (%i)!' % \
                       ( len( output_files ), numChunks )
                       )
            sys.exit( 3 )

        if multi:
            log.info( "Merging in parallel (with %i jobs)." % numProcesses )
            jobPool = multiprocessing.Pool( processes=numProcesses )

            try:
                result = jobPool.map_async( mergeMulti, arguments )
                result.wait()
                jobPool.close()
            except KeyboardInterrupt:
                jobPool.terminate()
                jobPool.join()
            else:
                result.get()

        log.info( "Merging intermediate files to '%s'." % options.output )
        merge( options, output_files, options.output, final=options.final )

        # Remove intermediate files (if not desired otherwise).
        if not options.keep:
            for file in output_files:
                os.remove( file )

    # Normal merging.
    else:
        merge( options,
               root_file_paths,
               options.output,
               final=options.final,
               )

    log.info( "Merged output written to file '%s'." % options.output )

    log.info( 'Done.' )

def addECsToSet( root_file, EC_list ):
    for object in root_file.GetListOfKeys():
        if object.GetClassName() == 'TEventClass':
            EC = object.ReadObj()
            ECName = EC.GetName()

            if 'Rec' in ECName:
                EC_list.add( ECName )
            EC.Delete()


def mergeMulti( items ):
    try:
        merge( *items )
    except KeyboardInterrupt:
        pass


def merge( options, root_file_paths, outputName, final=False, multi=False ):
    #~ sys.exit()
    processesToMerge    = set()
    processesNotToMerge = set()

    # dict to store EventClasses with all according processes.
    #
    EventClasses = defaultdict( set )
    processes = set()

    root_files = []

    # Loop over all files and EventClasses to find duplicates in the processes.
    #
    mergeAll  = options.merge == 'all'
    ignoreAll = options.merge == 'ignore'
    for root_file_path in root_file_paths:
        log.debug( "Processing file: '%s'" % root_file_path )
        root_file = r.TFile.Open( str( root_file_path ) )
        # Don't close the rootfiles, we need them later.
        #
        root_files.append( root_file )

        if not root_file:
            log.error( "Could not open root file: '%s'" % root_file_path )
            raise FileOpenError( root_file_path )

        for object in root_file.GetListOfKeys():
            if object.GetClassName() == 'TEventClass':
                EC = object.ReadObj()
                ECName = EC.GetName()

                # FIXME: Ignoring Gen EventClasses completely at the moment.
                #
                if 'Gen' in ECName: continue

                processList = EC.getGlobalProcessList()

                # Each EC has its own process list.
                #
                # For each process there are additional Jet Energy Scale (JES) up
                # and down processes in the EC. These were merged if you ran
                # ECMerger2 with --jes option (what should happen by default in
                # music). They are no longer in the process list anymore afterwards.
                # So, we don't need any special treatment.
                #
                for proc in processList:
                    # Store all processes.
                    #
                    processes.add( proc )

                    # If we had this EventClass already, check the processes.
                    # If the given process is already in the dict, this means
                    # that we found two equal EventClasses containing the same
                    # process. So, the user has to decide what to do (this is
                    # stored in processesToMerge or processesNotToMerge).
                    #
                    if ECName in EventClasses and proc in EventClasses[ ECName ] and \
                        proc not in processesToMerge and proc not in processesNotToMerge:

                        if not multi:
                            log.info( "In EventClass '%s' found existing " \
                                      "process '%s' in file '%s'." % \
                                      ( ECName, proc, root_file_path )
                                      )

                        if mergeAll:
                            if not multi:
                                log.info( "Adding process '%s' to merge list." %
                                          proc
                                          )
                            processesToMerge.add( str( proc ) )
                        else:
                            if ignoreAll:
                                continue
                            while True:
                                print "How to proceed?",
                                answer = raw_input( '[(i)gnore]/(m)erge/merge (a)ll: ' )
                                if answer == 'i' or answer == 'ignore' or len( answer ) == 0:
                                    processesNotToMerge.add( proc )
                                    break
                                if answer == 'm' or answer == 'merge':
                                    processesToMerge.add( str( proc ) )
                                    break
                                if answer == 'a' or answer == 'all' or answer == 'merge all':
                                    processesToMerge.add( str( proc ) )
                                    mergeAll = True
                                    break

                                else:
                                    print "Unrecognised answer '%s'" % answer
                                    print "Please try again:",

                    # If not, add it to the (global) dict and store all processes.
                    #
                    else:
                        EventClasses[ ECName ].add( proc )

                EC.Delete()     # If you do not delete this here, you get a huge memory leak! Thanks ROOT.

    # Very verbose output; print only when desired!
    log.debug( 'Found the following EventClasses:\n' +
               '\n'.join( sorted( EventClasses.keys() ) ) + '\n'
               )

    log.debug( 'Found the following processes:\n' +
               '\n'.join( sorted( list( processes ) ) ) + '\n'
               )

    procsToMerge = sorted( list( processesToMerge ) )

    # Yeah, again ugly (py)ROOT stuff. Well, better than nothing.
    # (This is a reimplementation of the stl container in ROOT.)
    #
    processesToMerge = r.set( 'string' )()
    for proc in procsToMerge:
        # When calling insert for the first time, (py)ROOT complains.
        # Dunno how to get rid of it atm., but you can ignore it.
        #
        processesToMerge.insert( processesToMerge.begin(), str( proc ) )

    if len( procsToMerge ) > 0 and not multi:
        info = 'Merging the following duplicate processes in all EventClasses:\n'
        info += '\n'.join( procsToMerge )
        info += '\n'

        log.info( info )

    if len( processesNotToMerge ) > 0 and not multi:
        info = 'Ignoring the following duplicate processes in all ' \
               'EventClasses (using only the first found):\n' + \
               '\n'.join( sorted( list( processesNotToMerge ) ) )

        log.info( info )

    output = r.TFile( outputName, 'RECREATE' )

    processesEvents = r.map( 'string', 'double' )()

    # First of all we need to know the total number of events processes for each
    # process, because we have to tell each EventClass.
    #
    empty_files = []
    for root_file in root_files:

        # Empty+X is always there!
        #
        EmptyEC = root_file.Get( 'Rec_Empty+X' )
        if EmptyEC:
            for proc in processes:
                processesEvents[ proc ] += EmptyEC.getTotalEventsUnweighted( proc )

            EmptyEC.Delete()
        else:
            # Nothing to worry about!
            #
            log.debug( "No EventClasses found in file: '%s'" % root_file.GetName() )
            empty_files.append( root_file )

    for file in empty_files:
        file.Close()
        file.Delete()
        root_files.remove( file )

    longest_p = max( map( len, [ i for i,j in processesEvents ] ) ) + 1
    longest_e = max( map( len, [ str( '%.1f' % j ) for i,j in processesEvents ] ) ) + 1

    # Print all processes and their event numbers:
    #
    debug = 'Added up event numbers for all processes found:\n'
    head = 'Process:'.ljust( longest_p ) + 'Number:'.rjust( longest_e )
    debug += head + '\n'
    debug += len( head ) * '-' + '\n'
    for process, events in processesEvents:
        debug += str( process +  ':' ).ljust( longest_p ) + str( ' %.1f' % events ).rjust( longest_e ) + '\n'
    log.debug( debug )

    # At this place we loop over all EventClasses that we have found above and
    # look for them in each rootfile and add them up. It is important to loop
    # over all rootfiles for each EventClass because if you do it vice versa, it
    # will blow up your memory and you do not want that.
    #
    max_num = float( len( EventClasses.keys() ) )

    if not multi:
        log.info( 'Merging...' )

    #~ if sys.stdout.isatty() and not multi:
        #~ import progressBar
        #~ progr = progressBar.progressBar( 0, max_num )

    for num, EC in enumerate( sorted( EventClasses.keys() ), 1 ):
        #~ if sys.stdout.isatty() and not multi:
            #~ progr.updateProgress( num )

        ECFinal = None

        for root_file in root_files:
            OneEC = root_file.Get( EC )
            if OneEC:
                if not ECFinal:
                    if options.data:
                        ECFinal = r.TEventClass( OneEC )
                    else:
                        ECFinal = r.TEventClass( OneEC, True )
                        ECFinal.setTotalEventsUnweighted( processesEvents )
                        ECFinal.addEventClass( OneEC, processesToMerge, options.data )
                else:
                    ECFinal.addEventClass( OneEC, processesToMerge, options.data )

                OneEC.Delete()

        # Look if all PDF histos for all processes are dropped, if only for some give warning
        pdfs_dropped_first = True
        if ECFinal.getNumOfPDFHistos( list( ECFinal.getProcessList() )[ 0 ] ) != 0:
            pdfs_dropped_first = False

        pdfs_dropped = pdfs_dropped_first

        for process in list( ECFinal.getProcessList() ):
            pdfs_dropped = True
            if ECFinal.getNumOfPDFHistos( process ) != 0:
                pdfs_dropped = False

            if pdfs_dropped != pdfs_dropped_first:
                log.warning( "PDF histos sometimes dropped, bur not in all processes" )
                pdfs_dropped = True
                break

        if not options.data and not pdfs_dropped:
            print "calculatePDFUncertainty for class %s" % ECFinal.GetName()
            # Necessary to get the right values!
            if not "Rec_Empty" in ECFinal.GetName(): ECFinal.calculatePDFUncertainty()
            # If this is a final merge (either merging the intermediate files,
            # or files specified by the user), and the uses used the '--final'
            # option, remove the PDF histograms from the final rootfile as they
            # take a lot of (disk)space.
            if final:
                ECFinal.dropPDFHistograms()

        ECFinal.addChangeLogEntry( "Merged into file %s" % outputName )
        output.cd()
        ECFinal.Write()
        ECFinal.Delete()

    output.Close()
    output.Delete()

    for file in root_files:
        file.Close()
        file.Delete()


def updateOutputName( fileName, iChunk, numChunks ):
    string = '_%i_of_%i' % ( iChunk, numChunks )
    if fileName.endswith( '.root' ):
        fileName = fileName.replace( '.root', string + '.root' )
    else:
        fileName += string

    return fileName


if __name__ == '__main__':
    startTime = datetime.now()
    main()

    delta = datetime.now() - startTime
    days, hours, mins, secs = delta.days, \
                              delta.seconds / 3600, \
                              delta.seconds % 3600 / 60, \
                              delta.seconds % 3600 % 60
    log.info( "Run time: %id%ih%im%is" % ( days, hours, mins, secs ) )
