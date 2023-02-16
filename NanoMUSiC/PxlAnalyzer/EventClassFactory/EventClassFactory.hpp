#ifndef EventClassFactory_hh
#define EventClassFactory_hh

#include "Main/EventSelector.hpp"
#include "Main/GenRecNameMap.hpp"

#include "Main/Systematics.hpp"
#include "Main/TOMLConfig.hpp"
#include "PDFTool.hpp"
#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"
#include "TEventClass.hpp"
#include "TFile.h"
#include <map>
#include <set>

#include "CrossSectionOrderErrorMap.hpp"
#include "EventClassFactory/ParticleMap.hpp"
#include "ParticleSplittingFunctions.hpp"

#define TRIGGERSTREAM

#ifdef TRIGGERSTREAM
#include "TriggerStream.hpp"
#endif

namespace pdf
{
class PDFInfo;
}
//----------------------------------------------------------------------

class EventClassFactory : public pxl::AnalysisProcess
{
  public:
    struct EventInfo
    {
        double sumpt;
        double minv;
        double met;
        unsigned int run;
        unsigned int lumisection;
        unsigned long eventnum;
        std::string dataset;
        std::string filename;
        unsigned int eventnumpxlio;
        double central_weight;
        double pileup;
        double pileup_up;
        double pileup_down;
        double prefire_weight;
        double prefire_weight_up;
        double prefire_weight_down;
        std::string process;
        std::string qcd_scale;
        bool has_scale_variation;
    };

    EventClassFactory(const Tools::MConfig &cutconfig,
                      const TOMLConfig &xsecs,
                      EventSelector &selector,
                      Systematics &syst_shifter,
                      const std::string outfilename = "",
                      const string hash = "dummyhash",
                      const bool debug = 1);
    virtual ~EventClassFactory()
    {
    }

    virtual void endJob(const Serializable *);

    virtual void analyseEvent(const pxl::Event *event_ptr);

  private:
#ifdef TRIGGERSTREAM
    TriggerStream trig_stream;
#endif

    TOMLConfig m_XSections;
    pdf::PDFTool m_pdfTool;
    EventSelector &m_selector;
    Systematics &systematics;
    std::string m_outfilename;
    string const m_runHash;
    bool const m_debug;

    // Store center of mass energy.
    double const m_cme;
    bool m_data;
    bool m_useWeight;
    bool m_usePDF;
    bool m_useTrigger;
    bool m_use_prefire_check;
    bool m_syst_initialized;
    // replace the map by map<>* for faster access
    // maps for Event-Classes
    std::map<std::string, TEventClass *> _genEventClasses; // map of gen-Eventclass pointers
    std::map<std::string, TEventClass *> _recEventClasses; // map of rec-Eventclass pointers
    // maps for Inlusive Event-Classes
    std::map<std::string, TEventClass *> _genInclEventClasses; // map of gen-Eventclass pointers
    std::map<std::string, TEventClass *> _recInclEventClasses; // map of rec-Eventclass pointers
    // maps for Jet Inlusive Event-Classes
    std::map<std::string, TEventClass *> _genJetInclEventClasses; // map of gen-Eventclass pointers
    std::map<std::string, TEventClass *> _recJetInclEventClasses; // map of rec-Eventclass pointers

    // cut flow histograms
    TH1F m_cutFlowUnweighted;
    TH1F m_cutFlowWeighted;
    // filter cut flow histograms
    TH1F m_cutFlowFilterUnweighted;
    TH1F m_cutFlowFilterWeighted;

    // keeps track of analyzed events
    std::map<std::string, unsigned int> eventCounts;
    std::map<std::string, double> totalEvents;
    std::map<std::string, double> totalEventsUnweighted;
    std::map<std::string, double> totalEventsUnweightedFiltered;
    std::map<std::string, double> totalEventsWeightedFiltered;
    std::map<std::string, unsigned int> numEventsFiltered;
    std::map<std::string, double> genTotalEventsWeighted;
    std::map<std::string, unsigned int> genTotalEventsUnweighted;
    std::map<std::string, double> recTotalEventsWeighted;
    std::map<std::string, unsigned int> recTotalEventsUnweighted;

    std::string m_lastprocess;
    std::string m_lastprocessOrder;
    std::string m_lastprocessGroup;

    bool const m_muo_use;
    bool const m_ele_use;
    bool const m_tau_use;
    bool const m_gam_use;
    bool const m_jet_use;
    bool const m_met_use;

    bool const m_ec_recOnly;
    bool const m_ec_genOnly;
    bool const m_ec_charge_use;

    double const m_ec_sumpt_min;
    double const m_ec_minv_min;
    double const m_ec_met_min;

    // Store the maximum number of events per EventClass to store in the event
    // lists.
    unsigned int const m_num_listed_events_max;
    // Maximum number of total jets in EC
    unsigned int const m_max_jet_ec;
    // Maps containing a limited number of events that are filled for each
    // EventClass, separately for each kinematic variable.
    std::map<std::string, std::map<double, EventInfo>> m_events_by_sumpt;
    std::map<std::string, std::map<double, EventInfo>> m_events_by_minv;
    std::map<std::string, std::map<double, EventInfo>> m_events_by_met;

