#! /usr/bin/env python2
from __future__ import division
import logging
logging.basicConfig(level=logging.WARNING)
logger = logging.getLogger(__name__)
import cesubmit
import os
import sys
import shutil
import time
import glob
import subprocess
import ConfigParser
import optparse
import aix3adb
from  aix3adb import Aix3adbException
import math
import shlex
import tarfile
import gridFunctions

def main():
    parser = optparse.OptionParser(usage="usage: %prog [options] arguments")
    groupRun = optparse.OptionGroup(parser, "Run mode Options")
    groupRun.add_option("-p", "--prepare", action="store_true", help="Prepare television tasks (default)", default=True)
    groupRun.add_option("-m", "--merge", action="store_true", help="Merge output files", default=False)
    parser.add_option_group(groupRun)

    groupCommon = optparse.OptionGroup(parser, "Common Options")
    parser.add_option_group(groupCommon)

    groupMerge = optparse.OptionGroup(parser, "Merge Options","Specify the television directories as arguments.")
    parser.add_option_group(groupMerge)

    groupPrepare = optparse.OptionGroup(parser, "Prepare Options","Specify one config file as argument")
    groupPrepare.add_option("-d", "--directory", action="store", help="Main television directory", default="./")
    groupPrepare.add_option("--section", action="append", help="Only prepare the following section. May be specified multiple times for multiple sections. If not specified, all sections are prepared.", default=[])
    groupPrepare.add_option("--test", action="store_true", help="Run one task per section with one small job only.", default=False)
    groupPrepare.add_option("--prepareConfigs", default = 'None',
                            choices=['None','MUSiC'],
                            help="Create config Files on the fly for given config style")
    groupPrepare.add_option("--useListFiles", action = "store_true", default=False,
                            help="Do not use fileslist from aix3adb but check on dCache instead")
    groupPrepare.add_option("--outLFNDirBase", default=None,
                            help="Alternative base directory for files if useListFiles is used")
    groupPrepare.add_option("--forceOverwrite", action="store_true", default = False,
                            help="Do not ask before replacing existing folders")

    #groupPrepare.add_option("--local", action="store_true", help="Run the tasks on local computer.", default=False)
    #groupPrepare.add_option("--testlocal", action="store_true", help="Run only one task with one small job locally.", default=False)
    groupPrepare.add_option("-s", "--skipcheck", action="store_true", help="Skip check if grid pack is outdated.", default=False)
    parser.add_option_group(groupPrepare)
    (options, args) = parser.parse_args()
    gridFunctions.checkAndRenewVomsProxy()
    if options.merge:
        merge(options, args)
    elif options.prepare:
        prepare(options, args)

def readConfig(options,args):
    dblink = aix3adb.aix3adb()
    config = MyConfigParser()
    config.read(args[0])

    file_options     = ['outputfiles',
                        'inputfiles',
                        "gridpackfiles",
                        'executable',
                        "localgridpackdirectory",
                        "localgridtarfile",
                        "remotegridtarfile"]

    other_options = [ "cmssw",
                    "gridoutputfiles",
                   "maxeventsoption",
                   "skipeventsoption",
                   "eventsperjob",
                   "filesperjob",
                   "datasections",
                   "useDcap" ]

    optionNames = other_options + file_options
    # Add options which are only present in certain cases
    try:
        setattr( options, 'site', config.get( "DEFAULT", 'site' ) )
    except ConfigParser.NoOptionError:
        options.site='T2_DE_RWTH'

    if not options.prepareConfigs == "None":
        optionNames.append( 'configdir' )
    for option in optionNames:
        setattr( options, option, config.get("DEFAULT",option) )
    try:
        options.lumi = int( config.get('DEFAULT', 'lumi') )
    except:pass

    try:
        options.useDcap = ( options.useDcap.lower() == 'true' or int( options.useDcap ) == 1 )
    except ConfigParser.NoOptionError:
        options.useDcap = True

    # replace environment variables
    for option in file_options:
        setattr( options, option, os.path.expandvars( getattr( options, option) ) )

    try:
        options.gridoutputfiles = options.gridoutputfiles.split(" ")
    except:
        print "error splitting gridoutputfiles"
    try:
        options.datasections = options.datasections.split(" ")
    except:
        print "error splitting datasections"
    # parse some infos to other types
    try:

        options.eventsperjob = int(options.eventsperjob)
    except:
        try:
            options.filesperjob = int(options.filesperjob)
        except:
            print "Error converting some files / rvents per job to int"
    sectionlist = {}
    for section in config.sections():
        if section in ["DEFAULT"]: continue
        if section != [] and section not in section: continue
        mc = not (section in options.datasections)
        identifiers=config.optionsNoDefault(section)
        sectionlist.update( {section:[]} )
        for identifier in identifiers:
            arguments = shlex.split(config.get(section, identifier))
            try:
                if identifier[0]=="/":
                    #it's a datasetpath
                    if mc:
                        skim, sample = dblink.getMCLatestSkimAndSampleByDatasetpath(identifier)
                    else:
                        skim, sample = dblink.getDataLatestSkimAndSampleByDatasetpath(identifier)
                elif identifier.isdigit():
                    #it's a skim id
                    if mc:
                        skim, sample = dblink.getMCSkimAndSampleBySkim(identifier)
                    else:
                        skim, sample = dblink.getDataSkimAndSampleBySkim(identifier)
                else:
                    #it's a sample name
                    if mc:
                        skim, sample = dblink.getMCLatestSkimAndSampleBySample(identifier)
                    else:
                        skim, sample = dblink.getDataLatestSkimAndSampleBySample(identifier)
            except Aix3adbException:
                print "Can not find sample and skim for identifier %s" % str(identifier)
                sys.exit(1)
            sectionlist[ section ].append( ( skim, sample, arguments) )

    return sectionlist

