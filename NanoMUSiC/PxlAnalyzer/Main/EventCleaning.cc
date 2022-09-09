#include "EventCleaning.hh"
#include "Main/ParticleSplittingFunctions.hh"

#include "TMath.h"

// PXL
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Tools/MConfig.hh"

EventCleaning::EventCleaning(Tools::MConfig const &cfg)
    : m_order(Tools::splitString<std::string>(cfg.GetItem<std::string>("Cleaning.Order"), true)),
      m_self_cleaning(Tools::splitString<std::string>(cfg.GetItem<std::string>("Cleaning.Self"), true)),
      m_particleUseMap(Tools::getConfigParticleMap(cfg, "use", true)),
      m_splitting_funcs(std::list<std::function<void(std::map<std::string, std::vector<pxl::Particle *>> &)>>()),
      m_merge_funcs(std::list<std::function<void(std::map<std::string, std::vector<pxl::Particle *>> &)>>()),
      m_self_cleaning_functions({{"Muon", &EventCleaning::cleanMuos},
                                 {"Ele", &EventCleaning::cleanEles},
                                 {"Gamma", &EventCleaning::cleanGams}}),
      m_deltaR_map({{"Muon", cfg.GetItem<double>("Muon.DeltaR.max")},
                    {"Ele", cfg.GetItem<double>("Ele.DeltaR.max")},
                    {"Gamma", cfg.GetItem<double>("Gamma.DeltaR.max")},
                    {"Tau", cfg.GetItem<double>("Tau.DeltaR.max")},
                    {"Jet", cfg.GetItem<double>("Jet.DeltaR.max")},
                    {"bJet", cfg.GetItem<double>("Jet.DeltaR.max")},
                    {"wJet", cfg.GetItem<double>("FatJet.DeltaR.max")},
                    {"FatJet", cfg.GetItem<double>("FatJet.DeltaR.max")}})
{
    // Extend standard use map with jet types.
    if (cfg.GetItem<bool>("Jet.BJets.use"))
        m_particleUseMap.emplace("Jet.BJets", true);
    else
        m_particleUseMap.emplace("Jet.BJets", false);
    if (cfg.GetItem<bool>("FatJet.WJets.use"))
        m_particleUseMap.emplace("FatJet.WJets", true);
    else
        m_particleUseMap.emplace("FatJet.WJets", false);

    // Make sure that non-existing particle types are removed, if they are specified in the order config option
    for (auto it = m_order.begin(); it != m_order.end();)
    {
        if (!m_particleUseMap[*it])
        {
            it = m_order.erase(it);
        }
        else
        {
            it++;
        }
    }

    // Add map of functions used to split and merge the particle map based on jet type.
    for (auto &part_name : m_order)
    {
        if (part_name == "Jet.BJets")
        {
            part_name = "bJet";
            m_splitting_funcs.push_back(Splitting::splitBjets);
            m_merge_funcs.push_back(Splitting::mergeBjets);
        }
        if (part_name == "FatJet.WJets")
        {
            part_name = "wJet";
            m_splitting_funcs.push_back(Splitting::splitWfatjets);
            m_merge_funcs.push_back(Splitting::mergeWfatjets);
        }
    }
}

void EventCleaning::cleanEvent(std::map<std::string, std::vector<pxl::Particle *>> &particleMap, bool const isRec) const
{
    for (auto &split_func : m_splitting_funcs)
    {
        split_func(particleMap);
    }
    for (auto &name : m_self_cleaning)
    {
        if (m_particleUseMap.find(name) == m_particleUseMap.end())
        {
            std::stringstream error;
            error << "[ERROR](EventCleaning::cleanEvent " << __LINE__ << ") Did not find particle '" << name
                  << "' in particle use map." << std::endl;
            throw(std::runtime_error(error.str()));
        }
        if (m_particleUseMap.at(name))
        {
            if (particleMap.find(name) == particleMap.end())
            {
                std::stringstream error;
                error << "[ERROR](EventCleaning::cleanEvent " << __LINE__ << ") Did not find particle '" << name
                      << "' in particle map." << std::endl;
                throw(std::runtime_error(error.str()));
            }
            if (m_self_cleaning_functions.find(name) != m_self_cleaning_functions.end())
                m_self_cleaning_functions.at(name)(*this, particleMap[name], isRec);
            else
            {
                std::stringstream error;
                error << "[ERROR](EventCleaning::cleanEvent " << __LINE__
                      << ") Did not find self cleaning function for '" << name << "'." << std::endl;
                throw(std::runtime_error(error.str()));
            }
        }
    }
    for (auto it = m_order.begin(); it != m_order.end(); it++)
    {
        for (auto jt = m_order.begin(); jt != it; jt++)
        {
            // use seed id
            if ((*it == "Ele" and *jt == "Gamma") or (*it == "Gamma" and *jt == "Ele"))
                removeGammaEleOverlap(particleMap[*it], particleMap[*jt], isRec);
            // standard
            removeOverlappingParticles(particleMap[*it], particleMap[*jt], m_deltaR_map.at(*it));
        }
    }
    for (auto &merge_func : m_merge_funcs)
    {
        merge_func(particleMap);
    }
}

