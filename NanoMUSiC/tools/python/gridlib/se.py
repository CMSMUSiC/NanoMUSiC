#!/usr/bin/env python
# Copyright [2016] <III Phys. Inst A, RWTH Aachen University>

# absolute imports
from __future__ import absolute_import

# standard library imports
import sys
import os
import hashlib
import logging
import collections
import tarfile
import xml.etree.ElementTree
import subprocess
import tempfile
import fnmatch
import stat
import re
import urlparse
from itertools import repeat
import time

# gridlib imports
import gridlib.util
import gridlib.gfal2context

# setup logging
log = logging.getLogger(__name__)

## Shell command template
#
# Made for the multitude of shell commands to be called in the context of se.py.
#
# @param command Command which to execute
# @param exception Exception which to throw in case of an invalid returncode
def sh_command(command, exception=None):
    # call command
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    stdout, stderr = process.communicate()
    # evaluate output
    if process.returncode is not 0 or 'ERROR' in stdout or 'MISSING' in stdout:
        # extend error message by replacing tuple
        exception.args = ('{0}\nReturncode: {1}\n{2}\n{3}'.format(
            exception.args[0],
            process.returncode,
            stdout,
            stderr
        ),)
        raise exception
    return stdout


class StorageElement(object):
    def __init__(self, site='T2_DE_RWTH'):
        if not site:
            raise ValueError('Site must be specified')
        lock_path = "/tmp/.storage_element_access_%s" % site
        if os.path.exists(lock_path):
            last_access = os.path.getmtime(lock_path)
            while time.time() - last_access < 2:
                last_access = os.path.getmtime(lock_path)
                time.sleep(1)
            with open(lock_path, "wb") as f:
                f.write("")
        # set variables
        self.site_config = None
        self.set_site(site)

    def __getstate__(self):
        pickle_blacklist = [
            '_gfal',
        ]
        state = {}
        for name, value in self.__dict__.iteritems():
            if name not in pickle_blacklist:
                state[name] = value
        return state

    @property
    def gfal(self):
        # for full list of options see configuration files in /etc/gfal2.d/
        options = {
            ('SRM PLUGIN', 'OPERATION_TIMEOUT'): 1800,
        }

        if not hasattr(self, '_gfal'):
            self._gfal = gridlib.gfal2context.Gfal2Context(gfal_options=options,
                                                           retry=5)
        return self._gfal

    ## Set the site on which to operate
    #
    # @param self Object pointer
    # @param site Site on which to operate
    def set_site(self, site):
        log.info('Setting site to {0}'.format(site))
        self.site = site
        self.build_config()


    ## Gather the physical file name (pfn) prefix and CE ID in a dict
    #
    # @param self Object pointer
    def build_config(self):
        # gather file access protocols and CE IDs
        protocols = self.get_site_protocols()
        clusters = self.get_clusters(self.site.split('_')[-1].lower())
        # generate site config dict
        self.site_config = protocols
        self.site_config['site_prefix'] = protocols['dcap']
        self.site_config['clusters'] = clusters
        log.debug('Loaded file access protocols and computing element ids')


    ## Determine the physical file name (pfn) prefix
    #
    # @param self Object pointer
    # @param extension Individual path extension for the specific site
    def get_site_protocols(self):
        # load xml configuration file
        path = '/cvmfs/cms.cern.ch/SITECONF/{0}/PhEDEx/storage.xml'.format(self.site)
        xml_tree = xml.etree.ElementTree.parse(path)
        root = xml_tree.getroot()
        # fill protocol dictionary
        protocols = {}
        for node in root.findall('lfn-to-pfn'):
            # direct nfs mount
            if node.get('protocol') == 'direct' and node.get('path-match').endswith('/+(.*)'):
                protocols['nfs'] = node.get('result').replace('$1', '')
            # srmv2 chained with direct
            if node.get('protocol') == 'srmv2' and node.get('chain') == 'direct':
                protocols['srmv2'] = node.get('result').rstrip('/$1') + protocols['nfs']
            # dcap chained with direct
            if node.get('protocol') == 'dcap' and node.get('chain') == 'direct':
                protocols['dcap'] = node.get('result').replace('/$1', protocols['nfs'])
        return protocols


    ## Determine the IDs of the computing element (CE)
    #
    # Get list of available clusters for cms VO
    #
    # @param self Object pointer
    # @param site_filter Grep expression to filter for sites
    def get_clusters(self, site_filter):
    # TODO # 
