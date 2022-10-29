#include "GenSelector.hpp"

#include <string>
#include <utility>

#include "Tools/Tools.hpp"

#include "Main/EventSelector.hpp"

GenSelector::GenSelector(Tools::MConfig const &cfg)
    :

      m_gen_label(cfg.GetItem<std::string>("Generator.Label")),
      m_gen_jet_label(cfg.GetItem<std::string>("Jet.Type.Gen")),

      // for cuts on single particle, what to do if particle not found
      m_ignore_missing_particle(cfg.GetItem<bool>("Generator.IgnoreMissingSingleParticle", false)),
      // Cut on generator binning.
      m_binningValue_max(cfg.GetItem<double>("Generator.BinningValue.max")),

      // Cut on generator level HT of the event.
      m_ht_min(cfg.GetItem<double>("Generator.ht.min", 0)), m_ht_max(cfg.GetItem<double>("Generator.ht.max", 0)),

      // Cuts on invariant mass of resonance particle.
      m_mass_min(cfg.GetItem<double>("Generator.Mass.min", 0)),
      m_mass_max(cfg.GetItem<double>("Generator.Mass.max", 0)),

      m_mass_use_single(cfg.GetItem<bool>("Generator.Mass.use.single")),
      m_mass_use_double(cfg.GetItem<bool>("Generator.Mass.use.double")),
      m_mass_single_types(Tools::splitString<std::string>(cfg.GetItem<std::string>("Generator.Mass.Types.single"))),
      m_mass_double_types_first(
          Tools::splitString<std::string>(cfg.GetItem<std::string>("Generator.Mass.Types.double.first"), true)),
      m_mass_double_types_second(
          Tools::splitString<std::string>(cfg.GetItem<std::string>("Generator.Mass.Types.double.second"), true)),

      // Cuts on transverse momentum of resonance particle.
      m_pt_min(cfg.GetItem<double>("Generator.pt.min", 0)), m_pt_max(cfg.GetItem<double>("Generator.pt.max", 0)),
      m_pt_use_single(cfg.GetItem<bool>("Generator.pt.use.single")),
      m_pt_use_double(cfg.GetItem<bool>("Generator.pt.use.double")),
      m_pt_single_types(Tools::splitString<std::string>(cfg.GetItem<std::string>("Generator.pt.Types.single"))),
      m_pt_double_types_first(
          Tools::splitString<std::string>(cfg.GetItem<std::string>("Generator.pt.Types.double.first"), true)),
      m_pt_double_types_second(
          Tools::splitString<std::string>(cfg.GetItem<std::string>("Generator.pt.Types.double.second"), true)),
      // cuts for gamma cleaning
      m_gamma_use(cfg.GetItem<bool>("Generator.Gamma.use")), m_gammaDY_use(cfg.GetItem<bool>("Generator.GammaDY.use")),
      m_gammaZy_use(cfg.GetItem<bool>("Generator.GammaZy.use")),
      m_gamma_cutmode(cfg.GetItem<std::string>("Generator.Gamma.cutmode")),
      m_gamma_statuses(Tools::splitString<int>(cfg.GetItem<std::string>("Generator.Gamma.statuses"))),
      m_gamma_dR_max(cfg.GetItem<double>("Generator.Gamma.dR.max")),
      m_gamma_genPtCut(cfg.GetItem<double>("Generator.Gamma.genPtCut")),
      m_gamma_gen_partner_id(Tools::splitString<int>(cfg.GetItem<std::string>("Generator.Gamma.PartnerIds")))

{
}

bool GenSelector::passGeneratorCuts(pxl::EventView *EvtView) const
{
    bool accept_binning = passBinningCuts(EvtView);
    bool accept_ht = passHtCuts(EvtView);

    // std::vector< pxl::Particle* > genParts = getGenParticles( EvtView );
    std::vector<pxl::Particle *> genParts = getGenParticles(EvtView, m_gen_label);

    bool accept_mass = passMassCuts(genParts);
    bool accept_pt = passTransverseMomentumCuts(genParts);

    std::vector<pxl::Particle *> genJets = getGenParticles(EvtView, m_gen_jet_label);
    bool accept_gammaDY = passDYGammaCuts(EvtView, genParts, genJets);
    bool accept_gammaZy = passZyGammaCuts(EvtView, genParts, genJets);

    return accept_binning and accept_ht and accept_mass and accept_pt and accept_gammaDY and accept_gammaZy;
}

