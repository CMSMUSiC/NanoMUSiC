#!/usr/bin/env python
# Copyright [2015] <III Phys. Inst A, RWTH Aachen University>

# standard library imports
import sys
import os
import yaml
import glob
import tarfile
import subprocess
import multiprocessing

# local imports
import ramutils


## Extract a compressed file
#
# @param inputfile Archive which to extract
# @param destination Destination of the archive contents
def archive_extract(inputfile, destination='./'):
    # open archive with transparent compression
    with tarfile.open(inputfile, 'r') as archive:
        archive.extractall(destination)


## Merge root files using hadd
#
# @param outputfile Name of the output file
# @param files String of files which to merge
def merge_histograms(outputfile, files):
    command = ['hadd', '-ff', outputfile, files]
    process = subprocess.Popen(' '.join(command), stdout=subprocess.PIPE, shell=True)
    stdout, stderr = process.communicate()
    if process.returncode is not 0:
        os.remove(outputfile)
        return os.path.basename(outputfile) + ': Merging FAILED. Output removed.'
    return os.path.basename(outputfile) + ': Merging successful.'


## Class to manage output merging
class Merger():
    ## The constructor
    #
    # @param self Object pointer
    # @param args Passed through command line arguments
    def __init__(self, args):
        # load settings from arguments
        self.directory = args.directory
        self.skipconfig = args.skip_config
        # instance variables
        self.histfiles = []

        # skip the last section if not using a config
        if self.skipconfig:
            return

        # load config file
        self.settings, self.samplecfg = ramutils.load_config(args.config)


    ## Class representation as a string
    #
    # @param self Object pointer
    def __str__(self):
        s = ('<Merger instance>\n'
             + str(len(self.samplelist)) + ' stored samples:\n')
        for sample in self.samplelist:
            s += sample.__str__()
        return s


    ## Check if the archive contanis suitable root files for merging
    #
    # @param self Object pointer
    # @param archive Name of the archive
    def archive_test(self, archive):
        with tarfile.open(glob.glob(os.path.join(self.directory, '*/*/grid-*', archive))[0], 'r') as testarchive:
            rootfiles = [rootfile for rootfile in testarchive.getnames() if rootfile.endswith('.root')]
        if rootfiles:
            self.histfiles = list(set(rootfiles))
            return True
        return False


    ## Prepares the output of the submitted analysis for merging
    #
    # @param self Object pointer
    def prepare_output(self):
        print 'Checking output files:',
        if self.skipconfig:
            rootfiles = [os.path.basename(rootfile) for rootfile
                         in glob.glob(os.path.join(self.directory, '*/*/grid-*', '*.root'))]
            archives = [os.path.basename(archive) for archive
                         in glob.glob(os.path.join(self.directory, '*/*/grid-*', '*.tar.*'))]
        else:
            # check the available output files
            outputfiles = self.settings['outputfiles']
            rootfiles = [rootfile for rootfile in outputfiles if rootfile.endswith('.root')]
            archives = [archive for archive in outputfiles if 'tar' in archive]
        print 'Found', len(rootfiles), 'root files and', len(archives), 'archives'

        # merge root files if there are any
        if rootfiles:
            print 'Preparing to merge', list(set(rootfiles))
            self.histfiles = list(set(rootfiles))

        # otherwise search for rootfiles in the archives
        elif archives:
            # check the archives for root files
            print 'Testing archives:',
            archive_candidate = None
            for archive in archives:
                if self.archive_test(archive):
                    archive_candidate = archive
                    break
            if not archive_candidate:
                raise Exception('No suitable root files found in the archive(s)')

            # extract archive for all directories
            print 'Preparing to merge', self.histfiles, 'from', archive_candidate
            files = glob.glob(os.path.join(self.directory, '*/*/grid-*', archive_candidate))
            progress = ramutils.Progress(maximum=len(files), text='Extracting archives:')
            pool = multiprocessing.Pool()
            for f in files:
                pool.apply_async(archive_extract, args=(f, os.path.dirname(f)),
                                 callback=progress.advance_by(1.0))
            pool.close()
            pool.join()
            progress.end()
        else:
            raise Exception('No root file or archive amongst the output files')



    ## Merge root files of different subdirectories
    #
    # @param self Object pointer
    def merge_output(self):
        if self.skipconfig:
            paths = [path for path in glob.glob(os.path.join(self.directory, '*/*')) if os.path.isdir(path)]
        else:
            # create paths based on database samples in the config
            paths = []
            samplelist = ramutils.get_samples(self.settings, self.samplecfg, False)
            for sample in samplelist:
                paths.append(os.path.join(self.directory, sample.samplegroup, sample.get_taskname()))

        progress = ramutils.Progress(maximum=len(paths), text='Merging samples:')
        pool = multiprocessing.Pool()
        results = []
        if not os.path.exists( os.path.join( self.directory, "output" ) ):
            os.mkdir( os.path.join( self.directory, "output" ) )
        for i, histfile in enumerate( self.histfiles ):
            if not os.path.exists( os.path.join( self.directory, "output", str(i) ) ):
                os.mkdir( os.path.join( self.directory, "output", str(i) ) )
            for path in paths:
                outputfile = os.path.basename(path) + '.root'
                outputfilepath = os.path.join(path, '..', '..', "output", str( i ), outputfile)
                #rootfilepath = os.path.join(path, 'grid-*', histfile)
                rootfilepath = os.path.join(path, '*', histfile)
                rootfiles = glob.glob(rootfilepath)
                if not rootfiles:
                    continue
                results.append(pool.apply_async(merge_histograms,
                                                args=(outputfilepath, rootfilepath),
                                                callback=progress.advance_by(1.0)))
        pool.close()
        pool.join()
        progress.end()

        # print summary
        print ''
        print 'Merging summary'
        for result in results:
            print result.get()



if __name__=='__main__':
    print 'This is a library, don\'t run it.'
    sys.exit(0)
