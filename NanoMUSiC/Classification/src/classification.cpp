#include "Classification.hpp"

#include "Configs.hpp"
#include "RtypesCore.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "fmt/core.h"
#include <cstdlib>
#include <fmt/format.h>
#include <optional>
#include <pybind11/pytypes.h>
#include <stdlib.h>
#include <string>

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
namespace py = pybind11;
using namespace pybind11::literals;

auto print_debug(long long global_event_index, bool debug) -> void
{
    // process monitoring
    if (debug)
    {
        if ((global_event_index < 10) or                                        //
            (global_event_index < 100 && global_event_index % 10 == 0) or       //
            (global_event_index < 1000 && global_event_index % 100 == 0) or     //
            (global_event_index < 10000 && global_event_index % 1000 == 0) or   //
            (global_event_index < 100000 && global_event_index % 10000 == 0) or //
            (global_event_index >= 100000 && global_event_index % 100000 == 0)  //
        )
        {
            fmt::print("\n\nProcessed {} events ...\n", global_event_index + 1);
            PrintProcessInfo();
        }
    }
    else
    {
        if ((global_event_index > 100000 and global_event_index % 100000 == 0))
        {
            fmt::print("\n\nProcessed {} events ...\n", global_event_index);
            PrintProcessInfo();
        }
    }
}

