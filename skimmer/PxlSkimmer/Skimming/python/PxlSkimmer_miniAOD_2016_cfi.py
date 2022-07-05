import FWCore.ParameterSet.Config as cms

Skimmer = cms.EDAnalyzer(
    "PxlSkimmer_miniAOD",
    # Set Year
    Year = cms.untracked.int32(2016),
    #output file name
    FileName =  cms.untracked.string("test_run.pxlio"),
    #symbolic name of the processed data
    Process = cms.untracked.string("test_run"),
    # Dataset of the processed data or MC.
    Dataset = cms.untracked.string( 'test_run' ),
    # GenOnly true mean no Rec-info in event, check for GenJets and GenMET
    GenOnly = cms.untracked.bool( False ),
    # Are we running on a FASTSIM sample?
    FastSim = cms.bool( False ),
    # UseSIM true means to use SIM info for finding converted photons
    UseSIM = cms.untracked.bool( False ),
    #Current pdfsets for the Pdf set mapping in the skimmer
    PdfSetFileName = cms.FileInPath("PxlSkimmer/Skimming/data/pdfsets.txt"),
    #labels of source
    genParticleCandidatesLabel  = cms.InputTag( "prunedGenParticles" ),
    genFinalParticlesLabel      = cms.InputTag( "packedGenParticles" ),
    #vertices with beam spot constraint
    VertexRecoLabel     = cms.untracked.string("offlineSlimmedPrimaryVertices"),
    #the following is all PAT
    patMuonLabel       = cms.InputTag("slimmedMuons"),
    patElectronLabel   = cms.InputTag("slimmedElectrons"),
    patTauTag          = cms.InputTag( 'slimmedTaus' ),
    #patTauTag          = cms.InputTag( 'slimmedTausNewID' ),#LOR CORR FOR CMSSW106X
    patTauBoostedTag   = cms.InputTag( 'slimmedTausBoosted' ),
    patGammaLabel      = cms.InputTag("slimmedPhotons"),
    patMETTag          = cms.InputTag( 'slimmedMETs' ),
    PUPPIMETTag        = cms.InputTag( 'slimmedMETsPuppi' ),
    patMETEleCorrTag   = cms.InputTag( 'slimmedMetsEGFixed' ),
    patPFCandiates     = cms.InputTag( 'packedPFCandidates' ),

    rhos = cms.VInputTag( cms.InputTag( 'fixedGridRhoAll' ),
                       cms.InputTag( 'fixedGridRhoFastjetAll' ),
                       cms.InputTag( 'fixedGridRhoFastjetAllCalo' ),
                       cms.InputTag( 'fixedGridRhoFastjetCentralCalo' ),
                       cms.InputTag( 'fixedGridRhoFastjetCentralChargedPileUp' ),
                       cms.InputTag( 'fixedGridRhoFastjetCentralNeutral' )
                       ),

    bits = cms.InputTag("TriggerResults","","HLT"),
    prescales = cms.InputTag("patTrigger"),

    #ECAL RecHits for supercluster information
    reducedSuperClusterCollection   = cms.InputTag("reducedEgamma","reducedESClusters"),
    reducedEBClusterCollection      = cms.InputTag("reducedEgamma","reducedEBEEClusters"),
    # This contains most filters which are already run by miniAOD as part of PAT.
    # Only met filters are stored as filter results for PAT, real triggers are saved in
    # the HLT collections.
    METFilterTag                    = cms.InputTag("TriggerResults","","PAT"),
    METFilterAlternativeTag         = cms.InputTag("TriggerResults","","RECO"),

    conversionsTag                  = cms.InputTag( "reducedEgamma","reducedConversions" ),
    conversionsSingleLegTag         = cms.InputTag("reducedEgamma","reducedSingleLegConversions" ),

    jets = cms.PSet(
        # REMARK: The names of the following PSets will be used as the names for the PXL particles that are the jets
        # the RecoLabels might be changed in Tools_miniAOD.py if updateJEC is chosen
        AK4 = cms.PSet(
            MCLabel = cms.InputTag( "slimmedGenJets" ),
            RecoLabel = cms.InputTag( "slimmedJets" ),
            isPF = cms.bool(True),
            # the following vector must hold the names of the IDs in the same sequence
            # as the qualities in PhysicsTools/SelectorUtils/interface/PFJetIDSelectionFunctor.h
            version = cms.string('WINTER16'),
            IDs = cms.vstring( 'LOOSE', 'TIGHT', 'TIGHTLEPVETO' )
            ),
        AK8 = cms.PSet(
            MCLabel = cms.InputTag( "slimmedGenJetsAK8" ),
            RecoLabel = cms.InputTag( "slimmedJetsAK8" ),
            isPF = cms.bool(True),
            # the following vector must hold the names of the IDs in the same sequence
            # as the qualities in PhysicsTools/SelectorUtils/interface/PFJetIDSelectionFunctor.h
            version = cms.string('WINTER16'),
            IDs = cms.vstring( 'LOOSE', 'TIGHT', 'TIGHTLEPVETO' )
        ),
    ),
    jecConf =  cms.PSet(
        # only important if readJECFromDB is true
        # to finde the right names:
        # - get db file from https://twiki.cern.ch/twiki/bin/viewauth/CMS/JECDataMC
        # - run conddb --db<dbfile.db> listTags
        # the database file should be put in $CMSSW_BASE/PxlSkimmer/Skimming/data/Spring16_25nsV6_DATA.db
        dbNameData = cms.string('sqlite_file:Spring16_25nsV6_DATA.db'),
        corrTagAK4chsData = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_DATA_AK4PFchs'),
        corrTagAK8chsData = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_DATA_AK8PFchs'),
        dbNameMC = cms.string('sqlite_file:Spring16_25nsV6_MC.db'),
        corrTagAK4chsMC = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_MC_AK4PFchs'),
        corrTagAK8chsMC = cms.string('JetCorrectorParametersCollection_Spring16_25nsV6_MC_AK8PFchs'),
    ),


    triggers = cms.PSet(
        #REMARK: The names of the following PSets will be used as the trigger identifier in the PXL output
        HLT = cms.PSet(
            process = cms.string( 'auto' ),
            L1_result = cms.InputTag( "gtDigis" ),
            results = cms.string('TriggerResults'),
            event   = cms.string('hltTriggerSummaryAOD'),

            # A list of triggers can be defined in the actual *cfg.py.
            # Otherwise all unprescaled triggers from the HLT config will
            # be used for each run.
            HLTriggers = cms.vstring(),
            # Only triggers from datastreams whose name is given in the following list
            # will be considered. Make sure to update this list regularly.
            datastreams = cms.vstring(
                                        "InitialPD",
                                        "Templates"
                                         #"AlCaLumiPixels",
                                        #"AlCaP0",
                                        #"AlCaPhiSym",
                                        "BTagCSV",
                                        "BTagMu",
                                        #"Charmonium",
                                        #"Commissioning",
                                        #"DisplacedJet",
                                        "DoubleEG",
                                        "DoubleMuon",
                                        "DoubleMuonLowMass",
                                        "EcalLaser",
                                        #"EventDisplay",
                                        #"ExpressPhysics",
                                        #"FullTrack",
                                        #"HINCaloJetsOther",
                                        #"HINMuon",
                                        #"HINPFJetsOther",
                                        #"HINPhoton",
                                        #"HLTPhysics",
                                        "HTMHT",
                                        #"HcalHPDNoise",
                                        #"HcalNZS",
                                        #"HighMultiplicity",
                                        "JetHT",
                                        #"L1Accept",
                                        #"LookAreaPD",
                                        "MET",
                                        #"MuOnia",
                                        "MuonEG",
                                        #"NoBPTX",
                                        #"OnlineMonitor",
                                        #"RPCMonitor",
                                        "SingleElectron",
                                        "SingleMuon",
                                        "SinglePhoton",
                                        "Tau",
                                        #"TestEnablesEcalHcal",
                                        #"TestEnablesEcalHcalDQM",
                                        #"ZeroBias",
            ),
        ),
        StoreL3Objects = cms.bool(False),
        trigger_obj = cms.vstring(
            "HLT_Mu8_v",
            "HLT_Mu17_v",
            "HLT_Mu20_v",
            "HLT_Mu27_v",
            "HLT_Mu50_v",
            "HLT_TkMu50_v",
            "HLT_Ele45_WPLoose_Gsf_v",
            "HLT_Ele115_CaloIdVT_GsfTrkIdT_v",
            "HLT_Photon175_v"
        )
    ),

    # This is used to access the results of all filters that ran.
    #
    filters = cms.PSet(
        AllFilters = cms.PSet(
            process = cms.string( 'PAT' ),
            results = cms.string( 'TriggerResults' ),
            paths = cms.vstring()
        )
    ),

    # "borrowed" from IB
    # for miniIsolation PF-weighted isolation
    # https://github.com/SUSYDileptonAC/SuSyAachen/blob/master/TagAndProbeTreeWriter/python/isolationFunctor_cfi.py
    # this version:
    # https://github.com/SUSYDileptonAC/SuSyAachen/commit/3fb6a2f71f234c437f3b5eccc27432ab0fb907b3
    isolationDefinitions = cms.PSet(
        rhoSource = cms.InputTag("fixedGridRhoFastjetCentralNeutral"),
        candSource = cms.InputTag("packedPFCandidates"),
        # this may be updated by updateJetMET in Tools_miniAOD.py to have up to date JEC
        jetCollection = cms.InputTag("slimmedJets"),
    ),

    cuts = cms.PSet(
        min_tau_pt  = cms.double( 27 ),
        min_muon_pt = cms.double( 10 ),
        min_ele_pt = cms.double( 10 ),
        min_gamma_pt = cms.double( 10 ),
        min_jet_pt = cms.double( 20 ),
        min_met = cms.double( 0 ),
        max_eta = cms.double( 3 ),
        vertex_minNDOF = cms.double( 3 ),
        vertex_maxZ = cms.double( 30 ),
        vertex_maxR = cms.double( 3 ),
        # These cuts come from:
        # https://twiki.cern.ch/twiki/bin/viewauth/CMSPublic/WorkBookChapter8?rev=27
        # See also:
        # CMS PAS TRK-10-005
        PV_minNDOF = cms.double( 4 ),
        PV_maxZ = cms.double( 24 ),
        PV_maxR = cms.double( 2 )
    )
)
