#include "TLorentzVector.h"

// L1 prefiring
void analyzePrefiringWeights(NanoAODReader &nano_reader, pxl::EventView *RecEvtView)
{
    RecEvtView->setUserRecord("prefiring_scale_factor", nano_reader.getVal<Float_t>("L1PreFiringWeight_Nom"));
    RecEvtView->setUserRecord("prefiring_scale_factor_up", nano_reader.getVal<Float_t>("L1PreFiringWeight_Up"));
    RecEvtView->setUserRecord("prefiring_scale_factor_down", nano_reader.getVal<Float_t>("L1PreFiringWeight_ECAL_Dn"));
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

    for (unsigned int idx_part = 0; idx_part < part_size; idx_part++)
    {
        pxl::Particle *part = RecEvtView->create<pxl::Particle>();
        part->setName("Tau");
        part->setCharge(Tau_charge[idx_part]);
        auto p_temp_ = TLorentzVector();
        p_temp_.SetPtEtaPhiM(Tau_pt[idx_part], Tau_eta[idx_part],Tau_phi[idx_part],Tau_mass[idx_part]);

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
    // pxl::Particle *part = RecEvtView->create<pxl::Particle>();
    // part->setName("Tau");
    // part->setCharge(nano_reader.getVal<Int_t>("Tau_charge"));
    // auto p_temp_ = TLorentzVector();
    // p_temp_.SetPtEtaPhiM(
    //     nano_reader.getVal<Float_t>("Tau_pt"),
    //     nano_reader.getVal<Float_t>("Tau_eta"),
    //     nano_reader.getVal<Float_t>("Tau_phi"),
    //     nano_reader.getVal<Float_t>("Tau_mass"));

    // part->setP4(p_temp_.Px(), p_temp_.Py(), p_temp_.Pz(), p_temp_.E());
    // for (auto &branch_name : nano_reader.getListOfBranches())
    // {
    //     if (branch_name.rfind("Tau_", 0) == 0)
    //     {
    //         part->setUserRecord(branch_name, nano_reader.getVal<Float_t>(branch_name));
    //     }
    // }
}