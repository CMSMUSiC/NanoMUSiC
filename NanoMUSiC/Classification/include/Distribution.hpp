#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <memory>
#include <unordered_map>

#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"

#include "ROOT/RVec.hxx"
#include "TH1.h"

using namespace ROOT;
using namespace ROOT::VecOps;

class IntegralPValueProps
{
  public:
    double total_data;
    double total_mc;
    double sigma_total;
    double sigma_stat;
    std::vector<double> total_per_process_group;
};

class PlotProps
{
  public:
    std::string class_name;
    std::string distribution_name;
    double x_min;
    double x_max;
    double y_min;
    double y_max;
    std::shared_ptr<TH1F> total_data_histogram;
    TGraphAsymmErrors data_graph;
    std::unordered_map<std::string, std::shared_ptr<TH1F>> mc_histograms;
    TGraphErrors mc_uncertainty;
    TGraphAsymmErrors ratio_graph;
    TGraphErrors ratio_mc_error_band;
};

class Distribution
{
  public:
    constexpr static double min_bin_width = 10.;

    bool m_scale_to_area;
    std::string m_distribution_name;
    std::string m_event_class_name;
    RVec<double> m_statistical_uncert;
    RVec<double> m_systematics_uncert;
    RVec<double> m_total_uncert;
    TH1F m_total_data_histogram;
    TH1F m_total_mc_histogram;
    unsigned long m_n_bins;
    TGraphAsymmErrors m_data_graph;
    TGraphErrors m_error_band;
    std::unordered_map<std::string, std::unordered_map<std::string, TH1F>> m_histogram_per_process_group_and_shift;

    // constructor and methods
    Distribution(const std::vector<std::string> &input_files,
                 const std::string &event_class_name,
                 const std::string &distribution_name,
                 bool allow_rescale_by_width);

    auto get_statistical_uncert() const -> RVec<double>;
    auto get_systematics_uncert(
        const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>>
            &unmerged_mc_histograms) const -> RVec<double>;

    auto get_plot_props() -> PlotProps;
    auto get_integral_pvalue_props() const -> IntegralPValueProps;
    auto save(const std::string &output_file) const -> std::string;
};

#endif // DISTRIBUTION_HPP
