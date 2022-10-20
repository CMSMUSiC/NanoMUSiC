#include "NanoMUSiC.hh"
// sleep
#include <chrono>
#include <thread>

void PrintProcessInfo()
{
    auto info = ProcInfo_t();
    gSystem->GetProcInfo(&info);
    std::cout.precision(1);
    std::cout << std::fixed;
    std::cout << "--> Process info:" << std::endl;
    std::cout << "    -------------" << std::endl;
    std::cout << "    CPU time elapsed: " << info.fCpuUser << " s" << std::endl;
    std::cout << "    Sys time elapsed: " << info.fCpuSys << " s" << std::endl;
    std::cout << "    Resident memory:  " << info.fMemResident / 1024. << " MB" << std::endl;
    std::cout << "    Virtual memory:   " << info.fMemVirtual / 1024. << " MB" << std::endl;
}

int main(int argc, char *argv[])
{
    // check for env setup
    if (getenv("PXLANALYZER_BASE") == NULL)
    {
        throw std::runtime_error("PXLANALYZER_BASE not set!");
    }

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
    const auto run_on_data = run_config.get<bool>("run_on_data");
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

    // const auto useSYST = config.GetItem<bool>("General.useSYST");
    // const auto run_on_data = config.GetItem<bool>("General.RunOnData");

    // Get the run config file from main config file.
    const auto golden_json_file = [&]() {
        if (run_on_data)
        {
            return Tools::AbsolutePath(config.GetItem<std::string>("General.RunConfig"));
        }
        return std::string();
    }();

    if (run_on_data)
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

    if (run_on_data)
        system("mkdir -p Event-lists");

    // save other configs with output
    system(("cp " + x_section_file + " . ").c_str());

    // Init the run config
    std::cout << " " << std::endl;
    std::cout << green << "Initializing ..." << def << std::endl;
    std::cout << " " << std::endl;

    std::cout << def << "[Initializing] Run Lumi Filter ..." << def << std::endl;
    auto run_lumi_filter = RunLumiFilter(golden_json_file);

    // read cross-sections files
    std::cout << def << "[Initializing] X-Sections ..." << def << std::endl;
    const auto x_sections = TOMLConfig::make_toml_config(x_section_file);

    std::cout << def << "[Initializing] Output file ..." << def << std::endl;
    std::string process_hash = get_hash256(std::accumulate(input_files.begin(), input_files.end(), std::string("")));
    const auto output_file_name = "nano_music_" + process + "_" + year + "_" + process_hash + ".root";
    std::unique_ptr<TFile> output_file(TFile::Open(output_file_name.c_str(), "RECREATE"));

    std::cout << def << "[Initializing] Output tree ..." << def << std::endl;
    std::ifstream t(run_config_file);
    std::stringstream output_tree_title;
    output_tree_title << t.rdbuf();
    auto output_tree = std::make_unique<TTree>("nano_music", output_tree_title.str().c_str());

    std::cout << def << "[Initializing] Output tree branches ..." << def << std::endl;
    EventContent event_content;
    register_branches(event_content, output_tree);

    std::cout << def << "[Initializing] Cut flow histo ..." << def << std::endl;
    const auto n_cuts = 10;
    auto cutflow_histo = TH1F("cutflow_histo", output_tree_title.str().c_str(), n_cuts, -0.5, n_cuts + 0.5);
    cutflow_histo.Sumw2();

    // performance monitoring
    double dTime1 = getCpuTime(); // Start Time
    int e = 0;                    // Event counter
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
        system(("rm -rf " + cache_dir).c_str()); // <-- FIX me!!
        std::cout << cache_dir << std::endl;
    }

    std::cout << " " << std::endl;
    std::cout << green << "Starting Classification ..." << def << std::endl;
    std::cout << " " << std::endl;

    // loop over files
    for (auto const &file_iter : input_files)
    {
        const auto fileName = file_iter;

        std::cout << "Opening file " << fileName << std::endl;

        std::string cacheread_option = "";
        if (cacheread)
        {
            TFile::SetCacheFileDir(cache_dir);
            cacheread_option = "CACHEREAD";
        }

        std::unique_ptr<TFile> input_root_file(TFile::Open(fileName.c_str(), cacheread_option.c_str()));

        if (!input_root_file)
        {
            std::cout << "ERROR: could not open file" << std::endl;
            exit(1);
        }

        time_t rawtime;
        time(&rawtime);
        std::cout << "Opening time: " << ctime(&rawtime);

        analyzed_files++;
        int event_counter_per_file = 0;

        // get "Events" TTree from file
        std::unique_ptr<TTree> events_tree =
            std::unique_ptr<TTree>(dynamic_cast<TTree *>(input_root_file->Get("Events")));

        // get NanoAODReader
        auto nano_reader = NanoAODReader(*events_tree);

        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////       [ BEGIN ]      //////////////////////////////////////
        //////////////////////////////////////   loop over events   //////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        while (nano_reader.next())
        {

            event_counter_per_file++;

            // if set, skip first events
            if (number_of_events_to_skip > pre_run_skipped)
            {
                //
                pre_run_skipped++;
                continue;
            }

            // HOW TO READ DATA FROM NanoAODReader:
            // std::cout << nano_reader.getVal<UInt_t>("nMuon") << std::endl;
            // std::cout << nano_reader.getVec<Float_t>("Muon_pt") << std::endl;
            // std::cout << nano_reader.getVal<Bool_t>("HLT_Mu18_Mu9") << std::endl;

            // load per event defaults
            event_content = {}; // reset

            // reload event const's
            event_content.run = nano_reader.getVal<UInt_t>("run");
            event_content.lumi_section = nano_reader.getVal<UInt_t>("luminosityBlock");
            event_content.event_number = nano_reader.getVal<ULong64_t>("event");

            // initialize object counter
            auto object_counter = ObjectCounter{};

            // only if data
            // filter run and lumi section
            if (!run_lumi_filter(event_content.run, event_content.lumi_section, run_on_data))
            {
                ++skipped;
                if (debug > 1)
                {
                    std::cerr << "[INFO] (SkipEvents): " << std::endl;
                    std::cerr << "Skipping Run/lumi_section/Event: ";
                    std::cerr << event_content.run << ":" << event_content.lumi_section << ":"
                              << event_content.event_number << std::endl;
                }
                continue;
            }

            auto muons = NanoObject::make_collection(
                nano_reader.getVec<Float_t>("Muon_pt"), nano_reader.getVec<Float_t>("Muon_eta"),
                nano_reader.getVec<Float_t>("Muon_phi"), NanoObject::MUON_MASS,
                std::make_pair("dxy", nano_reader.getVec<Float_t>("Muon_dxy")),
                std::make_pair("charge", nano_reader.getVec<Int_t>("Muon_charge")));

            // if (debug >= 2)
            if (true)
            {
                std::cout << "-----------------------------------------" << std::endl;
                std::cout << "Muons: " << muons << std::endl;
                std::cout << "Muons Pt: " << NanoObject::Pt(muons) << std::endl;
                std::cout << "Muons Eta: " << NanoObject::Eta(muons) << std::endl;
                std::cout << "Muons Phi: " << NanoObject::Phi(muons) << std::endl;
                std::cout << "Muons Mass: " << NanoObject::Mass(muons) << std::endl;
                std::cout << "Muons E: " << NanoObject::E(muons) << std::endl;
                std::cout << "Muons dxy: " << NanoObject::GetFeature<Float_t>(muons, "dxy") << std::endl;
                std::cout << "Muons charge: " << NanoObject::GetFeature<Int_t>(muons, "charge") << std::endl;
                std::cout << "Muons indices: " << NanoObject::Indices(muons) << std::endl;
                std::cout << "Muons GetByIndex: " << NanoObject::GetByIndex(muons, 0) << std::endl;
            }

            // if (muons.size() > 1)
            // {
            //     std::cout << "Selected: " << muons[1] << std::endl;
            // }
            if (true)
            {
                auto muon_filter = NanoObject::Pt(muons) > 2;
                std::cout << "Filter: " << muon_filter << std::endl;

                auto filtered_muons = muons[muon_filter];
                // auto filtered_muons = muons[NanoObject::Pt(muons) > 25];
                std::cout << "Filtered: " << filtered_muons << std::endl;
                std::cout << "Filtered - features: " << NanoObject::Pt(filtered_muons) << std::endl;
                std::cout << "Filtered - features: " << NanoObject::GetFeature<Int_t>(filtered_muons, "charge")
                          << std::endl;
            }
            auto met = NanoObject::make_object(
                nano_reader.getVal<Float_t>("MET_pt"), nano_reader.getVal<Float_t>("MET_phi"),
                std::make_pair("significance", nano_reader.getVal<Float_t>("MET_significance")),
                std::make_pair("MetUnclustEnUpDeltaX", nano_reader.getVal<Float_t>("MET_MetUnclustEnUpDeltaX")));

            auto met_filter = met.pt() > 35.0;
            // std::cout << "++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
            // std::cout << "MET: " << met << std::endl;
            // std::cout << "MET - features: " << met.get<float>("significance") << std::endl;
            // std::cout << "MET - features: " << met.get<float>("MetUnclustEnUpDeltaX") << std::endl;
            // std::cout << "MET - index: " << met.index() << std::endl;

            met.set("abc", true);
            // std::cout << "MET - set features: " << met.get<bool>("abc") << std::endl;

            // try
            // {
            //     std::cout << "MET - index: " << met.get<float>("dadas") << std::endl;
            // }
            // catch (const std::runtime_error &e)
            // {
            //     std::cerr << e.what() << '\n';
            // }

            // std::cout << "Filter: " << met_filter << std::endl;
            // if (met_filter)
            // {
            //     std::cout << "Filtered: " << met << std::endl;
            //     std::cout << "Filtered - features: " << met.get<float>("significance") << std::endl;
            //     std::cout << "Filtered - features: " << met.get<float>("MetUnclustEnUpDeltaX") << std::endl;
            // }

            // write event class event_class_hash
            event_content.event_class_hash = get_event_class_hash(9, 5, 2, 1, 4, 5, 0);
            // get_event_class_hash(object_counter.n_muons, object_counter.n_electrons, object_counter.n_photons,
            // object_counter.n_taus, object_counter.n_bjets, object_counter.n_jets, object_counter.n_met);

            // std::cout << "event_content.event_class_hash: " << event_content.event_class_hash << std::endl;

            // fill data
            output_tree->Fill();

            e++;
            // std::cout << e << " <--- Event number " << std::endl; //<<-- FIX me?!?
            if (e < 10 || (e < 100 && e % 10 == 0) || (e < 1000 && e % 100 == 0) || (e < 10000 && e % 1000 == 0) ||
                (e >= 10000 && e % 10000 == 0))
            {
                if (pre_run_skipped == 0)
                {
                    std::cout << e << " Events analyzed (" << skipped << " skipped)" << std::endl;
                }
                else
                {
                    std::cout << e << " Events analyzed (" << pre_run_skipped << " + " << skipped << " skipped)"
                              << std::endl;
                }
            }
            // stop if max number of events to be processed is reaced
            if (max_events != -1 && e > max_events)
            {
                break;
            }

            // PrintProcessInfo every 10_000 events
            if (e % 10000 == 0)
            {
                PrintProcessInfo();
            }
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////       [ END ]        //////////////////////////////////////
        //////////////////////////////////////   loop over events   //////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////

        // stop if max number of events to be processed is reaced
        if (max_events != -1 && e > max_events)
        {
            break;
        }

        // clear cache dir
        if (cacheread)
        // if (false) // <-- FIX ME!
        {
            std::cout << yellow << "Cleaning NanoAOD cache directory..." << def << std::endl;
            system(("rm -rf " + cache_dir + "/*").c_str());
        }
    }

    double dTime2 = getCpuTime();
    std::cout << "Analyzed " << e << " Events, skipped " << pre_run_skipped << " first events and " << skipped
              << " due to run/ls veto";
    std::cout << ", elapsed CPU time: " << dTime2 - dTime1 << " (" << double(e) / (dTime2 - dTime1) << " evts per sec)"
              << std::endl;
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
    if ((e + skipped) == 0)
    {
        std::cout << "Error: No event analayzed!" << std::endl;
        throw std::runtime_error("No event analayzed!");
    }
    std::cout << "\n\n\n" << std::endl;

    // write tree to file
    std::cout << def << "[Finalizing] Output file and tree ..." << def << std::endl;
    output_file->cd();
    output_tree->Write();

    PrintProcessInfo();
    return 0;
}
