# cmssw imports
import FWCore.ParameterSet.Config as cms
import FWCore.Framework.test.cmsExceptionsFatalOption_cff
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *
from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection
from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD
from CondCore.CondDB.CondDB_cfi import * #from CondCore.DBCommon.CondDBSetup_cfi import * LOR COM FOR CMSSW106X

# This function will configure the Skimmer based on the settings in PxlSkimmer_miniAOD_cfi.py
# and the python config you are using (e.g. mc_miniAOD.py / data_miniAOD.py)
#
def prepare( runOnGen, runOnData, name, datasetpath, globalTag, year, updateJEC=True, readJECFromDB=False, verbosity=0, runOnFast=False ):
    # create process, add modules will be added by calling functions below
    #
    process = cms.Process( 'PATprepare' )
    process.options = cms.untracked.PSet( allowUnscheduled = cms.untracked.bool(True) )

    # configure cmssw messenger
    #
    configureMessenger( process, verbosity )

    # load cmssw modules
    #
    # Transient track builder is used for the muon vertex refit in the ADD analysis
    process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')
    # connect to global tag
    # the global tag is set in pset file or overidden by the calling
    # script (e.g. music_crab3.py)
    process.load('Configuration.StandardSequences.Services_cff')
    process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
    process.GlobalTag.globaltag = globalTag
    process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
    process.load('Configuration.StandardSequences.MagneticField_cff')

    # load skimmer configuration from PxlSkimmer_miniAOD_cfi.py
    # adjust settings with arguemnts from prepare function
    #
    if year == 2016:
        process.load( 'PxlSkimmer.Skimming.PxlSkimmer_miniAOD_2016_cfi' )
    elif year == 2017:
        process.load( 'PxlSkimmer.Skimming.PxlSkimmer_miniAOD_2017_cfi' )
    elif year == 2018:
        process.load( 'PxlSkimmer.Skimming.PxlSkimmer_miniAOD_2018_cfi' )
    else:
        raise ValueError("Year must be either 2016, 2017 or 2018")

    process.Skimmer.FastSim = runOnFast
    process.Skimmer.FileName = name+'.pxlio'
    process.Skimmer.Process = name
    process.Skimmer.Dataset = datasetpath

    # update JEC for jets and PFMET according to arguments
    #
    if not updateJEC and readJECFromDB:
        print 'WARNING: readJECFromDB is true, while updateJEC is false'
        print 'readJECFromDB is set to false'
        readJECFromDB = False
    if updateJEC:
        updateJetMET( process, runOnData, readJECFromDB, process.Skimmer.jecConf, year)

    # Several filters are used while running over data or MC.
    # In order to be flexible, events *not* passing these filtes we do not want
    # to throw these events away but rather write the outcome as a bool into the
    # event.
    # To do this, each filter runs in an own path. These paths are stored in the
    # filterlist. This list is later used to access the value of the filter with
    # help of edm::TriggerResult.
    #
    process.Skimmer.filterlist = cms.vstring()
    process.Skimmer.filters.AllFilters.paths = process.Skimmer.filterlist
    process.Skimmer.filters.AllFilters.process = process.name_()

    # The skimmer is in the endpath because then the results of all preceding paths
    # are available. This is used to access the outcome of filters that ran.
    #
    process.p = cms.Path(process.jecSequence)
    addTauIDs( process )
    addPrefiringModule(process, year)
    #addHighptEleMETCorr(process) # LOR COR FOR CMSSW106X
    rerunMetFilter(process, year)
    scaleAndSmearing(process, year)
    process.e = cms.EndPath( process.Skimmer )

    return process

# Do EGamma smearing for 2018
#LOR COMMENTED IT OUT. TRY TO IMPLEMENT for UL2017
#def scaleAndSmearing(process, year):
    # See https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaMiniAODV2 for more info
