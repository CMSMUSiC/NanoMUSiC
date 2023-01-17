#include "TLorentzVector.h"

// Gen Related Info
void analyzeGenRelatedInfo(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto LHEWeight_originalXWGTUP = nano_reader.getVal<Float_t>("LHEWeight_originalXWGTUP");
    GenEvtView->setUserRecord("LHEWeight_originalXWGTUP", LHEWeight_originalXWGTUP);

    auto Generator_binvar = nano_reader.getVal<Float_t>("Generator_binvar");
    auto Generator_scalePDF = nano_reader.getVal<Float_t>("Generator_scalePDF");
    auto Generator_weight = nano_reader.getVal<Float_t>("Generator_weight");
    auto Generator_x1 = nano_reader.getVal<Float_t>("Generator_x1");
    auto Generator_x2 = nano_reader.getVal<Float_t>("Generator_x2");
    auto Generator_xpdf1 = nano_reader.getVal<Float_t>("Generator_xpdf1");
    auto Generator_xpdf2 = nano_reader.getVal<Float_t>("Generator_xpdf2");
    auto Generator_id1 = nano_reader.getVal<Int_t>("Generator_id1");
    auto Generator_id2 = nano_reader.getVal<Int_t>("Generator_id2");

    GenEvtView->setUserRecord("Generator_binvar", Generator_binvar);
    GenEvtView->setUserRecord("Generator_scalePDF", Generator_scalePDF);
    GenEvtView->setUserRecord("Generator_weight", Generator_weight);
    GenEvtView->setUserRecord("Generator_x1", Generator_x1);
    GenEvtView->setUserRecord("Generator_x2", Generator_x2);
    GenEvtView->setUserRecord("Generator_xpdf1", Generator_xpdf1);
    GenEvtView->setUserRecord("Generator_xpdf2", Generator_xpdf2);
    GenEvtView->setUserRecord("Generator_id1", Generator_id1);
    GenEvtView->setUserRecord("Generator_id2", Generator_id2);

    GenEvtView->setUserRecord("nLHEScaleWeight", nano_reader.getVal<UInt_t>("nLHEScaleWeight"));
    auto LHEScaleWeight = nano_reader.getVec<Float_t>("LHEScaleWeight");
    for (unsigned int idx = 0; idx < nano_reader.getVal<UInt_t>("nLHEScaleWeight"); idx++)
    {
        GenEvtView->setUserRecord("LHEScaleWeight_" + std::to_string(idx), LHEScaleWeight[idx]);
    }

    GenEvtView->setUserRecord("nPSWeight", nano_reader.getVal<UInt_t>("nPSWeight"));
    auto PSWeight = nano_reader.getVec<Float_t>("PSWeight");
    for (unsigned int idx = 0; idx < nano_reader.getVal<UInt_t>("nPSWeight"); idx++)
    {
        GenEvtView->setUserRecord("PSWeight_" + std::to_string(idx), PSWeight[idx]);
    }

    GenEvtView->setUserRecord("nLHEPdfWeight", nano_reader.getVal<UInt_t>("nLHEPdfWeight"));
    auto LHEPdfWeight = nano_reader.getVec<Float_t>("LHEPdfWeight");
    for (unsigned int idx = 0; idx < nano_reader.getVal<UInt_t>("nLHEPdfWeight"); idx++)
    {
        GenEvtView->setUserRecord("LHEPdfWeight_" + std::to_string(idx), LHEPdfWeight[idx]);
    }

    GenEvtView->setUserRecord("nLHEReweightingWeight", nano_reader.getVal<UInt_t>("nLHEReweightingWeight"));
    auto LHEReweightingWeight = nano_reader.getVec<Float_t>("LHEReweightingWeight");
    for (unsigned int idx = 0; idx < nano_reader.getVal<UInt_t>("nLHEReweightingWeight"); idx++)
    {
        GenEvtView->setUserRecord("LHEReweightingWeight_" + std::to_string(idx), LHEReweightingWeight[idx]);
    }
}

// gen Weight
void analyzegenWeight(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto genWeight = nano_reader.getVal<Float_t>("genWeight");
    GenEvtView->setUserRecord("genWeight", genWeight);
}

// LHE  info
void analyzeLHEInfo(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto LHE_HT = nano_reader.getVal<Float_t>("LHE_HT");
    auto LHE_HTIncoming = nano_reader.getVal<Float_t>("LHE_HTIncoming");
    auto LHE_Vpt = nano_reader.getVal<Float_t>("LHE_Vpt");
    auto LHE_AlphaS = nano_reader.getVal<Float_t>("LHE_AlphaS");
    auto LHE_Njets = nano_reader.getVal<UChar_t>("LHE_Njets");
    auto LHE_Nb = nano_reader.getVal<UChar_t>("LHE_Nb");
    auto LHE_Nc = nano_reader.getVal<UChar_t>("LHE_Nc");
    auto LHE_Nuds = nano_reader.getVal<UChar_t>("LHE_Nuds");
    auto LHE_Nglu = nano_reader.getVal<UChar_t>("LHE_Nglu");
    auto LHE_NpNLO = nano_reader.getVal<UChar_t>("LHE_NpNLO");
    auto LHE_NpLO = nano_reader.getVal<UChar_t>("LHE_NpLO");

    // set all available records
    GenEvtView->setUserRecord("LHE_HT", LHE_HT);
    GenEvtView->setUserRecord("LHE_HTIncoming", LHE_HTIncoming);
    GenEvtView->setUserRecord("LHE_Vpt", LHE_Vpt);
    GenEvtView->setUserRecord("LHE_AlphaS", LHE_AlphaS);
    GenEvtView->setUserRecord("LHE_Njets", LHE_Njets);
    GenEvtView->setUserRecord("LHE_Nb", LHE_Nb);
    GenEvtView->setUserRecord("LHE_Nc", LHE_Nc);
    GenEvtView->setUserRecord("LHE_Nuds", LHE_Nuds);
    GenEvtView->setUserRecord("LHE_Nglu", LHE_Nglu);
    GenEvtView->setUserRecord("LHE_NpNLO", LHE_NpNLO);
    GenEvtView->setUserRecord("LHE_NpLO", LHE_NpLO);
}

// LHE particles
void analyzeLHEParticles(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{

    auto LHEPart_pt = nano_reader.getVec<Float_t>("LHEPart_pt");
    auto LHEPart_eta = nano_reader.getVec<Float_t>("LHEPart_eta");
    auto LHEPart_phi = nano_reader.getVec<Float_t>("LHEPart_phi");
    auto LHEPart_mass = nano_reader.getVec<Float_t>("LHEPart_mass");
    auto LHEPart_incomingpz = nano_reader.getVec<Float_t>("LHEPart_incomingpz");
    auto LHEPart_pdgId = nano_reader.getVec<Int_t>("LHEPart_pdgId");
    auto LHEPart_status = nano_reader.getVec<Int_t>("LHEPart_status");
    auto LHEPart_spin = nano_reader.getVec<Int_t>("LHEPart_spin");

    auto part_size = nano_reader.getVal<UInt_t>("nLHEPart");
    GenEvtView->setUserRecord("NumLHEPart", part_size);

    // loop over gen jets AK8
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = GenEvtView->create<pxl::Particle>();
        part->setName("GenPart");
        part->setCharge((LHEPart_pdgId[idx_part] > 0) - (LHEPart_pdgId[idx_part] < 0));
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(LHEPart_pt[idx_part], LHEPart_eta[idx_part], LHEPart_phi[idx_part],
                             LHEPart_mass[idx_part]);

        // set all available records
        part->setUserRecord("pt", LHEPart_pt[idx_part]);
        part->setUserRecord("eta", LHEPart_eta[idx_part]);
        part->setUserRecord("phi", LHEPart_phi[idx_part]);
        part->setUserRecord("mass", LHEPart_mass[idx_part]);
        part->setUserRecord("incomingpz", LHEPart_incomingpz[idx_part]);
        part->setUserRecord("pdgId", LHEPart_pdgId[idx_part]);
        part->setUserRecord("status", LHEPart_status[idx_part]);
        part->setUserRecord("spin", LHEPart_spin[idx_part]);
    }
}

// gen vertices
void analyzeGenVertices(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto GenVtx_x = nano_reader.getVal<Float_t>("GenVtx_x");
    auto GenVtx_y = nano_reader.getVal<Float_t>("GenVtx_y");
    auto GenVtx_z = nano_reader.getVal<Float_t>("GenVtx_z");
    auto GenVtx_t0 = nano_reader.getVal<Float_t>("GenVtx_t0");

    GenEvtView->setUserRecord("GenVtx_x", GenVtx_x);
    GenEvtView->setUserRecord("GenVtx_y", GenVtx_y);
    GenEvtView->setUserRecord("GenVtx_z", GenVtx_z);
    GenEvtView->setUserRecord("GenVtx_t0", GenVtx_t0);
}

// gen particles
void analyzeGenParticles(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto GenPart_eta = nano_reader.getVec<Float_t>("GenPart_eta");
    auto GenPart_mass = nano_reader.getVec<Float_t>("GenPart_mass");
    auto GenPart_phi = nano_reader.getVec<Float_t>("GenPart_phi");
    auto GenPart_pt = nano_reader.getVec<Float_t>("GenPart_pt");
    auto GenPart_genPartIdxMother = nano_reader.getVec<Int_t>("GenPart_genPartIdxMother");
    auto GenPart_pdgId = nano_reader.getVec<Int_t>("GenPart_pdgId");
    auto GenPart_status = nano_reader.getVec<Int_t>("GenPart_status");
    auto GenPart_statusFlags = nano_reader.getVec<Int_t>("GenPart_statusFlags");

    auto part_size = nano_reader.getVal<UInt_t>("nGenPart");
    GenEvtView->setUserRecord("NumGenPart", part_size);

    // loop over gen jets AK8
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = GenEvtView->create<pxl::Particle>();
        part->setName("GenPart");
        part->setCharge((GenPart_pdgId[idx_part] > 0) - (GenPart_pdgId[idx_part] < 0));
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(GenPart_pt[idx_part], GenPart_eta[idx_part], GenPart_phi[idx_part],
                             GenPart_mass[idx_part]);

        // set all available records
        part->setUserRecord("eta", GenPart_eta[idx_part]);
        part->setUserRecord("mass", GenPart_mass[idx_part]);
        part->setUserRecord("phi", GenPart_phi[idx_part]);
        part->setUserRecord("pt", GenPart_pt[idx_part]);
        part->setUserRecord("genPartIdxMother", GenPart_genPartIdxMother[idx_part]);
        part->setUserRecord("pdgId", GenPart_pdgId[idx_part]);
        part->setUserRecord("status", GenPart_status[idx_part]);
        part->setUserRecord("statusFlags", GenPart_statusFlags[idx_part]);
    }

    GenEvtView->setUserRecord("NumVertices", 1);
}

