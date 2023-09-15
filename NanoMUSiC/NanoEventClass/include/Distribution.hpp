#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <cstddef>
#include <memory>
#include <unordered_map>

#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "fmt/format.h"

#include "NanoEventClass.hpp"
#include "ROOT/RVec.hxx"
#include "TFile.h"
#include "TH1.h"
#include "TROOT.h"
#include "TSystem.h"

using namespace ROOT;
using namespace ROOT::VecOps;

class PValueProps
{
  public:
    double total_data;
    double total_mc;
    double sigma_total;
    double sigma_stat;
    std::vector<double> total_per_process_group;
};

class Distribution
{
  public:
    std::string m_distribution_name;
    std::string m_event_class_name;
    RVec<double> m_statistical_uncert;
    RVec<double> m_systematics_uncert;
    RVec<double> m_total_uncert;
    std::shared_ptr<TH1F> m_total_data_histogram;
    std::shared_ptr<TH1F> m_total_mc_histogram;
    TGraphAsymmErrors m_data_graph;
    TGraphErrors m_error_band;
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<TH1F>>>
        m_histogram_per_process_group_and_shift;

    Distribution(const NanoEventClass &ec, const std::string &distribution_name);

    // auto plot() -> PlotProps;
    auto get_pvalue_props() const -> PValueProps;
};

#endif // DISTRIBUTION_HPP