def prepare(options, args):

    skimlist = readConfig( options, args )
    if not options.skipcheck:
        checkGridpack( options.localgridtarfile,
                       options.remotegridtarfile,
                       options.localgridpackdirectory,
                       options.gridpackfiles,
                       options.site )
    dblink = aix3adb.aix3adb()

    if not options.prepareConfigs == "None":
        prepareConfigs( skimlist, options  )

    #~ for ( skim, sample, arguments ) in skimlist:
    for section in skimlist.keys():
        print "Preparing tasks for section {0}.".format(section)
        print "Found {0} tasks.".format(len( skimlist[ section ] ))
        for ( skim, sample, arguments ) in skimlist[ section ]:
            makeTask( options, skim, sample, section, arguments )
        if options.test:
            break

def merge(options, args):
    print "Merging..."
    for directory in args:
        print directory
        rootfiles = set([os.path.basename(f) for f in glob.glob(os.path.join(directory, "grid*","*.root"))])
        for outfilename in rootfiles:
            joinfilenames=glob.glob(os.path.join(directory,"grid*",outfilename))
            mergedfilename=os.path.join(directory,"merged_"+outfilename)
            cmd = ["hadd","-f",mergedfilename]+joinfilenames
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
            (stdout, stderr) = p.communicate()

def makeTask(options, skim, sample, section, arguments):
    basedir = options.directory
    name = sample.name+"-skimid"+str(skim.id)
    print "Preparing task", name
    if options.cmssw:
        cmssw = options.cmssw
    else:
        cmssw = True
    directory = os.path.join(basedir,section,name)
    if os.path.exists( directory ):
        if options.forceOverwrite:
            shutil.rmtree( directory )
        elif ask( "Folder for task exists. Do you want to replace it? [y/n]" ):
            shutil.rmtree( directory )
    task=cesubmit.Task( name, directory=directory, cmsswVersion=cmssw, mode="CREATE", ceId=gridFunctions.getCeId( options.site ) )
    task.executable=options.executable
    task.uploadexecutable=False
    task.outputfiles.extend( shlex.split( options.outputfiles ) )
    task.inputfiles.extend( expandFiles("", options.inputfiles ) )
    task.addGridPack( options.remotegridtarfile, uploadsite=( gridFunctions.getSRMPrefix( options.site ) + '/store/user/{username}/' ) )


    for gridoutputfile in options.gridoutputfiles:
        task.copyResultsToDCache( gridoutputfile, uploadsite=( gridFunctions.getSRMPrefix( options.site ) + '/store/user/{username}/' ) )
    runfiles = prepareFileList(skim, sample, options)
    jobchunks=getJobChunks( runfiles, options )
    print "Number of jobs: ", len(jobchunks)
    for chunk in jobchunks:
        job=cesubmit.Job()
        job.arguments.extend(arguments)
        job.arguments.extend(chunk)
        task.addJob(job)
    print "Submitting..."
    task.submit(6)
    print "[Done]"

## Determines the adler32 check sum of a file
#
#@type path: string
#@param path: The path to the file.
#@return string The adler32 check sum value.
def adler32(path):
    cmd = ["adler32", path]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    return stdout.strip()