    double const m_fudge_sumpt;
    double const m_fudge_minv;
    double const m_fudge_met;
    double const m_bin_size_min;

    bool const m_useBJets;
    std::string const m_jet_bJets_algo;
    bool const m_gam_regions_use;
    bool const m_ele_regions_use;

    double m_xsec;
    double m_fEff;
    double m_kFac;

    std::list<std::function<void(std::map<std::string, std::vector<pxl::Particle *>> &)>> splittings;

    std::map<std::string, double> m_fakeErrorMap;
    std::map<std::string, double> m_chargeErrorMap;
    CrossSectionOrderErrorMap m_xsOrderErrorMap;

    GenRecNameMap const m_gen_rec_map;

    std::map<std::string, double> readFakeErrorMap(const Tools::MConfig &cutconfig);
    std::map<std::string, double> readChargeErrorMap(const Tools::MConfig &cutconfig);
    void addConstantSystematics();
    std::string getQCDSystWeightName(std::string shift_type) const;
    void CheckForCrossSection(std::string &process);

    void fillFilterCutFlow(const double weight);
    void fillCutFlow(const pxl::Event *event, const double weight);
    void fillCutFlow(const double sumPt,
                     const double invMass,
                     const double met,
                     const int numPart,
                     const std::map<std::string, int> &countMap,
                     const double weight);

    void FinishEventClasses(std::map<std::string, TEventClass *> &EvtClasses,
                            std::set<std::string> &ProcessList,
                            const std::string &Label);
    // Strip out extension sample strings from process name
    void prepareProcessName(std::string &proc);

    std::map<std::string, TEventClass *> &getReferenceExclusiveMap(std::string type)
    {
        if (type == "Gen")
            return _genEventClasses;
        if (type == "Rec")
            return _recEventClasses;
        std::cout << "Exclusive Map " << type << " not found!!" << std::endl;
        // return an empty dummy event-class-map. gcc complained, I dunno what would have happened
        // if a non-existing event-class is a critical error, we should crash here!
        return *(new std::map<std::string, TEventClass *>);
    }

    std::map<std::string, TEventClass *> &getReferenceInclusiveMap(std::string type)
    {
        if (type == "Gen")
            return _genInclEventClasses;
        if (type == "Rec")
            return _recInclEventClasses;
        std::cout << "Inclusive Map " << type << " not found!!" << std::endl;
        // return an empty dummy event-class-map. gcc complained, I dunno what would have happened
        // if a non-existing event-class is a critical error, we should crash here!
        return *(new std::map<std::string, TEventClass *>);
    }

    std::map<std::string, TEventClass *> &getReferenceJetInclusiveMap(std::string type)
    {
        if (type == "Gen")
            return _genJetInclEventClasses;
        if (type == "Rec")
            return _recJetInclEventClasses;
        std::cout << "Inclusive Map " << type << " not found!!" << std::endl;
        // return an empty dummy event-class-map. gcc complained, I dunno what would have happened
        // if a non-existing event-class is a critical error, we should crash here!
        return *(new std::map<std::string, TEventClass *>);
    }

    std::vector<EventInfo> GetListedEvents(std::string const &ECname);

    // Fill given TEventClass with given event (i.e. set of particles).
    // Same function for data and MC, for data the weights are simply not used
    // at all!
    void FillEventClass(TEventClass *EventClassToFill,
                        EventInfo &event_info,
                        std::string const &process,
                        ParticleMap &particleMap,
                        std::map<std::string, int> &countMap,
                        double const eventWeight = 1.0,
                        std::string systName = "");

    void FillInclusiveRecursive(std::map<std::string, TEventClass *> &inclEventClasses,
                                std::vector<std::string> partNameVector,
                                std::string Type,
                                std::string Process,
                                double const weight,
                                EventInfo &event_info,
                                int iterator,
                                std::map<std::string, int> recCountMap,
                                std::map<std::string, int> refCountMap,
                                ParticleMap &particleMap,
                                std::string systName,
                                std::string nameSuffix);

    void Fill(pxl::EventView *EvtView,
              double const weight,
              EventInfo &event_info,
              std::string systName = "" // Differential syst are filled using the same functions but with
    );                                  //   systName

    bool hasTooManyJets(std::map<std::string, int> countMap);
    bool CheckEventToList(double const sumPt, double const Minv, double const MET) const;

    void FillEventList(std::string const &ECname, EventInfo const &event_info);
    TEventClass *InitTEventClass(std::string const &Type,
                                 std::string const &ECName,
                                 ParticleMap &particleMap,
                                 std::map<std::string, int> countMap,
                                 int const absCharge,
                                 bool const inclusiveEC) const;

    // Create a text file for each EventClass to store the "most interesting" events.
    void WriteEventListToFile(std::string const &ECname);
    void WriteAnalysisInfoToFile(std::set<std::string> const &processList);

    inline bool ends_with(std::string const &value, std::string const &ending)
    {
        if (ending.size() > value.size())
            return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }
};
//----------------------------------------------------------------------
#endif
