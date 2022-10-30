#include "music.hpp"
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
    std::cout << yellow << "Checking configuration [" << run_config_file << "] ..." << def << std::endl;
    std::cout << " " << std::endl;

    // read parameters from TOML file
    const auto run_config = [&]() {
        try
        {
            auto run_config = toml::parse_file(run_config_file);
            return run_config;
        }
        catch (const toml::parse_error &err)
        {
            std::cerr << red << "ERROR: Config file [" << run_config_file << "] parsing failed.\n"
                      << err << def << "\n";
            exit(-1);
        }
    }();

    const auto config_toml = run_config["music"];
    const auto output_directory = *(config_toml["output"].value<std::string>());
    const auto analysis_config_file = Tools::AbsolutePath(*(config_toml["config"].value<std::string>()));
    const auto process = *(config_toml["process"].value<std::string>());
    const auto dataset = *(config_toml["dataset"].value<std::string>());
    const auto max_events = *(config_toml["max_events"].value<int>());
    const auto number_of_events_to_skip = *(config_toml["number_of_events_to_skip"].value<int>());
    const auto debug = *(config_toml["debug"].value<int>());
    const auto x_section_file = Tools::AbsolutePath(*(config_toml["x_section_file"].value<std::string>()));
    const auto run_hash = *(config_toml["hash"].value<std::string>());
    const auto year = *(config_toml["year"].value<std::string>());
    const auto output_file = *(config_toml["output_file"].value<std::string>());
    const auto cacheread = *(config_toml["cacheread"].value<bool>());

    const auto input_files = [&]() {
        auto f = std::vector<std::string>();
        for (const auto &i : *(config_toml["input_files"].as_array()))
        {
            f.push_back(*(i.value<std::string>()));
        }
        return f;
    }();

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
    {
        std::cout << "INFO: Using Config file: " << analysis_config_file << std::endl;
    }

    auto config = Tools::MConfig(analysis_config_file);
    config.setYear(year);

    const auto useSYST = config.GetItem<bool>("General.useSYST");
    const auto is_data = config.GetItem<bool>("General.RunOnData");

    // Get the run config file from main config file.
    const auto golden_json_file = [&]() {
        if (is_data)
        {
            return Tools::AbsolutePath(config.GetItem<std::string>("General.RunConfig"));
        }
        return std::string();
    }();

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

    std::cout << def << "[Initializing] PXL Core ..." << def << std::endl;
    pxl::Core::initialize();
    pxl::Hep::initialize();

    std::cout << def << "[Initializing] Run Lumi Filter ..." << def << std::endl;
    auto run_lumi_filter = RunLumiFilter(golden_json_file);

    std::cout << def << "[Initializing] Skip events ..." << def << std::endl;
    auto skipEvents = SkipEvents(config);

    // initialize the EventSelector
    std::cout << def << "[Initializing] Event selector ..." << def << std::endl;
    auto selector = EventSelector(config);

    // Initialize the EventAdaptor.
    std::cout << def << "[Initializing] Event adaptor ..." << def << std::endl;
    auto adaptor = EventAdaptor(config, debug);

    // Initialize the ParticleMatcher.
    std::cout << def << "[Initializing] Particle matcher ..." << def << std::endl;
    const auto matcher = ParticleMatcher(config, debug);

    // Initialize the Systematics.
    std::cout << def << "[Initializing] Systematics ..." << def << std::endl;
    auto syst_shifter = Systematics(config, debug);

    // read cross-sections files
    std::cout << def << "[Initializing] X-Sections ..." << def << std::endl;
    const auto x_sections = TOMLConfig::make_toml_config(x_section_file);

    std::cout << def << "[Initializing] Event Class Factory ..." << def << std::endl;
    auto event_class_factory = EventClassFactory(config, x_sections, selector, syst_shifter, output_file, run_hash);

    // performance monitoring
    double dTime1 = pxl::getCpuTime(); // Start Time
    int e = 0;                         // Event counter
    unsigned int skipped = 0;          // number of events skipped from run/lumi_section config
    int pre_run_skipped = 0;           // number of events skipped due to skipped option

    std::cout << def << "[Initializing] ReWeighter ..." << def << std::endl;

    if (config.GetItem<bool>("Pileup.UseSampleName", false))
    {
        ReWeighter::adaptConfig(config, input_files[0]);
    }
    ReWeighter reweighter = ReWeighter(config, 0);
    ReWeighter reweighterup = ReWeighter(config, 1);
    ReWeighter reweighterdown = ReWeighter(config, -1);

    unsigned int analyzed_files = 0;
    unsigned int lost_files = 0;

    // temp cache dir
    string process_hash = std::to_string(
        std::hash<std::string>{}(std::accumulate(input_files.begin(), input_files.end(), std::string(""))));
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
            std::cout << "ERROR: could not open data file" << std::endl;
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

        // loop over events
        while (nano_reader.next())
        {
            std::unique_ptr<pxl::Event> event_ptr =
                make_pxlevent(e, nano_reader, year, process, dataset, is_data, debug);

            event_counter_per_file++;
            if (!event_ptr)
                continue;

            // if set, skip first events
            if (number_of_events_to_skip > pre_run_skipped)
            {
                //
                pre_run_skipped++;
                continue;
            }

            if (event_ptr->getObjectOwner().findObject<pxl::EventView>("Rec") == 0)
            {
                event_class_factory.analyseEvent(event_ptr.get());
                //
                e++;
                continue;
            }

            // check if shall analyze this event
            const unsigned long run = event_ptr->getUserRecord("Run");
            const unsigned long lumi_section = event_ptr->getUserRecord("LumiSection");
            const unsigned long eventNum = event_ptr->getUserRecord("EventNum");
            if (!run_lumi_filter(run, lumi_section))
            {
                ++skipped;
                if (debug > 1)
                {
                    std::cerr << "[INFO] (SkipEvents): " << std::endl;
                    std::cerr << "Skipping Run/lumi_section/Event: ";
                    std::cerr << run << ":" << lumi_section << ":" << eventNum << std::endl;
                }

                continue;
            }

            if (is_data && skipEvents.skip(run, lumi_section, eventNum))
            {
                ++skipped;

                if (debug > 1)
                {
                    std::cerr << "[INFO] (SkipEvents): " << std::endl;
                    std::cerr << "Skipping Run/lumi_section/Event (file Veto): ";
                    std::cerr << run << ":" << lumi_section << ":" << eventNum << std::endl;
                }
                //
                continue;
            }

            pxl::EventView *RecEvtView = event_ptr->getObjectOwner().findObject<pxl::EventView>("Rec");
            pxl::EventView *TrigEvtView = event_ptr->getObjectOwner().findObject<pxl::EventView>("Trig");
            pxl::EventView *FilterView = event_ptr->getObjectOwner().findObject<pxl::EventView>("Filter");

            // Start Adaptors
            bool const muoCocktailUse = config.GetItem<bool>("Muon.UseCocktail");
            bool const eleHEEPUse = config.GetItem<bool>("Ele.UseHEEP");
            bool const fatjetPUPPIUse = config.GetItem<bool>("FatJet.UsePUPPI");
            bool const jetResCorrUse = config.GetItem<bool>("Jet.Resolutions.Corr.use");
            // Event Adaptor only needs to be initalized per event if needed.
            bool initAdaptor = eleHEEPUse or muoCocktailUse or jetResCorrUse or fatjetPUPPIUse;

            // intialize event for adaption
            if (initAdaptor)
            {
                adaptor.initEvent(RecEvtView);
            }

            if (muoCocktailUse)
            {
                // Switch to cocktail muons (use the four momentum from
                // TeV-optimised reconstructors.)
                adaptor.applyCocktailMuons();
            }

            // it is not possible to adpt electron for (eta-->SCEta and phi-->SCphi), using NanoAOD information
            // if (eleHEEPUse)
            // {
            //    // Switch to HEEP ele kinematic for eles above switch pt
            //    // adaptor.applyHEEPElectrons();
            // }

            if (fatjetPUPPIUse)
            {
                // Switch to puppi kinematics for fat jets
                adaptor.applyPUPPIFatJets();
            }

            if (is_data)
            {
                // Only needed for 2016 data //LOR COMM IT OUT
                // adaptor.adaptDoubleEleTrigger( run, TrigEvtView );
                // for data we just need to run the selection
                selector.performSelection(RecEvtView, 0, TrigEvtView, FilterView);
                selector.removeOverlaps(RecEvtView);
                selector.performOfflineTriggerSelection(RecEvtView, TrigEvtView);
            }
            else // run on MC
            {
                RecEvtView->setUserRecord("EventSeed", event_ptr->getUserRecord("Run").toInt32() +
                                                           event_ptr->getUserRecord("LumiSection").toInt32() +
                                                           event_ptr->getUserRecord("EventNum").toInt32());

                // Don't do this on data, haha! And also not for special Ana hoho
                reweighter.ReWeightEvent(event_ptr.get());
                reweighterup.ReWeightEvent(event_ptr.get());
                reweighterdown.ReWeightEvent(event_ptr.get());
                pxl::EventView *GenEvtView = event_ptr->getObjectOwner().findObject<pxl::EventView>("Gen");

                // Sometimes events have missing PDF information (mainly POWHEG).
                // This is checked in the skimmer and if PDF weights are missing, the event is tagged
                if (config.GetItem<bool>("General.usePDF") and config.GetItem<bool>("PDF.SkipIncomplete") and
                    GenEvtView->hasUserRecord("Incomplete_PDF_weights") and
                    GenEvtView->getUserRecord("Incomplete_PDF_weights"))
                {
                    skipped++;
                    //
                    continue;
                }

                // (Pre)Matching must be done before selection, because the matches
                // will be used to adapt the event. We only need the jets matched,
                // so no "custom" matching.
                // We use a different link name here, so the "normal" matching after
                // all selection cuts can go ahead without changes.
                // This link name is only used for the matching before cuts and to
                // adapt the events (jet/met smearing).
                std::string const linkName = "pre-priv-gen-rec";
                matcher.matchObjects(GenEvtView, RecEvtView, linkName, false);

                if (jetResCorrUse)
                {
                    // Change event properties according to official recommendations.
                    // Don't do this on data!
                    adaptor.applyJETMETSmearing(GenEvtView, RecEvtView, linkName);
                }

                try
                {
                    if (useSYST)
                    {
                        // perform systematic pre. selection on all selected event views
                        //  with loosened kinematic cuts
                        selector.performSelection(RecEvtView, GenEvtView, TrigEvtView, FilterView, true);
                        // use the    system(("rm -rf " + output_directory).c_str());config files to activate
                        // systematics for some objects
                        syst_shifter.init(event_ptr.get());
                        // create new event views with systematic shifts
                        syst_shifter.createShiftedViews();
                        // Check if particles fullfill standard kinematic cuts after shifting
                        for (auto &systInfo : syst_shifter.m_activeSystematics)
                        {
                            for (auto &evtView : systInfo->eventViewPointers)
                            {
                                selector.performKinematicsSelection(evtView, false);
                                selector.removeOverlaps(evtView);
                                selector.performOfflineTriggerSelection(evtView, TrigEvtView);
                            }
                        }
                        // we need to check if normal rec view also survives
                        // cuts with standard kinematics cuts
                        selector.performKinematicsSelection(RecEvtView, false);
                        selector.removeOverlaps(RecEvtView);
                        // Now finally check triggering ( if active ) after view is fully selected
                        selector.performOfflineTriggerSelection(RecEvtView, TrigEvtView);
                    }
                    else
                    {
                        selector.performSelection(RecEvtView, GenEvtView, TrigEvtView, FilterView);
                        selector.removeOverlaps(RecEvtView);
                        selector.performOfflineTriggerSelection(RecEvtView, TrigEvtView);
                    }

                    // Sometimes a particle is unsorted in an event, where it should be
                    // sorted by pt. This seems to be a PXL problem.
                    // Best idea until now is to skip the whole event.
                    // Do this only for MC at the moment. If this ever happens for data,
                    // you should investigate!
                }
                catch (Tools::unsorted_error &exc)
                {
                    std::cerr << "[WARNING] (main): ";
                    std::cerr << "Found unsorted particle in event no. " << e << ". ";
                    std::cerr << "Skipping this event!" << std::endl;
                    //
                    if (is_data)
                        exit(1);
                    else
                        continue;
                }
            }
            // set user record for file name and event number in this file for further use in analysis
            if (fileName.rfind('/') != std::string::npos)
            {
                event_ptr->setUserRecord("Filename", fileName.substr(fileName.rfind('/') + 1));
            }
            else
            {
                event_ptr->setUserRecord("Filename", fileName);
            }
            event_ptr->setUserRecord("EventNumPxlio", event_counter_per_file);

            // run the fork ..
            event_class_factory.analyseEvent(event_ptr.get());

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

    double dTime2 = pxl::getCpuTime();
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

    // 0 (zero) is dummy
    // it is here just to match the method definition
    event_class_factory.endJob(0);

    PrintProcessInfo();
    return 0;
}
