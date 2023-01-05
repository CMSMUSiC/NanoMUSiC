#ifndef MUSIC_EVENT_DATA
#define MUSIC_EVENT_DATA

// On: 28.10.2022
// https://ericniebler.github.io/range-v3
// https://github.com/ericniebler/range-v3
// #include <range/v3/all.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
// using fmt::print;

#include "NanoObjects.hpp"
#include "Outputs.hpp"
#include "Trigger.hpp"

// using namespace ranges;

class EventData
{
  private:
    bool is_null = true;

  public:
    TriggerBits trigger_bits;
    NanoObjects::EventInfo event_info;
    NanoObjects::Muons muons;
    RVec<int> good_muons_mask;
    NanoObjects::Electrons electrons;
    RVec<int> good_electrons_mask;
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
    bool is_data = true;
    Year year = Year::kTotalYears;

    EventData(const bool &_is_data, const Year &_year, std::string_view variation = "Default", std::string_view shift = "Nominal")
        : is_null(false), is_data(_is_data), year(_year)
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

    // un-null-ify - not sure when/if it would be needed, but...
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
                outputs.set_event_weight("Generator", event_info.genWeight.get());
                outputs.set_event_weight("PileUp", "Nominal", pu_weight({event_info.Pileup_nTrueInt.get(), "nominal"}));
                outputs.set_event_weight("PileUp", "Up", pu_weight({event_info.Pileup_nTrueInt.get(), "up"}));
                outputs.set_event_weight("PileUp", "Down", pu_weight({event_info.Pileup_nTrueInt.get(), "down"}));
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
                // FIX ME: check if it is good gen event
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
            if (_run_lumi_filter(event_info.run.get(), event_info.lumi.get(), is_data))
            {
                // // fmt::print("\nDEBUG - run_lumi_filter");
                outputs.fill_cutflow_histo("RunLumi", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS RUN_LUMI FILTER: {} - {}", event_info.run.get(), event_info.lumi.get());
            return *this;
        }
        return *this;
    }

