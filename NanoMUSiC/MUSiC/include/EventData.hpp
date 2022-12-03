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
#include "NanoAODReader.hpp"
#include "NanoObjects.hpp"
#include "Trigger.hpp"

using namespace ranges;

class EventData
{
  private:
    bool is_null = true;

  public:
    TH1F cutflow_histo = imp_make_cutflow_histo();
    bool is_data = true;
    Year year = Year::kTotalYears;
    UInt_t run = 0;
    UInt_t lumi_section = 0;
    ULong64_t event_number = 0;
    TriggerBits trigger_bits;
    NanoObject::NanoObjectCollection muons;
    NanoObject::NanoObjectCollection electrons;
    NanoObject::NanoObjectCollection photons;
    NanoObject::NanoObjectCollection taus;
    NanoObject::NanoObjectCollection bjets;
    NanoObject::NanoObjectCollection jets;
    NanoObject::NanoObject met;
    std::set<unsigned long> classes;
    EventContent event_content;

    EventData()
    {
    }

    EventData(const bool &_is_data, const Year &_year, const UInt_t &_run, const UInt_t &_lumi_section,
              const ULong64_t &_event_number, TriggerBits &&_trigger_bits, NanoObject::NanoObjectCollection &&_muons,
              NanoObject::NanoObjectCollection &&_electrons, NanoObject::NanoObjectCollection &&_photons,
              NanoObject::NanoObjectCollection &&_taus, NanoObject::NanoObjectCollection &&_bjets,
              NanoObject::NanoObjectCollection &&_jets, NanoObject::NanoObject &&_met)
        : is_null(false), is_data(_is_data), year(_year), run(_run), lumi_section(_lumi_section), event_number(_event_number),
          trigger_bits(_trigger_bits), muons(_muons), electrons(_electrons), photons(_photons), taus(_taus), bjets(_bjets),
          jets(_jets), met(_met), classes(std::set<unsigned long>())
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

    // unnullify - not sure when it would be needed, but...
    void unset_null()
    {
        this->is_null = true;
    }