// gen dressed leptons
void analyzeGenDressedLeptons(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto GenDressedLepton_eta = nano_reader.getVec<Float_t>("GenDressedLepton_eta");
    auto GenDressedLepton_mass = nano_reader.getVec<Float_t>("GenDressedLepton_mass");
    auto GenDressedLepton_phi = nano_reader.getVec<Float_t>("GenDressedLepton_phi");
    auto GenDressedLepton_pt = nano_reader.getVec<Float_t>("GenDressedLepton_pt");
    auto GenDressedLepton_pdgId = nano_reader.getVec<Int_t>("GenDressedLepton_pdgId");
    auto GenDressedLepton_hasTauAnc = nano_reader.getVec<Bool_t, unsigned int>("GenDressedLepton_hasTauAnc");

    auto part_size = nano_reader.getVal<UInt_t>("nGenDressedLepton");
    GenEvtView->setUserRecord("NumGenDressedLepton", part_size);

    // loop over gen jets AK8
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = GenEvtView->create<pxl::Particle>();
        part->setName("GenPart");
        part->setCharge((GenDressedLepton_pdgId[idx_part] > 0) - (GenDressedLepton_pdgId[idx_part] < 0));
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(GenDressedLepton_pt[idx_part], GenDressedLepton_eta[idx_part],
                             GenDressedLepton_phi[idx_part], GenDressedLepton_mass[idx_part]);

        // set all available records
        part->setUserRecord("eta", GenDressedLepton_eta[idx_part]);
        part->setUserRecord("mass", GenDressedLepton_mass[idx_part]);
        part->setUserRecord("phi", GenDressedLepton_phi[idx_part]);
        part->setUserRecord("pt", GenDressedLepton_pt[idx_part]);
        part->setUserRecord("pdgId", GenDressedLepton_pdgId[idx_part]);
        part->setUserRecord("hasTauAnc", GenDressedLepton_hasTauAnc[idx_part]);
    }
}

// PU Info
void analyzeGenPU(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto Pileup_nTrueInt = nano_reader.getVal<Float_t>("Pileup_nTrueInt");
    auto Pileup_pudensity = nano_reader.getVal<Float_t>("Pileup_pudensity");
    auto Pileup_gpudensity = nano_reader.getVal<Float_t>("Pileup_gpudensity");
    auto Pileup_nPU = nano_reader.getVal<Int_t>("Pileup_nPU");
    auto Pileup_sumEOOT = nano_reader.getVal<Int_t>("Pileup_sumEOOT");
    auto Pileup_sumLOOT = nano_reader.getVal<Int_t>("Pileup_sumLOOT");

    GenEvtView->setUserRecord("Pileup_nTrueInt", Pileup_nTrueInt);
    GenEvtView->setUserRecord("Pileup_pudensity", Pileup_pudensity);
    GenEvtView->setUserRecord("Pileup_gpudensity", Pileup_gpudensity);
    GenEvtView->setUserRecord("Pileup_nPU", Pileup_nPU);
    GenEvtView->setUserRecord("Pileup_sumEOOT", Pileup_sumEOOT);
    GenEvtView->setUserRecord("Pileup_sumLOOT", Pileup_sumLOOT);
}

// gen jets
void analyzeGenJets(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto GenJet_eta = nano_reader.getVec<Float_t>("GenJet_eta");
    auto GenJet_mass = nano_reader.getVec<Float_t>("GenJet_mass");
    auto GenJet_phi = nano_reader.getVec<Float_t>("GenJet_phi");
    auto GenJet_pt = nano_reader.getVec<Float_t>("GenJet_pt");
    auto GenJet_partonFlavour = nano_reader.getVec<Int_t>("GenJet_partonFlavour");
    auto GenJet_hadronFlavour = nano_reader.getVec<UChar_t>("GenJet_hadronFlavour");

    auto part_size = nano_reader.getVal<UInt_t>("nGenJet");
    GenEvtView->setUserRecord("NumGenJet", part_size);

    // loop over gen jets
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = GenEvtView->create<pxl::Particle>();
        part->setName("GenJet");
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(GenJet_pt[idx_part], GenJet_eta[idx_part], GenJet_phi[idx_part], GenJet_mass[idx_part]);

        // set all available records
        part->setUserRecord("eta", GenJet_eta[idx_part]);
        part->setUserRecord("mass", GenJet_mass[idx_part]);
        part->setUserRecord("phi", GenJet_phi[idx_part]);
        part->setUserRecord("pt", GenJet_pt[idx_part]);
        part->setUserRecord("partonFlavour", GenJet_partonFlavour[idx_part]);
        part->setUserRecord("hadronFlavour", GenJet_hadronFlavour[idx_part]);
    }
}

// gen jets AK8
void analyzeGenJetsAK8(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto GenJetAK8_eta = nano_reader.getVec<Float_t>("GenJetAK8_eta");
    auto GenJetAK8_mass = nano_reader.getVec<Float_t>("GenJetAK8_mass");
    auto GenJetAK8_phi = nano_reader.getVec<Float_t>("GenJetAK8_phi");
    auto GenJetAK8_pt = nano_reader.getVec<Float_t>("GenJetAK8_pt");
    auto GenJetAK8_partonFlavour = nano_reader.getVec<Int_t>("GenJetAK8_partonFlavour");
    auto GenJetAK8_hadronFlavour = nano_reader.getVec<UChar_t>("GenJetAK8_hadronFlavour");

    auto part_size = nano_reader.getVal<UInt_t>("nGenJetAK8");
    GenEvtView->setUserRecord("NumGenJet", part_size);

    // loop over gen jets AK8
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = GenEvtView->create<pxl::Particle>();
        part->setName("GenJetAK8");
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(GenJetAK8_pt[idx_part], GenJetAK8_eta[idx_part], GenJetAK8_phi[idx_part],
                             GenJetAK8_mass[idx_part]);

        // set all available records
        part->setUserRecord("eta", GenJetAK8_eta[idx_part]);
        part->setUserRecord("mass", GenJetAK8_mass[idx_part]);
        part->setUserRecord("phi", GenJetAK8_phi[idx_part]);
        part->setUserRecord("pt", GenJetAK8_pt[idx_part]);
        part->setUserRecord("partonFlavour", GenJetAK8_partonFlavour[idx_part]);
        part->setUserRecord("hadronFlavour", GenJetAK8_hadronFlavour[idx_part]);
    }
}

// gen MET
void analyzeGenMET(NanoAODReader &nano_reader, pxl::EventView *GenEvtView)
{
    auto GenMET_phi = nano_reader.getVal<Float_t>("GenMET_phi");
    auto GenMET_pt = nano_reader.getVal<Float_t>("GenMET_pt");

    pxl::Particle *part = GenEvtView->create<pxl::Particle>();
    part->setName("GenMET");
    auto p_temp_ = TLorentzVector();
    p_temp_.SetPtEtaPhiM(GenMET_pt, 0, GenMET_phi, 0);

    // set all available records
    part->setUserRecord("phi", GenMET_phi);
    part->setUserRecord("pt", GenMET_pt);
}

// Trigger results
void analyzeTrigger(NanoAODReader &nano_reader, pxl::EventView *TrigEvtView)
{
    for (auto &trigger_name : nano_reader.getListOfBranches())
    {
        if (trigger_name.rfind("HLT_", 0) == 0)
        {
            if (nano_reader.getVal<Bool_t>(trigger_name))
            {
                TrigEvtView->setUserRecord(trigger_name, 1);
            }
        }
    }
}

// MET Filters
void analyseMETFilter(NanoAODReader &nano_reader, pxl::EventView *FilterEvtView)
{
    for (auto &filter_name : nano_reader.getListOfBranches())
    {
        if (filter_name.rfind("Flag_", 0) == 0)
        {
            FilterEvtView->setUserRecord(filter_name, nano_reader.getVal<Bool_t>(filter_name));
        }
    }
}

// rho
void analyzeRho(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    RecEvtView->setUserRecord("fixedGridRhoFastjetAll", nano_reader.getVal<Float_t>("fixedGridRhoFastjetAll"));
    // RecEvtView->setUserRecord("fixedGridRhoFastjetAllCalo",
    // nano_reader.getVal<Float_t>("fixedGridRhoFastjetAllCalo"));
    RecEvtView->setUserRecord("fixedGridRhoFastjetCentralCalo",
                              nano_reader.getVal<Float_t>("fixedGridRhoFastjetCentralCalo"));
    RecEvtView->setUserRecord("fixedGridRhoFastjetCentralChargedPileUp",
                              nano_reader.getVal<Float_t>("fixedGridRhoFastjetCentralChargedPileUp"));
    RecEvtView->setUserRecord("fixedGridRhoFastjetCentralNeutral",
                              nano_reader.getVal<Float_t>("fixedGridRhoFastjetCentralNeutral"));
    RecEvtView->setUserRecord("fixedGridRhoFastjetAll", nano_reader.getVal<Float_t>("fixedGridRhoFastjetAll"));
}

// primary vertices
void analyzeRecVertices(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    auto PV_ndof = nano_reader.getVal<Float_t>("PV_ndof");
    auto PV_x = nano_reader.getVal<Float_t>("PV_x");
    auto PV_y = nano_reader.getVal<Float_t>("PV_y");
    auto PV_z = nano_reader.getVal<Float_t>("PV_z");
    auto PV_chi2 = nano_reader.getVal<Float_t>("PV_chi2");
    auto PV_score = nano_reader.getVal<Float_t>("PV_score");
    auto PV_npvs = nano_reader.getVal<Int_t>("PV_npvs");
    auto PV_npvsGood = nano_reader.getVal<Int_t>("PV_npvsGood");

    pxl::Vertex *vtx = RecEvtView->create<pxl::Vertex>();
    vtx->setName("PV");
    vtx->setXYZ(PV_x, PV_y, PV_z);

    // set all available records
    vtx->setUserRecord("ndof", PV_ndof);
    vtx->setUserRecord("x", PV_x);
    vtx->setUserRecord("y", PV_y);
    vtx->setUserRecord("z", PV_z);
    vtx->setUserRecord("chi2", PV_chi2);
    vtx->setUserRecord("score", PV_score);
    vtx->setUserRecord("npvs", PV_npvs);
    vtx->setUserRecord("npvsGood", PV_npvsGood);

    RecEvtView->setUserRecord("NumVertices", 1 + nano_reader.getVal<UInt_t>("nOtherPV"));
}

