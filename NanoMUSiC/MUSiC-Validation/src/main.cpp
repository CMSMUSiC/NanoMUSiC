#include "Configs.hpp"
#include "Math/Vector4Dfwd.h"
#include "Outputs.hpp"
#include "ROOT/RVec.hxx"
#include "TFile.h"
#include "TH1.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "Validation.hpp"
#include "fmt/core.h"
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

auto main(int argc, char *argv[]) -> int
{
    // set SumW2 as default
    TH1::SetDefaultSumw2(true);

    // command line options
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    const std::string config_file = cmdl({"-c", "--config"}).str();
    const std::string output_path = cmdl({"-o", "--output"}).str();
    const double effective_x_section = std::stod(cmdl({"-x", "--xsec"}).str());
    const std::string input_file = cmdl({"-i", "--input"}).str();

    if (show_help or config_file == "" or output_path == "" or input_file == "")
    {
        fmt::print("Usage: validation [-h|--help] --config <config_file>\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -c|--config: Task config file.\n");
        fmt::print("          -o|--output: Output path.\n");
        fmt::print("          -i|--input: Input file..\n");
        return (config_file == "" ? -1 : 0);
    }

    // task configuration
    const auto task_config = TOMLConfig::make_toml_config(config_file);
    const auto is_data = task_config.get<bool>("is_data");
    const auto x_section_file(MUSiCTools::parse_and_expand_music_base(task_config.get<std::string>("x_section_file")));
    const auto process = task_config.get<std::string>("process");
    const auto year = task_config.get<std::string>("year");

    // create tree reader and add values and arrays
    auto this_file = std::unique_ptr<TFile>(TFile::Open(input_file.c_str()));
    auto tree_reader = TTreeReader("nano_music", this_file.get());

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

    ADD_VALUE_READER(nJet, unsigned int);
    ADD_ARRAY_READER(Jet_pt, float);
    ADD_ARRAY_READER(Jet_eta, float);
    ADD_ARRAY_READER(Jet_phi, float);

    ADD_VALUE_READER(nMET, unsigned int);
    ADD_ARRAY_READER(MET_pt, float);

    auto z_to_mu_mu_x = ZToLepLepX(fmt::format(
        "{}/z_to_mu_mu_x_{}_{}.root", output_path, std::to_string(std::hash<std::string>{}(input_file)), year));

    auto z_to_ele_ele_x = ZToLepLepX(fmt::format(
        "{}/z_to_ele_ele_x_{}_{}.root", output_path, std::to_string(std::hash<std::string>{}(input_file)), year));

    auto z_to_mu_mu_x_Z_mass = ZToLepLepX(fmt::format("{}/z_to_mu_mu_x_Z_mass_{}_{}.root",
                                                      output_path,
                                                      std::to_string(std::hash<std::string>{}(input_file)),
                                                      year),
                                          true);

    auto z_to_ele_ele_x_Z_mass = ZToLepLepX(fmt::format("{}/z_to_ele_ele_x_Z_mass_{}_{}.root",
                                                        output_path,
                                                        std::to_string(std::hash<std::string>{}(input_file)),
                                                        year),
                                            true);

    const auto cutflow_file = std::unique_ptr<TFile>(TFile::Open(fmt::format("{}/cutflow.root", output_path).c_str()));
    const auto cutflow_histo = cutflow_file->Get<TH1F>("cutflow");
    const auto total_generator_weight = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorWeight") + 1);
    const auto no_cuts = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("NoCuts") + 1);
    const auto generator_filter = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorFilter") + 1);

    // fmt::print("\n[MUSiC Validation] Creating set of processed events ...\n");
    std::unordered_map<unsigned int, std::unordered_set<unsigned long>> processed_data_events;

    // launch event loop for Data or MC
    // const auto total_number_of_events = tree_reader.GetEntries();

    if (this_file->Get<TH1F>("cutflow")->GetBinContent(Outputs::Cuts.index_of("TriggerMatch") + 1) != 0)
    {

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
                    processed_data_events.insert({this_run, std::unordered_set<unsigned long>()});
                }
            }

            if (not(already_processed))
            {
                // get effective event weight
                auto weight = 1.f;
                if (not(is_data))
                {
                    weight =
                        Outputs::get_event_weight(unwrap(weights_nominal), unwrap(weights_up), unwrap(weights_down)) /
                        generator_filter * effective_x_section;

                    fmt::print("------------------\n");
                    fmt::print(
                        "event weight: {}\n",
                        Outputs::get_event_weight(unwrap(weights_nominal), unwrap(weights_up), unwrap(weights_down)));
                    fmt::print("weights_nominal: {}\n", weights_nominal);
                    fmt::print("Eff xSec: {}\n", effective_x_section);
                    fmt::print("Weight: {}\n", weight);
                    fmt::print("total gen weight: {}\n", generator_filter);
                }

                // reorder objects
                // muons
                const auto muon_reordering_mask = VecOps::Argsort(unwrap(Muon_pt));
                const auto muon_pt = VecOps::Take(unwrap(Muon_pt), muon_reordering_mask);
                const auto muon_eta = VecOps::Take(unwrap(Muon_eta), muon_reordering_mask);
                const auto muon_phi = VecOps::Take(unwrap(Muon_phi), muon_reordering_mask);

                // electrons
                const auto electron_reordering_mask = VecOps::Argsort(unwrap(Electron_pt));
                const auto electron_pt = VecOps::Take(unwrap(Electron_pt), electron_reordering_mask);
                const auto electron_eta = VecOps::Take(unwrap(Electron_eta), electron_reordering_mask);
                const auto electron_phi = VecOps::Take(unwrap(Electron_phi), electron_reordering_mask);

                // // photons
                // const  auto photon_reordering_mask = VecOps::Argsort(unwrap(Photon_pt));
                // const auto photon_pt = VecOps::Take(unwrap(Photon_pt),photon_reordering_mask);
                // const auto photon_eta = VecOps::Take(unwrap(Photon_eta),photon_reordering_mask);
                // const auto photon_phi = VecOps::Take(unwrap(Photon_phi),photon_reordering_mask);

                // bjets
                const auto bjet_reordering_mask = VecOps::Argsort(unwrap(BJet_pt));
                const auto bjet_pt = VecOps::Take(unwrap(BJet_pt), bjet_reordering_mask);
                const auto bjet_eta = VecOps::Take(unwrap(BJet_eta), bjet_reordering_mask);
                const auto bjet_phi = VecOps::Take(unwrap(BJet_phi), bjet_reordering_mask);

                // jets
                const auto jet_reordering_mask = VecOps::Argsort(unwrap(Jet_pt));
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
    }
    else
    {
        //
        throw std::runtime_error(fmt::format("EMPTY: {}", process));
    }
    // fmt::print("\n[MUSiC Validation] Saving outputs ...\n");
    z_to_mu_mu_x.dump_outputs();
    z_to_ele_ele_x.dump_outputs();
    z_to_mu_mu_x_Z_mass.dump_outputs();
    z_to_ele_ele_x_Z_mass.dump_outputs();

    // fmt::print("\n[MUSiC Validation] Done ...\n");
    // PrintProcessInfo();

    // fmt::print("({}) Not enough muons: {} / {} = {}\n",
    //            process,
    //            _debug_enough_muons,
    //            total_number_of_events,
    //            static_cast<double>(_debug_enough_muons) / static_cast<double>(total_number_of_events));

    return EXIT_SUCCESS;
}