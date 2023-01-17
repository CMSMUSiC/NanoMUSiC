import os
import stat
import glob
import logging

import luigi

from luigi_music.util import job_dir, exit_code_from_info

import gridlib.se
import gridlib.util

logger = logging.getLogger( 'luigi-interface' )

__all__ = [ "GridFileSystem", "GridTarget", "GridTaskTarget" ]

# /store/user/jlieb
# /pnfs/physik.rwth-aachen.de/cms/store/user/jlieb
# srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/jlieb

@gridlib.util.memoize
class GridFileSystem( luigi.target.FileSystem ):
    #def __init__( self, site, delegation_id=None ): #Yannik commented it out (debug)
    def __init__( self, site ):
        self.storage_element = gridlib.se.get_se_instance( site )

    def exists( self, path ):
        return self.storage_element.exists( path )

    def remove( self, path, recursive=True, skip_trash=True ):
        if not skip_trash:
            raise ValueError( "Trash not supported" )
        return self.storage_element.rm( path, recursive=recursive )

    def mkdir( self, path, parents=True, raise_if_exists=False ):
        if raise_if_exists and self.exists( path ):
            raise ValueError( "Directory already exists" )
        return self.storage_element.mkdir( path )

    def isdir( self, path ):
        return self.storage_element.isdir( path )

    def listdir( self, path ):
        return self.storage_element.ls( path )

    def move( self, path, dest ):
        return self.storage_element.mv( path, dest )

    def copy( self, path, dest ):
        return self.storage_element.remote_copy( path, dest )

    def fetch( self, path, dest ):
        return self.storage_element.fetch( path, dest )

    def put( self, path, dest ):
        return self.storage_element.put( path, dest )

    def open( self, path, mode="r" ):
        return self.storage_element.open( path, mode )


class GridTarget( luigi.target.FileSystemTarget ):
    #def __init__( self, path, site, delegation_id=None ): Yannik commented it out (debug)
    def __init__( self, path, site ):
        super( GridTarget, self ).__init__( path )
        self.file_system = GridFileSystem( site )

    @property
    def fs( self ):
        return self.file_system

    def open( self, mode ):
        return self.fs.open( self.path, mode=mode )

    def fetch( self, target ):
        self.fs.fetch( self.path, target )
        return luigi.LocalTarget( target )

    def put( self, source ):
        return self.fs.put( source, self.path )

    @property
    def site( self ):
        return self.fs.storage_element.site

    @property
    def srm_path( self ):
        # e.g. src://host/?SFN=/...
        return self.fs.storage_element.get_srm_path( self.path )

    @property
    def site_path( self ):
        # e.g. dcap://host/...
        return self.fs.storage_element.get_site_path( self.path )

    @property
    def external_path( self ):
        site_path = self.site_path
        needle = "grid-dcap.physik.rwth-aachen.de"
        replacement = "grid-dcap-extern.physik.rwth-aachen.de"
        if not needle in site_path:
            raise ValueError( "Cannot get *-extern path for path '%s'" % site_path )
        return site_path.replace( needle, replacement )

class GridTaskTarget( luigi.target.Target ):
    """ This target can be used to determine if a ce task is finsihed"""
    def __init__(self, taskfolder, max_failed=0.0, target_files=[]):
        """ Object Constructor

        @param taskfolder: path to a gridlib ce task folder
        @param max_failed: maximum allowed fraction of failed jobs to accept for the task to be complete
        """
        self.taskfolder = taskfolder
        self.max_failed = max_failed
        self.target_files = target_files
        self._ce_task = None

    @property
    def ce_task(self):
        if not self._ce_task:
            if os.path.exists(os.path.join(self.taskfolder, "task.pkl")):
                self._ce_task = gridlib.ce.Task.load( self.taskfolder )
        return self._ce_task

    @property
    def files(self):
        out_files = []
        for job in self.ce_task.jobs:
            for target_file in self.target_files:
                out_files += glob.glob(os.path.join(self.taskfolder,
                                                    job_dir(job),
                                                    target_file))
        return out_files

    def exists(self):
        if not self.ce_task:
            return False

        njobs = len(self.ce_task.jobs)
        success = 0
        for job in self.ce_task.jobs:
            exit_code = exit_code_from_info( job.infos )
            out_files = []
            for target_file in self.target_files:
                out_files += glob.glob(os.path.join(self.taskfolder,
                                      job_dir(job),
                                      target_file))
            logger.debug("%s %d" % (job.status, exit_code) )
            if job.status == "RETRIEVED" and exit_code == 0 and not out_files:
                logger.info("exit_code")
                logger.info(exit_code)
                logger.info("%d %d " % (len(out_files), len(self.target_files)))
            if len(out_files) == len(self.target_files):
                success += 1

        failed_ratio = 1. - (1 * success / njobs)
        if failed_ratio >  self.max_failed:
            #logger.info("%d of %d jobs failed: %.1f allowed %.1f" % (success, njobs, failed_ratio, self.max_failed) )
            #logger.info(self.files)
            return False
        return True