## Asks a yes/no question at the prompt
#
#@type question: string
#@param question: The question to ask
#@return boolean returns True if the user answered y, False if the user answered n.
def ask(question):
    while True:
        print question
        answer = raw_input("[y/n]")
        if answer in "yY":
            return True
        elif answer in "nN":
            return False

## This function prepares a list of pxlio files depending on the chosen options
#
# @param A aix3adb skim object
# @param Program wide options saved in Option class from optparse
# @return A list of dicts as used in the skim.files field
def prepareFileList(skim, sample, options):

    filelist = []
    if not options.useListFiles:
        for f in skim.files:
            filelist.append( { 'path' : gridFunctions.formatPath( path=f['path'], site=options.site, useDcap=options.useDcap ),
                               'nevents' : f['nevents'] } )
    else:
        if options.outLFNDirBase is not None:
            outlfn = os.path.join( options.outLFNDirBase, sample.name )
        else:
            outlfn = os.path.join( skim.owner, 'PxlSkim', skim.skimmer_version, sample.name )
        print "use outlfn %s" % outlfn
        dCacheFiles = []
        for flist in gridFunctions.getdcachelist( outlfn ) :
            dCacheFiles += flist
        #format to correct list format as used by skim objects
        filelist = [ {'path': gridFunctions.formatPath( path=dfile, site=options.site, useDcap=options.useDcap ), 'nevents':-1} for dfile in dCacheFiles]

    return filelist

## Check a grid pack and asks if it should be updated
#
# Two checks are performed. (1) the timestamp of all files that should enter the grid pack
# is compared to the local grid pack timestamp. If at least one file is newer than the grid pack,
# it is suggested to upgrade the grid pack. (2) The adler32 check sum of the remote grid pack
# is compared to the local grid pack. If they differ or no remote grid pack is found, it is suggested
# to upload the grid pack.
#@type local: string
#@param local: The local path where the grid tar.gz file is stored.
#@type remote: string
#@param remote: The remote path where the grid tar.gz file is stored.
#@type gpdir: string
#@param gpdir: The local directory containing all the files and subdirectories that are part of the gridpack.
#@type gpfilestring: string.
#@param gpfilestring: A string describing all files that should be contained in the grid pack. Usual shell syntax, such as astersiks is allowed. Directories are included recursively.
def checkGridpack( local, remote, gpdir, gpfilestring, site ):
    print "Comparing gridpack files timestamps..."
    gppaths = expandFiles(gpdir, gpfilestring)
    # check time stamps
    outdated = False
    createnew = False
    try:
        timestampzipped = os.path.getmtime(local)
    except OSError:
        print "Local grid pack file does not exist. Creating..."
        createnew=True
    if not createnew:
        for path in gppaths:
            if os.path.getmtime(path)>timestampzipped:
                print "Found outdated file",path
                outdated=True
        if outdated:
            if ask("Local grid pack zip file is outdated. Would you like to create a current version?"):
                createnew=True
    if createnew:
        createGridpack(local, gpdir, gpfilestring)
    # check hashes
    try:
        adlerRemote = gridFunctions.adler32( os.path.join( cesubmit.getCernUserName(), remote ), site )
    except gridFunctions.FileNotFound:
        print "Remote gridpack not found"
        adlerRemote = None
    adlerLocal = adler32(local)
    if adlerLocal!=adlerRemote:
        if ask("Remote gridpack differs from local gridpack. Do you want to upload the current local gridpack now?"):
            cesubmit.uploadGridPack( local, remote, uploadsite=( gridFunctions.getSRMPrefix( site ) + '/store/user/{username}/' ) )

## Creates a local grid pack tar.gz file
#
#@type localpath: string
#@param localpath: The local path where the grid tar.gz file is stored.
#@type gpdir: string
#@param gpdir: The local directory containing all the files and subdirectories that are part of the gridpack.
#@type gpfilestring: string.
#@param gpfilestring: A string describing all files that should be contained in the grid pack. Usual shell syntax, such as astersiks is allowed. Directories are included recursively.
def createGridpack(localpath, gpdir, gpfilestring):
    gppaths = expandFiles(gpdir, gpfilestring)
    tar = tarfile.open(localpath, "w:gz")
    for name in gppaths:
        if gpdir in name:
            extractname=name[len(gpdir):].lstrip("/")
        else:
            extractname=os.path.basename( os.path.normpath( name) )
        tar.add(name, extractname)
    tar.close()