std::vector<pxl::Particle *>::iterator EventCleaning::removeParticle(
    std::vector<pxl::Particle *> &particles, std::vector<pxl::Particle *>::iterator &particle) const
{
    // Remove the particle from the EventView!
    (*particle)->owner()->remove(*particle);

    // Erasing an object from a vector invalidates the iterator,
    // hence go one back to find the new next one during the next increment.
    particle = particles.erase(particle);
    particle--;

    return particle;
}

void EventCleaning::cleanMuos(std::vector<pxl::Particle *> &muos, bool const isRec) const
{
    std::vector<pxl::Particle *>::iterator part1;
    std::vector<pxl::Particle *>::iterator part2;

    // It is not perfectly clear, if we really want to clean muons against muons.
    // In general, there can be ambiguities in the reconstruction, but e.g. for
    // high-pt Z candidates you expect two muons to be very close in Delta R.
    // TODO: We need a study/reference here.
    for (part1 = muos.begin(); part1 != muos.end(); ++part1)
    {
        // Start inner loop with "next" particle.
        for (part2 = part1 + 1; part2 != muos.end(); ++part2)
        {
            int const ret = checkMuonOverlap(*part1, *part2);
            if (ret == -1)
            {
                part1 = removeParticle(muos, part1);

                // No use to continue inner loop once part1 is removed.
                break;
            }
            else if (ret == 1)
            {
                part2 = removeParticle(muos, part2);
            }
        }
    }
}

void EventCleaning::cleanEles(std::vector<pxl::Particle *> &eles, bool const isRec) const
{
    std::vector<pxl::Particle *>::iterator part1;
    std::vector<pxl::Particle *>::iterator part2;

    for (part1 = eles.begin(); part1 != eles.end(); ++part1)
    {
        // Start inner loop with next particle
        for (part2 = part1 + 1; part2 != eles.end(); ++part2)
        {
            int const ret = checkElectronOverlap(*part1, *part2, isRec);
            if (ret == -1)
            {
                part1 = removeParticle(eles, part1);

                // No use to continue inner loop once part1 is removed.
                break;
            }
            else if (ret == 1)
            {
                part2 = removeParticle(eles, part2);
            }
        }
    }

    // NOTE: As of end 2013, the official muon reconstruction does not handle
    // bremsstrahlung photons reliably/at all. Thus, we remove all electrons that
    // are close to a muon because they might be a fake arising from a radiated
    // photon. This should be updated if the algorithms change.

    // Remove eles in proximity to muons.
}

void EventCleaning::cleanGams(std::vector<pxl::Particle *> &gams, bool const isRec) const
{
    // Overlap removal of gammas:
    std::vector<pxl::Particle *>::iterator part1;
    std::vector<pxl::Particle *>::iterator part2;

    for (part1 = gams.begin(); part1 != gams.end(); ++part1)
    {
        for (part2 = part1 + 1; part2 != gams.end(); ++part2)
        {
            int const ret = checkGammaOverlap(*part1, *part2, isRec);
            if (ret == -1)
            {
                part1 = removeParticle(gams, part1);

                // No use to continue inner loop once part1 is removed.
                break;
            }
            else if (ret == 1)
            {
                part2 = removeParticle(gams, part2);
            }
        }
    }

    // NOTE: As of end 2013, the official muon reconstruction does not handle
    // bremsstrahlung photons reliably/at all. Thus, we remove all photons that
    // are close to a muon. This should be updated if the algorithms change.

    // Remove gammas in proximity to muons.
}

void EventCleaning::removeGammaEleOverlap(std::vector<pxl::Particle *> &gams, std::vector<pxl::Particle *> const &eles,
                                          bool const isRec) const
{
    std::vector<pxl::Particle *>::iterator gam;
    for (gam = gams.begin(); gam != gams.end(); ++gam)
    {
        std::vector<pxl::Particle *>::const_iterator ele;
        for (ele = eles.begin(); ele != eles.end(); ++ele)
        {
            if (checkParticleOverlap(*gam, *ele, m_deltaR_map.at("Ele")) and
                (not isRec or checkSeedOverlap(*gam, *ele)))
            {
                gam = removeParticle(gams, gam);

                // No use to continue inner loop once gamma is removed.
                break;
            }
        }
    }
}