#    if year != 2018:
#        return
#    from RecoEgamma.EgammaTools.EgammaPostRecoTools import setupEgammaPostRecoSeq
#    setupEgammaPostRecoSeq(process,era='2018-Prompt')  
#    process.p += process.egammaPostRecoSeq
    #a sequence egammaPostRecoSeq has now been created and should be added to your path, eg process.p=cms.Path(process.egammaPostRecoSeq)

# Do EGamma smearing for UL2017
def scaleAndSmearing(process, year):
    # See https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaMiniAODV2 for more info
    if year == 2017:
        from RecoEgamma.EgammaTools.EgammaPostRecoTools import setupEgammaPostRecoSeq
        setupEgammaPostRecoSeq(process,
                       runVID=False, #saves CPU time by not needlessly re-running VID, if you want the Fall17V2 IDs, set this to True or remove (default is True)
                       era='2017-UL')    
        process.p += process.egammaPostRecoSeq
    #a sequence egammaPostRecoSeq has now been created and should be added to your path, eg process.p=cms.Path(process.egammaPostRecoSeq)



# Rerun ecalBadCalibReducedMINIAODFilter (see: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2)
def rerunMetFilter(process, year):
    if year == 2016:
        return
    process.load('RecoMET.METFilters.ecalBadCalibFilter_cfi')

    baddetEcallist = cms.vuint32(
        [872439604,872422825,872420274,872423218,
         872423215,872416066,872435036,872439336,
         872420273,872436907,872420147,872439731,
         872436657,872420397,872439732,872439339,
         872439603,872422436,872439861,872437051,
         872437052,872420649,872422436,872421950,
         872437185,872422564,872421566,872421695,
         872421955,872421567,872437184,872421951,
         872421694,872437056,872437057,872437313])

    process.ecalBadCalibReducedMINIAODFilter = cms.EDFilter(
        "EcalBadCalibFilter",
        EcalRecHitSource = cms.InputTag("reducedEgamma:reducedEERecHits"),
        ecalMinEt        = cms.double(50.),
        baddetEcal    = baddetEcallist,
        taggingMode = cms.bool(True),
        debug = cms.bool(False)
        )
    process.p += process.ecalBadCalibReducedMINIAODFilter

# High pt electron MET correction (see https://github.com/cms-sw/cmssw/commit/350527dc4758894a09f790ce05ba3ae9389a1e11)
def addHighptEleMETCorr(process):
    process.corrEgammaBadPF = cms.EDProducer("EGMiniAODType1METCorrProducer",
        eles=cms.InputTag("slimmedElectrons"),
        jets=cms.InputTag("slimmedJets"),
        correctorCfg = cms.PSet(
            minJetPt = cms.double(15),
            maxJetEMFrac = cms.double(0.9)
        )
    )


    process.slimmedMetsEGFixed = cms.EDProducer("CorrectedPatMETProducer",
        src = cms.InputTag("slimmedMETs"),
        srcCorrections = cms.VInputTag(cms.InputTag("corrEgammaBadPF"))
    )


    process.p += cms.Sequence( process.corrEgammaBadPF*process.slimmedMetsEGFixed )