bool GenSelector::passBinningCuts(pxl::EventView const *EvtView) const
{
    // check binning value
    if (m_binningValue_max > 0 and EvtView->getUserRecord("binScale").toDouble() > m_binningValue_max)
    {
        return false;
    }
    return true;
}

bool GenSelector::passMassCuts(std::vector<pxl::Particle *> genParts) const
{

    // Check if a generator invariant mass cut is being applied.
    if (m_mass_min <= 0 and m_mass_max <= 0)
        return true;

    pxl::LorentzVector genCutObject = getGenCutObject(std::move(genParts), "mass");

    if (m_mass_min <= 0 and m_mass_max > 0)
    {
        return genCutObject.getMass() <= m_mass_max;
    }
    else if (m_mass_min > 0 and m_mass_max <= 0)
    {
        return genCutObject.getMass() >= m_mass_min;
    }
    else
    { // m_mass_min > 0 and m_mass_max > 0
        return (genCutObject.getMass() >= m_mass_min && genCutObject.getMass() <= m_mass_max);
    }
}

bool GenSelector::passHtCuts(pxl::EventView const *EvtView) const
{
    // Check if a generator HT cut is being applied.
    if (m_ht_min <= 0 and m_ht_max <= 0)
        return true;

    // To-Do: Not all samples have been re-skimmed with the HT variable.
    // Remove try-catch-throw' statement once this is the case.
    if (EvtView->hasUserRecord("genHT"))
    {
        double ht = EvtView->getUserRecord("genHT");
        // If LHE information cannot be accessed in the skimmer, HT is
        // set to '-1' for the event.
        if (ht >= 0)
        {
            if (m_ht_min <= 0 and m_ht_max > 0)
            {
                return ht <= m_ht_max;
            }
            else if (m_ht_min > 0 and m_ht_max <= 0)
            {
                return ht >= m_ht_min;
            }
            else if (m_ht_min <= 0 and m_ht_max <= 0)
            {
                return true;
            }
            else
            {
                // m_ht_min > 0 and m_ht_max > 0
                return (ht >= m_ht_min && ht <= m_ht_max);
            }
        }
        else
        {
            // Cut cannot be applied.
            return true;
        }
    }
    else
    {
        std::cout << "ERROR: GenSelector::passHtCuts: 'genHT' variable not available in this sample." << std::endl;
        // Cut cannot be applied, terminate the program.
        throw;
    }
}

bool GenSelector::passTransverseMomentumCuts(std::vector<pxl::Particle *> genParts) const
{

    // Check if a generator pt cut is being applied.
    if (m_pt_min <= 0 and m_pt_max <= 0)
        return true;

    pxl::LorentzVector genCutObject;
    try
    {
        genCutObject = getGenCutObject(std::move(genParts), "pt");
    }
    catch (std::runtime_error &exception)
    {
        if (m_ignore_missing_particle)
        {
            std::cout << " Caught exception : " << exception.what()
                      << " : Continuing anyway due to flag m_ignore_missing_particle" << std::endl;
            return true;
        }
        throw exception;
    }

    // pxl::LorentzVector genCutObject =  getGenCutObject( std::move(genParts), "pt" );

    if (m_pt_min <= 0 and m_pt_max > 0)
    {
        return genCutObject.getPt() <= m_pt_max;
    }
    else if (m_pt_min > 0 and m_pt_max <= 0)
    {
        return genCutObject.getPt() >= m_pt_min;
    }
    else
    { // m_pt_min > 0 and m_pt_max > 0
        return (genCutObject.getPt() >= m_pt_min && genCutObject.getPt() <= m_pt_max);
    }
}

bool GenSelector::CheckNumberOfParticles(pxlParticles const &s3_particlesSelected) const
{

    if (s3_particlesSelected.size() < 2)
    {
        for (pxlParticles::const_iterator part = s3_particlesSelected.begin(); part != s3_particlesSelected.end();
             ++part)
        {
            std::cerr << "ID=" << (*part)->getUserRecord("id").toInt32()
                      << " mother=" << (*part)->getUserRecord("mother_id").toInt32() << std::endl;
        }
        throw std::length_error("Can't build resonance particle with less than 2 particles.");
    }
    else if (s3_particlesSelected.size() > 3)
    {
        for (pxlParticles::const_iterator part = s3_particlesSelected.begin(); part != s3_particlesSelected.end();
             ++part)
        {
            std::cerr << "ID=" << (*part)->getUserRecord("id").toInt32()
                      << " mother=" << (*part)->getUserRecord("mother_id").toInt32() << std::endl;
        }
        throw std::length_error("Can't build resonance particle with more than 2 particles.");
    }
    return true;
}

