#ifndef MUSIC_PROCESS_EVENTS
#define MUSIC_PROCESS_EVENTS

#include "EventData.hpp"
#include <vector>

using EventProcessResult_t = std::array<EventData, total_variations_and_shifts>;

EventData load_event_data(NanoAODReader &nano_reader, const bool &is_data, const Year &year)
{
    return EventData(
        is_data, year, nano_reader.getVal<UInt_t>("run"), nano_reader.getVal<UInt_t>("luminosityBlock"),
        nano_reader.getVal<ULong64_t>("event"), TriggerBits{},
        // Muons
        NanoObjects::make_collection(
            nano_reader.getVec<Float_t>("Muon_pt"), nano_reader.getVec<Float_t>("Muon_eta"),
            nano_reader.getVec<Float_t>("Muon_phi"), nano_reader.getVec<Float_t>("Muon_mass"),
            NanoObjects::make_feature("tightId", nano_reader.getVecOfBools("Muon_tightId")),
            NanoObjects::make_feature("highPtId", nano_reader.getVec<UChar_t>("Muon_highPtId")),
            NanoObjects::make_feature("pfRelIso03_all", nano_reader.getVec<Float_t>("Muon_pfRelIso03_all")),
            NanoObjects::make_feature("tkRelIso", nano_reader.getVec<Float_t>("Muon_tkRelIso"))),
        // Electrons
        NanoObjects::make_collection(nano_reader.getVec<Float_t>("Electron_pt"), nano_reader.getVec<Float_t>("Electron_eta"),
                                     nano_reader.getVec<Float_t>("Electron_phi"), nano_reader.getVec<Float_t>("Electron_mass")),
        // Photons
        NanoObjects::make_collection(nano_reader.getVec<Float_t>("Photon_pt"), nano_reader.getVec<Float_t>("Photon_eta"),
                                     nano_reader.getVec<Float_t>("Photon_phi"), nano_reader.getVec<Float_t>("Photon_mass")),
        // Taus
        NanoObjects::make_collection(nano_reader.getVec<Float_t>("Tau_pt"), nano_reader.getVec<Float_t>("Tau_eta"),
                                     nano_reader.getVec<Float_t>("Tau_phi"), nano_reader.getVec<Float_t>("Tau_mass")),
        // BJets
        // FIX ME: bjets should have: ObjConfig::Jets[year].btag_wp_tight
        NanoObjects::make_collection(
            nano_reader.getVec<Float_t>("Jet_pt"), nano_reader.getVec<Float_t>("Jet_eta"), nano_reader.getVec<Float_t>("Jet_phi"),
            nano_reader.getVec<Float_t>("Jet_mass"),
            NanoObjects::make_feature(
                std::string(ObjConfig::Jets[year].btag_algo),
                nano_reader.getVec<Float_t>(std::string("Jet_") + std::string(ObjConfig::Jets[year].btag_algo)))),
        // Jets
        // FIX ME: jets should NOT have: ObjConfig::Jets[year].btag_wp_tight
        NanoObjects::make_collection(
            nano_reader.getVec<Float_t>("Jet_pt"), nano_reader.getVec<Float_t>("Jet_eta"), nano_reader.getVec<Float_t>("Jet_phi"),
            nano_reader.getVec<Float_t>("Jet_mass"),
            NanoObjects::make_feature(
                std::string(ObjConfig::Jets[year].btag_algo),
                nano_reader.getVec<Float_t>(std::string("Jet_") + std::string(ObjConfig::Jets[year].btag_algo)))),
        // MET
        NanoObjects::make_object(
            nano_reader.getVal<Float_t>("MET_pt"), nano_reader.getVal<Float_t>("MET_phi"),
            NanoObjects::make_feature("significance", nano_reader.getVal<Float_t>("MET_significance")),
            NanoObjects::make_feature("MetUnclustEnUpDeltaX", nano_reader.getVal<Float_t>("MET_MetUnclustEnUpDeltaX"))));
    // end of EventData declaration
}

EventProcessResult_t process_event(NanoAODReader &nano_reader, const bool &is_data, const Year &year,
                                   const RunLumiFilter &run_lumi_filter, Corrector &pu_weight, BS::thread_pool &task_runners_pool)
{
    // holds the data that will be written to the output file
    EventProcessResult_t event_process_result;

    // holds data for processing only
    // + preliminary selection
    EventData event_data = load_event_data(nano_reader, is_data, year)
                               .set_const_weights(nano_reader, pu_weight)
                               .generator_filter(nano_reader)
                               .run_lumi_filter(run_lumi_filter)
                               .npv_filter(nano_reader)
                               .met_filter(nano_reader)
                               .trigger_filter(nano_reader)
                               .pre_selection(nano_reader, task_runners_pool);

    // launch variations
    // loop over variations, shifts and classes (aka multiplicities)

    std::array<std::future<EventData>, total_variations_and_shifts> variations_futures;
    ranges::for_each(range_cleanned_variation_and_shifts | views::remove_if([&is_data](auto variation_and_shift) {
                         const auto [variation, shift] = variation_and_shift;
                         return is_data && (variation != Variation::Default);
                     }),
                     [&](const auto &variation_and_shift) {
                         const auto [variation, shift] = variation_and_shift;

                         // modify objects according to the given variation
                         auto variation_task = [](auto event_data, auto variation, auto shift) {
                             return EventData::apply_corrections(event_data, variation, shift)
                                 .final_selection()
                                 .trigger_match()
                                 .set_scale_factors()
                                 .fill_event_content()
                                 .has_any_content_filter();
                         };
                         auto idx = variation_shift_to_index(variation, shift);
                         variations_futures.at(idx) = task_runners_pool.submit(variation_task, event_data, variation, shift);
                     });

    ranges::for_each(range_cleanned_variation_and_shifts | views::remove_if([&is_data](auto variation_and_shift) {
                         const auto [variation, shift] = variation_and_shift;
                         return is_data && (variation != Variation::Default);
                     }),
                     [&](const auto &variation_and_shift) {
                         const auto [variation, shift] = variation_and_shift;

                         // modify objects according to the given variation
                         auto idx = variation_shift_to_index(variation, shift);
                         event_process_result.at(idx) = variations_futures.at(idx).get();
                     });
    return event_process_result;
}

#endif /*MUSIC_PROCESS_EVENTS*/
