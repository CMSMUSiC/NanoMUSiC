#include "ROOT/RVec.hxx"
#include "TH1.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "Validation.hpp"

auto main(int argc, char *argv[]) -> int
{
    std::cout << emojicpp::emojize("\n\n              :signal_strength: [ MUSiC Validation ] :signal_strength:")
              << std::endl;

    // command line options
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    const std::string config_file = cmdl({"-c", "--config"}).str();

    if (show_help or config_file == "")
    {
        fmt::print("Usage: validation [-h|--help] --config <config_file>\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -c|--config: Input file.\n");
        return (config_file == "" ? -1 : 0);
    }

    // task configuration
    const auto task_config = TOMLConfig::make_toml_config(config_file);
    const auto output_folder = task_config.get<std::string>("output");
    const auto is_data = task_config.get<bool>("is_data");
    const auto x_section_file(MUSiCTools::parse_and_expand_music_base(task_config.get<std::string>("x_section_file")));
    const auto process = task_config.get<std::string>("process");
    const auto input_files = task_config.get_vector<std::string>("input_files");

    fmt::print("********************* Task Configuration *********************\n");
    fmt::print("Process: {}\n", process);
    fmt::print("Output Folder: {}\n", output_folder);
    fmt::print("Is Data(?): {}\n", is_data);
    // fmt::print("Cross-sections file: {}\n", x_section_file);
    fmt::print("**************************************************************\n");

    fmt::print("\n[MUSiC Validation] Loading input files ...\n");
    auto chain = TChain("nano_music");
    for (auto &&file : input_files)
    {
        chain.Add(file.c_str());
    }

    // create tree reader and add values and arrays
    auto tree_reader = TTreeReader(&chain);

    ADD_VALUE_READER(nMuon, unsigned int);
    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);

    ADD_VALUE_READER(nElectron, unsigned int);
    ADD_ARRAY_READER(Electron_pt, float);
    ADD_ARRAY_READER(Electron_eta, float);
    ADD_ARRAY_READER(Electron_phi, float);

    // ADD_VALUE_READER(nPhoton, unsigned int);
    // ADD_ARRAY_READER(Photon_pt, float);
    // ADD_ARRAY_READER(Photon_eta, float);
    // ADD_ARRAY_READER(Photon_phi, float);

    ADD_VALUE_READER(nBJet, unsigned int);
    ADD_ARRAY_READER(BJet_pt, float);
    ADD_ARRAY_READER(BJet_eta, float);
    ADD_ARRAY_READER(BJet_phi, float);

    ADD_VALUE_READER(nJet, unsigned int);
    ADD_ARRAY_READER(Jet_pt, float);
    ADD_ARRAY_READER(Jet_eta, float);
    ADD_ARRAY_READER(Jet_phi, float);

    ADD_VALUE_READER(nMET, unsigned int);
    ADD_ARRAY_READER(MET_pt, float);

    fmt::print("\n[MUSiC Validation] Initializing Z -> MuMU + X ...\n");
    auto z_to_mu_mu_x = ZToLepLepX();

    fmt::print("\n[MUSiC Validation] Initializing Z -> EleEle + X ...\n");
    auto z_to_ele_ele_x = ZToLepLepX();

    fmt::print("\n[MUSiC Validation] Initializing Z -> MuMU + X (Z mass) ...\n");
    auto z_to_mu_mu_x_Z_mass = ZToLepLepX();

    fmt::print("\n[MUSiC Validation] Initializing Z -> EleEle (Z mass) + X ...\n");
    auto z_to_ele_ele_x_Z_mass = ZToLepLepX();

    fmt::print("\n[MUSiC Validation] Starting timer  ...\n");
    // auto dTime1 = getCpuTime(); // Start Timer

    // launch event loop for Data or MC
    fmt::print("\n[MUSiC Validation] Launching event loop ...\n");

    // const auto total_number_of_events = chain.GetEntries();
    for (auto &&event : tree_reader)
    {
        (void)event; // remove the unused warning during compilation

        // reorder objects
        // muons
        auto muon_reordering_mask = VecOps::Argsort(unwrap(Muon_pt));
        auto muon_pt = unwrap(Muon_pt)[muon_reordering_mask];
        auto muon_eta = unwrap(Muon_eta)[muon_reordering_mask];
        auto muon_phi = unwrap(Muon_phi)[muon_reordering_mask];

        // electrons
        auto electron_reordering_mask = VecOps::Argsort(unwrap(Electron_pt));
        auto electron_pt = unwrap(Electron_pt)[electron_reordering_mask];
        auto electron_eta = unwrap(Electron_eta)[electron_reordering_mask];
        auto electron_phi = unwrap(Electron_phi)[electron_reordering_mask];

        // // photons
        // auto photon_reordering_mask = VecOps::Argsort(unwrap(Photon_pt));
        // auto photon_pt = unwrap(Photon_pt)[photon_reordering_mask];
        // auto photon_eta = unwrap(Photon_eta)[photon_reordering_mask];
        // auto photon_phi = unwrap(Photon_phi)[photon_reordering_mask];

        // bjets
        auto bjet_reordering_mask = VecOps::Argsort(unwrap(BJet_pt));
        auto bjet_pt = unwrap(BJet_pt)[bjet_reordering_mask];
        auto bjet_eta = unwrap(BJet_eta)[bjet_reordering_mask];
        auto bjet_phi = unwrap(BJet_phi)[bjet_reordering_mask];

        // jets
        auto jet_reordering_mask = VecOps::Argsort(unwrap(Jet_pt));
        auto jet_pt = unwrap(Jet_pt)[jet_reordering_mask];
        auto jet_eta = unwrap(Jet_eta)[jet_reordering_mask];
        auto jet_phi = unwrap(Jet_phi)[jet_reordering_mask];

        // MET
        auto met_pt = unwrap(MET_pt);

        // process monitoring
        // if (event < 10 || (event < 100 && event % 10 == 0) || (event < 1000 && event % 100 == 0) ||
        // (event < 10000 && event % 1000 == 0) || (event >= 10000 && event % 10000 == 0))
        // {
        // fmt::print("\n\n[MUSiC Validation] Processed {} out of {} events.\n", event, total_number_of_events);
        // PrintProcessInfo();
        // }
    }
    fmt::print("\n[MUSiC Validation] Saving outputs ...\n");
    z_to_mu_mu_x.dump_outputs(fmt::format("{}/z_to_mu_mu_x.root", output_folder));
    z_to_ele_ele_x.dump_outputs(fmt::format("{}/z_to_mu_mu_x.root", output_folder));
    z_to_mu_mu_x_Z_mass.dump_outputs(fmt::format("{}/z_to_mu_mu_x.root", output_folder));
    z_to_ele_ele_x_Z_mass.dump_outputs(fmt::format("{}/z_to_mu_mu_x.root", output_folder));

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();
    return 0;
}