// taus
void analyzeRecTaus(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    auto Tau_chargedIso = nano_reader.getVec<Float_t>("Tau_chargedIso");
    auto Tau_dxy = nano_reader.getVec<Float_t>("Tau_dxy");
    auto Tau_dz = nano_reader.getVec<Float_t>("Tau_dz");
    auto Tau_eta = nano_reader.getVec<Float_t>("Tau_eta");
    auto Tau_leadTkDeltaEta = nano_reader.getVec<Float_t>("Tau_leadTkDeltaEta");
    auto Tau_leadTkDeltaPhi = nano_reader.getVec<Float_t>("Tau_leadTkDeltaPhi");
    auto Tau_leadTkPtOverTauPt = nano_reader.getVec<Float_t>("Tau_leadTkPtOverTauPt");
    auto Tau_mass = nano_reader.getVec<Float_t>("Tau_mass");
    auto Tau_neutralIso = nano_reader.getVec<Float_t>("Tau_neutralIso");
    auto Tau_phi = nano_reader.getVec<Float_t>("Tau_phi");
    auto Tau_photonsOutsideSignalCone = nano_reader.getVec<Float_t>("Tau_photonsOutsideSignalCone");
    auto Tau_pt = nano_reader.getVec<Float_t>("Tau_pt");
    auto Tau_puCorr = nano_reader.getVec<Float_t>("Tau_puCorr");
    auto Tau_rawDeepTau2017v2p1VSe = nano_reader.getVec<Float_t>("Tau_rawDeepTau2017v2p1VSe");
    auto Tau_rawDeepTau2017v2p1VSjet = nano_reader.getVec<Float_t>("Tau_rawDeepTau2017v2p1VSjet");
    auto Tau_rawDeepTau2017v2p1VSmu = nano_reader.getVec<Float_t>("Tau_rawDeepTau2017v2p1VSmu");
    auto Tau_rawIso = nano_reader.getVec<Float_t>("Tau_rawIso");
    auto Tau_rawIsodR03 = nano_reader.getVec<Float_t>("Tau_rawIsodR03");
    auto Tau_charge = nano_reader.getVec<Int_t>("Tau_charge");
    auto Tau_decayMode = nano_reader.getVec<Int_t>("Tau_decayMode");
    auto Tau_jetIdx = nano_reader.getVec<Int_t>("Tau_jetIdx");
    auto Tau_idAntiEleDeadECal = nano_reader.getVec<Bool_t, unsigned int>("Tau_idAntiEleDeadECal");
    auto Tau_idAntiMu = nano_reader.getVec<UChar_t>("Tau_idAntiMu");
    auto Tau_idDecayModeOldDMs = nano_reader.getVec<Bool_t, unsigned int>("Tau_idDecayModeOldDMs");
    auto Tau_idDeepTau2017v2p1VSe = nano_reader.getVec<UChar_t>("Tau_idDeepTau2017v2p1VSe");
    auto Tau_idDeepTau2017v2p1VSjet = nano_reader.getVec<UChar_t>("Tau_idDeepTau2017v2p1VSjet");
    auto Tau_idDeepTau2017v2p1VSmu = nano_reader.getVec<UChar_t>("Tau_idDeepTau2017v2p1VSmu");
    auto Tau_cleanmask = nano_reader.getVec<UChar_t>("Tau_cleanmask");
    auto Tau_genPartIdx = nano_reader.getVec<Int_t>("Tau_genPartIdx");
    auto Tau_genPartFlav = nano_reader.getVec<UChar_t>("Tau_genPartFlav");

    auto part_size = nano_reader.getVal<UInt_t>("nTau");
    RecEvtView->setUserRecord("NumTau", part_size);

    // loop over taus
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = RecEvtView->create<pxl::Particle>();
        part->setName("Tau");
        part->setCharge(Tau_charge[idx_part]);
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(Tau_pt[idx_part], Tau_eta[idx_part], Tau_phi[idx_part], Tau_mass[idx_part]);

        // set all available records
        part->setUserRecord("chargedIso", Tau_chargedIso[idx_part]);
        part->setUserRecord("dxy", Tau_dxy[idx_part]);
        part->setUserRecord("dz", Tau_dz[idx_part]);
        part->setUserRecord("eta", Tau_eta[idx_part]);
        part->setUserRecord("leadTkDeltaEta", Tau_leadTkDeltaEta[idx_part]);
        part->setUserRecord("leadTkDeltaPhi", Tau_leadTkDeltaPhi[idx_part]);
        part->setUserRecord("leadTkPtOverTauPt", Tau_leadTkPtOverTauPt[idx_part]);
        part->setUserRecord("mass", Tau_mass[idx_part]);
        part->setUserRecord("neutralIso", Tau_neutralIso[idx_part]);
        part->setUserRecord("phi", Tau_phi[idx_part]);
        part->setUserRecord("photonsOutsideSignalCone", Tau_photonsOutsideSignalCone[idx_part]);
        part->setUserRecord("pt", Tau_pt[idx_part]);
        part->setUserRecord("puCorr", Tau_puCorr[idx_part]);
        part->setUserRecord("rawDeepTau2017v2p1VSe", Tau_rawDeepTau2017v2p1VSe[idx_part]);
        part->setUserRecord("rawDeepTau2017v2p1VSjet", Tau_rawDeepTau2017v2p1VSjet[idx_part]);
        part->setUserRecord("rawDeepTau2017v2p1VSmu", Tau_rawDeepTau2017v2p1VSmu[idx_part]);
        part->setUserRecord("rawIso", Tau_rawIso[idx_part]);
        part->setUserRecord("rawIsodR03", Tau_rawIsodR03[idx_part]);
        part->setUserRecord("charge", Tau_charge[idx_part]);
        part->setUserRecord("decayMode", Tau_decayMode[idx_part]);
        part->setUserRecord("jetIdx", Tau_jetIdx[idx_part]);
        part->setUserRecord("idAntiEleDeadECal", Tau_idAntiEleDeadECal[idx_part]);
        part->setUserRecord("idAntiMu", Tau_idAntiMu[idx_part]);
        part->setUserRecord("idDecayModeOldDMs", Tau_idDecayModeOldDMs[idx_part]);
        part->setUserRecord("idDeepTau2017v2p1VSe", Tau_idDeepTau2017v2p1VSe[idx_part]);
        part->setUserRecord("idDeepTau2017v2p1VSjet", Tau_idDeepTau2017v2p1VSjet[idx_part]);
        part->setUserRecord("idDeepTau2017v2p1VSmu", Tau_idDeepTau2017v2p1VSmu[idx_part]);
        part->setUserRecord("cleanmask", Tau_cleanmask[idx_part]);
        part->setUserRecord("genPartIdx", Tau_genPartIdx[idx_part]);
        part->setUserRecord("genPartFlav", Tau_genPartFlav[idx_part]);
    }
}

// boosted taus
void analyzeRecBoostedTaus(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    auto boostedTau_chargedIso = nano_reader.getVec<Float_t>("boostedTau_chargedIso");
    auto boostedTau_eta = nano_reader.getVec<Float_t>("boostedTau_eta");
    auto boostedTau_leadTkDeltaEta = nano_reader.getVec<Float_t>("boostedTau_leadTkDeltaEta");
    auto boostedTau_leadTkDeltaPhi = nano_reader.getVec<Float_t>("boostedTau_leadTkDeltaPhi");
    auto boostedTau_leadTkPtOverTauPt = nano_reader.getVec<Float_t>("boostedTau_leadTkPtOverTauPt");
    auto boostedTau_mass = nano_reader.getVec<Float_t>("boostedTau_mass");
    auto boostedTau_neutralIso = nano_reader.getVec<Float_t>("boostedTau_neutralIso");
    auto boostedTau_phi = nano_reader.getVec<Float_t>("boostedTau_phi");
    auto boostedTau_photonsOutsideSignalCone = nano_reader.getVec<Float_t>("boostedTau_photonsOutsideSignalCone");
    auto boostedTau_pt = nano_reader.getVec<Float_t>("boostedTau_pt");
    auto boostedTau_puCorr = nano_reader.getVec<Float_t>("boostedTau_puCorr");
    auto boostedTau_rawAntiEle2018 = nano_reader.getVec<Float_t>("boostedTau_rawAntiEle2018");
    auto boostedTau_rawIso = nano_reader.getVec<Float_t>("boostedTau_rawIso");
    auto boostedTau_rawIsodR03 = nano_reader.getVec<Float_t>("boostedTau_rawIsodR03");
    auto boostedTau_rawMVAnewDM2017v2 = nano_reader.getVec<Float_t>("boostedTau_rawMVAnewDM2017v2");
    auto boostedTau_rawMVAoldDM2017v2 = nano_reader.getVec<Float_t>("boostedTau_rawMVAoldDM2017v2");
    auto boostedTau_rawMVAoldDMdR032017v2 = nano_reader.getVec<Float_t>("boostedTau_rawMVAoldDMdR032017v2");
    auto boostedTau_charge = nano_reader.getVec<Int_t>("boostedTau_charge");
    auto boostedTau_decayMode = nano_reader.getVec<Int_t>("boostedTau_decayMode");
    auto boostedTau_jetIdx = nano_reader.getVec<Int_t>("boostedTau_jetIdx");
    auto boostedTau_rawAntiEleCat2018 = nano_reader.getVec<Int_t>("boostedTau_rawAntiEleCat2018");
    auto boostedTau_idAntiEle2018 = nano_reader.getVec<UChar_t>("boostedTau_idAntiEle2018");
    auto boostedTau_idAntiMu = nano_reader.getVec<UChar_t>("boostedTau_idAntiMu");
    auto boostedTau_idMVAnewDM2017v2 = nano_reader.getVec<UChar_t>("boostedTau_idMVAnewDM2017v2");
    auto boostedTau_idMVAoldDM2017v2 = nano_reader.getVec<UChar_t>("boostedTau_idMVAoldDM2017v2");
    auto boostedTau_idMVAoldDMdR032017v2 = nano_reader.getVec<UChar_t>("boostedTau_idMVAoldDMdR032017v2");
    auto boostedTau_genPartIdx = nano_reader.getVec<Int_t>("boostedTau_genPartIdx");
    auto boostedTau_genPartFlav = nano_reader.getVec<UChar_t>("boostedTau_genPartFlav");

    auto part_size = nano_reader.getVal<UInt_t>("nboostedTau");
    RecEvtView->setUserRecord("NumBoostedTau", part_size);

    // loop over boosted taus
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = RecEvtView->create<pxl::Particle>();
        part->setName("BoostedTau");
        part->setCharge(boostedTau_charge[idx_part]);
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(boostedTau_pt[idx_part], boostedTau_eta[idx_part], boostedTau_phi[idx_part],
                             boostedTau_mass[idx_part]);

        // set all available records
        part->setUserRecord("chargedIso", boostedTau_chargedIso[idx_part]);
        part->setUserRecord("eta", boostedTau_eta[idx_part]);
        part->setUserRecord("leadTkDeltaEta", boostedTau_leadTkDeltaEta[idx_part]);
        part->setUserRecord("leadTkDeltaPhi", boostedTau_leadTkDeltaPhi[idx_part]);
        part->setUserRecord("leadTkPtOverTauPt", boostedTau_leadTkPtOverTauPt[idx_part]);
        part->setUserRecord("mass", boostedTau_mass[idx_part]);
        part->setUserRecord("neutralIso", boostedTau_neutralIso[idx_part]);
        part->setUserRecord("phi", boostedTau_phi[idx_part]);
        part->setUserRecord("photonsOutsideSignalCone", boostedTau_photonsOutsideSignalCone[idx_part]);
        part->setUserRecord("pt", boostedTau_pt[idx_part]);
        part->setUserRecord("puCorr", boostedTau_puCorr[idx_part]);
        part->setUserRecord("rawAntiEle2018", boostedTau_rawAntiEle2018[idx_part]);
        part->setUserRecord("rawIso", boostedTau_rawIso[idx_part]);
        part->setUserRecord("rawIsodR03", boostedTau_rawIsodR03[idx_part]);
        part->setUserRecord("rawMVAnewDM2017v2", boostedTau_rawMVAnewDM2017v2[idx_part]);
        part->setUserRecord("rawMVAoldDM2017v2", boostedTau_rawMVAoldDM2017v2[idx_part]);
        part->setUserRecord("rawMVAoldDMdR032017v2", boostedTau_rawMVAoldDMdR032017v2[idx_part]);
        part->setUserRecord("charge", boostedTau_charge[idx_part]);
        part->setUserRecord("decayMode", boostedTau_decayMode[idx_part]);
        part->setUserRecord("jetIdx", boostedTau_jetIdx[idx_part]);
        part->setUserRecord("rawAntiEleCat2018", boostedTau_rawAntiEleCat2018[idx_part]);
        part->setUserRecord("idAntiEle2018", boostedTau_idAntiEle2018[idx_part]);
        part->setUserRecord("idAntiMu", boostedTau_idAntiMu[idx_part]);
        part->setUserRecord("idMVAnewDM2017v2", boostedTau_idMVAnewDM2017v2[idx_part]);
        part->setUserRecord("idMVAoldDM2017v2", boostedTau_idMVAoldDM2017v2[idx_part]);
        part->setUserRecord("idMVAoldDMdR032017v2", boostedTau_idMVAoldDMdR032017v2[idx_part]);
        part->setUserRecord("genPartIdx", boostedTau_genPartIdx[idx_part]);
        part->setUserRecord("genPartFlav", boostedTau_genPartFlav[idx_part]);
    }
}

