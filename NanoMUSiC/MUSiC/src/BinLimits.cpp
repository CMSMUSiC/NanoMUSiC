
#include "BinLimits.hpp"
#include "fmt/core.h"
#include <cmath>
#include <cstdlib>
#include <functional>
#include <stdexcept>

auto BinLimits::getApproximateResolution(const std::unordered_map<ObjectNames, int> &count_map,
                                         double sumpt,
                                         double const fudge) -> const double
{
    double res = 0;
    int numObjects = 0;
    for (const auto &[object, count] : count_map)
    {
        numObjects += count;
    }

    double ptPerObj = sumpt / numObjects;

    // for (auto &count : count_map)
    for (const auto &[object, count] : count_map)
    {
        // MET depending on sumpt and not MET itself
        if (object == ObjectNames::MET)
        {
            res += count * std::pow(callResolutionFunction(ObjectNames::MET, sumpt), 2);
        }
        else
        {
            res += count * std::pow(callResolutionFunction(object, ptPerObj), 2);
        }
    }
    return fudge * std::sqrt(res);
}

auto BinLimits::getApproximateResolutionMET(const std::unordered_map<ObjectNames, int> &count_map,
                                            double sumpt,
                                            double const fudge) -> const double
{
    return fudge * callResolutionFunction(ObjectNames::MET, sumpt);
}

auto BinLimits::callResolutionFunction(const ObjectNames name, const double &value) -> const double
{

    const std::unordered_map<ObjectNames, std::function<double(double)>> resolutionFuncMap = {
        {ObjectNames::Muon, ResolutionsFuncs::muon},
        {ObjectNames::Electron, ResolutionsFuncs::electron},
        {ObjectNames::Photon, ResolutionsFuncs::photon},
        {ObjectNames::Tau, ResolutionsFuncs::tau},
        {ObjectNames::bJet, ResolutionsFuncs::jet},
        {ObjectNames::Jet, ResolutionsFuncs::jet},
        {ObjectNames::MET, ResolutionsFuncs::met}};

    double res;
    try
    {
        res = resolutionFuncMap.at(name)(value);
    }
    catch (std::bad_function_call &e)
    {
        fmt::print(stderr, "ERROR: Cound not find function in resolutionFuncMap for given object.");
        std::exit(EXIT_FAILURE);
    }
    return res;
}

auto BinLimits::limits(const std::unordered_map<ObjectNames, int> &count_map,
                       const bool isMET,
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

    std::function<double(std::unordered_map<ObjectNames, int>, double, const double)> resfunc;
    if (isMET)
    {
        resfunc = [](std::unordered_map<ObjectNames, int> count_map, double sumpt, double const fudge) -> double
        {
            return getApproximateResolutionMET(count_map, sumpt, fudge);
        };
    }
    else
    {
        resfunc = [](std::unordered_map<ObjectNames, int> count_map, double sumpt, double const fudge) -> double
        {
            return getApproximateResolution(count_map, sumpt, fudge);
        };
    }

    // work as long we are before the end
    while (next_value < max)
    {
        // enlarge bin by step_size as long as it is smaller than the resolution
        while ((next_value - last_value) < resfunc(count_map, (next_value + last_value) / 2, fudge))
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
