#ifndef MUSIC_EVENT_DATA
#define MUSIC_EVENT_DATA

#include <functional>

#include <fmt/core.h>

#include "Configs.hpp"
#include "NanoObjects.hpp"
#include "Outputs.hpp"
#include "RunLumiFilter.hpp"
#include "Trigger.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;

class EventData
{
  private:
    bool is_null = true;

  public:
    TriggerBits trigger_bits;
    float trigger_sf_nominal;
    float trigger_sf_up;
    float trigger_sf_down;

    NanoObjects::EventInfo event_info;

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
    std::string_view trigger_stream;

    EventData(const bool &_is_data, const Year &_year, const std::string &_trigger_stream)
        : is_null(false), is_data(_is_data), year(_year), trigger_stream(_trigger_stream)
    {
    }

    // builder interface
    EventData &set_event_info(NanoObjects::EventInfo &&_event_info)
    {
        if (*this)
        {
            event_info = _event_info;
            return *this;
        }
        return *this;
    }

    EventData &set_muons(NanoObjects::Muons &&_muons, RVec<int> &&mask)
    {
        if (*this)
        {
            muons = _muons;
            good_muons_mask = mask;
            good_low_pt_muons_mask = mask;
            good_high_pt_muons_mask = mask;
            return *this;
        }
        return *this;
    }

    EventData &set_muons(NanoObjects::Muons &&_muons)
    {
        return set_muons(std::move(_muons), RVec<int>(_muons.size, 1));
    }

    EventData &set_electrons(NanoObjects::Electrons &&_electrons, RVec<int> &&mask)
    {
        if (*this)
        {
            electrons = _electrons;
            good_electrons_mask = mask;
            good_low_pt_electrons_mask = mask;
            good_high_pt_electrons_mask = mask;
            return *this;
        }
        return *this;
    }

    EventData &set_electrons(NanoObjects::Electrons &&_electrons)
    {
        return set_electrons(std::move(_electrons), RVec<int>(_electrons.size, 1));
    }

    EventData &set_photons(NanoObjects::Photons &&_photons, RVec<int> &&mask)
    {
        if (*this)
        {
            photons = _photons;
            good_photons_mask = mask;
            return *this;
        }
        return *this;
    }

    EventData &set_photons(NanoObjects::Photons &&_photons)
    {
        return set_photons(std::move(_photons), RVec<int>(_photons.size, 1));
    }

    EventData &set_taus(NanoObjects::Taus &&_taus, RVec<int> &&mask)
    {
        if (*this)
        {
            taus = _taus;
            good_taus_mask = mask;
            return *this;
        }
        return *this;
    }

    EventData &set_taus(NanoObjects::Taus &&_taus)
    {
        return set_taus(std::move(_taus), RVec<int>(_taus.size, 1));
    }

    EventData &set_bjets(NanoObjects::BJets &&_bjets, RVec<int> &&mask)
    {
        if (*this)
        {
            bjets = _bjets;
            good_bjets_mask = mask;
            return *this;
        }
        return *this;
    }

    EventData &set_bjets(NanoObjects::BJets &&_bjets)
    {
        return set_bjets(std::move(_bjets), RVec<int>(_bjets.size, 1));
    }

    EventData &set_jets(NanoObjects::Jets &&_jets, RVec<int> &&mask)
    {
        if (*this)
        {
            jets = _jets;
            good_jets_mask = mask;
            return *this;
        }
        return *this;
    }

    EventData &set_jets(NanoObjects::Jets &&_jets)
    {
        return set_jets(std::move(_jets), RVec<int>(_jets.size, 1));
    }

    EventData &set_met(NanoObjects::MET &&_met, RVec<int> &&mask)
    {
        if (*this)
        {
            met = _met;
            good_met_mask = mask;
            return *this;
        }
        return *this;
    }

    EventData &set_met(NanoObjects::MET &&_met)
    {
        return set_met(std::move(_met), RVec<int>(_met.size, 1));
    }

    EventData &set_trgobjs(NanoObjects::TrgObjs &&_trgobjs, RVec<int> &&mask)
    {
        if (*this)
        {
            trgobjs = _trgobjs;
            good_trgobjs_mask = mask;
            return *this;
        }
        return *this;
    }

    EventData &set_trgobjs(NanoObjects::TrgObjs &&_trgobjs)
    {
        return set_trgobjs(std::move(_trgobjs), RVec<int>(_trgobjs.size, 1));
    }

    // is it a null event
    operator bool() const
    {
        return !is_null;
    }

