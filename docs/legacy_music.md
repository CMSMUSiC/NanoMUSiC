# legacy MUSiC code

```C++
#include <time.h>
#include <string>

#include "Pxl/Pxl/interface/pxl/hep.hpp"
#include "Pxl/Pxl/interface/pxl/core.hpp"
#include <iostream>
#include <csignal>
#include <iomanip>

#include "Tools/Tools.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wattributes"
#include <boost/filesystem/path.hpp>
#pragma GCC diagnostic pop
#include "boost/program_options.hpp"

#include "Main/EventAdaptor.hpp"
#include "Main/JetTypeWriter.hpp"
#include "Main/EventSelector.hpp"
#include "Main/ParticleMatcher.hpp"
#include "Main/ReWeighter.hpp"
#include "Main/RunLumiRanges.hpp"
#include "Main/SkipEvents.hpp"


// Include user defined Analysis or use Validator as default
// Implement your own analysis composer and use export to define the
// header file as environment variable MYPXLANA.
#define Q(x) #x
#define QUOTE(x) Q(x)

#include QUOTE(MYPXLANA)

#include "Main/Systematics.hpp"

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
   if (getenv("PXLANALYZER_BASE") == NULL)
      throw std::runtime_error("PXLANALYZER_BASE not set!");

   std::cout << "Running Analysis '" QUOTE(MYPXLANA) "'..." << std::endl;

   do_break = false;

   TDirectory::AddDirectory(kFALSE); // Force ROOT to give directories in our hand - Yes, we can
   TH1::AddDirectory(kFALSE);        // Force ROOT to give histograms in our hand - Yes, we can
   AnalysisComposer thisAnalysis;

   // Variables for argstream.
   // The values they are initialized with serve as default values.
   // The number of music options should be kept short, so it is easier to use
   // it within condor/grid.
   // All analysis based options/configurations belong into the config file!!!
   //
   std::string outputDirectory = "./AnalysisOutput";
   int numberOfEvents = -1;
   int numberOfSkipEvents = 0;
   std::string year = "9999";
   std::string FinalCutsFile;
   std::vector<std::string> input_files;

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
   std::vector<std::string> arguments;

   po::options_description genericOptions("Generic options");
   genericOptions.add_options()("help", "produce help message")("Output,o", po::value<std::string>(&outputDirectory), "Output directory")("CONFIG", po::value<std::string>(&FinalCutsFile)->required(), "The main config file")("PXLIO_FILE(S)", po::value<std::vector<std::string>>(&input_files)->required(), "A list of pxlio files to run on")("Num,N", po::value<int>(&numberOfEvents), "Number of events to analyze.")("skip", po::value<int>(&numberOfSkipEvents), "Number of events to skip.")("debug", po::value<int>(&debug), "Set the debug level.\n0 = ERRORS,\n1 = WARNINGS,\n2 = INFO, 3 = DEBUG,\n4 = EVEN MORE DEBUG");

   // add positional arguments
   po::positional_options_description pos;
   pos.add("CONFIG", 1);
   pos.add("PXLIO_FILE(S)", -1);

   // get user defined command line arguments
   po::options_description analysisOptions = thisAnalysis.getCmdArguments();

   // Add all option groups
   po::options_description allOptions("Available options");
   allOptions.add(genericOptions).add(analysisOptions);

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
      std::cerr << "ERROR: " << e.what() << std::endl
                << std::endl;
      std::cerr << allOptions << std::endl;
      return ERROR_IN_COMMAND_LINE;
   }

   // if( not fs::exists( FinalCutsFile ) ) {
   //    std::cout<<"Yan DEBUG : : in FinalCutsFile flag ERROR block"<<std::endl; //Yannik change
   //    throw Tools::file_not_found( FinalCutsFile, "Config file" );
   // }
   // else std::cout << "INFO: Using Config file: " << FinalCutsFile << std::endl;

   Tools::MConfig config(FinalCutsFile);

   // Get the run config file from config file.
   std::string RunConfigFile;

   bool const muoCocktailUse = config.GetItem<bool>("Muon.UseCocktail");
   bool const eleHEEPUse = config.GetItem<bool>("Ele.UseHEEP");
   bool const fatjetPUPPIUse = config.GetItem<bool>("FatJet.UsePUPPI");
   bool const jetResCorrUse = config.GetItem<bool>("Jet.Resolutions.Corr.use");
   // Event Adaptor only needs to be initalized per event if needed.
   bool initAdaptor = eleHEEPUse or muoCocktailUse or jetResCorrUse or fatjetPUPPIUse;

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
      std::cout << "INFO: Using Run config file: " << RunConfigFile << std::endl;

   const std::string startDir = getcwd(NULL, 0);

   signal(SIGINT, KeyboardInterrupt_endJob);

   // (Re)create outputDirectory dir and cd into it.
   //
   system(("rm -rf " + outputDirectory).c_str());
   system(("mkdir -p " + outputDirectory).c_str());
   system(("cd " + outputDirectory).c_str());
   chdir(outputDirectory.c_str());
   system(("cp " + FinalCutsFile + " . ").c_str());
   if (!RunConfigFile.empty())
      system(("cp " + RunConfigFile + " . ").c_str());

   if (runOnData)
      system("mkdir -p Event-lists");

   // Init the run config
   lumi::RunLumiRanges runcfg(RunConfigFile);
   SkipEvents skipEvents(config);

   pxl::Core::initialize();
   pxl::Hep::initialize();

   // initialize the EventSelector
   EventSelector Selector(config);

   // Initialize the EventAdaptor.
   EventAdaptor Adaptor(config, debug);

   // Initialize the ParticleMatcher.
   ParticleMatcher Matcher(config, debug);

   // Initialize the Systematics.
   Systematics syst_shifter(config, debug);

   // Get fork from AnalysisComposer
   pxl::AnalysisFork fork = thisAnalysis.addForkObjects(config,
                                                        outputDirectory,
                                                        Selector,
                                                        syst_shifter,
                                                        debug);

   // begin analysis
   fork.beginJob();
   fork.beginRun();

   // performance monitoring
   double dTime1 = pxl::getCpuTime(); // Start Time
   int e = 0;                         // Event counter
   unsigned int skipped = 0;          // number of events skipped from run/LS config
   int pre_run_skipped = 0;           // number of events skipped due to skipped option

   if (config.GetItem<bool>("Pileup.UseSampleName", false))
      ReWeighter::adaptConfig(config, input_files[0]);
   ReWeighter reweighter = ReWeighter(config, 0);
   ReWeighter reweighterup = ReWeighter(config, 1);
   ReWeighter reweighterdown = ReWeighter(config, -1);

   unsigned int analyzed_files = 0;
   unsigned int lost_files = 0;

   // loop over all files
   std::vector<std::string>::const_iterator file_iter = input_files.begin();

   // initialize process info object
   ProcInfo_t info;
   // Get file handler to access files.
   // New PXL version knows how to handle dcap protocol.
   // std::auto_prt< pxl::InputFile > inFile = pxl::InputFile();
   pxl::InputFile inFile;
   for (unsigned int f = 0; f < input_files.size() && (numberOfEvents == -1 || e < numberOfEvents); f++)
   {
      std::string const fileName = *file_iter;
      // Open File:
      // open file(s):
      // pxl::InputHandler* input =
      // if( filename.substr(0,7)  == "dcap://" || filename.substr(0,6) == "/pnfs/") {
      // input = new pxl::dCacheInputFile();
      //} else {
      // if( filename.substr(0,1) != "/" ) {
      // filename = startDir + "/" + filename;
      //}
      // input = new pxl::InputFile();
      //}

      std::cout << "Opening file " << fileName << std::endl;
      unsigned int numTrials = 0;
      // we need to get done in 3 days, so don't wait too long
      // unsigned int timeout = 3*24*60*60/input_files.size();
      // cout << "Opening file (timeout " << timeout << " seconds) " << filename << endl;
      while (true)
      {
         numTrials++;
         try
         {
            time_t rawtime;
            time(&rawtime);
            std::cout << "Opening time: " << ctime(&rawtime);
            inFile.open(fileName);
         }
         catch (std::runtime_error &e)
         {
            // Wait for ( 10^numTrials - 1 ) seconds before retrying.
            // double const sleep = std::pow( 10, numTrials ) - 1.0;
            // boost::this_thread::sleep( boost::posix_time::seconds( sleep ) );
            std::cout << "Did you use an absolute path to the .pxlio file?" << std::endl;
            if (numTrials < 3)
            {
               numTrials++;
               // Retry!
               continue;
            }

            if (not runOnData)
            {
               // increase lost files counter, but don't try again
               lost_files++;
               std::cerr << "Failed to open file '" << fileName
                         << "', skipping..." << std::endl;
            }
            else
            {
               std::cerr << "Failed to open file '" << fileName
                         << "' three times. Aborting!" << std::endl;
               throw e;
            }
         }
         // increase successful files counter
         analyzed_files++;
         break;
      }

      int event_counter_per_file = 0;
      // run event loop:
      while (inFile.good())
      {
         pxl::Event *event_ptr = 0;
         try
         {
            event_ptr = dynamic_cast<pxl::Event *>(inFile.readNextObject());
         }
         catch (std::runtime_error &e)
         {
            std::cout << "end of file or unreadable event.    " << std::endl;
            break;
         }
         event_counter_per_file++;
         if (!event_ptr)
            continue;
         // pxl::Event event = *event_ptr;

         if (numberOfSkipEvents > pre_run_skipped)
         {
            delete event_ptr;
            pre_run_skipped++;
            continue;
         }
         if (numberOfEvents > -1 and e >= numberOfEvents)
            break;
         // Break the event loop if the current event is not sensible (formatted correctly).
         // if( event_ptr->getUserRecords().size() == 0 ) {
         // std::cout << "WARNING: Found corrupt pxlio event with User Record size 0 in file " << fileName << "." << std::endl;
         // std::cout << "WARNING: Continue with next event." << std::endl;
         // delete event_ptr;
         // continue;
         //}

         if (event_ptr->getObjectOwner().findObject<pxl::EventView>("Rec") == 0)
         {
            fork.analyseEvent(event_ptr);
            delete event_ptr;
            e++;
            continue;
         }

         // check if we shall analyze this event
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
            delete event_ptr;
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
            delete event_ptr;
            continue;
         }

         pxl::EventView *RecEvtView = event_ptr->getObjectOwner().findObject<pxl::EventView>("Rec");
         pxl::EventView *TrigEvtView = event_ptr->getObjectOwner().findObject<pxl::EventView>("Trig");
         pxl::EventView *FilterView = event_ptr->getObjectOwner().findObject<pxl::EventView>("Filter");

         // intialize event for adaption
         if (initAdaptor)
            Adaptor.initEvent(RecEvtView);

         if (muoCocktailUse)
         {
            // Switch to cocktail muons (use the four momentum from
            // TeV-optimised reconstructors.)
            Adaptor.applyCocktailMuons();
         }

         if (eleHEEPUse)
         {
            // Switch to HEEP ele kinematic for eles above switch pt
            Adaptor.applyHEEPElectrons();
         }

         if (fatjetPUPPIUse)
         {
            // Switch to puppi kniemtaics for fat jets
            Adaptor.applyPUPPIFatJets();
         }

         if (runOnData)
         {
            // Only needed for 2016 data //LOR COMM IT OUT
            // Adaptor.adaptDoubleEleTrigger( run, TrigEvtView );
            // for data we just need to run the selection
            Selector.performSelection(RecEvtView, 0, TrigEvtView, FilterView);
            Selector.removeOverlaps(RecEvtView);
            Selector.performOfflineTriggerSelection(RecEvtView, TrigEvtView);
         }
         else
         {
            RecEvtView->setUserRecord("EventSeed", event_ptr->getUserRecord("Run").toInt32() +
                                                       event_ptr->getUserRecord("LumiSection").toInt32() +
                                                       event_ptr->getUserRecord("EventNum").toInt32());

            // Don't do this on data, haha! And also not for special Ana hoho
            reweighter.ReWeightEvent(event_ptr);
            reweighterup.ReWeightEvent(event_ptr);
            reweighterdown.ReWeightEvent(event_ptr);
            pxl::EventView *GenEvtView = event_ptr->getObjectOwner().findObject<pxl::EventView>("Gen");

            // Sometimes events have missing PDF information (mainly POWHEG).
            // This is checked in the skimmer and if PDF weights are missing, the event is tagged
            if (config.GetItem<bool>("General.usePDF") and config.GetItem<bool>("PDF.SkipIncomplete") and GenEvtView->hasUserRecord("Incomplete_PDF_weights") and GenEvtView->getUserRecord("Incomplete_PDF_weights"))
            {
               skipped++;
               delete event_ptr;
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
            Matcher.matchObjects(GenEvtView, RecEvtView, linkName, false);

            if (jetResCorrUse)
            {
               // Change event properties according to official recommendations.
               // Don't do this on data!
               Adaptor.applyJETMETSmearing(GenEvtView, RecEvtView, linkName);
            }
            try
            {
               if (useSYST)
               {
                  // perform systematic pre. selection on all selected event views
                  //  with loosened kinematic cuts
                  Selector.performSelection(RecEvtView, GenEvtView, TrigEvtView, FilterView, true);
                  // use the config files to activate systematics for some objects
                  syst_shifter.init(event_ptr);
                  // create new event views with systematic shifts
                  syst_shifter.createShiftedViews();
                  // Check if particles fullfill standard kinematic cuts after shifting
                  for (auto &systInfo : syst_shifter.m_activeSystematics)
                  {
                     for (auto &evtView : systInfo->eventViewPointers)
                     {
                        Selector.performKinematicsSelection(evtView, false);
                        Selector.removeOverlaps(evtView);
                        Selector.performOfflineTriggerSelection(evtView, TrigEvtView);
                     }
                  }
                  // we need to check if normal rec view also survives
                  // cuts with standard kinematics cuts
                  Selector.performKinematicsSelection(RecEvtView, false);
                  Selector.removeOverlaps(RecEvtView);
                  // Now finally check triggering ( if active ) after view is fully selected
                  Selector.performOfflineTriggerSelection(RecEvtView, TrigEvtView);
               }
               else
               {
                  Selector.performSelection(RecEvtView, GenEvtView, TrigEvtView, FilterView);
                  Selector.removeOverlaps(RecEvtView);
                  Selector.performOfflineTriggerSelection(RecEvtView, TrigEvtView);
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
               delete event_ptr;
               if (runOnData)
                  exit(1);
               else
                  continue;
            }
         }
         // set user record for file name and event number in this file for further use in analysis
         if (fileName.rfind('/') != std::string::npos)
            event_ptr->setUserRecord("Filename", fileName.substr(fileName.rfind('/') + 1));
         else
            event_ptr->setUserRecord("Filename", fileName);
         event_ptr->setUserRecord("EventNumPxlio", event_counter_per_file);
         // run the fork ..
         fork.analyseEvent(event_ptr);
         fork.finishEvent(event_ptr);
         delete event_ptr;
         e++;
         std::cout << e << " <--- Event number " << std::endl;
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

         // if( e % 100000 == 0 ) PrintProcessInfo( info );
         if (do_break)
            break;
      }
      inFile.close();
      ++file_iter;
      if (do_break)
         break;
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

   fork.endRun();
   fork.endJob();

   thisAnalysis.endAnalysis();

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
```