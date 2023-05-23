
#include "BinLimits.hpp"
#include "fmt/format.h"
#include <cmath>
#include <functional>
#include <stdexcept>

auto BinLimits::getApproximateResolution(const std::map<std::string, int> &countMap, double sumpt, double const fudge)
    -> const double
{
    double res = 0;
    int numObjects = 0;
    for (const auto &[object, count] : countMap)
    {
        numObjects += count;
    }

    double ptPerObj = sumpt / numObjects;

    // for (auto &count : countMap)
    for (const auto &[object, count] : countMap)
    {
        // MET depending on sumpt and not MET itself
        if (object == "MET")
        {
            res += count * std::pow(callResolutionFunction("MET", sumpt), 2);
        }
        else
        {
            res += count * std::pow(callResolutionFunction(object, ptPerObj), 2);
        }
    }
    return fudge * std::sqrt(res);
}

auto BinLimits::getApproximateResolutionMET(const std::map<std::string, int> &countMap,
                                            double sumpt,
                                            double const fudge) -> const double
{
    return fudge * callResolutionFunction("MET", sumpt);
}

auto BinLimits::callResolutionFunction(const std::string &name, const double &value) -> const double
{

    const std::map<std::string, std::function<double(double)>> resolutionFuncMap = {
        {"Ele", ResolutionsFuncs::electron},
        {"EleEE", ResolutionsFuncs::electron},
        {"EleEB", ResolutionsFuncs::electron},
        {"Muon", ResolutionsFuncs::muon},
        {"Gamma", ResolutionsFuncs::gamma},
        {"GammaEB", ResolutionsFuncs::gamma},
        {"GammaEE", ResolutionsFuncs::gamma},
        {"Tau", ResolutionsFuncs::tau},
        {"Jet", ResolutionsFuncs::jet},
        {"bJet", ResolutionsFuncs::jet},
        {"MET", ResolutionsFuncs::met}};

    double res;
    try
    {
        res = resolutionFuncMap.at(name)(value);
    }
    catch (std::bad_function_call &e)
    {

        throw(std::runtime_error(fmt::format(
            "Found no function in resolutionFuncMap for particle {}. Did you forget to add a function in the "
            "constructor?",
            name)));
    }
    return res;
}

auto BinLimits::get_bin_limits(const std::string &distribution,
                               const std::map<std::string, int> &countMap,
                               double min,
                               double max,
                               double step_size,
                               const double fudge) -> const std::vector<double>
{
    // check a number of things
    if (min < 0)
        throw std::range_error("min must be >= 0!");
    if (max <= 0)
        throw std::range_error("max must be > 0!");
    if (max <= min)
        throw std::range_error("max must be > min!");
    if (step_size <= 0)
        throw std::range_error("step_size must be > 0!");

    std::vector<double> bins;

    // first bin is always the minimal value
    bins.push_back(min);

    double last_value = min;
    double next_value = min + step_size;

    std::function<double(std::map<std::string, int>, double, const double)> resfunc;
    if (distribution == "MET")
    {
        resfunc = [](std::map<std::string, int> countMap, double sumpt, double const fudge) -> double
        {
            return getApproximateResolutionMET(countMap, sumpt, fudge);
        };
    }
    else
    {
        resfunc = [](std::map<std::string, int> countMap, double sumpt, double const fudge) -> double
        {
            return getApproximateResolution(countMap, sumpt, fudge);
        };
    }

    // work as long we are before the end
    while (next_value < max)
    {
        // enlarge bin by step_size as long as it is smaller than the resolution
        while ((next_value - last_value) < resfunc(countMap, (next_value + last_value) / 2, fudge))
        {
            if (next_value >= max)
                break; // stop in case we ran over the end
            else
                next_value += step_size;
        }
        if (next_value >= max)
            break; // we are done once we are at or beyond the end
        else
        {
            // new end of bin is valid, so add it
            bins.push_back(next_value);
            last_value = next_value;
            next_value += step_size;
        }
    }

    // the last bin should also be at least as wide as step_size
    if ((bins.back() + step_size) > max)
    {
        // but we need at least one bin
        if (bins.size() > 1)
            bins.pop_back();
    }
    // last bin is always the maximal value
    bins.push_back(max);

    // and done
    return bins;
}