######## Needs update for condor! lcg-info does not work like this anymore ##############
        # lcg-infosites requires a VOMS proxy
        gridlib.util.voms_proxy_ensure()
        # build lcg-info command
        command = 'lcg-info --list-ce --bdii lcg-bdii.cern.ch:2170'
        # add query arguments
        queries = ['_clusterck=GlueClusterUniqueID=*{0}*'.format(site_filter),
                   'Cluster=*{0}*'.format(site_filter)]
        command +=' --query \'{0}\''.format(','.join(queries))
        # specify attributes which to request
        attributes = ['MaxWCTime',
                      'WaitingJobs',
                      'FreeJobSlots',
                      'Cluster',
                      'MaxCPUTime',
                      'EstRespTime',
                      'CEVOs']
        command += ' --attrs \'{0}\''.format(','.join(attributes))
        # run command
        exception = EnvironmentError('Could not get Computing Element IDs')
        command_hash = hashlib.sha1(command).hexdigest()
        cache_path = os.path.expanduser("~/.cache/gridlib_get_cluster_{}".format(command_hash))
        if os.path.exists(cache_path) and time.time() - os.path.getmtime(cache_path) < 60:
            with open(cache_path, "r") as cache_file:
                stdout = cache_file.read()
        else:
            # BDII endpoint can be unresponsive with high load, retry once
            try:
                stdout = sh_command(command, exception)
            except:
                time.sleep(1)
                stdout = sh_command(command, exception)
            with open(cache_path, "w") as cache_file:
                cache_file.write(stdout)
        # parse output and filter CMS clusters
        clusters = self.parse_lcg_info(stdout)
        return {key: clusters[key] for key in clusters if 'VO:cms' in clusters[key]['CEVOs']}


    ## Parse output from lcg-info into a dictionary
    #
    # @param self Object pointer
    # @param output Stdout from lcg-info call
    def parse_lcg_info(self, output):
        clusters = {}
        ce_id = None
        key = None
        # read each cluster from stdout blocks
        for i, line in enumerate(output.split('\n')):
            # strip off whitespace and skip empty lines
            line = line.strip()
            if not line:
                continue
            # check for new cluster; pattern example:
            # - CE: grid-ce.physik.rwth-aachen.de:8443/cream-pbs-cms
            if line.startswith('- CE:'):
                ce_id = line.split('- CE: ')[1].strip()
                clusters[ce_id] = {}
            # check for new attribute; patter example:
            # - CEVOs               VO:cms
            elif line.startswith('- '):
                line = line.split()
                key = line[1]
                value = int(line[2]) if line[2].isdigit() else line[2]
                clusters[ce_id][key] = value
            # otherwise add as attribute entry; patter example:
            # VO:cms
            else:
                # check if entry is list, otherwise make it a list
                if not isinstance(clusters[ce_id][key], list):
                    old_value = clusters[ce_id][key]
                    clusters[ce_id][key] = [old_value]
                # append new value
                clusters[ce_id][key].append(line)
        # return clusters dictionary
        return clusters


    ## Returns fastest available computing cluster matching requirements
    #
    # Note that default analysis jobs are supposed to run on the standard queue
    # and the short queue (max_runtime <= 120) is only meant for test jobs.
    #
    # @param self Object pointer
    # @param min_runtime Minimum job runtime for ce selection [default: 2880mins]
    def get_ce_id(self, max_runtime=2880):
        clusters = self.site_config['clusters']
        print self.site_config
        exit()
        matches = {key: clusters[key] for key in clusters
                   if max_runtime <= clusters[key]['MaxCPUTime']}
        sorted_list = sorted(matches, key=lambda k: matches[k]['EstRespTime'])
        site_name = self.site.split('_')[-1].lower()
        if site_name == 'rwth':
            site_name += '-aachen'
        for cluster in sorted_list:
            if site_name in cluster:
                return cluster
        return sorted_list[0]

    ## Build and return a physical path with the specified prefix from a logical path
    #
    # Logical paths are identical on different sites, usually looking like
    # 'store/user/radziej/gridpacks/gridpack.tar.bz2'. To become a physical
    # file name, a protocol prefix needs to be added.
    #
    # @param self Object pointer
    # @param protocol_prefix Prefix specific to the protocol
    # @param lfn_path A logical file name/path starting from the username
    def get_prefixed_path(self, protocol_prefix, lfn_path):
        # prevent common mistake of starting relative paths with /
        if lfn_path.startswith('/'):
            lfn_path = lfn_path[1:]
        return str(os.path.join(protocol_prefix, lfn_path))


    ## Return a physical SRMv2 path from a logical one
    #
    # The SRM protocol is used to communicate with the site from external
    # sources, like personal PCs. For more information on the prefixing, check
    # the documentation on get_prefixed_path().
    #
    # @param self Object pointer
    # @param path A logical file name/path starting from the username
    def get_srm_path(self, lfn_path):
        return self.get_prefixed_path(self.site_config['srmv2'], lfn_path)


    ## Return a physical path (when running on a computing site) from a logical one
    #
    # When computing on a selected site, there is usually a preferred protocol
    # for accessing files on their dcache system. Using this function ensures
    # that this preference is being into account. For more information on
    # prefixing, check the documentation on get_prefixed_path().
    #
    # @param self Object pointer
    # @param path A logical file name/path starting from the username
    def get_site_path(self, lfn_path):
        return self.get_prefixed_path(self.site_config['site_prefix'], lfn_path)


    ## Return a local file path (containing the file:/// prefix)
    #
    # @param self Object pointer
    # @param path A logical path to be interpreted as local path
    def get_local_path(self, path):
        return self.get_prefixed_path('file:///', os.path.abspath(path))

    ## Test whether the storage element can be accessed properly
    #
    # @param self Object pointer
    # @param username CERN user name
    # @return boolean True if test was succesful, False if not
    def test_storage(self, username=gridlib.util.get_username_cern()):
        input_path = self.get_local_path('/bin/bash')
        return_path = self.get_local_path('/tmp/srm_test')
        remote_path = self.get_srm_path(os.path.join('store/user', username, 'site_test'))
        remote_path = os.path.join('store/user', username, 'site_test')
        try:
            # try to copy a file via srm
            self.cp(input_path, self.get_srm_path(remote_path), True)
            # try to copy the file back and delete it
            self.cp(self.get_srm_path(remote_path), return_path, True)
            os.remove('/tmp/srm_test')
            # try to remove test file on grid
            self.rm(remote_path)
        except IOError:
            # test failed
            return False
        # test success
        return True


    ## Copy (recursively) from source location to destination
    #
    # Since it is ambiguous whether the source and destination are local, site
    # or srm paths, one has to specify this manually. The functions
    # get_local_path, get_site_path and get_srm_path might be of use.
    #
    # @param self Object pointer
    # @param source Source location of the file
    # @param destination Destination on the storage element
    # @param force Overwrite, if file exists
    def cp(self, source, destination, force=False):
        # 'raw' copy
        transfer_options = {
            'overwrite': force,
        }
        return self.gfal.filecopy(source, destination, transfer_options=transfer_options)


    ## Copy files within the remote storage
    #
    # Both paths are assumed to be remote paths
    #
    # @param self Object pointer
    # @param source Remote source location of the file
    # @param destination Remote destination location of the file
    # @param force Overwrite, if file exists
    def remote_cp(self, source, destination, force=False):
        return self.cp(self.get_srm_path(source), self.get_srm_path(destination), force=force)


    ## Fetch (copy) file from remote storage to local hard drive
    #
    # @param self Object pointer
    # @param source Remote source location of the file
    # @param destination Local destination location of the file
    # @param force Overwrite, if file exists
    def fetch(self, source, destination, force=False):
        return self.cp(self.get_srm_path(source), self.get_local_path(destination), force=force)


    ## Put (copy) file from local hard drive to remote storage
    #
    # @param self Object pointer
    # @param source Local source location of the file
    # @param destination Remote destination location of the file
    # @param force Overwrite, if file exists
    def put(self, source, destination, force=False):
        return self.cp(self.get_local_path(source), self.get_srm_path(destination), force=force)


    ## Get stat object of remote file
    #
    # Documentation on the stat object:
    # https://docs.python.org/2/library/os.html#os.stat
    # Also note the Python module for interpreting the st_mode part:
    # https://docs.python.org/2/library/stat.html
    #
    # @param self Object pointer
    # @param path Remote path to file/folder
    def stat(self, path):
        return self.gfal.stat(self.get_srm_path(path))


    ## Check whether a remote path points to a directory
    #
    # @param self Object pointer
    # @param path Remote path to check
    def isdir(self, path):
        return self.gfal.isdir(self.get_srm_path(path))


    ## Check whether a remote path points to anything
    #
    # @param self Object pointer
    # @param path Remote path to check
    def exists(self, path):
        return self.gfal.exists(self.get_srm_path(path))


    ## Remove target (recursively) at given path
    #
    # @param self Object pointer
    # @param path Path to target which to remove
    # @param recursive Recurse into directory and subdirectory when deleting
    def rm(self, path, recursive=False):
        path = self.get_srm_path(path)
        if recursive and self.isdir(path):
            for entry in self.ls(path):
                entry_path = os.path.join(path, entry)
                self.rm(entry_path, recursive=True)

        return self.gfal.unlink(path)


    ## Move target on remote target
    #
    # @param source Source path, interpreted as remote (SRM) path
    # @param destination Destination path, also interpreted as remote path
    def mv(self, source, destination):
        return self.gfal.rename(self.get_srm_path(source), self.get_srm_path(destination))


    ## Create directory at given path
    #
    # @param self Object pointer
    # @param path Remote path to new directory
    def mkdir(self, dir_path):
        return self.gfal.mkdir(self.get_srm_path(dir_path))


    ## List contents of path
    #
    # @param self Object pointer
    # @param dir_path Remote path of which to list the contents
    # @param full_path Return list of full paths instead of basenames
    # @param stat Batch-execute stat on all entries, yield objects containing .name, and .stat
    # @return list List of strings if stat not given, dirents otherwise
    def ls(self, dir_path, full_path=False, stat=False):
        if full_path and stat:
            raise NotImplementedError('Cannot combine full_path and stat (yet)')

        dir_path = self.get_srm_path(dir_path)
        if not stat:
            names = self.gfal.listdir(dir_path)

            if full_path:
                return [os.path.join(dir_path, name) for name in names]
            else:
                return names
        else:
            return self.gfal.readpp(dir_path)


    ## Compute adler32 checksum of remote file
    #
    # @param self Object pointer
    # @param file_path Remote path to compute the checksum on
    # @return str 8-character lowercase hex string (2 chars per byte, 4 bytes)
    def adler32(self, file_path):
        try:
            return self.gfal.checksum(self.get_srm_path(file_path), algorithm='ADLER32')
        except Exception as e:
            if 'No such file or directory' in str(e):
                raise IOError('Could not get adler32 checksum for {0}'.format(file_path))
            else:
                raise e


    ## Open remote file for reading/writing [EXPERIMENTAL]
    #
    # In direct mode (default), this function returns a FileType object on which
    # you can call .read() and .write() to interact with the remote file directly.
    # To close the file (as far as I know), one has to call the destructor by
    # calling 'del' on the FileType object or running out of scope...
    #
    # In indirect mode, this function uses the RemoteFileProxy class. It copies
    # the entire file to a temporary file and uploads it, once
    # RemoteFileProxy.close() is called.
    #
    # @param self Object pointer
    # @param mode Opening mode: 'r' or 'w'
    # @param direct Use direct mode (default: True)
    # @return object FileType or RemoteFileProxy
    def open(self, file_path, mode='r', direct=True):
        file_path = self.get_srm_path(file_path)
        if direct:
            # https://pythonhosted.org/gfal2-python/gfal2.Gfal2Context.FileType-class.html
            return self.gfal.open(file_path, mode)
        else:
            return RemoteFileProxy(self, file_path, mode=mode)


    ## Read the attributes (xattrs) of a remote file/directory
    #
    # @param self Object pointer
    # @param path Remote file/directory to analyze
    # @return dict Dictionary of attributes
    def xattrs(self, path):
        rpath = self.get_srm_path(path)
        keys = self.gfal.listxattr(rpath)
        attrs = {}
        for key in keys:
            try:
                value = self.gfal.getxattr(rpath, key)
            except gridlib.gfal2context.gfal2.GError as e:
                if 'Path is a directory' in e.message:
                    continue
                else:
                    raise IOError(e.message)
            attrs[key] = value
        return attrs