    // null-ify the event
    void set_null()
    {
        this->is_null = true;
    }

    // un-null-ify - not sure when/if it would be needed, but ...
    void unset_null()
    {
        this->is_null = true;
    }

    // set generator weight
    // should be called before any EventData method
    EventData &set_const_weights(Outputs &outputs, Corrector &pu_weight)
    {
        if (*this)
        {
            // // fmt::print("\nDEBUG - set_const_weights");
            if (!is_data)
            {
                outputs.set_event_weight("Generator", event_info.genWeight);
                outputs.set_event_weight("PileUp", "Nominal", pu_weight({event_info.Pileup_nTrueInt, "nominal"}));
                outputs.set_event_weight("PileUp", "Up", pu_weight({event_info.Pileup_nTrueInt, "up"}));
                outputs.set_event_weight("PileUp", "Down", pu_weight({event_info.Pileup_nTrueInt, "down"}));
            }
            return *this;
        }
        return *this;
    }

    EventData &generator_filter(Outputs &outputs)
    {
        if (*this)
        {
            bool is_good_gen = true;
            // if MC
            if (!is_data)
            {
                /////////////////////////////////////////////////
                // FIXME: check if it is good gen event
                /////////////////////////////////////////////////
                if (true)
                {
                    is_good_gen = true;
                }
            }
            if (is_good_gen)
            {
                // // fmt::print("\nDEBUG - generator_filter");
                outputs.fill_cutflow_histo("NoCuts", 1.);
                outputs.fill_cutflow_histo("GeneratorWeight", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS generator FILTER");
            return *this;
        }
        return *this;
    }

    EventData &run_lumi_filter(Outputs &outputs, const RunLumiFilter &_run_lumi_filter)
    {
        if (*this)
        {
            if (_run_lumi_filter(event_info.run, event_info.lumi, is_data))
            {
                // // fmt::print("\nDEBUG - run_lumi_filter");
                outputs.fill_cutflow_histo("RunLumi", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS RUN_LUMI FILTER: {} - {}", event_info.run,
            // event_info.lumi);
            return *this;
        }
        return *this;
    }

    EventData &npv_filter(Outputs &outputs)
    {
        if (*this)
        {
            if (event_info.PV_npvsGood > 0)
            {
                // // fmt::print("\nDEBUG - PV_npvsGood");
                outputs.fill_cutflow_histo("nPV", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS n_pv FILTER");
            return *this;
        }
        return *this;
    }

    EventData &met_filter(Outputs &outputs)
    {
        if (*this)
        {
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // MET event filters
            // https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFiltersRun2#MET_Filter_Recommendations_for_R
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            bool pass_MET_filters = true;
            if (year == Year::Run2016APV || year == Year::Run2016)
            {
                // clang-format off
                pass_MET_filters = pass_MET_filters
                                   && event_info.Flag_goodVertices
                                   && event_info.Flag_globalSuperTightHalo2016Filter
                                   && event_info.Flag_HBHENoiseFilter
                                   && event_info.Flag_HBHENoiseIsoFilter
                                   && event_info.Flag_EcalDeadCellTriggerPrimitiveFilter
                                   && event_info.Flag_BadPFMuonFilter
                                   && event_info.Flag_BadPFMuonDzFilter
                                   && event_info.Flag_eeBadScFilter;
                // clang-format on
                // event_info.Flag_BadChargedCandidateFilter;
                // event_info.Flag_hfNoisyHitsFilter;
            }

            if (year == Year::Run2017 || year == Year::Run2018)
            {
                // clang-format off
                pass_MET_filters = pass_MET_filters
                                   && event_info.Flag_goodVertices
                                   && event_info.Flag_globalSuperTightHalo2016Filter
                                   && event_info.Flag_HBHENoiseFilter
                                   && event_info.Flag_HBHENoiseIsoFilter
                                   && event_info.Flag_EcalDeadCellTriggerPrimitiveFilter
                                   && event_info.Flag_BadPFMuonFilter
                                   && event_info.Flag_BadPFMuonDzFilter
                                   && event_info.Flag_eeBadScFilter
                                   && event_info.Flag_ecalBadCalibFilter;
                // clang-format on
                // event_info.Flag_hfNoisyHitsFilter;
                // event_info.Flag_BadChargedCandidateFilter;
            }

            if (pass_MET_filters)
            {
                // // fmt::print("\nDEBUG - met_filter");
                outputs.fill_cutflow_histo("METFilters", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS met FILTER");
            return *this;
        }
        return *this;
    }

    //////////////////////////////////////////////////////////
    /// Set `TriggerBits` for this event. They will be checked latter.
    // Clear `TriggerBits` and save the seed trigger.
    // Will loop over the `TriggerBits`. Once a fired trigger is found:
    //  1 - the fired trigger bit is saved as trigger_seed
    //  2 - all others bits are set to false
    ///
    EventData &set_trigger_bits()
    {
        if (*this)
        {
            switch (year)
            {
            case Year::Run2016APV:
                break;
            case Year::Run2016:
                break;
            case Year::Run2017:
                trigger_bits.set("SingleMuonLowPt", event_info.HLT_IsoMu27)
                    .set("SingleMuonHighPt", event_info.HLT_Mu50 || event_info.HLT_TkMu100 || event_info.HLT_OldMu100)
                    .set("SingleElectron", false)
                    .set("DoubleMuon", false)
                    .set("DoubleElectron", false)
                    .set("Photon", false)
                    .set("Tau", false)
                    .set("BJet", false)
                    .set("Jet", false)
                    .set("MET", false);
                break;
            case Year::Run2018:
                break;
            default:
                throw std::runtime_error("Year (" + std::to_string(year) +
                                         ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
            }

            ////////////////////////////////////////////////////////////////////////////
            // remove filter possible trigger duplicates, based on the trigger stream

            // those hana maps are complicated to work with.
            //  one can not simply ask for a key at runtime (e.g. my_map["foo"])
            // the solution is to loop over all keys and whether taht is the key that you are looking for
            bool found_hana_key = false;
            boost::hana::for_each(TriggerConfig::TriggerStreamRedList, [&](const auto &stream) {
                if (HANA_FIRST_TO_SV(stream) == trigger_stream)
                {
                    found_hana_key = true;
                    boost::hana::for_each(                                           //
                        boost::hana::second(stream), [&](const auto &trigger_path) { //
                            trigger_bits.set(trigger_path, false);                   //
                        });
                }
            });

            // just a sanitiy check ...
            if (not found_hana_key)
            {
                throw std::runtime_error(
                    fmt::format("[ ERROR ] The requested Trigger Stream ({}) could not be found.", trigger_stream));
            }

            return *this;
        }
        return *this;
    }

    //////////////////////////////////////////////////////////////
    /// Filter events that did not fired any trigger or do not pass double trigger firing check
    ///
    EventData &trigger_filter(Outputs &outputs)
    {
        if (*this)
        {
            // fmt::print("\nDEBUG - trigger_filter: {}\n", trigger_bits.as_string());
            // fmt::print("\nDEBUG - muon pt: {}\n", muons.pt);
            if (trigger_bits.any())
            {
                outputs.fill_cutflow_histo("TriggerCut", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS hlt_trigger FILTER");
            return *this;
        }
        return *this;
    }

    // Low pT muon filter
    auto get_low_pt_muons_selection_mask() -> RVec<int>
    {

        // fmt::print("Eta: {}\n", muons.eta);
        // fmt::print("Eta mask: {}\n", VecOps::abs(muons.eta) <= 2.4);
        // fmt::print("Eta mask from Config: {}\n", VecOps::abs(muons.eta) <= ObjConfig::Muons[year].MaxAbsEta);
        return (muons.pt >= ObjConfig::Muons[year].MinLowPt)                   //
               && (muons.pt < ObjConfig::Muons[year].MaxLowPt)                 //
               && (VecOps::abs(muons.eta) <= ObjConfig::Muons[year].MaxAbsEta) //
               && (muons.tightId)                                              //
               && (muons.pfRelIso03_all < ObjConfig::Muons[year].PFRelIso_WP);
    }

    // High pT muon filter
    auto get_high_pt_muons_selection_mask() -> RVec<int>
    {
        return (muons.pt >= ObjConfig::Muons[year].MaxLowPt)                   //
               && (VecOps::abs(muons.eta) <= ObjConfig::Muons[year].MaxAbsEta) //
               && (muons.highPtId >= 1)                                        //
               && (muons.tkRelIso < ObjConfig::Muons[year].TkRelIso_WP);
    }

    // Low pT Electrons
    auto get_low_pt_electrons_selection_mask() -> RVec<int>
    {
        return ((electrons.pt >= ObjConfig::Electrons[year].MinLowPt) &&
                (electrons.pt < ObjConfig::Electrons[year].MaxLowPt)) //
               && ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 1.442) ||
                   ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) >= 1.566) &&
                    (VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 2.5))) //
               && (electrons.cutBased >= 4);
    }

    // High pT Electrons
    auto get_high_pt_electrons_selection_mask() -> RVec<int>
    {
        return (electrons.pt >= ObjConfig::Electrons[year].MaxLowPt) //
               && ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 1.442) ||
                   ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) >= 1.566) &&
                    (VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 2.5))) //
               && (electrons.cutBased_HEEP);
    }

    // Photons
    auto get_photons_selection_mask() -> RVec<int>
    {
        return photons.pt >= ObjConfig::Photons[year].PreSelPt;
    }

    // Taus
    auto get_taus_selection_mask() -> RVec<int>
    {
        return taus.pt >= ObjConfig::Taus[year].PreSelPt;
    }

    // BJets
    auto get_bjets_selection_mask() -> RVec<int>
    {
        return bjets.pt >= ObjConfig::BJets[year].PreSelPt;
    }

    // Jets
    auto get_jets_selection_mask() -> RVec<int>
    {
        return jets.pt >= ObjConfig::Jets[year].PreSelPt;
    }

    // MET
    auto get_met_selection_mask() -> RVec<int>
    {
        return met.pt >= ObjConfig::MET[year].PreSelPt;
    }

    EventData &object_selection()
    {
        if (*this)
        {
            // launch selection tasks
            good_low_pt_muons_mask = get_low_pt_muons_selection_mask();
            good_high_pt_muons_mask = get_high_pt_muons_selection_mask();
            good_muons_mask = good_low_pt_muons_mask || good_high_pt_muons_mask;

            good_low_pt_electrons_mask = get_low_pt_electrons_selection_mask();
            good_high_pt_electrons_mask = get_high_pt_electrons_selection_mask();

            good_photons_mask = get_photons_selection_mask();

            good_taus_mask = get_taus_selection_mask();

            good_bjets_mask = get_bjets_selection_mask();

            good_jets_mask = get_jets_selection_mask();

            good_met_mask = get_met_selection_mask();
            return *this;
        }
        return *this;
    }

    EventData &has_selected_objects_filter(Outputs &outputs)
    {
        if (*this)
        {
            if (                                          //
                (VecOps::Sum(good_muons_mask) > 0) ||     //
                (VecOps::Sum(good_electrons_mask) > 0) || //
                (VecOps::Sum(good_photons_mask) > 0) ||   //
                (VecOps::Sum(good_taus_mask) > 0) ||      //
                (VecOps::Sum(good_bjets_mask) > 0) ||     //
                (VecOps::Sum(good_jets_mask) > 0) ||      //
                (VecOps::Sum(good_met_mask) > 0)          //
            )
            {
                outputs.fill_cutflow_histo("AtLeastOneSelectedObject", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS has_selected_objects_filter FILTER");
            return *this;
        }
        return *this;
    }

    /////////////////////////////////////////////////////////
    /// Not actually being used. Can stay, just in case ...
    ///
    // EventData &dummy()
    // {
    //     if (*this)
    //     {
    //         fmt::print("--> Muon_pt (inside matcher) : {}\n", muons.pt);
    //         fmt::print("--> Electron_pt (inside matcher) : {}\n", electrons.pt);
    //         fmt::print("--> trigger filter bits (inside matcher 1): {}\n", trgobjs.filterBits);
    //         return *this;
    //     }
    //     return *this;
    // }

    /////////////////////////////////////////////////////////////////////
    /// Will check if the current event has a matched object to a good TrgObj
    // here we have to break the single responsability rule ...
    // this filter also get the trigger scale factor,
    // otherwise we would have to look over the objects twice
    ///
    EventData &trigger_match_filter(Outputs &outputs, std::map<std::string_view, Corrector> &trigger_sf_correctors)
    {
        // fmt::print("--> Jet_eta (inside matcher 1): {}\n", jets.eta);
        // fmt::print("--> trigger filter bits (inside matcher 1): {}\n", trgobjs.filterBits);
        if (*this)
        {
            // fmt::print("--> trigger filter bits (inside matcher 2): {}\n", trgobjs.filterBits);
            // define matchers for each trigger
            std::map<std::string_view, std::function<bool()>> matchers;
            matchers["SingleMuonLowPt"] = [&]() {
                //////////////////////////////////////////
                // DEBUG
                // fmt::print("----------------------------- DEBUG ----------------------\n");
                // fmt::print("HLT Path: {}\n", "SingleMuonLowPt");
                /////////////////////////////////////////////////////

                auto trgobj_mask = (                                                          //
                    (trgobjs.id == PDG::Muon::Id) &&                                          //
                    (Trigger::check_trigger_bit(trgobjs.filterBits, "SingleMuonLowPt", year)) //
                );

                // fmt::print("Id: {}\n", trgobjs.id);
                // fmt::print("Id mask: {}\n", (trgobjs.id == PDG::Muon::Id));
                // fmt::print("Bit : {}\n", trgobjs.filterBits);
                // fmt::print("Bit mask: {}\n",
                //            Trigger::check_trigger_bit(trgobjs.filterBits, "SingleMuonLowPt", year));
                // fmt::print("---------\n");
                // fmt::print("Trigger Eta: {}\n", trgobjs.eta[trgobj_mask]);
                // fmt::print("Obj Eta: {}\n", muons.eta[good_low_pt_muons_mask]);
                // fmt::print("Trigger Phi: {}\n", trgobjs.phi[trgobj_mask]);
                // fmt::print("Obj Phi: {}\n", muons.phi[good_low_pt_muons_mask]);
                // fmt::print("---------\n");
                // fmt::print("MASK: {}\n", good_low_pt_muons_mask);
                // fmt::print("RAW Obj pt: {}\n", muons.pt);
                // fmt::print("RAW Obj eta: {}\n", muons.eta);
                // fmt::print("RAW Obj Phi: {}\n", muons.phi);

                const auto [_has_trigger_match, _matched_nano_object_pt, _matched_nano_object_eta] =
                    Trigger::trigger_matcher(trgobjs.pt[trgobj_mask],           //
                                             trgobjs.eta[trgobj_mask],          //
                                             trgobjs.phi[trgobj_mask],          //
                                             muons.pt[good_low_pt_muons_mask],  //
                                             muons.eta[good_low_pt_muons_mask], //
                                             muons.phi[good_low_pt_muons_mask], //
                                             ObjConfig::Muons[year].MaxDeltaRTriggerMatch);

                // will run if a trigger is found and matched
                if (_has_trigger_match)
                {
                    trigger_sf_nominal = //
                        trigger_sf_correctors.at("SingleMuonLowPt")({Trigger::get_year_for_muon_sf(year),
                                                                     fabs(_matched_nano_object_eta),
                                                                     _matched_nano_object_pt, "sf"});
                    trigger_sf_up = //
                        trigger_sf_correctors.at("SingleMuonLowPt")({Trigger::get_year_for_muon_sf(year),
                                                                     fabs(_matched_nano_object_eta),
                                                                     _matched_nano_object_pt, "systup"});
                    trigger_sf_down = //
                        trigger_sf_correctors.at("SingleMuonLowPt")({Trigger::get_year_for_muon_sf(year),
                                                                     fabs(_matched_nano_object_eta),
                                                                     _matched_nano_object_pt, "systdown"});

                    // fmt::print("SF: {} - {} - {}\n", trigger_sf_nominal, trigger_sf_up, trigger_sf_down);
                }
                return _has_trigger_match;
            };
            matchers["SingleMuonHighPt"] = [&]() {
                ////////////////////////////////////////
                // DEBUG
                // fmt::print("----------------------------- DEBUG ----------------------\n");
                // fmt::print("HLT Path: {}\n", "SingleMuonHighPt");
                ///////////////////////////////////////////////////

                auto trgobj_mask = (                                                           //
                    (trgobjs.id == PDG::Muon::Id) &&                                           //
                    (Trigger::check_trigger_bit(trgobjs.filterBits, "SingleMuonHighPt", year)) //
                );

                // fmt::print("Id: {}\n", trgobjs.id);
                // fmt::print("Id mask: {}\n", (trgobjs.id == PDG::Muon::Id));
                // fmt::print("Bit : {}\n", trgobjs.filterBits);
                // fmt::print("Bit mask: {}\n",
                //            Trigger::check_trigger_bit(trgobjs.filterBits, "SingleMuonHighPt", year));
                // fmt::print("---------\n");
                // fmt::print("Trigger Eta: {}\n", trgobjs.eta[trgobj_mask]);
                // fmt::print("Obj Eta: {}\n", muons.eta[good_high_pt_muons_mask]);
                // fmt::print("Trigger Phi: {}\n", trgobjs.phi[trgobj_mask]);
                // fmt::print("Obj Phi: {}\n", muons.phi[good_high_pt_muons_mask]);
                // fmt::print("---------\n");
                // fmt::print("MASK: {}\n", good_high_pt_muons_mask);
                // fmt::print("RAW Obj pt: {}\n", muons.pt);
                // fmt::print("RAW Obj eta: {}\n", muons.eta);
                // fmt::print("RAW Obj Phi: {}\n", muons.phi);

                const auto [_has_trigger_match, _matched_nano_object_pt, _matched_nano_object_eta] =
                    Trigger::trigger_matcher(trgobjs.pt[trgobj_mask],            //
                                             trgobjs.eta[trgobj_mask],           //
                                             trgobjs.phi[trgobj_mask],           //
                                             muons.pt[good_high_pt_muons_mask],  //
                                             muons.eta[good_high_pt_muons_mask], //
                                             muons.phi[good_high_pt_muons_mask], //
                                             ObjConfig::Muons[year].MaxDeltaRTriggerMatch);

                // will run if a trigger is found and matched
                if (_has_trigger_match)
                {
                    trigger_sf_nominal = //
                        trigger_sf_correctors.at("SingleMuonHighPt")({Trigger::get_year_for_muon_sf(year),
                                                                      fabs(_matched_nano_object_eta),
                                                                      _matched_nano_object_pt, "sf"});
                    trigger_sf_up = //
                        trigger_sf_correctors.at("SingleMuonHighPt")({Trigger::get_year_for_muon_sf(year),
                                                                      fabs(_matched_nano_object_eta),
                                                                      _matched_nano_object_pt, "systup"});
                    trigger_sf_down = //
                        trigger_sf_correctors.at("SingleMuonHighPt")({Trigger::get_year_for_muon_sf(year),
                                                                      fabs(_matched_nano_object_eta),
                                                                      _matched_nano_object_pt, "systdown"});

                    // fmt::print("SF: {} - {} - {}\n", trigger_sf_nominal, trigger_sf_up, trigger_sf_down);
                }
                return _has_trigger_match;
            };
            // FIXME: Include the trigger matching for all other objects.

            // given the trigger seed produce a triggerobj mask for that seed
            // test the correspondents objects to that seed
            // if fail, try next seed (bit)
            for (auto &&hlt_path : Trigger::HLTPath)
            {
                if (trigger_bits.pass(hlt_path))
                {
                    if (matchers.at(hlt_path)())
                    {
                        outputs.fill_cutflow_histo("TriggerMatch", outputs.get_event_weight());
                        return *this;
                    }
                }
            }

            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS triggermatch FILTER");
            return *this;
        }
        return *this;
    }

    EventData &set_scale_factors(Outputs &outputs)
    {
        if (*this)
        {
            // trigger
            outputs.set_event_weight("Trigger", "Nominal", trigger_sf_nominal);
            outputs.set_event_weight("Trigger", "Up", trigger_sf_up);
            outputs.set_event_weight("Trigger", "Down", trigger_sf_down);
            return *this;
        }
        return *this;
    }

    EventData &muon_corrections()
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    EventData &electron_corrections()
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    EventData &photon_corrections()
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    EventData &tau_corrections()
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    EventData &bjet_corrections()
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    EventData &jet_corrections()
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    EventData &met_corrections()
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    EventData &fill_event_content(Outputs &outputs)
    {
        if (*this)
        {
            outputs.run = event_info.run;
            outputs.lumi_section = event_info.lumi;
            outputs.event_number = event_info.event;
            outputs.trigger_bits = trigger_bits.as_ulong();

            outputs.fill_branches(
                // muons
                muons.pt[good_muons_mask],  //
                muons.eta[good_muons_mask], //
                muons.phi[good_muons_mask], //
                // electrons
                electrons.pt[good_electrons_mask],  //
                electrons.eta[good_electrons_mask], //
                electrons.phi[good_electrons_mask], //
                // photons
                photons.pt[good_photons_mask],  //
                photons.eta[good_photons_mask], //
                photons.phi[good_photons_mask], //
                // taus
                taus.pt[good_taus_mask],  //
                taus.eta[good_taus_mask], //
                taus.phi[good_taus_mask], //
                // bjets
                bjets.pt[good_bjets_mask],  //
                bjets.eta[good_bjets_mask], //
                bjets.phi[good_bjets_mask], //
                // jets
                jets.pt[good_jets_mask],  //
                jets.eta[good_jets_mask], //
                jets.phi[good_jets_mask], //
                // met
                met.pt[good_met_mask], //
                met.phi[good_met_mask]);

            return *this;
        }
        return *this;
    }
};

#endif /*MUSIC_EVENT_DATA*/