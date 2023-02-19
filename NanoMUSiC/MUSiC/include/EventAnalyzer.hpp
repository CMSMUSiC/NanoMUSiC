#ifndef MUSIC_event_analyzer
#define MUSIC_event_analyzer

#include <algorithm>
#include <complex>
#include <exception>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string_view>

#include <fmt/format.h>

#include "LHAPDF/PDF.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "GeneratorFilters.hpp"
#include "JetCorrector.hpp"
#include "NanoObjects.hpp"
#include "Outputs.hpp"
#include "RunLumiFilter.hpp"
#include "Trigger.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;

class EventAnalyzer
{
  private:
    bool is_null = true;

  public:
    TriggerBits trigger_bits;
    float trigger_sf_nominal = 1.;
    float trigger_sf_up = 1.;
    float trigger_sf_down = 1.;

    NanoObjects::EventInfo event_info;

    NanoObjects::GeneratorInfo generator_info;

    NanoObjects::LHEInfo lhe_info;
    int lha_id;
    float alpha_s_up = 1.;
    float alpha_s_down = 1.;
    float scale_envelope_weight_up = 1.;
    float scale_envelope_weight_down = 1.;

    NanoObjects::GenParticles gen_particles;

    NanoObjects::LHEParticles lhe_particles;

    NanoObjects::GenJets gen_jets;

    NanoObjects::Muons muons;
    RVec<int> good_muons_mask;
    RVec<int> good_low_pt_muons_mask;
    RVec<int> good_high_pt_muons_mask;

    NanoObjects::Electrons electrons;
    RVec<int> good_electrons_mask;
    RVec<int> good_low_pt_electrons_mask;
    RVec<int> good_high_pt_electrons_mask;

    NanoObjects::Photons photons;
    RVec<int> good_photons_mask;

    NanoObjects::Taus taus;
    RVec<int> good_taus_mask;

    NanoObjects::BJets bjets;
    RVec<int> good_bjets_mask;

    NanoObjects::Jets jets;
    RVec<int> good_jets_mask;

    NanoObjects::MET met;
    RVec<int> good_met_mask;

    NanoObjects::TrgObjs trgobjs;
    RVec<int> good_trgobjs_mask;

    bool is_data = true;
    Year year = Year::kTotalYears;

    EventAnalyzer(const bool &_is_data, const Year &_year, Outputs &outputs);

    // builder interface
    auto set_event_info(NanoObjects::EventInfo &&_event_info) -> EventAnalyzer &;
    auto set_generator_info(NanoObjects::GeneratorInfo &&_generator_info) -> EventAnalyzer &;
    auto set_lhe_info(NanoObjects::LHEInfo &&_lhe_info) -> EventAnalyzer &;
    // auto set_gen_particles(NanoObjects::GenParticles &&_gen_particles) -> EventAnalyzer &;
    auto set_lhe_particles(NanoObjects::LHEParticles &&_lhe_particles) -> EventAnalyzer &;
    auto set_gen_jets(NanoObjects::GenJets &&_gen_jets) -> EventAnalyzer &;
    auto set_muons(NanoObjects::Muons &&_muons, RVec<int> &&mask) -> EventAnalyzer &;
    auto set_muons(NanoObjects::Muons &&_muons) -> EventAnalyzer &;
    auto set_electrons(NanoObjects::Electrons &&_electrons, RVec<int> &&mask) -> EventAnalyzer &;
    auto set_electrons(NanoObjects::Electrons &&_electrons) -> EventAnalyzer &;
    auto set_photons(NanoObjects::Photons &&_photons, RVec<int> &&mask) -> EventAnalyzer &;
    auto set_photons(NanoObjects::Photons &&_photons) -> EventAnalyzer &;
    auto set_taus(NanoObjects::Taus &&_taus, RVec<int> &&mask) -> EventAnalyzer &;
    auto set_taus(NanoObjects::Taus &&_taus) -> EventAnalyzer &;
    auto set_bjets(NanoObjects::BJets &&_bjets, RVec<int> &&mask) -> EventAnalyzer &;
    auto set_bjets(NanoObjects::BJets &&_bjets) -> EventAnalyzer &;
    auto set_jets(NanoObjects::Jets &&_jets, RVec<int> &&mask) -> EventAnalyzer &;
    auto set_jets(NanoObjects::Jets &&_jets) -> EventAnalyzer &;
    auto set_met(NanoObjects::MET &&_met, RVec<int> &&mask) -> EventAnalyzer &;
    auto set_met(NanoObjects::MET &&_met) -> EventAnalyzer &;
    auto set_trgobjs(NanoObjects::TrgObjs &&_trgobjs, RVec<int> &&mask) -> EventAnalyzer &;
    auto set_trgobjs(NanoObjects::TrgObjs &&_trgobjs) -> EventAnalyzer &;

