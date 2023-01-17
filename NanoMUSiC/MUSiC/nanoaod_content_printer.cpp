#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wattributes"
#include <boost/filesystem/path.hpp>
#pragma GCC diagnostic pop
#include "boost/program_options.hpp"

// ROOT Stuff
#include "TFile.h"
#include "TTree.h"

#include "NanoAODReader.hpp"

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
    if (getenv("PXLANALYZER_BASE") == NULL)
    {
        throw std::runtime_error("PXLANALYZER_BASE not set!");
    }

    std::cout << "-->> NanoAOD Printer <<--" << std::endl;

    std::string input_file;
    std::string particle = "";
    std::string outputFile = "";
    std::vector<std::string> arguments;

    po::options_description genericOptions("Generic options");
    genericOptions.add_options()("help", "produce help message");
    genericOptions.add_options()(
        "particle", po::value<std::string>(&particle),
        "Get a template of how to get information on a given particle. Example: \"--particle Tau_\"");
    genericOptions.add_options()("output", po::value<std::string>(&outputFile),
                                 "Path to output file, in order to save contents.");
    genericOptions.add_options()("input", po::value<std::string>(&input_file)->required(),
                                 "A NanoAOD file to print content.");

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
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << allOptions << std::endl;
        return ERROR_IN_COMMAND_LINE;
    }

    std::cout << "Opening file " << input_file << std::endl;

    std::unique_ptr<TFile> inFile(TFile::Open(input_file.c_str()));

    if (!inFile)
    {
        std::cout << "ERROR: could not open data file" << std::endl;
        exit(1);
    }

    // get "Events" TTree from file
    std::unique_ptr<TTree> events_tree = std::unique_ptr<TTree>(dynamic_cast<TTree *>(inFile->Get("Events")));
    // events_tree->Print();

    std::ofstream f_out(outputFile);
    f_out.rdbuf();
    std::streambuf *coutbuf = std::cout.rdbuf(); // save old buf
    if (outputFile != "")
    {
        std::cout.rdbuf(f_out.rdbuf()); // redirect std::cout to file
    }

    // get NanoAODReader
    NanoAODReader nano_reader(*events_tree);

    nano_reader.next();
    nano_reader.printContent();

    if (particle != "")
    {
        nano_reader.next();
        nano_reader.getTemplate(particle);
    }

    std::cout.rdbuf(coutbuf); // reset to standard output again
    return 0;
}