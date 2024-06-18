#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <cstdlib>
#include <memory>
#include <unordered_map>

#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"

#include "Shifts.hpp"

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
    std::string year_to_plot;
    double x_min;
    double x_max;
    double y_min;
    double y_max;
    TH1F total_data_histogram;
    TGraphAsymmErrors data_graph;
    std::unordered_map<std::string, TH1F> mc_histograms;
    TGraphErrors mc_uncertainty;
    TGraphAsymmErrors ratio_graph;
    TGraphErrors ratio_mc_error_band;
};

struct ECHistogram
{
    std::string analysis_name;
    std::string process_group;
    std::string xs_order;
    std::string sample;
    std::string year;
    std::size_t shift;
    std::string histo_name;
    std::shared_ptr<TH1F> histogram;

    auto get_shift() const -> std::string
    {
        return Shifts::variation_to_string(static_cast<Shifts::Variations>(shift));
    }
};

class Distribution
{
  public:
    enum class YearToPlot
    {
        Run2016,
        Run2017,
        Run2018,
        Run2,
    };

    static auto years_to_plot() -> std::vector<YearToPlot>
    {
        return {
            YearToPlot::Run2016,
            YearToPlot::Run2017,
            YearToPlot::Run2018,
            YearToPlot::Run2,
        };
    }

    static auto year_to_string(YearToPlot year_to_plot) -> std::string
    {
        switch (year_to_plot)
        {
        case YearToPlot::Run2016:
            return "2016";
        case YearToPlot::Run2017:
            return "2017";
        case YearToPlot::Run2018:
            return "2018";
        case YearToPlot::Run2:
            return "Run2";
        default:
            fmt::print(stderr, "ERROR: Could not convert YearToPlot to string.");
            std::exit(EXIT_FAILURE);
        }
    }

    static auto year_filter(YearToPlot year_to_plot, const std::string &year) -> bool
    {
        switch (year_to_plot)
        {
        case YearToPlot::Run2:
            return (year == year_to_string(YearToPlot::Run2016) or year == year_to_string(YearToPlot::Run2017) or
                    year == year_to_string(YearToPlot::Run2018));
        default:
            return year == year_to_string(year_to_plot);
        }
    }

    static constexpr double min_bin_width = 10.;
    static constexpr std::size_t total_variations = static_cast<std::size_t>(Shifts::Variations::kTotalVariations);

    bool m_scale_to_area;
    std::string m_distribution_name;
    std::string m_event_class_name;
    std::string m_year_to_plot;
    RVec<double> m_statistical_uncert;
    RVec<double> m_systematics_uncert;
    RVec<double> m_total_uncert;
    TH1F m_total_data_histogram;
    TH1F m_total_mc_histogram;
    unsigned long m_n_bins;
    TGraphAsymmErrors m_data_graph;
    TGraphErrors m_error_band;

    std::array<std::unordered_map<std::string, TH1F>, total_variations> m_histogram_per_process_group_and_shift;

    // constructor and methods
    Distribution() = default;

    // Distribution(const std::vector<std::unique_ptr<TFile>> &input_root_files,
    Distribution(const std::vector<ECHistogram> &event_class_histograms,
                 const std::string &event_class_name,
                 const std::string &distribution_name,
                 bool allow_rescale_by_width,
                 YearToPlot year_to_plot);

    static auto get_distribution_props(const std::string &analysis_name) -> std::vector<std::tuple<std::string, bool>>
    {
        if (analysis_name.find("EC_") != std::string::npos)
        {
            return {{"counts", false}, {"sum_pt", true}, {"invariant_mass", true}, {"met", true}};
        }

        return {{"invariant_mass", true}};
    }

    static auto make_distributions(const std::vector<std::string> &input_files,
                                   const std::string &output_dir,
                                   std::vector<std::string> &analysis_to_plot
                                   ) -> void;

    auto get_statistical_uncert() -> RVec<double>;
    auto get_systematics_uncert(const std::array<std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>,
                                                 total_variations> &unmerged_mc_histograms) -> RVec<double>;

    auto make_plot_props() -> PlotProps;
    auto make_integral_pvalue_props()  -> IntegralPValueProps;
};

#endif // DISTRIBUTION_HPP
