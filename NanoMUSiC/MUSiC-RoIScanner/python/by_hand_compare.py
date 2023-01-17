import json
import math
import argparse

def main():
    args = parse_arguments()
    addUpRegion( args )
    #scanResultSummary( args )

def scanResultSummary( args ):
    with open( args.json , "rb" ) as jfile:
        js = json.load( jfile )

    for res in js["ScanResults"]:
        combined_err = 0.
        #~ combined_err = pow( res[ "mc_error" ], 2 )
        for err in res["errors"]:
            combined_err += pow( float( res["errors"][err] ), 2 )
        print "combined", math.sqrt( combined_err )


def addUpRegion( args ):
    with open( args.json , "rb" ) as jfile:
        js = json.load( jfile )
    nevents = 0.
    mcerror = 0.
    syserror = 0.
    total_syst_map = {}
    mc = 0.
    for i,mcbin in enumerate( js[ "MCBins" ] ):
        le = float( mcbin["lowerEdge"] )
        wi = float( mcbin["width"] )
        print le
        #~ print 'nevents', js["DataBins"][i]
        print 'mc', mcbin["Nevents"]
        if ( le + wi )  > ( args.lower + args.width ):
            break
        if le >= args.lower:
            print "in",i
            mc += mcbin["Nevents"]
            mcerror += float( mcbin["MCerror"]) * float( mcbin["MCerror"])
            for sys in mcbin["systematics"]:
                #if "Eff" in sys and mcbin["systematics"][sys] > 0: print sys, mcbin["systematics"][sys]
                if not sys in total_syst_map:
                    total_syst_map[ sys ] = mcbin["systematics"][sys]
                else:
                    total_syst_map[ sys ] += mcbin["systematics"][sys]
                #~ syserror += pow( float( mcbin["systematics"][sys] ), 2 )
            #~ nevents += js["DataBins"][i]
    mcerror = math.sqrt( mcerror )

    for uncert in ("JES","Uncl", "MuoES", "EleES", "GamES" ):
        total_syst_map[ uncert ] = ( abs( total_syst_map[ uncert + "_Up"] ) + abs( total_syst_map[ uncert + "_Down"] ) ) / 2.
        total_syst_map.pop( uncert + "_Up" )
        total_syst_map.pop(  uncert + "_Down" )
    print "--------"
    for uncert in total_syst_map:
        print uncert, total_syst_map[ uncert ]
        #total_syst_map[ uncert + "_Up"] = 0.
        #total_syst_map[ uncert + "_Down"] = 0.
    for sys in total_syst_map:
        syserror += pow(  total_syst_map[sys] ,2 )
    syserror += pow(  mcerror ,2 )
    syserror = math.sqrt( syserror )
    print "----------"
    print 'nevents', nevents
    print 'mc', mc
    print 'mcerror', mcerror
    print 'syserror', syserror
    print 'total error ', math.sqrt( pow( mcerror, 2) + pow( syserror, 2) )
def parse_arguments():
    parser = argparse.ArgumentParser( description="Validate 8-TeV scan results with the 'new' 13-TeV scanner." )
    #~ parser.add_argument( "json", nargs="+", type=str, help="Pickle file(s) to validate." )
    parser.add_argument( "json",  type=str, help="jsonfile(s) to validate." )
    parser.add_argument( "--lower", type=float, default=10., help="Lower edge for chosen bin" )
    parser.add_argument( "--width", type=float, default=10., help="width for chosen bin" )
    return parser.parse_args()

if __name__=="__main__":
    main()