## Returns a list of paths from a base directory and a shell conform list of subpath
#
# The function prepends the base directory and resolves wildcards
#@type gpdir: string
#@param gpdir: The local directory containing all the files and subdirectories that are part of the gridpack.
#@type gpfilestring: string.
#@param gpfilestring: A string describing all files that should be contained in the grid pack. Usual shell syntax, such as astersiks is allowed. Directories are included recursively.
#@return: A list of all paths corresponding to the input
def expandFiles(gpdir, gpfilestring):
    gpfiles = shlex.split(gpfilestring)
    # get a list of all filepaths with realtive path to basedir (resolving asterisks and joining the basedir to the path)
    gppaths = [filename for sublist in [glob.glob(p) for p in [os.path.join(gpdir,f) for f in gpfiles if not os.path.isabs( f )] ] for filename in sublist]
    # Add a list of all filepaths with absolute path to basedir (resolving asterisks and joining the basedir to the path)
    gppaths += [filename for sublist in [glob.glob(p) for p in [ f for f in gpfiles if os.path.isabs( f )] ] for filename in sublist]
    return gppaths

def getJobChunks(files, options):
    if options.test:
        return [ [options.maxeventsoption, "100", "--PXLIO_FILE" , files[0]['path'] ]]
    if options.eventsperjob and not options.filesperjob:
        result = determineJobChunksByEvents(files, options.eventsperjob)
        return [ [ options.maxeventsoption,
                   str(options.eventsperjob),
                   options.skipeventsoption,
                   str(skip)]+x
                   for (skip, x) in result]
    elif options.filesperjob:
        result = determineJobChunksByFiles(files, options.filesperjob)
        return result
    raise Exception("Please specify either eventsperjob or filesperjob")

def determineJobChunksByFiles(files, filesperjob):
    filenames = [f['path'] for f in files]
    return list(chunks(filenames, filesperjob))

def determineJobChunksByEvents(files, eventsperjob):
    events=[int(f['nevents']) for f in files]
    cummulativeHigh=list(cummulative(events))
    cummulativeLow=[0]+cummulativeHigh[:-1]
    totalevents=cummulativeHigh[-1]
    result=[]
    for i in xrange(int(math.ceil(totalevents/eventsperjob))):
        skipEventsGlobal=eventsperjob*i
        skipEventsLocal=getSkipEventsLocal(cummulativeLow, skipEventsGlobal)
        filenames=getFileList(files, cummulativeHigh, cummulativeLow, skipEventsGlobal, eventsperjob)
        result.append((skipEventsLocal,filenames))
    return result

def getFileList(files, cummulativeHigh, cummulativeLow, skipEventsGlobal, eventsperjob):
    filenames = [files[i]['path'] for i in xrange(len(files)) if skipEventsGlobal<=cummulativeHigh[i] and skipEventsGlobal+eventsperjob>cummulativeLow[i]]
    return filenames

def getSkipEventsLocal(cummulativeLow, skipEventsGlobal):
    skipEventsLocal=0
    for i in xrange(len(cummulativeLow)):
        if skipEventsGlobal>=cummulativeLow[i]:
            skipEventsLocal=skipEventsGlobal-cummulativeLow[i]
        else:
            return skipEventsLocal
    return skipEventsLocal

## This function may be used to implement dynamic creation of config files during submission
#
# Currently only MUSiC configs are implemented
#@param options A ConfigOptions object holding current script wide options
#@param args A list of additional command line arguments
#@param A list of skims and samples as created by readConfig
def prepareConfigs( skimlist, options ):
    if options.prepareConfigs == "MUSiC":
        from aix3adb2music import getConfigDicts,writeConfigDicts,flattenRemoteSkimDict
        scalesdict, playlistdict = getConfigDicts( flattenRemoteSkimDict( skimlist , options.datasections ) )
        writeConfigDicts( scalesdict, playlistdict , lumi=options.lumi )

## Yield successive n-sized chunks from l. The last chunk may be smaller than n.
#
#@type l: iterable
#@param l: The iterable from which chunks are determiend.
#@type l: iterable
#@param l: The iterable from which chunks are determiend.
#@return A list of chunks from l.
def chunks(l, n):
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

def cummulative(l):
    """Yield cummulative sums."""
    n=0
    for i in xrange(len(l)):
        n+=l[i]
        yield n

class MyConfigParser(ConfigParser.SafeConfigParser):
    """Can get options() without defaults
    """
    def optionsNoDefault(self, section):
        """Return a list of option names for the given section name."""
        try:
            opts = self._sections[section].copy()
        except KeyError:
            raise NoSectionError(section)
        if '__name__' in opts:
            del opts['__name__']
        return opts.keys()
    def optionxform(self, optionstr):
        return optionstr

if __name__=="__main__":
    main()
