#include "NanoMUSiC.hpp"

void PrintProcessInfo()
{
    auto info = ProcInfo_t();
    gSystem->GetProcInfo(&info);
    std::cout.precision(1);
    std::cout << std::fixed;
    std::cout << "-------------" << std::endl;
    std::cout << "Process info:" << std::endl;
    std::cout << "-------------" << std::endl;
    std::cout << "CPU time elapsed: " << info.fCpuUser << " s" << std::endl;
    std::cout << "Sys time elapsed: " << info.fCpuSys << " s" << std::endl;
    std::cout << "Resident memory:  " << info.fMemResident / 1024. << " MB" << std::endl;
    std::cout << "Virtual memory:   " << info.fMemVirtual / 1024. << " MB" << std::endl;
}

int main(int argc, char *argv[])
{
    TDirectory::AddDirectory(kFALSE); // Force ROOT to give directories in our hand - Yes, we can
    TH1::AddDirectory(kFALSE);        // Force ROOT to give histograms in our hand - Yes, we can

    // command line options
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    bool batch_mode = cmdl[{"-b", "--batch"}];
    std::string run_config_file = cmdl({"-c", "--run-config"}).str();

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

    // set colors
    Color::Modifier yellow(Color::FG_YELLOW, batch_mode);
    Color::Modifier green(Color::FG_GREEN, batch_mode);
    Color::Modifier blue(Color::FG_BLUE, batch_mode);
    Color::Modifier cyan(Color::FG_CYAN, batch_mode);
    Color::Modifier acqua(Color::FG_ACQUA, batch_mode);
    Color::Modifier red(Color::FG_RED, batch_mode);
    Color::Modifier def(Color::FG_DEFAULT, batch_mode);

    // print pretty stuff
    if (!batch_mode)
    {
        system("clear");
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << acqua << "        ███    ███ ██    ██ ███████ ██  ██████ " << def << std::endl;
        std::cout << acqua << "        ████  ████ ██    ██ ██      ██ ██      " << def << std::endl;
        std::cout << acqua << "        ██ ████ ██ ██    ██ ███████ ██ ██      " << def << std::endl;
        std::cout << acqua << "        ██  ██  ██ ██    ██      ██ ██ ██      " << def << std::endl;
        std::cout << acqua << "        ██      ██  ██████  ███████ ██  ██████ " << def << std::endl;
    }

    std::cout << " " << std::endl;
    std::cout << " " << std::endl;
    std::cout << acqua << "        MUSiC - Model Unspecific Search in CMS" << def << std::endl;
    std::cout << acqua << emojicpp::emojize("              :signal_strength: Run2 - Ultra Legacy :signal_strength:") << def
              << std::endl;
    std::cout << " " << std::endl;

    std::cout << " " << std::endl;
    std::cout << yellow << "Checking run configuration [" << run_config_file << "] ..." << def << std::endl;
    std::cout << " " << std::endl;

    // read parameters from TOML file
    const auto run_config = TOMLConfig::make_toml_config(run_config_file);
    const auto output_directory = run_config.get<std::string>("output");
    const auto process = run_config.get<std::string>("process");
    const auto dataset = run_config.get<std::string>("dataset");
    // const auto max_events = run_config.get<int>("max_events");
    // const auto number_of_events_to_skip = run_config.get<int>("number_of_events_to_skip");
    const auto is_data = run_config.get<bool>("is_data");
    const auto is_crab_job = run_config.get<bool>("is_crab_job");

    // const auto debug = run_config.get<int>("debug");
    const auto x_section_file = MUSiCTools::parse_and_expand_music_base(run_config.get<std::string>("x_section_file"));
    const auto run_hash = run_config.get<std::string>("hash");
    const auto year_str = run_config.get<std::string>("year");
    // const auto cacheread = run_config.get<bool>(cacheread");
    // const auto max_file_load_error_rate = run_config.get<float>("max_file_load_error_rate");
    const auto input_files = run_config.get_vector<std::string>("input_files");
    const auto _n_threads = run_config.get<int>("n_threads");
    if (_n_threads < 1)
    {
        throw std::runtime_error("The provided number of threads (" + std::to_string(_n_threads) + ") should be at least 1.");
    }
    std::size_t n_threads = std::min(_n_threads, static_cast<int>(input_files.size()));

    // get year as enum
    auto year = get_runyear(year_str);

    // Get the run config file from main config file.
    const auto golden_json_file = MUSiCTools::parse_and_expand_music_base(RunConfig::Runs[year].golden_json);

    if (is_data)
    {
        if (not std::filesystem::exists(golden_json_file))
        {
            std::stringstream error;
            error << "golden_json_file not found";
            throw MUSiCTools::config_error(error.str());
        }
    }
    if (!golden_json_file.empty())
    {
        std::cout << "INFO: Using Run/Lumi JSON file: " << golden_json_file << std::endl;
    }

    std::cout << " " << std::endl;
    std::cout << yellow << "Preparing output buffer ..." << def << std::endl;
    std::cout << " " << std::endl;

    const std::string startDir = getcwd(NULL, 0);

    // (Re)create output_directory dir and cd into it.
    system(("rm -rf " + output_directory).c_str());
    system(("mkdir -p " + output_directory).c_str());
    system(("cd " + output_directory).c_str());
    chdir(output_directory.c_str());

    if (!golden_json_file.empty())
        system(("cp " + golden_json_file + " . ").c_str());

    if (is_data)
        system("mkdir -p Event-lists");

    // save other configs with output
    system(("cp " + x_section_file + " . ").c_str());

    // copy rootlogon.C
    system(("cp " + MUSiCTools::parse_and_expand_music_base("$MUSIC_BASE/rootlogon.C") + " . ").c_str());

    // Init the run config
    std::cout << " " << std::endl;
    std::cout << green << "Initializing ..." << def << std::endl;
    std::cout << " " << std::endl;

    std::cout << def << "[ Initializing ] Run Lumi Filter ..." << def << std::endl;
    auto run_lumi_filter = RunLumiFilter(golden_json_file);

    std::cout << def << "[ Initializing ] PU corrections ..." << def << std::endl;
    auto pu_weight = Corrector(CorrectionTypes::PU, year, is_data);

    // read cross-sections files
    std::cout << def << "[ Initializing ] X-Sections ..." << def << std::endl;
    const auto x_sections = TOMLConfig::make_toml_config(x_section_file);

    std::cout << def << "[ Initializing ] Thread pool (" << n_threads << " threads) ..." << def << std::endl;
    ROOT::EnableImplicitMT(n_threads);

    std::cout << def << "[ Initializing ] Rochester Muon Momentum Corrections ..." << def << std::endl;
    auto rochester_corrections = Corrector(CorrectionTypes::MuonLowPt, year, is_data);

    // performance monitoring
    std::cout << def << "[ Initializing ] Performance Monitoring ..." << def << std::endl;
    auto event_counter = RVec<unsigned int>(n_threads);
    auto event_counter_mutexes = std::vector<std::mutex>(n_threads);

    const std::string process_hash = get_hash256(std::accumulate(input_files.begin(), input_files.end(), std::string("")));
    auto output_file_vec = std::vector<std::unique_ptr<TFile>>(n_threads);
    auto output_tree_vec = std::vector<std::unique_ptr<TTree>>(n_threads);
    auto classes_tree_vec = std::vector<std::unique_ptr<TTree>>(n_threads);

    auto music_event_vec = std::vector<MUSiCEvent>(n_threads);
    auto cutflow_histos_vec = std::vector<std::array<TH1F, total_variations_and_shifts>>(n_threads);
    auto classes_vec = std::vector<std::array<std::set<unsigned long>, total_variations_and_shifts>>(n_threads);
    auto classes_to_save_vec = std::vector<std::vector<unsigned long>>(n_threads);
    auto number_of_classes_vec = std::vector<unsigned long>(n_threads);
    auto variation_id_vec = std::vector<unsigned int>(n_threads);

    for (std::size_t slot = 0; slot < n_threads; slot++)
    {
        // output file
        std::string output_file_name =
            "nano_music_" + process + "_" + year_str + "_" + process_hash + "_" + std::to_string(slot) + ".root";
        if (is_crab_job)
        {
            output_file_name = "nano_music.root";
        }
        output_file_vec.at(slot) = std::unique_ptr<TFile>(TFile::Open(output_file_name.c_str(), "RECREATE"));

        // output tree
        std::ifstream t(run_config_file);
        std::stringstream output_tree_title;
        output_tree_title << t.rdbuf();
        output_tree_vec.at(slot) = std::make_unique<TTree>("nano_music", output_tree_title.str().c_str());
        output_tree_vec.at(slot)->SetDirectory(output_file_vec.at(slot).get());

        // classes tree
        classes_tree_vec.at(slot) = std::make_unique<TTree>(
            "nano_music_classes", "Classes tree: each entry corresponds to a variation. and each is a class.");
        classes_tree_vec.at(slot)->SetDirectory(output_file_vec.at(slot).get());

        // tree branches
        music_event_vec.at(slot) = MUSiCEvent{};
        output_tree_vec.at(slot)->Branch("music_event", &(music_event_vec.at(slot)), 256000, 99);
        classes_tree_vec.at(slot)->Branch("classes", &(classes_to_save_vec.at(slot)));
        classes_tree_vec.at(slot)->Branch("nClasses", &(number_of_classes_vec.at(slot)));
        classes_tree_vec.at(slot)->Branch("variation_id", &(variation_id_vec.at(slot)));

        // cutflow histo
        cutflow_histos_vec.at(slot) = make_cutflow_histos(output_file_vec.at(slot).get());
    }

    // output storages
    std::cout << def << "[ Initializing ] Temporary output storages ..." << def << std::endl;
    using EventProcessResult_t = std::array<EventData, total_variations_and_shifts>;
    auto process_results = RVec<RVec<EventProcessResult_t>>(n_threads);

    // load RDataFrame
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

    // clean Data columns
    if (is_data)
    {
        columns.erase(std::remove(columns.begin(), columns.end(), "genWeight"), columns.end());
        columns.erase(std::remove(columns.begin(), columns.end(), "Pileup_nTrueInt"), columns.end());
    }

    auto df = ROOT::RDataFrame("Events", input_files, columns);
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
        {
            const std::lock_guard<std::mutex> lock(event_counter_mutexes[slot]);
            event_counter[slot]++;
        }

        // temporaries
        RVec<float> temp_met_pt = {MET_pt};
        RVec<float> temp_met_eta = RVec<float>(1, 0.);
        RVec<float> temp_met_phi = {MET_phi};

        auto event_data = EventData(
            NanoObjects::EventInfo(run, lumi, event_number, Pileup_nTrueInt, genWeight, PV_npvsGood, Flag_goodVertices,
                                   Flag_globalSuperTightHalo2016Filter, Flag_HBHENoiseFilter, Flag_HBHENoiseIsoFilter,
                                   Flag_EcalDeadCellTriggerPrimitiveFilter, Flag_BadPFMuonFilter, Flag_BadPFMuonDzFilter,
                                   Flag_eeBadScFilter, Flag_ecalBadCalibFilter, HLT_IsoMu27, HLT_Mu50, HLT_TkMu100, HLT_OldMu100),
            NanoObjects::Muons(Muon_pt, Muon_eta, Muon_phi, Muon_tightId, Muon_highPtId, Muon_pfRelIso03_all, Muon_tkRelIso),
            NanoObjects::Electrons(Electron_pt, Electron_eta, Electron_phi),
            NanoObjects::Photons(Photon_pt, Photon_eta, Photon_phi), NanoObjects::Taus(Tau_pt, Tau_eta, Tau_phi),
            NanoObjects::BJets(Jet_pt, Jet_eta, Jet_phi), NanoObjects::Jets(Jet_pt, Jet_eta, Jet_phi),
            NanoObjects::MET(temp_met_pt, temp_met_eta, temp_met_phi), is_data, year);

        event_data = event_data.set_const_weights(pu_weight)
                         .generator_filter(cutflow_histos_vec[slot])
                         .run_lumi_filter(cutflow_histos_vec[slot], run_lumi_filter)
                         .npv_filter(cutflow_histos_vec[slot])
                         .met_filter(cutflow_histos_vec[slot])
                         .trigger_filter(cutflow_histos_vec[slot])
                         .pre_selection();

        // launch variations

        // loop over variations, shifts and classes (aka multiplicities)
        EventProcessResult_t _process_result_buffer;
        std::array<std::future<EventData>, total_variations_and_shifts> variations_futures;
        ranges::for_each(range_cleanned_variation_and_shifts | views::remove_if([&is_data](auto variation_and_shift) {
                             const auto [variation, shift] = variation_and_shift;
                             return is_data && (variation != Variation::Default);
                         }),
                         [&](const auto &variation_and_shift) {
                             const auto [variation, shift] = variation_and_shift;

                             // modify objects according to the given variation
                             _process_result_buffer.at(variation_shift_to_index(variation, shift)) =
                                 EventData::apply_corrections(event_data, variation, shift)
                                     .final_selection()
                                     .trigger_match()
                                     .set_scale_factors()
                                     .fill_event_content()
                                     .has_any_content_filter();
                         });
        //  write to tree
        // process_results.at(slot).push_back(_process_result_buffer);
        for (std::size_t idx_variation = 0; idx_variation < _process_result_buffer.size(); idx_variation++)
        {
            auto res = _process_result_buffer.at(idx_variation);
            if (res)
            {
                music_event_vec.at(slot) = MUSiCEvent{};
                music_event_vec.at(slot).run = res.event_info.run;
                music_event_vec.at(slot).lumi_section = res.event_info.lumi;
                music_event_vec.at(slot).event_number = res.event_info.event;
                music_event_vec.at(slot).trigger_bits = res.trigger_bits.as_ulong();
                music_event_vec.at(slot).fill(std::move(res.event_content), idx_variation);

                classes_vec.at(slot).at(idx_variation).merge(res.classes);
            }
        }
        // save data into output tree
        output_tree_vec.at(slot)->Fill();
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
    std::cout << green << "Starting Classification ..." << def << std::endl;
    std::cout << " " << std::endl;

    std::cout << acqua << "Launching monitoring thread ..." << def << std::endl;
    double dTime1 = getCpuTime(); // Start Timer
    bool run_monitoring = true;
    std::thread monitoring_thread([&]() {
        unsigned int processed_events_counter = 0;
        while (run_monitoring)
        {
            // will sleep for 10 sec
            std::this_thread::sleep_for(10000ms);

            // loop over threads and count the number of processed events
            for (std::size_t i_slot = 0; i_slot < n_threads; i_slot++)
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

    std::cout << green << "\nLaunching event loop ..." << def << std::endl;
    if (is_data)
    {
        df.ForeachSlot(event_processor_Data, columns);
    }
    else
    {
        df.ForeachSlot(event_processor_MC, columns);
    }
    run_monitoring = false;
    monitoring_thread.join();
    std::cout << green << "Event loop done ..." << def << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////       [ END ]        //////////////////////////////////////
    //////////////////////////////////////   loop over events   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////

    std::cout << " " << std::endl;
    std::cout << green << "Classification done ..." << def << std::endl;
    std::cout << " " << std::endl;

    double dTime2 = getCpuTime();
    std::cout << cyan << "[ Final Performance Report ] Analyzed " << Sum(event_counter) << " events";
    std::cout << ", elapsed CPU time: " << dTime2 - dTime1 << "sec (" << double(Sum(event_counter)) / (dTime2 - dTime1)
              << " evts per sec)" << def << std::endl;

    if (Sum(event_counter) == 0)
    {
        std::cout << "Error: No event was analyzed!" << std::endl;
        throw std::runtime_error("No event was analyzed!");
    }

    std::cout << "\n" << std::endl;

    // writes data to disk
    std::cout << yellow << "[ Finalizing ] Output file, cutflow and tree ..." << def << std::endl;
    std::vector<std::thread> output_writers;
    for (std::size_t i_slot = 0; i_slot < n_threads; i_slot++)
    {
        output_writers.push_back(std::thread(
            [&](auto slot) {
                // write outputs to disk
                output_file_vec.at(slot)->cd();
                output_tree_vec.at(slot)->Write();
                for (auto &histo : cutflow_histos_vec.at(slot))
                {
                    histo.Write();
                }

                // convert the std::set<unsigned long> of classes that were touched a std::vector<unsigned long>
                for (std::size_t idx_var = 0; idx_var < classes_vec.at(slot).size(); idx_var++)
                {
                    classes_to_save_vec.at(slot) = std::vector<unsigned long>(classes_vec.at(slot).at(idx_var).begin(),
                                                                              classes_vec.at(slot).at(idx_var).end());
                    number_of_classes_vec.at(slot) = classes_to_save_vec.at(slot).size();
                    variation_id_vec.at(slot) = idx_var;
                    classes_tree_vec.at(slot)->Fill();
                }
                // saves into the output root file
                output_file_vec.at(slot)->cd();
                classes_tree_vec.at(slot)->Write();
            },
            i_slot));
    }

    std::cout << yellow << "[ Finalizing ] Waiting for data to be written ...\n" << def << std::endl;
    for (auto &writer : output_writers)
    {
        writer.join();
    }

    PrintProcessInfo();
    return 0;
}
