#include "ECScanner.hh"
#include "boost/program_options.hpp"
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include "Profiler.hh"

// definitions for command line parsing
namespace po = boost::program_options;
namespace
{
// Define error messages for program_options
const size_t ERROR_IN_COMMAND_LINE = 1;
const size_t SUCCESS = 0;
const size_t ERROR_UNHANDLED_EXCEPTION = 2;

} // namespace

int readCommandLineOptions(int argc, char *argv[], std::string &jsonFilePath, std::string &outputDirectory, int &rounds,
                           int &startRound, std::string &shiftsFilePath, std::string &lutFilePath)
{
    po::options_description allOptions("Generic options");
    allOptions.add_options()("help",
                             "produce help message")("json,j", po::value<std::string>(&jsonFilePath)->required(),
                                                     "The input json file containing a vector of bin information")(
        "output,o", po::value<std::string>(&outputDirectory)->default_value("."),
        "Directory for output files (json, csv, root, ...). Files will still be prefixed with EC name and "
        "distribution.")("rounds,n", po::value<int>(&rounds)->default_value(1), "Number of rounds to dice")(
        "start,l", po::value<int>(&startRound)->default_value(0), "Number of round to skip in shifts file")(
        "shifts,s", po::value<std::string>(&shiftsFilePath)->default_value("shifts.json"),
        "Json file containing vectors of normalized systematic shifts for each systematic")(
        "lut", po::value<std::string>(&lutFilePath)->default_value(""),
        "LUT table to use, will choose automatically if not given.");

    // add json as positional argument
    po::positional_options_description pos;
    pos.add("json", 1);

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
    return 0;
}

int main(int argc, char *argv[])
{
    Profiler totalTimeProfiler("Total");
    totalTimeProfiler.start();

    // Initalize variables for command line options
    std::string jsonFilePath;
    std::string outputDirectory;
    int rounds;
    int startRound;
    std::string shiftsFilePath;
    std::string lutFilePath;

    // Read in command line options
    int success = readCommandLineOptions(argc, argv, jsonFilePath, outputDirectory, rounds, startRound, shiftsFilePath,
                                         lutFilePath);
    if (success > 0)
        return success;

    // Create ECScanner object
    ECScanner scanner(rounds, startRound);

    // Read and parse JSON
    scanner.readInputJson(jsonFilePath);
    scanner.readLookupTable(lutFilePath);

    // Check if input contains data info ( we don't want to perform pseudo scan)
    if (scanner.isDataScan())
    {
        scanner.findRoI();
    }
    else
    {
        // Perform multiple scans with pseudo-data otherwise (signal or MC/MC)

        // Load normalized systematic shifts: The json file is expected to contain a dictionary
        // of lists. Each dictionary entry corresponds to one systematic, each systematic
        // has multiple "pseudo-real-world" values, drawn from a normal distribution of width 1
        // around 0. The normalized values will later be scaled and shifted to represent an
        // uncertainty on the bin count.
        scanner.readSystematicShiftsFile(shiftsFilePath);

        for (unsigned int i = 0; i < scanner.getDicingRounds(); i++)
        {
            // calculate index used for loading the right shifts from the shifts.json file
            unsigned int real_round_index = i + scanner.getFirstDicingRound();

            // Dice pseudo experiment...
            if (scanner.isSignalScan())
            {
                // Dice around the signal expectation.
                // Note that the dicing result is still scanned against the *SM* expectation,
                // not against the signal expectation.
                scanner.diceSignalPseudoData(real_round_index);
            }
            else
            {
                // Dice around the SM expectation
                scanner.diceMcPseudoData(real_round_index);
            }

            // find roi in pseudo data
            scanner.findRoI();

            // print progress
            if (i < 10 || (i < 100 && i % 10 == 0) || (i < 1000 && i % 100 == 0) || (i >= 1000 && i % 1000 == 0))
            {
                std::cout << i << "/" << scanner.getDicingRounds() << " rounds proccessed" << std::endl;
            }
        }
    }

    // Write result to CSV file
    scanner.writeOutputFiles(outputDirectory);

    totalTimeProfiler.stop();

    scanner.finalize();

    totalTimeProfiler.print();

    std::cout << "Scan complete" << std::endl;
}
