#!/bin/env python
"""
ECScaler.py - tool for scaling of TEventClasses
"""
from __future__ import print_function

import configargparser # MUSiC Utils combined ar
from roothelpers import root_setup
from ectools.register import ecroot


def scale_to_lumi( conf, i=None, N=None, mc_ec=None, data_ec=None, **kwargs ):
    if not mc_ec and not data_ec:
        return False

    if i % 20 == 0:
        percent = int( 100.*i/N )
        print( "Progress: %d %%: %d / %d" % (percent, i, N) )

    if mc_ec:
        mc_ec.addChangeLogEntry( "Scaled luminosity to %g" % conf.lumi )
        mc_ec.scaleLumi( conf.lumi )
    if data_ec:
        data_ec.addChangeLogEntry( "Set luminosity to %g" % conf.lumi )
        data_ec.setLumi( conf.lumi )

def scale_with_xsec_factor( conf, i=None, N=None, mc_ec=None, data_ec=None, **kwargs ):
    if not mc_ec and not data_ec:
        return False

    if i % 20 == 0:
        percent = int( 100.*i/N )
        print( "Progress: %d %%: %d / %d" % (percent, i, N) )

    if mc_ec:
        mc_ec.addChangeLogEntry( "Scale all cross-sections by factor %g" % conf.xsec_factor )
        mc_ec.scaleAllCrossSections( conf.xsec_factor )

def print_lumi( filename ):
    import ROOT
    file = ROOT.TFile.Open( filename )
    try:
        key = file.GetKey("Rec_Empty+X")
        obj = key.ReadObj()
        print( "Luminosity of '%s': %g pb^-1" % ( filename, obj.getLumi() ) )
        processes = list( obj.getGlobalProcessList() )
        print( "%d processes found" % len( processes ) )
        print( "Cross-Section * k-Factor * Filter-Efficiency:" )
        total_xsec = 0
        for process in processes:
            xsec = obj.getCrossSection( process )
            print( "\t%s: %g pb" % ( process, xsec ) )
            total_xsec += xsec
        print( "Total x-sec: %g pb" % total_xsec )
        obj.Delete()
    finally:
        file.Close()

def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""

    parser = configargparser.ArgumentParser( description="ECSaler.py - tool for scaling of TEventClasses" )

    ecroot.add_ec_standard_options( parser )
    parser.add_argument( "--lumi", help="New luminosity used to scale hists, in pb^-1", type=float )
    parser.add_argument( "--xsec-factor", help="Scale all cross sections by this factor", type=float )

    conf = parser.parse_args()

    query = not (conf.lumi or conf.xsec_factor)

    if query:
        print( "Will only query current luminosity." )
    if conf.lumi and conf.data:
        print( "Lumi scaling is only applied to MC. Setting new luminosity for data." )
    if conf.xsec_factor and conf.data:
        print( "Cross section scaling only makes sense for MC." )
        exit( 1 )
    if not query and not conf.ecout:
        print( "Will not write anything to disk. Please provide an output file (--ecout) for that." )
        exit( 1 )
    return conf

def main():
    """main function of the project."""

    conf = parse_arguments()

    query = not (conf.lumi or conf.xsec_factor)

    if conf.lumi:
        ecroot.ec_loop( conf, scale_to_lumi )
    if conf.xsec_factor:
        ecroot.ec_loop( conf, scale_with_xsec_factor )
    if query:
        root_setup()

        if conf.mc:
            print_lumi( conf.mc )
        if conf.data:
            print_lumi( conf.data )


if __name__=="__main__":
    main()
