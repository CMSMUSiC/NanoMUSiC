#ifndef TRIGGERGROUP
#define TRIGGERGROUP

#include <map>
#include <set>
#include <string>
#include <vector>

#include "Tools/MConfig.hpp"

namespace pxl
{
class EventView;
class Particle;
} // namespace pxl

class TriggerGroup
{
  public:
    typedef std::vector<double> TriggerCuts;
    typedef std::map<std::string, TriggerCuts> TriggerCutsCollection;
    typedef std::set<std::string> Triggers;
    typedef std::map<std::string, bool> TriggerResults;

    TriggerGroup(Tools::MConfig const &cfg, std::string const &triggerPrefix, std::string const &groupName);
    ~TriggerGroup()
    {
    }

    // Getters:
    bool const &getRequire() const
    {
        return m_require;
    }
    bool const &getReject() const
    {
        return m_reject;
    }

    // Check if there are any pt-cuts set for the given particle type and
    // store it in the given variable.
    bool const getCuts(std::string const &particleType, TriggerCuts &cuts) const
    {
        TriggerCutsCollection::const_iterator found = m_cuts_map.find(particleType);

        if (found != m_cuts_map.end())
        {
            cuts = (*found).second;
            return true;
        }
        else
            return false;
    }

    // Get the number of pt-cuts for the given particle type.
    // If no cuts are set return -1.
    // This is used for topological selection.
    int getNCuts(std::string const &particleType) const
    {
        TriggerCutsCollection::const_iterator found = m_cuts_map.find(particleType);

        if (found != m_cuts_map.end())
        {
            return static_cast<int>((*found).second.size());
        }
        else
            return -1;
    }

    // Functions to be called from outside for event selection:

    TriggerResults getTriggerResults(pxl::EventView const *evtView) const;

    // Check whether the given set of particles fulfills the topological
    // requirements of the given trigger group. I.e. do we have enough particles of
    // each type.
    bool checkTopology(const std::map<std::string, int> count_map) const;
    // Check if there are as many particles (of each type) in the event as required
    // by the trigger(s) and check if they fulfill the pt requirements.
    bool passTriggerParticles(const std::map<std::string, std::vector<pxl::Particle *>> &particleMap) const;
    // Get name
    std::string getName() const
    {
        return m_name;
    };

  private:
    // Each trigger group (in the config file) can define one oder more
    // triggers. All of these triggers must be of the "same type", e.g.
    // trigger group 1 could define several SingleMu triggers and accordingly
    // one set of pt-cuts for this trigger.
    Triggers initTriggers(Tools::MConfig const &cfg) const;

    // There are six types of particles that are considered at the moment. For each
    // trigger group, look if there is one or more pt cuts set for each of these
    // particles. If they are, these cuts will be used to select the event. The
    // set of these cuts also defines the topological selection of the events. I.e.
    // if there a two cuts for electrons in the config file, there must be at least
    // two electrons in the event to be accepted.
    // The topology selection is implicitly done by the number of cuts for each
    // particle.
    TriggerCutsCollection initCuts(Tools::MConfig const &cfg) const;

    // Check the pt for all required particles.
    bool checkTriggerParticles(std::string const &particleType, std::vector<pxl::Particle *> const &particles) const;

    // Member variables:
    std::string const m_triggerPrefix;
    std::string const m_prefix;
    std::string const m_name;
    Triggers const m_triggers;
    bool const m_require;
    bool const m_reject;
    TriggerCutsCollection const m_cuts_map;
};

#endif /*TRIGGERGROUP*/
