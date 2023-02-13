#include "EventAnalyzer.hpp"

auto EventAnalyzer::set_l1_pre_firing_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        // L1 prefiring
        outputs.set_event_weight("L1PreFiring", "Nominal", event_info.L1PreFiringWeight_Nom);
        outputs.set_event_weight("L1PreFiring", "Up", event_info.L1PreFiringWeight_Up);
        outputs.set_event_weight("L1PreFiring", "Down", event_info.L1PreFiringWeight_Dn);

        return *this;
    }
    return *this;
}

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

        RVec<float> good_muons_pt = muons.pt[good_muons_mask];
        RVec<float> good_muons_pt_low_pt = muons.pt[good_low_pt_muons_mask];
        RVec<float> good_muons_pt_high_pt = muons.pt[good_high_pt_muons_mask];
        RVec<float> good_muons_eta = VecOps::abs(muons.eta[good_muons_mask]);
        RVec<float> good_muons_eta_low_pt = VecOps::abs(muons.eta[good_low_pt_muons_mask]);
        RVec<float> good_muons_eta_high_pt = VecOps::abs(muons.eta[good_high_pt_muons_mask]);

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

/////////////////////////////////////////////////////////////////////////////////////////
/// Electron  SFs, in the correctionlib JSONs, are implemented in a single key: UL-Electron-ID-SF
/// inputs: year (string), variation (string), WorkingPoint (string), eta_SC (real), pt (real)
/// Examples:
/// - year: 2016preVFP, 2016postVFP, 2017, 2018
/// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
/// - WorkingPoint: Loose, Medium, RecoAbove20, RecoBelow20, Tight, Veto, wp80iso, wp80noiso, wp90iso, wp90noiso
/// - eta: [-inf, inf)
/// - pt [10., inf)
///
/// Low pT
/// RECO: RecoAbove20
/// ID: Tight
/// ISO: No recomendations (already incorporated).
///
/// TODO: High Pt - Doesn't use the Correctionlib
/// RECO: Same as Low Pt.
/// ID: Example:
/// https://github.com/CMSLQ/rootNtupleAnalyzerV2/blob/2dd8f9415e7a9c3465c7e28916eb68866ff337ff/src/ElectronScaleFactors.C
/// 2016 prompt: 0.971±0.001 (stat) (EB), 0.983±0.001 (stat) (EE)
///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
///              uncertainty (syst?): EE ET < 90 GeV: 1% else min(1+(ET-90)*0.0143)%,4%)
///
/// 2016 legacy: 0.983±0.000 (stat) (EB), 0.991±0.001 (stat) (EE) (taken from slide 10 of [0])
///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
///              uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)
///
/// 2017 prompt: 0.968±0.001 (stat) (EB), 0.973±0.002 (stat) (EE)
///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
///              uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)
///
/// 2018 rereco (Autumn 18): 0.969 +/- 0.000 (stat) (EB), and 0.984 +/- 0.001 (stat) (EE).
///                          uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
///                          uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)

/// For more details see here https://twiki.cern.ch/twiki/bin/view/CMS/HEEPElectronIdentificationRun2#Scale_Factor.
/// As always, HEEP ID SF are just two numbers, one for EB and one for EE.
///
/// [0] -
/// https://indico.cern.ch/event/831669/contributions/3485543/attachments/1871797/3084930/ApprovalSlides_EE_v3.pdf