void EventCleaning::removeOverlappingParticles(std::vector<pxl::Particle *> &toBeCleanedCollection,
                                               std::vector<pxl::Particle *> const &inputCollection,
                                               double const DeltaR_max) const
{
    std::vector<pxl::Particle *>::iterator toBeCleaned;
    for (toBeCleaned = toBeCleanedCollection.begin(); toBeCleaned != toBeCleanedCollection.end(); ++toBeCleaned)
    {
        std::vector<pxl::Particle *>::const_iterator inputParticle;
        for (inputParticle = inputCollection.begin(); inputParticle != inputCollection.end(); ++inputParticle)
        {
            if (checkParticleOverlap(*toBeCleaned, *inputParticle, DeltaR_max))
            {
                toBeCleaned = removeParticle(toBeCleanedCollection, toBeCleaned);

                // No use to continue inner loop once outer particle is removed.
                break;
            }
        }
    }
}

int EventCleaning::checkMuonOverlap(pxl::Particle const *paI, pxl::Particle const *paJ) const
{
    // Check particle DeltaR.
    if (checkParticleOverlap(paI, paJ, m_deltaR_map.at("Muon")))
    {
        try
        {
            // In future, get the track prbability from chi2 and ndf.
            return checkProbability(paI, paJ);
        }
        catch (std::runtime_error &e)
        {
            // So far decide using smallest chi2/ndf, probably not the best choice...
            return checkNormChi2(paI, paJ);
        }
    }
    return 0;
}

int EventCleaning::checkElectronOverlap(pxl::Particle const *paI, pxl::Particle const *paJ, bool const isRec) const
{
    // Check particle DeltaR.
    if (checkParticleOverlap(paI, paJ, m_deltaR_map.at("Ele")))
    {
        // Check if both electrons have same SC-seed (and different track) or same track (and different SuperCluster).
        // For Gen: Assume electrons to be the same.
        if (not isRec or checkSeedOverlap(paI, paJ) or checkTrack(paI, paJ))
        {
            double const eI = paI->getE();
            double const eJ = paJ->getE();

            return eI < eJ ? -1 : 1;
        }
    }
    return 0;
}

int EventCleaning::checkGammaOverlap(pxl::Particle const *paI, pxl::Particle const *paJ, bool const isRec) const
{
    // Check DeltaR.
    if (checkParticleOverlap(paI, paJ, m_deltaR_map.at("Gamma")) and (not isRec or checkSeedOverlap(paI, paJ)))
    {
        // So far decide using higher energy, probably not the best choice...
        double const eI = paI->getE();
        double const eJ = paJ->getE();

        return eI < eJ ? -1 : 1;
    }
    return 0;
}

int EventCleaning::checkNormChi2(pxl::Particle const *p1, pxl::Particle const *p2) const
{
    double const normChi2_1 = p1->getUserRecord("NormChi2");
    double const normChi2_2 = p2->getUserRecord("NormChi2");

    return normChi2_1 >= normChi2_2 ? -1 : 1;
}

int EventCleaning::checkProbability(pxl::Particle const *p1, pxl::Particle const *p2) const
{
    double const chi2_1 = p1->getUserRecord("chi2");
    double const ndof_1 = p1->getUserRecord("ndof");

    double const chi2_2 = p2->getUserRecord("chi2");
    double const ndof_2 = p2->getUserRecord("ndof");

    double const prob_1 = TMath::Prob(chi2_1, ndof_1);
    double const prob_2 = TMath::Prob(chi2_2, ndof_2);

    // Higher probability wins.
    return prob_1 < prob_2 ? -1 : 1;
}

bool EventCleaning::checkParticleOverlap(pxl::Particle const *p1, pxl::Particle const *p2,
                                         double const DeltaR_max) const
{
    pxl::LorentzVector const &vec1 = p1->getVector();
    pxl::LorentzVector const &vec2 = p2->getVector();

    double const deltaR = vec1.deltaR(&vec2);
    if (deltaR < DeltaR_max)
        return true;

    return false;
}

bool EventCleaning::checkSeedOverlap(pxl::Particle const *p1, pxl::Particle const *p2) const
{
    unsigned int const seedID1 = p1->getUserRecord("seedId");
    unsigned int const seedID2 = p2->getUserRecord("seedId");

    if (seedID1 == seedID2)
        return true;

    return false;
}

bool EventCleaning::checkTrack(pxl::Particle const *p1, pxl::Particle const *p2) const
{
    double const TrackerP1 = p1->getUserRecord("TrackerP");
    double const TrackerP2 = p2->getUserRecord("TrackerP");

    if (TrackerP1 == TrackerP2)
        return true;

    return false;
}
