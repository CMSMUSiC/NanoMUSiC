#!/bin/env python
import ROOT

import argparse, os, sys
import subprocess

scale_type_dict = {"": 0, "Up":1, "Down":-1}

def main():
    args = parse_arguments()

    if not os.path.isfile( args.inputjson ):
        print "Can't find file: %s" % args.inputjson
        sys.exit( 1 )

    outfile_name = "NVtx_" + os.path.splitext( os.path.basename(  args.inputjson ) )[0] + ".root"

    outfile = ROOT.TFile.Open( outfile_name, "RECREATE")

    print "### Adjust and add these lines to your config or non_particle.cff for general use###"
    for uncerttype in scale_type_dict:
        outfilename = create_PU_hist( args, uncerttype )
        pileup_file = ROOT.TFile.Open( outfilename )
        print 'Pileup.DataHistFile%s = "$PXLANA/ConfigFiles/ConfigInputs/%s"' % ( uncerttype, outfile_name )
        hist = pileup_file.Get( "pileup" )
        hist.Scale( 1. / hist.Integral( 0, hist.GetNbinsX() ) )
        delimiter = ""
        if len( uncerttype ) > 0: delimiter = "_"
        hist.SetName( "pileup" + delimiter + uncerttype )
        print 'Pileup.DataHistName%s = "pileup%s%s"' % ( uncerttype, delimiter, uncerttype )
        outfile.cd()
        hist.Write()
        pileup_file.Close()

    outfile.Close()

def create_PU_hist( args, uncerttype ):

    arguments = ["pileupCalc.py"]
    arguments += ["-i", args.inputjson]
    #arguments += ["--inputLumiJSON", "/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions15/13TeV/PileUp/pileup_latest.txt"]
    #arguments += ["--inputLumiJSON", "/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV/PileUp/UltraLegacy/pileup_latest.txt"]
    arguments += ["--inputLumiJSON", "./pileup_latest.txt"]
    arguments += ["--calcMode", "true"]
    arguments += ["--maxPileupBin %d" % args.maxPileupBin, "--numPileupBins 100"]
    scalefac =  1 + scale_type_dict[ uncerttype ] * args.minBiasXsecUncert
    arguments += ["--minBiasXsec", "%.3f" % (args.minBiasXsec * scalefac ) ]
    outname = args.inputjson.split(".")[0] + uncerttype + "_raw" + ".root"
    arguments += [ outname ]
    print " ".join( arguments )
    #subprocess.call( map( str, arguments ), stderr=sys.stderr, stdout=sys.stdout )
    subprocess.call( " ".join( arguments ), shell=True, stderr=sys.stderr, stdout=sys.stdout )
    return outname

def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""
    dscr = "Create PileupHistos with syst shifts for data."
    dscr += "If option mcfile is used a weigting hist is created on the fly."
    parser = argparse.ArgumentParser( description= dscr, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('inputjson', help='A json file containing the lumi sections for your analysis ')
    parser.add_argument('--mcfile_pilup_scenario', default="", help='The pileup scenario used for the mcfile')
    parser.add_argument('--minBiasXsec', type=float, default=69000, help='The minimum bias cross section')
    parser.add_argument('--minBiasXsecUncert', type=float, default=0.05, help='The minimum bias cross section uncertainty in percent')
    parser.add_argument('--maxPileupBin', type=int, default=100, help='The number of bins for the nvtx distribution')

    args = parser.parse_args()

    return args



if __name__=="__main__":
    main()
