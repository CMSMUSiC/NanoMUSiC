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
    const auto max_events = run_config.get<int>("max_events");
    const auto number_of_events_to_skip = run_config.get<int>("number_of_events_to_skip");
    const auto is_data = run_config.get<bool>("is_data");
    const auto debug = run_config.get<int>("debug");
    const auto x_section_file = MUSiCTools::parse_and_expand_music_base(run_config.get<std::string>("x_section_file"));
    const auto run_hash = run_config.get<std::string>("hash");
    const auto year_str = run_config.get<std::string>("year");
    const auto cacheread = run_config.get<bool>("cacheread");
    const auto max_file_load_error_rate = run_config.get<float>("max_file_load_error_rate");
    const auto input_files = run_config.get_vector<std::string>("input_files");
    const auto n_threads = run_config.get<int>("n_threads");
    if (n_threads < 1)
    {
        throw std::runtime_error("The provided number of threads (" + std::to_string(n_threads) + ") should be at least 1.");
    }

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

    // Init the run config
    std::cout << " " << std::endl;
    std::cout << green << "Initializing ..." << def << std::endl;
    std::cout << " " << std::endl;

    std::cout << def << "[Initializing] Run Lumi Filter ..." << def << std::endl;
    auto run_lumi_filter = RunLumiFilter(golden_json_file);

    std::cout << def << "[Initializing] PU corrections ..." << def << std::endl;
    auto pu_corrections = Corrector(CorrectionTypes::PU, year, is_data);

    // read cross-sections files
    std::cout << def << "[Initializing] X-Sections ..." << def << std::endl;
    const auto x_sections = TOMLConfig::make_toml_config(x_section_file);

    std::cout << def << "[Initializing] Output file ..." << def << std::endl;
    const std::string process_hash = get_hash256(std::accumulate(input_files.begin(), input_files.end(), std::string("")));
    const auto output_file_name = "nano_music_" + process + "_" + year_str + "_" + process_hash + ".root";
    std::unique_ptr<TFile> output_file(TFile::Open(output_file_name.c_str(), "RECREATE"));

    std::cout << def << "[Initializing] Output tree ..." << def << std::endl;
    std::ifstream t(run_config_file);
    std::stringstream output_tree_title;
    output_tree_title << t.rdbuf();
    auto output_tree = std::make_unique<TTree>("nano_music", output_tree_title.str().c_str());
    output_tree->SetDirectory(output_file.get());

    std::cout << def << "[Initializing] Output tree branches ..." << def << std::endl;
    auto music_event = MUSiCEvent{};
    output_tree->Branch("music_event", &music_event, 256000, 99);

    std::cout << def << "[Initializing] Cut flow histo ..." << def << std::endl;
    auto cutflow_histo = make_cutflow_histo(output_file.get(), output_tree_title);

    std::cout << def << "[Initializing] Set of classes ..." << def << std::endl;
    std::set<unsigned long> classes;

    std::cout << def << "[Initializing] Task runners (thread pool) ..." << def << std::endl;
    BS::thread_pool task_runners_pool(n_threads);

    // performance monitoring
    double dTime1 = getCpuTime(); // Start Time
    int event_counter = -1;       // Event counter
    unsigned int skipped = 0;     // number of events skipped from run/lumi_section config
    int pre_run_skipped = 0;      // number of events skipped due to skipped option

    // file counters
    unsigned int analyzed_files = 0;
    int file_load_error = 0; // number of files that could not be opened

    // temp cache dir
    std::string cache_dir = "/tmp/music/proc_" + process_hash;
    if (cacheread)
    {
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << yellow << "Preparing cache directory for NanoAOD files: " << def << std::endl;
        system(("rm -rf " + cache_dir).c_str());
        system(("mkdir -p " + cache_dir).c_str());
        std::cout << cache_dir << std::endl;
    }

    std::cout << " " << std::endl;
    std::cout << green << "Starting Classification ..." << def << std::endl;
    std::cout << " " << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////       [ BEGIN ]      //////////////////////////////////////
    //////////////////////////////////////    loop over files   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    auto first_file = input_files.at(0);
    std::cout << "Loading first file ..." << std::endl;
    OptionalFuture_t input_root_file_future = std::async(std::launch::async, file_loader, first_file, cacheread, cache_dir, true);
    auto input_root_file = input_root_file_future->get();
    std::cout << "... done." << std::endl;
    for (size_t idx_file = 0; idx_file < input_files.size(); idx_file++)
    {
        // after first file iteration, load next file
        if (input_root_file_future && idx_file > 0)
        {
            std::cout << "Getting next file ..." << std::endl;
            input_root_file = input_root_file_future->get();
        }

        if (!input_root_file)
        {
            if (is_data)
            {
                std::cout << "[ERROR]: could not open file" << std::endl;
                exit(1);
            }
            std::cout << "[WARNINIG]: could not open file: " << input_files.at(idx_file) << std::endl;
            file_load_error++;

            // will exit, if error rate < max
            if (float(file_load_error) / float(input_files.size()) > max_file_load_error_rate)
            {
                std::cout << "Too many file load errors: " << file_load_error << " out of " << input_files.size() << "."
                          << std::endl;
                exit(1);
            }

            // skip file if not Data and error rate < max
            continue;
        }

        time_t rawtime;
        time(&rawtime);
        std::cout << "Opening time: " << ctime(&rawtime);

        analyzed_files++;

        unsigned int event_counter_per_file = 0;

        // get "Events" TTree from file
        auto events_tree = std::unique_ptr<TTree>(input_root_file->Get<TTree>("Events"));
        // launch new thread
        if (idx_file + 1 < input_files.size())
        {
            input_root_file_future =
                std::async(std::launch::async, file_loader, input_files.at(idx_file + 1), cacheread, cache_dir, true);
        }
        else
        {
            std::cout << "End of file list ..." << std::endl;
            input_root_file_future = std::nullopt;
        }

        std::cout << def << yellow << "Analyzing: " << input_files.at(idx_file) << def << std::endl;

        // get NanoAODReader
        auto nano_reader = NanoAODReader(*events_tree);

        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////       [ BEGIN ]      //////////////////////////////////////
        //////////////////////////////////////   loop over events   //////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        double dTime_0 = getCpuTime();
        unsigned int event_counter_buffer = 0;

        while (nano_reader.next())
        {
            event_counter++;
            // if set, skip first events
            if (number_of_events_to_skip > pre_run_skipped)
            {
                pre_run_skipped++;
                event_counter_per_file++;
                continue;
            }

            if (event_counter > 0)
            {
                // print every tenth
                if (is_tenth(event_counter))
                {
                    auto _now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    std::cout << "\n[INFO] - " << std::strtok(std::ctime(&_now), "\n") << " - ";
                    if (pre_run_skipped == 0)
                    {
                        std::cout << event_counter << " Events analyzed (" << skipped << " skipped)";
                    }
                    else
                    {
                        std::cout << event_counter << " Events analyzed (" << pre_run_skipped << " + " << skipped << " skipped)";
                    }

                    std::cout << " - Processing rate: " << double(event_counter - event_counter_buffer) / (getCpuTime() - dTime_0)
                              << " Events/s" << std::endl;
                    event_counter_buffer = event_counter;
                    dTime_0 = getCpuTime();

                    PrintProcessInfo();
                }
            }

            // stop if max number of events to be processed is reached
            if (max_events >= 0 && event_counter > max_events)
            {
                break;
            }

            // reset content per event
            music_event = MUSiCEvent{};

            event_counter_per_file++;

            // load event const's
            music_event.run = nano_reader.getVal<UInt_t>("run");
            music_event.lumi_section = nano_reader.getVal<UInt_t>("luminosityBlock");
            music_event.event_number = nano_reader.getVal<ULong64_t>("event");

            // only if data
            // filter run and lumi section
            if (!run_lumi_filter(music_event.run, music_event.lumi_section, is_data))
            {
                ++skipped;
                if (debug > 1)
                {
                    std::cerr << "[INFO] (SkipEvents): " << std::endl;
                    std::cerr << "Skipping Run/lumi_section/Event: ";
                    std::cerr << music_event.run << ":" << music_event.lumi_section << ":" << music_event.event_number
                              << std::endl;
                }
                continue;
            }

            // initialize default weight
            float current_event_weight = [&]() {
                if (is_data)
                {
                    return 1.0;
                }
                /////////////////////////////////////////////
                /////////////////////////////////////////////
                // FIX ME: get MC weight
                /////////////////////////////////////////////
                /////////////////////////////////////////////
                return 1.0;
            }();
            cutflow_histo.Fill(CutFlow::NoCuts, current_event_weight);

            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // Apply CMS standard PV cuts
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            if (!(nano_reader.getVal<Int_t>("PV_npvsGood") > 0))
            {
                continue;
            }
            cutflow_histo.Fill(CutFlow::nPV, current_event_weight);

            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // MET event filters
            // https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFiltersRun2#MET_Filter_Recommendations_for_R
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            bool pass_MET_filters = true;
            if (year == Year::Run2016APV || year == Year::Run2016)
            {
                pass_MET_filters =
                    pass_MET_filters && nano_reader.getVal<Bool_t>("Flag_goodVertices") &&
                    nano_reader.getVal<Bool_t>("Flag_globalSuperTightHalo2016Filter") &&
                    nano_reader.getVal<Bool_t>("Flag_HBHENoiseFilter") && nano_reader.getVal<Bool_t>("Flag_HBHENoiseIsoFilter") &&
                    nano_reader.getVal<Bool_t>("Flag_EcalDeadCellTriggerPrimitiveFilter") &&
                    nano_reader.getVal<Bool_t>("Flag_BadPFMuonFilter") && nano_reader.getVal<Bool_t>("Flag_BadPFMuonDzFilter") &&
                    nano_reader.getVal<Bool_t>("Flag_eeBadScFilter");
                // nano_reader.getVal<Bool_t>("Flag_BadChargedCandidateFilter");
                // nano_reader.getVal<Bool_t>("Flag_hfNoisyHitsFilter");
            }

            if (year == Year::Run2017 || year == Year::Run2017)
            {

                pass_MET_filters =
                    pass_MET_filters && nano_reader.getVal<Bool_t>("Flag_goodVertices") &&
                    nano_reader.getVal<Bool_t>("Flag_globalSuperTightHalo2016Filter") &&
                    nano_reader.getVal<Bool_t>("Flag_HBHENoiseFilter") && nano_reader.getVal<Bool_t>("Flag_HBHENoiseIsoFilter") &&
                    nano_reader.getVal<Bool_t>("Flag_EcalDeadCellTriggerPrimitiveFilter") &&
                    nano_reader.getVal<Bool_t>("Flag_BadPFMuonFilter") && nano_reader.getVal<Bool_t>("Flag_BadPFMuonDzFilter") &&
                    nano_reader.getVal<Bool_t>("Flag_eeBadScFilter") && nano_reader.getVal<Bool_t>("Flag_ecalBadCalibFilter");
                // nano_reader.getVal<Float_t>("Flag_hfNoisyHitsFilter");
                // nano_reader.getVal<Float_t>("Flag_BadChargedCandidateFilter");
            }
            if (!pass_MET_filters)
            {
                continue;
            }
            cutflow_histo.Fill(CutFlow::MetFilters, current_event_weight);

            ////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////
            // fill trigger bits
            ////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////
            TriggerBits trigger_bits;
            trigger_bits.set(HLTPath::SingleMuonLowPt, false)
                .set(HLTPath::SingleMuonHighPt, false)
                .set(HLTPath::SingleElectron, false)
                .set(HLTPath::DoubleMuon, false)
                .set(HLTPath::DoubleElectron, false)
                .set(HLTPath::Tau, false)
                .set(HLTPath::BJet, false)
                .set(HLTPath::MET, false)
                .set(HLTPath::Photon, false);
            if (year == Year::Run2016APV || year == Year::Run2016)
            {
            }
            else if (year == Year::Run2017)
            {

                trigger_bits.set(HLTPath::SingleMuonLowPt, nano_reader.getVal<Bool_t>("HLT_IsoMu27"))
                    .set(HLTPath::SingleMuonHighPt, nano_reader.getVal<Bool_t>("HLT_Mu50") ||
                                                        nano_reader.getVal<Bool_t>("HLT_TkMu100") ||
                                                        nano_reader.getVal<Bool_t>("HLT_OldMu100"))
                    .set(HLTPath::SingleElectron, false)
                    .set(HLTPath::DoubleMuon, false)
                    .set(HLTPath::DoubleElectron, false)
                    .set(HLTPath::Tau, false)
                    .set(HLTPath::BJet, false)
                    .set(HLTPath::MET, false)
                    .set(HLTPath::Photon, false);
            }
            else if (year == Year::Run2018)
            {
            }
            else
            {
                throw std::runtime_error("Year (" + std::to_string(year) +
                                         ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
            }
            music_event.trigger_bits = trigger_bits.as_uint();

            // skip event event if no trigger is fired
            if (!trigger_bits.any())
            {
                continue;
            }
            cutflow_histo.Fill(CutFlow::TriggerCut, current_event_weight);

            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // FIX ME: Prefiring weights!!!!
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////

            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // Load NanoObjects
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////

            // Muons
            auto muons = NanoObject::make_collection(
                nano_reader.getVec<Float_t>("Muon_pt"), nano_reader.getVec<Float_t>("Muon_eta"),
                nano_reader.getVec<Float_t>("Muon_phi"), nano_reader.getVec<Float_t>("Muon_mass"),
                NanoObject::make_feature("tightId", nano_reader.getVecOfBools("Muon_tightId")),
                NanoObject::make_feature("highPtId", nano_reader.getVec<UChar_t>("Muon_highPtId")),
                NanoObject::make_feature("pfRelIso03_all", nano_reader.getVec<Float_t>("Muon_pfRelIso03_all")),
                NanoObject::make_feature("tkRelIso", nano_reader.getVec<Float_t>("Muon_tkRelIso")));

            // Muon - Filter
            auto good_muons_future = task_runners_pool.submit(
                [&](const auto &muons) {
                    return NanoObject::Filter(muons, [&](const auto &muon) {
                        // low pT muon
                        if (muon.pt() >= ObjConfig::Muons[year].MinLowPt && muon.pt() < ObjConfig::Muons[year].MaxLowPt &&
                            std::fabs(muon.eta()) <= ObjConfig::Muons[year].MaxAbsEta)
                        {
                            if (muon.template get<UInt_t>("tightId") &&
                                muon.template get<Float_t>("pfRelIso03_all") < ObjConfig::Muons[year].PFRelIso_WP)
                            {
                                return true;
                            }
                        }

                        // high pT muon
                        if (muon.pt() >= ObjConfig::Muons[year].MaxLowPt &&
                            std::fabs(muon.eta()) <= ObjConfig::Muons[year].MaxAbsEta)
                        {
                            if (muon.template get<UChar_t>("highPtId") >= 1 &&
                                muon.template get<Float_t>("tkRelIso") < ObjConfig::Muons[year].TkRelIso_WP)
                            {
                                return true;
                            }
                        }
                        return false;
                    });
                },
                muons);

            // Electrons
            auto electrons = NanoObject::make_collection(
                nano_reader.getVec<Float_t>("Electron_pt"), nano_reader.getVec<Float_t>("Electron_eta"),
                nano_reader.getVec<Float_t>("Electron_phi"), nano_reader.getVec<Float_t>("Electron_mass"));

            // Photons
            auto photons = NanoObject::make_collection(
                nano_reader.getVec<Float_t>("Photon_pt"), nano_reader.getVec<Float_t>("Photon_eta"),
                nano_reader.getVec<Float_t>("Photon_phi"), nano_reader.getVec<Float_t>("Photon_mass"));

            // Taus
            auto taus =
                NanoObject::make_collection(nano_reader.getVec<Float_t>("Tau_pt"), nano_reader.getVec<Float_t>("Tau_eta"),
                                            nano_reader.getVec<Float_t>("Tau_phi"), nano_reader.getVec<Float_t>("Tau_mass"));

            // Jets
            auto jets = NanoObject::make_collection(
                nano_reader.getVec<Float_t>("Jet_pt"), nano_reader.getVec<Float_t>("Jet_eta"),
                nano_reader.getVec<Float_t>("Jet_phi"), nano_reader.getVec<Float_t>("Jet_mass"),
                NanoObject::make_feature(
                    std::string(ObjConfig::Jets[year].btag_algo),
                    nano_reader.getVec<Float_t>(std::string("Jet_") + std::string(ObjConfig::Jets[year].btag_algo))));

            // BJets (a subset of jets)
            auto bjets = NanoObject::Filter(jets, [&](auto &jet) {
                if ((jet.template get<Float_t>(ObjConfig::Jets[year].btag_algo)) >= ObjConfig::Jets[year].btag_wp_tight)
                {
                    return true;
                }
                return false;
            });

            // Jets again: one should remove bjets from jets
            jets = NanoObject::Filter(jets, [&](auto &&jet) {
                if (!((jet.template get<Float_t>(ObjConfig::Jets[year].btag_algo)) >= ObjConfig::Jets[year].btag_wp_tight))
                {
                    return true;
                }
                return false;
            });

            // MET
            auto met = NanoObject::make_object(
                nano_reader.getVal<Float_t>("MET_pt"), nano_reader.getVal<Float_t>("MET_phi"),
                NanoObject::make_feature("significance", nano_reader.getVal<Float_t>("MET_significance")),
                NanoObject::make_feature("MetUnclustEnUpDeltaX", nano_reader.getVal<Float_t>("MET_MetUnclustEnUpDeltaX")));

            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // Modify and Filter Objects
            // FIX ME: add corrections!!!
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////

            // Electron - Filter
            auto good_electrons_future = task_runners_pool.submit(
                [](const auto &electrons) {
                    return NanoObject::Filter(electrons, [](const auto &electron) {
                        if (electron.pt() > 25.0)
                        {
                            return true;
                        }
                        return false;
                    });
                },
                electrons);

            // Photon - Filter
            auto good_photons_future = task_runners_pool.submit(
                [](const auto &photons) {
                    return NanoObject::Filter(photons, [](const auto &photon) {
                        if (photon.pt() > 25.0)
                        {
                            return true;
                        }
                        return false;
                    });
                },
                photons);

            // Tau - Filter
            auto good_taus_future = task_runners_pool.submit(
                [](const auto &taus) {
                    return NanoObject::Filter(taus, [](const auto &tau) {
                        // dummy filter
                        return true;
                    });
                },
                taus);

            // BJet - Filter
            auto good_bjets_future = task_runners_pool.submit(
                [](const auto &bjets) {
                    return NanoObject::Filter(bjets, [](const auto &bjet) {
                        if (bjet.pt() > 50.0)
                        {
                            return true;
                        }
                        return false;
                    });
                },
                bjets);

            // Jet - Filter
            auto good_jets_future = task_runners_pool.submit(
                [](const auto &jets) {
                    return NanoObject::Filter(jets, [](const auto &jet) {
                        if (jet.pt() > 50.0)
                        {
                            return true;
                        }
                        return false;
                    });
                },
                jets);

            // MET - Filter
            met.set("good_met", [&]() {
                if (met.pt() > 100.)
                {
                    return true;
                }
                return false;
            }());

            // collect async'ed filtered objects
            auto good_muons = get_and_check_future(good_muons_future);
            auto good_electrons = get_and_check_future(good_electrons_future);
            auto good_photons = get_and_check_future(good_photons_future);
            auto good_taus = get_and_check_future(good_taus_future);
            auto good_bjets = get_and_check_future(good_bjets_future);
            auto good_jets = get_and_check_future(good_jets_future);

            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // Match objects to trigger
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////

            // Trigger Objects
            auto trigger_ids = nano_reader.getVec<Int_t>("TrigObj_id");
            std::vector<float> trigger_objects_masses;
            std::transform(trigger_ids.cbegin(), trigger_ids.cend(), std::back_inserter(trigger_objects_masses),
                           [&](const auto &id) {
                               if (std::abs(id) == PDG::Muon::Id)
                               {
                                   return PDG::Muon::Mass;
                               }
                               if (std::abs(id) == PDG::Electron::Id)
                               {
                                   return PDG::Electron::Mass;
                               }
                               return float(0.);
                           });
            auto trigger_objects = NanoObject::make_collection(
                nano_reader.getVec<Float_t>("TrigObj_pt"), nano_reader.getVec<Float_t>("TrigObj_eta"),
                nano_reader.getVec<Float_t>("TrigObj_phi"), std::move(trigger_objects_masses),
                NanoObject::make_feature("id", nano_reader.getVec<Int_t>("TrigObj_id")),
                NanoObject::make_feature("filterBits", nano_reader.getVec<Int_t>("TrigObj_filterBits")));

            // holds the trigger match event state
            bool has_trigger_match = false;

            // test matches for SingleMuon
            auto trigger_low_pt_muons =
                trigger_objects |
                views::remove_if([&](const auto &trg_obj) { return (std::abs(trg_obj.template get<Int_t>("id")) != 13); }) |
                views::remove_if([&](const auto &trg_obj) {
                    return !(TriggerBits::check_bit(trg_obj.template get<Int_t>("filterBits"), HLTPath::SingleMuonLowPt, year));
                });

            auto trigger_high_pt_muons =
                trigger_objects |
                views::remove_if([&](const auto &trg_obj) { return (std::abs(trg_obj.template get<Int_t>("id")) != 13); }) |
                views::remove_if([&](const auto &trg_obj) {
                    return !(TriggerBits::check_bit(trg_obj.template get<Int_t>("filterBits"), HLTPath::SingleMuonHighPt, year));
                });

            auto low_pt_good_muons =
                good_muons | views::remove_if([&](const auto &muon) { return (muon.pt() >= ObjConfig::Muons[year].MaxLowPt); });

            auto high_pt_good_muons =
                good_muons | views::remove_if([&](const auto &muon) { return (muon.pt() < ObjConfig::Muons[year].MaxLowPt); });

            // checks for Single Muon - Low pT
            if (!has_trigger_match && trigger_bits.pass(HLTPath::SingleMuonLowPt))
            {
                auto [_trigger_match, _sf_nominal, _sf_up, _sf_down] =
                    TriggerBits::trigger_matcher(trigger_low_pt_muons, low_pt_good_muons, year);
                if (_trigger_match)
                {
                    has_trigger_match = true;
                    music_event.event_weight.set(Weight::Trigger, Shift::Nominal, _sf_nominal);
                    music_event.event_weight.set(Weight::Trigger, Shift::Up, _sf_up);
                    music_event.event_weight.set(Weight::Trigger, Shift::Down, _sf_down);
                }
            }

            // checks for Single Muon - High pT
            if (!has_trigger_match && trigger_bits.pass(HLTPath::SingleMuonHighPt))
            {
                auto [_trigger_match, _sf_nominal, _sf_up, _sf_down] =
                    TriggerBits::trigger_matcher(trigger_high_pt_muons, high_pt_good_muons, year);
                if (_trigger_match)
                {
                    has_trigger_match = true;
                    music_event.event_weight.set(Weight::Trigger, Shift::Nominal, _sf_nominal);
                    music_event.event_weight.set(Weight::Trigger, Shift::Up, _sf_up);
                    music_event.event_weight.set(Weight::Trigger, Shift::Down, _sf_down);
                }
            }

            // skip event event if no trigger object is matched
            if (!has_trigger_match)
            {
                continue;
            }
            cutflow_histo.Fill(CutFlow::TriggerMatch, current_event_weight);

            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // Fill total number of events per events
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            /* total number of good muons */
            music_event.n_muons = good_muons.size();
            /* total number of good electrons */
            music_event.n_electrons = good_electrons.size();
            /* total number of good photons */
            music_event.n_photons = good_photons.size();
            /* total number of good taus */
            music_event.n_taus = 0;
            /* total number of good bjets */
            music_event.n_bjets = good_bjets.size();
            /* total number of good jets */
            music_event.n_jets = good_jets.size();
            /* total number of good jets */
            music_event.n_met = static_cast<int>(met.get<bool>("good_met"));

            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // Get classes (aka multiplicities) and fill event
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////
            // holds the number of classes for this event
            auto multiplicities =
                EventContent::get_multiplicities(good_muons.size(),     /* total number of good muons */
                                                 good_electrons.size(), /* total number of good electrons */
                                                 good_photons.size(),   /* total number of good photons */
                                                 0,                     /* total number of good taus */
                                                 good_bjets.size(),     /* total number of good bjets */
                                                 good_jets.size(),      /* total number of good jets */
                                                 static_cast<int>(met.get<bool>("good_met")) /* is good MET? */);

            unsigned long n_classes = ranges::distance(multiplicities.begin(), multiplicities.end());

            // skip event if there are no classes to be analyzed
            if (n_classes == 0)
            {
                continue;
            }
            cutflow_histo.Fill(CutFlow::AtLeastOneClass, current_event_weight);

            // fill total of classes and update the set of classes
            ranges::for_each(multiplicities,
                             [&](const auto &multiplicity) { classes.emplace(EventContent::get_class_hash(multiplicity)); });
            music_event.n_classes = n_classes;

            // 2D array to hold futures-fills and event content (for each pair of Variation x Shift)
            std::array<std::future<void>, Variation::kTotalVariations * Shift::kTotalShifts> content_fill_futures;
            std::array<EventContent, Variation::kTotalVariations * Shift::kTotalShifts> content_buffers;

            // loop over variations, shifts and classes (aka multiplicities)
            ranges::for_each(views::cartesian_product(range_variations, range_shifts), [&](const auto &variation_and_shift) {
                const auto [variation, shift] = variation_and_shift;

                // modify objects according to the given variation
                const auto varied_objects = apply_variation(variation, shift, is_data, good_muons, good_electrons, good_photons,
                                                            good_taus, good_bjets, good_jets, met);

                // launch async tasks
                content_fill_futures.at(Shift::kTotalShifts * variation + shift) = task_runners_pool.submit(
                    [&](const auto &variation, const auto &shift, const auto &varied_objects) {
                        ranges::for_each(multiplicities, [&](const auto &multiplicity) {
                            content_buffers.at(Shift::kTotalShifts * variation + shift).fill(multiplicity, varied_objects);
                        });
                    },
                    variation, shift, varied_objects);
            });

            // loop over variations, shifts and classes (aka multiplicities)
            ranges::for_each(views::cartesian_product(range_variations, range_shifts), [&](const auto &variation_and_shift) {
                const auto [variation, shift] = variation_and_shift;

                try
                {
                    // check whether async task is done
                    content_fill_futures.at(Shift::kTotalShifts * variation + shift).get();

                    // save event content for this pair of variation + shift
                    music_event.fill(variation, shift, std::move(content_buffers.at(Shift::kTotalShifts * variation + shift)));
                }
                catch (const std::exception &e)
                {
                    std::cout << "[ERROR] Caught exception when trying to fill content buffers." << std::endl;
                    std::cout << e.what() << std::endl;
                    exit(1);
                }
            });

            // save data into output tree
            output_tree->Fill();
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////       [ END ]        //////////////////////////////////////
        //////////////////////////////////////   loop over events   //////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////

        // flushes the output_tree to the output_file every input_file
        if (output_tree->GetEntriesFast() > 0)
        {
            output_tree->AutoSave();
        }

        // stop if max number of events to be processed is reaced
        if (max_events != -1 && event_counter > max_events)
        {
            break;
        }

        // clear cache dir
        if (cacheread)
        {
            std::cout << yellow << "Cleaning NanoAOD cache ..." << def << std::endl;
            std::string cached_file_path = input_root_file->GetName();
            system(("rm -rf " + cached_file_path).c_str()); // FIX-ME!!
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////        [ END ]       //////////////////////////////////////
    //////////////////////////////////////    loop over files   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    double dTime2 = getCpuTime();
    std::cout << "Analyzed " << event_counter << " Events, skipped " << pre_run_skipped << " first events and " << skipped
              << " due to run/ls veto";
    std::cout << ", elapsed CPU time: " << dTime2 - dTime1 << " (" << double(event_counter) / (dTime2 - dTime1)
              << " evts per sec)" << std::endl;

    if (file_load_error > 0)
    {
        std::cout << "Warning: " << file_load_error << " of " << (file_load_error + analyzed_files)
                  << " files lost due to timeouts or read errors." << std::endl;
    }

    if ((event_counter + skipped) == 0)
    {
        std::cout << "Error: No event analayzed!" << std::endl;
        throw std::runtime_error("No event analayzed!");
    }

    std::cout << "\n\n\n" << std::endl;

    // write tree to file
    std::cout << def << "[Finalizing] Output file, cutflow and tree ..." << def << std::endl;
    output_file->cd();
    output_tree->Write();
    cutflow_histo.Write();

    // convert the std::list of classes that were touched to a comma separated
    // string and write it to the the outputfile.classes
    save_class_storage(classes, output_file_name);

    PrintProcessInfo();
    return 0;
}