# Prefiring
#
def addPrefiringModule(process, year):
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/L1ECALPrefiringWeightRecipe
    #if year == 2018:
    #    return
    if year == 2017:
        data_era = "2017BtoF"
    elif year == 2016:
        data_era = "2016BtoH"
    else:
        raise ValueError("Year must be either 2016, 2017 or 2018")
    

    if year == 2017: #NEW PERCEIPE ONLY FOR UL2017 https://twiki.cern.ch/twiki/bin/viewauth/CMS/L1ECALPrefiringWeightRecipe
        # from PhysicsTools.PatUtils.l1ECALPrefiringWeightProducer_cfi import l1ECALPrefiringWeightProducer
        from PhysicsTools.PatUtils.l1PrefiringWeightProducer_cfi import l1PrefiringWeightProducer
        # process.prefiringweight= l1ECALPrefiringWeightProducer.clone(
        # process.prefiringweight= l1PrefiringWeightProducer.clone(
        #     #TheJets = cms.InputTag("updatedPatJetsUpdatedJEC"), #this should be the slimmedJets collection with up to date JECs !
        #     TheJets = cms.InputTag("updatedPatJetsUpdatedJECAK4"), #this should be the slimmedJets collection with up to date JECs !
        #     L1Maps = cms.string("L1PrefiringMaps.root"),
        #     DataEra = cms.string('UL2017BtoF'),
        #     UseJetEMPt = cms.bool(False),
        #     PrefiringRateSystematicUncty = cms.double(0.2),
        #     SkipWarnings = False
        #     )
        process.prefiringweight = l1PrefiringWeightProducer.clone(
            # TheJets = cms.InputTag("updatedPatJetsUpdatedJEC"), #this should be the slimmedJets collection with up to date JECs !
            TheJets = cms.InputTag("updatedPatJetsUpdatedJECAK4"), #this should be the slimmedJets collection with up to date JECs !
            DataEraECAL = cms.string("UL2017BtoF"),
            DataEraMuon = cms.string("20172018"),
            UseJetEMPt = cms.bool(False),
            PrefiringRateSystematicUnctyECAL = cms.double(0.2),
            PrefiringRateSystematicUnctyMuon = cms.double(0.2)
        )

    elif year == 2018: # Recipe used : https://twiki.cern.ch/twiki/bin/viewauth/CMS/L1PrefiringWeightRecipe#2018_UL
        from PhysicsTools.PatUtils.l1PrefiringWeightProducer_cfi import l1PrefiringWeightProducer 
        process.prefiringweight = l1PrefiringWeightProducer.clone(
            TheJets = cms.InputTag("updatedPatJetsUpdatedJEC"), #this should be the slimmedJets collection with up to date JECs !
            DataEraECAL = cms.string("None"),
            DataEraMuon = cms.string("20172018"),
            UseJetEMPt = cms.bool(False),
            PrefiringRateSystematicUnctyECAL = cms.double(0.2),
            PrefiringRateSystematicUnctyMuon = cms.double(0.2)
        )


    #The first part below should be for all not UL samples. LOR is using at the moment 2017 as UL2017
    else:
        from PhysicsTools.PatUtils.l1ECALPrefiringWeightProducer_cfi import l1ECALPrefiringWeightProducer
        process.prefiringweight = l1ECALPrefiringWeightProducer.clone(
            DataEra = cms.string(data_era), #Use 2016BtoH for 2016
            UseJetEMPt = cms.bool(False),
            PrefiringRateSystematicUncty = cms.double(0.2),
            SkipWarnings = False)

    process.p += process.prefiringweight



