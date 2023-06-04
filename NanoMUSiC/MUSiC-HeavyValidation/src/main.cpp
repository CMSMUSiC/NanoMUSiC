#include "HeavyValidation.hpp"

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
        fmt::print("          -i|--input: Path to a txt with input files (one per line).\n");

        exit(-1);
    }
    const double effective_x_section = std::stod(effective_x_section_str);

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

    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);
    ADD_ARRAY_READER(Muon_tightId, bool);
    ADD_ARRAY_READER(Muon_highPtId, UChar_t);
    ADD_ARRAY_READER(Muon_pfRelIso04_all, float);
    ADD_ARRAY_READER(Muon_tkRelIso, float);
    ADD_ARRAY_READER(Muon_tunepRelPt, float);

    ADD_ARRAY_READER(Electron_pt, float);
    ADD_ARRAY_READER(Electron_eta, float);
    ADD_ARRAY_READER(Electron_phi, float);

    // ADD_ARRAY_READER(Photon_pt, float);
    // ADD_ARRAY_READER(Photon_eta, float);
    // ADD_ARRAY_READER(Photon_phi, float);

    ADD_ARRAY_READER(Jet_pt, float);
    ADD_ARRAY_READER(Jet_eta, float);
    ADD_ARRAY_READER(Jet_phi, float);
    ADD_ARRAY_READER(Jet_mass, float);

    ADD_ARRAY_READER(MET_pt, float);
    ADD_ARRAY_READER(MET_phi, float);

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

    auto z_to_mu_mu_x =
        ZToLepLepX(fmt::format("{}/z_to_mu_mu_x_{}_{}.root", output_path, process, year), z_to_mu_mu_x_count_map);

    auto z_to_mu_mu_x_Z_mass = ZToLepLepX(
        fmt::format("{}/z_to_mu_mu_x_Z_mass_{}_{}.root", output_path, process, year), z_to_mu_mu_x_count_map, true);

    const auto cutflow_file =
        std::unique_ptr<TFile>(TFile::Open(fmt::format("{}/cutflow_{}_{}.root", output_path, process, year).c_str()));
    const auto cutflow_histo = cutflow_file->Get<TH1F>("cutflow");
    // const auto total_generator_weight = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorWeight") + 1);
    const auto no_cuts = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("NoCuts") + 1);
    const auto generator_filter = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorFilter") + 1);

    // Efficiency
    TH1F all_probes = TH1F("all_probes", "all_probes", 100, 0, 10);
    all_probes = rebin_histogram(all_probes, z_to_mu_mu_x_count_map, false, "validation_plot", 0, 1000);
    TH1F pass_probes = TH1F("pass_probes", "pass_probes", 100, 0, 10);
    pass_probes = rebin_histogram(all_probes, z_to_mu_mu_x_count_map, false, "validation_plot", 0, 1000);
    auto efficiency = TEfficiency(pass_probes, all_probes);

    // ////////////////////////////////////////////////////
    // ////////////////////////////////////////////////////
    // ////////////////////////////////////////////////////
    auto h_pt = TH1F(fmt::format("h_pt").c_str(), //
                     fmt::format("h_pt").c_str(), //
                     100,                         //
                     0.,                          //
                     2000.                        //
    );
    h_pt.Sumw2();

    auto h_pt_low = TH1F(fmt::format("h_pt_low").c_str(), //
                         fmt::format("h_pt_low").c_str(), //
                         100,                             //
                         0.,                              //
                         2000.                            //
    );
    h_pt_low.Sumw2();

    auto h_pt_high = TH1F(fmt::format("h_pt_high").c_str(), //
                          fmt::format("h_pt_high").c_str(), //
                          100,                              //
                          0.,                               //
                          2000.                             //
    );
    h_pt_high.Sumw2();

    auto h_mass = TH1F(fmt::format("h_mass").c_str(), //
                       fmt::format("h_mass").c_str(), //
                       100,                           //
                       0.,                            //
                       1000.                          //
    );
    h_mass.Sumw2();

    auto h_mass_v2 = TH1F(fmt::format("h_mass_v2").c_str(), //
                          fmt::format("h_mass_v2").c_str(), //
                          100,                              //
                          0.,                               //
                          1000.                             //
    );
    h_mass_v2.Sumw2();

    auto h_mass_pt = TH2F(fmt::format("h_mass_pt").c_str(), //
                          fmt::format("h_mass_pt").c_str(), //
                          100,                              //
                          0.,                               //
                          1000.,
                          100,  //
                          0.,   //
                          2000. //                                 //
    );
    h_mass_pt.Sumw2();
    // ////////////////////////////////////////////////////
    // ////////////////////////////////////////////////////
    // ////////////////////////////////////////////////////
    // launch event loop for Data or MC
    for (auto &&event : tree_reader)
    {
        (void)event; // remove the "unused variable" warning during compilation

        if (event > 10000000)
        {
            break;
        }

        // get effective event weight
        auto weight = 1.f;
        auto const_weights = 1.f;

        if (not(is_data))
        {
            weight = const_weights * generator_filter / no_cuts / generator_filter * effective_x_section;
        }

        // trigger
        // bool is_good_trigger = unwrap(pass_low_pt_muon_trigger) and (unwrap(pass_high_pt_muon_trigger)) and
        //                        (unwrap(pass_low_pt_electron_trigger)) and (unwrap(pass_high_pt_electron_trigger));
        bool is_good_trigger = unwrap(pass_low_pt_muon_trigger) or unwrap(pass_high_pt_muon_trigger);
        if (not(is_good_trigger))
        {
            continue;
        }

        ////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////
        if (unwrap(Muon_pt).size() >= 1)
        {
            bool pass_low_pt_trigger = unwrap(pass_low_pt_muon_trigger);
            bool pass_high_pt_trigger = unwrap(pass_high_pt_muon_trigger);

            auto leading_muon = Math::PtEtaPhiMVector(Muon_pt[0],  //
                                                      Muon_eta[0], //
                                                      Muon_phi[0], //
                                                      PDG::Muon::Mass);

            bool is_good_low_pt_muon = (Muon_pt[0] >= 25.)                //
                                       && (Muon_pt[0] < 200.)             //
                                       && (std::fabs(Muon_eta[0]) <= 2.4) //
                                       && (Muon_tightId[0])               //
                                       && (Muon_pfRelIso04_all[0] < 0.15);

            bool is_good_high_pt_muon = (Muon_pt[0] >= 200.)               //
                                        && (std::fabs(Muon_eta[0]) <= 2.4) //
                                        && (Muon_highPtId[0] >= 2)         //
                                        && (Muon_tkRelIso[0] < 0.10);

            if (is_good_high_pt_muon)
            {
                // for some reason, the Relative pT Tune can yield very low corrected pT. Because of this,
                // they will be caped to 25., in order to not break the JSON SFs bound checks.
                // https://cms-nanoaod-integration.web.cern.ch/commonJSONSFs/summaries/MUO_2016postVFP_UL_muon_Z.html
                leading_muon.SetPt(std::max(Muon_tunepRelPt[0] * leading_muon.pt(), 25.));
            }

            if ((pass_low_pt_trigger and is_good_low_pt_muon) or (pass_high_pt_trigger and is_good_high_pt_muon))
            {
                h_pt.Fill(leading_muon.pt(), 1.);
            }

            if (pass_low_pt_trigger and is_good_low_pt_muon)
            {
                h_pt_low.Fill(leading_muon.pt(), 1.);
            }

            if (pass_high_pt_trigger and is_good_high_pt_muon)
            {
                h_pt_high.Fill(leading_muon.pt(), 1.);
            }

            // dimuon check
            if (unwrap(Muon_pt).size() >= 2 and
                ((pass_low_pt_trigger and is_good_low_pt_muon) or (pass_high_pt_trigger and is_good_high_pt_muon)))
            {
                auto sub_leading_muon = Math::PtEtaPhiMVector(Muon_pt[1],  //
                                                              Muon_eta[1], //
                                                              Muon_phi[1], //
                                                              PDG::Muon::Mass);
                is_good_low_pt_muon = (Muon_pt[1] >= 25.)                //
                                      && (Muon_pt[1] < 200.)             //
                                      && (std::fabs(Muon_eta[1]) <= 2.4) //
                                      && (Muon_tightId[1])               //
                                      && (Muon_pfRelIso04_all[1] < 0.15);

                is_good_high_pt_muon = (Muon_pt[1] >= 200.)               //
                                       && (std::fabs(Muon_eta[1]) <= 2.4) //
                                       && (Muon_highPtId[1] >= 2)         //
                                       && (Muon_tkRelIso[1] < 0.10);

                if (is_good_high_pt_muon)
                {
                    // for some reason, the Relative pT Tune can yield very low corrected pT. Because of this,
                    // they will be caped to 25., in order to not break the JSON SFs bound checks.
                    // https://cms-nanoaod-integration.web.cern.ch/commonJSONSFs/summaries/MUO_2016postVFP_UL_muon_Z.html
                    sub_leading_muon.SetPt(std::max(Muon_tunepRelPt[1] * sub_leading_muon.pt(), 25.));
                }

                if ((pass_low_pt_trigger and is_good_low_pt_muon) or (pass_high_pt_trigger and is_good_high_pt_muon))
                {
                    h_mass.Fill((leading_muon + sub_leading_muon).mass(), 1.);
                    h_mass_pt.Fill((leading_muon + sub_leading_muon).mass(), leading_muon.pt(), 1.);
                }
            }
        }
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////

        // muons
        auto muons = make_muons(unwrap(Muon_pt),
                                unwrap(Muon_eta),
                                unwrap(Muon_phi),
                                unwrap(Muon_tightId),
                                unwrap(Muon_highPtId),
                                unwrap(Muon_pfRelIso04_all),
                                unwrap(Muon_tkRelIso),
                                unwrap(Muon_tunepRelPt),
                                unwrap(pass_low_pt_muon_trigger),
                                unwrap(pass_high_pt_muon_trigger));

        // fmt::print("====================\n");
        // for (auto &&muon : muons)
        // {
        //     fmt::print("{} : ", muon.pt());
        // }

        // MuMu + X
        if (muons.size() >= 2)
        {
            auto muon_1 = muons.at(0);
            auto muon_2 = muons.at(1);

            h_mass_v2.Fill((muon_1 + muon_2).mass(), weight);

            fmt::print("-------------------\n");
            fmt::print("Mass V2: {}\n", (muon_1 + muon_2).mass());

            // wide mass range
            z_to_mu_mu_x.fill(muon_1, muon_2, 0, std::nullopt, 0, std::nullopt, std::nullopt, weight);

            // Z mass range
            if (PDG::Z::Mass - 20. < (muon_1 + muon_2).mass() and (muon_1 + muon_2).mass() < PDG::Z::Mass + 20.)
            {
                z_to_mu_mu_x_Z_mass.fill(muon_1, muon_2, 0, std::nullopt, 0, std::nullopt, std::nullopt, weight);
            }

            // // Efficiency test
            // for (std::size_t i = 0; i < muons.size(); i++)
            // {
            //     if (muons[i].pt() > 50.)
            //     {
            //         for (std::size_t j = i + 1; j < muons.size(); j++)
            //         {
            //             if (PDG::Z::Mass - 10. < (muons[i] + muons[j]).mass() and
            //                 (muons[i] + muons[j]).mass() < PDG::Z::Mass + 10.)
            //             {
            //                 all_probes.Fill(muon_2.pt(), weight);
            //                 if (is_good_trigger)
            //                 {
            //                     pass_probes.Fill(muon_2.pt(), weight);
            //                     efficiency.FillWeighted(true, weight, muon_2.pt());
            //                 }
            //                 else
            //                 {
            //                     efficiency.FillWeighted(false, weight, muon_2.pt());
            //                 }
            //             }
            //         }
            //     }
            // }
        }
    }

    fmt::print("\n[MUSiC Validation] Saving outputs ({} - {} - {}) ...\n", output_path, process, year);
    z_to_mu_mu_x.dump_outputs(efficiency);
    z_to_mu_mu_x_Z_mass.dump_outputs(efficiency);

    fmt::print("Printing Efficiency ...\n");
    auto c1 = TCanvas("c1");
    // c1.SetLogy(true);
    efficiency.Draw("AP");
    // efficiency.Print("all");
    c1.SaveAs("h_eff.png");

    auto c2 = TCanvas("c2");
    c2.SetLogy(true);
    all_probes.Draw();
    // all_probes.Print("all");
    c2.SaveAs("h_all_probes.png");

    auto c3 = TCanvas("c3");
    c3.SetLogy(true);
    pass_probes.Draw();
    // pass_probes.Print("all");
    c3.SaveAs("h_pass_probes.png");

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    save_as(h_pt, "histo_pt");
    save_as(h_pt_low, "histo_pt_low");
    save_as(h_pt_high, "histo_pt_high");
    save_as(h_mass, "h_mass");
    save_as(h_mass_v2, "h_mass_v2");
    save_as(h_mass_pt, "h_mass_pt");

    save_as(z_to_mu_mu_x.h_invariant_mass, "h_invariant_mass");

    return EXIT_SUCCESS;
}