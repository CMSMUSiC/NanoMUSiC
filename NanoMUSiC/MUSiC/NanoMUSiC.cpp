#include "NanoMUSiC.hpp"

int main(int argc, char *argv[])
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
        system("clear");
        std::cout << " " << std::endl;
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
    std::cout << colors.acqua << emojicpp::emojize("              :signal_strength: Run2 - Ultra Legacy :signal_strength:")
              << colors.def << std::endl;
    std::cout << " " << std::endl;

    std::cout << " " << std::endl;
    std::cout << colors.yellow << "Checking run configuration [" << run_config_file << "] ..." << colors.def << std::endl;
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

    std::cout << colors.def << "[ Initializing ] PU corrections ..." << colors.def << std::endl;
    auto pu_weight = Corrector(CorrectionTypes::PU, configuration.year, configuration.is_data);

    // read cross-sections files
    std::cout << colors.def << "[ Initializing ] X-Sections ..." << colors.def << std::endl;
    const auto x_sections = TOMLConfig::make_toml_config(configuration.x_section_file);

    std::cout << colors.def << "[ Initializing ] Thread pool (" << configuration.n_threads << " threads) ..." << colors.def
              << std::endl;
    ROOT::EnableImplicitMT(configuration.n_threads);

    std::cout << colors.def << "[ Initializing ] Rochester Muon Momentum Corrections ..." << colors.def << std::endl;
    auto rochester_corrections = Corrector(CorrectionTypes::MuonLowPt, configuration.year, configuration.is_data);

    // performance monitoring
    std::cout << colors.def << "[ Initializing ] Performance Monitoring ..." << colors.def << std::endl;
    auto event_counter = RVec<unsigned int>(configuration.n_threads);
    auto event_counter_mutexes = std::vector<std::mutex>(configuration.n_threads);

    // build outputs
    std::vector<Outputs> outputs;
    for (auto &&slot : RangesHelpers::index_range(configuration.n_threads))
    {
        // fmt::print("DEBUG - slot: {}\n", slot);
        std::string output_file_name =
            configuration.is_crab_job
                ? "nano_music.root"
                : "nano_music_" + configuration.process + "_" + configuration.year_str + "_" + std::to_string(slot) + ".root";
        outputs.emplace_back(output_file_name);
    }

    // define columns to be processed
    std::vector<std::string> columns = {
        // event info
        "run", "luminosityBlock", "event", "Pileup_nTrueInt", "genWeight", "PV_npvsGood", "Flag_goodVertices",
        "Flag_globalSuperTightHalo2016Filter", "Flag_HBHENoiseFilter", "Flag_HBHENoiseIsoFilter",
        "Flag_EcalDeadCellTriggerPrimitiveFilter", "Flag_BadPFMuonFilter", "Flag_BadPFMuonDzFilter", "Flag_eeBadScFilter",
        "Flag_ecalBadCalibFilter", "HLT_IsoMu27", "HLT_Mu50", "HLT_TkMu100", "HLT_OldMu100",

        // muons
        "Muon_pt", "Muon_eta", "Muon_phi", "Muon_tightId", "Muon_highPtId", "Muon_pfRelIso03_all", "Muon_tkRelIso",

        // electrons
        "Electron_pt", "Electron_eta", "Electron_phi",

        // photons
        "Photon_pt", "Photon_eta", "Photon_phi",

        // taus
        "Tau_pt", "Tau_eta", "Tau_phi",

        // jets
        "Jet_pt", "Jet_eta", "Jet_phi",

        // met
        "MET_pt", "MET_phi"

    };

    // clear columns for Data processing
    if (configuration.is_data)
    {
        columns.erase(std::remove(columns.begin(), columns.end(), "genWeight"), columns.end());
        columns.erase(std::remove(columns.begin(), columns.end(), "Pileup_nTrueInt"), columns.end());
    }

    // create RDataFrame
    auto df = ROOT::RDataFrame("Events", configuration.input_files, columns);
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////       [ BEGIN ]      //////////////////////////////////////
    //////////////////////////////////////   loop over events   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    auto event_processor = [&](unsigned int slot,
                               // event info
                               const UInt_t &run, const UInt_t &lumi, const ULong64_t &event_number, const float &Pileup_nTrueInt,
                               const float &genWeight, const int &PV_npvsGood, const bool &Flag_goodVertices,
                               const bool &Flag_globalSuperTightHalo2016Filter, const bool &Flag_HBHENoiseFilter,
                               const bool &Flag_HBHENoiseIsoFilter, const bool &Flag_EcalDeadCellTriggerPrimitiveFilter,
                               const bool &Flag_BadPFMuonFilter, const bool &Flag_BadPFMuonDzFilter,
                               const bool &Flag_eeBadScFilter, const bool &Flag_ecalBadCalibFilter, const bool &HLT_IsoMu27,
                               const bool &HLT_Mu50, const bool &HLT_TkMu100, const bool &HLT_OldMu100,

                               // muons
                               const RVec<float> &Muon_pt, const RVec<float> &Muon_eta, const RVec<float> &Muon_phi,
                               const RVec<bool> &Muon_tightId, const RVec<UChar_t> &Muon_highPtId,
                               const RVec<float> &Muon_pfRelIso03_all, const RVec<float> &Muon_tkRelIso,

                               // electrons
                               const RVec<float> &Electron_pt, const RVec<float> &Electron_eta, const RVec<float> &Electron_phi,

                               // photons
                               const RVec<float> &Photon_pt, const RVec<float> &Photon_eta, const RVec<float> &Photon_phi,

                               // taus
                               const RVec<float> &Tau_pt, const RVec<float> &Tau_eta, const RVec<float> &Tau_phi,

                               // jets
                               const RVec<float> &Jet_pt, const RVec<float> &Jet_eta, const RVec<float> &Jet_phi,

                               // met
                               const float &MET_pt, const float &MET_phi) {
        // increment the number of processed events
        {
            const std::lock_guard<std::mutex> lock(event_counter_mutexes.at(slot));
            event_counter.at(slot)++;
        }

        // clear outputs
        outputs.at(slot).clear_event_trees();

        // temporaries
        RVec<float> temp_met_pt = {MET_pt};
        RVec<float> temp_met_eta = {0.};
        RVec<float> temp_met_phi = {MET_phi};

        // build event data
        auto event_data = EventData(
            // event info
            NanoObjects::EventInfo(run, lumi, event_number, Pileup_nTrueInt, genWeight, PV_npvsGood, Flag_goodVertices,
                                   Flag_globalSuperTightHalo2016Filter, Flag_HBHENoiseFilter, Flag_HBHENoiseIsoFilter,
                                   Flag_EcalDeadCellTriggerPrimitiveFilter, Flag_BadPFMuonFilter, Flag_BadPFMuonDzFilter,
                                   Flag_eeBadScFilter, Flag_ecalBadCalibFilter, HLT_IsoMu27, HLT_Mu50, HLT_TkMu100, HLT_OldMu100),
            // muons
            NanoObjects::Muons(Muon_pt, Muon_eta, Muon_phi, Muon_tightId, Muon_highPtId, Muon_pfRelIso03_all, Muon_tkRelIso),
            // electrons
            NanoObjects::Electrons(Electron_pt, Electron_eta, Electron_phi),
            // photons
            NanoObjects::Photons(Photon_pt, Photon_eta, Photon_phi),
            // taus
            NanoObjects::Taus(Tau_pt, Tau_eta, Tau_phi),
            // bjets
            NanoObjects::BJets(Jet_pt, Jet_eta, Jet_phi),
            // jets
            NanoObjects::Jets(Jet_pt, Jet_eta, Jet_phi),
            // met
            NanoObjects::MET(temp_met_pt, temp_met_eta, temp_met_phi),
            // non-event data
            configuration.is_data, configuration.year, Variation::Default, Shift::Nominal);

        event_data = event_data.set_const_weights(outputs.at(slot), pu_weight)
                         .generator_filter(outputs.at(slot))
                         .run_lumi_filter(outputs.at(slot), run_lumi_filter)
                         .npv_filter(outputs.at(slot))
                         .met_filter(outputs.at(slot))
                         .trigger_filter(outputs.at(slot))
                         .pre_selection();

        // loop over variations, shifts and classes (aka multiplicities)
        // the range::view was cleanned to skip all variations for Data
        for (const auto &variation_and_shift :
             RangesHelpers::VariationsAndShifts | views::remove_if([&](auto variation_and_shift) {
                 const auto [variation, shift] = variation_and_shift;
                 return (configuration.is_data) && (variation != Variation::Default);
             }))
        {
            const auto [variation, shift] = variation_and_shift;

            // modify objects according to the given variation
            auto corrected_event_data = EventData::apply_corrections(event_data, variation, shift)
                                            .final_selection()
                                            .trigger_match()
                                            .set_scale_factors()
                                            .has_at_least_one_class_filter()
                                            .fill_event_content(outputs.at(slot));

            // fill output event tree
            if (corrected_event_data)
            {
                auto idx_var = corrected_event_data.idx_var;
                outputs.at(slot).fill_event_tree(idx_var);
            }
        }
    };

    auto event_processor_MC =
        [&](unsigned int slot,
            // event info
            const UInt_t &run, const UInt_t &lumi, const ULong64_t &event_number, const float &Pileup_nTrueInt,
            const float &genWeight, const int &PV_npvsGood, const bool &Flag_goodVertices,
            const bool &Flag_globalSuperTightHalo2016Filter, const bool &Flag_HBHENoiseFilter,
            const bool &Flag_HBHENoiseIsoFilter, const bool &Flag_EcalDeadCellTriggerPrimitiveFilter,
            const bool &Flag_BadPFMuonFilter, const bool &Flag_BadPFMuonDzFilter, const bool &Flag_eeBadScFilter,
            const bool &Flag_ecalBadCalibFilter, const bool &HLT_IsoMu27, const bool &HLT_Mu50, const bool &HLT_TkMu100,
            const bool &HLT_OldMu100,

            // muons
            const RVec<float> &Muon_pt, const RVec<float> &Muon_eta, const RVec<float> &Muon_phi, const RVec<bool> &Muon_tightId,
            const RVec<UChar_t> &Muon_highPtId, const RVec<float> &Muon_pfRelIso03_all, const RVec<float> &Muon_tkRelIso,

            // electrons
            const RVec<float> &Electron_pt, const RVec<float> &Electron_eta, const RVec<float> &Electron_phi,

            // photons
            const RVec<float> &Photon_pt, const RVec<float> &Photon_eta, const RVec<float> &Photon_phi,

            // taus
            const RVec<float> &Tau_pt, const RVec<float> &Tau_eta, const RVec<float> &Tau_phi,

            // jets
            const RVec<float> &Jet_pt, const RVec<float> &Jet_eta, const RVec<float> &Jet_phi,

            // met
            const float &MET_pt, const float &MET_phi) {
            event_processor(slot,
                            // event info
                            run, lumi, event_number, Pileup_nTrueInt, genWeight, PV_npvsGood, Flag_goodVertices,
                            Flag_globalSuperTightHalo2016Filter, Flag_HBHENoiseFilter, Flag_HBHENoiseIsoFilter,
                            Flag_EcalDeadCellTriggerPrimitiveFilter, Flag_BadPFMuonFilter, Flag_BadPFMuonDzFilter,
                            Flag_eeBadScFilter, Flag_ecalBadCalibFilter, HLT_IsoMu27, HLT_Mu50, HLT_TkMu100, HLT_OldMu100,

                            // muons
                            Muon_pt, Muon_eta, Muon_phi, Muon_tightId, Muon_highPtId, Muon_pfRelIso03_all, Muon_tkRelIso,

                            // electrons
                            Electron_pt, Electron_eta, Electron_phi,

                            // photons
                            Photon_pt, Photon_eta, Photon_phi,

                            // taus
                            Tau_pt, Tau_eta, Tau_phi,

                            // jets
                            Jet_pt, Jet_eta, Jet_phi,

                            // met
                            MET_pt, MET_phi);
        };

    auto event_processor_Data =
        [&](unsigned int slot,
            // event info
            const UInt_t &run, const UInt_t &lumi, const ULong64_t &event_number, const int &PV_npvsGood,
            const bool &Flag_goodVertices, const bool &Flag_globalSuperTightHalo2016Filter, const bool &Flag_HBHENoiseFilter,
            const bool &Flag_HBHENoiseIsoFilter, const bool &Flag_EcalDeadCellTriggerPrimitiveFilter,
            const bool &Flag_BadPFMuonFilter, const bool &Flag_BadPFMuonDzFilter, const bool &Flag_eeBadScFilter,
            const bool &Flag_ecalBadCalibFilter, const bool &HLT_IsoMu27, const bool &HLT_Mu50, const bool &HLT_TkMu100,
            const bool &HLT_OldMu100,

            // muons
            const RVec<float> &Muon_pt, const RVec<float> &Muon_eta, const RVec<float> &Muon_phi, const RVec<bool> &Muon_tightId,
            const RVec<UChar_t> &Muon_highPtId, const RVec<float> &Muon_pfRelIso03_all, const RVec<float> &Muon_tkRelIso,

            // electrons
            const RVec<float> &Electron_pt, const RVec<float> &Electron_eta, const RVec<float> &Electron_phi,

            // photons
            const RVec<float> &Photon_pt, const RVec<float> &Photon_eta, const RVec<float> &Photon_phi,

            // taus
            const RVec<float> &Tau_pt, const RVec<float> &Tau_eta, const RVec<float> &Tau_phi,

            // jets
            const RVec<float> &Jet_pt, const RVec<float> &Jet_eta, const RVec<float> &Jet_phi,

            // met
            const float &MET_pt, const float &MET_phi) {
            event_processor(slot,
                            // event info
                            run, lumi, event_number, 1., 1., PV_npvsGood, Flag_goodVertices, Flag_globalSuperTightHalo2016Filter,
                            Flag_HBHENoiseFilter, Flag_HBHENoiseIsoFilter, Flag_EcalDeadCellTriggerPrimitiveFilter,
                            Flag_BadPFMuonFilter, Flag_BadPFMuonDzFilter, Flag_eeBadScFilter, Flag_ecalBadCalibFilter,
                            HLT_IsoMu27, HLT_Mu50, HLT_TkMu100, HLT_OldMu100,

                            // muons
                            Muon_pt, Muon_eta, Muon_phi, Muon_tightId, Muon_highPtId, Muon_pfRelIso03_all, Muon_tkRelIso,

                            // electrons
                            Electron_pt, Electron_eta, Electron_phi,

                            // photons
                            Photon_pt, Photon_eta, Photon_phi,

                            // taus
                            Tau_pt, Tau_eta, Tau_phi,

                            // jets
                            Jet_pt, Jet_eta, Jet_phi,

                            // met
                            MET_pt, MET_phi);
        };

    std::cout << " " << std::endl;
    std::cout << colors.green << "Starting Classification ..." << colors.def << std::endl;
    std::cout << " " << std::endl;

    std::cout << colors.acqua << "Launching monitoring thread ..." << colors.def << std::endl;
    double dTime1 = getCpuTime(); // Start Timer

    // monitoring thread will run while this is true
    bool run_monitoring = true;
    std::thread monitoring_thread([&]() {
        unsigned int processed_events_counter = 0;
        while (run_monitoring)
        {
            // will sleep for 10 sec
            std::this_thread::sleep_for(10000ms);

            // loop over threads and count the number of processed events
            for (std::size_t i_slot = 0; i_slot < configuration.n_threads; i_slot++)
            {
                {
                    const std::lock_guard<std::mutex> lock(event_counter_mutexes[i_slot]);
                    processed_events_counter += event_counter[i_slot];
                }
            }

            double dTime2 = getCpuTime();
            std::cout << "\n[ Performance Monitoring ] Analyzed " << processed_events_counter << " events"
                      << ", elapsed CPU time: " << dTime2 - dTime1 << "sec ("
                      << double(processed_events_counter) / (dTime2 - dTime1) << " evts per sec)" << std::endl;
            PrintProcessInfo();
            processed_events_counter = 0;
        }
    });

    // launch event loop for Data or MC
    std::cout << colors.green << "\nLaunching event loop ..." << colors.def << std::endl;
    if (configuration.is_data)
    {
        df.ForeachSlot(event_processor_Data, columns);
    }
    else
    {
        df.ForeachSlot(event_processor_MC, columns);
    }

    // stop run monitoring
    run_monitoring = false;
    monitoring_thread.join();

    std::cout << colors.green << "Event loop done ..." << colors.def << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////       [ END ]        //////////////////////////////////////
    //////////////////////////////////////   loop over events   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////

    std::cout << " " << std::endl;
    std::cout << colors.green << "Classification done ..." << colors.def << std::endl;
    std::cout << " " << std::endl;

    // show final performance report
    print_final_report(dTime1, event_counter);

    // writes data to disk
    std::cout << colors.yellow << "[ Finalizing ] Output files, cutflow histograms and event data trees ..." << colors.def
              << std::endl;
    Outputs::write_data(outputs);

    PrintProcessInfo();
    std::cout << colors.green << "\nDone ..." << colors.def << std::endl;

    return 0;
}
