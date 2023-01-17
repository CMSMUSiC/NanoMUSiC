#include <cmath>
#include <iomanip>
#include <iostream>

#include "boost/program_options.hpp"

#include "ConvolutionComputer.hpp"
#include "ConvolutionLookup.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    double N_obs = 0;
    double N_exp = 0;
    double sigma_exp = 0;
    int debugLevel = 1;

    po::options_description allOptions("Generic options");
    allOptions.add_options()("help", "produce help message")("batch", "only print p-value and return")(
        "normal", "use normal prior instead of log-normal")(
        "no-lookup", "skip the lookup table and perform full calculation")("obs", po::value<double>(&N_obs)->required(),
                                                                           "observed event yield in this region")(
        "exp", po::value<double>(&N_exp)->required(), "expected event yield in this region")(
        "sigma-exp", po::value<double>(&sigma_exp)->required(), "total uncertainty on expected event yield")(
        "debug", po::value<int>(&debugLevel), "debuglevel: 0 = errors only, 1 = warnings, 2 = info, 3 = debug");

    // parse command line options
    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(allOptions).run(), vm);
        if (vm.count("help"))
        {
            std::cout << "A simple tool to calculate p-values with a hybrid bayesian / frequentist approach"
                      << std::endl;
            std::cout << allOptions << std::endl;
            return 0;
        }
        po::notify(vm);
    }
    catch (po::error &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << allOptions << std::endl;
        return 1;
    }

    const PriorMode prior = vm.count("normal") ? NORMAL_PRIOR : LOGNORMAL_PRIOR;

    const bool quiet = vm.count("batch");

    LookupTable lookupTable(true);
    lookupTable.readFile();

    double p = -1.0;
    if (!vm.count("no-lookup"))
    {
        p = lookupTable.lookup(N_obs, N_exp, sigma_exp);
        if (p < 0)
        {
            if (!quiet)
            {
                std::cout << "p-value not found in lookup table, calculating manually..." << std::endl;
            }
            p = compute_p_convolution(N_obs, N_exp, sigma_exp, prior, debugLevel);
        }
        else if (!quiet)
        {
            std::cout << "p-value found in lookup table." << std::endl;
        }
    }
    else
    {
        if (!quiet)
        {
            std::cout << "User requested manual calculation..." << std::endl;
        }
        p = compute_p_convolution(N_obs, N_exp, sigma_exp, prior, debugLevel);
    }

    if (quiet)
    {
        std::cout << p << std::endl;
    }
    else
    {
        std::cout << "Input:" << std::endl
                  << "\tExpected: ( " << N_exp << " +- " << sigma_exp << " )" << std::endl
                  << "\tObserved:  " << N_obs << std::endl
                  << "Output:" << std::endl
                  << "\tp-value:        " << p << std::endl;
        if (p < 0.05)
        {
            const double u = -2. * std::log(p * std::sqrt(2 * M_PI));
            const double Z = std::sqrt(u - std::log(u));
            std::cout << "\tapprox Z-value: " << std::fixed << std::setprecision(1) << Z << std::endl;
        }
    }

    return 0;
}