# functions for adding IDs
#
def addTauIDs( process ): #Try to do 
    updatedTauName = "slimmedTausNewID" #name of pat::Tau collection with new tau-Ids
    #from runTauIdMVA import *
    import RecoTauTag.RecoTau.tools.runTauIdMVA as tauIdConfig
    tauIdEmbedder = tauIdConfig.TauIDEmbedder(process, cms, debug = False,
                        updatedTauName = updatedTauName,
                        toKeep = [ "2017v2", "dR0p32017v2", "newDM2017v2",
                                   "deepTau2017v2p1", #deepTau TauIDs
                                  #"2017v2", #mva2017v2
                                   ])
    tauIdEmbedder.runTauID()
    # Path and EndPath definitions
    process.p += process.rerunMvaIsolationSequence * getattr(process,updatedTauName) #* process.NewTauIDsEmbedded
        
        #from RecoTauTag.RecoTau.TauDiscriminatorTools import noPrediscriminants
    #process.load('RecoTauTag.Configuration.loadRecoTauTagMVAsFromPrepDB_cfi')
    #from RecoTauTag.RecoTau.PATTauDiscriminationByMVAIsolationRun2_cff import *

    #process.rerunDiscriminationByIsolationMVArun2v1raw = patDiscriminationByIsolationMVArun2v1raw.clone(
    #    PATTauProducer = cms.InputTag('slimmedTaus'),
    #    Prediscriminants = noPrediscriminants,
    #    loadMVAfromDB = cms.bool(True),
    #    mvaName = cms.string("RecoTauTag_tauIdMVAIsoDBoldDMwLT2016v1"), # name of the training you want to use
    #    mvaOpt = cms.string("DBoldDMwLT"), # option you want to use for your training (i.e., which variables are used to compute the BDT score)
    #    requireDecayMode = cms.bool(True),
    #    verbosity = cms.int32(0)
    #)

    #process.rerunDiscriminationByIsolationMVArun2v1VLoose = patDiscriminationByIsolationMVArun2v1VLoose.clone(
    #    PATTauProducer = cms.InputTag('slimmedTaus'),
    #    Prediscriminants = noPrediscriminants,
    #    toMultiplex = cms.InputTag('rerunDiscriminationByIsolationMVArun2v1raw'),
    #    key = cms.InputTag('rerunDiscriminationByIsolationMVArun2v1raw:category'),
    #    loadMVAfromDB = cms.bool(True),
    #    mvaOutput_normalization = cms.string("RecoTauTag_tauIdMVAIsoDBoldDMwLT2016v1_mvaOutput_normalization"), # normalization of the training you want to use
    #    mapping = cms.VPSet(
    #        cms.PSet(
    #            category = cms.uint32(0),
    #            cut = cms.string("RecoTauTag_tauIdMVAIsoDBoldDMwLT2016v1_WPEff90"), # this is the name of the working point you want to use
    #            variable = cms.string("pt"),
    #        )
    #    )
    #)

    ## here we produce all the other working points for the training
    #process.rerunDiscriminationByIsolationMVArun2v1Loose = process.rerunDiscriminationByIsolationMVArun2v1VLoose.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1Loose.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBoldDMwLT2016v1_WPEff80")
    #process.rerunDiscriminationByIsolationMVArun2v1Medium = process.rerunDiscriminationByIsolationMVArun2v1VLoose.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1Medium.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBoldDMwLT2016v1_WPEff70")
    #process.rerunDiscriminationByIsolationMVArun2v1Tight = process.rerunDiscriminationByIsolationMVArun2v1VLoose.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1Tight.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBoldDMwLT2016v1_WPEff60")
    #process.rerunDiscriminationByIsolationMVArun2v1VTight = process.rerunDiscriminationByIsolationMVArun2v1VLoose.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1VTight.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBoldDMwLT2016v1_WPEff50")
    #process.rerunDiscriminationByIsolationMVArun2v1VVTight = process.rerunDiscriminationByIsolationMVArun2v1VLoose.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1VVTight.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBoldDMwLT2016v1_WPEff40")

    ## this sequence has to be included in your cms.Path() before your analyzer which accesses the new variables is called.
    #process.rerunMvaIsolation2SeqRun2 = cms.Sequence(
    #    process.rerunDiscriminationByIsolationMVArun2v1raw
    #    *process.rerunDiscriminationByIsolationMVArun2v1VLoose
    #    *process.rerunDiscriminationByIsolationMVArun2v1Loose
    #    *process.rerunDiscriminationByIsolationMVArun2v1Medium
    #    *process.rerunDiscriminationByIsolationMVArun2v1Tight
    #    *process.rerunDiscriminationByIsolationMVArun2v1VTight
    #    *process.rerunDiscriminationByIsolationMVArun2v1VVTight
    #)

    #process.p += process.rerunMvaIsolation2SeqRun2

    #process.rerunDiscriminationByIsolationMVArun2v1rawNewDM = patDiscriminationByIsolationMVArun2v1raw.clone(
    #    PATTauProducer = cms.InputTag('slimmedTaus'),
    #    Prediscriminants = noPrediscriminants,
    #    loadMVAfromDB = cms.bool(True),
    #    mvaName = cms.string("RecoTauTag_tauIdMVAIsoDBnewDMwLT2016v1"), # name of the training you want to use
    #    mvaOpt = cms.string("DBnewDMwLT"), # option you want to use for your training (i.e., which variables are used to compute the BDT score)
    #    requireDecayMode = cms.bool(True),
    #    verbosity = cms.int32(0)
    #)

    #process.rerunDiscriminationByIsolationMVArun2v1VLooseNewDM = patDiscriminationByIsolationMVArun2v1VLoose.clone(
    #    PATTauProducer = cms.InputTag('slimmedTaus'),
    #    Prediscriminants = noPrediscriminants,
    #    toMultiplex = cms.InputTag('rerunDiscriminationByIsolationMVArun2v1raw'),
    #    key = cms.InputTag('rerunDiscriminationByIsolationMVArun2v1raw:category'),
    #    loadMVAfromDB = cms.bool(True),
    #    mvaOutput_normalization = cms.string("RecoTauTag_tauIdMVAIsoDBnewDMwLT2016v1_mvaOutput_normalization"), # normalization of the training you want to use
    #    mapping = cms.VPSet(
    #        cms.PSet(
    #            category = cms.uint32(0),
    #            cut = cms.string("RecoTauTag_tauIdMVAIsoDBnewDMwLT2016v1_WPEff90"), # this is the name of the working point you want to use
    #            variable = cms.string("pt"),
    #        )
    #    )
    #)

    ## here we produce all the other working points for the training
    #process.rerunDiscriminationByIsolationMVArun2v1LooseNewDM = process.rerunDiscriminationByIsolationMVArun2v1VLooseNewDM.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1LooseNewDM.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBnewDMwLT2016v1_WPEff80")
    #process.rerunDiscriminationByIsolationMVArun2v1MediumNewDM = process.rerunDiscriminationByIsolationMVArun2v1VLooseNewDM.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1MediumNewDM.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBnewDMwLT2016v1_WPEff70")
    #process.rerunDiscriminationByIsolationMVArun2v1TightNewDM = process.rerunDiscriminationByIsolationMVArun2v1VLooseNewDM.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1TightNewDM.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBnewDMwLT2016v1_WPEff60")
    #process.rerunDiscriminationByIsolationMVArun2v1VTightNewDM = process.rerunDiscriminationByIsolationMVArun2v1VLooseNewDM.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1VTightNewDM.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBnewDMwLT2016v1_WPEff50")
    #process.rerunDiscriminationByIsolationMVArun2v1VVTightNewDM = process.rerunDiscriminationByIsolationMVArun2v1VLooseNewDM.clone()
    #process.rerunDiscriminationByIsolationMVArun2v1VVTightNewDM.mapping[0].cut = cms.string("RecoTauTag_tauIdMVAIsoDBnewDMwLT2016v1_WPEff40")

    ## this sequence has to be included in your cms.Path() before your analyzer which accesses the new variables is called.
    #process.rerunMvaIsolation2SeqRun2NewDM = cms.Sequence(
    #    process.rerunDiscriminationByIsolationMVArun2v1rawNewDM
    #    *process.rerunDiscriminationByIsolationMVArun2v1VLooseNewDM
    #    *process.rerunDiscriminationByIsolationMVArun2v1LooseNewDM
    #    *process.rerunDiscriminationByIsolationMVArun2v1MediumNewDM
    #    *process.rerunDiscriminationByIsolationMVArun2v1TightNewDM
    #    *process.rerunDiscriminationByIsolationMVArun2v1VTightNewDM
    #    *process.rerunDiscriminationByIsolationMVArun2v1VVTightNewDM
    #)

    #process.p += process.rerunMvaIsolation2SeqRun2NewDM

