## MC simulation skimming config
#
# This config is meant to be used for MC files. This means that the sample is
# not a data sample and usually means that it contains more than just the GEN
# information. Thus both runOnData and runOnGen are False by default.
#
# The fileNames variable contains the explicit file names for local testing. One
# can either choose locally stored files or those that are kept on the CERN
# tiers.

# Run on data
runOnData = True
# Run on GEN sample
runOnGen = False

import FWCore.ParameterSet.Config as cms
import sys

# Verbosity: 0 = normal messaging, 1 = human readable, 2 = insane, 3 = INFO from all modules
verbosityLvl = 0

if runOnGen and runOnData :
    print "runOnData and runOnGen can't be true at the same time!"
    import sys
    sys.exit( 1 )

import PxlSkimmer.Skimming.Tools_miniAOD as Tools_miniAOD

print sys.argv
name="test"
datasetpath="/SingleElectron/Run2017D-31Mar2018-v1/MINIAOD"
globalTag="94X_dataRun2_v11"
for option in sys.argv:
    splitoption=option.split('=')
    if "name" in option and len(splitoption) > 1:
        name = splitoption[1]
    if "datasetpath" in option and len(splitoption) > 1:
        datasetpath = splitoption[1]
    if "globalTag" in option and len(splitoption) > 1:
        globalTag = splitoption[1]

print "music_crab3 name %s"%name
print "music_crab3 datasetpath %s"%datasetpath
print "music_crab3 globalTag %s"%globalTag

process = Tools_miniAOD.prepare( runOnGen, runOnData, name, datasetpath, globalTag, year=2017, updateJEC=True, readJECFromDB=False, verbosity=verbosityLvl )

# source
process.source = cms.Source(
    'PoolSource',
    skipEvents = cms.untracked.uint32( 0 ),
    fileNames = cms.untracked.vstring(
        '/store/data/Run2017D/SingleElectron/MINIAOD/31Mar2018-v1/100000/066FB563-1739-E811-BABA-B8CA3A70A520.root'
        )
    )

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32( -1 ) )
print 'INFO: Using global tag:', process.GlobalTag.globaltag
