#include "HeavyValidation.hpp"

#include "Configs.hpp"
#include "Math/Vector4Dfwd.h"
#include "Outputs.hpp"
#include "ROOT/RVec.hxx"
#include "RtypesCore.h"
#include "TFile.h"
#include "TH1.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "fmt/core.h"
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

auto starts_with(const std::string &str, const std::string &prefix) -> bool
{
    return str.compare(0, prefix.length(), prefix) == 0;
}

inline auto trigger_filter(const std::string &process,
                           bool is_data,
                           bool pass_low_pt_muon_trigger,
                           bool pass_high_pt_muon_trigger,
                           bool pass_low_pt_electron_trigger,
                           bool pass_high_pt_electron_trigger) -> bool
{
    if (is_data)
    {
        // Electron/Photon dataset
        if (process.find("EGamma") != std::string::npos      //
            or process.find("Electron") != std::string::npos //
            or process.find("Photon") != std::string::npos)
        {
            return not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger) and
                   (pass_low_pt_electron_trigger or pass_high_pt_electron_trigger);
        }

        // Muon dataset
        return (pass_low_pt_muon_trigger or pass_high_pt_muon_trigger) and
               not(pass_low_pt_electron_trigger or pass_high_pt_electron_trigger);
    }

    // MC
    return (pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_low_pt_electron_trigger or
            pass_high_pt_electron_trigger);
};

inline auto jets_trigger_filter(bool pass_jet_ht_trigger, bool pass_jet_pt_trigger) -> bool
{
    return pass_jet_ht_trigger or pass_jet_pt_trigger;
};

