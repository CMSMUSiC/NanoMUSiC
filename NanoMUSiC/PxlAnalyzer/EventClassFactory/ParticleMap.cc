#include "ParticleMap.hh"
#include "ParticleVector.hh"
#include "Resolutions.hh"

#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>

ParticleMap::ParticleMap(const std::map<std::string, std::vector<pxl::Particle *>> &particleMap)
    : m_map(), m_countMap(std::map<std::string, int>()), m_resolutionFuncMap({{"Ele", Resolutions::electron},
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
      m_funcMap({{"pt", std::bind(&pxl::Particle::getPt, std::placeholders::_1)},
                 {"et", std::bind(&pxl::Particle::getEt, std::placeholders::_1)},
                 {"px", std::bind(&pxl::Particle::getPx, std::placeholders::_1)},
                 {"py", std::bind(&pxl::Particle::getPy, std::placeholders::_1)}})
{
    for (auto &partVec : particleMap)
    {
        m_map.emplace(partVec.first, ParticleVector(partVec.first, partVec.second));
        m_countMap.emplace(partVec.first, m_map[partVec.first].getCount());
    }
}

// split one particle in two new particles according to splittingFunc
void ParticleMap::split(std::vector<pxl::Particle *> particles, std::function<bool(pxl::Particle *)> splittingFunc,
                        const std::string name1, const std::string name2)
{
    std::vector<pxl::Particle *> first = std::vector<pxl::Particle *>();
    std::vector<pxl::Particle *> second = std::vector<pxl::Particle *>();
    for (auto particle : particles)
    {
        if (splittingFunc(particle))
            first.push_back(particle);
        else
            second.push_back(particle);
    }
    m_map.emplace(name1, ParticleVector(name1, first));
    m_map.emplace(name2, ParticleVector(name1, second));
}

const ParticleVector &ParticleMap::getParticleVector(std::string &name) const
{
    return m_map.at(name);
}

int ParticleMap::getLeptonCharge(const std::map<std::string, int> &countMap) const
{
    int totalCharge = 0;
    std::list<std::string> leptons;
    leptons.push_back("Ele");
    leptons.push_back("Muon");
    leptons.push_back("Tau");
    for (auto &lepton : leptons)
    {
        auto iterator = m_map.find(lepton);
        if (iterator != m_map.end())
        {
            int counter = 0;
            for (auto &particle : iterator->second.getParticles())
            {
                counter++;
                if (counter > countMap.at(lepton))
                    break;
                totalCharge += (int)particle->getCharge();
            }
        }
    }
    return totalCharge;
}

std::map<std::string, int> ParticleMap::getCountMap() const
{
    return m_countMap;
}

double ParticleMap::getKinematicVariable(const std::string var, const std::map<std::string, int> &countMap) const
{
    double sum = 0;
    for (auto &partVec : m_map)
    {
        int counter = 0;
        for (auto &part : partVec.second.getParticles())
        {
            counter++;
            if (counter > countMap.at(partVec.first))
                break;
            sum += m_funcMap.at(var)(part);
        }
    }
    return sum;
}

double ParticleMap::getSumPt(const std::map<std::string, int> &countMap) const
{
    return getKinematicVariable("pt", countMap);
}

double ParticleMap::getSumEt(const std::map<std::string, int> &countMap) const
{
    return getKinematicVariable("et", countMap);
}

double ParticleMap::getSumPx(const std::map<std::string, int> &countMap) const
{
    return getKinematicVariable("px", countMap);
}

double ParticleMap::getSumPy(const std::map<std::string, int> &countMap) const
{
    return getKinematicVariable("py", countMap);
}

double ParticleMap::getMass(const std::map<std::string, int> &countMap) const
{
    if (countMap.at("MET"))
        return getTransverseMass(countMap);
    else
        return getInvMass(countMap);
}

double ParticleMap::getInvMass(const std::map<std::string, int> &countMap) const
{
    pxl::Particle vec_sum = pxl::Particle();
    for (auto &partVec : m_map)
    {
        int counter = 0;
        for (auto &part : partVec.second.getParticles())
        {
            counter++;
            if (counter > countMap.at(partVec.first))
                break;
            vec_sum += *part;
        }
    }
    return vec_sum.getMass();
}

double ParticleMap::getTransverseMass(const std::map<std::string, int> &count_map) const
{
    double sumEt = 0;
    double sumPx = 0;
    double sumPy = 0;
    for (auto &part_vec : m_map)
    {
        if (count_map.find(part_vec.first) == count_map.end())
            continue;
        int counter = 0;
        for (auto &part : part_vec.second.getParticles())
        {
            counter++;
            if (counter > count_map.at(part_vec.first))
                break;
            sumEt += part->getEt();
            sumPx += part->getPx();
            sumPy += part->getPy();
        }
    }

    return std::sqrt(sumEt * sumEt - sumPx * sumPx - sumPy * sumPy);
}

double ParticleMap::getMET(const std::map<std::string, int> &countMap) const
{
    if (countMap.at("MET") != 0)
        return m_map.at("MET").at(0)->getPt();
    else
        return 0;
}

double ParticleMap::getScaleFactor(const std::map<std::string, int> &countMap) const
{
    double scale_factor = 1;
    for (auto &partVec : m_map)
    {
        int counter = 0;
        for (auto &part : partVec.second.getParticles())
        {
            counter++;
            if (counter > countMap.at(partVec.first))
                break;
            scale_factor *= part->getUserRecord("scale_factor").toDouble();
        }
    }
    return scale_factor;
}

double ParticleMap::getScaleFactorError(const std::string error_type, const std::map<std::string, int> &countMap) const
{
    double scale_factor_error = 0;
    for (auto &partVec : m_map)
    {
        int counter = 0;
        for (auto &part : partVec.second.getParticles())
        {
            counter++;
            if (counter > countMap.at(partVec.first))
                break;
            if (part->hasUserRecord(error_type))
            {
                scale_factor_error += std::pow(part->getUserRecord(error_type).toDouble(), 2);
            }
        }
    }
    return std::sqrt(scale_factor_error);
}

double ParticleMap::getScaleFactorStatError(const std::map<std::string, int> &countMap) const
{
    return getScaleFactorError("scale_factor_error", countMap);
}

double ParticleMap::getScaleFactorSystError(const std::map<std::string, int> &countMap) const
{
    return getScaleFactorError("scale_factor_error_syst", countMap);
}

std::map<std::string, int> ParticleMap::getFakeMap(const std::map<std::string, int> &countMap) const
{
    std::map<std::string, int> fakeMap = std::map<std::string, int>();
    for (auto &partVec : m_map)
    {
        int counter = 0;
        for (auto &part : partVec.second.getParticles())
        {
            counter++;
            if (counter > countMap.at(partVec.first))
                break;
            if (part->hasUserRecord("Match") and part->getUserRecord("Match").toInt32() == -1)
                fakeMap[partVec.first]++;
        }
    }
    return fakeMap;
}

std::map<std::string, int> ParticleMap::getChargeFakeMap(const std::map<std::string, int> &countMap) const
{
    std::map<std::string, int> fakeMap = std::map<std::string, int>();
    for (auto &partVec : m_map)
    {
        int counter = 0;
        for (auto &part : partVec.second.getParticles())
        {
            counter++;
            if (counter > countMap.at(partVec.first))
                break;
            if (part->hasUserRecord("ChargeMatch") and part->getUserRecord("ChargeMatch").toInt32() == -1)
                fakeMap[partVec.first]++;
        }
    }
    return fakeMap;
}

std::vector<double> ParticleMap::getBinLimits(std::string distribution, std::map<std::string, int> &countMap,
                                              double min, double max, double step_size, double const fudge) const
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

    std::function<double(std::map<std::string, int>, double, double const)> resfunc;
    if (distribution == "MET")
    {
        resfunc = [this](std::map<std::string, int> countMap, double sumpt, double const fudge) {
            return getApproximateResolutionMET(countMap, sumpt, fudge);
        };
    }
    else
    {
        resfunc = [this](std::map<std::string, int> countMap, double sumpt, double const fudge) {
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

double ParticleMap::getRealResolution(const std::map<std::string, int> &countMap) const
{
    double res = 0;
    for (auto &partVec : m_map)
    {
        // MET will be done later (depending on sumpt and not met itself)
        if (partVec.first == "MET")
        {
            continue;
        }
        int counter = 0;
        for (auto &part : partVec.second.getParticles())
        {
            counter++;
            if (counter > countMap.at(partVec.first))
                break;
            res += std::pow(callResolutionFunction(partVec.first, part->getPt()), 2);
        }
    }
    if (countMap.at("MET") != 0)
    {
        double sumpt = getSumPt(countMap);
        res += std::pow(callResolutionFunction("MET", sumpt), 2);
    }
    return std::sqrt(res);
}

double ParticleMap::getApproximateResolution(const std::map<std::string, int> &countMap, double const fudge) const
{
    double sumpt = getSumPt(countMap);
    return getApproximateResolution(countMap, sumpt, fudge);
}

double ParticleMap::getApproximateResolution(const std::map<std::string, int> &countMap, double sumpt,
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

double ParticleMap::getApproximateResolutionMET(const std::map<std::string, int> &countMap, double sumpt,
                                                double const fudge) const
{
    return fudge * callResolutionFunction("MET", sumpt);
}
