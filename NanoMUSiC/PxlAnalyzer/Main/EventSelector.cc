#include "EventSelector.hh"
#include "Tools/PXL/Sort.hh"
#include "Tools/Tools.hh"
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace pxl;
using namespace std;

//--------------------Constructor-----------------------------------------------------------------

EventSelector::EventSelector(const Tools::MConfig &cfg)
    : // initialize config object maps
      particleUseMap(Tools::getConfigParticleMap(cfg, "use", true)),
      particleTagMap(Tools::getConfigParticleMap(cfg, "ID.Tag", true)),
      particleRhoLabelMap(Tools::getConfigParticleMap(cfg, "Rho.Label", std::string("dummy"))),
      particleIDTypeMap(Tools::getConfigParticleMap(cfg, "ID.Type", std::string("dummy"))),

      // oldNameMapper
      oldNameMap(),

      // initalize object selectors
      m_ele_selector(cfg, &oldNameMap), m_muo_selector(cfg, &oldNameMap), m_tau_selector(cfg, &oldNameMap),
      m_gamma_selector(cfg, &oldNameMap), m_jet_selector(cfg, &oldNameMap), m_fatjet_selector(cfg, &oldNameMap),
      m_met_selector(cfg, &oldNameMap), selector_map({{}}),

      m_data(cfg.GetItem<bool>("General.RunOnData")), m_ignoreOverlaps(cfg.GetItem<bool>("General.IgnoreOverlaps")),

      // When running on data, FastSim is always false!
      m_runOnFastSim(not m_data and cfg.GetItem<bool>("General.FastSim")),
      m_useTrigger(cfg.GetItem<bool>("General.UseTriggers")),

      // Generator selection:
      m_gen_use(cfg.GetItem<bool>("Generator.use")), m_gen_selector(cfg),

      // Filters:
      m_filterSet_name(cfg.GetItem<string>("FilterSet.Name")),
      m_filterSet_genList(Tools::splitString<string>(cfg.GetItem<string>("FilterSet.GenList"), true)),
      m_filterSet_recList(Tools::splitString<string>(cfg.GetItem<string>("FilterSet.RecList"), true)),
      m_filterHLT_recList(Tools::splitString<string>(cfg.GetItem<string>("FilterSet.RecList_HLT"), true)),
      m_filter_bookkeeping(cfg.GetItem<bool>("FilterSet.FillBookkeepingVector")),

      // rho default
      m_rho_use(cfg.GetItem<string>("Rho.Label", cfg.GetItem<string>("Ele.Rho.Label"))),

      // Muons
      m_muo_use(cfg.GetItem<bool>("Muon.use")), m_muo_idtag(cfg.GetItem<bool>("Muon.IDTag", false)),
      m_muo_rho_label(cfg.GetItem<string>("Muon.Rho.Label")),

      // Electrons:
      m_ele_use(cfg.GetItem<bool>("Ele.use")), m_ele_idtag(cfg.GetItem<bool>("Ele.IdTag", false)),
      m_ele_rho_label(cfg.GetItem<string>("Ele.Rho.Label")),

      // Taus
      m_tau_use(cfg.GetItem<bool>("Tau.use")),

      // Photons:
      m_gam_use(cfg.GetItem<bool>("Gamma.use")), m_gam_rho_label(cfg.GetItem<string>("Gamma.Rho.Label")),
      // jet
      m_jet_ID_use(cfg.GetItem<bool>("Jet.ID.use")),

      // MET:
      m_met_use(cfg.GetItem<bool>("MET.use")),

      m_gen_rec_map(cfg),

      m_eventCleaning(cfg), m_triggerSelector(cfg), m_typeWriter(cfg)
{
    selector_map.emplace("Ele", &m_ele_selector);
    selector_map.emplace("Muon", &m_muo_selector);
    selector_map.emplace("Tau", &m_tau_selector);
    selector_map.emplace("Gamma", &m_gamma_selector);
    selector_map.emplace("Jet", &m_jet_selector);
    selector_map.emplace("FatJet", &m_fatjet_selector);
    selector_map.emplace("MET", &m_met_selector);
    // we save all filters in filter event view. Add all selected filters into one vector and loop
    std::vector<std::string> all_filters = std::vector<std::string>(m_filterHLT_recList);
    all_filters.insert(all_filters.end(), m_filterSet_recList.begin(), m_filterSet_recList.end());
    all_filters.insert(all_filters.end(), m_filterSet_genList.begin(), m_filterSet_genList.end());
    for (std::string filter : all_filters)
    {
        m_all_filters_map.insert(std::pair<std::string, int>(filter, 0));
    }
}

