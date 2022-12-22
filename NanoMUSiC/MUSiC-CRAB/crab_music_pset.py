import FWCore.ParameterSet.Config as cms

process = cms.Process('NoSplit')

# process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring("root://xrootd-cms.infn.it///store/mc/RunIISummer20UL17NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/100000/04426F83-BF53-0440-B2D1-F2DD9AB96EB8.root", "root://xrootd-cms.infn.it///store/mc/RunIISummer20UL17NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/100000/04426F83-BF53-0440-B2D1-F2DD9AB96EB8.root"))
process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring("/store/mc/RunIISummer20UL17NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/100000/04426F83-BF53-0440-B2D1-F2DD9AB96EB8.root", "root://xrootd-cms.infn.it///store/mc/RunIISummer20UL17NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/100000/04426F83-BF53-0440-B2D1-F2DD9AB96EB8.root"))
# process.source = cms.Source("EmptySource")
 
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))
# process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

# process.output = cms.OutputModule("PoolOutputModule",
#     outputCommands = cms.untracked.vstring("drop *"),
#     fileName = cms.untracked.string('output.root'),
# ) 
# process.out = cms.EndPath(process.output)
