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
    const bool batch_mode = cmdl[{"-b", "--batch"}];
    const std::string run_config_file = cmdl({"-c", "--run-config"}).str();
    if (show_help || run_config_file == "")
    {
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << "MUSiC - Model Unspecific Search in CMS" << std::endl;
        std::cout << emojicpp::emojize("      :signal_strength: Run2 - Ultra Legacy :signal_strength:") << std::endl;
        std::cout << " " << std::endl;
        if (run_config_file == "")
        {
            std::cout << "ERROR: the option '--config' is required but missing" << std::endl;
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
    std::cout << acqua << emojicpp::emojize("              :signal_strength: Run2 - Ultra Legacy :signal_strength:")
              << def << std::endl;
    std::cout << " " << std::endl;

    std::cout << " " << std::endl;
    std::cout << yellow << "Checking run configuration [" << run_config_file << "] ..." << def << std::endl;
    std::cout << " " << std::endl;

    // read parameters from TOML file
    const auto run_config = TOMLConfig::make_toml_config(run_config_file);
    const auto output_directory = run_config.get<std::string>("output");
    const auto analysis_config_file = Tools::AbsolutePath(run_config.get<std::string>("config"));
    const auto process = run_config.get<std::string>("process");
    const auto dataset = run_config.get<std::string>("dataset");
    const auto max_events = run_config.get<int>("max_events");
    const auto number_of_events_to_skip = run_config.get<int>("number_of_events_to_skip");
    const auto is_data = run_config.get<bool>("is_data");
    const auto debug = run_config.get<int>("debug");
    const auto x_section_file = Tools::AbsolutePath(run_config.get<std::string>("x_section_file"));
    const auto run_hash = run_config.get<std::string>("hash");
    const auto year = run_config.get<std::string>("year");
    const auto cacheread = run_config.get<bool>("cacheread");
    const auto input_files = run_config.get_vector<std::string>("input_files");

    // check year
    if (year != "2016APV" && year != "2016" && year != "2017" && year != "2018")
    {
        std::cerr << red << "ERROR: year should be 2016APV, 2016, 2017 or 2018" << def << std::endl;
        return 1;
    }

    if (not std::filesystem::exists(analysis_config_file))
    {
        throw Tools::file_not_found(analysis_config_file, "Config file");
    }
    else
        std::cout << "INFO: Using Config file: " << analysis_config_file << std::endl;

    auto config = Tools::MConfig(analysis_config_file);
    config.setYear(year);

    // Get the run config file from main config file.
    const auto golden_json_file = std::string(std::getenv("MUSIC_BASE")) + "/configs/golden_jsons/" + year + ".txt";

    if (is_data)
    {
        if (not std::filesystem::exists(golden_json_file))
        {
            std::stringstream error;
            error << "golden_json_file '" << golden_json_file << "' ";
            error << "in config file: '" << analysis_config_file << "' not found!";
            throw Tools::config_error(error.str());
        }
    }
    if (!golden_json_file.empty())
    {
        std::cout << "INFO: Using Run config file: " << golden_json_file << std::endl;
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
    system(("cp " + analysis_config_file + " . ").c_str());

    // dump config to file
    config.DumpToFile("config_full.txt");

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
    const std::string process_hash =
        get_hash256(std::accumulate(input_files.begin(), input_files.end(), std::string("")));
    const auto output_file_name = "nano_music_" + process + "_" + year + "_" + process_hash + ".root";
    std::unique_ptr<TFile> output_file(TFile::Open(output_file_name.c_str(), "RECREATE"));

    std::cout << def << "[Initializing] Output tree ..." << def << std::endl;
    std::ifstream t(run_config_file);
    std::stringstream output_tree_title;
    output_tree_title << t.rdbuf();
    auto output_tree = std::make_unique<TTree>("nano_music", output_tree_title.str().c_str());

    std::cout << def << "[Initializing] Output tree branches ..." << def << std::endl;
    auto music_event = MUSiCEvent{};
    output_tree->Branch("music_event", &music_event, 256000, 99);

    std::cout << def << "[Initializing] Cut flow histo ..." << def << std::endl;
    const auto n_cuts = 10;
    auto cutflow_histo = TH1F("cutflow_histo", output_tree_title.str().c_str(), n_cuts, -0.5, n_cuts + 0.5);
    cutflow_histo.Sumw2();

    // performance monitoring
    double dTime1 = getCpuTime(); // Start Time
    int event_counter = -1;       // Event counter
    unsigned int skipped = 0;     // number of events skipped from run/lumi_section config
    int pre_run_skipped = 0;      // number of events skipped due to skipped option

    // file counters
    unsigned int analyzed_files = 0;
    unsigned int lost_files = 0;

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
    OptionalFuture_t input_root_file_future =
        std::async(std::launch::async, get_TFile, first_file, cacheread, cache_dir);
    auto input_root_file = input_root_file_future->get();
    std::cout << "... done." << std::endl;
    for (size_t i = 0; i < input_files.size(); i++)
    {
        if (!input_root_file)
        {
            std::cout << "ERROR: could not open file" << std::endl;
            exit(1);
        }
        else

        {
            time_t rawtime;
            time(&rawtime);
            std::cout << "Opening time: " << ctime(&rawtime);

            analyzed_files++;
            int event_counter_per_file = 0;

            // get "Events" TTree from file
            auto events_tree = std::unique_ptr<TTree>(input_root_file->Get<TTree>("Events"));

            // launch new thread
            if (i + 1 < input_files.size())
            {
                input_root_file_future =
                    std::async(std::launch::async, get_TFile, input_files.at(i + 1), cacheread, cache_dir);
            }
            else
            {
                std::cout << "End of file list ..." << std::endl;
                input_root_file_future = std::nullopt;
            }

            std::cout << yellow << "Analyzing: " << input_files.at(i) << def << std::endl;

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
                            std::cout << event_counter << " Events analyzed (" << pre_run_skipped << " + " << skipped
                                      << " skipped)";
                        }

                        std::cout << " - Processing rate: "
                                  << double(event_counter - event_counter_buffer) / (getCpuTime() - dTime_0)
                                  << " Events/s" << std::endl;
                        event_counter_buffer = event_counter;
                        dTime_0 = getCpuTime();

                        PrintProcessInfo();
                    }

                    // // PrintProcessInfo every 10_000 events
                    // if (event_counter % 10000 == 0)
                    // {
                    //     PrintProcessInfo();
                    // }
                }

                // stop if max number of events to be processed is reaced
                if (max_events != -1 && event_counter > max_events)
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
                        std::cerr << music_event.run << ":" << music_event.lumi_section << ":"
                                  << music_event.event_number << std::endl;
                    }
                    continue;
                }

                // fill trigger bits
                music_event.trigger_bits = [&]() {
                    TriggerBits trigger_bits;
                    if (year == "2016APV")
                    {
                        return trigger_bits.as_uint();
                    }
                    if (year == "2016")
                    {
                        return trigger_bits.as_uint();
                    }
                    if (year == "2017")
                    {
                        return trigger_bits.set(HLTPaths::SingleMuon, nano_reader.getVal<Bool_t>("HLT_Mu50"))
                            .set(HLTPaths::SingleElectron,
                                 nano_reader.getVal<Bool_t>("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL"))
                            .set(HLTPaths::DoubleMuon, nano_reader.getVal<Bool_t>("HLT_Mu50"))
                            .set(HLTPaths::DoubleElectron, nano_reader.getVal<Bool_t>("HLT_Mu50"))
                            .set(HLTPaths::Tau, false)
                            .set(HLTPaths::BJet, false)
                            .set(HLTPaths::MET, false)
                            .set(HLTPaths::Photon, false)
                            .as_uint();
                    }
                    if (year == "2018")
                    {
                        return trigger_bits.as_uint();
                    }
                    throw std::runtime_error(
                        "Year (" + year + ") not mathcing with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");

                    // dummy return, to avoid compilation warnings
                    return trigger_bits.as_uint();
                }();

                // skip event event if no trigger is fired
                if (music_event.trigger_bits != 0)
                {
                    continue;
                }

                // load nanoaod objects + minimal dummy object filtering
                // Muons
                auto muons = NanoObject::make_collection(
                    nano_reader.getVec<Float_t>("Muon_pt"), nano_reader.getVec<Float_t>("Muon_eta"),
                    nano_reader.getVec<Float_t>("Muon_phi"), NanoObject::MUON_MASS,
                    std::make_pair("dxy", nano_reader.getVec<Float_t>("Muon_dxy")),
                    std::make_pair("charge", nano_reader.getVec<Int_t>("Muon_charge")));

                // MET
                auto met = NanoObject::make_object(
                    nano_reader.getVal<Float_t>("MET_pt"), nano_reader.getVal<Float_t>("MET_phi"),
                    std::make_pair("significance", nano_reader.getVal<Float_t>("MET_significance")),
                    std::make_pair("MetUnclustEnUpDeltaX", nano_reader.getVal<Float_t>("MET_MetUnclustEnUpDeltaX")));

                // select objects
                auto good_muons = NanoObject::Filter(muons, [](const auto &muon) {
                    if (muon.pt() > 2.0)
                    {
                        return true;
                    }
                    return false;
                });

                met.set("good_met", [&]() {
                    if (met.pt() > 20.)
                    {
                        return true;
                    }
                    return false;
                }());

                // will hold the number of classes
                auto multiplicities =
                    EventContent::get_multiplicities(good_muons.size(), /* total number of muons */
                                                     0,                 /* total number of electrons */
                                                     0,                 /* total number of photons */
                                                     0,                 /* total number of taus */
                                                     0,                 /* total number of bjets */
                                                     0,                 /* total number of jets */
                                                     static_cast<int>(met.get<bool>("good_met")) /* MET */);

                unsigned long n_classes = ranges::distance(multiplicities.begin(), multiplicities.end());
                if (n_classes == 0)
                {
                    continue;
                }
                music_event.n_classes = n_classes;

                // loop over variations
                for (const auto &variation : Tools::index_range<Variation>(Variation::kTotalVariations))
                {
                    for (const auto &shift : Tools::index_range<Shift>(Shift::kTotalShifts))
                    {
                        // modify objects according to the given variation
                        const auto varied_objects = apply_variation(variation, shift, is_data, good_muons, met);

                        // will hold event content for all classes
                        auto content_buffer = EventContent{};

                        ////////////////////////////////////////////////////////////
                        ////////////////////////////////////////////////////////////
                        ///         loop over classes (multiplicities)           ///
                        ////////////////////////////////////////////////////////////
                        ////////////////////////////////////////////////////////////
                        for (auto multiplicity : multiplicities)
                        {
                            // DEBUG
                            // std::cout << i_muons << " - " << i_electrons << " - " << i_photons << " - " << i_taus
                            //           << " - " << i_bjets << " - " << i_jets << " - " << i_met << std::endl;
                            content_buffer.fill(multiplicity, varied_objects);
                        }

                        // save event content for this pair of variation + shift
                        music_event.fill(variation, shift, std::move(content_buffer));
                    }
                }

                // save data into output tree
                output_tree->Fill();
            }
            //////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////       [ END ]        //////////////////////////////////////
            //////////////////////////////////////   loop over events   //////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////

            // stop if max number of events to be processed is reaced
            if (max_events != -1 && event_counter > max_events)
            {
                break;
            }
        }

        // clear cache dir
        if (cacheread)
        {
            std::cout << yellow << "Cleaning NanoAOD cache ..." << def << std::endl;
            std::string cached_file_path = input_root_file->GetName();
            system(("rm -rf " + cached_file_path).c_str()); // FIX-ME!!
        }

        // load next file
        if (input_root_file_future)
        {
            std::cout << "Getting next file ..." << std::endl;
            input_root_file = input_root_file_future->get();
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
    std::cout << "Analyzed " << event_counter << " Events, skipped " << pre_run_skipped << " first events and "
              << skipped << " due to run/ls veto";
    std::cout << ", elapsed CPU time: " << dTime2 - dTime1 << " (" << double(event_counter) / (dTime2 - dTime1)
              << " evts per sec)" << std::endl;
    if (lost_files >= 0.5 * (lost_files + analyzed_files))
    {
        std::cout << "Error: Too many files lost!" << std::endl;
        throw std::runtime_error("Too many files lost.");
    }
    else if (lost_files > 0)
    {
        std::cout << "Warning: " << lost_files << " of " << (lost_files + analyzed_files)
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

    PrintProcessInfo();
    return 0;
}
