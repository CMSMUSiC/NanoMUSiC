#ifndef EventSelector_hh
#define EventSelector_hh

/*

This class performs the Final Event Selection based on Final Cuts and Trigger
Decision.

*/
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Tools/MConfig.hh"
#include "TriggerSelector.hh"
#include <string>

#include "EleSelector.hh"
#include "FatJetSelector.hh"
#include "GammaSelector.hh"
#include "JetSelector.hh"
#include "METSelector.hh"
#include "MuonSelector.hh"
#include "ObjectSelector.hh"
#include "TauSelector.hh"

#include "EffectiveArea.hh"
#include "EventCleaning.hh"
#include "GenRecNameMap.hh"
#include "GenSelector.hh"

#include "JetTypeWriter.hh"
#include "OldNameMapper.hh"

class EventSelector
{
  public:
    EventSelector(const Tools::MConfig &config);

    // Destruktor
    ~EventSelector();
    // main method to perform the selection
    void performSelection(pxl::EventView *RecEvtView, pxl::EventView *GenEvtView, pxl::EventView *TrigEvtView,
                          pxl::EventView *FilterView, const bool isSyst = false);
    // function to perform only kinematics (pt, eta) selection
    void performKinematicsSelection(pxl::EventView *EvtView, const bool isSyst = false);
    // function to perform trigger selection MUSiC style
    void performTriggerSelection(pxl::EventView *RecEvtView, pxl::EventView *TrigEvtView);
    // Only Offline Cuts
    void performOfflineTriggerSelection(pxl::EventView *RecEvtView, pxl::EventView *TrigEvtView) const;
    // Get a map with particle lists sorted by part type and ordered by pt
    // std::map< std::string, std::vector< pxl::Particle* > > getGenParticleLists ( const pxl::EventView* EvtView )
    // const;
    static std::map<std::string, std::vector<pxl::Particle *>> getGenParticleLists(const pxl::EventView *EvtView);
    std::map<std::string, std::vector<pxl::Particle *>> getParticleLists(
        const pxl::EventView *EvtView, bool selectedOnly = false,
        std::list<std::function<void(std::map<std::string, std::vector<pxl::Particle *>> &)>> functions =
            std::list<std::function<void(std::map<std::string, std::vector<pxl::Particle *>> &)>>()) const;
    // Get a map of counts for a map of particle lists as created by getParticleLists
    std::map<std::string, int> getParticleCountMap(
        const std::map<std::string, std::vector<pxl::Particle *>> &particleLists) const;

    void removeOverlaps(pxl::EventView *RecEvtView) const;
    TriggerSelector const &getTriggerSelector() const
    {
        return m_triggerSelector;
    }
    // void checkOrder( std::vector< pxl::Particle* > const &particles ) const;
    static void checkOrder(std::vector<pxl::Particle *> const &particles);
    // selectors may be accessed as public objects. This allows to run different
    // selection later in ypur own analysis.
    std::map<std::string, int> &getFilterMap()
    {
        return m_all_filters_map;
    }

    const ObjectSelector *getObjectSelector(const std::string &object) const;
    const GenSelector *getGenSelector() const
    {
        return &m_gen_selector;
    };

  private:
    // objects
    // Map with .use option for each particle type
    std::map<std::string, bool> particleUseMap;
    // map with .ID.Tag option for each type
    std::map<std::string, bool> particleTagMap;
    // map with .Rho.Label option for each particle type
    std::map<std::string, std::string> particleRhoLabelMap;
    // map with ID.Type option for each particle type
    std::map<std::string, std::string> particleIDTypeMap;

    OldNameMapper oldNameMap;
    EleSelector m_ele_selector;
    MuonSelector m_muo_selector;
    TauSelector m_tau_selector;
    GammaSelector m_gamma_selector;
    JetSelector m_jet_selector;
    FatJetSelector m_fatjet_selector;
    METSelector m_met_selector;
    std::map<std::string, ObjectSelector *> selector_map;

    // Methods;
    void initSelectorMap(const Tools::MConfig &cfg);
    std::map<std::string, double> getRhoMap(pxl::EventView *EvtView);
    bool passFilterSelection(pxl::EventView *EvtView);
    // perform cuts on Particle Level
    // ATTENTION: changes particle vector!
    void applyCutsOnObject(std::vector<pxl::Particle *> &objects, const std::string &partName, double objectRho,
                           const std::string &idType, const bool isSyst);

    void applyKinematicCuts(std::vector<pxl::Particle *> &objects, const std::string &partName, const bool isSyst);
    // Perform "cuts" on topology.
    bool applyCutsOnTopology(std::vector<pxl::Particle *> const &muons, std::vector<pxl::Particle *> const &eles,
                             std::vector<pxl::Particle *> const &taus, std::vector<pxl::Particle *> const &gammas,
                             std::vector<pxl::Particle *> const &jets, std::vector<pxl::Particle *> const &mets);

    // set user records with scale factors
    void setScaleFactors(std::vector<pxl::Particle *> &objects, const std::string &partName);

    /////////////////////////////////////////////////////////////////////////////
    ////////////////////////// Selection variables: /////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    // General selection:

    bool const m_data;
    // Don't check for overlaps?
    bool const m_ignoreOverlaps;
    // Running on FASTSIM?
    bool const m_runOnFastSim;
    // Use the music triiger system?
    bool const m_useTrigger;

    // Generator selection:
    bool const m_gen_use;
    GenSelector const m_gen_selector;

    // Filters
    std::string const m_filterSet_name;
    std::vector<std::string> const m_filterSet_genList;
    std::vector<std::string> const m_filterSet_recList;
    std::vector<std::string> const m_filterHLT_recList;
    std::map<std::string, int> m_all_filters_map;
    bool const m_filter_bookkeeping;

    std::string const m_rho_use;

    // Muons:
    bool const m_muo_use;
    bool const m_muo_idtag;
    std::string const m_muo_rho_label;

    // Electrons:
    bool const m_ele_use;
    bool const m_ele_idtag;
    std::string const m_ele_rho_label;

    // Taus:
    bool const m_tau_use;

    // Photons:
    bool const m_gam_use;
    std::string const m_gam_rho_label;

    // jet
    bool const m_jet_ID_use;

    // MET:
    bool const m_met_use;

    /////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// Other variables: /////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    // Class mapping Gen and Rec particle names.
    GenRecNameMap const m_gen_rec_map;

    EventCleaning const m_eventCleaning;
    TriggerSelector const m_triggerSelector;

    JetTypeWriter m_typeWriter;
};
#endif
