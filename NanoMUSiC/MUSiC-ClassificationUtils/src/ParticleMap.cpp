#include "ParticleMap.hpp"
#include "Resolutions.hpp"

#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>

#include "fmt/format.h"

ParticleMap::ParticleMap(const ParticleMap_t &particleMap,
                         const ScaleFactorMap_t &ScaleFactorMap,
                         const MatchMap_t &matchMap)
    : m_map(particleMap),
      m_scale_factor_map(ScaleFactorMap),
      m_match_map(matchMap),
      m_countMap(CountMap_t()),
      m_resolutionFuncMap({{"Ele", Resolutions::electron},
                           {"EleEE", Resolutions::electron},
                           {"EleEB", Resolutions::electron},
                           {"Muon", Resolutions::muon},
                           {"Gamma", Resolutions::gamma},
                           {"GammaEB", Resolutions::gamma},
                           {"GammaEE", Resolutions::gamma},
                           {"Tau", Resolutions::tau},
                           {"Jet", Resolutions::jet},
                           {"bJet", Resolutions::jet},
                           {"MET", Resolutions::met}}),
      m_funcMap({{"pt",
                  [](const Math::PtEtaPhiMVector &particle) -> double
                  {
                      return particle.pt();
                  }},
                 {"et",
                  [](const Math::PtEtaPhiMVector &particle) -> double
                  {
                      return particle.Et();
                  }},
                 {"px",
                  [](const Math::PtEtaPhiMVector &particle) -> double
                  {
                      return particle.px();
                  }},
                 {"py",
                  [](const Math::PtEtaPhiMVector &particle) -> double
                  {
                      return particle.py();
                  }}})
{
    for (auto &&[part_type, part_vector] : particleMap)
    {
        m_countMap.emplace(part_type, part_vector.size());
    }
}

auto ParticleMap::make_empty_particle_maps() -> std::tuple<ParticleMap_t, ScaleFactorMap_t, MatchMap_t>
{

    auto particles_map = ParticleMap::ParticleMap_t({{"Ele", {}},
                                                     {"EleEE", {}},
                                                     {"EleEB", {}},
                                                     {"Muon", {}},
                                                     {"Gamma", {}},
                                                     {"GammaEB", {}},
                                                     {"GammaEE", {}},
                                                     {"Tau", {}},
                                                     {"Jet", {}},
                                                     {"bJet", {}},
                                                     {"MET", {}}});
    auto scalefactors_map = ParticleMap::ScaleFactorMap_t({{"Ele", {}},
                                                           {"EleEE", {}},
                                                           {"EleEB", {}},
                                                           {"Muon", {}},
                                                           {"Gamma", {}},
                                                           {"GammaEB", {}},
                                                           {"GammaEE", {}},
                                                           {"Tau", {}},
                                                           {"Jet", {}},
                                                           {"bJet", {}},
                                                           {"MET", {}}});
    auto matches_map = ParticleMap::MatchMap_t({{"Ele", {}},
                                                {"EleEE", {}},
                                                {"EleEB", {}},
                                                {"Muon", {}},
                                                {"Gamma", {}},
                                                {"GammaEB", {}},
                                                {"GammaEE", {}},
                                                {"Tau", {}},
                                                {"Jet", {}},
                                                {"bJet", {}},
                                                {"MET", {}}});

    return std::tuple<ParticleMap_t, ScaleFactorMap_t, MatchMap_t>(particles_map, scalefactors_map, matches_map);
}

auto ParticleMap::erase(const std::string &particle_type) -> void
{
    if (

        m_map.count(particle_type) != 0 or m_scale_factor_map.count(particle_type) != 0 or
        m_match_map.count(particle_type) != 0 or m_countMap.count(particle_type) != 0

    )
    {
        m_map.erase(particle_type);
        m_scale_factor_map.erase(particle_type);
        m_match_map.erase(particle_type);
        m_countMap.erase(particle_type);
        return;
    }
    throw std::runtime_error(
        fmt::format("ERROR: could not erase ParticleMap. Particle type {} not found.", particle_type));
}

// not needed (?)
// split one particle in two new particles according to splittingFunc
// void ParticleMap::split(std::vector<pxl::Particle *> particles,
//                         std::function<bool(pxl::Particle *)> splittingFunc,
//                         const std::string name1,
//                         const std::string name2)
// {
//     std::vector<pxl::Particle *> first = std::vector<pxl::Particle *>();
//     std::vector<pxl::Particle *> second = std::vector<pxl::Particle *>();
//     for (auto particle : particles)
//     {
//         if (splittingFunc(particle))
//             first.push_back(particle);
//         else
//             second.push_back(particle);
//     }
//     m_map.emplace(name1, ParticleVector(name1, first));
//     m_map.emplace(name2, ParticleVector(name1, second));
// }

