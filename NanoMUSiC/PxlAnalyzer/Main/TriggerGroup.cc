#include "TriggerGroup.hh"

#include <exception>
#include <sstream>
#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Tools/Tools.hh"

using std::string;

TriggerGroup::TriggerGroup(Tools::MConfig const &cfg, string const &triggerPrefix, string const &groupName)
    : m_triggerPrefix(triggerPrefix), m_prefix("Trigger." + groupName + "."),
      m_name(cfg.GetItem<string>(m_prefix + "Name")), m_triggers(initTriggers(cfg)),
      m_require(cfg.GetItem<bool>(m_prefix + "Require")), m_reject(cfg.GetItem<bool>(m_prefix + "Reject")),
      m_cuts_map(initCuts(cfg))
{
    if (m_require and m_reject)
    {
        std::stringstream err;
        err << "Trigger group '";
        err << m_name << "' ";
        err << "cannot be 'required' and 'rejected' at the same time.";
        throw Tools::config_error(err.str());
    }
}

std::set<string> TriggerGroup::initTriggers(Tools::MConfig const &cfg) const
{
    std::vector<string> const trig_vec = Tools::splitString<string>(cfg.GetItem<string>(m_prefix + "Triggers"));

    std::set<string> const triggers(trig_vec.begin(), trig_vec.end());

    return triggers;
}

TriggerGroup::TriggerCutsCollection TriggerGroup::initCuts(Tools::MConfig const &cfg) const
{
    TriggerCutsCollection cuts_map;

    const std::vector<std::string> part_names_config = {"Mu", "E", "Tau", "Gamma", "Jet", "MET"};
    const std::vector<std::string> part_names = {"Muon", "Ele", "Tau", "Gamma", "Jet", "MET"};
    for (unsigned int i = 0; i < part_names_config.size(); i++)
    {
        TriggerCuts cuts =
            Tools::splitString<double>(cfg.GetItem<string>(m_prefix + "Cuts." + part_names_config[i], ""), true);
        if (not cuts.empty())
        {
            cuts_map[part_names[i]] = cuts;
        }
    }

    return cuts_map;
}

bool TriggerGroup::passTriggerParticles(const std::map<std::string, std::vector<pxl::Particle *>> &particleMap) const
{

    bool particles_accepted = true;
    for (auto &part : particleMap)
    {
        const auto &name = part.first;
        const auto &particles = part.second;
        particles_accepted = checkTriggerParticles(name, particles);
        if (not particles_accepted)
            break;
    }
    return particles_accepted;
}

// Check offline pt cut for leading particles
bool TriggerGroup::checkTriggerParticles(string const &particleType,
                                         std::vector<pxl::Particle *> const &particles) const
{
    TriggerCuts cuts;
    bool const cutsSet = getCuts(particleType, cuts);

    // Do we have anything to do?
    if (not cutsSet)
        return true;

    // Do we have enough partices of the given type in this event?
    if (particles.size() < cuts.size())
        return false;

    std::vector<pxl::Particle *>::const_iterator particle = particles.begin();
    for (TriggerCuts::const_iterator cut = cuts.begin(); cut != cuts.end(); cut++, particle++)
    {
        if ((*particle)->getPt() < *cut)
        {
            return false;
        }
    }

    return true;
}

// Check if we demand any cuts on each particle and then check if there are
// enough particles in the given topology.
bool TriggerGroup::checkTopology(const std::map<std::string, int> count_map) const
{
    bool all_accepted = true;
    for (auto &part : count_map)
    {
        const std::string &name = part.first;
        const int &count = part.second;
        all_accepted = count >= getNCuts(name);
        if (not all_accepted)
            break;
    }
    return all_accepted;
}

TriggerGroup::TriggerResults TriggerGroup::getTriggerResults(pxl::EventView const *triggerEvtView) const
{
    TriggerResults triggerResults;

    bool any_trigger_found = false;

    if ((*m_triggers.begin()) != "")
    {
        for (Triggers::const_iterator trigger = m_triggers.begin(); trigger != m_triggers.end(); ++trigger)
        {
            string const triggerName = *trigger;
            try
            {
                bool found = false;
                for (const auto &userRecord : triggerEvtView->getUserRecords())
                {
                    if (userRecord.first.find(triggerName) != std::string::npos)
                    {
                        triggerResults[triggerName] = true;
                        any_trigger_found = true;
                        found = true;
                        break;
                    }
                }
                if (not found)
                    triggerResults[triggerName] = false;
            }
            catch (std::runtime_error &exc)
            {
                std::cout << "Weird try catch in getTriggerResults in TriggerGroup.cc" << std::endl;
            }
        }
    }

    const pxl::UserRecords &allTriggerRecords = triggerEvtView->getUserRecords();

    for (pxl::UserRecords::const_iterator trigger = allTriggerRecords.begin(); trigger != allTriggerRecords.end();
         ++trigger)
    {
        if (trigger->first.find(m_triggerPrefix) != std::string::npos)
        {
            any_trigger_found = true;
        }
    }
    if (not any_trigger_found)
    {
        std::stringstream err;
        err << "In TriggerSelector::passHLTrigger(...): ";
        err << "None of the specified triggers in trigger group '";
        err << m_name;
        err << "' found!";
        throw std::runtime_error(err.str());
    }

    return triggerResults;
}
