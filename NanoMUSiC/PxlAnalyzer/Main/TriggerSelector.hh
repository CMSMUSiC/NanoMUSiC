#ifndef TRIGGERSELECTOR
#define TRIGGERSELECTOR

#include <list>
#include <string>
#include <vector>

#include "Tools/MConfig.hh"
#include "Tools/Tools.hh"

#include "TriggerGroup.hh"

namespace pxl
{
class EventView;
class Particle;
} // namespace pxl

class TriggerSelector
{
  public:
    typedef std::list<TriggerGroup> TriggerGroupCollection;

    explicit TriggerSelector(Tools::MConfig const &cfg);
    ~TriggerSelector()
    {
    }

    // Functions to be called from outside for event selection:

    // Check if any of the required triggers (in the config file) fired in this event.
    bool passHLTrigger(const std::map<std::string, std::vector<pxl::Particle *>> &particleMap,
                       pxl::EventView *evtView) const;

    // Returns true, if a "reject" trigger has fired AND if the event topology fits.
    bool checkVeto(const std::map<std::string, std::vector<pxl::Particle *>> &particleMap,
                   pxl::EventView const *evtView) const;

    // Checks the offline pt cuts
    bool checkOfflineCut(const std::map<std::string, std::vector<pxl::Particle *>> &particleMap,
                         pxl::EventView *evtView) const;

    // Checks only trigger strings
    bool passHLTriggerWithoutOffline(const std::map<std::string, std::vector<pxl::Particle *>> &particleMap,
                                     pxl::EventView *evtView) const;

    // Check if the given set of particles fulfills any of the possible required
    // trigger topologies.
    bool passEventTopology(const std::map<std::string, int> &count_map) const;

  private:
    // Get the trigger groups from the config file.
    TriggerGroupCollection initTriggerGroups(Tools::MConfig const &cfg) const;
    bool anyTriggerFired(TriggerGroup::TriggerResults const &triggerResults) const;

    // Member variables:
    bool const m_runOnData;
    bool const m_ignoreL1;
    bool const m_ignoreHLT;
    std::string const m_triggerPrefix;
    TriggerGroupCollection const m_triggerGroups;
};

#endif /*TRIGGERSELECTOR*/