// muons
void analyzeRecMuons(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    auto Muon_dxy = nano_reader.getVec<Float_t>("Muon_dxy");
    auto Muon_dxyErr = nano_reader.getVec<Float_t>("Muon_dxyErr");
    auto Muon_dxybs = nano_reader.getVec<Float_t>("Muon_dxybs");
    auto Muon_dz = nano_reader.getVec<Float_t>("Muon_dz");
    auto Muon_dzErr = nano_reader.getVec<Float_t>("Muon_dzErr");
    auto Muon_eta = nano_reader.getVec<Float_t>("Muon_eta");
    auto Muon_ip3d = nano_reader.getVec<Float_t>("Muon_ip3d");
    auto Muon_jetPtRelv2 = nano_reader.getVec<Float_t>("Muon_jetPtRelv2");
    auto Muon_jetRelIso = nano_reader.getVec<Float_t>("Muon_jetRelIso");
    auto Muon_mass = nano_reader.getVec<Float_t>("Muon_mass");
    auto Muon_miniPFRelIso_all = nano_reader.getVec<Float_t>("Muon_miniPFRelIso_all");
    auto Muon_miniPFRelIso_chg = nano_reader.getVec<Float_t>("Muon_miniPFRelIso_chg");
    auto Muon_pfRelIso03_all = nano_reader.getVec<Float_t>("Muon_pfRelIso03_all");
    auto Muon_pfRelIso03_chg = nano_reader.getVec<Float_t>("Muon_pfRelIso03_chg");
    auto Muon_pfRelIso04_all = nano_reader.getVec<Float_t>("Muon_pfRelIso04_all");
    auto Muon_phi = nano_reader.getVec<Float_t>("Muon_phi");
    auto Muon_pt = nano_reader.getVec<Float_t>("Muon_pt");
    auto Muon_ptErr = nano_reader.getVec<Float_t>("Muon_ptErr");
    auto Muon_segmentComp = nano_reader.getVec<Float_t>("Muon_segmentComp");
    auto Muon_sip3d = nano_reader.getVec<Float_t>("Muon_sip3d");
    auto Muon_softMva = nano_reader.getVec<Float_t>("Muon_softMva");
    auto Muon_tkRelIso = nano_reader.getVec<Float_t>("Muon_tkRelIso");
    auto Muon_tunepRelPt = nano_reader.getVec<Float_t>("Muon_tunepRelPt");
    auto Muon_mvaLowPt = nano_reader.getVec<Float_t>("Muon_mvaLowPt");
    auto Muon_mvaTTH = nano_reader.getVec<Float_t>("Muon_mvaTTH");
    auto Muon_charge = nano_reader.getVec<Int_t>("Muon_charge");
    auto Muon_jetIdx = nano_reader.getVec<Int_t>("Muon_jetIdx");
    auto Muon_nStations = nano_reader.getVec<Int_t>("Muon_nStations");
    auto Muon_nTrackerLayers = nano_reader.getVec<Int_t>("Muon_nTrackerLayers");
    auto Muon_pdgId = nano_reader.getVec<Int_t>("Muon_pdgId");
    auto Muon_tightCharge = nano_reader.getVec<Int_t>("Muon_tightCharge");
    auto Muon_fsrPhotonIdx = nano_reader.getVec<Int_t>("Muon_fsrPhotonIdx");
    auto Muon_highPtId = nano_reader.getVec<UChar_t>("Muon_highPtId");
    auto Muon_highPurity = nano_reader.getVec<Bool_t, unsigned int>("Muon_highPurity");
    auto Muon_inTimeMuon = nano_reader.getVec<Bool_t, unsigned int>("Muon_inTimeMuon");
    auto Muon_isGlobal = nano_reader.getVec<Bool_t, unsigned int>("Muon_isGlobal");
    auto Muon_isPFcand = nano_reader.getVec<Bool_t, unsigned int>("Muon_isPFcand");
    auto Muon_isStandalone = nano_reader.getVec<Bool_t, unsigned int>("Muon_isStandalone");
    auto Muon_isTracker = nano_reader.getVec<Bool_t, unsigned int>("Muon_isTracker");
    auto Muon_jetNDauCharged = nano_reader.getVec<UChar_t>("Muon_jetNDauCharged");
    auto Muon_looseId = nano_reader.getVec<Bool_t, unsigned int>("Muon_looseId");
    auto Muon_mediumId = nano_reader.getVec<Bool_t, unsigned int>("Muon_mediumId");
    auto Muon_mediumPromptId = nano_reader.getVec<Bool_t, unsigned int>("Muon_mediumPromptId");
    auto Muon_miniIsoId = nano_reader.getVec<UChar_t>("Muon_miniIsoId");
    auto Muon_multiIsoId = nano_reader.getVec<UChar_t>("Muon_multiIsoId");
    auto Muon_mvaId = nano_reader.getVec<UChar_t>("Muon_mvaId");
    auto Muon_mvaLowPtId = nano_reader.getVec<UChar_t>("Muon_mvaLowPtId");
    auto Muon_pfIsoId = nano_reader.getVec<UChar_t>("Muon_pfIsoId");
    auto Muon_puppiIsoId = nano_reader.getVec<UChar_t>("Muon_puppiIsoId");
    auto Muon_softId = nano_reader.getVec<Bool_t, unsigned int>("Muon_softId");
    auto Muon_softMvaId = nano_reader.getVec<Bool_t, unsigned int>("Muon_softMvaId");
    auto Muon_tightId = nano_reader.getVec<Bool_t, unsigned int>("Muon_tightId");
    auto Muon_tkIsoId = nano_reader.getVec<UChar_t>("Muon_tkIsoId");
    auto Muon_triggerIdLoose = nano_reader.getVec<Bool_t, unsigned int>("Muon_triggerIdLoose");
    auto Muon_genPartIdx = nano_reader.getVec<Int_t>("Muon_genPartIdx");
    auto Muon_genPartFlav = nano_reader.getVec<UChar_t>("Muon_genPartFlav");
    auto Muon_cleanmask = nano_reader.getVec<UChar_t>("Muon_cleanmask");

    auto part_size = nano_reader.getVal<UInt_t>("nMuon");
    RecEvtView->setUserRecord("NumMuon", part_size);

    // loop over boosted muons
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = RecEvtView->create<pxl::Particle>();
        part->setName("Muon");
        part->setCharge(Muon_charge[idx_part]);
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(Muon_pt[idx_part], Muon_eta[idx_part], Muon_phi[idx_part], Muon_mass[idx_part]);

        // set all available records
        part->setUserRecord("dxy", Muon_dxy[idx_part]);
        part->setUserRecord("dxyErr", Muon_dxyErr[idx_part]);
        part->setUserRecord("dxybs", Muon_dxybs[idx_part]);
        part->setUserRecord("dz", Muon_dz[idx_part]);
        part->setUserRecord("dzErr", Muon_dzErr[idx_part]);
        part->setUserRecord("eta", Muon_eta[idx_part]);
        part->setUserRecord("ip3d", Muon_ip3d[idx_part]);
        part->setUserRecord("jetPtRelv2", Muon_jetPtRelv2[idx_part]);
        part->setUserRecord("jetRelIso", Muon_jetRelIso[idx_part]);
        part->setUserRecord("mass", Muon_mass[idx_part]);
        part->setUserRecord("miniPFRelIso_all", Muon_miniPFRelIso_all[idx_part]);
        part->setUserRecord("miniPFRelIso_chg", Muon_miniPFRelIso_chg[idx_part]);
        part->setUserRecord("pfRelIso03_all", Muon_pfRelIso03_all[idx_part]);
        part->setUserRecord("pfRelIso03_chg", Muon_pfRelIso03_chg[idx_part]);
        part->setUserRecord("pfRelIso04_all", Muon_pfRelIso04_all[idx_part]);
        part->setUserRecord("phi", Muon_phi[idx_part]);
        part->setUserRecord("pt", Muon_pt[idx_part]);
        part->setUserRecord("ptErr", Muon_ptErr[idx_part]);
        part->setUserRecord("segmentComp", Muon_segmentComp[idx_part]);
        part->setUserRecord("sip3d", Muon_sip3d[idx_part]);
        part->setUserRecord("softMva", Muon_softMva[idx_part]);
        part->setUserRecord("tkRelIso", Muon_tkRelIso[idx_part]);
        part->setUserRecord("tunepRelPt", Muon_tunepRelPt[idx_part]);
        part->setUserRecord("mvaLowPt", Muon_mvaLowPt[idx_part]);
        part->setUserRecord("mvaTTH", Muon_mvaTTH[idx_part]);
        part->setUserRecord("charge", Muon_charge[idx_part]);
        part->setUserRecord("jetIdx", Muon_jetIdx[idx_part]);
        part->setUserRecord("nStations", Muon_nStations[idx_part]);
        part->setUserRecord("nTrackerLayers", Muon_nTrackerLayers[idx_part]);
        part->setUserRecord("pdgId", Muon_pdgId[idx_part]);
        part->setUserRecord("tightCharge", Muon_tightCharge[idx_part]);
        part->setUserRecord("fsrPhotonIdx", Muon_fsrPhotonIdx[idx_part]);
        part->setUserRecord("highPtId", Muon_highPtId[idx_part]);
        part->setUserRecord("highPurity", Muon_highPurity[idx_part]);
        part->setUserRecord("inTimeMuon", Muon_inTimeMuon[idx_part]);
        part->setUserRecord("isGlobal", Muon_isGlobal[idx_part]);
        part->setUserRecord("isPFcand", Muon_isPFcand[idx_part]);
        part->setUserRecord("isStandalone", Muon_isStandalone[idx_part]);
        part->setUserRecord("isTracker", Muon_isTracker[idx_part]);
        part->setUserRecord("jetNDauCharged", Muon_jetNDauCharged[idx_part]);
        part->setUserRecord("looseId", Muon_looseId[idx_part]);
        part->setUserRecord("mediumId", Muon_mediumId[idx_part]);
        part->setUserRecord("mediumPromptId", Muon_mediumPromptId[idx_part]);
        part->setUserRecord("miniIsoId", Muon_miniIsoId[idx_part]);
        part->setUserRecord("multiIsoId", Muon_multiIsoId[idx_part]);
        part->setUserRecord("mvaId", Muon_mvaId[idx_part]);
        part->setUserRecord("mvaLowPtId", Muon_mvaLowPtId[idx_part]);
        part->setUserRecord("pfIsoId", Muon_pfIsoId[idx_part]);
        part->setUserRecord("puppiIsoId", Muon_puppiIsoId[idx_part]);
        part->setUserRecord("softId", Muon_softId[idx_part]);
        part->setUserRecord("softMvaId", Muon_softMvaId[idx_part]);
        part->setUserRecord("tightId", Muon_tightId[idx_part]);
        part->setUserRecord("tkIsoId", Muon_tkIsoId[idx_part]);
        part->setUserRecord("triggerIdLoose", Muon_triggerIdLoose[idx_part]);
        part->setUserRecord("genPartIdx", Muon_genPartIdx[idx_part]);
        part->setUserRecord("genPartFlav", Muon_genPartFlav[idx_part]);
        part->setUserRecord("cleanmask", Muon_cleanmask[idx_part]);
    }
}