auto main(int argc, char *argv[]) -> int
{
    // silence LHAPDF
    LHAPDF::setVerbosity(0);

    // set SumW2 as default
    TH1::SetDefaultSumw2(true);

    // command line options
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    const std::string process = cmdl({"-p", "--process"}).str();
    const std::string year = cmdl({"-y", "--year"}).str();
    const bool is_data = cmdl[{"-d", "--is_data"}];
    const std::string output_path = cmdl({"-o", "--output"}).str();
    const std::string x_section_str = cmdl({"-x", "--xsection"}).str();
    const std::string filter_eff_str = cmdl({"-x", "--filter_eff"}).str();
    const std::string k_factor_str = cmdl({"-x", "--k_factor"}).str();
    const std::string luminosity_str = cmdl({"-x", "--luminosity"}).str();
    const std::string process_order = cmdl({"-x", "--process_order"}).str();
    const std::string process_group = cmdl({"-x", "--process_group"}).str();
    const std::string input_file = cmdl({"-i", "--input"}).str();
    const bool debug = cmdl[{"--debug"}];

    if (show_help or process == "" or year == "" or output_path == "" or input_file == "" or x_section_str == "" or
        filter_eff_str == "" or k_factor_str == "" or luminosity_str == "" or process_order == "" or
        process_group == "")
    {
        fmt::print("Usage: heavy_validation [OPTIONS]\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -p|--process: Process (aka sample).\n");
        fmt::print("          -y|--year: Year.\n");
        fmt::print("          -d|--is_data: Is data ?\n");
        fmt::print("          -o|--output: Output path.\n");
        fmt::print("          -x|--xsection: cross-section.\n");
        fmt::print("          -x|--filter_eff: Generator level filter efficiency.\n");
        fmt::print("          -x|--k_factor: K-Factor.\n");
        fmt::print("          -x|--luminosity: Int. luminosity.\n");
        fmt::print("          -x|--process_order: Process order.\n");
        fmt::print("          -x|--process_group: Process group.\n");
        fmt::print("          -i|--input: Path to a txt with input files (one per line).\n");
        fmt::print("          --debug: Run in debug mode..\n");

        exit(-1);
    }
    const double x_section = std::stod(x_section_str);
    const double filter_eff = std::stod(filter_eff_str);
    const double k_factor = std::stod(k_factor_str);
    const double luminosity = std::stod(luminosity_str);

    // create tree reader and add values and arrays
    TChain input_chain("nano_music");

    for (auto &&file : load_input_files(input_file))
    {
        input_chain.Add(file.c_str());
    }

    auto tree_reader = TTreeReader(&input_chain);

    ADD_VALUE_READER(pass_low_pt_muon_trigger, bool);
    ADD_VALUE_READER(pass_high_pt_muon_trigger, bool);
    ADD_VALUE_READER(pass_low_pt_electron_trigger, bool);
    ADD_VALUE_READER(pass_high_pt_electron_trigger, bool);
    ADD_VALUE_READER(pass_jet_ht_trigger, bool);
    ADD_VALUE_READER(pass_jet_pt_trigger, bool);

    ADD_VALUE_READER(gen_weight, float);
    ADD_VALUE_READER(Pileup_nTrueInt, float);
    ADD_VALUE_READER(L1PreFiringWeight_Up, float);
    ADD_VALUE_READER(L1PreFiringWeight_Dn, float);
    ADD_VALUE_READER(L1PreFiringWeight_Nom, float);

    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);
    ADD_ARRAY_READER(Muon_tightId, bool);
    ADD_ARRAY_READER(Muon_highPtId, UChar_t);
    ADD_ARRAY_READER(Muon_pfRelIso04_all, float);
    ADD_ARRAY_READER(Muon_tkRelIso, float);
    ADD_ARRAY_READER(Muon_tunepRelPt, float);
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

    ADD_VALUE_READER(MET_pt, float);
    ADD_VALUE_READER(MET_phi, float);

    const auto shifts = Shifts(is_data);

    // corrections
    auto correctionlib_utils = CorrectionLibUtils();
    auto jet_corrections = JetCorrector(get_runyear(year), get_era_from_process_name(process, is_data), is_data);
    auto pu_corrector = correctionlib_utils.make_correctionlib_ref("PU", year);
    auto muon_sf_reco = correctionlib_utils.make_correctionlib_ref("MuonReco", year);
    auto muon_sf_id_low_pt = correctionlib_utils.make_correctionlib_ref("MuonIdLowPt", year);
    auto muon_sf_id_high_pt = correctionlib_utils.make_correctionlib_ref("MuonIdHighPt", year);
    auto muon_sf_iso_low_pt = correctionlib_utils.make_correctionlib_ref("MuonIsoLowPt", year);
    auto muon_sf_iso_high_pt = correctionlib_utils.make_correctionlib_ref("MuonIsoHighPt", year);
    auto electron_sf = correctionlib_utils.make_correctionlib_ref("ElectronSF", year);
    auto photon_sf = correctionlib_utils.make_correctionlib_ref("PhotonSF", year);
    auto pixel_veto_sf = correctionlib_utils.make_correctionlib_ref("PixelVetoSF", year);

    const std::map<std::string, int> z_to_mu_mu_x_count_map = {{"Ele", 0},
                                                               {"EleEE", 0},
                                                               {"EleEB", 0},
                                                               {"Muon", 2},
                                                               {"Gamma", 0},
                                                               {"GammaEB", 0},
                                                               {"GammaEE", 0},
                                                               {"Tau", 0},
                                                               {"Jet", 0},
                                                               {"bJet", 0},
                                                               {"MET", 0}};

    const std::map<std::string, int> z_to_ele_ele_x_count_map = {{"Ele", 2},
                                                                 {"EleEE", 0},
                                                                 {"EleEB", 0},
                                                                 {"Muon", 0},
                                                                 {"Gamma", 0},
                                                                 {"GammaEB", 0},
                                                                 {"GammaEE", 0},
                                                                 {"Tau", 0},
                                                                 {"Jet", 0},
                                                                 {"bJet", 0},
                                                                 {"MET", 0}};

    const std::map<std::string, int> dijets_count_map = {{"Ele", 0},
                                                         {"EleEE", 0},
                                                         {"EleEB", 0},
                                                         {"Muon", 0},
                                                         {"Gamma", 0},
                                                         {"GammaEB", 0},
                                                         {"GammaEE", 0},
                                                         {"Tau", 0},
                                                         {"Jet", 2},
                                                         {"bJet", 0},
                                                         {"MET", 0}};

    // build DY analysis
    auto z_to_mu_mu_x = ZToLepLepX(fmt::format("{}/z_to_mu_mu_x_{}_{}.root", output_path, process, year),
                                   z_to_mu_mu_x_count_map,
                                   false,
                                   shifts.get_shifts(),
                                   process,
                                   year);
    auto z_to_mu_mu_x_Z_mass = ZToLepLepX(fmt::format("{}/z_to_mu_mu_x_Z_mass_{}_{}.root", output_path, process, year),
                                          z_to_mu_mu_x_count_map,
                                          true,
                                          shifts.get_shifts(),
                                          process,
                                          year);

    auto z_to_ele_ele_x = ZToLepLepX(fmt::format("{}/z_to_ele_ele_x_{}_{}.root", output_path, process, year),
                                     z_to_ele_ele_x_count_map,
                                     false,
                                     shifts.get_shifts(),
                                     process,
                                     year);
    auto z_to_ele_ele_x_Z_mass =
        ZToLepLepX(fmt::format("{}/z_to_ele_ele_x__Z_mass_{}_{}.root", output_path, process, year),
                   z_to_ele_ele_x_count_map,
                   true,
                   shifts.get_shifts(),
                   process,
                   year);

    // build Dijets
    // auto dijets = Dijets(fmt::format("{}/dijets_{}_{}.root", output_path, process, year), dijets_count_map);

    const auto cutflow_file =
        std::unique_ptr<TFile>(TFile::Open(fmt::format("{}/cutflow_{}_{}.root", output_path, process, year).c_str()));
    const auto cutflow_histo = cutflow_file->Get<TH1F>("cutflow");
    // const auto total_generator_weight = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorWeight") + 1);
    const auto no_cuts = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("NoCuts") + 1);
    const auto generator_filter = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorFilter") + 1);

    //  launch event loop for Data or MC
    for (auto &&event : tree_reader)
    {
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

        // remove the "unused variable" warning during compilation
        static_cast<void>(event);

        if (event > 500000)
        {
            break;
        }

        // Trigger
        //
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //
        // ATTENTION:The function "trigger_filter" should be modified
        // for the JetHT data set!!!!!!
        //
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //
        // pass_low_pt_muon_trigger
        // pass_high_pt_muon_trigger
        // pass_low_pt_electron_trigger
        // pass_high_pt_electron_trigger
        // pass_jet_ht_trigger
        // pass_jet_pt_trigger
        bool is_good_trigger = trigger_filter(process,
                                              is_data,
                                              unwrap(pass_low_pt_muon_trigger),
                                              unwrap(pass_high_pt_muon_trigger),
                                              unwrap(pass_low_pt_electron_trigger),
                                              unwrap(pass_high_pt_electron_trigger));

        // bool is_good_trigger = jets_trigger_filter(unwrap(pass_jet_ht_trigger), //
        //                                            unwrap(pass_jet_pt_trigger));

        if (not(is_good_trigger))
        {
            continue;
        }

        // muons
        // build good objects
        auto nominal_muons = ObjectFactories::make_muons(unwrap(Muon_pt),             //
                                                         unwrap(Muon_eta),            //
                                                         unwrap(Muon_phi),            //
                                                         unwrap(Muon_tightId),        //
                                                         unwrap(Muon_highPtId),       //
                                                         unwrap(Muon_pfRelIso04_all), //
                                                         unwrap(Muon_tkRelIso),       //
                                                         unwrap(Muon_tunepRelPt),     //
                                                         unwrap(Muon_genPartIdx),     //
                                                         muon_sf_reco,                //
                                                         muon_sf_id_low_pt,           //
                                                         muon_sf_id_high_pt,          //
                                                         muon_sf_iso_low_pt,          //
                                                         muon_sf_iso_high_pt,         //
                                                         is_data,                     //
                                                         year,                        //
                                                         "Nominal");

        auto nominal_electrons = ObjectFactories::make_electrons(unwrap(Electron_pt),            //
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
                                                                 "Nominal");

        auto nominal_photons = ObjectFactories::make_photons(unwrap(Photon_pt),          //
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
                                                             "Nominal");

        auto [nominal_jets, nominal_bjets] = ObjectFactories::make_jets(unwrap(Jet_pt),                 //
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
                                                                        "Nominal");

        auto nominal_met = ObjectFactories::make_met(unwrap(MET_pt),                      //
                                                     unwrap(MET_phi),                     //
                                                     nominal_muons.get_delta_met_x(),     //
                                                     nominal_muons.get_delta_met_y(),     //
                                                     nominal_electrons.get_delta_met_x(), //
                                                     nominal_electrons.get_delta_met_y(), //
                                                     nominal_photons.get_delta_met_x(),   //
                                                     nominal_photons.get_delta_met_y(),   //
                                                     nominal_jets.get_delta_met_x(),      //
                                                     nominal_jets.get_delta_met_y(),      //
                                                     nominal_bjets.get_delta_met_x(),     //
                                                     nominal_bjets.get_delta_met_y(),     //
                                                     is_data,                             //
                                                     year,                                //
                                                     "Nominal");

        // loop over each shift (systematic uncerts)
        for (const auto &shift : shifts)
        {
            // recalculate shift specific data
            MUSiCObjects *muons = &nominal_muons;
            MUSiCObjects *electrons = &nominal_electrons;
            MUSiCObjects *photons = &nominal_photons;
            MUSiCObjects *jets = &nominal_jets;
            MUSiCObjects *bjets = &nominal_bjets;
            MUSiCObjects *met = &nominal_met;

            MUSiCObjects shifted_muons = MUSiCObjects();
            MUSiCObjects shifted_electrons = MUSiCObjects();
            MUSiCObjects shifted_photons = MUSiCObjects();
            MUSiCObjects shifted_jets = MUSiCObjects();
            MUSiCObjects shifted_bjets = MUSiCObjects();
            MUSiCObjects shifted_met = MUSiCObjects();

            bool is_differential_shift = false;

            if (starts_with(shift, "Muon"))
            {
                shifted_muons = ObjectFactories::make_muons(unwrap(Muon_pt),             //
                                                            unwrap(Muon_eta),            //
                                                            unwrap(Muon_phi),            //
                                                            unwrap(Muon_tightId),        //
                                                            unwrap(Muon_highPtId),       //
                                                            unwrap(Muon_pfRelIso04_all), //
                                                            unwrap(Muon_tkRelIso),       //
                                                            unwrap(Muon_tunepRelPt),     //
                                                            unwrap(Muon_genPartIdx),     //
                                                            muon_sf_reco,                //
                                                            muon_sf_id_low_pt,           //
                                                            muon_sf_id_high_pt,          //
                                                            muon_sf_iso_low_pt,          //
                                                            muon_sf_iso_high_pt,         //
                                                            is_data,                     //
                                                            year,                        //
                                                            shift);
                muons = &shifted_muons;
                is_differential_shift = true;
            }

            if (starts_with(shift, "Electron"))
            {
                shifted_electrons = ObjectFactories::make_electrons(unwrap(Electron_pt),            //
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
                                                                    shift);
                electrons = &shifted_electrons;
                is_differential_shift = true;
            }

            if (starts_with(shift, "Photon"))
            {
                shifted_photons = ObjectFactories::make_electrons(unwrap(Electron_pt),            //
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
                                                                  shift);
                photons = &shifted_photons;
                is_differential_shift = true;
            }

            if (starts_with(shift, "Jet"))
            {
                auto [_shifted_jets, _shifted_bjets] =
                    ObjectFactories::make_jets(unwrap(Jet_pt),                 //
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
                                               shift);

                shifted_jets = _shifted_jets;
                shifted_bjets = _shifted_bjets;
                jets = &shifted_jets;
                bjets = &shifted_bjets;
                is_differential_shift = true;
            }

            // remove overlaps
            electrons->clear(muons);
            photons->clear(electrons);
            photons->clear(muons);
            jets->clear(photons, 0.5);
            bjets->clear(photons, 0.5);
            jets->clear(electrons, 0.5);
            bjets->clear(electrons, 0.5);
            jets->clear(muons, 0.5);
            bjets->clear(muons, 0.5);

            if (is_differential_shift)
            {
                shifted_met = ObjectFactories::make_met(unwrap(MET_pt),               //
                                                        unwrap(MET_phi),              //
                                                        muons->get_delta_met_x(),     //
                                                        muons->get_delta_met_y(),     //
                                                        electrons->get_delta_met_x(), //
                                                        electrons->get_delta_met_y(), //
                                                        photons->get_delta_met_x(),   //
                                                        photons->get_delta_met_y(),   //
                                                        jets->get_delta_met_x(),      //
                                                        jets->get_delta_met_y(),      //
                                                        bjets->get_delta_met_x(),     //
                                                        bjets->get_delta_met_y(),     //
                                                        is_data,                      //
                                                        year,                         //
                                                        shift);
                met = &shifted_met;
            }

            // Here goes the real analysis...
            // get effective event weight
            double weight = 1.;

            if (not(is_data))
            {
                auto pu_weight = pu_corrector->evaluate({unwrap(Pileup_nTrueInt), Shifts::get_pu_variation(shift)});

                auto prefiring_weight = Shifts::get_prefiring_weight( //
                    unwrap(L1PreFiringWeight_Nom, 1.),                //
                    unwrap(L1PreFiringWeight_Up, 1.),                 //
                    unwrap(L1PreFiringWeight_Dn, 1.),                 //
                    shift);

                weight = unwrap(gen_weight, 1.) * pu_weight * prefiring_weight * generator_filter / no_cuts /
                         generator_filter * x_section * filter_eff * k_factor *
                         Shifts::scale_luminosity(luminosity, shift);
            }

            // MuMu + X
            if (muons->size() >= 2)
            {
                auto muon_1 = muons->p4[0];
                auto muon_2 = muons->p4[1];

                // wide mass range
                z_to_mu_mu_x.fill(muon_1, muon_2, bjets->p4, jets->p4, met->p4, weight, shift);

                // Z mass range
                if (PDG::Z::Mass - 20. < (muon_1 + muon_2).mass() and (muon_1 + muon_2).mass() < PDG::Z::Mass + 20.)
                {
                    z_to_mu_mu_x_Z_mass.fill(muon_1, muon_2, bjets->p4, jets->p4, met->p4, weight, shift);
                }
            }

            // EleEle + X
            if (electrons->size() >= 2)
            {
                auto electron_1 = electrons->p4[0];
                auto electron_2 = electrons->p4[1];

                // wide mass range
                z_to_ele_ele_x.fill(electron_1, electron_2, bjets->p4, jets->p4, met->p4, weight, shift);

                // Z mass range
                if (PDG::Z::Mass - 20. < (electron_1 + electron_2).mass() and
                    (electron_1 + electron_2).mass() < PDG::Z::Mass + 20.)
                {
                    z_to_ele_ele_x_Z_mass.fill(electron_1, electron_2, bjets->p4, jets->p4, met->p4, weight, shift);
                }
            }

            // // Dijets
            // if (jets.size() >= 2)
            // {
            //     auto jet_1 = jets.p4[0];
            //     auto jet_2 = jets.p4[1];

            //     if ((jet_1.pt() > 600.) and std::fabs(jet_1.eta() - jet_2.eta()) < 1.1)
            //     {
            //         dijets.fill(jet_1, jet_2, std::nullopt, weight);
            //     }
            // }
        }

        // process monitoring
        if (debug)
        {
            if ((event < 10) or                           //
                (event < 100 && event % 10 == 0) or       //
                (event < 1000 && event % 100 == 0) or     //
                (event < 10000 && event % 1000 == 0) or   //
                (event < 100000 && event % 10000 == 0) or //
                (event >= 100000 && event % 100000 == 0)  //
            )
            {
                fmt::print("\n\nProcessed {} events ...\n", event);
                PrintProcessInfo();
            }
        }
    }

    fmt::print("\n[MUSiC Validation] Saving outputs ({} - {} - {}) ...\n", output_path, process, year);
    z_to_mu_mu_x.dump_outputs();
    z_to_mu_mu_x_Z_mass.dump_outputs();
    // dijets.dump_outputs();

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}