class RemoteFileProxy(object):
    def __init__(self, session, rpath, mode='r'):
        super(RemoteFileProxy, self).__init__()

        self.session = session
        self.rpath = rpath
        self.mode = mode

        self._tmp_filename = tempfile.mkstemp()[1]
        if 'r' in self.mode:
            self.session.fetch(rpath, self._tmp_filename, force=True)
        self._file = open(self._tmp_filename, mode)

    def __del__(self):
        self.close()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    @property
    def closed(self):
        return self._file.closed

    def close(self):
        if not self.closed:
            self._file.close()
            if 'w' in self.mode:
                self.session.put(self._tmp_filename, self.rpath, force=True)
            os.remove(self._tmp_filename)

    def seek(self, *args, **kwargs):
        return self._file.seek(*args, **kwargs)

    def write(self, *args, **kwargs):
        return self._file.write(*args, **kwargs)

    def read(self, *args, **kwargs):
        return self._file.read(*args, **kwargs)


# Use this instead of @memoize because otherwise pickle will complain
_se_instances = {}
def get_se_instance(*args, **kwargs):
    key = gridlib.util.memoization_id(StorageElement, args, kwargs)
    if key not in _se_instances:
        _se_instances[key] = StorageElement(*args, **kwargs)
    return _se_instances[key]


