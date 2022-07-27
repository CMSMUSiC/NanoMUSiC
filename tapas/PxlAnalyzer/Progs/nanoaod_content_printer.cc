#include <string>
#include <unordered_set>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wattributes"
#include <boost/filesystem/path.hpp>
#pragma GCC diagnostic pop
#include "boost/program_options.hpp"

// ROOT Stuff
#include "TFile.h"
#include "TTree.h"

#include "Main/NanoAODReader.hh"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace
{
   // Define error messages for program_options
   const size_t ERROR_IN_COMMAND_LINE = 1;
   const size_t SUCCESS = 0;
   const size_t ERROR_UNHANDLED_EXCEPTION = 2;

} // namespace

int main(int argc, char *argv[])
{
   if (getenv("MUSIC_BASE") == NULL)
   {
      throw std::runtime_error("MUSIC_BASE not set!");
   }

   std::cout << "-->> NanoAOD Printer <<--" << std::endl;

   std::string input_file;
   std::vector<std::string> arguments;

   po::options_description genericOptions("Generic options");
   genericOptions.add_options()("help", "produce help message")("input ", po::value<std::string>(&input_file)->required(), "A NanoAOD file to print content.");

   // add positional arguments
   po::positional_options_description pos;
   pos.add("NANOAOD_FILE", 1);

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
      std::cerr << "ERROR: " << e.what() << std::endl
                << std::endl;
      std::cerr << allOptions << std::endl;
      return ERROR_IN_COMMAND_LINE;
   }

   // temp cache dir
   std::cout << "Preparing cache dir: " << std::endl;
   std::string process_hash = std::to_string(std::hash<std::string>{}(input_file));
   std::string cache_dir = "/tmp/music/proc_" + process_hash;
   system(("rm -rf " + cache_dir).c_str());
   std::cout << cache_dir << std::endl;

   std::cout << "Opening file " << input_file << std::endl;

   TFile::SetCacheFileDir(cache_dir);
   std::unique_ptr<TFile> inFile(TFile::Open(input_file.c_str(), "CACHEREAD"));

   if (!inFile)
   {
      std::cout << "ERROR: could not open data file" << std::endl;
      exit(1);
   }

   std::cout << "Opening time: " << std::endl;

   // get "Events" TTree from file
   auto events_tree = (TTree *)inFile->Get("Events");

   // get NanoAODReader
   auto nano_reader = new NanoAODReader(events_tree);

   // loop over events
   while (nano_reader->next())
   {
      nano_reader->printContent();
      break;
   }

   inFile->Close();

   // clear cache dir
   std::cout << "Cleaning cache dir..." << std::endl;
   system(("rm -rf " + cache_dir + "/*").c_str());

   return 0;
}