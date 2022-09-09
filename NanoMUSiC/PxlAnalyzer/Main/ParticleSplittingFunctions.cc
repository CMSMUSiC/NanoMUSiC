#include "Main/ParticleSplittingFunctions.hh"

void Splitting::split(std::map<std::string, std::vector<pxl::Particle *>> &map,
                      const std::function<bool(pxl::Particle *)> &splittingFunc, const std::string &origName,
                      const std::string &name1, const std::string &name2)
{
    std::vector<pxl::Particle *> first = std::vector<pxl::Particle *>();
    std::vector<pxl::Particle *> second = std::vector<pxl::Particle *>();

    if (map.find(origName) == map.end())
    {
        std::stringstream error;
        error << "[ERROR](Splitting::split) Did not find particle '" << origName << "' in particle map." << std::endl;
        throw(std::runtime_error(error.str()));
    }

    for (auto particle : map[origName])
    {
        if (splittingFunc(particle))
            first.push_back(particle);
        else
            second.push_back(particle);
    }

    map.erase(origName);

    map.emplace(name1, first);
    map.emplace(name2, second);
}

void Splitting::merge(std::map<std::string, std::vector<pxl::Particle *>> &map, const std::vector<std::string> &names,
                      const std::string &newName)
{
    for (auto &name : names)
    {
        if (map.find(name) == map.end())
        {
            std::stringstream error;
            error << "[ERROR](Splitting::merge) Did not find particle '" << name << "' in particle map." << std::endl;
            throw(std::runtime_error(error.str()));
        }
    }
    // sort pt
    auto sortFunc = [](pxl::Particle *particle1, pxl::Particle *particle2) {
        return particle1->getPt() > particle2->getPt();
    };
    // first list for quicker sort
    std::list<pxl::Particle *> list;
    for (auto &name : names)
    {
        std::copy(map[name].begin(), map[name].end(), std::back_inserter(list));
    }
    list.sort(sortFunc);
    std::vector<pxl::Particle *> newVector = std::vector<pxl::Particle *>();
    newVector.reserve(list.size());
    std::copy(list.begin(), list.end(), std::back_inserter(newVector));

    for (auto &name : names)
    {
        map.erase(name);
    }
    map.emplace(newName, newVector);
}

void Splitting::merge(std::map<std::string, std::vector<pxl::Particle *>> &map, const std::string &name1,
                      const std::string &name2, const std::string &newName)
{
    if (map.find(name1) == map.end())
    {
        std::stringstream error;
        error << "[ERROR](Splitting::merge) Did not find particle '" << name1 << "' in particle map." << std::endl;
        throw(std::runtime_error(error.str()));
    }
    if (map.find(name2) == map.end())
    {
        std::stringstream error;
        error << "[ERROR](Splitting::merge) Did not find particle '" << name2 << "' in particle map." << std::endl;
        throw(std::runtime_error(error.str()));
    }
    // sort pt
    auto sortFunc = [](pxl::Particle *particle1, pxl::Particle *particle2) {
        return particle1->getPt() > particle2->getPt();
    };
    // first list for quicker sort
    std::list<pxl::Particle *> list1 = std::list<pxl::Particle *>();
    std::list<pxl::Particle *> list2 = std::list<pxl::Particle *>();
    std::copy(map[name1].begin(), map[name1].end(), std::back_inserter(list1));
    std::copy(map[name2].begin(), map[name2].end(), std::back_inserter(list2));
    list1.merge(list2, sortFunc);
    std::vector<pxl::Particle *> newVector = std::vector<pxl::Particle *>();
    newVector.reserve(list1.size());
    std::copy(list1.begin(), list1.end(), std::back_inserter(newVector));

    map.erase(name1);
    map.erase(name2);
    map.emplace(newName, newVector);
}

void Splitting::splitBjets(std::map<std::string, std::vector<pxl::Particle *>> &map)
{
    split(
        map,
        [](pxl::Particle *particle) { return particle->hasUserRecord("isBjet") && particle->getUserRecord("isBjet"); },
        "Jet", "bJet", "Jet");
}

void Splitting::splitGammaEndcap(std::map<std::string, std::vector<pxl::Particle *>> &map)
{
    split(
        map,
        [](pxl::Particle *particle) {
            return particle->hasUserRecord("isEndcap") && particle->getUserRecord("isEndcap");
        },
        "Gamma", "GammaEE", "GammaEB");
}

void Splitting::splitEleEndcap(std::map<std::string, std::vector<pxl::Particle *>> &map)
{
    split(
        map,
        [](pxl::Particle *particle) {
            return particle->hasUserRecord("isEndcap") && particle->getUserRecord("isEndcap");
        },
        "Ele", "EleEE", "EleEB");
}