auto classification(const std::string process,
                    const std::string year,
                    const bool is_data,
                    const double x_section,
                    const double filter_eff,
                    const double k_factor,
                    const double luminosity,
                    const std::string xs_order,
                    const std::string process_group,
                    const std::string sum_weights_json_filepath,
                    const std::string input_file,
                    // [EVENT_CLASS_NAME, [SHIFT, EVENT_CLASS_OBJECT] ]
                    EventClassContainer &event_classes,
                    std::optional<unsigned long> first_event,
                    std::optional<long> last_event,
                    const bool debug) -> void
{
    fmt::print("\n[MUSiC Classification] Starting ...\n");
    if (debug)
    {
        fmt::print("[MUSiC Classification] Will process file: {}\n", input_file);
    }

    if (not(first_event))
    {
        first_event = 0;
    }
    if (not(last_event))
    {
        last_event = -1;
    }

    const auto shifts = Shifts(is_data);

    // corrections
    auto correctionlib_utils = CorrectionLibUtils();
    auto jet_corrections = JetCorrector(get_runyear(year), get_era_from_process_name(process, is_data), is_data);
    auto pu_corrector = correctionlib_utils.make_correctionlib_ref("PU", year);

    auto low_pt_muon_trigger_sf = correctionlib_utils.make_correctionlib_ref("SingleMuonLowPt", year);
    auto high_pt_muon_trigger_sf = correctionlib_utils.make_correctionlib_ref("SingleMuonHighPt", year);

    auto muon_sf_reco = correctionlib_utils.make_correctionlib_ref("MuonReco", year);
    auto muon_sf_id_low_pt = correctionlib_utils.make_correctionlib_ref("MuonIdLowPt", year);
    auto muon_sf_id_high_pt = correctionlib_utils.make_correctionlib_ref("MuonIdHighPt", year);
    auto muon_sf_iso_low_pt = correctionlib_utils.make_correctionlib_ref("MuonIsoLowPt", year);
    auto muon_sf_iso_high_pt = correctionlib_utils.make_correctionlib_ref("MuonIsoHighPt", year);

    auto electron_sf = correctionlib_utils.make_correctionlib_ref("ElectronSF", year);

    auto photon_sf = correctionlib_utils.make_correctionlib_ref("PhotonSF", year);
    auto pixel_veto_sf = correctionlib_utils.make_correctionlib_ref("PixelVetoSF", year);

    auto deep_tau_2017_v2_p1_vs_e = correctionlib_utils.make_correctionlib_ref("TauVSe", year);
    auto deep_tau_2017_v2_p1_vs_mu = correctionlib_utils.make_correctionlib_ref("TauVSmu", year);
    auto deep_tau_2017_v2_p1_vs_jet = correctionlib_utils.make_correctionlib_ref("TauVSjet", year);
    auto tau_energy_scale = correctionlib_utils.make_correctionlib_ref("TauEnergyScale", year);

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    // [ BEGIN ]  LHAPDF
    /////////////////////////////////////////////
    /////////////////////////////////////////////
    // initilize pdf sets for fallback cases ...
    // silence LHAPDF
    LHAPDF::setVerbosity(0);
    // NNPDF31_nnlo_as_0118_hessian
    int lha_id_fallback = 304400;
    int lha_size = 101;

    // Compute the PDF weight for this event using NNPDF31_nnlo_as_0118_hessian (304400) and divide the
    // new weight by the weight from the PDF the event was produced with.
    std::tuple<std::vector<std::unique_ptr<LHAPDF::PDF>>, std::unique_ptr<LHAPDF::PDF>, std::unique_ptr<LHAPDF::PDF>>
        default_pdf_sets;
    std::get<0>(default_pdf_sets).reserve(101);
    for (int idx = lha_id_fallback; idx < (lha_id_fallback + lha_size); idx++)
    {
        std::get<0>(default_pdf_sets).push_back(std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(lha_id_fallback)));
    }

    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0120 (319500) and divide the new
    // weight by the weight from the PDF the event was produced with.
    std::get<1>(default_pdf_sets) = std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(319500));

    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0116 (319300) and divide the new
    // weight by the weight from the PDF the event was produced with.
    std::get<2>(default_pdf_sets) = std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(319300));

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    // [ END ] LHAPDF
    /////////////////////////////////////////////
    /////////////////////////////////////////////

    // get sum of weights
    auto sum_weights_json_file = std::ifstream(sum_weights_json_filepath);
    if (!sum_weights_json_file.is_open())
    {
        fmt::print(stderr, "ERROR: Coudl not open sum of weights JSON file. {}\n", sum_weights_json_filepath);
        std::exit(EXIT_FAILURE);
    }
    json sum_weights_json = json::parse(sum_weights_json_file);
    const auto [sum_weights, counter_raw_events, should_use_LHEWeight] = std::tuple<double, double, double>(1., 1., 1.);
    // const auto [sum_weights, counter_raw_events, should_use_LHEWeight] =
    //     [&sum_weights_json, &process, &year, is_data]() -> std::tuple<double, double, bool>
    // {
    //     if (not(is_data))
    //     {
    //         double w_LHEWeight_originalXWGTUP = sum_weights_json[process][year]["sum_LHEWeight_originalXWGTUP"];
    //         double w_genWeight = sum_weights_json[process][year]["sum_genWeight"];
    //         long long counter_raw_events = sum_weights_json[process][year]["raw_events"];
    //
    //         if (w_genWeight == 0. and w_LHEWeight_originalXWGTUP > 0.)
    //         {
    //             return {w_LHEWeight_originalXWGTUP, counter_raw_events, true};
    //         }
    //
    //         if (w_genWeight > 0.)
    //         {
    //             if (w_genWeight / static_cast<double>(counter_raw_events) == 1.)
    //             {
    //                 if (w_LHEWeight_originalXWGTUP > 0.)
    //                 {
    //                     return {w_LHEWeight_originalXWGTUP, counter_raw_events, true};
    //                 }
    //                 fmt::print(stderr, "ERROR: Could not get sum of weights. Both weights are invalid.\n");
    //                 std::exit(EXIT_FAILURE);
    //             }
    //             return {w_genWeight, counter_raw_events, false};
    //         }
    //
    //         fmt::print(stderr, "ERROR: Could not get sum of weights. Both weights are invalid.\n");
    //         std::exit(EXIT_FAILURE);
    //     }
    //     return {1., 1., false};
    // }();
    //

    auto input_root_file = std::unique_ptr<TFile>(TFile::Open(input_file.c_str()));
    auto input_ttree = input_root_file->Get<TTree>("Events");
    input_ttree->SetBranchStatus("*", false);
    auto tree_reader = TTreeReader(input_ttree);

    // check for the available pdf weights in the tree
    std::optional<std::pair<unsigned int, unsigned int>> lha_indexes =
        PDFAlphaSWeights::get_pdf_ids(tree_reader.GetTree());

    std::optional<std::unique_ptr<LHAPDF::PDF>> this_sample_pdf = std::nullopt;
    if (lha_indexes)
    {
        this_sample_pdf = std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(std::get<0>(*lha_indexes)));
    }

    ADD_VALUE_READER(HLT_IsoMu24, bool);

    ADD_VALUE_READER(genWeight, float);

    ADD_VALUE_READER(Pileup_nTrueInt, float);
    ADD_VALUE_READER(L1PreFiringWeight_Up, float);
    ADD_VALUE_READER(L1PreFiringWeight_Dn, float);
    ADD_VALUE_READER(L1PreFiringWeight_Nom, float);

    ADD_ARRAY_READER(LHEPdfWeight, float);
    ADD_VALUE_READER(Generator_scalePDF, float);
    ADD_VALUE_READER(Generator_x1, float);
    ADD_VALUE_READER(Generator_x2, float);
    ADD_VALUE_READER(Generator_id1, int);
    ADD_VALUE_READER(Generator_id2, int);
    ADD_VALUE_READER(LHEWeight_originalXWGTUP, float);

    ADD_ARRAY_READER(LHEScaleWeight, float);

    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);
    ADD_ARRAY_READER(Muon_tightId, bool);
    ADD_ARRAY_READER(Muon_highPtId, UChar_t);
    ADD_ARRAY_READER(Muon_pfRelIso04_all, float);
    ADD_ARRAY_READER(Muon_tkRelIso, float);
    ADD_ARRAY_READER(Muon_tunepRelPt, float);
    ADD_ARRAY_READER(Muon_highPurity, bool);
    ADD_ARRAY_READER(Muon_genPartIdx, int);

    ADD_ARRAY_READER(Electron_pt, float);
    ADD_ARRAY_READER(Electron_eta, float);
    ADD_ARRAY_READER(Electron_phi, float);
    ADD_ARRAY_READER(Electron_deltaEtaSC, float);
    ADD_ARRAY_READER(Electron_cutBased, int);
    ADD_ARRAY_READER(Electron_cutBased_HEEP, bool);
    ADD_ARRAY_READER(Electron_scEtOverPt, float);
    ADD_ARRAY_READER(Electron_dEscaleUp, float);
    ADD_ARRAY_READER(Electron_dEscaleDown, float);
    ADD_ARRAY_READER(Electron_dEsigmaUp, float);
    ADD_ARRAY_READER(Electron_dEsigmaDown, float);
    ADD_ARRAY_READER(Electron_genPartIdx, int);

    ADD_ARRAY_READER(Tau_pt, float);
    ADD_ARRAY_READER(Tau_eta, float);
    ADD_ARRAY_READER(Tau_phi, float);
    ADD_ARRAY_READER(Tau_dz, float);
    ADD_ARRAY_READER(Tau_mass, float);
    ADD_ARRAY_READER(Tau_idDeepTau2017v2p1VSe, UChar_t);
    ADD_ARRAY_READER(Tau_idDeepTau2017v2p1VSjet, UChar_t);
    ADD_ARRAY_READER(Tau_idDeepTau2017v2p1VSmu, UChar_t);
    ADD_ARRAY_READER(Tau_decayMode, int);
    ADD_ARRAY_READER(Tau_genPartIdx, int);
    ADD_ARRAY_READER(Tau_genPartFlav, UChar_t);

    ADD_ARRAY_READER(Photon_pt, float);
    ADD_ARRAY_READER(Photon_eta, float);
    ADD_ARRAY_READER(Photon_phi, float);
    ADD_ARRAY_READER(Photon_isScEtaEB, bool);
    ADD_ARRAY_READER(Photon_isScEtaEE, bool);
    ADD_ARRAY_READER(Photon_cutBased, Int_t);
    ADD_ARRAY_READER(Photon_pixelSeed, bool);
    ADD_ARRAY_READER(Photon_dEscaleUp, float);
    ADD_ARRAY_READER(Photon_dEscaleDown, float);
    ADD_ARRAY_READER(Photon_dEsigmaUp, float);
    ADD_ARRAY_READER(Photon_dEsigmaDown, float);
    ADD_ARRAY_READER(Photon_genPartIdx, int);

    ADD_VALUE_READER(fixedGridRhoFastjetAll, float);

    ADD_ARRAY_READER(GenJet_pt, float);
    ADD_ARRAY_READER(GenJet_eta, float);
    ADD_ARRAY_READER(GenJet_phi, float);

    ADD_ARRAY_READER(Jet_pt, float);
    ADD_ARRAY_READER(Jet_eta, float);
    ADD_ARRAY_READER(Jet_phi, float);
    ADD_ARRAY_READER(Jet_mass, float);
    ADD_ARRAY_READER(Jet_jetId, Int_t);
    ADD_ARRAY_READER(Jet_btagDeepFlavB, float);
    ADD_ARRAY_READER(Jet_rawFactor, float);
    ADD_ARRAY_READER(Jet_area, float);
    ADD_ARRAY_READER(Jet_genJetIdx, Int_t);

    ADD_VALUE_READER(RawMET_pt, float);
    ADD_VALUE_READER(RawMET_phi, float);
    ADD_VALUE_READER(MET_MetUnclustEnUpDeltaX, float);
    ADD_VALUE_READER(MET_MetUnclustEnUpDeltaY, float);

    //  launch event loop for Data or MC
    fmt::print("\n[MUSiC Classification] Starting event loop ...\n");
    long long global_event_index = 0;
    for (auto &&event : tree_reader)
    {
        if (event < static_cast<long long>(*first_event))
        {
            continue;
        }
        else if (event > static_cast<long long>(*last_event) and static_cast<long long>(*last_event) >= 0)
        {
            break;
        }
        print_debug(global_event_index, debug);

        // check for chain readout quality
        // REFERENCE: https://root.cern.ch/doc/v608/classTTreeReader.html#a568e43c7d7d8b1f511bbbeb92c9094a8
        if (tree_reader.GetEntryStatus() != TTreeReader::EEntryStatus::kEntryValid)
        {
            fmt::print(stderr, "ERROR: Could not load TTree entry.");
            std::exit(EXIT_FAILURE);
        }

        // Trigger
        auto is_good_trigger = trigger_filter(process,             //
                                              is_data,             //
                                              get_runyear(year),   //
                                              unwrap(HLT_IsoMu24), //
                                              false,
                                              false,
                                              false,
                                              false,
                                              false,
                                              false,
                                              false,
                                              false
                                              // unwrap(pass_high_pt_muon_trigger),     //
                                              // unwrap(pass_double_muon_trigger),      //
                                              // unwrap(pass_low_pt_electron_trigger),  //
                                              // unwrap(pass_high_pt_electron_trigger), //
                                              // unwrap(pass_double_electron_trigger),  //
                                              //   unwrap(pass_high_pt_tau_trigger),      //
                                              //   unwrap(pass_double_tau_trigger), //
                                              // unwrap(pass_photon_trigger) //
        );
        if (not(is_good_trigger))
        {
            continue;
        }

        for (auto &&diff_shift : shifts.get_differential_shifts())
        {
            // build objects
            // muons
            auto muons = ObjectFactories::make_muons(unwrap(Muon_pt),             //
                                                     unwrap(Muon_eta),            //
                                                     unwrap(Muon_phi),            //
                                                     unwrap(Muon_tightId),        //
                                                     unwrap(Muon_highPtId),       //
                                                     unwrap(Muon_pfRelIso04_all), //
                                                     unwrap(Muon_tkRelIso),       //
                                                     unwrap(Muon_tunepRelPt),     //
                                                     unwrap(Muon_highPurity),     //
                                                     unwrap(Muon_genPartIdx),     //
                                                     muon_sf_reco,                //
                                                     muon_sf_id_low_pt,           //
                                                     muon_sf_id_high_pt,          //
                                                     muon_sf_iso_low_pt,          //
                                                     muon_sf_iso_high_pt,         //
                                                     is_data,                     //
                                                     year,                        //
                                                     diff_shift);

            auto electrons = ObjectFactories::make_electrons(unwrap(Electron_pt),               //
                                                             unwrap(Electron_eta),              //
                                                             unwrap(Electron_phi),              //
                                                             unwrap(Electron_deltaEtaSC),       //
                                                             unwrap(Electron_cutBased),         //
                                                             unwrap(Electron_cutBased_HEEP),    //
                                                             unwrap(Electron_scEtOverPt, true), //
                                                             unwrap(Electron_dEscaleUp),        //
                                                             unwrap(Electron_dEscaleDown),      //
                                                             unwrap(Electron_dEsigmaUp),        //
                                                             unwrap(Electron_dEsigmaDown),      //
                                                             unwrap(Electron_genPartIdx),       //
                                                             electron_sf,                       //
                                                             is_data,                           //
                                                             year,                              //
                                                             diff_shift);

            auto taus = ObjectFactories::make_taus(unwrap(Tau_pt),   //
                                                   unwrap(Tau_eta),  //
                                                   unwrap(Tau_phi),  //
                                                   unwrap(Tau_dz),   //
                                                   unwrap(Tau_mass), //
                                                   unwrap(Tau_genPartFlav),
                                                   unwrap(Tau_genPartIdx),             //
                                                   unwrap(Tau_decayMode),              //
                                                   unwrap(Tau_idDeepTau2017v2p1VSe),   //
                                                   unwrap(Tau_idDeepTau2017v2p1VSmu),  //
                                                   unwrap(Tau_idDeepTau2017v2p1VSjet), //
                                                   deep_tau_2017_v2_p1_vs_e,           //
                                                   deep_tau_2017_v2_p1_vs_mu,          //
                                                   deep_tau_2017_v2_p1_vs_jet,         //
                                                   tau_energy_scale,                   //
                                                   is_data,                            //
                                                   year,                               //
                                                   diff_shift);

            auto photons = ObjectFactories::make_photons(unwrap(Photon_pt),          //
                                                         unwrap(Photon_eta),         //
                                                         unwrap(Photon_phi),         //
                                                         unwrap(Photon_isScEtaEB),   //
                                                         unwrap(Photon_isScEtaEE),   //
                                                         unwrap(Photon_cutBased),    //
                                                         unwrap(Photon_pixelSeed),   //
                                                         unwrap(Photon_dEscaleUp),   //
                                                         unwrap(Photon_dEscaleDown), //
                                                         unwrap(Photon_dEsigmaUp),   //
                                                         unwrap(Photon_dEsigmaDown), //
                                                         unwrap(Photon_genPartIdx),  //
                                                         photon_sf,                  //
                                                         pixel_veto_sf,              //
                                                         is_data,                    //
                                                         year,                       //
                                                         diff_shift);

            auto [jets, bjets] = ObjectFactories::make_jets(unwrap(Jet_pt),                 //
                                                            unwrap(Jet_eta),                //
                                                            unwrap(Jet_phi),                //
                                                            unwrap(Jet_mass),               //
                                                            unwrap(Jet_jetId),              //
                                                            unwrap(Jet_btagDeepFlavB),      //
                                                            unwrap(Jet_rawFactor),          //
                                                            unwrap(Jet_area),               //
                                                            unwrap(Jet_genJetIdx),          //
                                                            unwrap(fixedGridRhoFastjetAll), //
                                                            jet_corrections,                //
                                                            // btag_sf_Corrector,                    //
                                                            NanoAODGenInfo::GenJets(unwrap(GenJet_pt),   //
                                                                                    unwrap(GenJet_eta),  //
                                                                                    unwrap(GenJet_phi)), //
                                                            is_data,                                     //
                                                            year,                                        //
                                                            diff_shift);

            auto met = ObjectFactories::make_met( //
                unwrap(RawMET_pt),                //
                unwrap(RawMET_phi),               //
                unwrap(MET_MetUnclustEnUpDeltaX), //
                unwrap(MET_MetUnclustEnUpDeltaY), //
                muons.get_delta_met_x(),          //
                muons.get_delta_met_y(),          //
                electrons.get_delta_met_x(),      //
                electrons.get_delta_met_y(),      //
                taus.get_delta_met_x(),           //
                taus.get_delta_met_y(),           //
                photons.get_delta_met_x(),        //
                photons.get_delta_met_y(),        //
                jets.get_delta_met_x(),           //
                jets.get_delta_met_y(),           //
                bjets.get_delta_met_x(),          //
                bjets.get_delta_met_y(),          //
                is_data,                          //
                year,                             //
                diff_shift);

            // clear objects
            electrons.clear(muons, 0.4);
            taus.clear(electrons, 0.4);
            taus.clear(muons, 0.4);
            photons.clear(taus, 0.4);
            photons.clear(electrons, 0.4);
            photons.clear(muons, 0.4);
            jets.clear(photons, 0.5);
            bjets.clear(photons, 0.5);
            jets.clear(taus, 0.5);
            bjets.clear(taus, 0.5);
            jets.clear(electrons, 0.5);
            bjets.clear(electrons, 0.5);
            jets.clear(muons, 0.5);
            bjets.clear(muons, 0.5);

            // check for trigger matching
            // is_good_trigger is garantied to be filled by the "if" statement above
            const auto trigger_matches =
                make_trigger_matches(*is_good_trigger, muons, electrons, taus, photons, get_runyear(year));

            bool has_trigger_match = false;
            for (auto &&[trigger_path, trigger_match] : trigger_matches)
            {
                if (trigger_match)
                {
                    has_trigger_match = true;
                    break;
                }
            }

            if (not(has_trigger_match))
            {
                continue;
            }

            // Here goes the real analysis...
            auto do_classification = [&](KinematicsBuffer &buffer,
                                         std::size_t num_muon,
                                         std::size_t num_electron,
                                         std::size_t num_tau,
                                         std::size_t num_photon,
                                         std::size_t num_bjet,
                                         std::size_t num_jet,
                                         std::size_t num_met) -> void
            {
                auto [event_class_name_exclusive, event_class_name_inclusive, event_class_name_jetinclusive] =
                    NanoEventClass::make_event_class_name({num_muon, muons.size()},         //
                                                          {num_electron, electrons.size()}, //
                                                          {num_tau, taus.size()},           //
                                                          {num_photon, photons.size()},     //
                                                          {num_jet, jets.size()},           //
                                                          {num_bjet, bjets.size()},         //
                                                          {num_met, met.size()});

                for (auto &&const_shift : shifts.get_constant_shifts(diff_shift))
                {
                    if (not(const_shift == Shifts::Variations::Nominal or diff_shift == Shifts::Variations::Nominal))
                    {
                        continue;
                    }

                    auto shift = Shifts::resolve_shifts(const_shift, diff_shift);

                    // get effective event weight
                    double weight = 1.;
                    if (not(is_data))
                    {
                        // get trigger SF
                        auto trigger_sf = 1.;

                        auto pu_weight =
                            pu_corrector->evaluate({unwrap(Pileup_nTrueInt), Shifts::get_pu_variation(shift)});

                        auto prefiring_weight = Shifts::get_prefiring_weight(unwrap_or(L1PreFiringWeight_Nom, 1.), //
                                                                             unwrap_or(L1PreFiringWeight_Up, 1.),  //
                                                                             unwrap_or(L1PreFiringWeight_Dn, 1.),
                                                                             shift);

                        auto pdf_as_weight = Shifts::get_pdf_alpha_s_weights(
                            shift,
                            lha_indexes,
                            default_pdf_sets,           //
                            unwrap(LHEPdfWeight),       //
                            unwrap(Generator_scalePDF), //
                            unwrap(Generator_x1),       //
                            unwrap(Generator_x2),       //
                            unwrap(Generator_id1),      //
                            unwrap(Generator_id2),      //
                            this_sample_pdf
                            // unwrap_or(LHEWeight_originalXWGTUP,
                            //                                                                1.f)
                        );
                        double mc_weight =
                            [is_data, &genWeight, &LHEWeight_originalXWGTUP, &should_use_LHEWeight]() -> double
                        {
                            if (not(is_data))
                            {
                                if (should_use_LHEWeight)
                                {
                                    return unwrap(LHEWeight_originalXWGTUP);
                                }
                                return unwrap(genWeight);
                            }
                            return 1.;
                        }();

                        weight = mc_weight * pu_weight * prefiring_weight * trigger_sf / sum_weights * x_section *
                                 luminosity * filter_eff * k_factor * pdf_as_weight *
                                 Shifts::get_reco_scale_factor(shift,
                                                               {num_muon, muons},
                                                               {num_electron, electrons},
                                                               {num_tau, taus},
                                                               {num_photon, photons},
                                                               {num_bjet, bjets},
                                                               {num_jet, jets},
                                                               {num_met, met}) *
                                 Shifts::get_fakes_variation_weight(shift,
                                                                    {num_muon, muons},
                                                                    {num_electron, electrons},
                                                                    {num_tau, taus},
                                                                    {num_photon, photons},
                                                                    {num_bjet, bjets},
                                                                    {num_jet, jets} //  {num_met, met}
                                                                    ) *
                                 Shifts::get_qcd_scale_weight(shift, unwrap(LHEScaleWeight));
                    }

                    // Check for NaNs
                    if (std::isnan(weight) or std::isinf(weight))
                    {
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr,
                                   "ERROR: NaN or INF weight found when "
                                   "processing shift: {}!\n",
                                   Shifts::variation_to_string(shift));
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr, "##########################\n");
                        std::exit(EXIT_FAILURE);
                    }

                    // fill event classes
                    for (auto &&class_name :
                         {event_class_name_exclusive, event_class_name_inclusive, event_class_name_jetinclusive})
                    {
                        if (not(class_name))
                        {
                            continue;
                        }

                        // create event class, if does not exists
                        if (not(event_classes.has_ec(*class_name)))
                        {
                            event_classes.push(*class_name);
                        }

                        // fill class
                        event_classes.unsafe_ec(*class_name)
                            .push(buffer.sum_pt(), buffer.mass(), buffer.met(), weight, shift);
                    }
                }
            };

            loop_over_object_combinations(do_classification, muons, electrons, taus, photons, bjets, jets, met);
        }

        global_event_index++;
    }

    fmt::print("\n[MUSiC Classification] Done ...\n");
    fmt::print("\n\nProcessed {} events ...\n", global_event_index);
    PrintProcessInfo();
}

