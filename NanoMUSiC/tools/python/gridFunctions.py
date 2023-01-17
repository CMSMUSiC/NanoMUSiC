#!/usr/bin/env python
## @package gridFunctions
# This module provides common functions for interaction with the local Tier2
#
import os
import sys
import subprocess
import time

def getSRMPrefix( site ):
    ''' Function which returns the SRM prefix for a given site (everything before /store in PFN for SRM).
    Raises a ValueError if site can't be found.
    '''
    #TODO this should use the siteconf to extract paths
    if site == "T2_DE_RWTH":
        return "srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2?SFN=/pnfs/physik.rwth-aachen.de/cms/"
    if site == "T2_DE_DESY":
        return "srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/cms/tier2/"
    raise ValueError( "Invalid storage site: '%s'" )

def getCeId( site ):
    ''' Function which returns the ID of the computing element for a site.
    Raises a ValueError if site can't be found.
    '''
    #TODO What about grid-cr1,2,3 for DESY?
    if site == "T2_DE_RWTH":
        return 'grid-ce.physik.rwth-aachen.de:8443/cream-pbs-cms'
    if site == "T2_DE_DESY":
        return 'grid-cr0.desy.de:8443/cream-pbs-cms'

    raise ValueError( "Invalid computing site: '%s'" )

def testSRM( site, username ):
    ''' Function to check if known existing file can be found
    pfn_suffix is a srm path until but excluding /store/
    e.g. srm://grid-srm.physik.rwth-aachen.de:8443/pnfs/physik.rwth-aachen.de/cms/
    '''

    pfn_prefix = getSRMPrefix( site )
    if not pfn_prefix:
        print "No SRM prefix found for site %s" % site
        return False

    remote_path = "%s%s" % ( pfn_prefix, os.path.join("/store/user/", username ,"deleteme_srm") )
    # try to copy a file via SRM

    cmd = "srmcp -2 -debug=true file:////bin/bash %s" % remote_path
    #print cmd
    p = subprocess.Popen( cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True )
    (string_out,string_err) = p.communicate()
    if p.returncode != 0:
        print string_err
        return False
    # try to copy file back
    cmd = "srmcp -2 -debug=true %s file:////tmp/deleteme"
    if p.returncode != 0:
        print string_err
        return False
    # try to delete testfile
    cmd ="srmrm %s" % remote_path
    p = subprocess.Popen( cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True )
    (string_out,string_err) = p.communicate()
    if p.returncode != 0:
        print string_err
        return False
    return True


## Get list of files and directories in dCap folder
# @type directory: string
# @param directory: The dCap folder without /pnfs/physik.rwth-aachen.de/cms/store/user/
def uberls(directory):
    cmd_readdcache = ["uberftp","grid-ftp.physik.rwth-aachen.de",r"ls /pnfs/physik.rwth-aachen.de/cms/store/user/%s" % (directory)]
    try:
        p = subprocess.Popen(cmd_readdcache,stdout=subprocess.PIPE)
        (stringdcache,stringdcache_err) = p.communicate()
        dcachelistraw = stringdcache.split("\n")
    except:
        # try again after 10 seconds if first try failed
        time.sleep(10)
        p = subprocess.Popen(cmd_readdcache,stdout=subprocess.PIPE)
        (stringdcache,stringdcache_err) = p.communicate()
        dcachelistraw = stringdcache.split("\n")
    filelist = []
    for line in dcachelistraw :
        infos = line.split()
        if len(infos)!=9: continue
        filelist.append("dcap://grid-dcap.physik.rwth-aachen.de/pnfs/physik.rwth-aachen.de/cms/store/user/{0}/{1}".format(directory, infos[8]))
    return filelist

