import FWCore.ParameterSet.Config as cms
process = cms.Process('NANOMUSIC')
process.source = cms.Source(
    "PoolSource",
    # "root://xrootd-cms.infn.it///store/mc/RunIISummer20UL17NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/100000/04426F83-BF53-0440-B2D1-F2DD9AB96EB8.root", "root://xrootd-cms.infn.it///store/mc/RunIISummer20UL17NanoAODv9/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/100000/04426F83-BF53-0440-B2D1-F2DD9AB96EB8.root"
    fileNames=cms.untracked.vstring('DUMMY_NAME.root'),
    # lumisToProcess=cms.untracked.VLuminosityBlockRange("254231:1-254231:24")
)
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(10))
process.output = cms.OutputModule("PoolOutputModule",
                                  fileName=cms.untracked.string('nano_music.root'))
process.out = cms.EndPath(process.output)