## Abstract class to manage gridpacks
#
# This is meant to be inherited by Submitter classes to manage their gridpacks.
# By calling gridpack_prepare(...), one creates a gridpack and ensures that it
# matches the one on the storage element.
class GridpackManager(object):

    ## Default constructor
    #
    # @param self Object pointer
    def __init__(self, storage_element):
        self.storage_element = storage_element
        self.username_cern = gridlib.util.get_username_cern()

    ## Compare checksums of local and remote gridpack
    #
    # @param self Object pointer
    # @param local_path Path to local gridpack
    # @param remote_path Logical path to gridpack on site
    def gridpack_outdated(self, local_path, remote_path):
        if not self.storage_element.exists(remote_path):
            print 'Remote gridpack not found'
            return True

        # determine remote checksum
        remote = self.storage_element.adler32(remote_path)
        # determine local checksum
        local = self.local_adler32(local_path)
        # return True if gridpack is outdated
        return remote != local


    ## Upload the local gridpack to the site
    #
    # @param self Object pointer
    # @param local_path Path to local gridpack
    # @param remote_path Logical path to gridpack on site
    def gridpack_upload(self, local_path, remote_path):
        # upload new gridpack (and potentially overwrite old one)
        log.debug('Uploading Gridpack: %s => %s', local_path, remote_path)
        self.storage_element.put(local_path, remote_path,
                             force=True)

    ## Create a local version of the gridpack
    #
    # @param self Object pointer
    # @param base_directory
    # @param list of files to include in gridpack
    # @param gridpack_local Name of the local .tar.bz2 gridpack file
    def gridpack_create(self, base_directory, files,
                        gridpack_local='gridpack.tar.bz2'):

        # create gridpack
        with tarfile.open(gridpack_local, 'w:bz2') as gridpack:
            cwd = os.getcwd()
            os.chdir(os.path.expandvars(base_directory))
            for item in files:
                item = os.path.expandvars(item)
                if os.path.isabs(item):
                    # add absolute paths to root
                    gridpack.add(item, arcname=os.path.basename(item))
                else:
                    # keep directory tree if path is relative relative paths
                    gridpack.add(item)
            os.chdir(cwd)

    ## Return ADLER32 checksum for local path
    #
    # @param self Object pointer
    # @param local_path Path to file for which to determine the checksum
    def local_adler32(self, local_path):
        return subprocess.Popen(['adler32', local_path],
                                stdout=subprocess.PIPE).communicate()[0].strip()

    ## Prepare the gridpack
    #
    # @param self Object pointer
    # @param base_directory
    # @param list of files to include in gridpack
    # @param gridpack_local Name of the local .tar.bz2 gridpack file
    # @param gridpack_remote Logical path to gridpack on site
    def gridpack_prepare(self, base_directory, files,
                         gridpack_local='gridpack.tar.bz2',
                         gridpack_remote=None,
                         ask_for_upload=True):
        # build gridpack_remote
        if not gridpack_remote:
            gridpack_remote = os.path.join('store/user', self.username_cern,
                                           'gridpacks', gridpack_local)

        self.gridpack_create(base_directory, files, gridpack_local)

        # determine action for gridpack
        if self.gridpack_outdated(gridpack_local, gridpack_remote):
            while ask_for_upload:
                print 'Remote gridpack does not match local one. Upload local gridpack?'
                answer = raw_input('[y/n] ')
                # replace gridpack
                if answer in 'yY':
                    self.gridpack_upload(gridpack_local, gridpack_remote)
                    break
                # keep gridpack for now
                elif answer in 'nN':
                    print 'Exiting'
                    sys.exit(0)
            if not ask_for_upload:
                self.gridpack_upload(gridpack_local, gridpack_remote)
        return gridpack_remote


