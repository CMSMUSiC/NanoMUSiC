from ROOT import RDataFrame, TFile, RDF
import hashlib
import os

MUSIC_TRIGGERS = [
    "HLT_IsoMu24",
    "HLT_IsoTkMu24",
    "HLT_IsoMu27",
    "HLT_Mu50",
    "HLT_TkMu50",
    "HLT_TkMu100",
    "HLT_OldMu100",
    "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ",
    "HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ",
    "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ",
    "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL",
    "HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL",
    "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL",
    "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8",
    "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8",
    "HLT_Ele27_WPTight_Gsf",
    "HLT_Ele35_WPTight_Gsf",
    "HLT_Ele32_WPTight_Gsf",
    "HLT_Photon175",
    "HLT_Ele115_CaloIdVT_GsfTrkIdT",
    "HLT_Photon200",
    "HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_MW",
    "HLT_DoubleEle33_CaloIdL_MW",
    "HLT_DoubleEle25_CaloIdL_MW",
    "HLT_VLooseIsoPFTau120_Trk50_eta2p1",
    "HLT_VLooseIsoPFTau140_Trk50_eta2p1",
    "HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1",
    "HLT_DoubleMediumIsoPFTau35_Trk1_eta2p1_Reg",
    "HLT_DoubleMediumCombinedIsoPFTau35_Trk1_eta2p1_Reg",
    "HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg",
    "HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg",
    "HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg",
    "HLT_DoubleMediumChargedIsoPFTauHPS35_Trk1_eta2p1_Reg",
    "HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90",
    "HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90",
]

MUSIC_BRANCHES = [
    "run",
    "luminosityBlock",
    "PV_npvsGood",
    "Flag_goodVertices",
    "Flag_globalSuperTightHalo2016Filter",
    "Flag_HBHENoiseFilter",
    "Flag_HBHENoiseIsoFilter",
    "Flag_EcalDeadCellTriggerPrimitiveFilter",
    "Flag_BadPFMuonFilter",
    "Flag_BadPFMuonDzFilter",
    "Flag_eeBadScFilter",
    "Flag_hfNoisyHitsFilter",
    "Flag_ecalBadCalibFilter",
    "HLT_IsoMu24",
    "HLT_IsoTkMu24",
    "HLT_IsoMu27",
    "HLT_Mu50",
    "HLT_TkMu50",
    "HLT_TkMu100",
    "HLT_OldMu100",
    "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ",
    "HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ",
    "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ",
    "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL",
    "HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL",
    "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL",
    "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8",
    "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8",
    "HLT_Ele27_WPTight_Gsf",
    "HLT_Ele35_WPTight_Gsf",
    "HLT_Ele32_WPTight_Gsf",
    "HLT_Photon175",
    "HLT_Ele115_CaloIdVT_GsfTrkIdT",
    "HLT_Photon200",
    "HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_MW",
    "HLT_DoubleEle33_CaloIdL_MW",
    "HLT_DoubleEle25_CaloIdL_MW",
    "HLT_VLooseIsoPFTau120_Trk50_eta2p1",
    "HLT_VLooseIsoPFTau140_Trk50_eta2p1",
    "HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1",
    "HLT_DoubleMediumIsoPFTau35_Trk1_eta2p1_Reg",
    "HLT_DoubleMediumCombinedIsoPFTau35_Trk1_eta2p1_Reg",
    "HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg",
    "HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg",
    "HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg",
    "HLT_DoubleMediumChargedIsoPFTauHPS35_Trk1_eta2p1_Reg",
    "HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90",
    "HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90",
    "genWeight",
    "LHEWeight_originalXWGTUP",
    "Pileup_nTrueInt",
    "L1PreFiringWeight_Up",
    "L1PreFiringWeight_Dn",
    "L1PreFiringWeight_Nom",
    "LHEPdfWeight",
    "Generator_scalePDF",
    "Generator_x1",
    "Generator_x2",
    "Generator_id1",
    "Generator_id2",
    "LHEScaleWeight",
    "LHEPart_pt",
    "LHEPart_eta",
    "LHEPart_phi",
    "LHEPart_mass",
    "LHEPart_incomingpz",
    "LHEPart_pdgId",
    "LHEPart_status",
    "GenPart_pt",
    "GenPart_eta",
    "GenPart_phi",
    "GenPart_mass",
    "GenPart_genPartIdxMother",
    "GenPart_pdgId",
    "GenPart_status",
    "GenPart_statusFlags",
    "Muon_pt",
    "Muon_eta",
    "Muon_phi",
    "Muon_tightId",
    "Muon_highPtId",
    "Muon_pfRelIso04_all",
    "Muon_tkRelIso",
    "Muon_tunepRelPt",
    "Muon_highPurity",
    "Muon_genPartIdx",
    "Electron_pt",
    "Electron_eta",
    "Electron_phi",
    "Electron_deltaEtaSC",
    "Electron_cutBased",
    "Electron_cutBased_HEEP",
    "Electron_scEtOverPt",
    "Electron_dEscaleUp",
    "Electron_dEscaleDown",
    "Electron_dEsigmaUp",
    "Electron_dEsigmaDown",
    "Electron_genPartIdx",
    "Tau_pt",
    "Tau_eta",
    "Tau_phi",
    "Tau_dz",
    "Tau_mass",
    "Tau_idDeepTau2017v2p1VSe",
    "Tau_idDeepTau2017v2p1VSjet",
    "Tau_idDeepTau2017v2p1VSmu",
    "Tau_decayMode",
    "Tau_genPartIdx",
    "Tau_genPartFlav",
    "Photon_pt",
    "Photon_eta",
    "Photon_phi",
    "Photon_isScEtaEB",
    "Photon_isScEtaEE",
    "Photon_cutBased",
    "Photon_pixelSeed",
    "Photon_mvaID_WP90",
    "Photon_electronVeto",
    "Photon_dEscaleUp",
    "Photon_dEscaleDown",
    "Photon_dEsigmaUp",
    "Photon_dEsigmaDown",
    "Photon_genPartIdx",
    "fixedGridRhoFastjetAll",
    "GenJet_pt",
    "GenJet_eta",
    "GenJet_phi",
    "Jet_pt",
    "Jet_eta",
    "Jet_phi",
    "Jet_mass",
    "Jet_jetId",
    "Jet_btagDeepFlavB",
    "Jet_rawFactor",
    "Jet_area",
    "Jet_genJetIdx",
    "RawMET_pt",
    "RawMET_phi",
    "MET_MetUnclustEnUpDeltaX",
    "MET_MetUnclustEnUpDeltaY",
]


