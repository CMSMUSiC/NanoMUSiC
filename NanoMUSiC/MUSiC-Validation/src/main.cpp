#include "Configs.hpp"
#include "Math/Vector4Dfwd.h"
#include "ROOT/RVec.hxx"
#include "TFile.h"
#include "TH1.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "Validation.hpp"
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>

auto main(int argc, char *argv[]) -> int
{
    // set SumW2 as default
    TH1::SetDefaultSumw2(true);

    std::cout << emojicpp::emojize("\n\n              :signal_strength: [ MUSiC Validation ] :signal_strength:\n\n")
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
    const auto is_data = task_config.get<bool>("is_data");
    const auto x_section_file(MUSiCTools::parse_and_expand_music_base(task_config.get<std::string>("x_section_file")));
    const auto process = task_config.get<std::string>("process");
    const auto input_files = task_config.get_vector<std::string>("input_files");

    fmt::print("********************* Task Configuration *********************\n");
    fmt::print("Process: {}\n", process);
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

    fmt::print("\n[MUSiC Validation] Cleanning output folder ...\n");
    std::filesystem::create_directories("validation_outputs");
    const std::string outputs_path = fmt::format("validation_outputs/{}", process, config_file);
    std::filesystem::remove_all(outputs_path);
    std::filesystem::create_directories(outputs_path);
    std::filesystem::copy_file(config_file, fmt::format("{}/task_config.toml", outputs_path));

    fmt::print("\n[MUSiC Validation] Initializing Z -> MuMU + X ...\n");
    auto z_to_mu_mu_x = ZToLepLepX(fmt::format("{}/z_to_mu_mu_x.root", outputs_path));

    fmt::print("\n[MUSiC Validation] Initializing Z -> EleEle + X ...\n");
    auto z_to_ele_ele_x = ZToLepLepX(fmt::format("{}/z_to_ele_ele_x.root", outputs_path));

    fmt::print("\n[MUSiC Validation] Initializing Z -> MuMU + X (Z mass) ...\n");
    auto z_to_mu_mu_x_Z_mass = ZToLepLepX(fmt::format("{}/z_to_mu_mu_x_Z_mass.root", outputs_path));

    fmt::print("\n[MUSiC Validation] Initializing Z -> EleEle (Z mass) + X ...\n");
    auto z_to_ele_ele_x_Z_mass = ZToLepLepX(fmt::format("{}/z_to_ele_ele_x_Z_mass.root", outputs_path));

    fmt::print("\n[MUSiC Validation] Starting timer  ...\n");
    // auto dTime1 = getCpuTime(); // Start Timer

    fmt::print("\n[MUSiC Validation] Merging cutflow histograms ...\n");
    std::string hadd_command = "hadd -f -T " + outputs_path + "/cutflow.root ";
    for (auto &&file : input_files)
    {
        hadd_command += file + " ";
    }
    int merger_exit_code, ret = std::system(hadd_command.c_str());
    merger_exit_code = WEXITSTATUS(ret);
    if (merger_exit_code != 0)
    {
        throw std::runtime_error(
            fmt::format("ERROR: Could not merge cutflow histograms.\n Command: {}.\n Return code: {}.",
                        hadd_command,
                        merger_exit_code));
    }

    fmt::print("\n[MUSiC Validation] Loading cutflow histograms ...\n");
    auto cutflow_file = std::unique_ptr<TFile>(TFile::Open(fmt::format("{}/cutflow.root", outputs_path).c_str()));
    auto cutflow_histo = cutflow_file->Get<TH1F>("curflow");

    // launch event loop for Data or MC
    fmt::print("\n[MUSiC Validation] Launching event loop ...\n");

    // const auto total_number_of_events = chain.GetEntries();
    for (auto &&event : tree_reader)
    {
        (void)event; // remove the unused warning during compilation

        // reorder objects
        // muons
        const auto muon_reordering_mask = VecOps::Argsort(unwrap(Muon_pt));
        const auto muon_pt = unwrap(Muon_pt)[muon_reordering_mask];
        const auto muon_eta = unwrap(Muon_eta)[muon_reordering_mask];
        const auto muon_phi = unwrap(Muon_phi)[muon_reordering_mask];

        // electrons
        const auto electron_reordering_mask = VecOps::Argsort(unwrap(Electron_pt));
        const auto electron_pt = unwrap(Electron_pt)[electron_reordering_mask];
        const auto electron_eta = unwrap(Electron_eta)[electron_reordering_mask];
        const auto electron_phi = unwrap(Electron_phi)[electron_reordering_mask];

        // // photons
        // const  auto photon_reordering_mask = VecOps::Argsort(unwrap(Photon_pt));
        // const auto photon_pt = unwrap(Photon_pt)[photon_reordering_mask];
        // const auto photon_eta = unwrap(Photon_eta)[photon_reordering_mask];
        // const auto photon_phi = unwrap(Photon_phi)[photon_reordering_mask];

        // bjets
        const auto bjet_reordering_mask = VecOps::Argsort(unwrap(BJet_pt));
        const auto bjet_pt = unwrap(BJet_pt)[bjet_reordering_mask];
        const auto bjet_eta = unwrap(BJet_eta)[bjet_reordering_mask];
        const auto bjet_phi = unwrap(BJet_phi)[bjet_reordering_mask];

        // jets
        const auto jet_reordering_mask = VecOps::Argsort(unwrap(Jet_pt));
        const auto jet_pt = unwrap(Jet_pt)[jet_reordering_mask];
        const auto jet_eta = unwrap(Jet_eta)[jet_reordering_mask];
        const auto jet_phi = unwrap(Jet_phi)[jet_reordering_mask];

        // MET
        auto met_pt = unwrap(MET_pt);

        auto get_leading = [](RVec<float> pt, RVec<float> eta, RVec<float> phi) -> std::optional<Math::PtEtaPhiMVector>
        {
            if (pt.size() == 0)
            {
                return std::nullopt;
            }
            return Math::PtEtaPhiMVector(pt[0], eta[0], phi[0], 1E-9);
        };

        auto get_met = [](RVec<float> met_pt) -> std::optional<float>
        {
            if (met_pt.size() == 0)
            {
                return std::nullopt;
            }
            return met_pt[0];
        };

        // MuMu + X
        if (muon_pt.size() >= 2)
        {
            auto muon_1 = Math::PtEtaPhiMVector(muon_pt[0], muon_eta[0], muon_phi[0], PDG::Muon::Mass);
            auto muon_2 = Math::PtEtaPhiMVector(muon_pt[1], muon_eta[1], muon_phi[1], PDG::Muon::Mass);
            auto bjet = get_leading(bjet_pt, bjet_eta, bjet_phi);
            auto jet = get_leading(jet_pt, jet_eta, jet_phi);
            auto met = get_met(met_pt);

            // wide mass range
            z_to_mu_mu_x.fill(muon_1, muon_2, bjet, jet, met, 1.);

            // Z mass range
            if ((muon_1 + muon_2).mass() > PDG::Z::Mass - 20. and (muon_1 + muon_2).mass() < PDG::Z::Mass + 20.)
            {
                z_to_mu_mu_x_Z_mass.fill(muon_1, muon_2, bjet, jet, met, 1.);
            }
        }

        // process monitoring
        if (event < 10 || (event < 100 && event % 10 == 0) || (event < 1000 && event % 100 == 0) ||
            (event < 10000 && event % 1000 == 0) || (event < 100000 && event % 10000 == 0) ||
            (event < 1000000 && event % 100000 == 0) || (event < 10000000 && event % 1000000 == 0) ||
            (event >= 10000000 && event % 10000000 == 0))
        {
            fmt::print("\n\n[MUSiC Validation] Processed {} events.\n", event);
            PrintProcessInfo();
        }
    }
    fmt::print("\n[MUSiC Validation] Saving outputs ...\n");
    z_to_mu_mu_x.dump_outputs();
    z_to_ele_ele_x.dump_outputs();
    z_to_mu_mu_x_Z_mass.dump_outputs();
    z_to_ele_ele_x_Z_mass.dump_outputs();

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}