    EventData &set_const_weights(NanoAODReader &nano_reader, Corrector &pu_weight)
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
            event_content.event_weight.set(Weight::Generator, nano_reader.getVal<Float_t>("genWeight"));
            event_content.event_weight.set(Weight::PileUp, Shift::Nominal,
                                           pu_weight({nano_reader.getVal<Float_t>("Pileup_nTrueInt"), "nominal"}));
            event_content.event_weight.set(Weight::PileUp, Shift::Up,
                                           pu_weight({nano_reader.getVal<Float_t>("Pileup_nTrueInt"), "up"}));
            event_content.event_weight.set(Weight::PileUp, Shift::Down,
                                           pu_weight({nano_reader.getVal<Float_t>("Pileup_nTrueInt"), "down"}));
        }
        return *this;
    }

    EventData &generator_filter(NanoAODReader &nano_reader)
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
                cutflow_histo.Fill(CutFlow::NoCuts, 1.);
                cutflow_histo.Fill(CutFlow::GeneratorWeight, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    EventData &run_lumi_filter(const RunLumiFilter &run_lumi_filter)
    {
        if (*this)
        {
            if (run_lumi_filter(run, lumi_section, is_data))
            {
                cutflow_histo.Fill(CutFlow::RunLumi, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    EventData &npv_filter(NanoAODReader &nano_reader)
    {
        if (*this)
        {
            if (nano_reader.getVal<Int_t>("PV_npvsGood") > 0)
            {
                cutflow_histo.Fill(CutFlow::nPV, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    EventData &met_filter(NanoAODReader &nano_reader)
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
                                   && nano_reader.getVal<Bool_t>("Flag_goodVertices") 
                                   && nano_reader.getVal<Bool_t>("Flag_globalSuperTightHalo2016Filter") 
                                   && nano_reader.getVal<Bool_t>("Flag_HBHENoiseFilter") 
                                   && nano_reader.getVal<Bool_t>("Flag_HBHENoiseIsoFilter") 
                                   && nano_reader.getVal<Bool_t>("Flag_EcalDeadCellTriggerPrimitiveFilter") 
                                   && nano_reader.getVal<Bool_t>("Flag_BadPFMuonFilter") 
                                   && nano_reader.getVal<Bool_t>("Flag_BadPFMuonDzFilter") 
                                   && nano_reader.getVal<Bool_t>("Flag_eeBadScFilter");
                // clang-format on
                // nano_reader.getVal<Bool_t>("Flag_BadChargedCandidateFilter");
                // nano_reader.getVal<Bool_t>("Flag_hfNoisyHitsFilter");
            }

            if (year == Year::Run2017 || year == Year::Run2018)
            {
                // clang-format off
                pass_MET_filters = pass_MET_filters 
                                   && nano_reader.getVal<Bool_t>("Flag_goodVertices") 
                                   && nano_reader.getVal<Bool_t>("Flag_globalSuperTightHalo2016Filter") 
                                   && nano_reader.getVal<Bool_t>("Flag_HBHENoiseFilter") 
                                   && nano_reader.getVal<Bool_t>("Flag_HBHENoiseIsoFilter") 
                                   && nano_reader.getVal<Bool_t>("Flag_EcalDeadCellTriggerPrimitiveFilter") 
                                   && nano_reader.getVal<Bool_t>("Flag_BadPFMuonFilter")
                                   && nano_reader.getVal<Bool_t>("Flag_BadPFMuonDzFilter") 
                                   && nano_reader.getVal<Bool_t>("Flag_eeBadScFilter")
                                   && nano_reader.getVal<Bool_t>("Flag_ecalBadCalibFilter");
                // clang-format on
                // nano_reader.getVal<Float_t>("Flag_hfNoisyHitsFilter");
                // nano_reader.getVal<Float_t>("Flag_BadChargedCandidateFilter");
            }
            if (pass_MET_filters)
            {
                cutflow_histo.Fill(CutFlow::MetFilters, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    EventData &trigger_filter(NanoAODReader &nano_reader)
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
                trigger_bits.set(HLTPath::SingleMuonLowPt, nano_reader.getVal<Bool_t>("HLT_IsoMu27"))
                    .set(HLTPath::SingleMuonHighPt, nano_reader.getVal<Bool_t>("HLT_Mu50") ||
                                                        nano_reader.getVal<Bool_t>("HLT_TkMu100") ||
                                                        nano_reader.getVal<Bool_t>("HLT_OldMu100"))
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
                cutflow_histo.Fill(CutFlow::TriggerCut, event_content.event_weight.get());
                return *this;
            }
            set_null();
        }
        return *this;
    }

    // launch async thread (into task runners pool)
    // returns a future to be gotten (.get())
    // Muon - Filter
    auto launch_muon_filter_task(const NanoObject::NanoObjectCollection &muons, BS::thread_pool &task_runners_pool)
    {
        return task_runners_pool.submit(
            [&](const auto &muons) {
                return NanoObject::Filter(muons, [&](const auto &muon) {
                    // low pT muon
                    if (muon.pt() >= ObjConfig::Muons[year].PreSelPt && muon.pt() < ObjConfig::Muons[year].MaxLowPt &&
                        std::fabs(muon.eta()) <= ObjConfig::Muons[year].MaxAbsEta)
                    {
                        if (muon.template get<UInt_t>("tightId") &&
                            muon.template get<Float_t>("pfRelIso03_all") < ObjConfig::Muons[year].PFRelIso_WP)
                        {
                            return true;
                        }
                    }

                    // high pT muon
                    if (muon.pt() >= ObjConfig::Muons[year].MaxLowPt && std::fabs(muon.eta()) <= ObjConfig::Muons[year].MaxAbsEta)
                    {
                        if (muon.template get<UChar_t>("highPtId") >= 1 &&
                            muon.template get<Float_t>("tkRelIso") < ObjConfig::Muons[year].TkRelIso_WP)
                        {
                            return true;
                        }
                    }
                    return false;
                });
            },
            muons);
    }

    // Electrons
    auto launch_electron_filter_task(const NanoObject::NanoObjectCollection &electrons, BS::thread_pool &task_runners_pool)
    {
        return task_runners_pool.submit(
            [&](const auto &electrons) {
                return NanoObject::Filter(electrons, [&](const auto &electron) {
                    if (electron.pt() >= ObjConfig::Electrons[year].PreSelPt &&
                        electron.pt() < ObjConfig::Electrons[year].MaxLowPt &&
                        std::fabs(electron.eta()) <= ObjConfig::Electrons[year].MaxAbsEta)
                    {
                        if ((electron.template get<UInt_t>(ObjConfig::Electrons[year].TightIdVar) ||
                             electron.template get<UInt_t>(ObjConfig::Electrons[year].HEEPIdVar)) &&
                            electron.template get<Float_t>(ObjConfig::Electrons[year].IsoVar) <
                                ObjConfig::Electrons[year].PFRelIso_WP)
                        {
                            return true;
                        }
                    }
                    return false;
                });
            },
            electrons);
    }

    // Photons
    auto launch_photon_filter_task(const NanoObject::NanoObjectCollection &photons, BS::thread_pool &task_runners_pool)
    {
        return task_runners_pool.submit(
            [&](const auto &photons) {
                return NanoObject::Filter(photons, [&](const auto &photon) {
                    if (photon.pt() >= ObjConfig::Photons[year].PreSelPt && photon.pt() < ObjConfig::Photons[year].MaxLowPt &&
                        std::fabs(photon.eta()) <= ObjConfig::Photons[year].MaxAbsEta)
                    {
                        if ((photon.template get<UInt_t>(ObjConfig::Photons[year].TightIdVar) ||
                             photon.template get<UInt_t>(ObjConfig::Photons[year].HEEPIdVar)) &&
                            photon.template get<Float_t>(ObjConfig::Photons[year].IsoVar) < ObjConfig::Photons[year].PFRelIso_WP)
                        {
                            return true;
                        }
                    }
                    return false;
                });
            },
            photons);
    }
    // Taus
    auto launch_tau_filter_task(const NanoObject::NanoObjectCollection &taus, BS::thread_pool &task_runners_pool)
    {
        return task_runners_pool.submit(
            [&](const auto &taus) {
                return NanoObject::Filter(taus, [&](const auto &tau) {
                    if (tau.pt() >= ObjConfig::Taus[year].PreSelPt && tau.pt() < ObjConfig::Taus[year].MaxLowPt &&
                        std::fabs(tau.eta()) <= ObjConfig::Taus[year].MaxAbsEta)
                    {
                        if ((tau.template get<UInt_t>(ObjConfig::Taus[year].TightIdVar) ||
                             tau.template get<UInt_t>(ObjConfig::Taus[year].HEEPIdVar)) &&
                            tau.template get<Float_t>(ObjConfig::Taus[year].IsoVar) < ObjConfig::Taus[year].PFRelIso_WP)
                        {
                            return true;
                        }
                    }
                    return false;
                });
            },
            taus);
    }

    // Bjets
    auto launch_bjet_filter_task(const NanoObject::NanoObjectCollection &bjets, BS::thread_pool &task_runners_pool)
    {
        return task_runners_pool.submit(
            [&](const auto &bjets) {
                return NanoObject::Filter(bjets, [&](const auto &bjet) {
                    if (bjet.pt() >= ObjConfig::Bjets[year].PreSelPt && bjet.pt() < ObjConfig::Bjets[year].MaxLowPt &&
                        std::fabs(bjet.eta()) <= ObjConfig::Bjets[year].MaxAbsEta)
                    {
                        if ((bjet.template get<UInt_t>(ObjConfig::Bjets[year].TightIdVar) ||
                             bjet.template get<UInt_t>(ObjConfig::Bjets[year].HEEPIdVar)) &&
                            bjet.template get<Float_t>(ObjConfig::Bjets[year].IsoVar) < ObjConfig::Bjets[year].PFRelIso_WP)
                        {
                            return true;
                        }
                    }
                    return false;
                });
            },
            bjets);
    }

    auto launch_jet_filter_task(const NanoObject::NanoObjectCollection &jets, BS::thread_pool &task_runners_pool)
    {
        return task_runners_pool.submit(
            [&](const auto &jets) {
                return NanoObject::Filter(jets, [&](const auto &jet) {
                    if (jet.pt() >= ObjConfig::Jets[year].PreSelPt && jet.pt() < ObjConfig::Jets[year].MaxLowPt &&
                        std::fabs(jet.eta()) <= ObjConfig::Jets[year].MaxAbsEta)
                    {
                        if ((jet.template get<UInt_t>(ObjConfig::Jets[year].TightIdVar) ||
                             jet.template get<UInt_t>(ObjConfig::Jets[year].HEEPIdVar)) &&
                            jet.template get<Float_t>(ObjConfig::Jets[year].IsoVar) < ObjConfig::Jets[year].PFRelIso_WP)
                        {
                            return true;
                        }
                    }
                    return false;
                });
            },
            jets);
    }

    auto launch_met_filter_task(const NanoObject::NanoObject &met, BS::thread_pool &task_runners_pool)
    {
        return task_runners_pool.submit(
            [&](const auto &met) {
                if (met.pt() >= ObjConfig::MET[year].PreSelPt)
                {
                    return met;
                }
                return NanoObject::NanoObject{};
            },
            met);
    }
    EventData &pre_selection(NanoAODReader &nano_reader, BS::thread_pool &task_runners_pool)
    {
        if (*this)
        {
            // launch pre-selection tasks
            auto good_muons_future = launch_muon_filter_task(muons, task_runners_pool);
            auto good_electrons_future = launch_electron_filter_task(electrons, task_runners_pool);
            auto good_photons_future = launch_photon_filter_task(photons, task_runners_pool);
            auto good_taus_future = launch_tau_filter_task(taus, task_runners_pool);
            auto good_bjets_future = launch_bjet_filter_task(bjets, task_runners_pool);
            auto good_jets_future = launch_jet_filter_task(jets, task_runners_pool);
            auto good_met_future = launch_met_filter_task(met, task_runners_pool);

            // collect futures
            muons = good_muons_future.get();
            electrons = good_electrons_future.get();
            taus = good_taus_future.get();
            bjets = good_bjets_future.get();
            jets = good_jets_future.get();
            met = good_met_future.get();

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
