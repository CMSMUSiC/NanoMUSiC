#include "Skimmer.hpp"

#include <cmath>
#include <fmt/core.h>
#include <limits>
#include <optional>
#include <stdexcept>

#include "ROOT/RVec.hxx"

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

    bool transform_mc_weight = false;
    if (not(configuration.is_data) and dataframe.HasColumn("LHEWeight_originalXWGTUP"))
    {
        auto genWeight = dataframe.Take<float, RVec<float>>("genWeight");
        if (VecOps::All(genWeight.GetValue() == 1.f))
        {
            transform_mc_weight = true;
        }
    }

    auto pre_skimmed_dataframe = dataframe.Define("DUMMY", "1.f");
    if (not(configuration.is_data) and not(dataframe.HasColumn("LHEPdfWeight")))
    {
        pre_skimmed_dataframe = pre_skimmed_dataframe.Define("LHEPdfWeight", "ROOT::RVec<float>{}");
    }
    if (not(configuration.is_data) and not(dataframe.HasColumn("LHEScaleWeight")))
    {
        pre_skimmed_dataframe = pre_skimmed_dataframe.Define("LHEScaleWeight", "ROOT::RVec<float>{}");
    }
    if (not(configuration.is_data) and not(dataframe.HasColumn("LHEWeight_originalXWGTUP")))
    {
        pre_skimmed_dataframe = pre_skimmed_dataframe.Define("LHEWeight_originalXWGTUP", "1.f");
    }
    if (not(configuration.is_data) and not(dataframe.HasColumn("LHE_HT")))
    {
        pre_skimmed_dataframe = pre_skimmed_dataframe.Define("LHE_HT", "0.f");
    }
    if (not(configuration.is_data) and not(dataframe.HasColumn("LHE_HTIncoming")))
    {
        pre_skimmed_dataframe = pre_skimmed_dataframe.Define("LHE_HTIncoming", "0.f");
    }

    // Define if does not exists
    auto skimmed_dataframe =
        pre_skimmed_dataframe
            .Define("mc_weight",
                    [&configuration, transform_mc_weight]() -> std::string_view
                    {
                        if (not(configuration.is_data))
                        {
                            return transform_mc_weight ? "LHEWeight_originalXWGTUP"sv : "genWeight"sv;
                        }
                        return "1.f"sv;
                    }())
            // Dummy filter- Used only for setting total event counts
            .Filter(
                [&cutflow_histo, &Cuts](float mc_weight) -> bool
                {
                    cutflow_histo.Fill(Cuts.index_of("NoCuts"), mc_weight);

                    return true;
                },
                {"mc_weight"})
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
                                                                  float mc_weight) -> bool
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
                        cutflow_histo.Fill(Cuts.index_of("GeneratorFilter"), mc_weight);
                        cutflow_histo.Fill(Cuts.index_of("GeneratorWeight"), mc_weight);
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
                 "mc_weight"})
            // Run/Lumi Filter
            .Filter(
                [&cutflow_histo, &Cuts, &run_lumi_filter, &configuration](
                    unsigned int run, unsigned int luminosityBlock, float mc_weight) -> bool
                {
                    if (run_lumi_filter(run, luminosityBlock, configuration.is_data))
                    {
                        cutflow_histo.Fill(Cuts.index_of("RunLumi"), mc_weight);
                        return true;
                    }
                    return false;
                },
                {"run", "luminosityBlock", "mc_weight"})
            // nPV Filter
            .Filter(
                [&cutflow_histo, &Cuts](int PV_npvsGood, float mc_weight) -> bool
                {
                    if (PV_npvsGood > 0)
                    {
                        cutflow_histo.Fill(Cuts.index_of("nPV"), mc_weight);
                        return true;
                    }
                    return false;
                },
                {"PV_npvsGood", "mc_weight"})
            // MET Filters (Flag)
            .Define(
                "pass_met_filters",
                [&configuration]() -> std::string_view
                {
                    if (configuration.year == Year::Run2016APV or configuration.year == Year::Run2016)
                    {
                        return "Flag_goodVertices && Flag_globalSuperTightHalo2016Filter && Flag_HBHENoiseFilter && Flag_HBHENoiseIsoFilter && Flag_EcalDeadCellTriggerPrimitiveFilter && Flag_BadPFMuonFilter && Flag_BadPFMuonDzFilter && Flag_eeBadScFilter"sv;
                    }

                    if (configuration.year == Year::Run2017 or configuration.year == Year::Run2018)
                    {
                        return "Flag_goodVertices && Flag_globalSuperTightHalo2016Filter && Flag_HBHENoiseFilter && Flag_HBHENoiseIsoFilter && Flag_EcalDeadCellTriggerPrimitiveFilter && Flag_BadPFMuonFilter && Flag_BadPFMuonDzFilter && Flag_eeBadScFilter && Flag_ecalBadCalibFilter"sv;
                    }

                    throw std::invalid_argument(
                        fmt::format("ERROR: Could not define trigger bits. The requested year ({}) is invalid.",
                                    configuration.year_str));
                }())
            .Filter(
                [&cutflow_histo, &Cuts](bool pass_met_filters, float mc_weight) -> bool
                {
                    if (pass_met_filters)
                    {
                        cutflow_histo.Fill(Cuts.index_of("METFilters"), mc_weight);
                        return true;
                    }
                    return false;
                },
                {"pass_met_filters", "mc_weight"})
            //  Define trigger columns
            .Define("pass_low_pt_muon_trigger",
                    [&configuration]() -> std::string_view
                    {
                        if (configuration.year == Year::Run2016APV)
                        {
                            return "HLT_IsoMu24 or HLT_IsoTkMu24"sv;
                        }

                        if (configuration.year == Year::Run2016)
                        {
                            return "HLT_IsoMu24 or HLT_IsoTkMu24"sv;
                        }

                        if (configuration.year == Year::Run2017)
                        {
                            return "HLT_IsoMu27"sv;
                        }

                        if (configuration.year == Year::Run2018)
                        {
                            return "HLT_IsoMu24"sv;
                        }
                        throw std::invalid_argument(
                            fmt::format("ERROR: Could not define trigger bits. The requested year ({}) is invalid.",
                                        configuration.year_str));
                    }())
            .Define("pass_high_pt_muon_trigger",
                    [&configuration]() -> std::string_view
                    {
                        if (configuration.year == Year::Run2016APV)
                        {
                            return "HLT_Mu50 or HLT_TkMu50"sv;
                        }

                        if (configuration.year == Year::Run2016)
                        {
                            return "HLT_Mu50 or HLT_TkMu50"sv;
                        }

                        if (configuration.year == Year::Run2017)
                        {
                            return "HLT_Mu50 or HLT_TkMu100 or HLT_OldMu100"sv;
                        }

                        if (configuration.year == Year::Run2018)
                        {
                            return "HLT_Mu50 or HLT_TkMu100 or HLT_OldMu100"sv;
                        }

                        throw std::invalid_argument(
                            fmt::format("ERROR: Could not define trigger bits. The requested year ({}) is invalid.",
                                        configuration.year_str));
                    }())
            .Define("pass_low_pt_electron_trigger",
                    [&configuration]() -> std::string_view
                    {
                        if (configuration.year == Year::Run2016APV)
                        {
                            return "HLT_Ele27_WPTight_Gsf or HLT_Photon175"sv;
                        }

                        if (configuration.year == Year::Run2016)
                        {
                            return "HLT_Ele27_WPTight_Gsf or HLT_Photon175"sv;
                        }
                        if (configuration.year == Year::Run2017)
                        {
                            return "HLT_Ele35_WPTight_Gsf or HLT_Photon200"sv;
                        }
                        if (configuration.year == Year::Run2018)
                        {
                            return "HLT_Ele35_WPTight_Gsf or HLT_Photon200"sv;
                        }

                        throw std::invalid_argument(
                            fmt::format("ERROR: Could not define trigger bits. The requested year ({}) is invalid.",
                                        configuration.year_str));
                    }())
            .Define("pass_high_pt_electron_trigger",
                    [&configuration]() -> std::string_view
                    {
                        if (configuration.year == Year::Run2016APV)
                        {
                            return "HLT_Ele27_WPTight_Gsf or HLT_Photon175 or HLT_Ele115_CaloIdVT_GsfTrkIdT"sv;
                        }

                        if (configuration.year == Year::Run2016)
                        {
                            return "HLT_Ele27_WPTight_Gsf or HLT_Photon175 or HLT_Ele115_CaloIdVT_GsfTrkIdT"sv;
                        }
                        if (configuration.year == Year::Run2017)
                        {
                            return "HLT_Ele35_WPTight_Gsf or HLT_Photon200 or HLT_Ele115_CaloIdVT_GsfTrkIdT"sv;
                        }
                        if (configuration.year == Year::Run2018)
                        {
                            return "HLT_Ele32_WPTight_Gsf or HLT_Photon200 or HLT_Ele115_CaloIdVT_GsfTrkIdT"sv;
                        }

                        throw std::invalid_argument(
                            fmt::format("ERROR: Could not define trigger bits. The requested year ({}) is invalid.",
                                        configuration.year_str));
                    }())
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
                                        float mc_weight) -> bool
                {
                    if (pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_low_pt_electron_trigger or
                        pass_high_pt_electron_trigger or pass_jet_ht_trigger or pass_jet_pt_trigger)
                    {
                        cutflow_histo.Fill(Cuts.index_of("TriggerCut"), mc_weight);
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
                 "mc_weight"})
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
