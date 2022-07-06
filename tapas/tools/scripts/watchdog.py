#!/usr/bin/env python
import os,sys,glob
import tarfile
import xml.etree.ElementTree as ET
import subprocess
import imp
import time
import datetime
import argparse
import logging

import gridlib.util

from multiprocessing import Process

from music_crab3 import readSampleFile as readMusicCrabConfig

import crabFunctions
import loggingFunctions
from gridlib.util import VomsProxy
import dbutilscms
import aachen3adb

from CRABClient.UserUtilities import getConsoleLogLevel, setConsoleLogLevel
from CRABClient.ClientUtilities import LOGLEVEL_MUTE

# some general definitions
COMMENT_CHAR = '#'

# Write everything into a log file in the directory you are submitting from.
log = logging.getLogger( 'watchdog' )

def commandline_parsing():
    descr = 'Simple Tool for crab job monitoring and submission of metainformation to aix3adb after completion'
    parser = argparse.ArgumentParser(description= descr, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-f' ,'--noFinalize', action='store_true', help='Do not finalize (get metainfo and submit to aix3adb)')
    parser.add_argument('-r' ,'--rFailed',    action='store_true', help='resubmit failed tasks')
    parser.add_argument('-i', '--kIdle',     action='store_true', help='Try to kill stuck idle jobs')
    parser.add_argument('-u', '--update',    action='store_true', help='Update latest skim instead of creating a new one. This sets the --ignoreComplete option true')
    parser.add_argument('-D', '--no-db',    action='store_true', help=' Do not inject samples to db')
    parser.add_argument('-c', '--continous',    action='store_true', help='Run continously until no non final task is found.' \
                                                                          'Adds folders which are added after statup')
    parser.add_argument( '-p','--ask-passphrase', action='store_true', help='Ask for passphrase and store it for proxy renwal')
    parser.add_argument('--db-key', default='~/private/CERN_UserCertificate.key',
                       help="Key for authentication to 'https://cms-project-aachen3a-db.web.cern.ch'. [default: %(default)s]" )
    parser.add_argument( '--db-cert', default='~/private/CERN_UserCertificate.pem',
                       help="Cert for authentication to 'https://cms-project-aachen3a-db.web.cern.ch'. [default: %(default)s]" )
    parser.add_argument('-m', '--musicCrab3Input',  metavar='FILES', type=str, nargs='+',
                   help='A list of music_crab input files. Gives summary of clompetness')
    parser.add_argument('--ignoreComplete',  action='store_true', help='Do not skip previously finalized samples')
    parser.add_argument('--addIncomplete',  action='store_true', help='Submit all samples to db, even if they are not finished')
    parser.add_argument('--only', action='store', dest='folder_tag', default='', help='Only loop over folder containing a string')
    parser.add_argument('--debug', metavar='LEVEL', default='WARNING', choices=loggingFunctions.LOG_CHOICES,
                       help='Set the debug level. Allowed values: ' + ', '.join( loggingFunctions.LOG_CHOICES ))
    args = parser.parse_args()

    loggingFunctions.setupLogging( log, args.debug, "watchdog.log" )

    if args.update: args.ignoreComplete = True


    # check if user has valid proxy
    import getpass
    proxy = gridlib.util.VomsProxy()
    expired = False
    min_lifetime = 300
    if proxy.timeleft < min_lifetime:
        print("Your proxy is valid for only %d seconds.")
        expired = True
    if args.ask_passphrase or expired:
        proxy.passphrase = getpass.getpass('Please enter your GRID pass phrase for proxy renewal:')
        proxy.ensure(min_lifetime)
    args.proxy = proxy

    if args.musicCrab3Input:
        args.maxJobRuntimeMin = -1
        args.config = ""
        args.config_dir = ""
        args.unitsPerJob = -1

    #~ args.user = crab.checkusername()
    return args

def createDBlink(args):
    # Create a database object.
    dblink = aachen3adb.ObjectDatabaseConnection(key=args.db_key,
                                                 certificate=args.db_cert)
    dblink.authorize()
    return dblink


def main():
    args = commandline_parsing()
    crab = crabFunctions.CrabController()
    dblink = createDBlink(args)
    i = 0
    print "Starting watchdog"
    updated = True
    while updated:
        updated = False
        start = time.time()
        # renew proxy if nescessarry
        if args.ask_passphrase:
            args.proxy.ensure(60)
        # check for new crab folders in run area
        crabSamples = [crabFolder.replace('crab_','') for crabFolder in crab.crabFolders]
        if args.folder_tag!='':
            crabSamples = filter(lambda x: args.folder_tag in x, crabSamples)
            if not crabSamples:
                log.error('Found no folders with tag {:s}. Exiting.'.format(args.folder_tag))
                sys.exit(1)
        for sample in crabSamples:
            task = crabFunctions.CrabTask(sample,
                                          initUpdate=False,
                                          proxy=args.proxy,
                                          dblink=dblink,
                                          debuglevel=args.debug)
            if task.inDB and task.isFinal:
                if not args.ignoreComplete:
                    continue
                task.state = "FINAL"
            print "Updated is true"
            updated = True and args.continous
            task.update( (not args.no_db) )
            if i % 50 == 0:
                print '{:<90} {:<6} {:<12} {:<8} {:<8} {:<8} {:<8} {:<8}'.format('Sample',
                                                                                 'SkimID',
                                                                                 'State',
                                                                                 'Running',
                                                                                 'Idle',
                                                                                 'Failed',
                                                                                 'Transfer',
                                                                                 'Finished')
            i+=1
            if ("COMPLETED" == task.state and not args.noFinalize)\
                or ("FINAL" == task.state and args.update)\
                or args.addIncomplete:
                    log.info( "Trying to finalize sample" )
                    task.finalizeTask()
                #~ finalizeSample( sample, dblink, crabConfig, args )
            if args.rFailed and task.nFailed > 0:
                task.resubmit_failed()
                task.update((not args.no_db))
            if  args.kIdle and "SUBMITTED" == task.state and task.nRunning < 1 and task.nIdle > 0 and task.nTransferring <1:
                idlejobs = [id for id in task.jobs.keys() if "idle" in task.jobs[id]['State']]
                idlejobs = ','.join(idlejobs)
                log.info( "IDLE:" + idlejobs)
                cmd = 'crab kill --jobids=%s %s' %(idlejobs,crab._prepareFoldername(sample))
                p = subprocess.Popen(cmd,stdout=subprocess.PIPE, shell=True)#,shell=True,universal_newlines=True)
                (out,err) = p.communicate()
                log.debug( "Output from kill" )
                log.debug( "cout: " + out )
                log.debug( "cerr: " + err )

            print '{:<90} {:<6} {:<12} {:<8} {:<8} {:<8} {:<8} {:<8}'.format(sample,
                                                                             task.dbSkim.id,
                                                                             task.state,
                                                                             str(task.nRunning),
                                                                             str(task.nIdle),
                                                                             str(task.nFailed),
                                                                             str(task.nTransferring),
                                                                             str(task.nFinished))
            if updated: # wait atleast 3mins second between updates for same sample
                time.sleep(max(0,180 - time.time() + start))
    print "No samples to check, stopping watchdog"

if __name__ == '__main__':
    main()