    ///////////////////////////////////////////////////////////////////////////////////
    /// is it a null event
    operator bool() const
    {
        return !is_null;
    }

    ///////////////////////////////////////////////////////////////////////////////////
    /// null-ify the event
    auto set_null() -> void;

    ///////////////////////////////////////////////////////////////////////////////////
    /// un-null-ify - not sure when/if it would be needed, but ...
    auto unset_null() -> void;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set PDF and Alpha_S uncertainties.
    /// Those are tricky beasts, since they are not simple weights added to the event, but rather, should be treated as
    /// variations and have their uncert. squared-summed in the end of the processing (classification).
    /// This method also saves the LHA ID that was used during generation or rescaling.
    auto set_pdf_alpha_s_weights(const std::optional<std::pair<unsigned int, unsigned int>> &lha_indexes,
                                 const std::tuple<std::vector<std::unique_ptr<LHAPDF::PDF>>,
                                                  std::unique_ptr<LHAPDF::PDF>,
                                                  std::unique_ptr<LHAPDF::PDF>> &default_pdf_sets) -> EventAnalyzer &;

    ////////////////////////////////////////////////////////////////////////////////////
    /// Set the QCD Scaling weights, using the envelope method. If the sample has no weights are kept as 1.
    auto set_scale_weights() -> EventAnalyzer &;

    /////////////////////////////////////////////////////////////////////////////////////////////
    /// set generator weight
    /// should be called before any EventAnalyzer method, but only after all weights are available (should wait for PDF
    /// and QCD Scale weights). The naming constant weights means weights that are the sample for the whole event, but
    /// could differ from one event to another, e.g. pile-up. As a negative example, Muons resolution corretions are not
    /// constants, within the whole event. Weights that are related to physical objects (e.g.: muon SFs) are set later,
    /// if the event pass the selection.
    auto set_const_weights(Outputs &outputs, Corrector &pu_weight) -> EventAnalyzer &;

