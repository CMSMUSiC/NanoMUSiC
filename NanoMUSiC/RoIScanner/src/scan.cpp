#include "scan.hpp"
#include "ECScanner.hpp"
#include "fmt/core.h"
#include <string>

auto scan(const std::string &jsonFilePath,
          const std::string &outputDirectory,
          int rounds,
          int startRound,
          const std::string &shiftsFilePath,
          const std::string &lutFilePath,
          const std::string &scanType,
          const bool is_debug) -> bool
{
    // Create ECScanner object
    ECScanner scanner(rounds, startRound);

    // Read and parse JSON
    scanner.readInputJson(jsonFilePath);
    scanner.readLookupTable(lutFilePath);

    // Check if input contains data info ( we don't want to perform pseudo scan)
    // if (scanner.isDataScan())
    if (scanType == "data")
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

        // loops over toy rounds
        for (unsigned int i = 0; i < scanner.getDicingRounds(); i++)
        {
            // calculate index used for loading the right shifts from the shifts.json file
            unsigned int real_round_index = i + scanner.getFirstDicingRound();

            // Dice pseudo experiment...
            // if (scanner.isSignalScan())
            if (scanType == "signal")
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
            // auto toy_sampled_data = scanner.diceMcPseudoDataMT(real_round_index);

            // find roi in pseudo data
            scanner.findRoI();

            // print progress
            if (is_debug and
                (i < 10 || (i < 100 && i % 10 == 0) || (i < 1000 && i % 100 == 0) || (i >= 1000 && i % 1000 == 0)))
            {
                fmt::print("{}/{} rounds proccessed\n", i, scanner.getDicingRounds());
            }
        }
    }

    // Write result to CSV file
    scanner.writeOutputFiles(outputDirectory, scanType);

    // scanner.finalize();

    return true;
}
