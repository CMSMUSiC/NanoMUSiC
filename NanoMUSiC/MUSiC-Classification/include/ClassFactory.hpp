#ifndef CLASS_FACTORY

#include <map>
#include <string>

#include "CrossSectionOrderErrorMap.hpp"
#include "TEventClass.hpp"

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
    };

    ClassFactory(const std::string outfilename = "", const std::string hash = "dummyhash", const bool debug = 1);
    auto analyse_event() -> void;
    auto end_job() -> void;

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
};

#endif // !CLASS_FACTORY