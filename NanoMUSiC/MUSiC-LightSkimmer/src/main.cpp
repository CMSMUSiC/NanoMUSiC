#include "ROOT/RVec.hxx"
#include "Skimmer.hpp"
#include <cmath>
#include <fmt/core.h>
#include <limits>
#include <optional>
#include <stdexcept>

auto main(int argc, char *argv[]) -> int
{
    TDirectory::AddDirectory(kFALSE); // Force ROOT to give directories in our hand - Yes, we can
    TH1::AddDirectory(kFALSE);        // Force ROOT to give histograms in our hand - Yes, we can

    // command line options
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    bool batch_mode = cmdl[{"-b", "--batch"}];
    const std::string run_config_file = cmdl({"-c", "--run-config"}).str();

    if (show_help || run_config_file == "")
    {
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << "MUSiC - Model Unspecific Search in CMS" << std::endl;
        std::cout << emojicpp::emojize("      :signal_strength: Run2 - Ultra Legacy :signal_strength:") << std::endl;
        std::cout << " " << std::endl;

        std::cout << run_config_file << std::endl;
        if (run_config_file == "")
        {
            std::cout << "ERROR: the option '--run-config' is required but missing" << std::endl;
        }
        std::cout << "Available options:" << std::endl;
        std::cout << " " << std::endl;
        std::cout << "  -h [ --help ]          produce help message" << std::endl;
        std::cout << "  -c [ --run-config ] arg    The main config file (TOML format)." << std::endl;
        std::cout << "  -b [ --batch ] arg     Set to 1, if running in batch mode." << std::endl;
        return -1;
    }

    const auto colors = Color::Colors(batch_mode);

    // print pretty stuff
    if (!batch_mode)
    {
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << colors.acqua << "        ███    ███ ██    ██ ███████ ██  ██████ " << colors.def << std::endl;
        std::cout << colors.acqua << "        ████  ████ ██    ██ ██      ██ ██      " << colors.def << std::endl;
        std::cout << colors.acqua << "        ██ ████ ██ ██    ██ ███████ ██ ██      " << colors.def << std::endl;
        std::cout << colors.acqua << "        ██  ██  ██ ██    ██      ██ ██ ██      " << colors.def << std::endl;
        std::cout << colors.acqua << "        ██      ██  ██████  ███████ ██  ██████ " << colors.def << std::endl;
    }

    std::cout << " " << std::endl;
    std::cout << " " << std::endl;
    std::cout << colors.acqua << "        MUSiC - Model Unspecific Search in CMS" << colors.def << std::endl;
    std::cout << colors.acqua
              << emojicpp::emojize("              :signal_strength: Run2 - Ultra Legacy :signal_strength:")
              << colors.def << std::endl;
    std::cout << " " << std::endl;

    std::cout << " " << std::endl;
    std::cout << colors.yellow << "Checking run configuration [" << run_config_file << "] ..." << colors.def
              << std::endl;
    std::cout << " " << std::endl;

    const auto configuration = TaskConfiguration(run_config_file);

    std::cout << " " << std::endl;
    std::cout << colors.yellow << "Preparing output buffer ..." << colors.def << std::endl;
    prepare_output_buffer(configuration);
    std::cout << " " << std::endl;

    // Init the run config
    std::cout << " " << std::endl;
    std::cout << colors.green << "Initializing ..." << colors.def << std::endl;
    std::cout << " " << std::endl;

    std::cout << colors.def << "[ Initializing ] Run Lumi Filter ..." << colors.def << std::endl;
    auto run_lumi_filter = RunLumiFilter(configuration.golden_json_file);

    // performance monitoring
    std::cout << colors.def << "[ Initializing ] Event counter, timer and cutflow histogram ..." << colors.def
              << std::endl;
    unsigned long event_counter = 0;
    double dTime1 = 0.; // Start Timer
    constexpr auto Cuts = Enumerate::make_enumerate("NoCuts",
                                                    "GeneratorFilter",
                                                    "GeneratorWeight",
                                                    "RunLumi",
                                                    "nPV",
                                                    "METFilters",
                                                    "TriggerCut",
                                                    "AtLeastOneSelectedObject",
                                                    "TriggerMatch");
    constexpr auto kTotalCuts = Cuts.size();
    auto cutflow_histo = TH1F("cutflow", "cutflow", kTotalCuts, -0.5, kTotalCuts - 0.5);
    cutflow_histo.Sumw2();

    // build outputs
    std::string output_file_name = configuration.is_crab_job
                                       ? "nano_music.root"
                                       : "nano_music_" + configuration.process + "_" + configuration.year_str + ".root";

    std::cout << colors.def << "Loading input files ..." << colors.def << std::endl;

    std::vector<std::string> files_to_load = {};
    for (auto &&file : configuration.input_files)
    {
        if (configuration.is_crab_job)
        {
            files_to_load.push_back(file);
        }

        else
        {
            if (file.find("root://"s) != std::string::npos)
            {
                files_to_load.push_back(load_from_local_cache(file));
            }
            else
            {
                files_to_load.push_back(file);
            }
        }
    }

    //////// DEBUG /////////////////
    debugger_t h_debug = configuration.is_crab_job ? std::nullopt : make_debugger();
    h_debug = std::nullopt;
    ////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////    [ Dataframe ]     //////////////////////////////////////
    //////////////////////////////////   filters and definitions  ////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////

    auto dataframe = RDataFrame("Events"s, files_to_load);

    auto skimmed_dataframe =
        dataframe
            // Define if does not exists
            .Define("_LHEPdfWeight",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        if (not(configuration.is_data) and dataframe.HasColumn("LHEPdfWeight"))
                        {
                            return "LHEPdfWeight"sv;
                        }
                        return "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEScaleWeight",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        if (not(configuration.is_data) and dataframe.HasColumn("LHEScaleWeight"))
                        {
                            return "LHEScaleWeight"sv;
                        }
                        return "ROOT::RVec<float>(9,-1.)"sv;
                    }())
            .Define("_LHEWeight_originalXWGTUP",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        if (not(configuration.is_data) and dataframe.HasColumn("LHEWeight_originalXWGTUP"))
                        {
                            return "LHEWeight_originalXWGTUP"sv;
                        }
                        return "-1."sv;
                    }())
            .Define("_LHE_HT",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        if (not(configuration.is_data) and dataframe.HasColumn("LHE_HT"))
                        {
                            return "LHE_HT"sv;
                        }
                        return "-1."sv;
                    }())
            .Define("_LHE_HTIncoming",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        if (not(configuration.is_data) and dataframe.HasColumn("LHE_HTIncoming"))
                        {
                            return "LHE_HTIncoming"sv;
                        }
                        return "-1."sv;
                    }())
            .Define("gen_weight",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        if (not(configuration.is_data))
                        {
                            if (dataframe.HasColumn("LHEWeight_originalXWGTUP"))
                            {
                                return "LHEWeight_originalXWGTUP"sv;
                            }
                            else
                            {
                                return "genWeight"sv;
                            }
                        }
                        return "1.f"sv;
                    }())
            // Dummy filter- Used only for setting total event counts
            .Filter(
                [&cutflow_histo, &Cuts](float gen_weight) -> bool
                {
                    cutflow_histo.Fill(Cuts.index_of("NoCuts"), gen_weight);

                    return true;
                },
                {"gen_weight"})
            .Define("_LHEPart_pt",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        return (not(configuration.is_data) and dataframe.HasColumn("LHEPart_pt"))
                                   ? "LHEPart_pt"sv
                                   : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_eta",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        return (not(configuration.is_data) and dataframe.HasColumn("LHEPart_eta"))
                                   ? "LHEPart_eta"sv
                                   : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_phi",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        return (not(configuration.is_data) and dataframe.HasColumn("LHEPart_phi"))
                                   ? "LHEPart_phi"sv
                                   : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_mass",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        return (not(configuration.is_data) and dataframe.HasColumn("LHEPart_mass"))
                                   ? "LHEPart_mass"sv
                                   : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_incomingpz",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        return (not(configuration.is_data) and dataframe.HasColumn("LHEPart_incomingpz"))
                                   ? "LHEPart_incomingpz"sv
                                   : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_pdgId",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        return (not(configuration.is_data) and dataframe.HasColumn("LHEPart_pdgId"))
                                   ? "LHEPart_pdgId"sv
                                   : "ROOT::RVec<Int_t>{}"sv;
                    }())
            .Define("_LHEPart_status",
                    [&configuration, &dataframe]() -> std::string_view
                    {
                        return (not(configuration.is_data) and dataframe.HasColumn("LHEPart_status"))
                                   ? "LHEPart_status"sv
                                   : "ROOT::RVec<Int_t>{}"sv;
                    }())
            .Define("_GenPart_pt",
                    [&configuration]() -> std::string_view
                    {
                        return configuration.is_data ? "ROOT::RVec<float>{}"sv : "GenPart_pt"sv;
                    }())
            .Define("_GenPart_eta",
                    [&configuration]() -> std::string_view
                    {
                        return configuration.is_data ? "ROOT::RVec<float>{}"sv : "GenPart_eta"sv;
                    }())
            .Define("_GenPart_phi",
                    [&configuration]() -> std::string_view
                    {
                        return configuration.is_data ? "ROOT::RVec<float>{}"sv : "GenPart_phi"sv;
                    }())
            .Define("_GenPart_mass",
                    [&configuration]() -> std::string_view
                    {
                        return configuration.is_data ? "ROOT::RVec<float>{}"sv : "GenPart_mass"sv;
                    }())
            .Define("_GenPart_genPartIdxMother",
                    [&configuration]() -> std::string_view
                    {
                        return configuration.is_data ? "ROOT::RVec<Int_t>{}"sv : "GenPart_genPartIdxMother"sv;
                    }())
            .Define("_GenPart_pdgId",
                    [&configuration]() -> std::string_view
                    {
                        return configuration.is_data ? "ROOT::RVec<Int_t>{}"sv : "GenPart_pdgId"sv;
                    }())
            .Define("_GenPart_status",
                    [&configuration]() -> std::string_view
                    {
                        return configuration.is_data ? "ROOT::RVec<Int_t>{}"sv : "GenPart_status"sv;
                    }())
            .Define("_GenPart_statusFlags",
                    [&configuration]() -> std::string_view
                    {
                        return configuration.is_data ? "ROOT::RVec<Int_t>{}"sv : "GenPart_statusFlags"sv;
                    }())
            // generator filter
            .Filter(
                [&cutflow_histo, &Cuts, &configuration, &h_debug](const RVec<float> &_LHEPart_pt,
                                                                  const RVec<float> &_LHEPart_eta,
                                                                  const RVec<float> &_LHEPart_phi,
                                                                  const RVec<float> &_LHEPart_mass,
                                                                  const RVec<float> &_LHEPart_incomingpz,
                                                                  const RVec<Int_t> &_LHEPart_pdgId,
                                                                  const RVec<Int_t> &_LHEPart_status,
                                                                  const RVec<float> &_GenPart_pt,
                                                                  const RVec<float> &_GenPart_eta,
                                                                  const RVec<float> &_GenPart_phi,
                                                                  const RVec<float> &_GenPart_mass,
                                                                  const RVec<Int_t> &_GenPart_genPartIdxMother,
                                                                  const RVec<Int_t> &_GenPart_pdgId,
                                                                  const RVec<Int_t> &_GenPart_status,
                                                                  const RVec<Int_t> &_GenPart_statusFlags,
                                                                  float gen_weight) -> bool
                {
                    bool pass_gen_filter = true;
                    // if MC
                    if (!configuration.is_data)
                    {
                        if (configuration.generator_filter_key)
                        {
                            auto lhe_particles = NanoObjects::LHEParticles(_LHEPart_pt,
                                                                           _LHEPart_eta,
                                                                           _LHEPart_phi,
                                                                           _LHEPart_mass,
                                                                           _LHEPart_incomingpz,
                                                                           _LHEPart_pdgId,
                                                                           _LHEPart_status);

                            auto gen_particles = NanoObjects::GenParticles(_GenPart_pt,
                                                                           _GenPart_eta,
                                                                           _GenPart_phi,
                                                                           _GenPart_mass,
                                                                           _GenPart_genPartIdxMother,
                                                                           _GenPart_pdgId,
                                                                           _GenPart_status,
                                                                           _GenPart_statusFlags);
                            Year _year = configuration.year;
                            pass_gen_filter = GeneratorFilters::filters.at(*(configuration.generator_filter_key))(
                                lhe_particles, gen_particles, _year, h_debug);
                        }
                        else
                        {
                            pass_gen_filter = true;
                        }
                    }
                    if (pass_gen_filter)
                    {
                        cutflow_histo.Fill(Cuts.index_of("GeneratorFilter"), gen_weight);
                        cutflow_histo.Fill(Cuts.index_of("GeneratorWeight"), gen_weight);
                        return true;
                    }
                    return false;
                },
                {"_LHEPart_pt",
                 "_LHEPart_eta",
                 "_LHEPart_phi",
                 "_LHEPart_mass",
                 "_LHEPart_incomingpz",
                 "_LHEPart_pdgId",
                 "_LHEPart_status",
                 "_GenPart_pt",
                 "_GenPart_eta",
                 "_GenPart_phi",
                 "_GenPart_mass",
                 "_GenPart_genPartIdxMother",
                 "_GenPart_pdgId",
                 "_GenPart_status",
                 "_GenPart_statusFlags",
                 "gen_weight"})
            // Run/Lumi Filter
            .Filter(
                [&cutflow_histo, &Cuts, &run_lumi_filter, &configuration](
                    unsigned int run, unsigned int luminosityBlock, float gen_weight) -> bool
                {
                    if (run_lumi_filter(run, luminosityBlock, configuration.is_data))
                    {
                        cutflow_histo.Fill(Cuts.index_of("RunLumi"), gen_weight);
                        return true;
                    }
                    return false;
                },
                {"run", "luminosityBlock", "gen_weight"})
            // nPV Filter
            .Filter(
                [&cutflow_histo, &Cuts](int PV_npvsGood, float gen_weight) -> bool
                {
                    if (PV_npvsGood > 0)
                    {
                        cutflow_histo.Fill(Cuts.index_of("nPV"), gen_weight);
                        return true;
                    }
                    return false;
                },
                {"PV_npvsGood", "gen_weight"})
            // MET Filters (Flag)
            .Filter(
                [&cutflow_histo, &Cuts](bool Flag_goodVertices,
                                        bool Flag_globalSuperTightHalo2016Filter,
                                        bool Flag_HBHENoiseFilter,
                                        bool Flag_HBHENoiseIsoFilter,
                                        bool Flag_EcalDeadCellTriggerPrimitiveFilter,
                                        bool Flag_BadPFMuonFilter,
                                        bool Flag_BadPFMuonDzFilter,
                                        bool Flag_eeBadScFilter,
                                        bool Flag_ecalBadCalibFilter,
                                        float gen_weight) -> bool
                {
                    if (Flag_goodVertices and Flag_globalSuperTightHalo2016Filter and Flag_HBHENoiseFilter and
                        Flag_HBHENoiseIsoFilter and Flag_EcalDeadCellTriggerPrimitiveFilter and Flag_BadPFMuonFilter and
                        Flag_BadPFMuonDzFilter and Flag_eeBadScFilter and Flag_ecalBadCalibFilter)
                    {
                        cutflow_histo.Fill(Cuts.index_of("METFilters"), gen_weight);
                        return true;
                    }
                    return false;
                },
                {"Flag_goodVertices",
                 "Flag_globalSuperTightHalo2016Filter",
                 "Flag_HBHENoiseFilter",
                 "Flag_HBHENoiseIsoFilter",
                 "Flag_EcalDeadCellTriggerPrimitiveFilter",
                 "Flag_BadPFMuonFilter",
                 "Flag_BadPFMuonDzFilter",
                 "Flag_eeBadScFilter",
                 "Flag_ecalBadCalibFilter",
                 "gen_weight"})
            //  Define trigger columns
            .Define("pass_low_pt_muon_trigger",
                    [](bool HLT_IsoMu24) -> bool
                    {
                        if (HLT_IsoMu24)
                        {
                            return true;
                        }
                        return false;
                    },
                    {"HLT_IsoMu24"})
            .Define("pass_high_pt_muon_trigger",
                    [](bool HLT_Mu50, bool HLT_TkMu100, bool HLT_OldMu100) -> bool
                    {
                        if (HLT_Mu50 or HLT_TkMu100 or HLT_OldMu100)
                        {
                            return true;
                        }
                        return false;
                    },
                    {"HLT_Mu50", "HLT_TkMu100", "HLT_OldMu100"})
            .Define("pass_low_pt_electron_trigger",
                    [](bool HLT_Ele32_WPTight_Gsf, bool HLT_Photon200) -> bool
                    {
                        if (HLT_Ele32_WPTight_Gsf or HLT_Photon200)
                        {
                            return true;
                        }
                        return false;
                    },
                    {"HLT_Ele32_WPTight_Gsf", "HLT_Photon200"})
            .Define("pass_high_pt_electron_trigger",
                    [](bool HLT_Ele32_WPTight_Gsf, bool HLT_Photon200, bool HLT_Ele115_CaloIdVT_GsfTrkIdT) -> bool
                    {
                        if (HLT_Ele32_WPTight_Gsf or HLT_Photon200 or HLT_Ele115_CaloIdVT_GsfTrkIdT)
                        {
                            return true;
                        }
                        return false;
                    },
                    {"HLT_Ele32_WPTight_Gsf", "HLT_Photon200", "HLT_Ele115_CaloIdVT_GsfTrkIdT"})
            .Define("pass_jet_ht_trigger",
                    [](bool HLT_PFHT1050) -> bool
                    {
                        // if (HLT_PFHT1050)
                        // {
                        //     return true;
                        // }
                        return false;
                    },
                    {"HLT_PFHT1050"})
            .Define("pass_jet_pt_trigger",
                    [](bool HLT_PFJet500) -> bool
                    {
                        // if (HLT_PFJet500)
                        // {
                        //     return true;
                        // }
                        return false;
                    },
                    {"HLT_PFJet500"})
            // Trigger Filter
            .Filter(
                [&cutflow_histo, &Cuts](bool pass_low_pt_muon_trigger,
                                        bool pass_high_pt_muon_trigger,
                                        bool pass_low_pt_electron_trigger,
                                        bool pass_high_pt_electron_trigger,
                                        bool pass_jet_ht_trigger,
                                        bool pass_jet_pt_trigger,
                                        float gen_weight) -> bool
                {
                    if (pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_low_pt_electron_trigger or
                        pass_high_pt_electron_trigger or pass_jet_ht_trigger or pass_jet_pt_trigger)
                    {
                        cutflow_histo.Fill(Cuts.index_of("TriggerCut"), gen_weight);
                        return true;
                    }
                    return false;
                },
                {"pass_low_pt_muon_trigger",
                 "pass_high_pt_muon_trigger",
                 "pass_low_pt_electron_trigger",
                 "pass_high_pt_electron_trigger",
                 "pass_jet_ht_trigger",
                 "pass_jet_pt_trigger",
                 "gen_weight"})
            // // Object Pre-object_selection
            // // Muons
            // .Define("good_muons",
            //         [&configuration](const RVec<float> &Muon_eta,
            //                          const RVec<bool> &Muon_tightId,
            //                          const RVec<UChar_t> &Muon_highPtId,
            //                          const RVec<float> &Muon_pfRelIso04_all,
            //                          const RVec<float> &Muon_tkRelIso) -> RVec<int>
            //         {
            //             const auto good_eta = VecOps::abs(Muon_eta) <=
            //             ObjConfig::Muons[configuration.year].MaxAbsEta; const auto good_id = (Muon_highPtId >= 2)
            //             || (Muon_tightId); const auto good_iso =
            //                 (Muon_pfRelIso04_all < ObjConfig::Muons[configuration.year].PFRelIso_WP) ||
            //                 (Muon_tkRelIso < ObjConfig::Muons[configuration.year].TkRelIso_WP);

            //             return good_eta && good_id && good_iso;
            //         },
            //         {"Muon_eta", "Muon_tightId", "Muon_highPtId", "Muon_pfRelIso04_all", "Muon_tkRelIso"})
            // .Redefine("Muon_pt",
            //           [](const RVec<float> &Muon_pt, const RVec<int> &good_muons)
            //           {
            //               return Muon_pt[good_muons];
            //           },
            //           {
            //               "Muon_pt",
            //               "good_muons",
            //           })
            // .Redefine("Muon_eta",
            //           [](const RVec<float> &Muon_eta, const RVec<int> &good_muons)
            //           {
            //               return Muon_eta[good_muons];
            //           },
            //           {
            //               "Muon_eta",
            //               "good_muons",
            //           })
            // .Redefine("Muon_phi",
            //           [](const RVec<float> &Muon_phi, const RVec<int> &good_muons)
            //           {
            //               return Muon_phi[good_muons];
            //           },
            //           {
            //               "Muon_phi",
            //               "good_muons",
            //           })
            // .Redefine("Muon_tightId",
            //           [](const RVec<bool> &Muon_tightId, const RVec<int> &good_muons)
            //           {
            //               return Muon_tightId[good_muons];
            //           },
            //           {
            //               "Muon_tightId",
            //               "good_muons",
            //           })
            // .Redefine("Muon_highPtId",
            //           [](const RVec<UChar_t> &Muon_highPtId, const RVec<int> &good_muons)
            //           {
            //               return Muon_highPtId[good_muons];
            //           },
            //           {
            //               "Muon_highPtId",
            //               "good_muons",
            //           })
            // .Redefine("Muon_pfRelIso04_all",
            //           [](const RVec<float> &Muon_pfRelIso04_all, const RVec<int> &good_muons)
            //           {
            //               return Muon_pfRelIso04_all[good_muons];
            //           },
            //           {
            //               "Muon_pfRelIso04_all",
            //               "good_muons",
            //           })
            // .Redefine("Muon_tkRelIso",
            //           [](const RVec<float> &Muon_tkRelIso, const RVec<int> &good_muons)
            //           {
            //               return Muon_tkRelIso[good_muons];
            //           },
            //           {
            //               "Muon_tkRelIso",
            //               "good_muons",
            //           })
            // .Redefine("Muon_highPurity",
            //           [](const RVec<bool> &Muon_highPurity, const RVec<int> &good_muons)
            //           {
            //               return Muon_highPurity[good_muons];
            //           },
            //           {
            //               "Muon_highPurity",
            //               "good_muons",
            //           })
            // .Redefine("Muon_tunepRelPt",
            //           [](const RVec<float> &Muon_tunepRelPt, const RVec<int> &good_muons)
            //           {
            //               return Muon_tunepRelPt[good_muons];
            //           },
            //           {
            //               "Muon_tunepRelPt",
            //               "good_muons",
            //           })
            // // Electrons
            // .Define("good_electrons",
            //         [&configuration](const RVec<float> &Electron_eta,
            //                          const RVec<float> &Electron_deltaEtaSC,
            //                          const RVec<int> &Electron_cutBased,
            //                          const RVec<bool> &Electron_cutBased_HEEP) -> RVec<int>
            //         {
            //             return ((VecOps::abs(Electron_eta + Electron_deltaEtaSC) <= 1.442) ||
            //                     ((VecOps::abs(Electron_eta + Electron_deltaEtaSC) >= 1.566) &&
            //                      (VecOps::abs(Electron_eta + Electron_deltaEtaSC) <= 2.5))) &&
            //                    ((Electron_cutBased >= ObjConfig::Electrons[configuration.year].cutBasedId) ||
            //                     (Electron_cutBased_HEEP));
            //         },
            //         {"Electron_eta", "Electron_deltaEtaSC", "Electron_cutBased", "Electron_cutBased_HEEP"})
            // .Redefine("Electron_pt", "Electron_pt[good_electrons]")
            // .Redefine("Electron_eta", "Electron_eta[good_electrons]")
            // .Redefine("Electron_phi", "Electron_phi[good_electrons]")
            // .Redefine("Electron_cutBased", "Electron_cutBased[good_electrons]")
            // .Redefine("Electron_cutBased_HEEP", "Electron_cutBased_HEEP[good_electrons]")
            // .Redefine("Electron_deltaEtaSC", "Electron_deltaEtaSC[good_electrons]")
            // .Redefine("Electron_dEscaleDown", "Electron_dEscaleDown[good_electrons]")
            // .Redefine("Electron_dEscaleUp", "Electron_dEscaleUp[good_electrons]")
            // .Redefine("Electron_dEsigmaDown", "Electron_dEsigmaDown[good_electrons]")
            // .Redefine("Electron_dEsigmaUp", "Electron_dEsigmaUp[good_electrons]")
            // .Redefine("Electron_eCorr", "Electron_eCorr[good_electrons]")
            // // Photons
            // .Define("good_photons",
            //         [&configuration](const RVec<bool> &Photon_isScEtaEB,
            //                          const RVec<bool> &Photon_isScEtaEE,
            //                          const RVec<int> &Photon_cutBased,
            //                          const RVec<bool> &Photon_pixelSeed) -> RVec<int>
            //         {
            //             return (Photon_isScEtaEB)        //
            //                    && (not Photon_isScEtaEE) // only EB photons
            //                    && (Photon_cutBased >= ObjConfig::Photons[configuration.year].cutBasedId) //
            //                    && (Photon_pixelSeed == false);
            //         },
            //         {"Photon_isScEtaEB", "Photon_isScEtaEE", "Photon_cutBased", "Photon_pixelSeed"})
            // .Redefine("Photon_pt", "Photon_pt[good_photons]")
            // .Redefine("Photon_eta", "Photon_eta[good_photons]")
            // .Redefine("Photon_phi", "Photon_phi[good_photons]")
            // .Redefine("Photon_cutBased", "Photon_cutBased[good_photons]")
            // .Redefine("Photon_pixelSeed", "Photon_pixelSeed[good_photons]")
            // .Redefine("Photon_isScEtaEB", "Photon_isScEtaEB[good_photons]")
            // .Redefine("Photon_isScEtaEE", "Photon_isScEtaEE[good_photons]")
            // // Jets
            // .Define("good_jets",
            //         [&configuration](const RVec<float> &Jet_eta, const RVec<int> &Jet_jetId) -> RVec<int>
            //         {
            //             return (VecOps::abs(Jet_eta) <= ObjConfig::Jets[configuration.year].MaxAbsEta) &&
            //                    (Jet_jetId >= ObjConfig::Jets[configuration.year].MinJetID);
            //         },
            //         {
            //             "Jet_eta",
            //             "Jet_jetId",
            //         })
            // .Redefine("Jet_pt", "Jet_pt[good_jets]")
            // .Redefine("Jet_eta", "Jet_eta[good_jets]")
            // .Redefine("Jet_phi", "Jet_phi[good_jets]")
            // .Redefine("Jet_mass", "Jet_mass[good_jets]")
            // .Redefine("Jet_jetId", "Jet_jetId[good_jets]")
            // .Redefine("Jet_btagDeepFlavB", "Jet_btagDeepFlavB[good_jets]")
            // .Redefine("Jet_rawFactor", "Jet_rawFactor[good_jets]")
            // .Redefine("Jet_area", "Jet_area[good_jets]")
            // .Redefine(
            //     [&configuration]()
            //     {
            //         if (configuration.is_data)
            //         {
            //             return "Jet_pt"sv;
            //         }
            //         return "Jet_genJetIdx"sv;
            //     }(),
            //     [&configuration]()
            //     {
            //         if (configuration.is_data)
            //         {
            //             return "Jet_pt"sv;
            //         }
            //         return "Jet_genJetIdx[good_jets]"sv;
            //     }())
            // .Redefine(
            //     [&configuration]()
            //     {
            //         if (configuration.is_data)
            //         {
            //             return "Jet_pt"sv;
            //         }
            //         return "Jet_hadronFlavour"sv;
            //     }(),
            //     [&configuration]()
            //     {
            //         if (configuration.is_data)
            //         {
            //             return "Jet_pt"sv;
            //         }
            //         return "Jet_hadronFlavour[good_jets]"sv;
            //     }())
            // // At least one good object Filter
            // .Filter(
            //     [&cutflow_histo, &Cuts](const RVec<int> &good_muons,
            //                                                           const RVec<int> &good_electrons,
            //                                                           const RVec<int> &good_photons,
            //                                                           const RVec<int> &good_jets, float gen_weight)
            //                                                           -> bool
            //     {
            //         if (VecOps::Sum(good_muons) > 0 or VecOps::Sum(good_electrons) > 0 or
            //             VecOps::Sum(good_photons) > 0 or VecOps::Sum(good_jets) > 0)
            //         {
            //             cutflow_histo.Fill(Cuts.index_of("AtLeastOneSelectedObject"), gen_weight);
            //             return true;
            //         }
            //         return false;
            //     },
            //     {"good_muons", "good_electrons", "good_photons", "good_jets", "gen_weight"})
            // Dummy filter - Used only for event counting
            .Filter(
                [&event_counter, &dTime1, &cutflow_histo]() -> bool
                {
                    // process monitoring
                    if (event_counter < 10 || (event_counter < 100 && event_counter % 10 == 0) ||
                        (event_counter < 1000 && event_counter % 100 == 0) ||
                        (event_counter < 10000 && event_counter % 1000 == 0) ||
                        (event_counter >= 10000 && event_counter % 10000 == 0))
                    {
                        print_report(dTime1, event_counter, cutflow_histo);
                        PrintProcessInfo();
                    }
                    event_counter++;

                    // always return true
                    return true;
                },
                {});

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////       [ BEGIN ]      //////////////////////////////////////
    //////////////////////////////////////   loop over events   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    std::cout << colors.acqua << "Starting timer ..." << colors.def << std::endl;
    dTime1 = getCpuTime(); // Start Timer

    // launch event loop for Data or MC
    std::cout << colors.green << "\nLaunching event loop ..." << colors.def << std::endl;

    skimmed_dataframe.Snapshot("nano_music", output_file_name, get_output_branches(configuration));
    auto output_file = TFile::Open(output_file_name.c_str(), "UPDATE");
    output_file->cd();
    cutflow_histo.Write();
    output_file->Close();

    std::cout << colors.green << "Event loop done ..." << colors.def << std::endl;
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////       [ END ]        //////////////////////////////////////
    //////////////////////////////////////   loop over events   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////// DEBUG //////////////////////
    if (h_debug)
    {
        fmt::print("\nSaving DEBUG Histogram ...\n");
        auto c = TCanvas("c");
        c.SetLogy();
        // c.SetLogx();
        h_debug->h_total.SetLineColor(kBlue);
        h_debug->h_pass.SetLineColor(kRed);
        h_debug->h_total.Draw("hist");
        h_debug->h_pass.Draw("same hist");

        auto legend = TLegend(0.1, .9, 0.3, 1.);
        legend.AddEntry(&h_debug->h_pass, "Pass", "l");
        legend.AddEntry(&h_debug->h_total, "Total", "l");
        legend.Draw();

        c.SaveAs("h_debug.png");
        h_debug->h_pass.Print();
        h_debug->h_total.Print();
        fmt::print("... done.\n");
    }
    ////////////////////////////////////////////////

    std::cout << " " << std::endl;
    std::cout << colors.green << "Skimming done ..." << colors.def << std::endl;
    std::cout << " " << std::endl;

    // show final performance report
    print_report(dTime1, event_counter, cutflow_histo, true);

    PrintProcessInfo();

    // this is a dummy file, create to flag if the codde run all the way to the end
    // should be used by CRAB to confirm the completion of the task
    if (batch_mode)
    {
        fmt::print("Saving success_flag.out ...\n");
        std::ofstream success_flag("success_flag.out");
        success_flag << "Yay!";
        success_flag.close();
    }

    std::cout << colors.green << "\nDone ..." << colors.def << std::endl;
    return 0;
}
