#include "music.hh"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace
{
   // Define error messages for program_options
   const size_t ERROR_IN_COMMAND_LINE = 1;
   const size_t SUCCESS = 0;
   const size_t ERROR_UNHANDLED_EXCEPTION = 2;

} // namespace

//----------------------------------------------------------------------

void PrintProcessInfo(ProcInfo_t &info);

bool do_break;

void KeyboardInterrupt_endJob(int signum)
{
   do_break = true;
}

int main(int argc, char *argv[])
{
   // check for env setup
   if (getenv("MUSIC_BASE") == NULL)
   {
      throw std::runtime_error("MUSIC_BASE not set!");
   }

   do_break = false;

   TDirectory::AddDirectory(kFALSE); // Force ROOT to give directories in our hand - Yes, we can
   TH1::AddDirectory(kFALSE);        // Force ROOT to give histograms in our hand - Yes, we can


   // Variables for argstream.
   // The values they are initialized with serve as default values.
   // The number of music options should be kept short, so it is easier to use
   // it within condor/grid.
   // All analysis based options/configurations belong into the config file!!!
   std::string inputs;
   std::string outputDirectory = "./AnalysisOutput";
   int numberOfEvents = -1;
   int numberOfSkipEvents = 0;
   std::string year = "";
   std::string process = "";
   std::string dataset = "";
   std::string FinalCutsFile;
   std::vector<std::string> input_files;
   toml::array _input_files;
   bool cacheread = true;
   std::string analysisName = "MUSiC Classification";
   std::string XSectionsFile = "$PXLANA/scales.txt";
   std::string RunHash = "dummyhash";
   std::string outfilename = ""; // constructed from process, if omitted
   bool batch_mode = false;

   // Debug levels are:
   //    - 0: Display only ERRORS/EXCEPTIONS
   //    - 1: Display also WARNINGS
   //    - 2: Display also INFO
   //    - 3: Display also DEBUG
   //    - 4: Display also verbose DEBUG
   //
   // By default, debug = 1, means only WARNINGS will be displayed.
   // (ERRORS should always be handled with help of exceptions!)
   //
   // The debug information is meant to be written to stderr.
   // Additionally there is "normal" program output that goes to stdout.
   //
   // The output formatting should be the following:
   // "[<LEVEL>] (<invoking_class>): <message>"
   // e.g.:
   // "[DEBUG] (ParticleMatcher): Something's fishy!"
   // Please keep to that.
   int debug = 1;

   po::options_description genericOptions("");
   genericOptions.add_options()("help,h", "produce help message");
   genericOptions.add_options()("config,c", po::value<std::string>(&inputs)->required(), "The main config file (TOML format).");
   genericOptions.add_options()("batch,b", po::value<bool>(&batch_mode), "Set to 1, if running in batch mode.");

   // add positional arguments
   po::positional_options_description pos;
   pos.add("inputs", 1);

   // Add all option groups
   po::options_description allOptions("Available options");
   allOptions.add(genericOptions);

   // parse command line options
   po::variables_map vm;

   try
   {
      po::store(po::command_line_parser(argc, argv).options(allOptions).positional(pos).run(), vm);
      if (vm.count("help"))
      {
         std::cout << allOptions << std::endl;
         return 0;
      }
      po::notify(vm);
   }
   catch (po::error &e)
   {
      std::cout << " " << std::endl;
      std::cout << " " << std::endl;
      std::cout << "MUSiC - Model Unspecific Search in CMS" << std::endl;
      std::cout << emojicpp::emojize("      :signal_strength: Run2 - Ultra Legacy :signal_strength:") << std::endl;
      std::cout << " " << std::endl;

      std::cerr << "ERROR: " << e.what() << std::endl;
      std::cout << allOptions << std::endl;
      return ERROR_IN_COMMAND_LINE;
   }

   // set colors
   // define colors
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
      std::cout << acqua << "███    ███ ██    ██ ███████ ██  ██████ " << def << std::endl;
      std::cout << acqua << "████  ████ ██    ██ ██      ██ ██      " << def << std::endl;
      std::cout << acqua << "██ ████ ██ ██    ██ ███████ ██ ██      " << def << std::endl;
      std::cout << acqua << "██  ██  ██ ██    ██      ██ ██ ██      " << def << std::endl;
      std::cout << acqua << "██      ██  ██████  ███████ ██  ██████ " << def << std::endl;
   }

   std::cout << " " << std::endl;
   std::cout << " " << std::endl;
   std::cout << acqua << "MUSiC - Model Unspecific Search in CMS" << def << std::endl;
   std::cout << acqua << emojicpp::emojize("      :signal_strength: Run2 - Ultra Legacy :signal_strength:") << def << std::endl;
   std::cout << " " << std::endl;

   std::cout << " " << std::endl;
   std::cout << yellow << "Checking configuration ..." << def << std::endl;
   std::cout << " " << std::endl;

   // read parameters from TOML file
   try
   {
      auto config_file = toml::parse_file(inputs);
      auto toml_config = config_file["music"];
      outputDirectory = *(toml_config["output"].value<std::string>());
      FinalCutsFile = *(toml_config["config"].value<std::string>());
      FinalCutsFile = Tools::AbsolutePath(FinalCutsFile);
      process = *(toml_config["process"].value<std::string>());
      dataset = *(toml_config["dataset"].value<std::string>());
      numberOfEvents = *(toml_config["number_of_events"].value<int>());
      numberOfSkipEvents = *(toml_config["number_of_skip_events"].value<int>());
      debug = *(toml_config["debug"].value<int>());
      XSectionsFile = *(toml_config["xsections"].value<std::string>());
      XSectionsFile = Tools::AbsolutePath(XSectionsFile);
      RunHash = *(toml_config["hash"].value<std::string>());
      year = *(toml_config["year"].value<std::string>());
      cacheread = *(toml_config["cacheread"].value<bool>());
      _input_files = *(toml_config["input"].as_array());

      // fill input_files from toml::node
      for (const auto &f : _input_files)
      {
         input_files.push_back(*(f.value<std::string>()));
      }
      // std::cout << toml_config << std::endl;
   }
   catch (const toml::parse_error &err)
   {
      std::cerr << red << "ERROR: TOML Parse failed.\n"
                << err << def << "\n";
      return -1;
   }

   // check year
   if (year != "2016APV" && year != "2016" && year != "2017" && year != "2018")
   {
      std::cerr << red << "ERROR: year should be 2016APV, 2016, 2017 or 2018" << def << std::endl;
      return 1;
   }

   if (not fs::exists(FinalCutsFile))
   {
      throw Tools::file_not_found(FinalCutsFile, "Config file");
   }
   else
      std::cout << "INFO: Using Config file: " << FinalCutsFile << std::endl;

   Tools::MConfig config(FinalCutsFile);
   config.setYear(year);

   // Get the run config file from config file.
   std::string RunConfigFile;

   bool const useSYST = config.GetItem<bool>("General.useSYST");
   bool runOnData = config.GetItem<bool>("General.RunOnData");
   if (runOnData)
   {
      RunConfigFile = Tools::AbsolutePath(config.GetItem<std::string>("General.RunConfig"));
      if (not fs::exists(RunConfigFile))
      {
         std::stringstream error;
         error << "RunConfigFile '" << RunConfigFile << "' ";
         error << "in config file: '" << FinalCutsFile << "' not found!";
         throw Tools::config_error(error.str());
      }
   }
   if (!RunConfigFile.empty())
   {
      std::cout << "INFO: Using Run config file: " << RunConfigFile << std::endl;
   }

   std::cout << " " << std::endl;
   std::cout << yellow << "Preparing output buffer ..." << def << std::endl;
   std::cout << " " << std::endl;

   const std::string startDir = getcwd(NULL, 0);

   signal(SIGINT, KeyboardInterrupt_endJob);

   // (Re)create outputDirectory dir and cd into it.
   system(("rm -rf " + outputDirectory).c_str());
   system(("mkdir -p " + outputDirectory).c_str());
   system(("cd " + outputDirectory).c_str());
   chdir(outputDirectory.c_str());
   system(("cp " + FinalCutsFile + " . ").c_str());

   // dump config to file
   config.DumpToFile("config_full.txt");

   if (!RunConfigFile.empty())
      system(("cp " + RunConfigFile + " . ").c_str());

   if (runOnData)
      system("mkdir -p Event-lists");

   // Init the run config
   std::cout << " " << std::endl;
   std::cout << green << "Initializing ..." << def << std::endl;
   std::cout << " " << std::endl;

   lumi::RunLumiRanges runcfg(RunConfigFile);

   SkipEvents skipEvents(config);

   pxl::Core::initialize();

   pxl::Hep::initialize();

   // initialize the EventSelector
   EventSelector selector(config);

   // Initialize the EventAdaptor.
   EventAdaptor adaptor(config, debug);

   // Initialize the ParticleMatcher.
   ParticleMatcher matcher(config, debug);

   // Initialize the Systematics.
   Systematics syst_shifter(config, debug);

   // // pxl::AnalysisFork fork;
   // event_class_factory.setName(analysisName);
   

   const Tools::MConfig XSections(XSectionsFile);

   // save other configs with output
   system(("cp " + XSectionsFile + " . ").c_str());

   // EventClassFactory *event_class_factory = new EventClassFactory(config,XSections,selector,syst_shifter,outfilename,RunHash);
   auto event_class_factory = EventClassFactory(config,XSections,selector,syst_shifter,outfilename,RunHash);

   // // Get fork from AnalysisComposer
   // pxl::AnalysisFork fork = thisAnalysis.addForkObjects(config,
   //                                                      outputDirectory,
   //                                                      selector,
   //                                                      syst_shifter,
   //                                                      debug);

   // begin analysis
   event_class_factory.beginJob();
   event_class_factory.beginRun();

   // performance monitoring
   double dTime1 = pxl::getCpuTime(); // Start Time
   int e = 0;                         // Event counter
   unsigned int skipped = 0;          // number of events skipped from run/LS config
   int pre_run_skipped = 0;           // number of events skipped due to skipped option

   if (config.GetItem<bool>("Pileup.UseSampleName", false))
   {
      ReWeighter::adaptConfig(config, input_files[0]);
   }
   ReWeighter reweighter = ReWeighter(config, 0);
   ReWeighter reweighterup = ReWeighter(config, 1);
   ReWeighter reweighterdown = ReWeighter(config, -1);

   unsigned int analyzed_files = 0;
   unsigned int lost_files = 0;

   // initialize process info object
   ProcInfo_t info;

   // temp cache dir
   string process_hash = std::to_string(std::hash<std::string>{}(std::accumulate(input_files.begin(), input_files.end(), std::string(""))));
   std::string cache_dir = "/tmp/music/proc_" + process_hash;
   if (cacheread)
   {
      std::cout << " " << std::endl;
      std::cout << " " << std::endl;
      std::cout << " " << std::endl;
      std::cout << yellow << "Preparing cache directory for NanoAOD files: " << def << std::endl;
      // system(("rm -rf " + cache_dir).c_str()); // <-- FIX me!!
      std::cout << cache_dir << std::endl;
   }

   std::cout << " " << std::endl;
   std::cout << green << "Starting Classfication ..." << def << std::endl;
   std::cout << " " << std::endl;

   // loop over files
   for (auto const &file_iter : input_files)
   {
      std::string const fileName = file_iter;

      std::cout << "Opening file " << fileName << std::endl;

      std::string cacheread_option = "";
      if (cacheread)
      {
         TFile::SetCacheFileDir(cache_dir);
         cacheread_option = "CACHEREAD";
      }

      std::unique_ptr<TFile> inFile(TFile::Open(fileName.c_str(), cacheread_option.c_str()));

      if (!inFile)
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
      std::unique_ptr<TTree> events_tree = std::unique_ptr<TTree>(dynamic_cast<TTree *>(inFile->Get("Events")));

      // get NanoAODReader
      NanoAODReader nano_reader(*events_tree);

      // loop over events
      while (nano_reader.next())
      {
         // pxl::Event *event_ptr = NULL;
         std::unique_ptr<pxl::Event> event_ptr = buildPxlEvent(e, nano_reader, year, process, dataset, runOnData, debug);

         event_counter_per_file++;
         if (!event_ptr)
            continue;

         // if set, skip first events
         if (numberOfSkipEvents > pre_run_skipped)
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
         lumi::ID run = event_ptr->getUserRecord("Run");
         lumi::ID LS = event_ptr->getUserRecord("LumiSection");
         lumi::ID eventNum = event_ptr->getUserRecord("EventNum");
         if (!runcfg.check(run, LS))
         {
            ++skipped;
            if (debug > 1)
            {
               std::cerr << "[INFO] (SkipEvents): " << std::endl;
               std::cerr << "Skipping Run/LS/Event: ";
               std::cerr << run << ":" << LS << ":" << eventNum << std::endl;
            }
            //
            continue;
         }

         if (runOnData && skipEvents.skip(run, LS, eventNum))
         {
            ++skipped;

            if (debug > 1)
            {
               std::cerr << "[INFO] (SkipEvents): " << std::endl;
               std::cerr << "Skipping Run/LS/Event (file Veto): ";
               std::cerr << run << ":" << LS << ":" << eventNum << std::endl;
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
         std::cout << "111" << std::endl;
         if (runOnData)
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
            RecEvtView->setUserRecord("EventSeed",
                                      event_ptr->getUserRecord("Run").toInt32() +
                                          event_ptr->getUserRecord("LumiSection").toInt32() +
                                          event_ptr->getUserRecord("EventNum").toInt32());

            // Don't do this on data, haha! And also not for special Ana hoho
            reweighter.ReWeightEvent(event_ptr.get());
            reweighterup.ReWeightEvent(event_ptr.get());
            reweighterdown.ReWeightEvent(event_ptr.get());
            pxl::EventView *GenEvtView = event_ptr->getObjectOwner().findObject<pxl::EventView>("Gen");

            // Sometimes events have missing PDF information (mainly POWHEG).
            // This is checked in the skimmer and if PDF weights are missing, the event is tagged
            if (config.GetItem<bool>("General.usePDF") and config.GetItem<bool>("PDF.SkipIncomplete") and GenEvtView->hasUserRecord("Incomplete_PDF_weights") and GenEvtView->getUserRecord("Incomplete_PDF_weights"))
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
                  // use the    system(("rm -rf " + outputDirectory).c_str());config files to activate systematics for some objects
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
               if (runOnData)
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
         event_class_factory.finishEvent(event_ptr.get());

         e++;
         // std::cout << e << " <--- Event number " << std::endl; //<<-- FIX me?!?
         if (e < 10 || (e < 100 && e % 10 == 0) ||
             (e < 1000 && e % 100 == 0) ||
             (e < 10000 && e % 1000 == 0) ||
             (e >= 10000 && e % 10000 == 0))
         {
            if (pre_run_skipped == 0)
            {
               std::cout << e << " Events analyzed (" << skipped << " skipped)" << std::endl;
            }
            else
            {
               std::cout << e << " Events analyzed (" << pre_run_skipped << " + " << skipped << " skipped)" << std::endl;
            }
         }

         // end of file
         if (e % 10000 == 0)
         {
            PrintProcessInfo(info);
         }
      }

      if (do_break || (numberOfEvents != -1 && e > numberOfEvents))
      {
         break;
      }

      // clear cache dir
      // if (cacheread)
      if (false) // <-- FIX ME!
      {
         std::cout << yellow << "Cleaning NanoAOD cache directory..." << def << std::endl;
         system(("rm -rf " + cache_dir + "/*").c_str());
      }
   }

   double dTime2 = pxl::getCpuTime();
   std::cout << "Analyzed " << e << " Events, skipped " << pre_run_skipped << " first events and " << skipped << " due to run/ls veto";
   std::cout << ", elapsed CPU time: " << dTime2 - dTime1 << " (" << double(e) / (dTime2 - dTime1) << " evts per sec)" << std::endl;
   if (lost_files >= 0.5 * (lost_files + analyzed_files))
   {
      std::cout << "Error: Too many files lost!" << std::endl;
      throw std::runtime_error("Too many files lost.");
   }
   else if (lost_files > 0)
   {
      std::cout << "Warning: " << lost_files << " of " << (lost_files + analyzed_files) << " files lost due to timeouts or read errors." << std::endl;
   }
   if ((e + skipped) == 0)
   {
      std::cout << "Error: No event analayzed!" << std::endl;
      throw std::runtime_error("No event analayzed!");
   }
   std::cout << "\n\n\n"
             << std::endl;

   // zero is dummy
   // it is here just to match the method definition
   event_class_factory.endRun(0);
   event_class_factory.endJob(0);


   // in the old analysis composer, this was empty.
   // Should stay?
   // thisAnalysis.endAnalysis(); 

   PrintProcessInfo(info);
   return 0;
}

void PrintProcessInfo(ProcInfo_t &info)
{
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