//--------------------Destructor-----------------------------------------------------------------

EventSelector::~EventSelector()
{
}

std::map<std::string, double> EventSelector::getRhoMap(pxl::EventView *EvtView)
{
    std::map<std::string, double> rhoMap;
    for (auto labelpair : particleRhoLabelMap)
    {
        if (!labelpair.second.empty())
        {
            rhoMap.emplace(labelpair.first, EvtView->getUserRecord(labelpair.second));
        }
        else
        {
            rhoMap.emplace(labelpair.first, -1.);
        }
    }
    return rhoMap;
}

// ------------------ Check if the given filters which are saved in one EventView -------------------
bool EventSelector::passFilterSelection(pxl::EventView *EvtView)
{
    bool passFilters = true;
    for (auto &element : m_all_filters_map)
    {
        // reset value
        element.second = 0;
        std::string filter = element.first;
        // if there is no filter by this name we can not say anything about it!
        if (!EvtView->hasUserRecord(filter))
            continue;
        // check filter result
        // - true = event passes filter requirements -> keep event
        // - false = event does not pass filter requirements -> reject event
        bool filterResult = EvtView->getUserRecord(filter).toBool();
        if (!filterResult)
        {
            if (m_filter_bookkeeping)
            {
                element.second = -1;
                passFilters = false;
            }
            else
            {
                return false;
            }
        }
        else
        {
            element.second = 1;
        }
    }
    return passFilters;
}

void EventSelector::applyKinematicCuts(std::vector<pxl::Particle *> &objects, const std::string &partName,
                                       const bool isSyst)
{
    vector<pxl::Particle *> objectsAfterCut;
    std::string systPrefix = "";
    if (isSyst)
        systPrefix = "syst";
    //~ for( auto& object : objects) {
    for (vector<Particle *>::const_iterator object = objects.begin(); object != objects.end(); ++object)
    {
        if (selector_map[partName]->passKinematics(*object, isSyst))
        {
            objectsAfterCut.push_back(*object);
            if ((!isSyst and (*object)->hasUserRecord("systIDpassed") and
                 (*object)->getUserRecord("systIDpassed").toBool()) or
                isSyst)
            {
                (*object)->setUserRecord(systPrefix + "IDpassed", true);
            }
        }
        else if (particleTagMap[partName])
        {
            (*object)->setUserRecord(systPrefix + "IDpassed", false);
        }
        else
        {
            (*object)->owner()->remove(*object);
        }
    }

    // ATTENTION: changing object-vector!
    if (!particleTagMap[partName])
        objects = objectsAfterCut;
}

//--------------------Apply cuts on Particles-----------------------------------------------------------------
// ATTENTION: particle-vector is changed if object.Id.Tag = 0
void EventSelector::applyCutsOnObject(std::vector<pxl::Particle *> &objects, const std::string &partName,
                                      double objectRho, const std::string &idType, const bool isSyst)
{
    vector<pxl::Particle *> objectsAfterCut;
    std::string systPrefix = "";
    if (isSyst)
        systPrefix = "syst";
    //~ for( auto& object : objects) {
    for (vector<Particle *>::const_iterator object = objects.begin(); object != objects.end(); ++object)
    {
        if (selector_map[partName]->passObjectSelection(*object, objectRho, idType, isSyst) == 0)
        {
            objectsAfterCut.push_back(*object);
            (*object)->setUserRecord(systPrefix + "IDpassed", true);
        }
        else if (particleTagMap[partName])
        {
            (*object)->setUserRecord(systPrefix + "IDpassed", false);
        }
        else
        {
            (*object)->owner()->remove(*object);
            //~ object->setUserRecord( systPrefix + "IDpassed", false);
        }
    }
    // ATTENTION: changing object-vector!
    if (!particleTagMap[partName])
        objects = objectsAfterCut;
}