# function to update JEC and propagate changes to MET
#
def updateJetMET( process, runOnData, readJECFromDB, jecConf, year ):
    # miniAOD already contain JEC after production
    # if newer JEC are released they need to be updated
    # these changes have to be propagated to MET
    # JEC can either be read from the globalTag or from a file
    # code is based on
    # https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections?rev=141
    # https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription?rev=72
    #

    # update reco labels of jets which will be used in skimmer
    #
    process.Skimmer.jets.AK4.RecoLabel = cms.InputTag('updatedPatJetsUpdatedJECAK4')
    process.Skimmer.jets.AK8.RecoLabel = cms.InputTag('updatedPatJetsUpdatedJECAK8')
    process.Skimmer.isolationDefinitions.jetCollection = cms.InputTag('updatedPatJetsUpdatedJECAK4')

    # this is only needed when JEC should be read from sqlite DB
    # the names below have to be specified in PxlSkimmer_miniAOD_cfi.py
    # the tags differ for data / mc and database name
    #
    if readJECFromDB:
        process.load("CondCore.DBCommon.CondDBCommon_cfi")
        if runOnData:
            corrTagAK4chs = jecConf.corrTagAK4chsData
            corrTagAK8chs = jecConf.corrTagAK8chsData
            dbName = jecConf.dbNameData
        else:
            corrTagAK4chs = jecConf.corrTagAK4chsMC
            corrTagAK8chs = jecConf.corrTagAK8chsMC
            dbName = jecConf.dbNameMC
        process.jec = cms.ESSource("PoolDBESSource",
                                   connect = dbName,
                                   DBParameters = cms.PSet(
                                        messageLevel = cms.untracked.int32(0)
                                        ),
                                   timetype = cms.string('runnumber'),
                                   toGet = cms.VPSet(
                                   cms.PSet(
                                            record = cms.string('JetCorrectionsRecord'),
                                            tag    = corrTagAK4chs,
                                            label  = cms.untracked.string('AK4PFchs')
                                            ),
                                   cms.PSet(
                                            record = cms.string('JetCorrectionsRecord'),
                                            tag    = corrTagAK8chs,
                                            label  = cms.untracked.string('AK8PFchs')
                                            ),
                                   )
        )
        # add an es_prefer statement to resolve a possible conflict from simultaneous connection to a global tag
        process.es_prefer_jec = cms.ESPrefer('PoolDBESSource','jec')

    # chose levels of JEC which should be applied. L2L3Residual is safe for MC (equal 1)
    #
    corrections = cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute', 'L2L3Residual'])

    # actual update of jets
    updateJetCollection(process,
                        jetSource = cms.InputTag('slimmedJets'),
                        labelName = 'UpdatedJECAK4',
                        jetCorrections = ('AK4PFchs', corrections, 'None'))
    updateJetCollection(process,
                        jetSource = cms.InputTag('slimmedJetsAK8'),
                        labelName = 'UpdatedJECAK8',
                        jetCorrections = ('AK8PFchs', corrections, 'None'))

    # propagate changes to type1 corrected PFlow MET
    if year == 2017:
        # 2017 EE Noise Issue
        # https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETUncertaintyPrescription#Instructions_for_9_4_X_X_9_for_2
        runMetCorAndUncFromMiniAOD(process,
                                   isData=runOnData,
                                   fixEE2017=True,
                                   fixEE2017Params={'userawPt': True, 'ptThreshold':50.0, 'minEtaThreshold':2.65, 'maxEtaThreshold': 3.139},
                                   )
    else:
        runMetCorAndUncFromMiniAOD(process, isData=runOnData)
    process.jecSequence = cms.Sequence(process.patJetCorrFactorsUpdatedJECAK4*process.updatedPatJetsUpdatedJECAK4*process.patJetCorrFactorsUpdatedJECAK8*process.updatedPatJetsUpdatedJECAK8*process.fullPatMetSequence)


