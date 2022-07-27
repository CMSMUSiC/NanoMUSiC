#!/usr/bin/env python

import logging
import os
import subprocess

log = logging.getLogger( __name__ )

# In general, deactivated the internal retry function ('-retry_num=0') in all
# commands! Sometimes the command fails (e.g. with
# 'java.lang.NullPointerException') and the retrying takes forever and is
# always unsuccessful. You should take care of retrying by your own.

class SRMError( Exception ):
    def __init__( self, error, output='' ):
        self.error = error
        self.output = output
    def __str__( self ):
        return 'Error: ' + repr( self.error ) + '\n' + \
               'Full output: ' + repr( self.output )


# dest must be one srmURL.
def srmmkdir( dest ):
    srmmkdir = [ 'srmmkdir', '-retry_num=0', dest ]

    log.debug( 'Calling srm: ' + ' '.join( srmmkdir ) )
    proc = subprocess.Popen( srmmkdir,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT,
                             )
    out = proc.communicate()[0]
    retcode = proc.returncode

    if retcode == 0:
        log.debug( "Successfully created directory '%s'." % dest )
        return
    else:
        if 'SRM_DUPLICATION_ERROR' in out:
            # Apparently the directory already exists, so stop trying to
            # create it!
            log.debug( "Directory '%s' already exists. Ignoring..." % dest )
            return
        else:
            raise SRMError( "Unable to create directory '%s'!" % dest, out )


# The last srmURL must be the target directory, the other ones the source files.
def srmcp( *srmURLs ):
    if len( srmURLs ) < 2:
        raise SRMError( "At least two srmURLs needed!" )

    dest = srmURLs[-1]
    source = list( srmURLs[:1] )

    srmcp = [ 'srmcp', '-retry_num=0' ] + source + [ dest ]

    log.debug( 'Calling srm: ' + ' '.join( srmcp ) )
    proc = subprocess.Popen( srmcp,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT,
                             )
    out = proc.communicate()[0]
    retcode = proc.returncode

    if retcode == 0:
        log.debug( "Copy successful." )
        return
    else:
        raise SRMError( "Failed to copy '" + ' '.join( source ) + \
                        "' to '" + dest + "'!",
                        out
                        )


# The srmURL must be a file on the dCache.
def srmrm( *srmURLs ):
    files = list( srmURLs )
    srmrm = [ 'srmrm', '-retry_num=0' ] + files

    log.debug( 'Calling srm: ' + ' '.join( srmrm ) )
    proc = subprocess.Popen( srmrm,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT,
                             )
    out = proc.communicate()[0]
    retcode = proc.returncode

    if retcode == 0:
        return
    else:
        # File not found, nevermind!
        if 'No such file' in out:
            log.debug( "Could not delete file(s) '" + \
                       ",'".join( files ) + "'" + \
                       ", file not found. Ignoring..."
                       )
            return
        else:
            raise SRMError( "Could not delete file(s) '" + \
                            ",'".join( files ) + "'",
                            out
                            )


# Retry to call any given function func with arguments args maxTry times.
def retry( func, args, maxTry=10 ):
    for numTry in range( 1, maxTry + 1 ):
        try:
            func( *args )
            break
        except:
            if numTry < maxTry:
                pass
            else:
                raise
