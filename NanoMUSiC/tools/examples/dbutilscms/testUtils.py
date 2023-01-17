import argparse
import dbutilscms

parser = argparse.ArgumentParser("Test MCM & das helper")
parser.add_argument("-d", "--datasetpath", default = "/ZZTo2L2Nu_13TeV_powheg_pythia8/RunIISpring15MiniAODv2-74X_mcRun2_asymptotic_v2-v2/MINIAODSIM",help="A datasetpath for testing" )
args = parser.parse_args()

mcm = dbutilscms.McMUtilities()
mcm.readURL( args.datasetpath )

print "Result for datasetpath: "
print args.datasetpath
print "xs", mcm.getCrossSection()
