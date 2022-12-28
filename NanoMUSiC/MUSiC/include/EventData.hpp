#ifndef MUSIC_EVENT_DATA
#define MUSIC_EVENT_DATA

// On: 28.10.2022
// https://ericniebler.github.io/range-v3
// https://github.com/ericniebler/range-v3
#include <range/v3/all.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
// using fmt::print;

#include "NanoObjects.hpp"
#include "Outputs.hpp"
#include "Trigger.hpp"

using namespace ranges;

class EventData
{
  private:
    bool is_null = true;

  public:
    TriggerBits trigger_bits;
    NanoObjects::EventInfo event_info;
    NanoObjects::Muons muons;
    NanoObjects::Electrons electrons;
    NanoObjects::Photons photons;
    NanoObjects::Taus taus;
    NanoObjects::BJets bjets;
    NanoObjects::Jets jets;
    NanoObjects::MET met;
    RVec<int> good_muons_mask;
    RVec<int> good_electrons_mask;
    RVec<int> good_photons_mask;
    RVec<int> good_taus_mask;
    RVec<int> good_bjets_mask;
    RVec<int> good_jets_mask;
    RVec<int> good_met_mask;
    bool is_data = true;
    Year year = Year::kTotalYears;
    unsigned int idx_var;

    EventData(NanoObjects::EventInfo &&_event_info, NanoObjects::Muons &&_muons, NanoObjects::Electrons &&_electrons,
              NanoObjects::Photons &&_photons, NanoObjects::Taus &&_taus, NanoObjects::BJets &&_bjets, NanoObjects::Jets &&_jets,
              NanoObjects::MET &&_met, const bool &_is_data, const Year &_year, Variation variation = Variation::Default,
              Shift shift = Shift::Nominal)
        : is_null(false), event_info(_event_info), muons(_muons), electrons(_electrons), photons(_photons), taus(_taus),
          bjets(_bjets), jets(_jets), met(_met), is_data(_is_data), year(_year),
          idx_var(variation_and_shift_to_index(variation, shift))
    {
        if (idx_var < 0 || idx_var > Outputs::kTotalVariationsAndShifts)
        {
            throw std::runtime_error("[ERROR] \"idx_var\" is out of range.");
        }
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

    void set_variation_and_shift(const Variation &variation, const Shift &shift)
    {
        this->idx_var = variation_and_shift_to_index(variation, shift);
    }

    template <typename T>
    std::string to_string_with_zero_padding(T &value, std::size_t total_length = 2)
    {
        std::string _str = std::to_string(value);
        if (_str.length() < total_length)
        {
            _str.insert(0, "0");
        }
        return _str;
    }

    template <typename T>
    unsigned long make_class_hash(const T multiplicity)
    {
        const auto [n_muons, n_electrons, n_photons, n_taus, n_bjets, n_jets, n_met] = multiplicity;
        return make_class_hash(n_muons, n_electrons, n_photons, n_taus, n_bjets, n_jets, n_met);
    }

    unsigned long make_class_hash(const int &n_muons, const int &n_electrons, const int &n_photons, const int &n_taus,
                                  const int &n_bjets, const int &n_jets, const int &n_met)
    {
        return std::stoul("9" + to_string_with_zero_padding(n_muons) + to_string_with_zero_padding(n_electrons) +
                          to_string_with_zero_padding(n_photons) + to_string_with_zero_padding(n_taus) +
                          to_string_with_zero_padding(n_bjets) + to_string_with_zero_padding(n_jets) +
                          to_string_with_zero_padding(n_met));
    }

    auto make_multiplicity_range(const int &n_muons, const int &n_electrons, const int &n_photons, const int &n_taus,
                                 const int &n_bjets, const int &n_jets, const int &n_met)
    {
        return views::cartesian_product(
                   views::ints(0, std::min(n_muons, MAX_OBJECTS) + 1), views::ints(0, std::min(n_electrons, MAX_OBJECTS) + 1),
                   views::ints(0, std::min(n_photons, MAX_OBJECTS) + 1), views::ints(0, std::min(n_taus, MAX_OBJECTS) + 1),
                   views::ints(0, std::min(n_bjets, MAX_OBJECTS) + 1), views::ints(0, std::min(n_jets, MAX_OBJECTS) + 1),
                   views::ints(0, std::min(n_met, MAX_OBJECTS) + 1)) |
               views::remove_if([&](const auto &multiplicity) {
                   const auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;

                   // no taus (for now)
                   if (i_taus > 0)
                   {
                       return true;
                   }

                   // MET filter
                   if (i_met > 1)
                   {
                       return true;
                   }

                   // at least one muon or one electron
                   if (i_muons == 0 && i_electrons == 0)
                   {
                       return true;
                   }

                   // no more than MAX_JETS
                   if (i_jets > MAX_JETS)
                   {
                       return true;
                   }

                   // default --> accepted
                   return false;
               });
    }

    EventData &set_const_weights(Outputs &outputs, Corrector &pu_weight)
    {
        if (*this)
        {
            // fmt::print("DEBUG - set_const_weights");
            // set generator weight
            // should be called before any EventData method
            if (!is_data)
            {
                outputs.set_event_weight(idx_var, Weight::Generator, event_info.genWeight.get());
                outputs.set_event_weight(idx_var, Weight::PileUp, Shift::Nominal,
                                         pu_weight({event_info.Pileup_nTrueInt.get(), "nominal"}));
                outputs.set_event_weight(idx_var, Weight::PileUp, Shift::Up, pu_weight({event_info.Pileup_nTrueInt.get(), "up"}));
                outputs.set_event_weight(idx_var, Weight::PileUp, Shift::Down,
                                         pu_weight({event_info.Pileup_nTrueInt.get(), "down"}));
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
                // fmt::print("DEBUG - generator_filter");
                outputs.fill_default_cutflow_histos(CutFlow::NoCuts, 1.);
                outputs.fill_default_cutflow_histos(CutFlow::GeneratorWeight, outputs.get_event_weight(idx_var));
                return *this;
            }
            set_null();
        }
        return *this;
    }

    EventData &run_lumi_filter(Outputs &outputs, const RunLumiFilter &run_lumi_filter)
    {
        if (*this)
        {
            if (run_lumi_filter(event_info.run.get(), event_info.lumi.get(), is_data))
            {
                // fmt::print("DEBUG - run_lumi_filter");
                outputs.fill_default_cutflow_histos(CutFlow::RunLumi, outputs.get_event_weight(idx_var));
                return *this;
            }
            set_null();
        }
        return *this;
    }

    EventData &npv_filter(Outputs &outputs)
    {
        if (*this)
        {
            if (event_info.PV_npvsGood.get() > 0)
            {
                // fmt::print("DEBUG - PV_npvsGood");
                outputs.fill_default_cutflow_histos(CutFlow::nPV, outputs.get_event_weight(idx_var));
                return *this;
            }
            set_null();
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
                // fmt::print("DEBUG - met_filter");
                outputs.fill_default_cutflow_histos(CutFlow::MetFilters, outputs.get_event_weight(idx_var));
                return *this;
            }
            set_null();
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
            trigger_bits.set(HLTPath::SingleMuonLowPt, false)
                .set(HLTPath::SingleMuonHighPt, false)
                .set(HLTPath::SingleElectron, false)
                .set(HLTPath::DoubleMuon, false)
                .set(HLTPath::DoubleElectron, false)
                .set(HLTPath::Tau, false)
                .set(HLTPath::BJet, false)
                .set(HLTPath::MET, false)
                .set(HLTPath::Photon, false);

            switch (year)
            {
            case Year::Run2016APV:
                break;
            case Year::Run2016:
                break;
            case Year::Run2017:
                trigger_bits.set(HLTPath::SingleMuonLowPt, event_info.HLT_IsoMu27.get())
                    .set(HLTPath::SingleMuonHighPt,
                         event_info.HLT_Mu50.get() || event_info.HLT_TkMu100.get() || event_info.HLT_OldMu100.get())
                    .set(HLTPath::SingleElectron, false)
                    .set(HLTPath::DoubleMuon, false)
                    .set(HLTPath::DoubleElectron, false)
                    .set(HLTPath::Tau, false)
                    .set(HLTPath::BJet, false)
                    .set(HLTPath::MET, false)
                    .set(HLTPath::Photon, false);
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
                // fmt::print("DEBUG - trigger_filter");
                outputs.fill_default_cutflow_histos(CutFlow::TriggerCut, outputs.get_event_weight(idx_var));
                return *this;
            }
            set_null();
        }
        return *this;
    }