// Get all particles from gen view, which match the gen label.
std::vector<pxl::Particle *> GenSelector::getGenParticles(pxl::EventView *GenEvtView, std::string gen_label) const
{
    // get all particles
    std::vector<pxl::Particle *> allparticles;
    GenEvtView->getObjectsOfType<pxl::Particle>(allparticles);
    std::vector<pxl::Particle *> genParticles;
    for (std::vector<pxl::Particle *>::const_iterator part = allparticles.begin(); part != allparticles.end(); ++part)
    {
        std::string name = (*part)->getName();
        // if( name == m_gen_label ) genParticles.push_back( *part );
        if (name == gen_label)
            genParticles.push_back(*part);
    }
    return genParticles;
}

// Get the object we want to apply the gen cut on.
// type determines which particles are selected to form the gen cut object options: "mass" , "pt"
pxl::LorentzVector GenSelector::getGenCutObject(std::vector<pxl::Particle *> genParts, const std::string &type) const
{
    std::vector<std::string> single_types;
    std::vector<std::string> double_types_first;
    std::vector<std::string> double_types_second;
    bool use_single = false;
    bool use_double = false;
    if (type == "mass")
    {

        single_types = m_mass_single_types;
        double_types_first = m_mass_double_types_first;
        double_types_second = m_mass_double_types_second;
        //~ for(auto type : m_mass_double_types_first ) std::cout << type << std::endl;
        //~ for(auto type : double_types_first ) std::cout << type << std::endl;
        use_single = m_mass_use_single;
        use_double = m_mass_use_double;
    }
    else if (type == "pt")
    {
        single_types = m_pt_single_types;
        double_types_first = m_pt_double_types_first;
        double_types_second = m_pt_double_types_second;
        use_single = m_pt_use_single;
        use_double = m_pt_use_double;
    }

    auto pdg_id_map = Tools::pdg_id_type_map();
    int pdgId = 0;
    pxl::Particle *firstPart = 0;
    pxl::Particle *secondPart = 0;
    //~ for(auto type : double_types_first ) std::cout << type << std::endl;
    //~ for( auto type : double_types_second ) std::cout << type << std::endl;
    //~ std::cout << "size double" << double_types_first.size() << std::endl;
    //~ std::cout << "size " << genParts.size() << std::endl;
    for (std::vector<pxl::Particle *>::const_iterator part = genParts.begin(); part != genParts.end(); ++part)
    {
        pdgId = (*part)->getPdgNumber();
        //~ std::cout <<pdg_id_map[ abs( pdgId ) ] <<std::endl;
        if (use_single and
            std::find(single_types.begin(), single_types.end(), pdg_id_map[abs(pdgId)]) != single_types.end())
            return (*part)->getVector();
        if (use_double and firstPart != 0 and secondPart == 0 and
            std::find(double_types_second.begin(), double_types_second.end(), pdg_id_map[abs(pdgId)]) !=
                double_types_second.end())
        {
            secondPart = *part;
        }
        if (use_double and firstPart == 0 and
            std::find(double_types_first.begin(), double_types_first.end(), pdg_id_map[abs(pdgId)]) !=
                double_types_first.end())
        {
            firstPart = *part;
        }
        // gen particles are sorted from left to right in Feynman graph. Stop if we found to particles
        if (firstPart != 0 and secondPart != 0)
            break;
    }
    if (use_single and !use_double)
    {
        throw std::runtime_error("No single particle found of requested type for Gen level cut.");
    }
    if (firstPart == 0 and secondPart == 0)
    {
        throw std::length_error("Can't build resonance particle for gen cuts with less than 2 particles.");
    }
    pxl::LorentzVector sum;
    sum += firstPart->getVector();
    sum += secondPart->getVector();
    return sum;
}

