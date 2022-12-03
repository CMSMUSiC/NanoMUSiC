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
    // const auto debug = run_config.get<int>("debug");
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
    auto pu_weight = Corrector(CorrectionTypes::PU, year, is_data);

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
    auto cutflow_histos = make_cutflow_histos(output_file.get(), output_tree_title);

    std::cout << def << "[Initializing] classes' set ..." << def << std::endl;
    std::array<std::set<unsigned long>, total_variations_and_shifts> classes;

    std::cout << def << "[Initializing] Task runners (thread pool) ..." << def << std::endl;
    BS::thread_pool task_runners_pool(n_threads);

    std::cout << def << "[Initializing] Rochester Muon Momentum Corrections ..." << def << std::endl;
    auto rochester_corrections = Corrector(CorrectionTypes::MuonLowPt, year, is_data);

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

            event_counter_per_file++;

            auto process_result = process_event(nano_reader, is_data, year, run_lumi_filter, pu_weight, task_runners_pool);

            for (unsigned int idx = 0; idx < process_result.size(); idx++)
            {
                auto res = process_result.at(idx);
                if (res)
                {
                    music_event.run = res.run;
                    music_event.lumi_section = res.lumi_section;
                    music_event.event_number = res.event_number;
                    music_event.trigger_bits = res.trigger_bits.as_ulong();
                    music_event.fill(std::move(res.event_content), idx);

                    cutflow_histos.at(idx) = cutflow_histos.at(idx) + res.cutflow_histo;

                    classes.at(idx).merge(res.classes);

                    // save data into output tree
                    output_tree->Fill();
                }
            }
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
    for (auto histo : cutflow_histos)
    {
        histo.Write();
    }

    // convert the std::list of classes that were touched to a comma separated
    // string and write it to the the outputfile.classes
    for (unsigned int i = 0; i < classes.size(); i++)
    {
        save_class_storage(classes.at(i), output_file_name, i);
    }

    PrintProcessInfo();
    return 0;
}
