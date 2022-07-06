import sys
import os


class progressBar:
    def __init__( self, minValue = 0, maxValue = 100, totalWidth = None, progBar = '[]', char = '=' ):
        rows, columns = os.popen( 'stty size', 'r' ).read().split()
        self.min            = minValue
        self.max            = maxValue
        self.progBar        = progBar               # This holds the progress bar delimiters.
        self.char           = char
        self.span           = maxValue - minValue
        self.amount         = minValue              # When amount == max, we are 100% done.
        self.update         = True
        self.oldPercentDone = None
        self.done           = False

        if not totalWidth:
            self.dynamicWidth = True
            self.width = int( columns )
        else:
            self.dynamicWidth = False
            self.width = int( totalWidth )

        if len( self.progBar ) != 2: raise Exception( 'progBar must be exactly 2 characters long!' )
        if len( self.char )    != 1: raise Exception( 'char must be exactly 1 character long!' )
        # Build progress bar string.
        #
        self.updateProgress( minValue )


    def updateProgress( self, newAmount = 0 ):
        if self.dynamicWidth:
            rows, columns = os.popen( 'stty size', 'r' ).read().split()
            self.width = int( columns )

        if newAmount < self.min: newAmount = self.min
        if newAmount > self.max: newAmount = self.max

        self.amount = newAmount

        # Figure out the new percent done, round to an integer.
        #
        diffFromMin = float( self.amount - self.min )
        percentDone = ( diffFromMin / float( self.span ) ) * 100.0
        percentDone = int( round( percentDone ) )

        if percentDone == self.oldPercentDone: self.update = False
        else:                                  self.update = True
        self.oldPercentDone = percentDone

        # Figure out how many progress characters the percentage bar should have.
        #
        allFull = self.width - 2
        numChars = ( float( percentDone ) / 100.0 ) * allFull
        numChars = int( round( numChars ) )

        # Build a progress bar with progress characters and spaces.
        #
        progressBar = self.progBar[0] + self.char * numChars + '>' + ' ' * ( allFull - numChars - 1 ) + self.progBar[1]
        if percentDone == 100:
            progressBar = self.progBar[0] + self.char * numChars + ' ' * ( allFull - numChars ) + self.progBar[1]

        # Figure out where to put the percentage (roughly centered).
        #
        percentString = ' ' + str( percentDone ) + ' % '
        percentPlace  = ( len( progressBar ) / 2 ) - len( str( percentString ) )

        # Slice the percentage into the bar.
        #
        progressBar = progressBar[0:percentPlace] + percentString + progressBar[percentPlace + len( percentString ):]

        # Print the complete status bar.
        # For the last printing, do a normal line break.
        #
        if self.update and percentDone != 100 and not self.done:
            sys.stdout.write( '\r' + progressBar  )
            sys.stdout.flush()
        if not self.done and percentDone == 100:
            print str( '\r' + progressBar )
            #sys.stdout.write( '\r' + progressBar  )
            self.done = True