## Copy a file / folder to the dCache
# @type source: string
# @param source: The local path to the file / folder name. If you want to copy a folder, the path has to end with an '/'.
# @type target: string
# @param target: The dCap folder without /pnfs/physik.rwth-aachen.de/cms/store/user/. The folder has to end with an '/'
def cp(source, target):
    full_target_path     = os.path.join( "gsiftp://grid-ftp/pnfs/physik.rwth-aachen.de/cms/store/user/", target )
    full_target_path_srm = os.path.join( "srm://grid-srm.physik.rwth-aachen.de:8443/pnfs/physik.rwth-aachen.de/cms/store/user/", target )
    full_source_path     = os.path.abspath( source )
    # os.path.abspath removes the trailing "/", but if you try to copy a folder the "/" is needed.
    if source [ -1 ] == "/":
        full_source_path += "/"

    # test if target is folder
    if full_target_path[ -1 ] == "/":

        # if target is folder, test if it exists
        folder_not_exist = 0
        add_to_folder = "n"

        try:
            fileinfos( target )
        except FileNotFound:
            folder_not_exist = 1

        if folder_not_exist:
            cmd_mkdir = ["srmmkdir", full_target_path_srm ]
            p = subprocess.Popen(cmd_mkdir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            (stdout, stderr) = p.communicate()
            print stderr
        else:
            add_to_folder = raw_input( "Folder already exist. Do you want to add the files to the existing folder? (y,[n]): " )

        if add_to_folder == "y":
            pass
        else:
            sys.exit( 0 )

    cmd = ["globus-url-copy", "-r", "-v", "-fast", "-p", "6", full_source_path, full_target_path ]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    print stderr
    return

## Copy a folder to the dCache
# @type source: string
# @param source: The local path to the folder name
# @type target: string
# @param target: The dCap folder without /pnfs/physik.rwth-aachen.de/cms/store/user/
def cpFolder(source, target):
    files = os.listdir( source )

    # test if folder already exists
    folder_not_exist = 0
    try:
        fileinfos( target )
    except FileNotFound:
        folder_not_exist = 1

    if folder_not_exist:
        for file in files:
            cp( file, target )
    else:
        print "Folder already exists. Delete first!"

    return




class FileNotFound(Exception):
    pass
## Get a dictionary of detailed file information of a remote file
# @type source: string
# @param source: The dCap file path without /pnfs/physik.rwth-aachen.de/cms/store/user/
def fileinfos( source, site='T2_DE_RWTH' ):
    cmd = [ "srmls", "-l", getSRMPrefix( site ) + '/store/user/' +  source ]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    infos=dict()
    if "SRM_INVALID_PATH" in stderr and "does not exist" in stderr:
        raise FileNotFound
    for line in stdout.splitlines()[1:]:
        kv = line.split(":",1)
        try:
            key=kv[0].strip("- ")
            value=kv[1].strip()
            infos[key]=value
        except IndexError:
            pass
    return infos

## Get the adler32 hash value of a dCache file
# @type source: string
# @param source: The dCap file path without /pnfs/physik.rwth-aachen.de/cms/store/user/
def adler32( source, site='T2_DE_RWTH' ):
    infos=fileinfos( source, site )
    try:
        if infos["Checksum type"]!="adler32": return None
    except KeyError:
        return None
    return infos["Checksum value"]

## Get list of files with certain file extension in dCap folder recursively
# @type dir: string
# @param dir: The dCap folder without /pnfs/physik.rwth-aachen.de/cms/store/user/
# @type mem_limit: int
# @param mem_limit: Maximum summed filesize before files are splitted in sublists [default:500000000]
# @param fileXtension: files you want to have [default:.pxlio]
# @param site: specify the side you want to run on [default:T2_DE_RWTH]
def getdcachelist(dir , Tag='' , mem_limit = 500000000, fileXtension= '.pxlio', site="T2_DE_RWTH"):
    # try to run ls -r query with uberftp in specified folder

    srmprefix=getSRMPrefix( site )
    cmd_readdcache="lcg-ls -l  %sstore/user/%s"%(srmprefix,dir)
    #cmd_readdcache=cmd_readdcache.split(" ")
    p = subprocess.Popen(cmd_readdcache,stdout=subprocess.PIPE,shell=True)
    (stringdcache,stringdcache_err) = p.communicate()
    if p.returncode!=0:
        print cmd_readdcache
        print stringdcache
        print stringdcache_err
        return []
    dcachelistraw = stringdcache.split("\n")

    folderList = filter(lambda line:"drwxrwxr-x" in line in line, dcachelistraw)
    done_folderList=[]
    while len(folderList)>0:
        folderList_new=[]
        for folder in folderList:
            if folder in done_folderList:
                continue
            cmd_readdcache=r"lcg-ls -l  %sstore/%s"%(srmprefix,formatPath( folder, site, False ).split( 'store/' ) [-1])
            p = subprocess.Popen(cmd_readdcache,stdout=subprocess.PIPE,shell=True)
            (stringdcache,stringdcache_err) = p.communicate()
            if p.returncode!=0:
                print "folder ls"
                print cmd_readdcache
                print stringdcache
                print stringdcache_err
                return []
            dcachelistraw += stringdcache.split("\n")
            done_folderList.append(folder)
            folderList_new+= filter(lambda line:"drwxrwxr-x" in line in line, dcachelistraw)
        folderList=folderList_new
    # filter list of returned files in subfolders for files with specified file extension
    dcachelistraw=set(dcachelistraw)
    dcachelistraw = filter(lambda line:fileXtension in line in line, dcachelistraw)
    dcachelistraw = filter(lambda line:Tag in line in line, dcachelistraw)

    filelistlist = []

    filelistlist.append([])
    memory = 0

    l = 1
    # split samples in sublists of specified sumed file size and add dcap prefix
    if len(dcachelistraw)==1:
        filelistlist[-1].append(formatPath( dcachelistraw[0].split()[6], site, site=="T2_DE_RWTH" ))
        return filelistlist
    for tmpstring in dcachelistraw :
        memory += int(tmpstring.split()[4])
        if memory>mem_limit:
            filelistlist.append([])
            memory = 0
            l+=1
        filelistlist[-1].append(formatPath( tmpstring.split()[6], site, site=="T2_DE_RWTH" ))
    if len(filelistlist[-1]) == 0:
      filelistlist.pop()
    filelistlist=filter(lambda x: len(x)>0, filelistlist)
    return filelistlist

## Returns the life time left. for a proxy.
#
#@return int time left for the proxy
def timeLeftVomsProxy():
    proc = subprocess.Popen( ['voms-proxy-info', '-timeleft' ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
    output = proc.communicate()[0]
    if proc.returncode != 0:
        return False
    else:
        return int( output )

## Checks if the proxy is valid longer than time
#
#@type time: int
#@param time: reference time in seconds to check against
#@return boolean returns True if the proxy is valid longer than time, False otherwise.
def checkVomsProxy( time=86400 ):
    timeleft = timeLeftVomsProxy()
    return timeleft > time

## Creates a new vom proxy
#
# This function creates a new
#@type voms: string
#@param voms: the voms group used to set up the server [default:cms:/cms/dcms]
#@type passphrase: string
#@param passphrase: Passphrase for GRID certificate [default:none]. The password request is send to the prompt if no passphrase given
def renewVomsProxy( voms='cms:/cms/dcms', passphrase=None ):
    """Make a new proxy with a lifetime of one week."""
    if passphrase:
        p = subprocess.Popen(['voms-proxy-init', '--voms', voms, '--valid', '192:00'], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
        stdout = p.communicate(input=passphrase+'\n')[0]
        retcode = p.returncode
        if not retcode == 0:
            raise ProxyError( 'Proxy initialization command failed: %s'%stdout )
    else:
        retcode = subprocess.call( ['voms-proxy-init', '--voms', voms, '--valid', '192:00'] )
    if not retcode == 0:
        raise ProxyError( 'Proxy initialization command failed.')

## Checks if the proxy is valid longer than time and renew if needed.
#
#@type time: int
#@param time: reference time in seconds to check against
#@param voms: the voms group used to set up the server [default:cms:/cms/dcms]
#@type passphrase: string
#@param passphrase: Passphrase for GRID certificate [default:none]. The password request is send to the prompt if no passphrase given
def checkAndRenewVomsProxy( time=604800, voms='cms:/cms/dcms', passphrase=None ):
    if not checkVomsProxy( time ):
        renewVomsProxy(passphrase=passphrase)
        if not checkVomsProxy( time ):
            raise ProxyError( 'Proxy still not valid long enough!' )

def formatPath( path, site, useDcap ):
    """ Function to ensure correct file path format.
    If we use the DCAP protocol, we need to produce a full URL.
    If we do not use DCAP, a path starting including /pnfs/ is used for direct access.
    """
    # Throw away everything before /user/
    # This is just to simulate the upcoming change in database paths.
    # TODO: Remove this once the paths in the database are changed.
    newPath = path.split( 'store/' ) [-1]

    if( useDcap ):
        newPath = 'dcap://grid-dcap.physik.rwth-aachen.de/pnfs/physik.rwth-aachen.de/cms/store/' + newPath
    else:
        newPath = getSRMPrefix( site ).split( 'SFN=' )[1] + '/store/' + newPath

    return newPath
