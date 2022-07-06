##@package terminalFunctions.py
# Functions to be used for terminal output
#
# Functions and classes for terminal output.
import sys, os

##@class bcolors
#
# Color definition for terminal output
class bcolors:
    HEADER = '\033[35m'
    OKBLUE = '\033[34m'
    OKGREEN = '\033[32m'
    WARNING = '\033[33m'
    FAIL = '\033[31m'
    CYAN = '\033[36m'
    WHITE = '\033[37m'
    ENDC = '\033[0m'

    ## Function to unset all colors
    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.CYAN = ''
        self.WHITE = ''
        self.ENDC = ''

## Function to get the size of the terminal
#
# Reads the width and height of the current terminal and returns
# this two values.
# @param[out] int(cr[1]) Width of the terminal
# @param[out] int(cr[0]) Height of the terminal
def getTerminalSize():
    env = os.environ
    def ioctl_GWINSZ(fd):
        try:
            import fcntl, termios, struct, os
            cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ,
        '1234'))
        except:
            return
        return cr
    cr = ioctl_GWINSZ(0) or ioctl_GWINSZ(1) or ioctl_GWINSZ(2)
    if not cr:
        try:
            fd = os.open(os.ctermid(), os.O_RDONLY)
            cr = ioctl_GWINSZ(fd)
            os.close(fd)
        except:
            pass
    if not cr:
        cr = (env.get('LINES', 25), env.get('COLUMNS', 80))
    return int(cr[1]), int(cr[0])

## Function to create and update a progress bar
#
# This function displays or updates a console progress bar
# It accepts a float between 0 and 1. Any int will be converted to a float.
# A value under 0 represents a 'halt'.
# A value at 1 or bigger represents 100%
# @param[in] progress Relative progress that should be displayed
def update_progress(progress):
    (width, height) = getTerminalSize()
    barLength = width-30 # Modify this to change the length of the progress bar
    status = ""
    if isinstance(progress, int):
        progress = float(progress)
    if not isinstance(progress, float):
        progress = 0
        status = bcolors.FAIL+"error: progress var must be float\r\n"+bcolors.ENDC
    if progress < 0:
        progress = 0
        status = bcolors.WARNING+"Halt...\r\n"+bcolors.ENDC
    if progress >= 1:
        progress = 1
        status = bcolors.OKGREEN+"Done...\r\n"+bcolors.ENDC
    block = int(round(barLength*progress))
    text = "\rPercent: [{0}] {1}% {2}".format( bcolors.HEADER+"#"*block+bcolors.ENDC + "-"*(barLength-block), int(progress*100), status)
    sys.stdout.write(text)
    sys.stdout.flush()