// check if particles are ordered by pt
// void EventSelector::checkOrder( std::vector< pxl::Particle* > const &particles ) const {
void EventSelector::checkOrder(std::vector<pxl::Particle *> const &particles)
{
    if (particles.size() < 2)
        return;
    std::vector<pxl::Particle *>::const_iterator part = particles.begin();
    bool resort = false;
    double first_pt = (*part)->getPt();
    ++part;
    for (; part != particles.end(); ++part)
    {
        double const pt = (*part)->getPt();
        if (pt > first_pt)
        {
            std::stringstream exc;
            exc << "[ERROR] (EventSelector): ";
            exc << "Unsorted particle no. " << part - particles.begin();
            exc << " of type: " << (*part)->getName() << "!" << std::endl;
            exc << "Full info of unsorted particle:" << std::endl;
            (*part)->print(1, exc);
            exc << "pt of previous particle: " << first_pt << std::endl;
            exc << "pt of all particles: " << std::endl;

            std::vector<pxl::Particle *>::const_iterator part2 = particles.begin();
            for (part2 = particles.begin(); part2 != particles.end(); ++part2)
            {
                exc << "pt: " << (*part2)->getPt() << std::endl;
            }
            // there are some files with this error we can not sovle it here without changing the function
            //  TODO fix the sorting
            std::cerr << exc.str() << std::endl;
            // throw Tools::unsorted_error( exc.str() );
            resort = true;
        }
        else
        {
            first_pt = pt;
        }
    }
    if (resort)
    {
        // we can not resort, because it is a const vector grrrrrrr
        // pxl::sortParticles( particles );
    }
}

// Get a map of counts for a map of particle lists as created by getParticleLists
// use this function e.g. if you have modified particle lists as count particles changes
// the eventView wide count user record
std::map<std::string, int> EventSelector::getParticleCountMap(
    const std::map<std::string, std::vector<pxl::Particle *>> &particleLists) const
{
    std::map<std::string, int> countMap;
    for (auto &partList : particleLists)
    {
        countMap.emplace(partList.first, partList.second.size());
    }
    return countMap;
}

// std::map< std::string, std::vector< pxl::Particle* > > EventSelector::getGenParticleLists ( const pxl::EventView*
// EvtView ) const {
std::map<std::string, std::vector<pxl::Particle *>> EventSelector::getGenParticleLists(const pxl::EventView *EvtView)
{

    std::map<std::string, std::vector<pxl::Particle *>> particleMap;
    // for( auto part_name_use : particleUseMap ){
    //    if ( part_name_use.second )
    //       particleMap[ part_name_use.first ] = std::vector< pxl::Particle* >();
    // }
    // Get map between pdg id and part type if we work on gen
    std::map<int, std::string> pdg_id_map = Tools::pdg_id_type_map();

    // get all particles
    vector<pxl::Particle *> allparticles;
    EvtView->getObjectsOfType<pxl::Particle>(allparticles);
    pxl::sortParticles(allparticles);

    for (vector<pxl::Particle *>::const_iterator part = allparticles.begin(); part != allparticles.end(); ++part)
    {
        if ((*part)->getName() == "AK4")
        {
            particleMap["genJet"].push_back(*part);
        }
        if ((*part)->getName() == "AK8")
        {
            particleMap["genFatJet"].push_back(*part);
        }
        else if ((*part)->getName() == "slimmedMETs_gen")
        {
            particleMap["genMET"].push_back(*part);
        }
        else if ((*part)->getName() == "gen")
        {
            if (pdg_id_map.find(abs((*part)->getPdgNumber())) != pdg_id_map.end())
            {
                particleMap[pdg_id_map[abs((*part)->getPdgNumber())]].push_back(*part);
            }
            else
            {
                particleMap["pdg_" + std::to_string(abs((*part)->getPdgNumber()))].push_back(*part);
            }
        }
    }
    // check that the particles are ordered by Pt
    for (auto part_name_vector : particleMap)
    {
        checkOrder(part_name_vector.second);
    }

    return particleMap;
}

