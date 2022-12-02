#ifndef MUSIC_PROCESS_EVENTS
#define MUSIC_PROCESS_EVENTS

#include "EventData.hpp"
#include <vector>

using OptMUSiCEvent_t = std::optional<std::tuple<MUSiCEvent, TH1F, std::set<unsigned long>>>;

struct EventProcessResult
{
    MUSiCEvent music_event;
    TH1F cutflow_histo;
    std::set<unsigned long> classes;
    bool is_null = false;
};

EventData load_event_data(NanoAODReader &nano_reader, const bool &is_data, const Year &year)
{
    return EventData(
        is_data, year, nano_reader.getVal<UInt_t>("run"), nano_reader.getVal<UInt_t>("luminosityBlock"),
        nano_reader.getVal<ULong64_t>("event"), TriggerBits{},
        // Muons
        NanoObject::make_collection(
            nano_reader.getVec<Float_t>("Muon_pt"), nano_reader.getVec<Float_t>("Muon_eta"),
            nano_reader.getVec<Float_t>("Muon_phi"), nano_reader.getVec<Float_t>("Muon_mass"),
            NanoObject::make_feature("tightId", nano_reader.getVecOfBools("Muon_tightId")),
            NanoObject::make_feature("highPtId", nano_reader.getVec<UChar_t>("Muon_highPtId")),
            NanoObject::make_feature("pfRelIso03_all", nano_reader.getVec<Float_t>("Muon_pfRelIso03_all")),
            NanoObject::make_feature("tkRelIso", nano_reader.getVec<Float_t>("Muon_tkRelIso"))),
        // Electrons
        NanoObject::make_collection(nano_reader.getVec<Float_t>("Electron_pt"), nano_reader.getVec<Float_t>("Electron_eta"),
                                    nano_reader.getVec<Float_t>("Electron_phi"), nano_reader.getVec<Float_t>("Electron_mass")),
        // Photons
        NanoObject::make_collection(nano_reader.getVec<Float_t>("Photon_pt"), nano_reader.getVec<Float_t>("Photon_eta"),
                                    nano_reader.getVec<Float_t>("Photon_phi"), nano_reader.getVec<Float_t>("Photon_mass")),
        // Taus
        NanoObject::make_collection(nano_reader.getVec<Float_t>("Tau_pt"), nano_reader.getVec<Float_t>("Tau_eta"),
                                    nano_reader.getVec<Float_t>("Tau_phi"), nano_reader.getVec<Float_t>("Tau_mass")),
        // BJets
        // FIX ME: bjets should have: ObjConfig::Jets[year].btag_wp_tight
        NanoObject::make_collection(
            nano_reader.getVec<Float_t>("Jet_pt"), nano_reader.getVec<Float_t>("Jet_eta"), nano_reader.getVec<Float_t>("Jet_phi"),
            nano_reader.getVec<Float_t>("Jet_mass"),
            NanoObject::make_feature(
                std::string(ObjConfig::Jets[year].btag_algo),
                nano_reader.getVec<Float_t>(std::string("Jet_") + std::string(ObjConfig::Jets[year].btag_algo)))),
        // Jets
        // FIX ME: jets should NOT have: ObjConfig::Jets[year].btag_wp_tight
        NanoObject::make_collection(
            nano_reader.getVec<Float_t>("Jet_pt"), nano_reader.getVec<Float_t>("Jet_eta"), nano_reader.getVec<Float_t>("Jet_phi"),
            nano_reader.getVec<Float_t>("Jet_mass"),
            NanoObject::make_feature(
                std::string(ObjConfig::Jets[year].btag_algo),
                nano_reader.getVec<Float_t>(std::string("Jet_") + std::string(ObjConfig::Jets[year].btag_algo)))),
        // MET
        NanoObject::make_object(
            nano_reader.getVal<Float_t>("MET_pt"), nano_reader.getVal<Float_t>("MET_phi"),
            NanoObject::make_feature("significance", nano_reader.getVal<Float_t>("MET_significance")),
            NanoObject::make_feature("MetUnclustEnUpDeltaX", nano_reader.getVal<Float_t>("MET_MetUnclustEnUpDeltaX"))));
    // end of EventData declaration
}

EventProcessResult process_event(NanoAODReader &nano_reader, const bool &is_data, const Year &year,
                                 const RunLumiFilter &run_lumi_filter, Corrector &pu_weight, BS::thread_pool &task_runners_pool)
{
    // holds the data that will be written to the output file
    auto music_event = MUSiCEvent{};

    // holds data for processing only
    // + preliminary selection
    auto event_data = load_event_data(nano_reader, is_data, year)
                          .set_const_weights(nano_reader, music_event, pu_weight)
                          .gen_filter(nano_reader, music_event)
                          .run_lumi_filter(run_lumi_filter, music_event)
                          .npv_filter(nano_reader, music_event)
                          .met_filter(nano_reader, music_event)
                          .trigger_filter(nano_reader, music_event)
                          .pre_selection(nano_reader, music_event, task_runners_pool);

    // launch nominal
    auto nominal_event_data = EventData::apply_corrections(event_data, Variation::Default, Shift::Nominal)
                                  .final_selection()
                                  .trigger_match()
                                  .set_scale_factors()
                                  .fill_event_content()
                                  .has_any_content_filter();

    if (nominal_event_data)
    {

        // fill nominal data
        music_event.fill(nominal_event_data.event_content, Variation::Default, Shift::Nominal);

        // launch variations
        // loop over variations, shifts and classes (aka multiplicities)
        ranges::for_each(views::cartesian_product(range_variations, range_shifts), [&](const auto &variation_and_shift) {
            const auto [variation, shift] = variation_and_shift;

            if (variation != Variation::Default)
            {
                // modify objects according to the given variation
                auto varied_event_data = EventData::apply_corrections(event_data, variation, shift);
                music_event.fill(varied_event_data.final_selection().fill_event_content().has_any_content_filter().event_content,
                                 variation, shift);
            }
        });
        // return OptMUSiCEvent_t(std::make_tuple(music_event, nominal_event_data.cutflow_histo, nominal_event_data.classes));
        return EventProcessResult{music_event, nominal_event_data.cutflow_histo, nominal_event_data.classes};
    }
    return EventProcessResult{.is_null = true};
}

#endif /*MUSIC_PROCESS_EVENTS*/
