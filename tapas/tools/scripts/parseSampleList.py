#!/usr/bin/env python

import logging
import optparse
import os
import re
import sys
import collections
from collections import OrderedDict

log = logging.getLogger( 'parseSampleList' )

class mc_sample:
    def __init__(self, dataset ):
        self.dataset = dataset
        self.name = ''
        self.generator = ''
    def parse_name(self, options, generators, showers = [], blacklist = []):
        # format of datasetpath: /.../.../...
        # first part contains name + additional tags ( cme, tune, .. )
        # second part has additional information ( campaign, extention sample? ,... )
        # third part contains sample 'format' (AOD, MINIAOD, ...)
        dataset_split = self.dataset.split('/')
        ds_pt1 = dataset_split[1]
        ds_pt2 = dataset_split[2]
        for generator in generators.keys():
            # subn() performs sub(), but returns tuple (new_string, number_of_subs_made)
            # using (?i) at the beginning of a regular expression makes it case insensitive
            ( ds_pt1, n ) = re.subn( r'(?i)[_-]' + generator, '', ds_pt1 )
            if n > 0:
                self.generator = generator
                for shower in showers:
                    ds_pt1 = re.sub( r'(?i)[_-]' + shower, '', ds_pt1 )
                break
            else:
                self.generator = 'unknown'
        for item in blacklist:
            ds_pt1 = re.sub( r'(?i)[_-]*' + item, '', ds_pt1 )
        match = re.search('ext\d\d*',ds_pt2)
        if match:
            self.name = ds_pt1 + "_" + options.cme + "TeV_" + match.group() + "_" + options.postfix + generators[self.generator]
        else:
            self.name = ds_pt1 + "_" + options.cme + "TeV_" + options.postfix + generators[self.generator]

def readSamplesFromFile( filename ):
    file = open( filename, 'r')
    file.seek( 0 )
    samples = []
    for line in file:
        line_strip = line.rstrip('\n')
        samples.append( mc_sample(line_strip) )
    file.close()
    return samples

def main():

    usage = '%prog [options] SAMPLELIST'
    description = 'This script parses a given file SAMPLELIST and writes a customizable list that can be given to music_crab.'

    parser = optparse.OptionParser( usage = usage, description = description, version = '%prog 0' )
    parser.add_option( '-c', '--config', action = 'store', default = 'mc_miniAOD_cfg.py', metavar = 'CONFIG',
                       help = 'Specify the CMSSW config file you want to use for skimming. [default = %default]' )
    parser.add_option( '-e', '--cme', action = 'store', default = '13', metavar = 'ENERGY',
                       help = 'The center-of-mass energy for this sample' )
    parser.add_option( '-p', '--prefix', action = 'store', default = None, metavar = 'PREFIX',
                       help = 'Specify a PREFIX for your output filename (e.g.  production version). [default = %default]' )
    parser.add_option( '-P', '--postfix', action = 'store', default = None, metavar = 'POSTFIX',
                       help = 'Specify a POSTFIX for every process name in the output file. [default = %default]' )
    parser.add_option( '-f', '--force', action = 'store_true', default = False,
                       help = 'Force overwriting of output files.' )
    parser.add_option(       '--debug', metavar = 'LEVEL', default = 'INFO',
                       help = 'Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default: %default]' )
    ( options, args ) = parser.parse_args()

    # Set loggig format and level
    #
    format = '%(levelname)s (%(name)s) [%(asctime)s]: %(message)s'
    date = '%F %H:%M:%S'
    logging.basicConfig( level = logging._levelNames[ options.debug ], format = format, datefmt = date )

    if options.prefix:
        options.prefix += '_'
    if options.prefix == None:
        options.prefix = ''

    if options.postfix:
        options.postfix += '_'
    if options.postfix == None:
        options.postfix = ''

    log.debug( 'Parsing files: ' + ' '.join( args ) )

    if len( args ) != 1:
        parser.error( 'Exactly 1 argument needed: state the file you want to parse.' )

    # dict of generators with shortname
    generators = OrderedDict()
    generators['madgraph'] = 'MG'
    generators['powheg'] = 'PH'
    generators['herwig6'] = 'HW'
    generators['herwigpp'] = 'HP'
    generators['herwig'] = 'HW'
    generators['sherpa'] = 'SP'
    generators['amcatnlo'] = 'AM'
    generators['amcnlo'] = 'AM'
    generators['alpgen'] = 'AG'
    generators['calchep'] = 'CA'
    generators['comphep'] = 'CO'
    generators['lpair'] = 'LP'
    generators['pythia6'] = 'P6'
    generators['pythia8'] = 'P8'
    generators['pythia'] = 'PY'
    generators['gg2ww'] = 'GG'
    generators['gg2zz'] = 'GG'
    generators['gg2vv'] = 'GG'
    generators['JHUGen'] = 'JG'
    generators['blackmax'] = 'BM'
    generators['unknown'] = '??'

    # list of generators used for hadronization on top of another generator (will be removed from name)
    showers = [ 'pythia8', 'pythia6', 'pythia', 'herwigpp']

    # list of tags which will be removed from name (case insensitive)
    blacklist = ['13tev',
                 'madspin',
                 'FXFX',
                 'MLM',
                 'NNPDF30',
                 'NNPDF31',
                 'TuneCUEP8M1',
                 'TuneCUETP8M1',
                 'TuneCUETP8M2T4',
                 'TuneCP5']

    # read samples from file and parse name
    sample_list = readSamplesFromFile( args[0] )
    for sample in sample_list:
        sample.parse_name(options, generators, showers, blacklist)

    # write .txt files for separated by generator
    unknown_gen = False
    for generator in generators:
        temp_list = filter(lambda sample: sample.generator == generator, sample_list)
        temp_list.sort( key = lambda sample : sample.name )
        if generator == 'unknown' and len(temp_list) > 0:
            unknown_gen = True
        elif len(temp_list) > 0:
            print ""
            print "Generator:\t" + generator.upper()
            outfile_name = 'mc_' + options.prefix + generator + '.txt'
            if os.path.exists( outfile_name ) and not options.force:
                raise Exception( "Found file '%s'! If you want to overwrite it use --force."% outfile_name )
            else:
                outfile = open(outfile_name, 'w')
                outfile.write( 'generator = ' + generator.upper() + '\n')
                outfile.write( 'CME = ' + options.cme + '\n')
                outfile.write( 'config = ' + options.config + '\n')
                outfile.write( '\n')
                for sample in temp_list:
                    print sample.name + ":\t" + sample.dataset
                    outfile.write(sample.name + ":" + sample.dataset + '\n')
                outfile.close()
    print ""
    print ' ----> Total number of datasets: %s  \n' % len( sample_list )
    if unknown_gen:
        log.warning( 'There are dataset(s) produced with unkown generator(s)!' )
        temp_list = filter(lambda sample: sample.generator == generator, sample_list)
        for sample in temp_list:
            print ""
            print 'Dataset produced with unkown generator:\n' + sample.name + ':\t' + sample.dataset

if __name__ == '__main__':
    main()
