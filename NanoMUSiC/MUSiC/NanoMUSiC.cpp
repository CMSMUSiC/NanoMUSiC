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

    std::cout << colors.def << "[ Initializing ] PU corrections ..." << colors.def << std::endl;
    auto pu_weight = Corrector(CorrectionTypes::PU, configuration.year, configuration.is_data);

    std::cout << colors.def << "[ Initializing ] Trigger SF corrections ..." << colors.def << std::endl;

    std::map<std::string_view, Corrector> trigger_sf_correctors = //
        {
            {"SingleMuonLowPt",
             Corrector(CorrectionTypes::TriggerSFMuonLowPt, configuration.year, configuration.is_data)},
            {"SingleMuonHighPt",
             Corrector(CorrectionTypes::TriggerSFMuonHighPt, configuration.year, configuration.is_data)},
            {"SingleElectronLowPt",
             Corrector(CorrectionTypes::TriggerSFElectronHighPt, configuration.year, configuration.is_data)},
            {"SingleElectronHighPt",
             Corrector(CorrectionTypes::TriggerSFElectronHighPt, configuration.year, configuration.is_data)},
            {"Photon", Corrector(CorrectionTypes::Photon, configuration.year, configuration.is_data)},
        };

    // sanity checks ...
    // the keys of the map above should match the defined HLP paths
    for (auto &&correction : trigger_sf_correctors)
    {
        const auto [name, corr] = correction;
        auto it = std::find(Trigger::HLTPath.cbegin(), Trigger::HLTPath.cend(), name);
        if (it == Trigger::HLTPath.cend())
        {
            throw std::runtime_error(
                fmt::format("The Trigger SF name ({}) is not present in the array of defined HLT paths ({}).\n", name,
                            Trigger::HLTPath));
        }
    }

    std::cout << colors.def << "[ Initializing ] Rochester Muon Momentum Corrections ..." << colors.def << std::endl;
    auto rochester_corrections = Corrector(CorrectionTypes::MuonLowPt, configuration.year, configuration.is_data);

    // read cross-sections files
    std::cout << colors.def << "[ Initializing ] X-Sections ..." << colors.def << std::endl;
    const auto x_sections = TOMLConfig::make_toml_config(configuration.x_section_file);

    // performance monitoring
    std::cout << colors.def << "[ Initializing ] Event counter and timer ..." << colors.def << std::endl;
    unsigned long event_counter = 0;
    double dTime1 = 0;

    // build outputs
    std::string output_file_name = configuration.is_crab_job
                                       ? "nano_music.root"
                                       : "nano_music_" + configuration.process + "_" + configuration.year_str + ".root";
    Outputs outputs(output_file_name);

    // create file chain and tree reader
    auto chain = TChain("Events");
    for (auto &&file : configuration.input_files)
    {
        chain.Add(file.c_str());
    }

    // create tree reader and add values and arrays
    auto tree_reader = TTreeReader(&chain);

    // event info
    ADD_VALUE_READER(run, UInt_t);
    // ADD_VALUE_READER(lumi, UInt_t);
    auto lumi = make_value_reader<UInt_t>(tree_reader, "luminosityBlock");
    // ADD_VALUE_READER(event_number, ULong64_t);
    auto event_number = make_value_reader<ULong64_t>(tree_reader, "event");
    ADD_VALUE_READER(Pileup_nTrueInt, float);
    ADD_VALUE_READER(genWeight, float);
    ADD_VALUE_READER(PV_npvsGood, int);
    ADD_VALUE_READER(Flag_goodVertices, bool);
    ADD_VALUE_READER(Flag_globalSuperTightHalo2016Filter, bool);
    ADD_VALUE_READER(Flag_HBHENoiseFilter, bool);
    ADD_VALUE_READER(Flag_HBHENoiseIsoFilter, bool);
    ADD_VALUE_READER(Flag_EcalDeadCellTriggerPrimitiveFilter, bool);
    ADD_VALUE_READER(Flag_BadPFMuonFilter, bool);
    ADD_VALUE_READER(Flag_BadPFMuonDzFilter, bool);
    ADD_VALUE_READER(Flag_eeBadScFilter, bool);
    ADD_VALUE_READER(Flag_ecalBadCalibFilter, bool);
    ADD_VALUE_READER(HLT_IsoMu27, bool);
    ADD_VALUE_READER(HLT_IsoMu24, bool);
    ADD_VALUE_READER(HLT_IsoTkMu24, bool);
    ADD_VALUE_READER(HLT_Mu50, bool);
    ADD_VALUE_READER(HLT_TkMu50, bool);
    ADD_VALUE_READER(HLT_TkMu100, bool);
    ADD_VALUE_READER(HLT_OldMu100, bool);
    ADD_VALUE_READER(HLT_Ele27_WPTight_Gsf, bool);
    ADD_VALUE_READER(HLT_Ele35_WPTight_Gsf, bool);
    ADD_VALUE_READER(HLT_Ele32_WPTight_Gsf, bool);
    ADD_VALUE_READER(HLT_Photon200, bool);
    ADD_VALUE_READER(HLT_Photon175, bool);
    ADD_VALUE_READER(HLT_Ele115_CaloIdVT_GsfTrkIdT, bool);
    ;

    // muons
    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);
    ADD_ARRAY_READER(Muon_tightId, bool);
    ADD_ARRAY_READER(Muon_highPtId, UChar_t);
    ADD_ARRAY_READER(Muon_pfRelIso03_all, float);
    ADD_ARRAY_READER(Muon_tkRelIso, float);

    // electrons
    ADD_ARRAY_READER(Electron_pt, float);
    ADD_ARRAY_READER(Electron_eta, float);
    ADD_ARRAY_READER(Electron_phi, float);
    ADD_ARRAY_READER(Electron_cutBased, int);
    ADD_ARRAY_READER(Electron_cutBased_HEEP, bool);
    ADD_ARRAY_READER(Electron_deltaEtaSC, float);

    // photons
    ADD_ARRAY_READER(Photon_pt, float);
    ADD_ARRAY_READER(Photon_eta, float);
    ADD_ARRAY_READER(Photon_phi, float);

    // taus
    ADD_ARRAY_READER(Tau_pt, float);
    ADD_ARRAY_READER(Tau_eta, float);
    ADD_ARRAY_READER(Tau_phi, float);

    // jets
    ADD_ARRAY_READER(Jet_pt, float);
    ADD_ARRAY_READER(Jet_eta, float);
    ADD_ARRAY_READER(Jet_phi, float);

    // met
    ADD_VALUE_READER(MET_pt, float);
    ADD_VALUE_READER(MET_phi, float);

    // trgobjs
    ADD_ARRAY_READER(TrigObj_pt, float);
    ADD_ARRAY_READER(TrigObj_eta, float);
    ADD_ARRAY_READER(TrigObj_phi, float);
    ADD_ARRAY_READER(TrigObj_id, int);
    ADD_ARRAY_READER(TrigObj_filterBits, int);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////       [ BEGIN ]      //////////////////////////////////////
    //////////////////////////////////////   loop over events   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    std::cout << " " << std::endl;
    std::cout << colors.green << "Starting Classification ..." << colors.def << std::endl;
    std::cout << " " << std::endl;

    std::cout << colors.acqua << "Starting timer ..." << colors.def << std::endl;
    dTime1 = getCpuTime(); // Start Timer

    // launch event loop for Data or MC
    std::cout << colors.green << "\nLaunching event loop ..." << colors.def << std::endl;

    for (auto &&evt : tree_reader)
    {
        event_counter = evt;

        // clear outputs buffers
        outputs.clear_event_tree();

        // build event data
        auto event_data =
            EventData(configuration.is_data, configuration.year, configuration.trigger_stream)
                // event info
                .set_event_info(NanoObjects::EventInfo(unwrap(run),                                     //
                                                       unwrap(lumi),                                    //
                                                       unwrap(event_number),                            //
                                                       unwrap(Pileup_nTrueInt),                         //
                                                       unwrap(genWeight),                               //
                                                       unwrap(PV_npvsGood),                             //
                                                       unwrap(Flag_goodVertices),                       //
                                                       unwrap(Flag_globalSuperTightHalo2016Filter),     //
                                                       unwrap(Flag_HBHENoiseFilter),                    //
                                                       unwrap(Flag_HBHENoiseIsoFilter),                 //
                                                       unwrap(Flag_EcalDeadCellTriggerPrimitiveFilter), //
                                                       unwrap(Flag_BadPFMuonFilter),                    //
                                                       unwrap(Flag_BadPFMuonDzFilter),                  //
                                                       unwrap(Flag_eeBadScFilter),                      //
                                                       unwrap(Flag_ecalBadCalibFilter),                 //
                                                       unwrap(HLT_IsoMu27),                             //
                                                       unwrap(HLT_IsoMu24),                             //
                                                       unwrap(HLT_IsoTkMu24),                           //
                                                       unwrap(HLT_Mu50),                                //
                                                       unwrap(HLT_TkMu50),                              //
                                                       unwrap(HLT_TkMu100),                             //
                                                       unwrap(HLT_OldMu100),                            //
                                                       unwrap(HLT_Ele27_WPTight_Gsf),                   //
                                                       unwrap(HLT_Ele35_WPTight_Gsf),                   //
                                                       unwrap(HLT_Ele32_WPTight_Gsf),                   //
                                                       unwrap(HLT_Photon200),                           //
                                                       unwrap(HLT_Photon175),                           //
                                                       unwrap(HLT_Ele115_CaloIdVT_GsfTrkIdT)))
                // muons
                .set_muons(NanoObjects::Muons(unwrap(Muon_pt), unwrap(Muon_eta), unwrap(Muon_phi), unwrap(Muon_tightId),
                                              unwrap(Muon_highPtId), unwrap(Muon_pfRelIso03_all),
                                              unwrap(Muon_tkRelIso)))
                // electrons
                .set_electrons(NanoObjects::Electrons(unwrap(Electron_pt), unwrap(Electron_eta), unwrap(Electron_phi),
                                                      unwrap(Electron_cutBased), unwrap(Electron_cutBased_HEEP),
                                                      unwrap(Electron_deltaEtaSC)))
                // photons
                .set_photons(NanoObjects::Photons(unwrap(Photon_pt), unwrap(Photon_eta), unwrap(Photon_phi)))
                // taus
                .set_taus(NanoObjects::Taus(unwrap(Tau_pt), unwrap(Tau_eta), unwrap(Tau_phi)))
                // bjets
                .set_bjets(NanoObjects::BJets(unwrap(Jet_pt), unwrap(Jet_eta), unwrap(Jet_phi)))
                // jets
                .set_jets(NanoObjects::Jets(unwrap(Jet_pt), unwrap(Jet_eta), unwrap(Jet_phi)))
                // met
                .set_met(NanoObjects::MET({unwrap(MET_pt)}, {0.}, {unwrap(MET_phi)}))
                // trgobjs
                .set_trgobjs(NanoObjects::TrgObjs(unwrap(TrigObj_pt), unwrap(TrigObj_eta), unwrap(TrigObj_phi),
                                                  unwrap(TrigObj_id), unwrap(TrigObj_filterBits)));

        event_data = event_data.set_const_weights(outputs, pu_weight)
                         .generator_filter(outputs)
                         .run_lumi_filter(outputs, run_lumi_filter)
                         .npv_filter(outputs)
                         .met_filter(outputs)
                         .set_trigger_bits()
                         .trigger_filter(outputs)
                         .object_selection()
                         .has_selected_objects_filter(outputs)
                         .trigger_match_filter(outputs, trigger_sf_correctors)
                         .set_scale_factors(outputs)
                         .muon_corrections()
                         .electron_corrections()
                         .photon_corrections()
                         .tau_corrections()
                         .bjet_corrections()
                         .jet_corrections()
                         .met_corrections()
                         .fill_event_content(outputs);

        // fill output event tree
        if (event_data)
        {
            outputs.fill_event_tree();
        }

        // fmt::print("a\n");
        // process monitoring
        if (event_counter < 10 || (event_counter < 100 && event_counter % 10 == 0) ||
            (event_counter < 1000 && event_counter % 100 == 0) ||
            (event_counter < 10000 && event_counter % 1000 == 0) ||
            (event_counter >= 10000 && event_counter % 10000 == 0))
        {
            print_report(dTime1, event_counter, outputs.cutflow_histo);
            PrintProcessInfo();
        }
        // fmt::print("e\n");
    }

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
    print_report(dTime1, event_counter, outputs.cutflow_histo, true);

    // writes data to disk
    std::cout << colors.yellow << "[ Finalizing ] Output file, cutflow histograms and event data trees ..."
              << colors.def << std::endl;
    outputs.write_data();

    PrintProcessInfo();
    std::cout << colors.green << "\nDone ..." << colors.def << std::endl;

    return 0;
}
