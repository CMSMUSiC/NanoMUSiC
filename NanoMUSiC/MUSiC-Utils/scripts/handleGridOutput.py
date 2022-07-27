#!/usr/bin/env python

from collections import defaultdict
import logging
import multiprocessing
import argparse
import os
import shutil
import time
import tarfile
import sys

log = logging.getLogger( 'handleGridOutput' )


def main():
    options = parseCommandline()
    if not allSubfoldersContainArchives( options ):
        log.warning( "Not all subfolders contain '%s' files. Did you use 'getTarFromDCache'?" % options.extension )

    tgzfiles = []
    for root in options.paths:
        for directory, _, filenames in os.walk( root ):
            for filename in filenames:
                if filename.endswith( options.extension ):
                    fullpath = os.path.join( root, directory, filename )

                    # This is dirty. Any better solution available?
                    if '/bak/' in fullpath:
                        log.debug( "Skipping backup '%s'." % fullpath )
                    else:
                        log.debug( "Found archive '%s'." % fullpath )
                        tgzfiles.append( fullpath )

    if not tgzfiles:
        log.error( "No files with extension '%s' found. Did you remember to use 'getTarFromDCache'?" % options.extension )
        log.error( "Exiting." )
        return

    if options.dryrun:
        log.info( "The following files would be extracted:\n" + "\n".join( tgzfiles ) )
    else:
        process_count = min( options.jobs, len( tgzfiles ) )
        log.info( "Extracting %d archives in %d worker processes..." % ( len( tgzfiles ), process_count ) )
        jobPool = multiprocessing.Pool( processes=process_count )
        arguments = ( ( tgzfile, options ) for tgzfile in tgzfiles )
        try:
            result = jobPool.map_async( extract, arguments )
            jobPool.close()

            last_number_left = 0
            chunksize, extra = divmod( len( tgzfiles ), process_count * 4 )
            if extra:
               chunksize += 1
            while not result.ready():
                if result._number_left != last_number_left:
                    log.info( "Finished: %d of %d jobs." % ( len( tgzfiles ) - result._number_left * chunksize + len( tgzfiles ) % chunksize, len( tgzfiles ) ) )
                    last_number_left = result._number_left
                time.sleep( 0.5 )
        except KeyboardInterrupt:
            jobPool.terminate()
            jobPool.join()
            log.warning( 'Terminated!' )

    log.info( 'Done.' )


def extract( args ):
    path, options = args

    root = os.path.normpath( os.path.join( os.path.dirname( path ) , "../../.." ) )

    relative = os.path.relpath( path, root )
    destination = os.path.dirname( os.path.join( options.output, relative ) )

    log.debug( "Extracting file: '%s' to '%s'." % ( path, destination ) )
    tar = tarfile.open( name=path )
    tar.extractall( path=destination )
    tar.close()

    if options.delete:
        log.debug( "Deleting archive: '%s'." % ( path ) )
        os.remove( path )

def parseCommandline():
    descr = 'Scan PATH recursively for zipped output of grid_radio and extract it to given (current) OUTDIR. ' \
            'PATH is the parent directory the process-named folders created by jukebox. ' \
            'Each of its subfolders (or respective subsequent subfolders) should contain a zipped file.'

    parser = argparse.ArgumentParser( description=descr )
    parser.add_argument(       '--debug', default='INFO', choices=('ERROR', 'WARNING', 'INFO', 'DEBUG'),
                       help='Set the debug level. [default = %(default)s]' )
    parser.add_argument( '-o', '--output', type=str, default='./',
                       help='Set the output directory for the zipped files. [default = %(default)s]' )
    parser.add_argument(       '--dry-run', dest='dryrun', action='store_true', default=False,
                       help="Don't do anything, just list the files. [default = %(default)s]" )
    parser.add_argument(       '--extension', type=str, default='.tar.gz',
                       help='Set the file extension of the files of interest. [default = %(default)s]' )
    parser.add_argument(       '--delete', action='store_true', default=False,
                       help="Delete archive after successful extraction. [default = %(default)s]" )
    parser.add_argument( '-j', '--jobs', type=int, default=4,
                       help='Set the maximum number of parallel jobs to be started [default = %(default)s].' )
    parser.add_argument( 'paths', type=str, nargs='+',
                       help='Parent directory of the process-named folders created by jukebox.' )

    options = parser.parse_args()

    format = '%(levelname)s (%(name)s) [%(asctime)s]: %(message)s'
    date = '%F %H:%M:%S'
    logging.basicConfig( level=logging._levelNames[ options.debug ], format=format, datefmt=date )

    options.paths = map( makeAbsPath, options.paths )

    for path in options.paths:
        if not os.path.isdir( path ):
            parser.error( "Could not find directory '%s'. Please specify an existing directory!" % path )

    options.output = makeAbsPath( options.output )

    return options


# Make path abolute.
# This means expand all $-variables and ~ and prepend the
# current working directory if path is relative.
def makeAbsPath( path ):
    return os.path.abspath( os.path.realpath( os.path.expandvars( os.path.expanduser( path ) ) ) )

def allSubfoldersContainArchives( options ):
    subfolders = []
    for path in options.paths:
        for element in os.listdir( path ):
            if os.path.isdir( element ):
                subfolders.append( os.path.join( path, element ) )

    tarFound = False
    for subfolder in subfolders:
        for path, dirs, files in os.walk( subfolder ):
            for thisfile in files:
                if thisfile.endswith( options.extension ):
                    tarFound = True
                    break

        if not tarFound:
            return False
    return True


if __name__ == '__main__':
    main()
