#include "EventAnalyzer.hpp"

///////////////////////////////////////////////////////////////
/// Muons
/// Ref (2018): https://cms-nanoaod-integration.web.cern.ch/commonJSONSFs/summaries/MUO_2018_UL_muon_Z.html
/// Low Pt Muons should consider:
/// -- Tracking efficiency: ~1.0
/// -- Reconstruction: NUM_TrackerMuons_DEN_genTracks
/// -- ID: NUM_TightID_DEN_TrackerMuons
/// -- Isolation: NUM_TightRelIso_DEN_TightIDandIPCut
/// -- Trigger: NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight (already implemented during trigger matching)
/// High Pt Muons should consider:
/// -- Tracking efficiency: ~1.0
/// -- Reconstruction: NUM_TrackerMuons_DEN_genTracks
/// -- ID: NUM_HighPtID_DEN_TrackerMuons
/// -- Isolation: NUM_TightRelTkIso_DEN_HighPtIDandIPCut
/// -- Trigger: NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose (already
/// implemented during trigger matching)
auto EventAnalyzer::set_muon_SFs(Outputs &outputs,                    //
                                 const Corrector &muon_sf_reco,       //
                                 const Corrector &muon_sf_id_low_pt,  //
                                 const Corrector &muon_sf_id_high_pt, //
                                 const Corrector &muon_sf_iso_low_pt, //
                                 const Corrector &muon_sf_iso_high_pt) -> EventAnalyzer &
{
    if (*this)
    {
        RVec<float> good_muons_pt = muons.pt[muons.good_muons_mask["nominal"]];
        RVec<float> good_muons_pt_low_pt = muons.pt[muons.good_low_pt_muons_mask["nominal"]];
        RVec<float> good_muons_pt_high_pt = muons.pt[muons.good_high_pt_muons_mask["nominal"]];
        RVec<float> good_muons_eta = VecOps::abs(muons.eta[muons.good_muons_mask["nominal"]]);
        RVec<float> good_muons_eta_low_pt = VecOps::abs(muons.eta[muons.good_low_pt_muons_mask["nominal"]]);
        RVec<float> good_muons_eta_high_pt = VecOps::abs(muons.eta[muons.good_high_pt_muons_mask["nominal"]]);

        // fmt::print("[ DEBUG ] =========================================================\n");
        // fmt::print("[ DEBUG ] Raw Muons: {}\n", muons.pt);
        // fmt::print("[ DEBUG ] good_muons_pt_low_pt: {}\n", good_muons_pt_low_pt);
        // fmt::print("[ DEBUG ] good_muons_pt_high_pt: {}\n", good_muons_pt_high_pt);

        // Muon Reco
        outputs.set_event_weight("MuonReco", "Nominal", muon_sf_reco(year, good_muons_pt, good_muons_eta, "sf"));
        outputs.set_event_weight("MuonReco", "Up", muon_sf_reco(year, good_muons_pt, good_muons_eta, "systup"));
        outputs.set_event_weight("MuonReco", "Down", muon_sf_reco(year, good_muons_pt, good_muons_eta, "systdown"));

        // Muon Id
        outputs.set_event_weight("MuonId",
                                 "Nominal",
                                 muon_sf_id_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "sf") *
                                     muon_sf_id_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "sf"));
        outputs.set_event_weight("MuonId",
                                 "Up",
                                 muon_sf_id_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "systup") *
                                     muon_sf_id_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "systup"));
        outputs.set_event_weight(
            "MuonId",
            "Down",
            muon_sf_id_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "systdown") *
                muon_sf_id_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "systdown"));

        // Muon Iso
        outputs.set_event_weight("MuonIso",
                                 "Nominal",
                                 muon_sf_iso_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "sf") *
                                     muon_sf_iso_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "sf"));
        outputs.set_event_weight(
            "MuonIso",
            "Up",
            muon_sf_iso_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "systup") *
                muon_sf_iso_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "systup"));
        outputs.set_event_weight(
            "MuonIso",
            "Down",
            muon_sf_iso_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "systdown") *
                muon_sf_iso_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "systdown"));

        return *this;
    }
    return *this;
}
