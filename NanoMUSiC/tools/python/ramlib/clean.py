#!/usr/bin/env python
# Copyright [2015] <III Phys. Inst A, RWTH Aachen University>

# standard library imports
import sys
import os

# local imports
import ramutils
from ROOT import TFile


## Cleaner class removing the output which has already been merged
#
# This class removes the directories containing the output of the analysis which
# ran on the grid. It determines which directories to remove by checking which
# merged root files already exist and can be opened properly.
class Cleaner():
    ## The constructor
    #
    # @param self Object pointer
    # @param args Command line arguments
    def __init__(self, args):
        # load settings from arguments
        self.directory = args.directory
        # list of trash to be deleted
        self.trash = []


    ## Find directories matching valid, merged root files
    #
    # @param self Object pointer
    def prepare_cleanup(self):
        import glob
        for samplegroupath in glob.glob(os.path.join(self.directory, '*')):
            if os.path.isdir(samplegroupath):
                print 'Checking subdirectory', samplegroupath

                # get files and directories
                files = [item for item in os.listdir(samplegroupath)
                         if (os.path.isfile(os.path.join(samplegroupath, item)) and item.endswith('.root'))]
                directories = [item for item in os.listdir(samplegroupath)
                               if os.path.isdir(os.path.join(samplegroupath, item))]

                # check if root files work
                for f in files:
                    filepath = os.path.join(samplegroupath, f)
                    tfile = TFile.Open(filepath)
                    if not tfile:
                        raise Exception('Root file ' + filepath + ' is broken')

                    # store trash in self.trash
                    trashdir = f[:-5]
                    if trashdir in directories:
                        print 'Adding to trash:', trashdir
                        self.trash.append(os.path.join(samplegroupath, trashdir))
                    else:
                        print 'Merged file found, but directory missing:', trashdir


    ## Delete previously determined trash
    #
    # @param self Object pointer
    def perform_cleanup(self):
        import shutil

        progress = ramutils.Progress(maximum=len(self.trash), text='Removing trash:')
        for trash in self.trash:
            shutil.rmtree(trash)
            progress.advance_by(1.0)
        progress.end()


if __name__=='__main__':
    print 'This is a library, don\'t run it'
    sys.exit(0)