if __name__=='__main__':
    print 'This is a library'

    ## BEGIN Create logger ##
    # logging.basicConfig(level=logging.WARNING)
    ## END ##

    ## BEGIN Storage test ##
    # se = StorageElementNewApi()
    # se.set_site('T2_DE_DESY')
    # se.set_site('T2_DE_RWTH')
    # print 'Full config:', se.site_config
    # print 'Clusters:', se.get_clusters('rwth')
    # print 'Preferred CE ID:', se.get_ce_id(120)
    # print 'Storage test:', se.test_storage()

    # print 'Simple ls:', se.ls('store/user/radziej/PxlSkim/CMSSW_7_6_v1.0/ZToMuMu_M_120_200_13TeV_PH/ZToMuMu_NNPDF30_13TeV-powheg_M_120_200/76X_mcRun2_asymptotic_v12/160120_111540/0000/')
    # print 'Ls with more information:', se.ls_l('store/user/radziej/PxlSkim/CMSSW_7_6_v1.0/ZToMuMu_M_120_200_13TeV_PH/ZToMuMu_NNPDF30_13TeV-powheg_M_120_200/76X_mcRun2_asymptotic_v12/160120_111540/')
    # print 'An adler32 checksum:', se.adler32('store/user/radziej/gridpacks/gridpack.tar.bz2')

    # print 'Finding files works', se.find('store/user/radziej/PxlSkim/CMSSW_7_6_v1.0/ZToMuMu_M_120_200_13TeV_PH/ZToMuMu_NNPDF30_13TeV-powheg_M_120_200/76X_mcRun2_asymptotic_v12/160120_111540/', '*.pxlio')
    # print 'Slow retrieval of a large number of files ...'
    # print 'Matching files found:', len(se.find('store/user/joroemer/PxlSkim/CMSSW_7_6_v1.0/QCD_Pt-50to80_EMEnriched_13TeV_P8/QCD_Pt-50to80_EMEnriched_TuneCUETP8M1_13TeV_pythia8/76X_mcRun2_asymptotic_v12/160224_145839/', '*.pxlio'))
    ## END ##

    ## BEGIN File operation tests ##
    # se.cp('/user/radziej/testdir/testfile', se.get_srm_path('radziej/testfile'))
    # se.rm('radziej/testfile')
    # se.rm('radziej/testfile', True)
    # print se.ls('radziej/')

    ## BEGIN Create and remove directory ##
    # se.mkdir('radziej/testdir')
    # print 'New dir testdir:', se.ls('radziej/')
    # se.rm('radziej/testdir')
    # print 'Removed testdir:', se.ls('radziej/')
    ## END ##

    ## BEGIN Jonas' tests ##
    # import time
    #se = StorageElementCli('T2_DE_RWTH')
    # se = StorageElement('T2_DE_RWTH')
    # path = 'store/user/jlieb/test.txt'
    #se.rm(path)
    #fd = se.open(path, 'w', direct=True)
    #try:
    #    fd.write('Hello World!')
    #finally:
        #fd.close()
    #    print('DONE')
    #time.sleep(10)
    # se.fetch('/store/user/tpook/chimera_check.py', 'test.txt', force=True)
    ## END ##

    sys.exit(0)
