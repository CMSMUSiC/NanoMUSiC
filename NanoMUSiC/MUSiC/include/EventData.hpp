#ifndef MUSIC_EVENT_DATA
#define MUSIC_EVENT_DATA

// On: 28.10.2022
// https://ericniebler.github.io/range-v3
// https://github.com/ericniebler/range-v3
// #include <range/v3/algorithm/for_each.hpp>
// #include <range/v3/numeric/accumulate.hpp>
// #include <range/v3/view/cartesian_product.hpp>
// #include <range/v3/view/iota.hpp>
// #include <range/v3/view/remove_if.hpp>
// #include <range/v3/view/take.hpp>
// #include <range/v3/view/transform.hpp>
#include <range/v3/all.hpp>

#include "MUSiCEvent.hpp"
#include "NanoObjects.hpp"
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
    std::set<unsigned long> classes;
    EventContent event_content;
    bool is_data = true;
    Year year = Year::kTotalYears;

    EventData()
    {
    }

    EventData(NanoObjects::EventInfo &&_event_info, NanoObjects::Muons &&_muons, NanoObjects::Electrons &&_electrons,
              NanoObjects::Photons &&_photons, NanoObjects::Taus &&_taus, NanoObjects::BJets &&_bjets, NanoObjects::Jets &&_jets,
              NanoObjects::MET &&_met, const bool &_is_data, const Year &_year)
        : is_null(false), event_info(_event_info), muons(_muons), electrons(_electrons), photons(_photons), taus(_taus),
          bjets(_bjets), jets(_jets), met(_met), is_data(_is_data), year(_year)
    {
    }

    operator bool() const
    {
        return is_null;
    }

    // nullify the event
    void set_null()
    {
        this->is_null = true;
    }

    // unnullify - not sure when/if it would be needed, but...
    void unset_null()
    {
        this->is_null = true;
    }

    void fill_cutflow_histo(TH1F &cutflow_histo, const CutFlow &cut, const float &weight)
    {
        cutflow_histo.Fill(cut, weight);
    }

    void fill_cutflow_histo(std::array<TH1F, total_variations_and_shifts> &cutflow_histos, const CutFlow &cut,
                            const float &weight)
    {
        for (auto histo : cutflow_histos)
        {
            histo.Fill(cut, weight);
        }
    }

    EventData &set_const_weights(Corrector &pu_weight)
    {
        // set generator weight
        // should be called before any EventData method
        if (is_data)
        {
            event_content.event_weight.set(Weight::Generator, 1.);
            event_content.event_weight.set(Weight::PileUp, 1.);
        }
        else
        {
            event_content.event_weight.set(Weight::Generator, event_info.genWeight);
            event_content.event_weight.set(Weight::PileUp, Shift::Nominal, pu_weight({event_info.Pileup_nTrueInt, "nominal"}));
            event_content.event_weight.set(Weight::PileUp, Shift::Up, pu_weight({event_info.Pileup_nTrueInt, "up"}));
            event_content.event_weight.set(Weight::PileUp, Shift::Down, pu_weight({event_info.Pileup_nTrueInt, "down"}));
        }
        return *this;
    }

    template <typename T>
    EventData &generator_filter(T &&cutflow_histo)
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
            // if Data
            else
            {
                is_good_gen = true;
            }
            if (is_good_gen)
            {
                fill_cutflow_histo(std::forward<T>(cutflow_histo), CutFlow::NoCuts, 1.);
                fill_cutflow_histo(std::forward<T>(cutflow_histo), CutFlow::GeneratorWeight, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    template <typename T>
    EventData &run_lumi_filter(T &&cutflow_histo, const RunLumiFilter &run_lumi_filter)
    {
        if (*this)
        {
            if (run_lumi_filter(event_info.run, event_info.lumi, is_data))
            {
                fill_cutflow_histo(std::forward<T>(cutflow_histo), CutFlow::RunLumi, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    template <typename T>
    EventData &npv_filter(T &&cutflow_histo)
    {
        if (*this)
        {
            if (event_info.PV_npvsGood > 0)
            {
                fill_cutflow_histo(std::forward<T>(cutflow_histo), CutFlow::nPV, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    template <typename T>
    EventData &met_filter(T &&cutflow_histo)
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
                fill_cutflow_histo(std::forward<T>(cutflow_histo), CutFlow::MetFilters, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    template <typename T>
    EventData &trigger_filter(T &&cutflow_histo)
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
                trigger_bits.set(HLTPath::SingleMuonLowPt, event_info.HLT_IsoMu27)
                    .set(HLTPath::SingleMuonHighPt, event_info.HLT_Mu50 || event_info.HLT_TkMu100 || event_info.HLT_OldMu100)
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
                fill_cutflow_histo(std::forward<T>(cutflow_histo), CutFlow::TriggerCut, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    // Muon - Filter
    auto get_muons_filter_mask()
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
    auto get_electrons_filter_mask()
    {
        return electrons.pt.get() >= ObjConfig::Electrons[year].PreSelPt;
    }

    // Photons
    auto get_photons_filter_mask()
    {
        return photons.pt.get() >= ObjConfig::Photons[year].PreSelPt;
    }

    // Taus
    auto get_taus_filter_mask()
    {
        return taus.pt.get() >= ObjConfig::Taus[year].PreSelPt;
    }

    // BJets
    auto get_bjets_filter_mask()
    {
        return bjets.pt.get() >= ObjConfig::BJets[year].PreSelPt;
    }

    // Jets
    auto get_jets_filter_mask()
    {
        return jets.pt.get() >= ObjConfig::Jets[year].PreSelPt;
    }

    // MET
    auto get_met_filter_mask()
    {
        return met.pt.get() >= ObjConfig::MET[year].PreSelPt;
    }

    EventData &pre_selection()
    {
        if (*this)
        {
            // launch pre-selection tasks
            good_muons_mask = get_muons_filter_mask();
            good_electrons_mask = get_electrons_filter_mask();
            good_photons_mask = get_photons_filter_mask();
            good_taus_mask = get_taus_filter_mask();
            good_bjets_mask = get_bjets_filter_mask();
            good_jets_mask = get_jets_filter_mask();
            good_met_mask = get_met_filter_mask();

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

    EventData &fill_event_content()
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    EventData &has_any_content_filter()
    {
        if (*this)
        {
            if (event_content.event_class_hash.size() > 0)
            {
                return *this;
            }
            set_null();
            return *this;
        }
        return *this;
    }

    static EventData apply_corrections(EventData event_data, const Variation &variation, const Shift &shift)
    {
        if (event_data)
        {
            auto new_event_data = EventData(event_data);

            //////////////////////////////////////////////////
            // FIX ME: apply corrections
            //////////////////////////////////////////////////

            return new_event_data;
        }
        return event_data;
    }
};

#endif /*MUSIC_EVENT_DATA*/
