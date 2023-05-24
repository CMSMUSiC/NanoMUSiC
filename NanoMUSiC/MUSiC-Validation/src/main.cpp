#include "Validation.hpp"

#include "Configs.hpp"
#include "Math/Vector4Dfwd.h"
#include "Outputs.hpp"
#include "ROOT/RVec.hxx"
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
    const std::string effective_x_section_str = cmdl({"-x", "--xsection"}).str();
    const std::string input_file = cmdl({"-i", "--input"}).str();

    if (show_help or process == "" or year == "" or output_path == "" or input_file == "" or
        effective_x_section_str == "")
    {
        fmt::print("Usage: validation [OPTIONS]\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -p|--process: Process (aka sample).\n");
        fmt::print("          -y|--year: Year.\n");
        fmt::print("          -d|--is_data: Is data ?\n");
        fmt::print("          -o|--output: Output path.\n");
        fmt::print("          -x|--xsection: Effective cross-section (xsection * lumi).\n");
        fmt::print("          -i|--input: Path to a txt withg input files (one per line).\n");

        exit(-1);
    }
    const double effective_x_section = std::stod(effective_x_section_str);

    // create tree reader and add values and arrays
    TChain input_chain("nano_music");
    ;
    for (auto &&file : load_input_files(input_file))
    {
        input_chain.Add(file.c_str());
    }
    auto tree_reader = TTreeReader(&input_chain);

    ADD_VALUE_READER(run, unsigned int);
    ADD_VALUE_READER(event_number, unsigned long);

    ADD_VALUE_READER(kTotalWeights, unsigned int);
    ADD_ARRAY_READER(weights_nominal, float);
    ADD_ARRAY_READER(weights_up, float);
    ADD_ARRAY_READER(weights_down, float);

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
    ADD_ARRAY_READER(BJet_mass, float);

    ADD_VALUE_READER(nJet, unsigned int);
    ADD_ARRAY_READER(Jet_pt, float);
    ADD_ARRAY_READER(Jet_eta, float);
    ADD_ARRAY_READER(Jet_phi, float);
    ADD_ARRAY_READER(Jet_mass, float);

    ADD_VALUE_READER(nMET, unsigned int);
    ADD_ARRAY_READER(MET_pt, float);

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

    auto z_to_mu_mu_x =
        ZToLepLepX(fmt::format("{}/z_to_mu_mu_x_{}_{}.root", output_path, process, year), z_to_mu_mu_x_count_map);

    auto z_to_ele_ele_x =
        ZToLepLepX(fmt::format("{}/z_to_ele_ele_x_{}_{}.root", output_path, process, year), z_to_ele_ele_x_count_map);

    auto z_to_mu_mu_x_Z_mass = ZToLepLepX(
        fmt::format("{}/z_to_mu_mu_x_Z_mass_{}_{}.root", output_path, process, year), z_to_mu_mu_x_count_map, true);

    auto z_to_ele_ele_x_Z_mass = ZToLepLepX(
        fmt::format("{}/z_to_ele_ele_x_Z_mass_{}_{}.root", output_path, process, year), z_to_ele_ele_x_count_map, true);

    const auto cutflow_file =
        std::unique_ptr<TFile>(TFile::Open(fmt::format("{}/cutflow_{}_{}.root", output_path, process, year).c_str()));
    const auto cutflow_histo = cutflow_file->Get<TH1F>("cutflow");
    // const auto total_generator_weight = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorWeight") + 1);
    const auto no_cuts = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("NoCuts") + 1);
    const auto generator_filter = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorFilter") + 1);

    // fmt::print("\n[MUSiC Validation] Creating set of processed events ...\n");
    // MAP[run_number : SET[event_number]]
    std::unordered_map<unsigned int, std::unordered_set<unsigned long>> processed_data_events;

    // launch event loop for Data or MC
    // const auto total_number_of_events = tree_reader.GetEntries();

    for (auto &&event : tree_reader)
    {
        (void)event; // remove the unused warning during compilation

        // check if the event has already been processed (only for Data)
        bool already_processed = false;
        auto this_run = unwrap(run);
        auto this_event_number = unwrap(event_number);
        if (is_data)
        {
            if (processed_data_events.count(this_run) > 0)
            {
                if (processed_data_events[this_run].count(this_event_number) > 0)
                {
                    already_processed = true;
                }
                else
                {
                    processed_data_events[this_run].insert(this_event_number);
                }
            }
            else
            {
                processed_data_events.insert({this_run, std::unordered_set<unsigned long>({this_event_number})});
            }
        }

        if (not(already_processed))
        {
            // get effective event weight
            auto weight = 1.f;
            if (not(is_data))
            {
                weight = Outputs::get_event_weight(unwrap(weights_nominal), unwrap(weights_up), unwrap(weights_down)) *
                         generator_filter / no_cuts / generator_filter * effective_x_section;

                // fmt::print("------------------\n");
                // fmt::print(
                //     "event weight: {}\n",
                //     Outputs::get_event_weight(unwrap(weights_nominal), unwrap(weights_up),
                //     unwrap(weights_down)));
                // fmt::print("weights_nominal: {}\n", weights_nominal);
                // fmt::print("Eff xSec: {}\n", effective_x_section);
                // fmt::print("Weight: {}\n", weight);
                // fmt::print("total gen weight: {}\n", generator_filter);
            }

            // reorder objects
            // muons
            const auto muon_reordering_mask = VecOps::Argsort(unwrap(Muon_pt),
                                                              [](double x, double y) -> bool
                                                              {
                                                                  return x > y;
                                                              });

            const auto muon_pt = VecOps::Take(unwrap(Muon_pt), muon_reordering_mask);
            const auto muon_eta = VecOps::Take(unwrap(Muon_eta), muon_reordering_mask);
            const auto muon_phi = VecOps::Take(unwrap(Muon_phi), muon_reordering_mask);

            // electrons
            const auto electron_reordering_mask = VecOps::Argsort(unwrap(Electron_pt),
                                                                  [](double x, double y) -> bool
                                                                  {
                                                                      return x > y;
                                                                  });
            const auto electron_pt = VecOps::Take(unwrap(Electron_pt), electron_reordering_mask);
            const auto electron_eta = VecOps::Take(unwrap(Electron_eta), electron_reordering_mask);
            const auto electron_phi = VecOps::Take(unwrap(Electron_phi), electron_reordering_mask);

            // // photons
            // const  auto photon_reordering_mask = VecOps::Argsort(unwrap(Photon_pt),
            //   [](double x, double y) -> bool
            //   {
            //       return x > y;
            //   });
            // const auto photon_pt = VecOps::Take(unwrap(Photon_pt),photon_reordering_mask);
            // const auto photon_eta = VecOps::Take(unwrap(Photon_eta),photon_reordering_mask);
            // const auto photon_phi = VecOps::Take(unwrap(Photon_phi),photon_reordering_mask);

            // bjets
            const auto bjet_reordering_mask = VecOps::Argsort(unwrap(BJet_pt),
                                                              [](double x, double y) -> bool
                                                              {
                                                                  return x > y;
                                                              });
            const auto bjet_pt = VecOps::Take(unwrap(BJet_pt), bjet_reordering_mask);
            const auto bjet_eta = VecOps::Take(unwrap(BJet_eta), bjet_reordering_mask);
            const auto bjet_phi = VecOps::Take(unwrap(BJet_phi), bjet_reordering_mask);

            // jets
            const auto jet_reordering_mask = VecOps::Argsort(unwrap(Jet_pt),
                                                             [](double x, double y) -> bool
                                                             {
                                                                 return x > y;
                                                             });
            const auto jet_pt = VecOps::Take(unwrap(Jet_pt), jet_reordering_mask);
            const auto jet_eta = VecOps::Take(unwrap(Jet_eta), jet_reordering_mask);
            const auto jet_phi = VecOps::Take(unwrap(Jet_phi), jet_reordering_mask);

            // MET
            auto met_pt = unwrap(MET_pt);

            auto get_leading = [](RVec<float> pt,
                                  RVec<float> eta,
                                  RVec<float> phi) -> std::pair<unsigned int, std::optional<Math::PtEtaPhiMVector>>
            {
                if (pt.size() == 0)
                {
                    return std::make_pair(0, std::nullopt);
                }
                return std::make_pair(pt.size(), Math::PtEtaPhiMVector(pt[0], eta[0], phi[0], 1E-9));
            };

            auto get_met = [](RVec<float> met_pt) -> std::optional<float>
            {
                if (met_pt.size() == 0)
                {
                    return std::nullopt;
                }
                return met_pt[0];
            };

            auto [nBJet, bjet] = get_leading(bjet_pt, bjet_eta, bjet_phi);
            auto [nJet, jet] = get_leading(jet_pt, jet_eta, jet_phi);
            auto met = get_met(met_pt);

            // MuMu + X
            if (muon_pt.size() >= 2)
            {
                auto muon_1 = Math::PtEtaPhiMVector(muon_pt[0], muon_eta[0], muon_phi[0], PDG::Muon::Mass);
                auto muon_2 = Math::PtEtaPhiMVector(muon_pt[1], muon_eta[1], muon_phi[1], PDG::Muon::Mass);

                // wide mass range
                z_to_mu_mu_x.fill(muon_1, muon_2, nBJet, bjet, nJet, jet, met, weight);

                // Z mass range
                if ((muon_1 + muon_2).mass() > PDG::Z::Mass - 20. and (muon_1 + muon_2).mass() < PDG::Z::Mass + 20.)
                {
                    z_to_mu_mu_x_Z_mass.fill(muon_1, muon_2, nBJet, bjet, nJet, jet, met, weight);
                }
            }

            // EleEle + X
            if (electron_pt.size() >= 2)
            {
                auto electron_1 =
                    Math::PtEtaPhiMVector(electron_pt[0], electron_eta[0], electron_phi[0], PDG::Electron::Mass);
                auto electron_2 =
                    Math::PtEtaPhiMVector(electron_pt[1], electron_eta[1], electron_phi[1], PDG::Electron::Mass);

                // wide mass range
                z_to_ele_ele_x.fill(electron_1, electron_2, nBJet, bjet, nJet, jet, met, weight);

                // Z mass range
                if ((electron_1 + electron_2).mass() > PDG::Z::Mass - 20. and
                    (electron_1 + electron_2).mass() < PDG::Z::Mass + 20.)
                {
                    z_to_ele_ele_x_Z_mass.fill(electron_1, electron_2, nBJet, bjet, nJet, jet, met, weight);
                }
            }

            // // process monitoring
            // if (event < 10 || (event < 100 && event % 10 == 0) || (event < 1000 && event % 100 == 0) ||
            //     (event < 10000 && event % 1000 == 0) || (event < 100000 && event % 10000 == 0) ||
            //     (event < 1000000 && event % 100000 == 0) || (event < 10000000 && event % 1000000 == 0) ||
            //     (event >= 10000000 && event % 10000000 == 0))
            // {
            //     fmt::print("\n\n[MUSiC Validation] Processed {} events.\n", event);
            //     PrintProcessInfo();
            // }
        }
    }

    fmt::print("\n[MUSiC Validation] Saving outputs ({} - {} - {}) ...\n", output_path, process, year);
    z_to_mu_mu_x.dump_outputs();
    z_to_ele_ele_x.dump_outputs();
    z_to_mu_mu_x_Z_mass.dump_outputs();
    z_to_ele_ele_x_Z_mass.dump_outputs();

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}