    // Muon - Filter
    auto get_muons_pre_selection_mask()
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
    auto get_electrons_pre_selection_mask()
    {
        return electrons.pt.get() >= ObjConfig::Electrons[year].PreSelPt;
    }

    // Photons
    auto get_photons_pre_selection_mask()
    {
        return photons.pt.get() >= ObjConfig::Photons[year].PreSelPt;
    }

    // Taus
    auto get_taus_pre_selection_mask()
    {
        return taus.pt.get() >= ObjConfig::Taus[year].PreSelPt;
    }

    // BJets
    auto get_bjets_pre_selection_mask()
    {
        return bjets.pt.get() >= ObjConfig::BJets[year].PreSelPt;
    }

    // Jets
    auto get_jets_pre_selection_mask()
    {
        return jets.pt.get() >= ObjConfig::Jets[year].PreSelPt;
    }

    // MET
    auto get_met_pre_selection_mask()
    {
        return met.pt.get() >= ObjConfig::MET[year].PreSelPt;
    }

    EventData &pre_selection()
    {
        if (*this)
        {
            // launch pre-selection tasks
            good_muons_mask = get_muons_pre_selection_mask();
            good_electrons_mask = get_electrons_pre_selection_mask();
            good_photons_mask = get_photons_pre_selection_mask();
            good_taus_mask = get_taus_pre_selection_mask();
            good_bjets_mask = get_bjets_pre_selection_mask();
            good_jets_mask = get_jets_pre_selection_mask();
            good_met_mask = get_met_pre_selection_mask();

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

    EventData &trigger_match()
    {
        if (*this)
        {
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

    EventData &has_at_least_one_class_filter()
    {
        // auto classes_size = distance(make_multiplicity_range(
        //     VecOps::Sum(good_muons_mask), VecOps::Sum(good_electrons_mask), VecOps::Sum(good_photons_mask),
        //     VecOps::Sum(good_taus_mask), VecOps::Sum(good_bjets_mask), VecOps::Sum(good_jets_mask),
        //     VecOps::Sum(good_met_mask)));

        // fmt::print("DEBUG - ObjConfig::Muons[year].PreSelPt: {}\n", ObjConfig::Muons[year].PreSelPt);
        // fmt::print("DEBUG - muons.pt: {}\n", muons.pt.get());
        // fmt::print("DEBUG - good_muons_mask: {}\n", good_muons_mask);
        // fmt::print("DEBUG - Classes size: {}\n", classes_size);

        // if (classes_size > 0)
        // {
        //     fmt::print("DEBUG - Hellow!! \n");
        //     for (auto &&multiplicity :
        //          make_multiplicity_range(VecOps::Sum(good_muons_mask), VecOps::Sum(good_electrons_mask),
        //                                  VecOps::Sum(good_photons_mask), VecOps::Sum(good_taus_mask),
        //                                  VecOps::Sum(good_bjets_mask), VecOps::Sum(good_jets_mask),
        //                                  VecOps::Sum(good_met_mask)))
        //     {
        //         const auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;

        //         fmt::print("DEBUG - Muons: {} - Electrons: {} - Photons: {} - Taus: {} - BJets: {} - Jets: {} - MET: {} \n",
        //                    i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met);
        //     }
        // }

        if (*this)
        {
            if (distance(make_multiplicity_range(VecOps::Sum(good_muons_mask), VecOps::Sum(good_electrons_mask),
                                                 VecOps::Sum(good_photons_mask), VecOps::Sum(good_taus_mask),
                                                 VecOps::Sum(good_bjets_mask), VecOps::Sum(good_jets_mask),
                                                 VecOps::Sum(good_met_mask))) > 0)
            {
                return *this;
            }
        }
        return *this;
    }

    EventData &fill_event_content(Outputs &outputs)
    {
        if (*this)
        {
            outputs.run.at(idx_var) = 2;
            outputs.lumi_section.at(idx_var) = 2;
            outputs.event_number.at(idx_var) = 2;
            outputs.trigger_bits.at(idx_var) = 2;
            // outputs.weights_nominal.at(idx_var) = {1, 2, 3};
            // outputs.weights_up.at(idx_var) = {1, 2, 3};
            // outputs.weights_down.at(idx_var) = {1, 2, 3};
            outputs.nClasses.at(idx_var) = 3;
            outputs.classes.at(idx_var) = {1, 2, 3};
            outputs.sum_pt.at(idx_var) = {1, 2, 3};
            outputs.invariant_mass.at(idx_var) = {1, 2, 3};
            outputs.met.at(idx_var) = {1, 2, 3};

            return *this;
        }
        return *this;
    }

    static EventData apply_corrections(const EventData &event_data, const Variation &variation, const Shift &shift)
    {
        if (event_data)
        {
            auto new_event_data = EventData(event_data);
            new_event_data.set_variation_and_shift(variation, shift);

            //////////////////////////////////////////////////
            // FIX ME: apply corrections
            //////////////////////////////////////////////////

            return new_event_data;
        }
        return event_data;
    }
};

#endif /*MUSIC_EVENT_DATA*/