const RVec<Math::PtEtaPhiMVector> &ParticleMap::getParticleVector(std::string &name) const
{
    return m_map.at(name);
}

// int ParticleMap::getLeptonCharge(const std::unordered_map<std::string, int> &countMap) const
// {
//     int totalCharge = 0;
//     std::list<std::string> leptons;
//     leptons.push_back("Ele");
//     leptons.push_back("Muon");
//     leptons.push_back("Tau");
//     for (auto &lepton : leptons)
//     {
//         auto iterator = m_map.find(lepton);
//         if (iterator != m_map.end())
//         {
//             int counter = 0;
//             for (auto &particle : iterator->second.getParticles())
//             {
//                 counter++;
//                 if (counter > countMap.at(lepton))
//                     break;
//                 tota
// lCharge += (int)particle->getCharge();
//             }
//         }
//     }
//     return totalCharge;
// }

ParticleMap::CountMap_t ParticleMap::getCountMap() const
{
    return m_countMap;
}

double ParticleMap::getKinematicVariable(const std::string var,
                                         const std::unordered_map<std::string, int> &countMap) const
{
    double sum = 0.;
    for (auto &&[part_type, part_vector] : m_map)
    {
        int counter = 0;
        for (auto &part : part_vector)
        {
            counter++;
            if (counter > countMap.at(part_type))
            {
                break;
            }
            sum += m_funcMap.at(var)(part);
        }
    }
    return sum;
}

double ParticleMap::getSumPt(const std::unordered_map<std::string, int> &countMap) const
{
    return getKinematicVariable("pt", countMap);
}

double ParticleMap::getSumEt(const std::unordered_map<std::string, int> &countMap) const
{
    return getKinematicVariable("et", countMap);
}

double ParticleMap::getSumPx(const std::unordered_map<std::string, int> &countMap) const
{
    return getKinematicVariable("px", countMap);
}

double ParticleMap::getSumPy(const std::unordered_map<std::string, int> &countMap) const
{
    return getKinematicVariable("py", countMap);
}

double ParticleMap::getMass(const std::unordered_map<std::string, int> &countMap) const
{
    if (countMap.at("MET"))
    {
        return getTransverseMass(countMap);
    }
    else
    {
        return getInvMass(countMap);
    }
}

double ParticleMap::getInvMass(const std::unordered_map<std::string, int> &countMap) const
{
    auto vec_sum = Math::PtEtaPhiMVector();

    for (auto &&[part_type, part_vector] : m_map)
    {
        int counter = 0;
        for (auto &part : part_vector)
        {
            counter++;
            if (counter > countMap.at(part_type))
            {
                break;
            }
            vec_sum += part;
        }
    }
    return vec_sum.mass();
}

double ParticleMap::getTransverseMass(const std::unordered_map<std::string, int> &count_map) const
{
    double sumEt = 0;
    double sumPx = 0;
    double sumPy = 0;

    for (auto &&[part_type, part_vector] : m_map)
    {
        if (count_map.find(part_type) == count_map.end())
            continue;
        int counter = 0;
        for (auto &part : part_vector)
        {
            counter++;
            if (counter > count_map.at(part_type))
            {
                break;
            }
            sumEt += part.Et();
            sumPx += part.px();
            sumPy += part.py();
        }
    }

    return std::sqrt(sumEt * sumEt - sumPx * sumPx - sumPy * sumPy);
}

double ParticleMap::getMET(const std::unordered_map<std::string, int> &countMap) const
{
    if (countMap.at("MET") != 0)
    {
        return m_map.at("MET").at(0).pt();
    }

    return 0;
}

double ParticleMap::getScaleFactor(const std::unordered_map<std::string, int> &countMap) const
{
    double scale_factor = 1;
    for (auto &[part_type, part_scale_factor] : m_scale_factor_map)
    {
        int counter = 0;
        for (auto &sf : part_scale_factor.at("scale_factor"))
        {
            counter++;
            if (counter > countMap.at(part_type))
            {
                break;
            }
            scale_factor *= sf;
        }
    }
    return scale_factor;
}

double ParticleMap::getScaleFactorError(const std::string error_type,
                                        const std::unordered_map<std::string, int> &countMap) const
{
    double scale_factor_error = 0;
    for (auto &[part_type, part_scale_factor] : m_scale_factor_map)
    {
        int counter = 0;
        if (part_scale_factor.find(error_type) != part_scale_factor.end())
        {
            for (auto &sf : part_scale_factor.at(error_type))
            {
                counter++;
                if (counter > countMap.at(part_type))
                {
                    break;
                }
                scale_factor_error += std::pow(sf, 2);
            }
        }
    }
    return std::sqrt(scale_factor_error);
}

