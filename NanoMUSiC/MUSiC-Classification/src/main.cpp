#include "Classification.hpp"

#include "Configs.hpp"
#include "Math/Vector4Dfwd.h"
#include "Outputs.hpp"
#include "ROOT/RVec.hxx"
#include "RtypesCore.h"
#include "TFile.h"
#include "TH1.h"
#include "TROOT.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "fmt/core.h"
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <type_traits>
#include <unordered_map>

auto main(int argc, char *argv[]) -> int
{
    // set SumW2 as default
    TH1::SetDefaultSumw2(true);

    // command line options
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    const std::string process = cmdl({"-p", "--process"}).str();
    const std::string year = cmdl({"-y", "--year"}).str();
    const bool is_data = cmdl[{"-d", "--is_data"}];
    const std::string output_path = cmdl({"-o", "--output"}).str();
    const std::string buffer_index = cmdl({"-b", "--buffer"}).str();
    const std::string x_section_str = cmdl({"-x", "--xsection"}).str();
    const std::string filter_eff_str = cmdl({"-f", "--filter_eff"}).str();
    const std::string k_factor_str = cmdl({"-k", "--k_factor"}).str();
    const std::string luminosity_str = cmdl({"-l", "--luminosity"}).str();
    const std::string xs_order = cmdl({"-po", "--xs_order"}).str();
    const std::string process_group = cmdl({"-pg", "--process_group"}).str();
    const std::string input_file = cmdl({"-i", "--input"}).str();
    const bool debug = cmdl[{"--debug"}];

    if (show_help or process == "" or year == "" or output_path == "" or input_file == "" or x_section_str == "" or
        filter_eff_str == "" or k_factor_str == "" or luminosity_str == "" or xs_order == "" or process_group == "" or
        buffer_index == "")
    {
        fmt::print("Usage: classification [OPTIONS]\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -p|--process: Process (aka sample).\n");
        fmt::print("          -y|--year: Year.\n");
        fmt::print("          -d|--is_data: Is data ?\n");
        fmt::print("          -o|--output: Output path.\n");
        fmt::print("          -x|--xsection: cross-section.\n");
        fmt::print("          -f|--filter_eff: Generator level filter efficiency.\n");
        fmt::print("          -k|--k_factor: K-Factor.\n");
        fmt::print("          -l|--luminosity: Int. luminosity.\n");
        fmt::print("          -po|--xs_order: Process order.\n");
        fmt::print("          -pg|--process_group: Process group.\n");
        fmt::print("          -i|--input: Path to a .txt with input files (one per line).\n");
        fmt::print("          --debug: Run in debug mode..\n\n");

        exit(-1);
    }

    const double x_section = std::stod(x_section_str);
    const double filter_eff = std::stod(filter_eff_str);
    const double k_factor = std::stod(k_factor_str);
    const double luminosity = std::stod(luminosity_str);

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

    // [EVENT_CLASS_NAME, [SHIFT, EVENT_CLASS] ]
    using ClassCollection_t =
        std::array<std::unique_ptr<EventClass>, static_cast<std::size_t>(Shifts::Variations::kTotalVariations)>;
    std::unordered_map<std::string, ClassCollection_t> event_classes;
    event_classes.reserve(4000);

    auto cutflow_file = std::unique_ptr<TFile>(
        TFile::Open(fmt::format("{}/../../cutflow_{}_{}.root", output_path, process, year).c_str()));
    const auto cutflow_histo = cutflow_file->Get<TH1F>("cutflow");
    // const auto total_generator_weight = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorWeight") +
    // 1);
    const auto no_cuts = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("NoCuts") + 1);
    const auto generator_filter = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorFilter") + 1);

    // create tree reader and add values and arrays
    auto inputs = load_input_files(input_file);

    long long global_event_number = -1;
    for (auto &&[file, first_event, last_event] : inputs)
    {
        // fmt::print("DEBUG: NEW FILE\n");

        TChain input_chain("Events");
        input_chain.Add(file.c_str());

        auto tree_reader = TTreeReader(&input_chain);

        // check for the available pdf weights in the tree
        std::optional<std::pair<unsigned int, unsigned int>> lha_indexes =
            PDFAlphaSWeights::get_pdf_ids(tree_reader.GetTree());

        std::optional<std::unique_ptr<LHAPDF::PDF>> this_sample_pdf = std::nullopt;
        if (lha_indexes)
        {
            this_sample_pdf = std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(std::get<0>(*lha_indexes)));
        }

        ADD_VALUE_READER(pass_low_pt_muon_trigger, bool);
        ADD_VALUE_READER(pass_high_pt_muon_trigger, bool);
        ADD_VALUE_READER(pass_double_muon_trigger, bool);
        ADD_VALUE_READER(pass_low_pt_electron_trigger, bool);
        ADD_VALUE_READER(pass_high_pt_electron_trigger, bool);
        ADD_VALUE_READER(pass_double_electron_trigger, bool);
        ADD_VALUE_READER(pass_photon_trigger, bool);
        ADD_VALUE_READER(pass_high_pt_tau_trigger, bool);
        ADD_VALUE_READER(pass_double_tau_trigger, bool);
        ADD_VALUE_READER(pass_jet_ht_trigger, bool);
        ADD_VALUE_READER(pass_jet_pt_trigger, bool);

        ADD_VALUE_READER(mc_weight, float);
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
        for (auto &&event : tree_reader)
        {
            if (event < first_event)
            {
                continue;
            }
            else if (event > last_event)
            {
                break;
            }
            global_event_number++;

            // if (global_event_number > 100)
            // {
            //     break;
            // }

            // check for chain readout quaility
            // REFERENCE: https://root.cern.ch/doc/v608/classTTreeReader.html#a568e43c7d7d8b1f511bbbeb92c9094a8
            auto reader_status = tree_reader.GetEntryStatus();
            if (reader_status == TTreeReader::EEntryStatus::kEntryChainFileError or
                reader_status == TTreeReader::EEntryStatus::kEntryNotLoaded or
                reader_status == TTreeReader::EEntryStatus::kEntryNoTree or
                reader_status == TTreeReader::EEntryStatus::kEntryNotFound or
                reader_status == TTreeReader::EEntryStatus::kEntryChainSetupError or
                reader_status == TTreeReader::EEntryStatus::kEntryDictionaryError)
            {
                throw std::runtime_error(fmt::format("ERROR: Could not load TChain entry."));
            }

            // Trigger
            auto is_good_trigger = trigger_filter(process,                               //
                                                  is_data,                               //
                                                  get_runyear(year),                     //
                                                  unwrap(pass_low_pt_muon_trigger),      //
                                                  unwrap(pass_high_pt_muon_trigger),     //
                                                  unwrap(pass_double_muon_trigger),      //
                                                  unwrap(pass_low_pt_electron_trigger),  //
                                                  unwrap(pass_high_pt_electron_trigger), //
                                                  unwrap(pass_double_electron_trigger),  //
                                                  //   unwrap(pass_high_pt_tau_trigger),      //
                                                  false, //
                                                         //   unwrap(pass_double_tau_trigger),       //
                                                  false, //
                                                  unwrap(pass_photon_trigger) //
            );

            if (is_good_trigger)
            {
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

                    auto electrons = ObjectFactories::make_electrons(unwrap(Electron_pt),            //
                                                                     unwrap(Electron_eta),           //
                                                                     unwrap(Electron_phi),           //
                                                                     unwrap(Electron_deltaEtaSC),    //
                                                                     unwrap(Electron_cutBased),      //
                                                                     unwrap(Electron_cutBased_HEEP), //
                                                                     unwrap(Electron_scEtOverPt),    //
                                                                     unwrap(Electron_dEscaleUp),     //
                                                                     unwrap(Electron_dEscaleDown),   //
                                                                     unwrap(Electron_dEsigmaUp),     //
                                                                     unwrap(Electron_dEsigmaDown),   //
                                                                     unwrap(Electron_genPartIdx),    //
                                                                     electron_sf,                    //
                                                                     is_data,                        //
                                                                     year,                           //
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
                                                                    NanoObjects::GenJets(unwrap(GenJet_pt),   //
                                                                                         unwrap(GenJet_eta),  //
                                                                                         unwrap(GenJet_phi)), //
                                                                    is_data,                                  //
                                                                    year,                                     //
                                                                    diff_shift);

                    auto met = ObjectFactories::make_met( //
                                                          // unwrap(MET_pt),                   //
                        unwrap(RawMET_pt),                //
                        // unwrap(MET_phi),                  //
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
                    // is_good_trigger is garantied to be filled by the if statement above
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

                    // Here goes the real analysis...
                    if (has_trigger_match)
                    {
                        loop_over_objects(
                            [&](std::size_t idx_muon,
                                std::size_t idx_electron,
                                std::size_t idx_tau,
                                std::size_t idx_photon,
                                std::size_t idx_bjet,
                                std::size_t idx_jet,
                                std::size_t idx_met) -> void
                            {
                                auto [event_class_name_exclusive,
                                      event_class_name_inclusive,
                                      event_class_name_jetinclusive] =
                                    make_event_class_name({idx_muon, muons.size()},         //
                                                          {idx_electron, electrons.size()}, //
                                                          {idx_tau, taus.size()},           //
                                                          {idx_photon, photons.size()},     //
                                                          {idx_jet, jets.size()},           //
                                                          {idx_bjet, bjets.size()},         //
                                                          {idx_met, met.size()}             //
                                                                                            // ,  trigger_matches //
                                    );

                                for (auto &&const_shift : shifts.get_constant_shifts(diff_shift))
                                {
                                    if (const_shift == Shifts::Variations::Nominal or
                                        diff_shift == Shifts::Variations::Nominal)
                                    {
                                        auto shift = Shifts::resolve_shifts(const_shift, diff_shift);

                                        // get effective event weight
                                        double weight = 1.;

                                        if (not(is_data))
                                        {
                                            // get trigger SF
                                            auto trigger_sf = 1.;

                                            // if
                                            // (trigger_matches.at("pass_low_pt_muon_trigger"))
                                            // {
                                            //     trigger_sf =
                                            //     low_pt_muon_trigger_sf->evaluate(
                                            //
                                            // {ObjectFactories::get_year_for_muon_sf(get_runyear(year)),
                                            //
                                            // std::fabs(trigger_matches.at("pass_low_pt_muon_trigger")->get_matched_eta(0)),
                                            //
                                            // trigger_matches.at("pass_low_pt_muon_trigger")->get_matched_pt(0),
                                            //          "sf"});
                                            // }
                                            // else if
                                            // (trigger_matches.at("pass_high_pt_muon_trigger"))
                                            // {
                                            //     trigger_sf =
                                            //     high_pt_muon_trigger_sf->evaluate(
                                            //
                                            // {ObectFactories::get_year_for_muon_sf(get_runyear(year)),
                                            //
                                            // std::fabs(trigger_matches.at("pass_high_pt_muon_trigger")->get_matched_eta(0)),
                                            //
                                            // trigger_matches.at("pass_high_pt_muon_trigger")->get_matched_pt(0),
                                            //          "sf"});
                                            // }

                                            auto pu_weight = pu_corrector->evaluate(
                                                {unwrap(Pileup_nTrueInt), Shifts::get_pu_variation(shift)});

                                            auto prefiring_weight = Shifts::get_prefiring_weight(
                                                //
                                                unwrap(L1PreFiringWeight_Nom, 1.), //
                                                unwrap(L1PreFiringWeight_Up, 1.),  //
                                                unwrap(L1PreFiringWeight_Dn, 1.),
                                                shift);

                                            auto pdf_as_weight =
                                                Shifts::get_pdf_alpha_s_weights(shift,
                                                                                lha_indexes,
                                                                                default_pdf_sets,           //
                                                                                unwrap(LHEPdfWeight),       //
                                                                                unwrap(Generator_scalePDF), //
                                                                                unwrap(Generator_x1),       //
                                                                                unwrap(Generator_x2),       //
                                                                                unwrap(Generator_id1),      //
                                                                                unwrap(Generator_id2),      //
                                                                                this_sample_pdf
                                                                                // unwrap(LHEWeight_originalXWGTUP, 1.f)
                                                );

                                            weight =
                                                unwrap(mc_weight, 1.) * pu_weight * prefiring_weight * trigger_sf *
                                                generator_filter / no_cuts / generator_filter * x_section * luminosity *
                                                filter_eff * k_factor * pdf_as_weight *
                                                Shifts::get_reco_scale_factor(shift,
                                                                              {idx_muon, muons},
                                                                              {idx_electron, electrons},
                                                                              {idx_tau, taus},
                                                                              {idx_photon, photons},
                                                                              {idx_bjet, bjets},
                                                                              {idx_jet, jets},
                                                                              {idx_met, met}) *
                                                Shifts::get_fakes_variation_weight(shift,
                                                                                   {idx_muon, muons},
                                                                                   {idx_electron, electrons},
                                                                                   {idx_tau, taus},
                                                                                   {idx_photon, photons},
                                                                                   {idx_bjet, bjets},
                                                                                   {idx_jet, jets} //  {idx_met, met}
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
                                        for (auto &&class_name : {event_class_name_exclusive,
                                                                  event_class_name_inclusive,
                                                                  event_class_name_jetinclusive})
                                        {
                                            // create event class, if does not exists
                                            if (class_name)
                                            {
                                                if (event_classes.find(*class_name) == event_classes.end())
                                                {
                                                    event_classes.insert({*class_name, ClassCollection_t()});
                                                    const auto [edges, edges_met] =
                                                        EventClass::make_bin_limits({{"Ele", idx_electron},
                                                                                     {"EleEE", 0},
                                                                                     {"EleEB", 0},
                                                                                     {"Muon", idx_muon},
                                                                                     {"Gamma", 0},
                                                                                     {"GammaEB", idx_photon},
                                                                                     {"GammaEE", 0},
                                                                                     {"Tau", idx_tau},
                                                                                     {"Jet", idx_jet},
                                                                                     {"bJet", idx_bjet},
                                                                                     {"MET", idx_met}});

                                                    shifts.for_each(
                                                        [&](const Shifts::Variations shift) -> void
                                                        {
                                                            if (shift == Shifts::Variations::Nominal)
                                                            {
                                                                event_classes[*class_name][static_cast<std::size_t>(
                                                                    shift)] =
                                                                    std::make_unique<EventClass>(*class_name,
                                                                                                 edges,
                                                                                                 edges_met,
                                                                                                 shift,
                                                                                                 process,
                                                                                                 year,
                                                                                                 process_group,
                                                                                                 xs_order);
                                                            }
                                                            else
                                                            {
                                                                event_classes[*class_name][static_cast<std::size_t>(
                                                                    shift)] =
                                                                    std::make_unique<EventClass>(
                                                                        *(event_classes
                                                                              [*class_name][static_cast<std::size_t>(
                                                                                  Shifts::Variations::Nominal)]));
                                                            }
                                                        });
                                                }

                                                // fill class
                                                event_classes[*class_name][static_cast<std::size_t>(shift)]->fill(
                                                    {idx_muon, muons},
                                                    {idx_electron, electrons},
                                                    {idx_tau, taus},
                                                    {idx_photon, photons},
                                                    {idx_bjet, bjets},
                                                    {idx_jet, jets},
                                                    {idx_met, met},
                                                    weight);
                                            }
                                        }
                                    }
                                }
                            },
                            muons.size(),
                            electrons.size(),
                            taus.size(),
                            photons.size(),
                            bjets.size(),
                            jets.size(),
                            met.size());
                    }
                }
            }

            // process monitoring
            if (debug)
            {
                if ((global_event_number < 10) or                                         //
                    (global_event_number < 100 && global_event_number % 10 == 0) or       //
                    (global_event_number < 1000 && global_event_number % 100 == 0) or     //
                    (global_event_number < 10000 && global_event_number % 1000 == 0) or   //
                    (global_event_number < 100000 && global_event_number % 10000 == 0) or //
                    (global_event_number >= 100000 && global_event_number % 100000 == 0)  //
                )
                {
                    fmt::print("\n\nProcessed {} events ...\n", event);
                    PrintProcessInfo();
                }
            }
            else
            {
                if ((global_event_number > 100000 and global_event_number % 100000 == 0))
                {
                    fmt::print("\n\nProcessed {} events ...\n", global_event_number);
                    PrintProcessInfo();
                }
            }
        }
    }

    fmt::print("\n[MUSiC Classification] Saving outputs ({} - {} - {}) ...\n", process, year, buffer_index);
    auto ec_output_file = std::unique_ptr<TFile>(TFile::Open( //
        get_output_file_path("ec_classes", ".", process, year, process_group, xs_order, is_data, buffer_index)
            .c_str(),              //
        "RECREATE",                //
        "nanomusic_event_classes", //
        0                          //
        ));

    shifts.for_each(
        [&](const Shifts::Variations shift) -> void
        {
            for (auto &&[class_name, class_collection] : event_classes)
            {
                if (is_data)
                {
                    if (shift == Shifts::Variations::Nominal)
                    {
                        class_collection[static_cast<std::size_t>(shift)]->dump_outputs(ec_output_file, shift);
                    }
                }
                else
                {
                    class_collection[static_cast<std::size_t>(shift)]->dump_outputs(ec_output_file, shift);
                }
            }
        });

    ec_output_file->Close();

    fmt::print("\n[MUSiC Classification] Done ...\n");
    fmt::print("\n\nProcessed {} events ...\n", global_event_number);
    PrintProcessInfo();

    return EXIT_SUCCESS;
}
