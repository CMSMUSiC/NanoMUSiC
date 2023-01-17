#!/usr/bin/env python

## @package aix3a2Finder
# Get sample info from aix3adb
#
#
# @author Tobias Pook
import aix3adb
from  aix3adb import Aix3adbException
import argparse

def commandline_parsing():
    descr = 'Get sample info from aix3adb'
    parser = argparse.ArgumentParser(description= descr)
    parser.add_argument( '--samplefile', help='A file list with one sample per line')
    parser.add_argument( '--datasetfile', help='A file list with one dataset per line')
    parser.add_argument( '--dataset', help='A single datasetpath')
    parser.add_argument( '--listFields',action='store_true', help='Print a list of fields for aix3adb objects')
    parser.add_argument( '--data', action='store_true')
    args = parser.parse_args()
    return args

def SkimAndSampleSummary(skim, sample):
    return '{:<75} {:<5} {:<5} {:<14} {:<12} {:<5} '.format(sample.name,
                                                                sample.id,
                                                                skim.id,
                                                                sample.crosssection,
                                                                sample.filterefficiency,
                                                                sample.kfactor)

def main():
    args = commandline_parsing()

    # Create a database object.
    dblink = aix3adb.aix3adb()

    if args.datasetfile or args.dataset or args.samplefile:
        print '{:<75} {:<5} {:<5} {:<14} {:<12} {:<5} '.format('sample',
                                                                'id',
                                                                'skimId',
                                                                'xs',
                                                                'filter_eff',
                                                                'kfactor')
    # work on a list of datasetpath or sample names
        if args.datasetfile: infilepath = args.datasetfile
        if args.samplefile: infilepath = args.samplefile
        missing = []
        with open( infilepath, 'r') as infile:
            identifiers = infile.read().split('\n')
            for identifier in identifiers:
                try:
                    if args.datasetfile:
                        skim, sample = dblink.getMCLatestSkimAndSampleByDatasetpath(identifier)
                    if args.samplefile:
                        skim, sample = dblink.getMCLatestSkimAndSampleBySample(identifier)
                    print SkimAndSampleSummary(skim, sample)
                except Aix3adbException:
                    missing.append( identifier )
        if len(missing) > 0:
            print "No aix3adb entry found for the following datasetpath:"
            for p in missing: print p

        if args.dataset:
            #~ print args.dataset
            try:
                skim, sample = dblink.getMCLatestSkimAndSampleByDatasetpath( args.dataset )
                print SkimAndSampleSummary(skim, sample)
            except Aix3adbException:
                print "no sample found for datasetpath: /n %s " % args.dataset

    if args.listFields:
        skim, sample = dblink.getMCSkimAndSampleBySkim( 1601 )
        print "Showing list of all available fields with example output"
        print "Sample:"
        for key in sample.__dict__.keys():
            print '{:<45}  :   {:<30}'.format( key, sample.__dict__[key])

        print "Skim:"
        for key in skim.__dict__.keys():
            print '{:<45}  :   {:<30}'.format( key, skim.__dict__[key])

if __name__ == '__main__':
    main()