// electrons
void analyzeRecElectrons(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{

    auto Electron_dEscaleDown = nano_reader.getVec<Float_t>("Electron_dEscaleDown");
    auto Electron_dEscaleUp = nano_reader.getVec<Float_t>("Electron_dEscaleUp");
    auto Electron_dEsigmaDown = nano_reader.getVec<Float_t>("Electron_dEsigmaDown");
    auto Electron_dEsigmaUp = nano_reader.getVec<Float_t>("Electron_dEsigmaUp");
    auto Electron_deltaEtaSC = nano_reader.getVec<Float_t>("Electron_deltaEtaSC");
    auto Electron_dr03EcalRecHitSumEt = nano_reader.getVec<Float_t>("Electron_dr03EcalRecHitSumEt");
    auto Electron_dr03HcalDepth1TowerSumEt = nano_reader.getVec<Float_t>("Electron_dr03HcalDepth1TowerSumEt");
    auto Electron_dr03TkSumPt = nano_reader.getVec<Float_t>("Electron_dr03TkSumPt");
    auto Electron_dr03TkSumPtHEEP = nano_reader.getVec<Float_t>("Electron_dr03TkSumPtHEEP");
    auto Electron_dxy = nano_reader.getVec<Float_t>("Electron_dxy");
    auto Electron_dxyErr = nano_reader.getVec<Float_t>("Electron_dxyErr");
    auto Electron_dz = nano_reader.getVec<Float_t>("Electron_dz");
    auto Electron_dzErr = nano_reader.getVec<Float_t>("Electron_dzErr");
    auto Electron_eCorr = nano_reader.getVec<Float_t>("Electron_eCorr");
    auto Electron_eInvMinusPInv = nano_reader.getVec<Float_t>("Electron_eInvMinusPInv");
    auto Electron_energyErr = nano_reader.getVec<Float_t>("Electron_energyErr");
    auto Electron_eta = nano_reader.getVec<Float_t>("Electron_eta");
    auto Electron_hoe = nano_reader.getVec<Float_t>("Electron_hoe");
    auto Electron_ip3d = nano_reader.getVec<Float_t>("Electron_ip3d");
    auto Electron_jetPtRelv2 = nano_reader.getVec<Float_t>("Electron_jetPtRelv2");
    auto Electron_jetRelIso = nano_reader.getVec<Float_t>("Electron_jetRelIso");
    auto Electron_mass = nano_reader.getVec<Float_t>("Electron_mass");
    auto Electron_miniPFRelIso_all = nano_reader.getVec<Float_t>("Electron_miniPFRelIso_all");
    auto Electron_miniPFRelIso_chg = nano_reader.getVec<Float_t>("Electron_miniPFRelIso_chg");
    auto Electron_mvaFall17V2Iso = nano_reader.getVec<Float_t>("Electron_mvaFall17V2Iso");
    auto Electron_mvaFall17V2noIso = nano_reader.getVec<Float_t>("Electron_mvaFall17V2noIso");
    auto Electron_pfRelIso03_all = nano_reader.getVec<Float_t>("Electron_pfRelIso03_all");
    auto Electron_pfRelIso03_chg = nano_reader.getVec<Float_t>("Electron_pfRelIso03_chg");
    auto Electron_phi = nano_reader.getVec<Float_t>("Electron_phi");
    auto Electron_pt = nano_reader.getVec<Float_t>("Electron_pt");
    auto Electron_r9 = nano_reader.getVec<Float_t>("Electron_r9");
    auto Electron_scEtOverPt = nano_reader.getVec<Float_t>("Electron_scEtOverPt");
    auto Electron_sieie = nano_reader.getVec<Float_t>("Electron_sieie");
    auto Electron_sip3d = nano_reader.getVec<Float_t>("Electron_sip3d");
    auto Electron_mvaTTH = nano_reader.getVec<Float_t>("Electron_mvaTTH");
    auto Electron_charge = nano_reader.getVec<Int_t>("Electron_charge");
    auto Electron_cutBased = nano_reader.getVec<Int_t>("Electron_cutBased");
    auto Electron_jetIdx = nano_reader.getVec<Int_t>("Electron_jetIdx");
    auto Electron_pdgId = nano_reader.getVec<Int_t>("Electron_pdgId");
    auto Electron_photonIdx = nano_reader.getVec<Int_t>("Electron_photonIdx");
    auto Electron_tightCharge = nano_reader.getVec<Int_t>("Electron_tightCharge");
    auto Electron_vidNestedWPBitmap = nano_reader.getVec<Int_t>("Electron_vidNestedWPBitmap");
    auto Electron_vidNestedWPBitmapHEEP = nano_reader.getVec<Int_t>("Electron_vidNestedWPBitmapHEEP");
    auto Electron_convVeto = nano_reader.getVec<Bool_t, unsigned int>("Electron_convVeto");
    auto Electron_cutBased_HEEP = nano_reader.getVec<Bool_t, unsigned int>("Electron_cutBased_HEEP");
    auto Electron_isPFcand = nano_reader.getVec<Bool_t, unsigned int>("Electron_isPFcand");
    auto Electron_jetNDauCharged = nano_reader.getVec<UChar_t>("Electron_jetNDauCharged");
    auto Electron_lostHits = nano_reader.getVec<UChar_t>("Electron_lostHits");
    auto Electron_mvaFall17V2Iso_WP80 = nano_reader.getVec<Bool_t, unsigned int>("Electron_mvaFall17V2Iso_WP80");
    auto Electron_mvaFall17V2Iso_WP90 = nano_reader.getVec<Bool_t, unsigned int>("Electron_mvaFall17V2Iso_WP90");
    auto Electron_mvaFall17V2Iso_WPL = nano_reader.getVec<Bool_t, unsigned int>("Electron_mvaFall17V2Iso_WPL");
    auto Electron_mvaFall17V2noIso_WP80 = nano_reader.getVec<Bool_t, unsigned int>("Electron_mvaFall17V2noIso_WP80");
    auto Electron_mvaFall17V2noIso_WP90 = nano_reader.getVec<Bool_t, unsigned int>("Electron_mvaFall17V2noIso_WP90");
    auto Electron_mvaFall17V2noIso_WPL = nano_reader.getVec<Bool_t, unsigned int>("Electron_mvaFall17V2noIso_WPL");
    auto Electron_seedGain = nano_reader.getVec<UChar_t>("Electron_seedGain");
    auto Electron_genPartIdx = nano_reader.getVec<Int_t>("Electron_genPartIdx");
    auto Electron_genPartFlav = nano_reader.getVec<UChar_t>("Electron_genPartFlav");
    auto Electron_cleanmask = nano_reader.getVec<UChar_t>("Electron_cleanmask");

    auto part_size = nano_reader.getVal<UInt_t>("nElectron");
    RecEvtView->setUserRecord("NumEle", part_size);

    // loop over electrons
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = RecEvtView->create<pxl::Particle>();
        part->setName("Ele");
        part->setCharge(Electron_charge[idx_part]);
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(Electron_pt[idx_part], Electron_eta[idx_part], Electron_phi[idx_part],
                             Electron_mass[idx_part]);

        // set all available records
        part->setUserRecord("dEscaleDown", Electron_dEscaleDown[idx_part]);
        part->setUserRecord("dEscaleUp", Electron_dEscaleUp[idx_part]);
        part->setUserRecord("dEsigmaDown", Electron_dEsigmaDown[idx_part]);
        part->setUserRecord("dEsigmaUp", Electron_dEsigmaUp[idx_part]);
        part->setUserRecord("deltaEtaSC", Electron_deltaEtaSC[idx_part]);
        part->setUserRecord("dr03EcalRecHitSumEt", Electron_dr03EcalRecHitSumEt[idx_part]);
        part->setUserRecord("dr03HcalDepth1TowerSumEt", Electron_dr03HcalDepth1TowerSumEt[idx_part]);
        part->setUserRecord("dr03TkSumPt", Electron_dr03TkSumPt[idx_part]);
        part->setUserRecord("dr03TkSumPtHEEP", Electron_dr03TkSumPtHEEP[idx_part]);
        part->setUserRecord("dxy", Electron_dxy[idx_part]);
        part->setUserRecord("dxyErr", Electron_dxyErr[idx_part]);
        part->setUserRecord("dz", Electron_dz[idx_part]);
        part->setUserRecord("dzErr", Electron_dzErr[idx_part]);
        part->setUserRecord("eCorr", Electron_eCorr[idx_part]);
        part->setUserRecord("eInvMinusPInv", Electron_eInvMinusPInv[idx_part]);
        part->setUserRecord("energyErr", Electron_energyErr[idx_part]);
        part->setUserRecord("eta", Electron_eta[idx_part]);
        part->setUserRecord("hoe", Electron_hoe[idx_part]);
        part->setUserRecord("ip3d", Electron_ip3d[idx_part]);
        part->setUserRecord("jetPtRelv2", Electron_jetPtRelv2[idx_part]);
        part->setUserRecord("jetRelIso", Electron_jetRelIso[idx_part]);
        part->setUserRecord("mass", Electron_mass[idx_part]);
        part->setUserRecord("miniPFRelIso_all", Electron_miniPFRelIso_all[idx_part]);
        part->setUserRecord("miniPFRelIso_chg", Electron_miniPFRelIso_chg[idx_part]);
        part->setUserRecord("mvaFall17V2Iso", Electron_mvaFall17V2Iso[idx_part]);
        part->setUserRecord("mvaFall17V2noIso", Electron_mvaFall17V2noIso[idx_part]);
        part->setUserRecord("pfRelIso03_all", Electron_pfRelIso03_all[idx_part]);
        part->setUserRecord("pfRelIso03_chg", Electron_pfRelIso03_chg[idx_part]);
        part->setUserRecord("phi", Electron_phi[idx_part]);
        part->setUserRecord("pt", Electron_pt[idx_part]);
        part->setUserRecord("r9", Electron_r9[idx_part]);
        part->setUserRecord("scEtOverPt", Electron_scEtOverPt[idx_part]);
        part->setUserRecord("sieie", Electron_sieie[idx_part]);
        part->setUserRecord("sip3d", Electron_sip3d[idx_part]);
        part->setUserRecord("mvaTTH", Electron_mvaTTH[idx_part]);
        part->setUserRecord("charge", Electron_charge[idx_part]);
        part->setUserRecord("cutBased", Electron_cutBased[idx_part]);
        part->setUserRecord("jetIdx", Electron_jetIdx[idx_part]);
        part->setUserRecord("pdgId", Electron_pdgId[idx_part]);
        part->setUserRecord("photonIdx", Electron_photonIdx[idx_part]);
        part->setUserRecord("tightCharge", Electron_tightCharge[idx_part]);
        part->setUserRecord("vidNestedWPBitmap", Electron_vidNestedWPBitmap[idx_part]);
        part->setUserRecord("vidNestedWPBitmapHEEP", Electron_vidNestedWPBitmapHEEP[idx_part]);
        part->setUserRecord("convVeto", Electron_convVeto[idx_part]);
        part->setUserRecord("cutBased_HEEP", Electron_cutBased_HEEP[idx_part]);
        part->setUserRecord("isPFcand", Electron_isPFcand[idx_part]);
        part->setUserRecord("jetNDauCharged", Electron_jetNDauCharged[idx_part]);
        part->setUserRecord("lostHits", Electron_lostHits[idx_part]);
        part->setUserRecord("mvaFall17V2Iso_WP80", Electron_mvaFall17V2Iso_WP80[idx_part]);
        part->setUserRecord("mvaFall17V2Iso_WP90", Electron_mvaFall17V2Iso_WP90[idx_part]);
        part->setUserRecord("mvaFall17V2Iso_WPL", Electron_mvaFall17V2Iso_WPL[idx_part]);
        part->setUserRecord("mvaFall17V2noIso_WP80", Electron_mvaFall17V2noIso_WP80[idx_part]);
        part->setUserRecord("mvaFall17V2noIso_WP90", Electron_mvaFall17V2noIso_WP90[idx_part]);
        part->setUserRecord("mvaFall17V2noIso_WPL", Electron_mvaFall17V2noIso_WPL[idx_part]);
        part->setUserRecord("seedGain", Electron_seedGain[idx_part]);
        part->setUserRecord("genPartIdx", Electron_genPartIdx[idx_part]);
        part->setUserRecord("genPartFlav", Electron_genPartFlav[idx_part]);
        part->setUserRecord("cleanmask", Electron_cleanmask[idx_part]);
    }
}