void Splitting::splitWfatjets(std::map<std::string, std::vector<pxl::Particle *>> &map)
{
    split(
        map,
        [](pxl::Particle *particle) { return particle->hasUserRecord("isWjet") && particle->getUserRecord("isWjet"); },
        "FatJet", "wJet", "FatJet");
}

void Splitting::mergeBjets(std::map<std::string, std::vector<pxl::Particle *>> &map)
{
    merge(map, "Jet", "bJet", "Jet");
}

void Splitting::mergeWfatjets(std::map<std::string, std::vector<pxl::Particle *>> &map)
{
    merge(map, "FatJet", "wJet", "FatJet");
}

void Splitting::mergeZ(std::map<std::string, std::vector<pxl::Particle *>> &map, double deltaM,
                       std::list<std::string> particles, std::string newName)
{
    const double nominalMass = 91.2;
    std::map<std::string, std::list<pxl::Particle *>> newLists;
    for (auto &particle : particles)
    {
        if (map.find(particle) == map.end())
        {
            std::stringstream error;
            error << "[ERROR](Splitting::mergeZ) Did not find particle '" << particle << "' in particle map."
                  << std::endl;
            throw(std::runtime_error(error.str()));
        }
        newLists.emplace(particle, std::list<pxl::Particle *>());
    }
    newLists.emplace(newName, std::list<pxl::Particle *>());

    for (auto &particle : particles)
    {
        // If one particle could be used for multiple Zs, take Z with closest mass
        // first create matrix of all possible two particle masses
        int n = map[particle].size();
        double **masses = new double *[n];
        for (int i = 0; i < n; i++)
        {
            masses[i] = new double[i];
        }
        int c1 = 0;
        for (auto part1 = map[particle].begin(); part1 != map[particle].end(); part1++, c1++)
        {
            int c2 = 0;
            for (auto part2 = map[particle].begin(); part2 != part1; part2++, c2++)
            {
                // pxl only knows += and not +
                pxl::Particle tmp = pxl::Particle(**part1);
                tmp += **part2;
                masses[c1][c2] = std::fabs(tmp.getMass() - nominalMass);
            }
        }
        // biuld hypotheses
        std::list<std::pair<int, int>> matchedParticles = std::list<std::pair<int, int>>();
        for (int j = 0; j < n; j++) // column
        {
            double bestFit = 10000;
            std::pair<int, int> bestFitIndex = std::pair<int, int>();
            for (int i = j + 1; i < n; i++) // row
            {
                if (masses[i][j] < bestFit)
                {
                    bestFit = masses[i][j];
                    bestFitIndex.first = i;
                    bestFitIndex.second = j;
                }
            }
            if (bestFit < deltaM)
            {
                matchedParticles.push_back(std::make_pair(bestFitIndex.first, bestFitIndex.second));
            }
        }
        // take best hypothesis
        for (auto it = matchedParticles.begin(); it != matchedParticles.end();)
        {
            bool doNotIterate = false;
            for (auto jt = std::next(it); jt != matchedParticles.end();)
            {
                if (it->first == jt->second or it->first == jt->first) // other combinations are false by definition
                {
                    if (masses[it->first][it->second] < masses[jt->first][jt->second])
                    {
                        jt = matchedParticles.erase(jt);
                    }
                    else
                    {
                        doNotIterate = true;
                        it = matchedParticles.erase(it);
                        break;
                    }
                }
                else
                {
                    jt++;
                }
            }
            if (not doNotIterate)
            {
                it++;
            }
        }
        std::set<int> matchedParticlesSet = std::set<int>();
        for (auto &match : matchedParticles)
        {
            matchedParticlesSet.insert(match.first);
            matchedParticlesSet.insert(match.second);
            // pxl only knows += and not +
            pxl::Particle *tmp = new pxl::Particle(*map[particle][match.first]);
            *tmp += *map[particle][match.second];
            newLists[newName].push_back(tmp);
        }
        for (int i = 0; i < n; i++)
        {
            if (matchedParticlesSet.find(i) == matchedParticlesSet.end())
            {
                newLists[particle].push_back(map[particle][i]);
            }
        }
        // delete
        for (int i = 0; i < n; i++)
        {
            delete[] masses[i];
        }
        delete[] masses;
        map.erase(particle);
    }
    for (auto &newList : newLists)
    {
        std::vector<pxl::Particle *> newVector = std::vector<pxl::Particle *>();
        newVector.reserve(newLists.size());
        std::copy(newList.second.begin(), newList.second.end(), std::back_inserter(newVector));
        map.emplace(newList.first, newVector);
    }
}
