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
runOnData = False
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
name="test_DEBSYST_TT_Mtt-1000ToInf_UL17"
datasetpath="dummy"
#LOR TRY TO USE LATEST GlobalTag
#globalTag="94X_mc2017_realistic_v17"
globalTag= "106X_mc2017_realistic_v8"
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

process = Tools_miniAOD.prepare( runOnGen, runOnData, name, datasetpath, globalTag, updateJEC=True, readJECFromDB=False, verbosity=verbosityLvl, year=2017 )

# source
process.source = cms.Source(
    'PoolSource',
    skipEvents = cms.untracked.uint32( 0 ),
    fileNames = cms.untracked.vstring(
        #'/store/mc/RunIIFall17MiniAODv2/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/10000/0893B9A4-FD41-E811-AF47-008CFAC94284.root'
        #'/store/mc/RunIIFall17MiniAODv2/DYJetsToLL_M-50_HT-400to600_TuneCP5_13TeV-madgraphMLM-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/20000/06741C42-CE41-E811-9A60-0CC47A74525A.root' #LOR TEST
        #'/store/mc/RunIISummer19UL17MiniAOD/WToTauNu_M-200_TuneCP5_13TeV-pythia8-tauola/MINIAODSIM/106X_mc2017_realistic_v6-v2/40000/02B4F014-9F1D-3D45-8321-04FA95DD2AB9.root'
        #'/store/mc/RunIISummer19UL17MiniAODv2/DY1JetsToLL_M-50_TuneCP5_13TeV-madgraphMLM-pythia8/MINIAODSIM/106X_mc2017_realistic_v9-v1/270000/03EC145B-727A-8140-831D-184A52CC0579.root' #LOR TRY UL2017 MiniAODv2
        #'/store/mc/RunIISummer20UL17MiniAODv2/ZToMuMu_M-400To800_TuneCP5_13TeV-powheg-pythia8/MINIAODSIM/106X_mc2017_realistic_v9-v2/280000/DA025EE2-8467-C54D-84CB-2C34752C0948.root' #LOR TRY UL2017 MiniAODv2
        '/store/mc/RunIISummer20UL17MiniAODv2/DYJetsToLL_Pt-250To400_MatchEWPDG20_TuneCP5_13TeV-amcatnloFXFX-pythia8/MINIAODSIM/106X_mc2017_realistic_v9-v1/100000/2840407A-35A9-1944-B378-48863821FDA4.root'#LOR TRY UL2017 MiniAODv2
        # '/store/mc/RunIISummer19UL17MiniAODv2/TT_Mtt-1000toInf_TuneCP5_13TeV-powheg-pythia8/MINIAODSIM/106X_mc2017_realistic_v9-v1/270000/58F3546B-9D4B-1544-BABE-9A4648BC35C0.root'
         )
    )

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1 ) )#LOR CHANGE FROM -1 TO TEST
###FIXME for miniAOD!!!
if not runOnGen:
    if not runOnData:
        # This is used for QCD samples only, and the filters only write a flag in the
        # event if they fired or not. The actual selection must happen in the
        # classification, i.e. you have to set in the config file which flag you want to
        # consider.
        #
        # remove events with electrons that come from b or c hadrons
        Tools_miniAOD.addBCtoEFilter( process )

        # remove events containing b quarks
        Tools_miniAOD.addBFilter( process )

        # remove events with at least one potential electron candidate
        Tools_miniAOD.addEMFilter( process )
        
        #LOR TRY TO ADD TAU ID
        #Tools_miniAOD.addTauIDs( process )
        
        # remove events with a muon of more than 5 GeV
        Tools_miniAOD.addMuGenFilter( process, pt = 5 )

        # remove events with a muon of more than 10 GeV
        Tools_miniAOD.addMuGenFilter( process, pt = 10 )

        # remove events with a muon of more than 15 GeV
        Tools_miniAOD.addMuGenFilter( process, pt = 15 )


print 'INFO: Using global tag:', process.GlobalTag.globaltag