// photons
void analyzeRecPhotons(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{

    auto Photon_dEscaleDown = nano_reader.getVec<Float_t>("Photon_dEscaleDown");
    auto Photon_dEscaleUp = nano_reader.getVec<Float_t>("Photon_dEscaleUp");
    auto Photon_dEsigmaDown = nano_reader.getVec<Float_t>("Photon_dEsigmaDown");
    auto Photon_dEsigmaUp = nano_reader.getVec<Float_t>("Photon_dEsigmaUp");
    auto Photon_eCorr = nano_reader.getVec<Float_t>("Photon_eCorr");
    auto Photon_energyErr = nano_reader.getVec<Float_t>("Photon_energyErr");
    auto Photon_eta = nano_reader.getVec<Float_t>("Photon_eta");
    auto Photon_hoe = nano_reader.getVec<Float_t>("Photon_hoe");
    auto Photon_mass = nano_reader.getVec<Float_t>("Photon_mass");
    auto Photon_mvaID = nano_reader.getVec<Float_t>("Photon_mvaID");
    auto Photon_mvaID_Fall17V1p1 = nano_reader.getVec<Float_t>("Photon_mvaID_Fall17V1p1");
    auto Photon_pfRelIso03_all = nano_reader.getVec<Float_t>("Photon_pfRelIso03_all");
    auto Photon_pfRelIso03_chg = nano_reader.getVec<Float_t>("Photon_pfRelIso03_chg");
    auto Photon_phi = nano_reader.getVec<Float_t>("Photon_phi");
    auto Photon_pt = nano_reader.getVec<Float_t>("Photon_pt");
    auto Photon_r9 = nano_reader.getVec<Float_t>("Photon_r9");
    auto Photon_sieie = nano_reader.getVec<Float_t>("Photon_sieie");
    auto Photon_charge = nano_reader.getVec<Int_t>("Photon_charge");
    auto Photon_cutBased = nano_reader.getVec<Int_t>("Photon_cutBased");
    auto Photon_cutBased_Fall17V1Bitmap = nano_reader.getVec<Int_t>("Photon_cutBased_Fall17V1Bitmap");
    auto Photon_electronIdx = nano_reader.getVec<Int_t>("Photon_electronIdx");
    auto Photon_jetIdx = nano_reader.getVec<Int_t>("Photon_jetIdx");
    auto Photon_pdgId = nano_reader.getVec<Int_t>("Photon_pdgId");
    auto Photon_vidNestedWPBitmap = nano_reader.getVec<Int_t>("Photon_vidNestedWPBitmap");
    auto Photon_electronVeto = nano_reader.getVec<Bool_t, unsigned int>("Photon_electronVeto");
    auto Photon_isScEtaEB = nano_reader.getVec<Bool_t, unsigned int>("Photon_isScEtaEB");
    auto Photon_isScEtaEE = nano_reader.getVec<Bool_t, unsigned int>("Photon_isScEtaEE");
    auto Photon_mvaID_WP80 = nano_reader.getVec<Bool_t, unsigned int>("Photon_mvaID_WP80");
    auto Photon_mvaID_WP90 = nano_reader.getVec<Bool_t, unsigned int>("Photon_mvaID_WP90");
    auto Photon_pixelSeed = nano_reader.getVec<Bool_t, unsigned int>("Photon_pixelSeed");
    auto Photon_seedGain = nano_reader.getVec<UChar_t>("Photon_seedGain");
    auto Photon_genPartIdx = nano_reader.getVec<Int_t>("Photon_genPartIdx");
    auto Photon_genPartFlav = nano_reader.getVec<UChar_t>("Photon_genPartFlav");
    auto Photon_cleanmask = nano_reader.getVec<UChar_t>("Photon_cleanmask");

    auto part_size = nano_reader.getVal<UInt_t>("nPhoton");
    RecEvtView->setUserRecord("NumGamma", part_size);

    // loop over photons
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = RecEvtView->create<pxl::Particle>();
        part->setName("Gamma");
        part->setCharge(Photon_charge[idx_part]);
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(Photon_pt[idx_part], Photon_eta[idx_part], Photon_phi[idx_part], Photon_mass[idx_part]);

        // set all available records
        part->setUserRecord("dEscaleDown", Photon_dEscaleDown[idx_part]);
        part->setUserRecord("dEscaleUp", Photon_dEscaleUp[idx_part]);
        part->setUserRecord("dEsigmaDown", Photon_dEsigmaDown[idx_part]);
        part->setUserRecord("dEsigmaUp", Photon_dEsigmaUp[idx_part]);
        part->setUserRecord("eCorr", Photon_eCorr[idx_part]);
        part->setUserRecord("energyErr", Photon_energyErr[idx_part]);
        part->setUserRecord("eta", Photon_eta[idx_part]);
        part->setUserRecord("hoe", Photon_hoe[idx_part]);
        part->setUserRecord("mass", Photon_mass[idx_part]);
        part->setUserRecord("mvaID", Photon_mvaID[idx_part]);
        part->setUserRecord("mvaID_Fall17V1p1", Photon_mvaID_Fall17V1p1[idx_part]);
        part->setUserRecord("pfRelIso03_all", Photon_pfRelIso03_all[idx_part]);
        part->setUserRecord("pfRelIso03_chg", Photon_pfRelIso03_chg[idx_part]);
        part->setUserRecord("phi", Photon_phi[idx_part]);
        part->setUserRecord("pt", Photon_pt[idx_part]);
        part->setUserRecord("r9", Photon_r9[idx_part]);
        part->setUserRecord("sieie", Photon_sieie[idx_part]);
        part->setUserRecord("charge", Photon_charge[idx_part]);
        part->setUserRecord("cutBased", Photon_cutBased[idx_part]);
        part->setUserRecord("cutBased_Fall17V1Bitmap", Photon_cutBased_Fall17V1Bitmap[idx_part]);
        part->setUserRecord("electronIdx", Photon_electronIdx[idx_part]);
        part->setUserRecord("jetIdx", Photon_jetIdx[idx_part]);
        part->setUserRecord("pdgId", Photon_pdgId[idx_part]);
        part->setUserRecord("vidNestedWPBitmap", Photon_vidNestedWPBitmap[idx_part]);
        part->setUserRecord("electronVeto", Photon_electronVeto[idx_part]);
        part->setUserRecord("isScEtaEB", Photon_isScEtaEB[idx_part]);
        part->setUserRecord("isScEtaEE", Photon_isScEtaEE[idx_part]);
        part->setUserRecord("mvaID_WP80", Photon_mvaID_WP80[idx_part]);
        part->setUserRecord("mvaID_WP90", Photon_mvaID_WP90[idx_part]);
        part->setUserRecord("pixelSeed", Photon_pixelSeed[idx_part]);
        part->setUserRecord("seedGain", Photon_seedGain[idx_part]);
        part->setUserRecord("genPartIdx", Photon_genPartIdx[idx_part]);
        part->setUserRecord("genPartFlav", Photon_genPartFlav[idx_part]);
        part->setUserRecord("cleanmask", Photon_cleanmask[idx_part]);
    }
}

// MET
void analyzeRecMET(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    auto MET_MetUnclustEnUpDeltaX = nano_reader.getVal<Float_t>("MET_MetUnclustEnUpDeltaX");
    auto MET_MetUnclustEnUpDeltaY = nano_reader.getVal<Float_t>("MET_MetUnclustEnUpDeltaY");
    auto MET_covXX = nano_reader.getVal<Float_t>("MET_covXX");
    auto MET_covXY = nano_reader.getVal<Float_t>("MET_covXY");
    auto MET_covYY = nano_reader.getVal<Float_t>("MET_covYY");
    auto MET_phi = nano_reader.getVal<Float_t>("MET_phi");
    auto MET_pt = nano_reader.getVal<Float_t>("MET_pt");
    auto MET_significance = nano_reader.getVal<Float_t>("MET_significance");
    auto MET_sumEt = nano_reader.getVal<Float_t>("MET_sumEt");
    auto MET_sumPtUnclustered = nano_reader.getVal<Float_t>("MET_sumPtUnclustered");
    auto MET_fiducialGenPhi = nano_reader.getVal<Float_t>("MET_fiducialGenPhi");
    auto MET_fiducialGenPt = nano_reader.getVal<Float_t>("MET_fiducialGenPt");

    RecEvtView->setUserRecord("NumMET", 1);

    pxl::Particle *part = RecEvtView->create<pxl::Particle>();
    part->setName("MET");
    part->setCharge(0);
    auto p_temp_ = TLorentzVector();
    p_temp_.SetPtEtaPhiM(MET_pt, 0.0, MET_phi, 0.0);

    // set all available records
    part->setUserRecord("MetUnclustEnUpDeltaX", MET_MetUnclustEnUpDeltaX);
    part->setUserRecord("MetUnclustEnUpDeltaY", MET_MetUnclustEnUpDeltaY);
    part->setUserRecord("covXX", MET_covXX);
    part->setUserRecord("covXY", MET_covXY);
    part->setUserRecord("covYY", MET_covYY);
    part->setUserRecord("phi", MET_phi);
    part->setUserRecord("pt", MET_pt);
    part->setUserRecord("significance", MET_significance);
    part->setUserRecord("sumEt", MET_sumEt);
    part->setUserRecord("sumPtUnclustered", MET_sumPtUnclustered);
    part->setUserRecord("fiducialGenPhi", MET_fiducialGenPhi);
    part->setUserRecord("fiducialGenPt", MET_fiducialGenPt);
}

// PuppiMET
void analyzeRecPuppiMET(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    auto PuppiMET_phi = nano_reader.getVal<Float_t>("PuppiMET_phi");
    auto PuppiMET_phiJERDown = nano_reader.getVal<Float_t>("PuppiMET_phiJERDown");
    auto PuppiMET_phiJERUp = nano_reader.getVal<Float_t>("PuppiMET_phiJERUp");
    auto PuppiMET_phiJESDown = nano_reader.getVal<Float_t>("PuppiMET_phiJESDown");
    auto PuppiMET_phiJESUp = nano_reader.getVal<Float_t>("PuppiMET_phiJESUp");
    auto PuppiMET_phiUnclusteredDown = nano_reader.getVal<Float_t>("PuppiMET_phiUnclusteredDown");
    auto PuppiMET_phiUnclusteredUp = nano_reader.getVal<Float_t>("PuppiMET_phiUnclusteredUp");
    auto PuppiMET_pt = nano_reader.getVal<Float_t>("PuppiMET_pt");
    auto PuppiMET_ptJERDown = nano_reader.getVal<Float_t>("PuppiMET_ptJERDown");
    auto PuppiMET_ptJERUp = nano_reader.getVal<Float_t>("PuppiMET_ptJERUp");
    auto PuppiMET_ptJESDown = nano_reader.getVal<Float_t>("PuppiMET_ptJESDown");
    auto PuppiMET_ptJESUp = nano_reader.getVal<Float_t>("PuppiMET_ptJESUp");
    auto PuppiMET_ptUnclusteredDown = nano_reader.getVal<Float_t>("PuppiMET_ptUnclusteredDown");
    auto PuppiMET_ptUnclusteredUp = nano_reader.getVal<Float_t>("PuppiMET_ptUnclusteredUp");
    auto PuppiMET_sumEt = nano_reader.getVal<Float_t>("PuppiMET_sumEt");

    RecEvtView->setUserRecord("NumPuppiMET", 1);

    pxl::Particle *part = RecEvtView->create<pxl::Particle>();
    part->setName("PuppiMET");
    part->setCharge(0);
    auto p_temp_ = TLorentzVector();
    p_temp_.SetPtEtaPhiM(PuppiMET_pt, 0.0, PuppiMET_phi, 0.0);

    // set all available records
    part->setUserRecord("phi", PuppiMET_phi);
    part->setUserRecord("phiJERDown", PuppiMET_phiJERDown);
    part->setUserRecord("phiJERUp", PuppiMET_phiJERUp);
    part->setUserRecord("phiJESDown", PuppiMET_phiJESDown);
    part->setUserRecord("phiJESUp", PuppiMET_phiJESUp);
    part->setUserRecord("phiUnclusteredDown", PuppiMET_phiUnclusteredDown);
    part->setUserRecord("phiUnclusteredUp", PuppiMET_phiUnclusteredUp);
    part->setUserRecord("pt", PuppiMET_pt);
    part->setUserRecord("ptJERDown", PuppiMET_ptJERDown);
    part->setUserRecord("ptJERUp", PuppiMET_ptJERUp);
    part->setUserRecord("ptJESDown", PuppiMET_ptJESDown);
    part->setUserRecord("ptJESUp", PuppiMET_ptJESUp);
    part->setUserRecord("ptUnclusteredDown", PuppiMET_ptUnclusteredDown);
    part->setUserRecord("ptUnclusteredUp", PuppiMET_ptUnclusteredUp);
    part->setUserRecord("sumEt", PuppiMET_sumEt);
}