auto EventAnalyzer::set_electron_SFs(Outputs &outputs, const ElectronSFCorrector &electron_sf) -> EventAnalyzer &
{
    if (*this)
    {
        RVec<float> good_electrons_pt = electrons.pt[good_electrons_mask];
        RVec<float> good_electrons_pt_low_pt = electrons.pt[good_low_pt_electrons_mask];
        RVec<float> good_electrons_pt_high_pt = electrons.pt[good_high_pt_electrons_mask];

        RVec<float> good_electrons_eta_sc = (electrons.eta + electrons.deltaEtaSC)[good_electrons_mask];
        RVec<float> good_electrons_eta_sc_low_pt = (electrons.eta + electrons.deltaEtaSC)[good_low_pt_electrons_mask];
        RVec<float> good_electrons_eta_sc_high_pt = (electrons.eta + electrons.deltaEtaSC)[good_high_pt_electrons_mask];

        // Electron Reco
        outputs.set_event_weight(
            "ElectronReco", "Nominal", electron_sf("sf", "RecoAbove20", good_electrons_pt, good_electrons_eta_sc));
        outputs.set_event_weight(
            "ElectronReco", "Up", electron_sf("sfup", "RecoAbove20", good_electrons_pt, good_electrons_eta_sc));
        outputs.set_event_weight(
            "ElectronReco", "Down", electron_sf("sfup", "RecoAbove20", good_electrons_pt, good_electrons_eta_sc));

        // Electron ID
        outputs.set_event_weight(
            "ElectronId",
            "Nominal",
            electron_sf("sf", "Tight", good_electrons_pt_low_pt, good_electrons_eta_sc_low_pt) *
                electron_sf("sf", "HEEPId", good_electrons_pt_high_pt, good_electrons_eta_sc_high_pt));
        outputs.set_event_weight(
            "ElectronId",
            "Up",
            electron_sf("sfup", "Tight", good_electrons_pt_low_pt, good_electrons_eta_sc_low_pt) *
                electron_sf("sfup", "HEEPId", good_electrons_pt_high_pt, good_electrons_eta_sc_high_pt));
        outputs.set_event_weight(
            "ElectronId",
            "Down",
            electron_sf("sfdown", "Tight", good_electrons_pt_low_pt, good_electrons_eta_sc_low_pt) *
                electron_sf("sfdown", "HEEPId", good_electrons_pt_high_pt, good_electrons_eta_sc_high_pt));

        // return modified event_data
        return *this;
    }
    return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Photons ID SFs, in the correctionlib JSONs, are implemented in: UL-Photon-ID-SF
/// inputs: year (string), variation (string), WorkingPoint (string), eta_SC (real), pt (real)
/// - year: 2016preVFP, 2016postVFP, 2017, 2018
/// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
/// - WorkingPoint: Loose, Medium, Tight, wp80, wp90
/// - eta: [-inf, inf)
/// - pt [20., inf)
///
/// Low pT
/// RECO: From Twiki [0]: "The scale factor to reconstruct a supercluster with H/E<0.5 is assumed to be 100%."
/// ID: Tight
/// ISO: No recomendations (already incorporated).
///
/// [0] - https://twiki.cern.ch/twiki/bin/view/CMS/EgammaRunIIRecommendations#E_gamma_RECO
///
/// Photons PixelSeed SFs, in the correctionlib JSONs, are implemented in:  UL-Photon-PixVeto-SF
/// These are the Photon Pixel Veto Scale Factors (nominal, up or down) for 2018 Ultra Legacy dataset.
/// - year: 2016preVFP, 2016postVFP, 2017, 2018
/// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
/// - WorkingPoint (SFs available for the cut-based and MVA IDs): Loose, MVA, Medium, Tight
/// - HasPixBin: For each working point of choice, they are dependent on the photon pseudorapidity and R9: Possible
/// bin choices: ['EBInc','EBHighR9','EBLowR9','EEInc','EEHighR9','EELowR9']
auto EventAnalyzer::set_photon_SFs(Outputs &outputs,
                                   const PhotonSFCorrector &photon_id_sf,
                                   const PhotonSFCorrector &photon_pixel_seed_sf) -> EventAnalyzer &
{
    if (*this)
    {
        RVec<float> good_photons_pt = photons.pt[good_photons_mask];
        RVec<float> good_photons_eta = photons.eta[good_photons_mask];

        // PhotonId
        outputs.set_event_weight("PhotonId", "Nominal", photon_id_sf("sf", good_photons_pt, good_photons_eta));
        outputs.set_event_weight("PhotonId", "Up", photon_id_sf("sfup", good_photons_pt, good_photons_eta));
        outputs.set_event_weight("PhotonId", "Down", photon_id_sf("sfup", good_photons_pt, good_photons_eta));

        // Photon PixelSeed
        // here the pt RVec is needed just to get the number of photons.
        // the PixelSeed SF is independent of pt and eta
        outputs.set_event_weight("PhotonPixelSeed", "Nominal", photon_pixel_seed_sf("sf", good_photons_pt));
        outputs.set_event_weight("PhotonPixelSeed", "Up", photon_pixel_seed_sf("sfup", good_photons_pt));
        outputs.set_event_weight("PhotonPixelSeed", "Down", photon_pixel_seed_sf("sfup", good_photons_pt));

        return *this;
    }
    return *this;
}

/// TODO: Taus
auto EventAnalyzer::set_tau_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// Jets
/// No SFs are assigned to Jets. They have been measured to be close to 1.
auto EventAnalyzer::set_jet_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

////////////////////////////////////////////////////////
/// BTagging
/// Using Method 1A - Per event weight
/// References:
/// - https://twiki.cern.ch/twiki/bin/view/CMS/BTagSFMethods#1a_Event_reweighting_using_scale
/// - https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideCMSDataAnalysisSchoolLPC2023TaggingExercise
/// - https://github.com/IreneZoi/CMSDAS2023-BTV/tree/master/BTaggingExercise
/// - https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation#UltraLegacy_scale_factor_uncerta
///
/// systematic (string): central, down, down_correlated, down_uncorrelated, up, up_correlated,
/// working_point (string): L, M, T
/// flavor (int): 5=b, 4=c, 0=udsg
/// abseta (real)
/// pt (real)
/// Official instructions on systematics:
/// - Simple "up" and "down" uncertainties are only to be used when one single data era is analyzed
/// - A breakdown of SFbc and SFlight uncertainties into "up/down_correlated/uncorrelated" is to be used when the
/// fullRunII dataset is analyzed. The "uncorrelated" uncertainties are to be decorrelated between years, and the
/// "correlated" uncertainties are to be correlated between years With this scheme you should have 10 uncertainties
/// - related to the b-tagging SFs in the end:
///- btagSFbc_correlated
///- btagSFlight_correlated
///- btagSFbc_2018
///- btagSFlight_2018
///- btagSFbc_2017
///- btagSFlight_2017
///- btagSFbc_2016postVFP
///- btagSFlight_2016postVFP
///- btagSFbc_2016preVFP
///- btagSFlight_2016preVFP
auto EventAnalyzer::set_bjet_SFs(Outputs &outputs, const BTagSFCorrector &btag_sf) -> EventAnalyzer &
{
    if (*this)
    {
        RVec<float> good_bjets_pt = bjets.pt[good_bjets_mask];
        RVec<float> good_bjets_abseta = VecOps::abs(bjets.eta[good_bjets_mask]);
        RVec<float> good_bjets_hadronFlavour = bjets.hadronFlavour[good_bjets_mask];

        RVec<float> good_jets_pt = jets.pt[good_jets_mask];
        RVec<float> good_jets_abseta = VecOps::abs(jets.eta[good_jets_mask]);
        RVec<float> good_jets_hadronFlavour = jets.hadronFlavour[good_jets_mask];

        // BJetCorrelated
        outputs.set_event_weight("BJetCorrelated",
                                 "Nominal",
                                 btag_sf("central",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("BJetCorrelated",
                                 "Up",
                                 btag_sf("up_correlated",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("BJetCorrelated",
                                 "Down",
                                 btag_sf("down_correlated",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));

        // LightJetCorrelated
        outputs.set_event_weight("LightJetCorrelated",
                                 "Nominal",
                                 btag_sf("central",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("LightJetCorrelated",
                                 "Up",
                                 btag_sf("central",
                                         "up_correlated",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("LightJetCorrelated",
                                 "Down",
                                 btag_sf("central",
                                         "down_correlated",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));

        // BJetUncorrelated
        outputs.set_event_weight("BJetUncorrelated",
                                 "Nominal",
                                 btag_sf("central",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("BJetUncorrelated",
                                 "Up",
                                 btag_sf("up_uncorrelated",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("BJetUncorrelated",
                                 "Down",
                                 btag_sf("down_uncorrelated",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));

        // LightJetUncorrelated
        outputs.set_event_weight("LightJetUncorrelated",
                                 "Nominal",
                                 btag_sf("central",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("LightJetUncorrelated",
                                 "Up",
                                 btag_sf("central",
                                         "up_uncorrelated",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("LightJetUncorrelated",
                                 "Down",
                                 btag_sf("central",
                                         "down_uncorrelated",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));

        return *this;
    }
    return *this;
}

/// MET
/// MET is present in, virtually, all events. It is not possible to assign SFs.
auto EventAnalyzer::set_met_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// Trigger
/// These weights have already been calculated during the trigger matching.
auto EventAnalyzer::set_trigger_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        outputs.set_event_weight("Trigger", "Nominal", trigger_sf_nominal);
        outputs.set_event_weight("Trigger", "Up", trigger_sf_up);
        outputs.set_event_weight("Trigger", "Down", trigger_sf_down);
        return *this;
    }
    return *this;
}