PYBIND11_MODULE(classification_imp, m)
{
    m.def("classification",
          &classification,
          "process"_a,
          "year"_a,
          "is_data"_a,
          "x_section"_a,
          "filter_eff"_a,
          "k_factor"_a,
          "luminosity"_a,
          "xs_order"_a,
          "process_group"_a,
          "sum_weights_json_filepath"_a,
          "input_file"_a,
          "event_classes"_a,
          "first_event"_a = std::nullopt,
          "last_event"_a = std::nullopt,
          "debug"_a = false,
          "Entry point for classification code.");
    m.doc() = "python bindings for classification";

    py::class_<EventClassContainer>(m, "EventClassContainer")
        .def(py::init<>())
        .def("merge_inplace", &EventClassContainer::merge_inplace, "event_classes_container"_a)
        .def_static(
            "serialize",
            [](EventClassContainer &cont) -> py::bytes
            {
                return py::bytes(EventClassContainer::serialize(cont));
            },
            "event_classes_container"_a)
        .def_static(
            "deserialize",
            [](const py::bytes &bytes) -> EventClassContainer
            {
                return EventClassContainer::deserialize(bytes);
            },
            "bytes"_a)
        .def_static(
            "serialize_to_root",
            [](EventClassContainer &cont,
               const std::string &ouput_file_path,
               const std::string &process_name,
               const std::string &process_group,
               const std::string &xsec_order,
               const std::string &year,
               bool is_data) -> void
            {
                EventClassContainer::serialize_to_root(
                    cont, ouput_file_path, process_name, process_group, xsec_order, year, is_data);
            },
            "event_classes_container"_a,
            "ouput_file_path"_a,
            "process_name"_a,
            "process_group"_a,
            "xsec_order"_a,
            "year"_a,
            "is_data"_a);
}