# functions for adding genfilters
#
def addMuGenFilter( process, pt ):
    mugenfilterName = 'mugenfilter' + str( pt )
    # this filter selects events containing muons with pt > pt GeV ...
    mugenfilter = cms.EDFilter( 'MCSmartSingleGenParticleFilter',
                                MaxDecayRadius = cms.untracked.vdouble( 2000.0, 2000.0 ),
                                Status = cms.untracked.vint32( 1, 1 ),
                                MinPt = cms.untracked.vdouble( float( pt ), float( pt ) ),
                                ParticleID = cms.untracked.vint32( 13, -13 ),
                                MaxEta = cms.untracked.vdouble( 2.5, 2.5 ),
                                MinEta = cms.untracked.vdouble( -2.5, -2.5 ),
                                MaxDecayZ = cms.untracked.vdouble( 4000.0, 4000.0 ),
                                MinDecayZ = cms.untracked.vdouble( -4000.0, -4000.0 ),
                                genParSource = cms.InputTag( 'prunedGenParticles' )
                                )
    setattr( process, mugenfilterName, mugenfilter.clone() )

    # ... but we don't want these events
    setattr( process, 'p_' + mugenfilterName, cms.Path( ~getattr( process, mugenfilterName ) ) )
    process.Skimmer.filterlist.append( 'p_' + mugenfilterName )

