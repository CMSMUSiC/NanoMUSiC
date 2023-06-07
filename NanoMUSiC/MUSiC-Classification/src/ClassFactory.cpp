#include "ClassFactory.hpp"

using namespace std::string_literals;

ClassFactory::ClassFactory(const std::string outfilename, const std::string hash, const bool debug)
    : m_outfilename(outfilename),
      m_runHash(hash),
      m_debug(debug),

      m_cme(13000.),

      m_usePDF(true),
      m_useTrigger(true),
      m_use_prefire_check(false),

      m_syst_initialized(false),
      m_cutFlowUnweighted(TH1F("cutflow_unweighted", "cutflow_unweighted", 10, 0, 10)),
      m_cutFlowWeighted(TH1F("cutflow_weighted", "cutflow_weighted", 10, 0, 10)),
      m_cutFlowFilterUnweighted(TH1F("cutflow_filter_unweighted", "cutflow_filter_unweighted", 10, 0, 10)),
      m_cutFlowFilterWeighted(TH1F("cutflow_filter_weighted", "cutflow_filter_weighted", 10, 0, 10)),
      m_lastprocessGroup("uninitialized"),

      m_muo_use(false),
      m_ele_use(false),
      m_tau_use(false),
      m_gam_use(false),
      m_jet_use(true),
      m_met_use(true),

      m_ec_recOnly(true),
      m_ec_genOnly(false),
      m_ec_charge_use(false),

      m_ec_sumpt_min(0.),
      m_ec_minv_min(0.),
      m_ec_met_min(0.),

      m_num_listed_events_max(100),
      m_max_jet_ec(6),

      m_events_by_sumpt(),
      m_events_by_minv(),
      m_events_by_met(),

      m_fudge_sumpt(1.),
      m_fudge_minv(1.),
      m_fudge_met(1.),
      m_bin_size_min(10),

      m_useBJets(true),
      m_jet_bJets_algo("DeepJet=DeepFlavour"s),
      m_gam_regions_use(true),
      m_ele_regions_use(false),

      m_xsOrderErrorMap(CrossSectionOrderErrorMap())
{
    m_fakeErrorMap = {
        {"Tau", 0.5},   //
        {"Ele", 0.5},   //
        {"Muon", 0.5},  //
        {"Jet", 0.},    //
        {"Gamma", 0.5}, //
        {"MET", 0.}     //
    };

    m_chargeErrorMap = {
        {"Tau", 0.5},   //
        {"Ele", 0.5},   //
        {"Muon", 0.5},  //
        {"Jet", 0.},    //
        {"Gamma", 0.5}, //
        {"MET", 0.}     //
    };
}

auto ClassFactory::analyse_event() -> void
{
}

auto ClassFactory::end_job() -> void
{
}
