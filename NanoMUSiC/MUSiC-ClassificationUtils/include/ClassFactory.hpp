#ifndef CLASS_FACTORY

#include <map>
#include <string>

#include "CrossSectionOrderErrorMap.hpp"
#include "ParticleMap.hpp"
#include "TEventClass.hpp"

#include "Event.hpp"

#include "Tools.hpp"

class ClassFactory
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
        std::vector<float> pdf_weights;
        std::pair<double, double> as_weights;
        double Global_ScalefactorError;
    };

    ClassFactory(const bool is_data,
                 const double cross_section,
                 const double filter_efficiency,
                 const double k_factor,
                 const double lumi,
                 unsigned int const numPDFs,
                 const std::string &process,
                 const std::string &processOrder,
                 const std::string &processGroup,
                 const std::string outfilename = "",
                 const std::string hash = "dummyhash",
                 const bool debug = 1);

    void analyseEvent(Event &event);
    void endJob();
    std::string getQCDSystWeightName(std::string shift_type) const;
    void FinishEventClasses(std::map<std::string, TEventClass *> &EvtClasses,
                            std::set<std::string> &ProcessList,
                            const std::string &Label);
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

    void WriteEventListToFile(std::string const &ECname);
    void WriteAnalysisInfoToFile(std::set<std::string> const &processList);
    // void CheckForCrossSection();
    void fillFilterCutFlow(const double weight);
    void fillCutFlow(Event &event, const double weight);
    void fillCutFlow(const double sumPt,
                     const double invMass,
                     const double met,
                     const int numPart,
                     const std::unordered_map<std::string, int> &countMap,
                     const double weight);
    std::vector<ClassFactory::EventInfo> GetListedEvents(std::string const &ECname);

    TEventClass *InitTEventClass(std::string const &Type,
                                 std::string const &ECName,
                                 ParticleMap &particleMap,
                                 std::unordered_map<std::string, int> countMap,
                                 int const absCharge,
                                 bool const inclusiveEC) const;

    // Fill given TEventClass with given event (i.e. set of particles).
    // Same function for data and MC, for data the weights are simply not used
    // at all!
    void FillEventClass(TEventClass *EventClassToFill,
                        EventInfo &event_info,
                        std::string const &process,
                        ParticleMap &particleMap,
                        std::unordered_map<std::string, int> &countMap,
                        double const eventWeight = 1.0,
                        std::string systName = "");

    void FillInclusiveRecursive(std::map<std::string, TEventClass *> &inclEventClasses,
                                std::vector<std::string> partNameVector,
                                std::string Type,
                                std::string Process,
                                double const weight,
                                EventInfo &event_info,
                                int iterator,
                                std::unordered_map<std::string, int> recCountMap,
                                std::unordered_map<std::string, int> refCountMap,
                                ParticleMap &particleMap,
                                std::string systName,
                                std::string nameSuffix);

    // Differential syst are filled using the same functions but with   systName
    void Fill(EventView &event_view, const double weight, EventInfo &event_info, std::string systName = "");

    bool hasTooManyJets(std::unordered_map<std::string, int> countMap);

    bool CheckEventToList(double const sumPt, double const Minv, double const MET) const;
    void FillEventList(std::string const &ECname, EventInfo const &event_info);

    inline bool ends_with(std::string const &value, std::string const &ending)
    {
        if (ending.size() > value.size())
            return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

  private:
    std::string m_outfilename;
    std::string const m_runHash;
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

    double const xsec;
    double const fEff;
    double const kFac;
    double const m_lumi;
    unsigned int const m_numPDFs;

    std::string const m_process;
    std::string const m_processOrder;
    std::string const m_processGroup;

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

    std::map<std::string, double> m_fakeErrorMap;
    std::map<std::string, double> m_chargeErrorMap;
    CrossSectionOrderErrorMap m_xsOrderErrorMap;

    std::set<std::string> m_differentialSystematics;
    std::set<std::string> m_constantSystematics;
};

#endif // !CLASS_FACTORY