def addEMFilter( process ):
    # this filter selects events containing at least one potential electron candidate ...
    process.emenrichingfilter = cms.EDFilter( 'EMEnrichingFilter',
                                              filterAlgoPSet = cms.PSet( requireTrackMatch = cms.bool( False ),
                                                                         caloIsoMax = cms.double( 10.0 ),
                                                                         isoGenParConeSize = cms.double( 0.1 ),
                                                                         tkIsoMax = cms.double( 5.0 ),
                                                                         hOverEMax = cms.double( 0.5 ),
                                                                         isoGenParETMin = cms.double( 5.0 ),
                                                                         genParSource = cms.InputTag( 'prunedGenParticles' ),
                                                                         isoConeSize = cms.double( 0.2 ),
                                                                         clusterThreshold = cms.double( 5.0 )
                                                                         )
                                              )

    # ... but we don't want these events
    process.p_emenrichingfilter = cms.Path( ~process.emenrichingfilter )
    process.Skimmer.filterlist.append( 'p_emenrichingfilter' )


def addBCtoEFilter( process ):
    # this filter selects events containing electrons that come from b or c hadrons ...
    process.bctoefilter = cms.EDFilter( 'BCToEFilter',
                                        filterAlgoPSet = cms.PSet( genParSource = cms.InputTag( 'prunedGenParticles' ),
                                                                   eTThreshold = cms.double( 10 )
                                                                   )
                                        )

    # ... but we don't want these events
    process.p_bctoefilter = cms.Path( ~process.bctoefilter )
    process.Skimmer.filterlist.append( 'p_bctoefilter' )


def addBFilter( process ):
    # this filter selects events containing b quarks
    process.bbfilter = cms.EDFilter( 'MCSingleGenParticleFilter',
                                     genParSource = cms.InputTag( 'prunedGenParticles' ),
                                     ParticleID = cms.untracked.vint32( 5, -5 ),
                                     Status = cms.untracked.vint32( 2, 2 )
                                     )

    # ... but we don't want these events
    process.p_bbfilter = cms.Path( ~process.bbfilter )
    process.Skimmer.filterlist.append( 'p_bbfilter' )


# Initialize MessageLogger and output report.
#
def configureMessenger( process, verbosity = 0 ):
    process.load( 'FWCore.MessageLogger.MessageLogger_cfi' )
    process.MessageLogger.cerr.threshold = 'INFO'
    process.MessageLogger.cerr.default.limit = -1
    process.MessageLogger.cerr.FwkReport.limit = 100
    process.MessageLogger.cerr.FwkReport.reportEvery = 1000
    process.MessageLogger.categories.append( 'TRIGGERINFO_PXLSKIMMER' )
    process.MessageLogger.categories.append( 'PDFINFO_PXLSKIMMER' )
    if verbosity > 0:
        process.MessageLogger.categories.append( 'EventInfo' )
        process.MessageLogger.categories.append( 'FilterInfo' )
        process.MessageLogger.categories.append( 'TriggerInfo' )
        process.MessageLogger.categories.append( 'PDFInfo' )
    if verbosity > 1:
        process.MessageLogger.categories.append( 'PxlSkimmer' )
    if verbosity > 2:
        process.MessageLogger.cerr.INFO = cms.untracked.PSet( limit = cms.untracked.int32( -1 ) )