// sar change
bool GenSelector::passGammaCuts(pxl::EventView const *EvtView, std::vector<pxl::Particle *> genParts,
                                std::vector<pxl::Particle *> genJets) const
{
    // check if gamma ucuts should be applied
    if (!m_gamma_use)
    {
        return true;
    }
    auto particleLists = EventSelector::getGenParticleLists(EvtView);
    std::vector<std::string> found_gamma_types;
    bool found_match = false;

    for (auto &gam : particleLists["Gamma"])
    {
        // don't consider very low enegetic photons?//sar change
        // if( gam->getPt() < m_gamma_genPtCut) continue;
        // if( gam->getPt() < 10.0) continue;
        //  stop once we have found a partner for the highest energetic valid gen muon
        if (found_match)
            break;
        // check if the candidate gamma has mothers
        if (gam->getMothers().empty())
        {
            continue;
        }
        // make sure candidate gamma has one of the allowed codes
        if (std::find(m_gamma_statuses.begin(), m_gamma_statuses.end(),
                      std::abs(gam->getUserRecord("Status").toInt32())) == m_gamma_statuses.end())
        {
            continue;
        }

        // check if we find a valid mother particle
        bool valid_mother = false;
        for (auto mother_relative : gam->getMothers())
        {
            auto mother = dynamic_cast<pxl::Particle *>(mother_relative);
            // sar change
            if (mother->getPdgNumber() <= 9 || mother->getPdgNumber() == 21 || mother->getPdgNumber() == 22 ||
                mother->getPdgNumber() == 23 || mother->getPdgNumber() == 2212)
            {
                // if( mother->getPdgNumber() <= 9 ||  mother->getPdgNumber() == 21 || mother->getPdgNumber() == 23 ||
                // mother->getPdgNumber() == 2212 ){ if( mother->getPdgNumber() == 23 ){
                // if( mother->getPdgNumber() <= 9 ||  mother->getPdgNumber() == 21 || mother->getPdgNumber() == 22 ||
                // mother->getPdgNumber() == 2212 ){
                valid_mother = true;
                break;
            }
        }
        if (!valid_mother)
            continue;

        // Try to find a partner with one of the pdg types defined in the config file
        for (auto &genP : genParts)
        {
            // skip if we compare particle to itself
            if (genP->getId() == gam->getId())
            {
                continue;
            }
            // accept only status 23 particle as dR partners
            if (genP->getUserRecord("Status").toInt32() != 23)
            {
                continue;
            }

            // Check if gen particle is in list of partners to check dR to
            if (std::find(m_gamma_gen_partner_id.begin(), m_gamma_gen_partner_id.end(),
                          std::abs(genP->getPdgNumber())) == m_gamma_gen_partner_id.end())
            {
                continue;
            }

            double dR = std::abs(gam->getVector().deltaR(genP->getVector()));
            found_match = true;
            if (m_gamma_cutmode.compare("greater") == 0)
            {
                if (dR < m_gamma_dR_max)
                {
                    return false;
                }
            }
            else if (m_gamma_cutmode.compare("smaller") == 0)
            {
                if (dR > m_gamma_dR_max)
                {
                    return false;
                }
            }
        }
        // if we found no match for our gamma (sometimes no status 23 quarks exist)
        // we try genJets
        if (!found_match)
        {
            if (!m_has_gamma_quark_partner)
            {
                return true;
            }
            for (auto &genJet : genJets)
            {
                double dR = std::abs(gam->getVector().deltaR(genJet->getVector()));
                if (m_gamma_cutmode.compare("greater") == 0)
                {
                    if (dR < m_gamma_dR_max)
                    {
                        return false;
                    }
                }
                else if (m_gamma_cutmode.compare("smaller") == 0)
                {
                    if (dR > m_gamma_dR_max)
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

// The DY sample remove events with one photon from ISR or Z at least
bool GenSelector::passDYGammaCuts(pxl::EventView const *EvtView, std::vector<pxl::Particle *> genParts,
                                  std::vector<pxl::Particle *> genJets) const
{
    // check if gamma ucuts should be applied
    if (!m_gammaDY_use)
    {
        return true;
    }

    // new check test
    auto particleLists = EventSelector::getGenParticleLists(EvtView);
    std::vector<std::string> found_gamma_types;
    bool photon_likeZGamma = false;
    int pdgId = 0;

    // for( auto &gam  : particleLists[ "Gamma" ] ){
    for (std::vector<pxl::Particle *>::const_iterator part = genParts.begin(); part != genParts.end(); ++part)
    {
        // stop once we have found a photon satisfying conditions
        if (photon_likeZGamma)
            break;
        if ((*part)->getUserRecord("Status").toInt32() == 1)
        {
            pdgId = (*part)->getPdgNumber();
            if (pdgId != 22)
                continue;
            if ((*part)->getPt() < m_gamma_genPtCut)
                continue;
            if (!((*part)->getUserRecord("isPromptFinalState")))
            {
                continue; // photon_likeZGamma = true;
            }
            for (auto mother_relative : (*part)->getMothers())
            {
                auto mother = dynamic_cast<pxl::Particle *>(mother_relative);
                if (std::abs(mother->getPdgNumber()) <= 21 || mother->getPdgNumber() == 22 ||
                    mother->getPdgNumber() == 23 || mother->getPdgNumber() == 2212)
                {
                    photon_likeZGamma = true;
                    // Try to find a partner with one of the pdg types defined in the config file
                    for (auto &genP : genParts)
                    {
                        // skip if we compare particle to itself
                        if (genP->getId() == (*part)->getId())
                        {
                            continue;
                        }
                        // accept only status 23 particle as dR partners
                        if (genP->getUserRecord("Status").toInt32() != 23)
                        {
                            continue;
                        }

                        // Check if gen particle is in list of partners to check dR to
                        int partnerPdgId = std::abs(genP->getPdgNumber());
                        if (partnerPdgId <= 9 || partnerPdgId == 21)
                        {

                            double dR = std::abs((*part)->getVector().deltaR(genP->getVector()));

                            if (dR < 0.4)
                            {
                                photon_likeZGamma = false;
                                break;
                            }
                        }
                    }
                    if (photon_likeZGamma)
                        break;
                }
            }
        }
    }
    return !photon_likeZGamma;
}

// sar change
// The Zy sample must contain one photon from ISR or Z at least
bool GenSelector::passZyGammaCuts(pxl::EventView const *EvtView, std::vector<pxl::Particle *> genParts,
                                  std::vector<pxl::Particle *> genJets) const
{
    // check if gamma ucuts should be applied
    if (!m_gammaZy_use)
    {
        return true;
    }

    // new check test
    auto particleLists = EventSelector::getGenParticleLists(EvtView);
    std::vector<std::string> found_gamma_types;
    bool photon_likeZGamma = false;
    int pdgId = 0;

    // for( auto &gam  : particleLists[ "Gamma" ] ){
    for (std::vector<pxl::Particle *>::const_iterator part = genParts.begin(); part != genParts.end(); ++part)
    {
        // stop once we have found a photon satisfying conditions
        if (photon_likeZGamma)
            break;
        // don't consider very low enegetic photons?//sar change
        if ((*part)->getUserRecord("Status").toInt32() == 1)
        {
            pdgId = (*part)->getPdgNumber();
            if (pdgId != 22)
                continue;
            if ((*part)->getPt() < m_gamma_genPtCut)
                continue;
            // if( (*part)->getPt() < 10.0) continue;
            if (!((*part)->getUserRecord("isPromptFinalState")))
            {
                continue; // photon_likeZGamma = true;
            }
            for (auto mother_relative : (*part)->getMothers())
            {
                auto mother = dynamic_cast<pxl::Particle *>(mother_relative);
                if (std::abs(mother->getPdgNumber()) <= 21 || mother->getPdgNumber() == 22 ||
                    mother->getPdgNumber() == 23 || mother->getPdgNumber() == 2212)
                {
                    photon_likeZGamma = true;
                    // valid_mother = true;
                    //  Try to find a partner with one of the pdg types defined in the config file
                    for (auto &genP : genParts)
                    {
                        // skip if we compare particle to itself
                        if (genP->getId() == (*part)->getId())
                        {
                            continue;
                        }
                        // accept only status 23 particle as dR partners
                        if (genP->getUserRecord("Status").toInt32() != 23)
                        {
                            continue;
                        }

                        // Check if gen particle is in list of partners to check dR to
                        int partnerPdgId = std::abs(genP->getPdgNumber());
                        if (partnerPdgId <= 9 || partnerPdgId == 21)
                        {

                            double dR = std::abs((*part)->getVector().deltaR(genP->getVector()));

                            if (dR < 0.4)
                            {
                                photon_likeZGamma = false;
                                break;
                            }
                        }
                    }
                    if (photon_likeZGamma)
                        break;
                }
            }
        }
    }
    return photon_likeZGamma;
}
