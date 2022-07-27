#!/usr/bin/env python

## @package aix3a2music
# Create MUSiC config files from aix3db
#
#
# @author Tobias Pook

import datetime
import os, csv
import sys
import time
import re
import logging
import optparse
import glob
import subprocess
import imp
import pickle
import fnmatch
import fnmatch

#custom libs
import aix3adb
from aix3adb import Aix3adbException
from ramlib import ramutils
import processgroup
# some general definitions
COMMENT_CHAR = '#'

# Write everything into a log file in the directory you are submitting from.
log = logging.getLogger( 'aix3a2music' )

def commandline_parsing():
    logging.basicConfig(stream=sys.stderr, level=logging.DEBUG, format='%(name)s - %(levelname)s: %(message)s')

    import argparse
    from cesubmit import getCernUserName
    user = getCernUserName()
    parser = argparse.ArgumentParser(description='Create music config files from DB')
    parser.add_argument('files', metavar='FILES', type=str, nargs='+',
                   help='A list of input files')
    parser.add_argument('--lumi',  type=float, default=35922., help='Luminosity default: %(default)s')
    parser.add_argument('--user',  type=str, default = user,
                   help='Change user name for dCache position default % s' % user)
    # should add simple file lists 'skimid','sampleid','samplename'
    # sampleid and samplename use the latest skim
    parser.add_argument('--inputType', default='ram',
                        choices=['music_crab','television','ram'],
                        help='Choose your input file format dafault: %(default)s')
    args = parser.parse_args()
    log.debug('Args: %r', args)
    return args

def readSampleFile(filename):
    samples = []

    after_config = False
    existing = [] #]getExistingProcesses()

    with open( filename, 'rb' ) as sample_file:
        for line in sample_file:
            line = line.strip()

            if not line or line.startswith( COMMENT_CHAR ):
                continue

            if line.startswith( 'config' ):
                after_config = True
                continue

            if after_config and not 'config' in line:
                #lumi-mask and lumis-per-job can be specified in the command line
                name, sample = line.split( ':' )
                samples.append( name )

    log.debug('Samples: %s', samples)
    return samples


def getSkimAndSampleList(args, dblink):
    outlist = []
    for inputFile in args.files:
        if args.inputType == 'music_crab':
            samples = readSampleFile( inputFile )
            for sample in samples:
                try:
                    dbSkim, dbSample = dblink.getMCLatestSkimAndSampleBySample( sample )
                    outlist.append( ( dbSkim, dbSample ) )
                except Aix3adbException:
                    log.error( 'Sample %s not found in aix3adb', sample )

        elif args.inputType == 'television':
            #little hack to use the remoteAnalysis Tool to read input files for it
            class ConfigDummy: pass
            from remoteAnalysis import readConfig
            dummy = ConfigDummy()
            dummy.__dict__['prepareConfigs'] = 'None'
            remoteDict = readConfig( dummy, [inputFile] )
            outlist += flattenRemoteSkimDict( remoteDict , dummy.datasections )

        elif args.inputType == 'ram':
            settings, samplecfg = ramutils.load_config( inputFile )
            samples = ramutils.get_samples(settings, samplecfg)
            for sample in samples:
                outlist.append( ( sample.skim, sample.sample ) )

    return outlist


# @param datasections a list of section which contain data samples
def flattenRemoteSkimDict( remoteDict , datasections):
    remoteList = []
    for section in remoteDict.keys():
        if section in datasections:
            continue
        for ( skim, sample, arguments ) in remoteDict[section]:
            remoteList.append( (skim, sample) )
    return remoteList

def getConfigDicts( skimlist ):
    playlistdict = {}
    scalesdict = {}

    no_ext_samples = [dbSample.name for dbSkim, dbSample in skimlist if not "_ext" in dbSample.name]

    for ( dbSkim, dbSample ) in skimlist:
        s = dbSample.name.split("_ext")
        cleaned_name = dbSample.name
        if len(s) > 1:
            cleaned_name = s[0] + s[1][1:]

        if "_ext" in dbSample.name and cleaned_name in no_ext_samples:
            continue
        dbSample.name = cleaned_name

        if not dbSample.generator in playlistdict.keys():
            playlistdict[dbSample.generator] = []
        if not dbSample.generator in scalesdict.keys():
            scalesdict[dbSample.generator] = []

        dCachedir = '%s/PxlSkim/%s' %( dbSkim.owner, dbSkim.version )
        outstring = '%s = %s::%s::MC_miniAOD.cfg' % (dbSample.id, dbSample.name, dCachedir ) #2015-02-08
        playlistdict[ dbSample.generator ].append( outstring )

        #~ scalesdict[ dbSample.generator ].append( dbSample.name + '.' + 'aix3adbID = ' + str(dbSample.id) )
        print dbSample.name, dbSample.crosssection, str(dbSample.crosssection)
        scalesdict[ dbSample.generator ].append( dbSample.name + '.' + 'XSec = ' + str(dbSample.crosssection))
        if dbSample.filterefficiency:
            scalesdict[ dbSample.generator ].append( dbSample.name + '.' + 'FilterEff = ' + str(dbSample.filterefficiency) )
        else:
            scalesdict[ dbSample.generator ].append( dbSample.name + '.' + 'FilterEff = 1'  )
        scalesdict[ dbSample.generator ].append( dbSample.name + '.' + 'kFactor = ' + str(dbSample.kfactor) )
        if float(dbSample.kfactor) -1 > 1e-6:
            scalesdict[ dbSample.generator ].append( dbSample.name + '.XSecOrder = ' + str(dbSample.kfactor_order)  )
        else:
            scalesdict[ dbSample.generator ].append( dbSample.name + '.XSecOrder = ' + str(dbSample.crosssection_order)  )
        scalesdict[ dbSample.generator ].append( dbSample.name + '.ProcessGroup = ' + processgroup.get_process_group(dbSample.name) )

    return scalesdict, playlistdict

def writeConfigDicts( scalesdict, playlistdict , configdir='', lumi =1.):
    lines = []
    lines.append('[GENERAL]')
    lines.append('type = mc')
    for generator in playlistdict.keys():
        lines.append( '\n[' + generator + ']')
        lines += playlistdict[ generator ]

    scalelines = []
    scalelines.append( 'Lumi = %.1f' % lumi)
    scalelines.append( 'LumiUnit = pb-1' )
    scalelines.append( '# Lumi error (CMS PAS LUM-13-001):' )
    scalelines.append( 'Global.ScalefactorError = 0.026' )

    for generator in scalesdict.keys():
        scalelines.append( '\n###' + generator + '###')
        scalelines += scalesdict[ generator ]

    with open( os.path.join( configdir, 'playlist.txt'), 'wb' ) as playlistfile:
        playlistfile.write( '\n'.join( lines ) )

    with open( os.path.join( configdir, 'scales.txt'), 'wb' ) as scalelistfile:
        scalelistfile.write( '\n'.join( scalelines ) )

def main():

    args = commandline_parsing()

    dblink = aix3adb.createDBlink( args.user )

    skimlist = getSkimAndSampleList( args, dblink )

    scalesdict, playlistdict = getConfigDicts( skimlist )

    writeConfigDicts( scalesdict, playlistdict , lumi =args.lumi)


if __name__ == '__main__':
    main()
