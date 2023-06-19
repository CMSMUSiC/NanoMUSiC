#include "EventAnalyzer.hpp"

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
        RVec<float> good_electrons_pt = electrons.pt[electrons.good_electrons_mask["nominal"]];
        RVec<float> good_electrons_pt_low_pt = electrons.pt[electrons.good_low_pt_electrons_mask["nominal"]];
        RVec<float> good_electrons_pt_high_pt = electrons.pt[electrons.good_high_pt_electrons_mask["nominal"]];

        RVec<float> good_electrons_eta_sc =
            (electrons.eta + electrons.deltaEtaSC)[electrons.good_electrons_mask["nominal"]];
        RVec<float> good_electrons_eta_sc_low_pt =
            (electrons.eta + electrons.deltaEtaSC)[electrons.good_low_pt_electrons_mask["nominal"]];
        RVec<float> good_electrons_eta_sc_high_pt =
            (electrons.eta + electrons.deltaEtaSC)[electrons.good_high_pt_electrons_mask["nominal"]];

        // Electron Reco
        outputs.set_event_weight(
            "ElectronReco", "Nominal", electron_sf("sf", "RecoAbove20", good_electrons_pt, good_electrons_eta_sc));
        outputs.set_event_weight(
            "ElectronReco", "Up", electron_sf("sfup", "RecoAbove20", good_electrons_pt, good_electrons_eta_sc));
        outputs.set_event_weight(
            "ElectronReco", "Down", electron_sf("sfdown", "RecoAbove20", good_electrons_pt, good_electrons_eta_sc));

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
        RVec<float> good_photons_pt = photons.pt[photons.good_photons_mask["nominal"]];
        RVec<float> good_photons_eta = photons.eta[photons.good_photons_mask["nominal"]];

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