// jets
void analyzeRecJets(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    auto Jet_area = nano_reader.getVec<Float_t>("Jet_area");
    auto Jet_btagCSVV2 = nano_reader.getVec<Float_t>("Jet_btagCSVV2");
    auto Jet_btagDeepB = nano_reader.getVec<Float_t>("Jet_btagDeepB");
    auto Jet_btagDeepCvB = nano_reader.getVec<Float_t>("Jet_btagDeepCvB");
    auto Jet_btagDeepCvL = nano_reader.getVec<Float_t>("Jet_btagDeepCvL");
    auto Jet_btagDeepFlavB = nano_reader.getVec<Float_t>("Jet_btagDeepFlavB");
    auto Jet_btagDeepFlavCvB = nano_reader.getVec<Float_t>("Jet_btagDeepFlavCvB");
    auto Jet_btagDeepFlavCvL = nano_reader.getVec<Float_t>("Jet_btagDeepFlavCvL");
    auto Jet_btagDeepFlavQG = nano_reader.getVec<Float_t>("Jet_btagDeepFlavQG");
    auto Jet_chEmEF = nano_reader.getVec<Float_t>("Jet_chEmEF");
    auto Jet_chFPV0EF = nano_reader.getVec<Float_t>("Jet_chFPV0EF");
    auto Jet_chHEF = nano_reader.getVec<Float_t>("Jet_chHEF");
    auto Jet_eta = nano_reader.getVec<Float_t>("Jet_eta");
    auto Jet_hfsigmaEtaEta = nano_reader.getVec<Float_t>("Jet_hfsigmaEtaEta");
    auto Jet_hfsigmaPhiPhi = nano_reader.getVec<Float_t>("Jet_hfsigmaPhiPhi");
    auto Jet_mass = nano_reader.getVec<Float_t>("Jet_mass");
    auto Jet_muEF = nano_reader.getVec<Float_t>("Jet_muEF");
    auto Jet_muonSubtrFactor = nano_reader.getVec<Float_t>("Jet_muonSubtrFactor");
    auto Jet_neEmEF = nano_reader.getVec<Float_t>("Jet_neEmEF");
    auto Jet_neHEF = nano_reader.getVec<Float_t>("Jet_neHEF");
    auto Jet_phi = nano_reader.getVec<Float_t>("Jet_phi");
    auto Jet_pt = nano_reader.getVec<Float_t>("Jet_pt");
    auto Jet_puIdDisc = nano_reader.getVec<Float_t>("Jet_puIdDisc");
    auto Jet_qgl = nano_reader.getVec<Float_t>("Jet_qgl");
    auto Jet_rawFactor = nano_reader.getVec<Float_t>("Jet_rawFactor");
    auto Jet_bRegCorr = nano_reader.getVec<Float_t>("Jet_bRegCorr");
    auto Jet_bRegRes = nano_reader.getVec<Float_t>("Jet_bRegRes");
    auto Jet_cRegCorr = nano_reader.getVec<Float_t>("Jet_cRegCorr");
    auto Jet_cRegRes = nano_reader.getVec<Float_t>("Jet_cRegRes");
    auto Jet_electronIdx1 = nano_reader.getVec<Int_t>("Jet_electronIdx1");
    auto Jet_electronIdx2 = nano_reader.getVec<Int_t>("Jet_electronIdx2");
    auto Jet_hfadjacentEtaStripsSize = nano_reader.getVec<Int_t>("Jet_hfadjacentEtaStripsSize");
    auto Jet_hfcentralEtaStripSize = nano_reader.getVec<Int_t>("Jet_hfcentralEtaStripSize");
    auto Jet_jetId = nano_reader.getVec<Int_t>("Jet_jetId");
    auto Jet_muonIdx1 = nano_reader.getVec<Int_t>("Jet_muonIdx1");
    auto Jet_muonIdx2 = nano_reader.getVec<Int_t>("Jet_muonIdx2");
    auto Jet_nElectrons = nano_reader.getVec<Int_t>("Jet_nElectrons");
    auto Jet_nMuons = nano_reader.getVec<Int_t>("Jet_nMuons");
    auto Jet_puId = nano_reader.getVec<Int_t>("Jet_puId");
    auto Jet_nConstituents = nano_reader.getVec<UChar_t>("Jet_nConstituents");
    auto Jet_genJetIdx = nano_reader.getVec<Int_t>("Jet_genJetIdx");
    auto Jet_hadronFlavour = nano_reader.getVec<Int_t>("Jet_hadronFlavour");
    auto Jet_partonFlavour = nano_reader.getVec<Int_t>("Jet_partonFlavour");
    auto Jet_cleanmask = nano_reader.getVec<UChar_t>("Jet_cleanmask");

    auto part_size = nano_reader.getVal<UInt_t>("nJet");
    RecEvtView->setUserRecord("NumJet", part_size);

    // loop over photons
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = RecEvtView->create<pxl::Particle>();
        part->setName("Jet");
        part->setCharge(0);
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(Jet_pt[idx_part], Jet_eta[idx_part], Jet_phi[idx_part], Jet_mass[idx_part]);

        // set all available records
        part->setUserRecord("area", Jet_area[idx_part]);
        part->setUserRecord("btagCSVV2", Jet_btagCSVV2[idx_part]);
        part->setUserRecord("btagDeepB", Jet_btagDeepB[idx_part]);
        part->setUserRecord("btagDeepCvB", Jet_btagDeepCvB[idx_part]);
        part->setUserRecord("btagDeepCvL", Jet_btagDeepCvL[idx_part]);
        part->setUserRecord("btagDeepFlavB", Jet_btagDeepFlavB[idx_part]);
        part->setUserRecord("btagDeepFlavCvB", Jet_btagDeepFlavCvB[idx_part]);
        part->setUserRecord("btagDeepFlavCvL", Jet_btagDeepFlavCvL[idx_part]);
        part->setUserRecord("btagDeepFlavQG", Jet_btagDeepFlavQG[idx_part]);
        part->setUserRecord("chEmEF", Jet_chEmEF[idx_part]);
        part->setUserRecord("chFPV0EF", Jet_chFPV0EF[idx_part]);
        part->setUserRecord("chHEF", Jet_chHEF[idx_part]);
        part->setUserRecord("eta", Jet_eta[idx_part]);
        part->setUserRecord("hfsigmaEtaEta", Jet_hfsigmaEtaEta[idx_part]);
        part->setUserRecord("hfsigmaPhiPhi", Jet_hfsigmaPhiPhi[idx_part]);
        part->setUserRecord("mass", Jet_mass[idx_part]);
        part->setUserRecord("muEF", Jet_muEF[idx_part]);
        part->setUserRecord("muonSubtrFactor", Jet_muonSubtrFactor[idx_part]);
        part->setUserRecord("neEmEF", Jet_neEmEF[idx_part]);
        part->setUserRecord("neHEF", Jet_neHEF[idx_part]);
        part->setUserRecord("phi", Jet_phi[idx_part]);
        part->setUserRecord("pt", Jet_pt[idx_part]);
        part->setUserRecord("puIdDisc", Jet_puIdDisc[idx_part]);
        part->setUserRecord("qgl", Jet_qgl[idx_part]);
        part->setUserRecord("rawFactor", Jet_rawFactor[idx_part]);
        part->setUserRecord("bRegCorr", Jet_bRegCorr[idx_part]);
        part->setUserRecord("bRegRes", Jet_bRegRes[idx_part]);
        part->setUserRecord("cRegCorr", Jet_cRegCorr[idx_part]);
        part->setUserRecord("cRegRes", Jet_cRegRes[idx_part]);
        part->setUserRecord("electronIdx1", Jet_electronIdx1[idx_part]);
        part->setUserRecord("electronIdx2", Jet_electronIdx2[idx_part]);
        part->setUserRecord("hfadjacentEtaStripsSize", Jet_hfadjacentEtaStripsSize[idx_part]);
        part->setUserRecord("hfcentralEtaStripSize", Jet_hfcentralEtaStripSize[idx_part]);
        part->setUserRecord("jetId", Jet_jetId[idx_part]);
        part->setUserRecord("muonIdx1", Jet_muonIdx1[idx_part]);
        part->setUserRecord("muonIdx2", Jet_muonIdx2[idx_part]);
        part->setUserRecord("nElectrons", Jet_nElectrons[idx_part]);
        part->setUserRecord("nMuons", Jet_nMuons[idx_part]);
        part->setUserRecord("puId", Jet_puId[idx_part]);
        part->setUserRecord("nConstituents", Jet_nConstituents[idx_part]);
        part->setUserRecord("genJetIdx", Jet_genJetIdx[idx_part]);
        part->setUserRecord("hadronFlavour", Jet_hadronFlavour[idx_part]);
        part->setUserRecord("partonFlavour", Jet_partonFlavour[idx_part]);
        part->setUserRecord("cleanmask", Jet_cleanmask[idx_part]);
    }
}