def get_trigger_and_branches_lists(input_file: str):
    f = TFile.Open(input_file)
    branches = [b.GetName() for b in f.Events.GetListOfBranches()]
    if f.Events.GetEntries() == 0:
        return None, None

    f.Close()
    available_triggers = []
    available_branches = []

    for trigger in MUSIC_TRIGGERS:
        if trigger in branches:
            available_triggers.append(trigger)
            available_branches.append(trigger)

    for branch in MUSIC_BRANCHES:
        if branch in branches:
            if not branch.startswith("HLT_"):
                available_branches.append(branch)

    return (" or ".join(available_triggers), available_branches)


def skim_imp(input_file: str, skimmed_file_path: str) -> None:
    triggers, branches = get_trigger_and_branches_lists(input_file)

    opts = RDF.RSnapshotOptions()
    # opts.fCompressionLevel = 207
    # opts.fCompressionLevel = 100
    opts.fCompressionLevel = 102

    df = RDataFrame("Events", input_file)
    if triggers and branches:
        trigger_filtered = df.Filter(triggers)
        trigger_filtered.Snapshot("Events", skimmed_file_path, branches, opts)
    else:
        df.Snapshot("Events", skimmed_file_path, opts)


def skim(
    input_file: str,
    process: str,
    year: str,
    is_dev_job: bool,
    skimmed_files_dir: str = "skimmed_files",
) -> str:
    if not is_dev_job:
        skimmed_files_dir = "../" + skimmed_files_dir

    if not os.path.exists(skimmed_files_dir):
        os.system(f"mkdir -p { skimmed_files_dir }")

    hash_object = hashlib.sha256(str.encode(input_file))
    skimmed_file_path = "{}/{}_{}_{}.root".format(
        skimmed_files_dir, process, year, hash_object.hexdigest()
    )

    if not os.path.exists(skimmed_file_path):
        skim_imp(input_file, skimmed_file_path)

    print("Skimmed: {}".format(input_file))

    return skimmed_file_path