    ////////////////////////////////////////////////////////////////////////////
    /// TODO: Filter events based on their Generator process. This is implemented in order to avoid overlap of
    /// phase-space between MC samples. Should come after all constant weights are available.
    auto generator_filter(Outputs &outputs, const std::string &process) -> EventAnalyzer &;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Filter events based on their lumi sectiona and run numbers, following the recommendations from the LUMI-POG (aka
    /// "golden JSON").
    auto run_lumi_filter(Outputs &outputs, const RunLumiFilter &_run_lumi_filter) -> EventAnalyzer &;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Apply good primary vertex filters.
    auto npv_filter(Outputs &outputs) -> EventAnalyzer &;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Apply JEMMET-POG recommendations on calorimeter detection quality.
    auto met_filter(Outputs &outputs) -> EventAnalyzer &;

    //////////////////////////////////////////////////////////
    /// Set `TriggerBits` for this event. They will be checked latter.
    // Clear `TriggerBits` and save the seed trigger.
    // Will loop over the `TriggerBits`. Once a fired trigger is found:
    //  1 - the fired trigger bit is saved as trigger_seed
    //  2 - all others bits are set to false
    ///
    auto set_trigger_bits() -> EventAnalyzer &;

    //////////////////////////////////////////////////////////////
    /// Filter events that did not fired any trigger or do not pass double trigger firing check
    ///
    auto trigger_filter(Outputs &outputs) -> EventAnalyzer &;
    // Low pT muon filter
    auto get_low_pt_muons_selection_mask() -> RVec<int>;

    // High pT muon filter
    auto get_high_pt_muons_selection_mask() -> RVec<int>;

    // Low pT Electrons
    auto get_low_pt_electrons_selection_mask() -> RVec<int>;

    // High pT Electrons
    auto get_high_pt_electrons_selection_mask() -> RVec<int>;

    // Photons
    auto get_photons_selection_mask() -> RVec<int>;

    // Taus
    auto get_taus_selection_mask() -> RVec<int>;

    // BJets
    auto get_bjets_selection_mask() -> RVec<int>;

    // Jets
    auto get_jets_selection_mask() -> RVec<int>;

    // MET
    auto get_met_selection_mask() -> RVec<int>;

    //////////////////////////////////////////////////////////////////
    /// Will clean (eta, phi) against it self
    /// Returns a mask, such that, if two distinct objects they match in DeltaR, one should be filtered.
    /// The callable `filter` will receive the objects being self-cleanned and the indexes (`i` and `j`) of the tested
    /// objects. It should return `true` to keep the i-th object or `false` to remove it.
    template <typename T, typename F>
    auto get_self_cleanning_mask(const T objects, RVec<int> mask, float max_delta_r, F &&filter) -> RVec<int>
    {
        auto cleanning_mask = RVec<int>(objects.size, 1);

        if (objects.size >= 2)
        {
            for (std::size_t i = 0; i < objects.size; i++)
            {
                if (mask[i])
                {
                    auto cleanning_result = true;
                    for (std::size_t j = 0; j < objects.eta.size(); j++)
                    {
                        if (i != j)
                        {
                            float delta_phi = VecOps::DeltaPhi(objects.phi[i], objects.phi[i]);
                            float delta_eta = std::abs(objects.eta[i] - objects.eta[i]);
                            float delta_r = std::sqrt(delta_phi * delta_phi + delta_eta * delta_eta);
                            if (delta_r < max_delta_r)
                            {
                                cleanning_result = cleanning_result && filter(objects, i, j);
                            }
                            if (cleanning_result == false)
                            {
                                break;
                            }
                        }
                    }
                    cleanning_mask[i] = static_cast<int>(cleanning_result);
                }
            }
        }
        return cleanning_mask;
    }

    //////////////////////////////////////////////////////////////////
    /// Will clean (eta1, phi1) x (eta2, phi2).
    /// Returns a mask, such that, if they match in DeltaR, object2 in filtered.
    /// (This means that the mask should be applied on object2)
    template <typename T1, typename T2>
    auto get_cross_cleanning_mask(const T1 &obj1, const T2 &obj2, const RVec<int> &mask, float max_delta_r) -> RVec<int>
    {
        auto cleanning_mask = RVec<int>(obj1.eta.size(), 1);

        if (obj1.eta.size() >= 1 and obj2.eta.size() >= 1)
        {
            for (std::size_t i = 0; i < obj1.eta.size(); i++)
            {
                if (mask[i])
                {
                    auto delta_phi = VecOps::DeltaPhi(obj1.phi[i], obj2.phi);
                    auto delta_eta = VecOps::abs(obj1.eta[i] - obj2.eta);
                    auto delta_r = VecOps::sqrt(delta_phi * delta_phi + delta_eta * delta_eta);
                    cleanning_mask[i] = static_cast<int>(not(VecOps::Any(delta_r < max_delta_r)));
                }
            }
        }

        return cleanning_mask;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Fill masks in order to select objects.
    // ATTENTION: Care should be taken to do not forget to merge (AND operation) all different masks per object. It
    // is need in order to filter out events that have no objects selected.
    auto object_selection() -> EventAnalyzer &;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Returns `true` if the event has at least one object selected.
    auto has_selected_objects_filter(Outputs &outputs) -> EventAnalyzer &;

    /////////////////////////////////////////////////////////////////////
    /// Will check if the current event has a matched object to a good TrgObj
    // here we have to break the single responsability rule ...
    // this filter also get the trigger scale factor,
    // otherwise we would have to look over the objects twice
    ///
    auto trigger_match_filter(Outputs &outputs, const std::map<std::string_view, TrgObjMatcher> &matchers)
        -> EventAnalyzer &;
    auto set_l1_pre_firing_SFs(Outputs &outputs) -> EventAnalyzer &;

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
    auto set_muon_SFs(Outputs &outputs,                    //
                      const Corrector &muon_sf_reco,       //
                      const Corrector &muon_sf_id_low_pt,  //
                      const Corrector &muon_sf_id_high_pt, //
                      const Corrector &muon_sf_iso_low_pt, //
                      const Corrector &muon_sf_iso_high_pt) -> EventAnalyzer &;

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

    auto set_electron_SFs(Outputs &outputs, const ElectronSFCorrector &electron_sf) -> EventAnalyzer &;

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
    auto set_photon_SFs(Outputs &outputs,
                        const PhotonSFCorrector &photon_id_sf,
                        const PhotonSFCorrector &photon_pixel_seed_sf) -> EventAnalyzer &;

    /// TODO: Taus
    auto set_tau_SFs(Outputs &outputs) -> EventAnalyzer &;

    /// Jets
    /// No SFs are assigned to Jets. They have been measured to be close to 1.
    auto set_jet_SFs(Outputs &outputs) -> EventAnalyzer &;

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
    auto set_bjet_SFs(Outputs &outputs, const BTagSFCorrector &btag_sf) -> EventAnalyzer &;

    /// MET
    /// MET is present in, virtually, all events. It is not possible to assign SFs.
    auto set_met_SFs(Outputs &outputs) -> EventAnalyzer &;

    /// Trigger
    /// These weights have already been calculated during the trigger matching.
    auto set_trigger_SFs(Outputs &outputs) -> EventAnalyzer &;

    /// TODO:
    auto transform_muons() -> EventAnalyzer &;

    /// TODO:
    auto transform_electrons() -> EventAnalyzer &;

    /// TODO:
    auto transform_photons() -> EventAnalyzer &;

    /// TODO:
    auto transform_taus() -> EventAnalyzer &;

    template <typename T>
    auto jet_transformer(T &_jets, RVec<int> &_mask, JetCorrector &jet_corrections) -> void
    {
        // Nominal
        auto dX_nominal = RVec<float>(_jets.size);
        auto dY_nominal = RVec<float>(_jets.size);
        auto new_pt_nominal = _jets.pt;
        auto new_mass_nominal = _jets.mass;

        // JES: Up - JER: Nominal
        auto dX_up_nominal = RVec<float>(_jets.size);
        auto dY_up_nominal = RVec<float>(_jets.size);
        auto new_pt_up_nominal = _jets.pt;
        auto new_mass_up_nominal = _jets.mass;

        //  JES: Up - JER: Nominal
        auto dX_down_nominal = RVec<float>(_jets.size);
        auto dY_down_nominal = RVec<float>(_jets.size);
        auto new_pt_down_nominal = _jets.pt;
        auto new_mass_down_nominal = _jets.mass;

        // JES: Nominal - JER: Up
        auto dX_nominal_up = RVec<float>(_jets.size);
        auto dY_nominal_up = RVec<float>(_jets.size);
        auto new_pt_nominal_up = _jets.pt;
        auto new_mass_nominal_up = _jets.mass;

        //  JES: Nominal - JER: Down
        auto dX_nominal_down = RVec<float>(_jets.size);
        auto dY_nominal_down = RVec<float>(_jets.size);
        auto new_pt_nominal_down = _jets.pt;
        auto new_mass_nominal_down = _jets.mass;

        for (std::size_t i = 0; i < _jets.size; i++)
        {
            if (_mask[i] == 1)
            { // JES: Nominal - JER: Nominal
                float scale_correction_nominal = jet_corrections.get_scale_correction(_jets.pt[i],
                                                                                      _jets.eta[i],
                                                                                      _jets.phi[i],
                                                                                      _jets.rawFactor[i],
                                                                                      event_info.rho,
                                                                                      _jets.area[i],
                                                                                      "Nominal"s);

                new_pt_nominal[i] = _jets.pt[i] * scale_correction_nominal;

                float resolution_correction_nominal = jet_corrections.get_resolution_correction(new_pt_nominal[i],
                                                                                                _jets.eta[i],
                                                                                                _jets.phi[i],
                                                                                                event_info.rho,
                                                                                                _jets.genJetIdx[i],
                                                                                                gen_jets,
                                                                                                "Nominal"s);

                new_pt_nominal[i] = _jets.pt[i] * scale_correction_nominal * resolution_correction_nominal;
                new_mass_nominal[i] = _jets.mass[i] * scale_correction_nominal * resolution_correction_nominal;
                dX_nominal[i] = (new_pt_nominal[i] - _jets.pt[i]) * std::cos(_jets.phi[i]);
                dY_nominal[i] = (new_pt_nominal[i] - _jets.pt[i]) * std::sin(_jets.phi[i]);

                // JES: Up - JER: Nominal
                auto scale_correction_up = jet_corrections.get_scale_correction(
                    _jets.pt[i], _jets.eta[i], _jets.phi[i], _jets.rawFactor[i], event_info.rho, _jets.area[i], "Up"s);

                new_pt_up_nominal[i] = _jets.pt[i] * scale_correction_up;

                resolution_correction_nominal = jet_corrections.get_resolution_correction(new_pt_up_nominal[i],
                                                                                          _jets.eta[i],
                                                                                          _jets.phi[i],
                                                                                          event_info.rho,
                                                                                          _jets.genJetIdx[i],
                                                                                          gen_jets,
                                                                                          "Nominal"s);

                new_pt_up_nominal[i] = _jets.pt[i] * scale_correction_up * resolution_correction_nominal;
                new_mass_up_nominal[i] = _jets.mass[i] * scale_correction_up * resolution_correction_nominal;
                dX_up_nominal[i] = (new_pt_up_nominal[i] - _jets.pt[i]) * std::cos(_jets.phi[i]);
                dY_up_nominal[i] = (new_pt_up_nominal[i] - _jets.pt[i]) * std::sin(_jets.phi[i]);

                // JES: Down - JER: Nominal
                auto scale_correction_down = jet_corrections.get_scale_correction(_jets.pt[i],
                                                                                  _jets.eta[i],
                                                                                  _jets.phi[i],
                                                                                  _jets.rawFactor[i],
                                                                                  event_info.rho,
                                                                                  _jets.area[i],
                                                                                  "Down"s);

                new_pt_down_nominal[i] = _jets.pt[i] * scale_correction_down;

                resolution_correction_nominal = jet_corrections.get_resolution_correction(new_pt_down_nominal[i],
                                                                                          _jets.eta[i],
                                                                                          _jets.phi[i],
                                                                                          event_info.rho,
                                                                                          _jets.genJetIdx[i],
                                                                                          gen_jets,
                                                                                          "Nominal"s);

                new_pt_down_nominal[i] = _jets.pt[i] * scale_correction_down * resolution_correction_nominal;
                new_mass_down_nominal[i] = _jets.mass[i] * scale_correction_down * resolution_correction_nominal;
                dX_down_nominal[i] = (new_pt_down_nominal[i] - _jets.pt[i]) * std::cos(_jets.phi[i]);
                dY_down_nominal[i] = (new_pt_down_nominal[i] - _jets.pt[i]) * std::sin(_jets.phi[i]);

                // JES: Nominal - JER: Up
                new_pt_nominal_up[i] = _jets.pt[i] * scale_correction_nominal;

                float resolution_correction_up = jet_corrections.get_resolution_correction(new_pt_nominal_up[i],
                                                                                           _jets.eta[i],
                                                                                           _jets.phi[i],
                                                                                           event_info.rho,
                                                                                           _jets.genJetIdx[i],
                                                                                           gen_jets,
                                                                                           "Up"s);

                new_pt_nominal_up[i] = _jets.pt[i] * scale_correction_nominal * resolution_correction_up;
                new_mass_nominal_up[i] = _jets.mass[i] * scale_correction_nominal * resolution_correction_up;
                dX_nominal_up[i] = (new_pt_nominal_up[i] - _jets.pt[i]) * std::cos(_jets.phi[i]);
                dY_nominal_up[i] = (new_pt_nominal_up[i] - _jets.pt[i]) * std::sin(_jets.phi[i]);

                // JES: Nominal - JER: Down
                new_pt_nominal_down[i] = _jets.pt[i] * scale_correction_nominal;

                float resolution_correction_down = jet_corrections.get_resolution_correction(new_pt_nominal_down[i],
                                                                                             _jets.eta[i],
                                                                                             _jets.phi[i],
                                                                                             event_info.rho,
                                                                                             _jets.genJetIdx[i],
                                                                                             gen_jets,
                                                                                             "Down"s);

                new_pt_nominal_down[i] = _jets.pt[i] * scale_correction_nominal * resolution_correction_down;
                new_mass_nominal_down[i] = _jets.mass[i] * scale_correction_nominal * resolution_correction_down;
                dX_nominal_down[i] = (new_pt_nominal_down[i] - _jets.pt[i]) * std::cos(_jets.phi[i]);
                dY_nominal_down[i] = (new_pt_nominal_down[i] - _jets.pt[i]) * std::sin(_jets.phi[i]);
            }
        }

        // set new values
        // nominal
        _jets.pt_nominal = new_pt_nominal;
        _jets.mass_nominal = new_mass_nominal;

        // JES
        _jets.pt_JES_up = new_pt_nominal_up;
        _jets.mass_JES_up = new_mass_nominal_up;
        _jets.pt_JES_down = new_pt_nominal_down;
        _jets.mass_JES_down = new_mass_nominal_down;

        // JER
        _jets.pt_JER_up = new_pt_up_nominal;
        _jets.mass_JER_up = new_mass_up_nominal;
        _jets.pt_JER_down = new_pt_down_nominal;
        _jets.mass_JER_down = new_mass_down_nominal;

        // MET
        auto met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_up_nominal);
        auto met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_up_nominal);
        met.JES_up[0] = std::sqrt(met_x * met_x + met_y * met_y);

        met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_down_nominal);
        met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_down_nominal);
        met.JES_down[0] = std::sqrt(met_x * met_x + met_y * met_y);

        met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_nominal_up);
        met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_nominal_up);
        met.JER_up[0] = std::sqrt(met_x * met_x + met_y * met_y);

        met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_nominal_down);
        met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_nominal_down);
        met.JER_down[0] = std::sqrt(met_x * met_x + met_y * met_y);

        met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_nominal);
        met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_nominal);
        met.et_nominal[0] = std::sqrt(met_x * met_x + met_y * met_y);
    }

    /// TODO:
    auto transform_bjets_and_jets(JetCorrector &jet_corrections) -> EventAnalyzer &;

    /// TODO:
    auto transform_met() -> EventAnalyzer &;

    auto fill_event_content(Outputs &outputs) -> EventAnalyzer &;
};

#endif /*MUSIC_event_analyzer*/