    EventData &npv_filter(Outputs &outputs)
    {
        if (*this)
        {
            if (event_info.PV_npvsGood.get() > 0)
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
                                   && event_info.Flag_goodVertices.get()
                                   && event_info.Flag_globalSuperTightHalo2016Filter.get()
                                   && event_info.Flag_HBHENoiseFilter.get()
                                   && event_info.Flag_HBHENoiseIsoFilter.get()
                                   && event_info.Flag_EcalDeadCellTriggerPrimitiveFilter.get()
                                   && event_info.Flag_BadPFMuonFilter.get()
                                   && event_info.Flag_BadPFMuonDzFilter.get()
                                   && event_info.Flag_eeBadScFilter.get();
                // clang-format on
                // event_info.Flag_BadChargedCandidateFilter.get();
                // event_info.Flag_hfNoisyHitsFilter.get();
            }

            if (year == Year::Run2017 || year == Year::Run2018)
            {
                // clang-format off
                pass_MET_filters = pass_MET_filters
                                   && event_info.Flag_goodVertices.get()
                                   && event_info.Flag_globalSuperTightHalo2016Filter.get()
                                   && event_info.Flag_HBHENoiseFilter.get()
                                   && event_info.Flag_HBHENoiseIsoFilter.get()
                                   && event_info.Flag_EcalDeadCellTriggerPrimitiveFilter.get()
                                   && event_info.Flag_BadPFMuonFilter.get()
                                   && event_info.Flag_BadPFMuonDzFilter.get()
                                   && event_info.Flag_eeBadScFilter.get()
                                   && event_info.Flag_ecalBadCalibFilter.get();
                // clang-format on
                // event_info.Flag_hfNoisyHitsFilter.get();
                // event_info.Flag_BadChargedCandidateFilter.get();
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

    EventData &trigger_filter(Outputs &outputs)
    {
        if (*this)
        {
            ////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////
            // fill trigger bits
            ////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////
            trigger_bits.set(TriggerBits::HLTPath.index_of("SingleMuonLowPt"), false)
                .set(TriggerBits::HLTPath.index_of("SingleMuonHighPt"), false)
                .set(TriggerBits::HLTPath.index_of("SingleElectron"), false)
                .set(TriggerBits::HLTPath.index_of("DoubleMuon"), false)
                .set(TriggerBits::HLTPath.index_of("DoubleElectron"), false)
                .set(TriggerBits::HLTPath.index_of("Photon"), false)
                .set(TriggerBits::HLTPath.index_of("Tau"), false)
                .set(TriggerBits::HLTPath.index_of("BJet"), false)
                .set(TriggerBits::HLTPath.index_of("MET"), false);

            switch (year)
            {
            case Year::Run2016APV:
                break;
            case Year::Run2016:
                break;
            case Year::Run2017:
                trigger_bits.set(TriggerBits::HLTPath.index_of("SingleMuonLowPt"), event_info.HLT_IsoMu27.get())
                    .set(TriggerBits::HLTPath.index_of("SingleMuonHighPt"),
                         event_info.HLT_Mu50.get() || event_info.HLT_TkMu100.get() || event_info.HLT_OldMu100.get())
                    .set(TriggerBits::HLTPath.index_of("SingleElectron"), false)
                    .set(TriggerBits::HLTPath.index_of("DoubleMuon"), false)
                    .set(TriggerBits::HLTPath.index_of("DoubleElectron"), false)
                    .set(TriggerBits::HLTPath.index_of("Tau"), false)
                    .set(TriggerBits::HLTPath.index_of("BJet"), false)
                    .set(TriggerBits::HLTPath.index_of("MET"), false)
                    .set(TriggerBits::HLTPath.index_of("Photon"), false);
                break;
            case Year::Run2018:
                break;
            default:
                throw std::runtime_error("Year (" + std::to_string(year) +
                                         ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
            }

            // skip event event if no trigger is fired
            if (trigger_bits.any())
            {
                // // fmt::print("\nDEBUG - trigger_filter");
                outputs.fill_cutflow_histo("TriggerCut", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS hlt_trigger FILTER");
            return *this;
        }
        return *this;
    }

    // Muon - Filter
    auto get_muons_selection_mask()
    {
        auto low_pt_mask = muons.pt.get() >= ObjConfig::Muons[year].PreSelPt &&
                           muons.pt.get() < ObjConfig::Muons[year].MaxLowPt &&
                           ROOT::VecOps::abs(muons.eta.get()) <= ObjConfig::Muons[year].MaxAbsEta && muons.tightId.get() &&
                           muons.pfRelIso03_all.get() < ObjConfig::Muons[year].PFRelIso_WP;

        auto high_pt_mask = muons.pt.get() >= ObjConfig::Muons[year].MaxLowPt &&
                            ROOT::VecOps::abs(muons.eta.get()) <= ObjConfig::Muons[year].MaxAbsEta && muons.highPtId.get() >= 1 &&
                            muons.tkRelIso.get() < ObjConfig::Muons[year].TkRelIso_WP;

        return low_pt_mask || high_pt_mask;
    }

    // Electrons
    auto get_electrons_selection_mask()
    {
        return electrons.pt.get() >= ObjConfig::Electrons[year].PreSelPt;
    }

    // Photons
    auto get_photons_selection_mask()
    {
        return photons.pt.get() >= ObjConfig::Photons[year].PreSelPt;
    }

    // Taus
    auto get_taus_selection_mask()
    {
        return taus.pt.get() >= ObjConfig::Taus[year].PreSelPt;
    }

    // BJets
    auto get_bjets_selection_mask()
    {
        return bjets.pt.get() >= ObjConfig::BJets[year].PreSelPt;
    }

    // Jets
    auto get_jets_selection_mask()
    {
        return jets.pt.get() >= ObjConfig::Jets[year].PreSelPt;
    }

    // MET
    auto get_met_selection_mask()
    {
        return met.pt.get() >= ObjConfig::MET[year].PreSelPt;
    }

    EventData &object_selection()
    {
        if (*this)
        {
            // launch pre-selection tasks
            good_muons_mask = get_muons_selection_mask();
            good_electrons_mask = get_electrons_selection_mask();
            good_photons_mask = get_photons_selection_mask();
            good_taus_mask = get_taus_selection_mask();
            good_bjets_mask = get_bjets_selection_mask();
            good_jets_mask = get_jets_selection_mask();
            good_met_mask = get_met_selection_mask();

            // muons = muons.filter(good_muons_mask);
            // electrons = electrons.filter(good_electrons_mask);
            // taus = taus.filter(good_taus_mask);
            // bjets = bjets.filter(good_bjets_mask);
            // jets = jets.filter(good_jets_mask);
            // met = met.filter(good_met_mask);

            return *this;
        }
        return *this;
    }

    EventData &final_selection()
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    EventData &trigger_match_filter(Outputs &outputs)
    {
        if (*this)
        {
            bool has_trigger_match = true;
            if (has_trigger_match)
            {
                outputs.fill_cutflow_histo("TriggerMatch", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS triggermatch FILTER");
            return *this;
        }
        return *this;
    }

    EventData &set_scale_factors()
    {
        if (*this)
        {
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

    EventData &has_selected_objects_filter(Outputs &outputs)
    {
        if (*this)
        {

            if (VecOps::Sum(good_muons_mask) > 0 || VecOps::Sum(good_electrons_mask) > 0 || VecOps::Sum(good_photons_mask) > 0 ||
                VecOps::Sum(good_taus_mask) > 0 || VecOps::Sum(good_bjets_mask) > 0 || VecOps::Sum(good_jets_mask) > 0 ||
                VecOps::Sum(good_met_mask))
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

    EventData &fill_event_content(Outputs &outputs)
    {
        if (*this)
        {
            outputs.run = event_info.run.get();
            outputs.lumi_section = event_info.lumi.get();
            outputs.event_number = event_info.event.get();
            outputs.trigger_bits = trigger_bits.as_ulong();

            outputs.fill_branches(
                // muons
                muons.pt.get()[good_muons_mask], muons.eta.get()[good_muons_mask], muons.phi.get()[good_muons_mask],
                // electrons
                electrons.pt.get()[good_electrons_mask], electrons.eta.get()[good_electrons_mask],
                electrons.phi.get()[good_electrons_mask],
                // photons
                photons.pt.get()[good_photons_mask], photons.eta.get()[good_photons_mask], photons.phi.get()[good_photons_mask],
                // taus
                taus.pt.get()[good_taus_mask], taus.eta.get()[good_taus_mask], taus.phi.get()[good_taus_mask],
                // bjets
                bjets.pt.get()[good_bjets_mask], bjets.eta.get()[good_bjets_mask], bjets.phi.get()[good_bjets_mask],
                // jets
                jets.pt.get()[good_jets_mask], jets.eta.get()[good_jets_mask], jets.phi.get()[good_jets_mask],
                // met
                met.pt.get()[good_met_mask], met.phi.get()[good_met_mask]);
            return *this;
        }
        return *this;
    }
};

#endif /*MUSIC_EVENT_DATA*/