// Sort the particles in an event in lists and return them
std::map<std::string, std::vector<pxl::Particle *>> EventSelector::getParticleLists(
    const pxl::EventView *EvtView, bool selectedOnly,
    std::list<std::function<void(std::map<std::string, std::vector<pxl::Particle *>> &)>> functions) const
{
    std::map<std::string, std::vector<pxl::Particle *>> particleMap;
    for (auto part_name_use : particleUseMap)
    {
        if (part_name_use.second)
            particleMap[part_name_use.first] = std::vector<pxl::Particle *>();
    }

    // get all particles
    vector<pxl::Particle *> allparticles;
    EvtView->getObjectsOfType<pxl::Particle>(allparticles);
    pxl::sortParticles(allparticles);

    for (vector<pxl::Particle *>::const_iterator part = allparticles.begin(); part != allparticles.end(); ++part)
    {
        string name = (*part)->getName();
        // skip this particle if we want only selected particles and IDpassed is false
        if (selectedOnly && (*part)->hasUserRecord("IDpassed") && !(*part)->getUserRecord("IDpassed").toBool())
            continue;
        // Only fill the collection if we want to use the particle!
        // If the collections are not filled, the particles are also ignored in
        // the event cleaning.
        for (auto part_name_use : particleUseMap)
        {
            if (part_name_use.second && name == m_gen_rec_map.get(part_name_use.first).RecName)
            {
                particleMap[part_name_use.first].push_back(*part);
                break;
            }
        }
    }

    // check that the particles are ordered by Pt
    for (auto part_name_vector : particleMap)
    {
        checkOrder(part_name_vector.second);
    }

    // execute functions
    for (auto &func : functions)
    {
        func(particleMap);
    }

    return particleMap;
}

void EventSelector::performKinematicsSelection(pxl::EventView *EvtView, const bool useSystCuts)
{
    std::map<std::string, std::vector<pxl::Particle *>> particleMap = getParticleLists(EvtView);
    for (auto &part_name_use : particleUseMap)
    {
        if (part_name_use.second)
            applyKinematicCuts(particleMap[part_name_use.first], part_name_use.first, useSystCuts);
    }
}

void EventSelector::setScaleFactors(std::vector<pxl::Particle *> &objects, const std::string &partName)
{
    for (auto &obj : objects)
    {
        selector_map[partName]->setScaleFactors(obj);
    }
}

// Trigger selection using MUSiC style
void EventSelector::performTriggerSelection(pxl::EventView *RecEvtView, pxl::EventView *TrigEvtView)
{
    // check if event matches event topolgy for given set of triggers
    std::map<std::string, std::vector<pxl::Particle *>> particleMap = getParticleLists(RecEvtView, true);
    auto count_map = getParticleCountMap(particleMap);

    if (m_useTrigger)
    {
        bool topo_accept = true;
        topo_accept = m_triggerSelector.passEventTopology(count_map);

        RecEvtView->setUserRecord("topo_accept", topo_accept);
        // check if the events must be vetoed
        bool const vetoed = m_triggerSelector.checkVeto(particleMap, TrigEvtView);
        RecEvtView->setUserRecord("Veto", vetoed);
        bool const HLT_accept_no_offline_cuts = m_triggerSelector.passHLTriggerWithoutOffline(particleMap, TrigEvtView);
        RecEvtView->setUserRecord("HLT_accept_no_offline_cuts", HLT_accept_no_offline_cuts);
        bool const HLT_accept_offline_cuts = m_triggerSelector.checkOfflineCut(particleMap, TrigEvtView);
        RecEvtView->setUserRecord("HLT_accept_offline_cuts", HLT_accept_offline_cuts);
        bool const triggerAccept = HLT_accept_no_offline_cuts and HLT_accept_offline_cuts;
        RecEvtView->setUserRecord("trigger_accept", triggerAccept);
    }
    else
    {
        RecEvtView->setUserRecord("topo_accept", true);
        RecEvtView->setUserRecord("HLT_accept", true);
        RecEvtView->setUserRecord("Veto", true);
        RecEvtView->setUserRecord("trigger_accept", true);
    }
}

// Redo offline cuts for trigger
void EventSelector::performOfflineTriggerSelection(pxl::EventView *RecEvtView, pxl::EventView *TrigEvtView) const
{
    if (not m_useTrigger)
        return;
    if (not RecEvtView->hasUserRecord("HLT_accept_no_offline_cuts"))
    {
        std::string err =
            "EventSelector::performOfflineTriggerSelection: No userRecord 'HLT_accept_no_offline_cuts' found!\n";
        err += "Call performTriggerSelection before!";
        throw std::runtime_error(err);
    }
    if (RecEvtView->getUserRecord("HLT_accept_no_offline_cuts"))
    {
        const std::map<std::string, std::vector<pxl::Particle *>> particleMap = getParticleLists(RecEvtView, true);
        auto count_map = getParticleCountMap(particleMap);

        RecEvtView->setUserRecord("topo_accept", m_triggerSelector.passEventTopology(count_map));

        bool const HLT_accept_offline_cuts = m_triggerSelector.checkOfflineCut(particleMap, TrigEvtView);
        RecEvtView->setUserRecord("HLT_accept_offline_cuts", HLT_accept_offline_cuts);
        RecEvtView->setUserRecord("trigger_accept", HLT_accept_offline_cuts);
    }
}