// fatjets
void analyzeRecFatJets(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    auto FatJet_area = nano_reader.getVec<Float_t>("FatJet_area");
    auto FatJet_btagCSVV2 = nano_reader.getVec<Float_t>("FatJet_btagCSVV2");
    auto FatJet_btagDDBvLV2 = nano_reader.getVec<Float_t>("FatJet_btagDDBvLV2");
    auto FatJet_btagDDCvBV2 = nano_reader.getVec<Float_t>("FatJet_btagDDCvBV2");
    auto FatJet_btagDDCvLV2 = nano_reader.getVec<Float_t>("FatJet_btagDDCvLV2");
    auto FatJet_btagDeepB = nano_reader.getVec<Float_t>("FatJet_btagDeepB");
    auto FatJet_btagHbb = nano_reader.getVec<Float_t>("FatJet_btagHbb");
    auto FatJet_deepTagMD_H4qvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTagMD_H4qvsQCD");
    auto FatJet_deepTagMD_HbbvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTagMD_HbbvsQCD");
    auto FatJet_deepTagMD_TvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTagMD_TvsQCD");
    auto FatJet_deepTagMD_WvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTagMD_WvsQCD");
    auto FatJet_deepTagMD_ZHbbvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTagMD_ZHbbvsQCD");
    auto FatJet_deepTagMD_ZHccvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTagMD_ZHccvsQCD");
    auto FatJet_deepTagMD_ZbbvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTagMD_ZbbvsQCD");
    auto FatJet_deepTagMD_ZvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTagMD_ZvsQCD");
    auto FatJet_deepTagMD_bbvsLight = nano_reader.getVec<Float_t>("FatJet_deepTagMD_bbvsLight");
    auto FatJet_deepTagMD_ccvsLight = nano_reader.getVec<Float_t>("FatJet_deepTagMD_ccvsLight");
    auto FatJet_deepTag_H = nano_reader.getVec<Float_t>("FatJet_deepTag_H");
    auto FatJet_deepTag_QCD = nano_reader.getVec<Float_t>("FatJet_deepTag_QCD");
    auto FatJet_deepTag_QCDothers = nano_reader.getVec<Float_t>("FatJet_deepTag_QCDothers");
    auto FatJet_deepTag_TvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTag_TvsQCD");
    auto FatJet_deepTag_WvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTag_WvsQCD");
    auto FatJet_deepTag_ZvsQCD = nano_reader.getVec<Float_t>("FatJet_deepTag_ZvsQCD");
    auto FatJet_eta = nano_reader.getVec<Float_t>("FatJet_eta");
    auto FatJet_mass = nano_reader.getVec<Float_t>("FatJet_mass");
    auto FatJet_msoftdrop = nano_reader.getVec<Float_t>("FatJet_msoftdrop");
    auto FatJet_n2b1 = nano_reader.getVec<Float_t>("FatJet_n2b1");
    auto FatJet_n3b1 = nano_reader.getVec<Float_t>("FatJet_n3b1");
    auto FatJet_particleNetMD_QCD = nano_reader.getVec<Float_t>("FatJet_particleNetMD_QCD");
    auto FatJet_particleNetMD_Xbb = nano_reader.getVec<Float_t>("FatJet_particleNetMD_Xbb");
    auto FatJet_particleNetMD_Xcc = nano_reader.getVec<Float_t>("FatJet_particleNetMD_Xcc");
    auto FatJet_particleNetMD_Xqq = nano_reader.getVec<Float_t>("FatJet_particleNetMD_Xqq");
    auto FatJet_particleNet_H4qvsQCD = nano_reader.getVec<Float_t>("FatJet_particleNet_H4qvsQCD");
    auto FatJet_particleNet_HbbvsQCD = nano_reader.getVec<Float_t>("FatJet_particleNet_HbbvsQCD");
    auto FatJet_particleNet_HccvsQCD = nano_reader.getVec<Float_t>("FatJet_particleNet_HccvsQCD");
    auto FatJet_particleNet_QCD = nano_reader.getVec<Float_t>("FatJet_particleNet_QCD");
    auto FatJet_particleNet_TvsQCD = nano_reader.getVec<Float_t>("FatJet_particleNet_TvsQCD");
    auto FatJet_particleNet_WvsQCD = nano_reader.getVec<Float_t>("FatJet_particleNet_WvsQCD");
    auto FatJet_particleNet_ZvsQCD = nano_reader.getVec<Float_t>("FatJet_particleNet_ZvsQCD");
    auto FatJet_particleNet_mass = nano_reader.getVec<Float_t>("FatJet_particleNet_mass");
    auto FatJet_phi = nano_reader.getVec<Float_t>("FatJet_phi");
    auto FatJet_pt = nano_reader.getVec<Float_t>("FatJet_pt");
    auto FatJet_rawFactor = nano_reader.getVec<Float_t>("FatJet_rawFactor");
    auto FatJet_tau1 = nano_reader.getVec<Float_t>("FatJet_tau1");
    auto FatJet_tau2 = nano_reader.getVec<Float_t>("FatJet_tau2");
    auto FatJet_tau3 = nano_reader.getVec<Float_t>("FatJet_tau3");
    auto FatJet_tau4 = nano_reader.getVec<Float_t>("FatJet_tau4");
    auto FatJet_lsf3 = nano_reader.getVec<Float_t>("FatJet_lsf3");
    auto FatJet_jetId = nano_reader.getVec<Int_t>("FatJet_jetId");
    auto FatJet_subJetIdx1 = nano_reader.getVec<Int_t>("FatJet_subJetIdx1");
    auto FatJet_subJetIdx2 = nano_reader.getVec<Int_t>("FatJet_subJetIdx2");
    auto FatJet_electronIdx3SJ = nano_reader.getVec<Int_t>("FatJet_electronIdx3SJ");
    auto FatJet_muonIdx3SJ = nano_reader.getVec<Int_t>("FatJet_muonIdx3SJ");
    auto FatJet_nConstituents = nano_reader.getVec<UChar_t>("FatJet_nConstituents");
    auto FatJet_genJetAK8Idx = nano_reader.getVec<Int_t>("FatJet_genJetAK8Idx");
    auto FatJet_hadronFlavour = nano_reader.getVec<Int_t>("FatJet_hadronFlavour");
    auto FatJet_nBHadrons = nano_reader.getVec<UChar_t>("FatJet_nBHadrons");
    auto FatJet_nCHadrons = nano_reader.getVec<UChar_t>("FatJet_nCHadrons");

    auto part_size = nano_reader.getVal<UInt_t>("nFatJet");
    RecEvtView->setUserRecord("NumFatJet", part_size);

    // loop over photons
    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = RecEvtView->create<pxl::Particle>();
        part->setName("FatJet");
        part->setCharge(0);
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(FatJet_pt[idx_part], FatJet_eta[idx_part], FatJet_phi[idx_part], FatJet_mass[idx_part]);

        // set all available records
        part->setUserRecord("area", FatJet_area[idx_part]);
        part->setUserRecord("btagCSVV2", FatJet_btagCSVV2[idx_part]);
        part->setUserRecord("btagDDBvLV2", FatJet_btagDDBvLV2[idx_part]);
        part->setUserRecord("btagDDCvBV2", FatJet_btagDDCvBV2[idx_part]);
        part->setUserRecord("btagDDCvLV2", FatJet_btagDDCvLV2[idx_part]);
        part->setUserRecord("btagDeepB", FatJet_btagDeepB[idx_part]);
        part->setUserRecord("btagHbb", FatJet_btagHbb[idx_part]);
        part->setUserRecord("deepTagMD_H4qvsQCD", FatJet_deepTagMD_H4qvsQCD[idx_part]);
        part->setUserRecord("deepTagMD_HbbvsQCD", FatJet_deepTagMD_HbbvsQCD[idx_part]);
        part->setUserRecord("deepTagMD_TvsQCD", FatJet_deepTagMD_TvsQCD[idx_part]);
        part->setUserRecord("deepTagMD_WvsQCD", FatJet_deepTagMD_WvsQCD[idx_part]);
        part->setUserRecord("deepTagMD_ZHbbvsQCD", FatJet_deepTagMD_ZHbbvsQCD[idx_part]);
        part->setUserRecord("deepTagMD_ZHccvsQCD", FatJet_deepTagMD_ZHccvsQCD[idx_part]);
        part->setUserRecord("deepTagMD_ZbbvsQCD", FatJet_deepTagMD_ZbbvsQCD[idx_part]);
        part->setUserRecord("deepTagMD_ZvsQCD", FatJet_deepTagMD_ZvsQCD[idx_part]);
        part->setUserRecord("deepTagMD_bbvsLight", FatJet_deepTagMD_bbvsLight[idx_part]);
        part->setUserRecord("deepTagMD_ccvsLight", FatJet_deepTagMD_ccvsLight[idx_part]);
        part->setUserRecord("deepTag_H", FatJet_deepTag_H[idx_part]);
        part->setUserRecord("deepTag_QCD", FatJet_deepTag_QCD[idx_part]);
        part->setUserRecord("deepTag_QCDothers", FatJet_deepTag_QCDothers[idx_part]);
        part->setUserRecord("deepTag_TvsQCD", FatJet_deepTag_TvsQCD[idx_part]);
        part->setUserRecord("deepTag_WvsQCD", FatJet_deepTag_WvsQCD[idx_part]);
        part->setUserRecord("deepTag_ZvsQCD", FatJet_deepTag_ZvsQCD[idx_part]);
        part->setUserRecord("eta", FatJet_eta[idx_part]);
        part->setUserRecord("mass", FatJet_mass[idx_part]);
        part->setUserRecord("msoftdrop", FatJet_msoftdrop[idx_part]);
        part->setUserRecord("n2b1", FatJet_n2b1[idx_part]);
        part->setUserRecord("n3b1", FatJet_n3b1[idx_part]);
        part->setUserRecord("particleNetMD_QCD", FatJet_particleNetMD_QCD[idx_part]);
        part->setUserRecord("particleNetMD_Xbb", FatJet_particleNetMD_Xbb[idx_part]);
        part->setUserRecord("particleNetMD_Xcc", FatJet_particleNetMD_Xcc[idx_part]);
        part->setUserRecord("particleNetMD_Xqq", FatJet_particleNetMD_Xqq[idx_part]);
        part->setUserRecord("particleNet_H4qvsQCD", FatJet_particleNet_H4qvsQCD[idx_part]);
        part->setUserRecord("particleNet_HbbvsQCD", FatJet_particleNet_HbbvsQCD[idx_part]);
        part->setUserRecord("particleNet_HccvsQCD", FatJet_particleNet_HccvsQCD[idx_part]);
        part->setUserRecord("particleNet_QCD", FatJet_particleNet_QCD[idx_part]);
        part->setUserRecord("particleNet_TvsQCD", FatJet_particleNet_TvsQCD[idx_part]);
        part->setUserRecord("particleNet_WvsQCD", FatJet_particleNet_WvsQCD[idx_part]);
        part->setUserRecord("particleNet_ZvsQCD", FatJet_particleNet_ZvsQCD[idx_part]);
        part->setUserRecord("particleNet_mass", FatJet_particleNet_mass[idx_part]);
        part->setUserRecord("phi", FatJet_phi[idx_part]);
        part->setUserRecord("pt", FatJet_pt[idx_part]);
        part->setUserRecord("rawFactor", FatJet_rawFactor[idx_part]);
        part->setUserRecord("tau1", FatJet_tau1[idx_part]);
        part->setUserRecord("tau2", FatJet_tau2[idx_part]);
        part->setUserRecord("tau3", FatJet_tau3[idx_part]);
        part->setUserRecord("tau4", FatJet_tau4[idx_part]);
        part->setUserRecord("lsf3", FatJet_lsf3[idx_part]);
        part->setUserRecord("jetId", FatJet_jetId[idx_part]);
        part->setUserRecord("subJetIdx1", FatJet_subJetIdx1[idx_part]);
        part->setUserRecord("subJetIdx2", FatJet_subJetIdx2[idx_part]);
        part->setUserRecord("electronIdx3SJ", FatJet_electronIdx3SJ[idx_part]);
        part->setUserRecord("muonIdx3SJ", FatJet_muonIdx3SJ[idx_part]);
        part->setUserRecord("nConstituents", FatJet_nConstituents[idx_part]);
        part->setUserRecord("genJetAK8Idx", FatJet_genJetAK8Idx[idx_part]);
        part->setUserRecord("hadronFlavour", FatJet_hadronFlavour[idx_part]);
        part->setUserRecord("nBHadrons", FatJet_nBHadrons[idx_part]);
        part->setUserRecord("nCHadrons", FatJet_nCHadrons[idx_part]);
    }
}

// BTag Weights
void analyzeRecBTagWeights(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    auto btagWeight_CSVV2 = nano_reader.getVal<Float_t>("btagWeight_CSVV2");
    auto btagWeight_DeepCSVB = nano_reader.getVal<Float_t>("btagWeight_DeepCSVB");

    // set all available records
    RecEvtView->setUserRecord("btagWeight_CSVV2", btagWeight_CSVV2);
    RecEvtView->setUserRecord("btagWeight_DeepCSVB", btagWeight_DeepCSVB);
}

// L1 prefiring
void analyzePrefiringWeights(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    RecEvtView->setUserRecord("prefiring_scale_factor", nano_reader.getVal<Float_t>("L1PreFiringWeight_Nom"));
    RecEvtView->setUserRecord("prefiring_scale_factor_up", nano_reader.getVal<Float_t>("L1PreFiringWeight_Up"));
    RecEvtView->setUserRecord("prefiring_scale_factor_down", nano_reader.getVal<Float_t>("L1PreFiringWeight_ECAL_Dn"));
}