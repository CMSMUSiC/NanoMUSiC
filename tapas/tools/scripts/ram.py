#!/usr/bin/env python
# Copyright [2015] <III Phys. Inst A, RWTH Aachen University>
#
# This script merges the output of remoteAnalysis.py based on its input config file

# standard library imports
import sys
import os
import argparse
import logging
import yaml

# setup logging
logging.basicConfig(level=logging.WARNING)


## Write a cache file containg information of the last ram call
#
# @param config Path to the configuration file
# @param directory Path of the output directory
def write_cache(config, directory):
    # set cache variables
    cache = {
        'recent': {
            'config': os.path.abspath(config),
            'directory': os.path.abspath(directory)
        }
    }

    # write cache
    cachedirectory = os.path.expanduser('~/.cache/tapas/')
    if not os.path.exists(cachedirectory):
        os.makedirs(cachedirectory)
    with open(cachedirectory + 'ram.yaml', 'w') as cachefile:
        cachefile.write(yaml.dump(cache, default_flow_style=False))


## Retrieve an option from the cache file
#
# @param option Option to be retrieved from the cache file
def get_cached_option(option):
    cachefile = os.path.expanduser('~/.cache/tapas/ram.yaml')
    if not os.path.isfile(cachefile):
        return 'No cache exists yet'

    # return request option
    with open(cachefile, 'r') as cache:
        return yaml.load(cache)['recent'][option]


## Run in cleaning mode
#
# @param args Command line arguments passed through
def clean(args):
    if not args.directory:
        args.directory = get_cached_option('directory')
    write_cache(get_cached_option('config'), args.directory)

    from ramlib import clean
    cleaner = clean.Cleaner(args)
    cleaner.prepare_cleanup()
    cleaner.perform_cleanup()


## Run in merging mode
#
# @param args Command line arguments passed through
def merge(args):
    if not args.directory:
        args.directory = get_cached_option('directory')
    write_cache(args.config, args.directory)

    from ramlib import merge
    merger = merge.Merger(args)
    merger.prepare_output()
    merger.merge_output()


## Run in submission mode
#
# @param args Command line arguments passed through
def submit(args):
    write_cache(args.config, args.directory)
    if os.path.exists(args.directory) and not args.force:
        raise Exception('Directory already exists - Use -f / --force to override')

    from ramlib import submit
    submitter = submit.Submitter(args)
    submitter.prepare_gridpack()
    submitter.prepare_jobs()
    submitter.submit_jobs()


## Main function steering the gridanalysis.py tool
#
# Automatically calls the respective function for each mode.
def main():
    # custom formatting class for more width in help output
    class CustomFormatter(argparse.ArgumentDefaultsHelpFormatter, argparse.HelpFormatter):
        def __init__(self,prog):
            super(CustomFormatter, self).__init__(prog, max_help_position=55, width=120)

    # top level argument parsing
    parser = argparse.ArgumentParser(description='Run your analysis on the grid and manage its output',
                                     formatter_class=CustomFormatter)
    subparsers = parser.add_subparsers(title='Available modes')


    # submission mode
    parser_submit = subparsers.add_parser('submit', help='Submit analysis to the grid',
                                          formatter_class=CustomFormatter)
    parser_submit.add_argument('-f', '--force', action='store_true', default=False,
                               help='Force overriding existing directories')
    parser_submit.add_argument('--test', action='store_true',
                               help='Submit one small test job for each section')
    import time
    parser_submit.add_argument('-d', '--directory', action='store',
                               default='ramout_{0}'.format(time.strftime('%Y-%m-%d_%H-%M-%S')),
                               metavar='DIR', help='Specify directory for the output')
    parser_submit.add_argument('-s', '--site', action='store',
                               choices=['T2_DE_RWTH', 'T2_DE_DESY'], default='T2_DE_RWTH',
                               help='Computing site on which to run')
    parser_submit.add_argument('config', nargs='?', default='{0}'.format(get_cached_option('config')),
                               metavar='CFG', help='Configuration file')
    parser_submit.set_defaults(func=submit)


    # merging mode
    parser_merge = subparsers.add_parser('merge', help='Merge retrieved output',
                                         formatter_class=CustomFormatter)
    group_merge = parser_merge.add_mutually_exclusive_group()
    group_merge.add_argument('-c', '--config', default='{0}'.format(get_cached_option('config')),
                             metavar='CFG', help='Configuration file')
    group_merge.add_argument('-s', '--skip-config', action='store_true', default=False,
                             help='Merge root files without config file')
    parser_merge.add_argument('directory', nargs='?', action='store',
                              default='{0}'.format(get_cached_option('directory')),
                              metavar='DIR',help='Specify top level output directory ')
    parser_merge.set_defaults(func=merge)


    # cleaning mode
    parser_clean = subparsers.add_parser('clean',
                                         help='Remove output directories for which a merged root file exists',
                                         formatter_class=CustomFormatter)
    parser_clean.add_argument('directory', nargs='?', action='store',
                              default='{0}'.format(get_cached_option('directory')),
                              metavar='DIR', help='Specify top level output directory')
    parser_clean.set_defaults(func=clean)



    # parse arguments - automatically selects appropriate function to call
    args = parser.parse_args()
    # if config is supposed to exist, but could not be loaded exit program
    if (args.config == 'No cache exists yet') if args.__contains__('config') else False:
        raise Exception('No cache exists - Specify output directory and config')

    # call function associated with the respective mode
    args.func(args)
    return 0


if __name__=='__main__':
    sys.exit(main())