//--------------------This is the main method to perform the selection-----------------------------------------
void EventSelector::performSelection(pxl::EventView *RecEvtView, pxl::EventView *GenEvtView,
                                     pxl::EventView *TrigEvtView, pxl::EventView *FilterView, const bool useSystCuts)
{ // used with either GenEvtView or RecEvtView
    string process = RecEvtView->getUserRecord("Process");

    bool filter_accept = false;
    if (FilterView)
    {
        filter_accept = passFilterSelection(FilterView);
    }
    else
    {
        bool filter_accept_rec = passFilterSelection(RecEvtView);
        bool filter_accept_gen = true;
        // GenEvtView is not always given
        if (GenEvtView)
        {
            filter_accept_gen = passFilterSelection(GenEvtView);
        }
        filter_accept = filter_accept_gen and filter_accept_rec;
    }
    RecEvtView->setUserRecord("filter_accept", filter_accept);

    std::map<std::string, double> rhoMap = getRhoMap(RecEvtView);

    std::map<std::string, std::vector<pxl::Particle *>> particleMap = getParticleLists(RecEvtView);

    // get vertices
    vector<pxl::Vertex *> vertices;
    RecEvtView->getObjectsOfType<pxl::Vertex>(vertices);
    for (auto &part_name_use : particleUseMap)
    {
        if (part_name_use.second)
            applyCutsOnObject(particleMap[part_name_use.first], part_name_use.first, rhoMap[part_name_use.first],
                              particleIDTypeMap[part_name_use.first], useSystCuts);
        if (part_name_use.first.find("Jet") != std::string::npos)
            m_typeWriter.writeJetTypes(particleMap[part_name_use.first]);
    }

    // Perform trigger selction
    performTriggerSelection(RecEvtView, TrigEvtView);

    // Set scalefactors for particles
    if (not m_data)
    {
        for (auto &part_name_use : particleUseMap)
        {
            if (particleUseMap[part_name_use.first])
                setScaleFactors(particleMap[part_name_use.first], part_name_use.first);
            if (part_name_use.first.find("Jet") !=
                std::string::npos) // sarFIXME check //separate method for BTag scale factor
                m_typeWriter.setBTagScaleFactor(particleMap[part_name_use.first],
                                                RecEvtView->getUserRecord("EventSeed").toInt32());
        }
    }

    // this is for tagging jets as taus with the syst id
    // by default all taus are jets so this is tricky!
    // if( m_tau_use and m_jet_ID_use){ //LOR COMM //UNCOMMENT IF YOU WANT TO USE TAUS
    //    TauSelector* tau_selector = dynamic_cast< TauSelector* >( selector_map["Tau"] );
    //  make sure dynamic_cast was successfull
    //   if( tau_selector ) tau_selector->tagJetsAsTau( particleMap["Tau"], particleMap["Jet"] );
    //} //END LOR COMM

    bool generator_accept = true;
    // For gen: check if generator cuts are fulfilled, e.g. to cut tail samples or ht binning
    if (m_gen_use && GenEvtView != 0)
    {
        generator_accept = m_gen_selector.passGeneratorCuts(GenEvtView);
    }
    RecEvtView->setUserRecord("generator_accept", generator_accept);
    if (GenEvtView)
    {
        GenEvtView->setUserRecord("generator_accept", generator_accept);
    }
}

void EventSelector::removeOverlaps(pxl::EventView *RecEvtView) const
{
    // consistently check GenView for duplicates, important especially for GenJets and efficiency-normalization
    std::map<std::string, std::vector<pxl::Particle *>> particleMapSelectedOnly = getParticleLists(RecEvtView, true);
    if (not m_ignoreOverlaps)
        m_eventCleaning.cleanEvent(particleMapSelectedOnly,
                                   true // dummy for isRec
        );
}

const ObjectSelector *EventSelector::getObjectSelector(const std::string &object) const
{
    return selector_map.at(object);
}
