#!/usr/bin/env python

import logging
import optparse
import os
import sys

log = logging.getLogger( 'processReader' )


class processInfo:
    def __init__( self, databaseID=None, origName=None, newName=None ):
        self.databaseID = databaseID
        self.origName = origName
        self.newName = newName

    def __repr__( self ):
        return ( 'process: '
                 + 'databaseID = %r - ' % self.databaseID
                 + 'origName = %r - '   % self.origName
                 + 'newName = %r'       % self.newName
                 )


class processReader:
    # Processes config fragment must consist of a list of process names
    # separated by linebreaks and/or commas. May contain empty lines and
    # comments.
    # Processes must accord to following syntax rules:
    # * If only the process name is given:
    #   processName
    # * If the process name and corresponding database ID are given:
    #   processName:databaseID
    # * If processes have been renamed (databaseID can also be omitted):
    #   newProcessName::originalProcessName:databaseID;originalProcessName:databaseID
    #
    def parseProcessFile( self, processFilePath ):
        COMMENT_CHAR = '#'
        allProcesses = []
        with open( processFilePath, 'r' ) as processFile:
            # Processes can be separated by line breaks.
            for line in processFile:
                # Remove empty lines.
                line = line.strip()
                if line:
                    allProcesses += self.parseProcessList( line )

        return allProcesses


    def parseProcessList( self, procs ):
        # Processes can be separated by commas or by line breaks.
        procs = [ x.split( ',' ) for x in procs.splitlines() ]
        procs = [ x.strip() for y in procs for x in y if x ]

        COMMENT_CHAR = '#'
        allProcesses = []
        for proc in procs:
            proc = self.removeCommentsFromLine( COMMENT_CHAR, proc )
            if proc:
                # A statement for renaming processes is given, mapping the
                # original names to a new name.
                if len( proc.split( '::' ) ) == 2:
                    newName         = proc.split( '::' )[0]
                    origNamesWithID = proc.split( '::' )[-1]
                    # The original names must be separated by a semicolon.
                    for origNameWithID in origNamesWithID.split( ';' ):
                        origName, databaseID = self.splitProcessAndID( origNameWithID )
                        processObj = processInfo( databaseID, origName, newName )
                        log.debug( 'Found process: %s' % processObj )
                        allProcesses.append( processObj )

                # Only original process names are given.
                elif len( proc.split( '::' ) ) == 1:
                    origName, databaseID = self.splitProcessAndID( proc )
                    processObj = processInfo( databaseID, origName )
                    log.debug( 'Found process: %s' % processObj )
                    allProcesses.append( processObj )

                else:
                    log.error( "Incorrect sytax in process '%s'." % proc )
                    log.error( "Check use of '::'. Only one occurance is allowed, separating the process new name from the old ones." )
                    sys.exit(1)

        return allProcesses


    def removeCommentsFromLine( self, COMMENT_CHAR, line ):
        # Remove comments from lines and ignore commented out lines.
        if line.startswith( COMMENT_CHAR ):
            line = None
        else:
            if COMMENT_CHAR in line:
                line, comment = line.split( COMMENT_CHAR, 1 )
                line = line.strip()

        return line


    # Separates a given process name and ID statement (syntax: 'processName:ID')
    # and returns both of them.
    # If no ID is given the name along with 'None' for the ID is returned. This
    # syntax is kept since e.g. MISMaster does not need IDs, but a warning will
    # be issued since they are necessary for several other scripts.
    def splitProcessAndID( self, processWithID ):
        if len( processWithID.split( ':' ) ) == 2:
            if processWithID.split( ':' )[1].isdigit():
                return processWithID.split( ':' )[0], processWithID.split( ':' )[1]
            else:
                log.error( "Database ID not formatted correctly in process '%s'." % processWithID )
                sys.exit(1)
        elif len( processWithID.split( ':' ) ) == 1:
            log.warning( "No database ID given for process '%s'." % processWithID )
            return processWithID, None
        else:
            log.error( "Incorrect sytax in process '%s'." % processWithID )
            log.error( "Check use of ':'. This is used to separate the process name from the database ID." )
            sys.exit(1)


    # Returns unique list of used process names from a given list of processInfo
    # objects. If the object contains a new name this is what will be added to
    # the set. Otherwise the original name will be used.
    def getUniqueProcessList( self, ProcessObjList ):
        processSet = set()
        for process in ProcessObjList:
            if process.newName:
                processSet.add( process.newName )
            else:
                processSet.add( process.origName )

        return list( processSet )
