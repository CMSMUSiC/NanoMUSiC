#!/usr/bin/env python
import os
import re
import datetime
import subprocess
import urllib2
from optparse import OptionParser



def main():

    usage = 'usage: %prog -c CMSSW -a ARCH\n -performs steps 0-1 shown in http://cms-sw.github.io/cmsdist/\n -should be run on a build machine\n -see http://cms-sw.github.io/latestIBs.html for option values'
    parser = OptionParser(usage=usage)
    cmssw_default = 'CMSSW_7_6_X'
    parser.add_option('-c', '--cmssw', dest='cmssw', default=cmssw_default, help='CMSSW release, default: '+cmssw_default)
    arch_default = 'slc6_amd64_gcc493'
    parser.add_option('-a', '--arch', dest='arch', default=arch_default , help='architecture for CMSSW release, default: '+arch_default)
    parser.add_option('-q', '--quiet', action="store_false", dest="quiet", default=False, help='less verbose & dont ask before executing commands, default: False')
    (options, args) = parser.parse_args()

    arch = options.arch
    cmssw = options.cmssw

    # Set the partition used for building.
    # Partition depends on the machine (/build1 and /build might be available on Linux,
    # and /build1 (/Volumes/build1) on Mac OS X)
    if 'slc' in arch:
        builddir = '/build'
    elif 'osx' in arch:
        builddir = '/build1'
    else:
        raise Exception( 'Could not choose builddir from given architecture \'%s\'. Architecture should contain either \'slc\' or \'osx\'.'%arch )



    # Create build area and prepare CMSDIST and PKGTOOLS:
    user = os.getenv('USER')
    here = builddir + '/' + user
    now = datetime.datetime.now()
    dt = now.strftime("%Y%m%d_%H%M")
    topdir = here + '/ext/' + cmssw + '/' + dt
    if not options.quiet:
        print "about to create dir:"
        print topdir
        raw_input("continue?")
    os.makedirs(topdir)
    os.chdir(topdir)

    url = 'https://raw.githubusercontent.com/cms-sw/cms-bot/master/config.map'
    req = urllib2.Request(url)
    res = urllib2.urlopen(req)
    pkgtools_tag, cmsdist_tag = '', ''
    for line in res.read().split('\n'):
        if 'SCRAM_ARCH=%s;'%arch in line:
            if 'RELEASE_QUEUE=%s;'%cmssw in line:
                match = re.search('PKGTOOLS_TAG=V\d\d-\d\d-..;',line)
                if match:
                    pkgtools_tag = match.group()[13:-1]
                else:
                    raise Exception( 'Could not extract PKGTOOLS_TAG from line:%s'%line )
                match = re.search('CMSDIST_TAG=IB/CMSSW_\d_\d_.*/.*;RELEASE',line)
                if match:
                    cmsdist_tag = match.group()[12:-8]
                else:
                    raise Exception( 'Could not extract PKGTOOLS_TAG from line:%s'%line )
    if not options.quiet:
        print ""
        print "ARCH:\t\t " + arch
        print "CMSSW:\t\t " + cmssw
        print "PKGTOOLS_TAG:\t " + pkgtools_tag
        print "CMSDIST_TAG:\t " + cmsdist_tag
        print ""

    cmd = 'git clone -b %s git@github.com:cms-sw/cmsdist.git CMSDIST'%cmsdist_tag
    if not options.quiet:
        print 'about to execute:'
        print cmd
        raw_input('continue?')
    p = subprocess.Popen(cmd, shell = True)
    p.wait()

    cmd = 'git clone -b %s git@github.com:cms-sw/pkgtools.git PKGTOOLS'%pkgtools_tag
    if not options.quiet:
        print 'about to execute:'
        print cmd
        raw_input('continue?')
    p = subprocess.Popen(cmd, shell = True)
    p.wait()


    print "next steps:"
    print "-cd to topdir and make your changes in cmsdist:"
    print "\t cd %s"%topdir
    print "-try building cmssw"
    print "\t screen -L time PKGTOOLS/cmsBuild -i a -a %s --builders 4 -j $(($(getconf _NPROCESSORS_ONLN) * 2)) build cmssw-tool-conf"%arch
    print "-push your changes"
    print "-create a pull request"



# Standard boilerplate to call the main() function to begin
# the program.
if __name__ == '__main__':
  main()