double ParticleMap::getScaleFactorStatError(const std::unordered_map<std::string, int> &countMap) const
{
    return getScaleFactorError("scale_factor_error", countMap);
}

double ParticleMap::getScaleFactorSystError(const std::unordered_map<std::string, int> &countMap) const
{
    return getScaleFactorError("scale_factor_error_syst", countMap);
}

ParticleMap::CountMap_t ParticleMap::getFakeMap(const ParticleMap::CountMap_t &countMap) const
{
    auto fakeMap = ParticleMap::CountMap_t();

    for (auto &[part_type, part_matches] : m_match_map)
    {
        int counter = 0;
        for (auto &has_match : part_matches)
        {
            counter++;
            if (counter > countMap.at(part_type))
            {
                break;
            }
            if (not(has_match))
                fakeMap[part_type]++;
        }
    }
    return fakeMap;
}

// std::unordered_map<std::string, int> ParticleMap::getChargeFakeMap(
//     const std::unordered_map<std::string, int> &countMap) const
// {
//     std::unordered_map<std::string, int> fakeMap = std::unordered_map<std::string, int>();
//     for (auto &partVec : m_map)
//     {
//         int counter = 0;
//         for (auto &part : partVec.second.getParticles())
//         {
//             counter++;
//             if (counter > countMap.at(partVec.first))
//                 break;
//             if (part->hasUserRecord("ChargeMatch") and part->getUserRecord("ChargeMatch").toInt32() == -1)
//                 fakeMap[partVec.first]++;
//         }
//     }
//     return fakeMap;
// }

std::vector<double> ParticleMap::getBinLimits(std::string distribution,
                                              std::unordered_map<std::string, int> &countMap,
                                              double min,
                                              double max,
                                              double step_size,
                                              double const fudge) const
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

    std::function<double(std::unordered_map<std::string, int>, double, const double)> resfunc;
    if (distribution == "MET")
    {
        resfunc = [this](std::unordered_map<std::string, int> countMap, double sumpt, double const fudge) -> double
        {
            return getApproximateResolutionMET(countMap, sumpt, fudge);
        };
    }
    else
    {
        resfunc = [this](std::unordered_map<std::string, int> countMap, double sumpt, double const fudge) -> double
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

double ParticleMap::callResolutionFunction(const std::string &name, const double &value) const
{
    double res;
    try
    {
        res = m_resolutionFuncMap.at(name)(value);
    }
    catch (std::bad_function_call &e)
    {
        std::stringstream error;
        error << "Found no function in m_resolutionFuncMap for particle " << name << std::endl;
        error << "Did you forget to add a function in the constructor?" << std::endl;
        throw(std::runtime_error(error.str()));
    }
    return res;
}

double ParticleMap::getRealResolution(const std::unordered_map<std::string, int> &countMap) const
{
    double res = 0;
    // for (auto &partVec : m_map)
    for (auto &[part_type, part_vector] : m_map)
    {
        // MET will be done later (depending on sumpt and not met itself)
        if (part_type == "MET")
        {
            continue;
        }

        int counter = 0;
        for (auto &part : part_vector)
        {
            counter++;
            if (counter > countMap.at(part_type))
                break;
            res += std::pow(callResolutionFunction(part_type, part.pt()), 2);
        }
    }

    if (countMap.at("MET") != 0)
    {
        double sumpt = getSumPt(countMap);
        res += std::pow(callResolutionFunction("MET", sumpt), 2);
    }
    return std::sqrt(res);
}

double ParticleMap::getApproximateResolution(const std::unordered_map<std::string, int> &countMap,
                                             double const fudge) const
{
    double sumpt = getSumPt(countMap);
    return getApproximateResolution(countMap, sumpt, fudge);
}

double ParticleMap::getApproximateResolution(const std::unordered_map<std::string, int> &countMap,
                                             double sumpt,
                                             double const fudge) const
{
    double res = 0;
    int numObjects = 0;
    for (auto &count : countMap)
        numObjects += count.second;
    double ptPerObj = sumpt / numObjects;
    for (auto &count : countMap)
    {
        // MET depending on sumpt and not met itself
        if (count.first == "MET")
        {
            res += count.second * std::pow(callResolutionFunction("MET", sumpt), 2);
        }
        else
        {
            res += count.second * std::pow(callResolutionFunction(count.first, ptPerObj), 2);
        }
    }
    return fudge * std::sqrt(res);
}

double ParticleMap::getApproximateResolutionMET(const std::unordered_map<std::string, int> &countMap,
                                                double sumpt,
                                                double const fudge) const
{
    return fudge * callResolutionFunction("MET", sumpt);
}