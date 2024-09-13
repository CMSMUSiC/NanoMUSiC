#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <cstdlib>
#include <memory>
#include <optional>
#include <unordered_map>

#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"

#include "Shifts.hpp"

#include "ROOT/RVec.hxx"
#include "TH1.h"

#include "roothelpers.hpp"

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
    bool m_has_data;
    std::unordered_map<std::string, RVec<double>> m_systematics_uncertainties;
    std::array<std::unordered_map<std::string, TH1F>, total_variations> m_histogram_per_process_group_and_shift;

    // constructor and methods
    Distribution() = default;

    /// A valid class should have some data and some MC
    static auto is_valid(const std::vector<ECHistogram> &event_class_histograms) -> bool
    {
        bool has_data = false;
        bool has_mc = false;
        for (const auto &hist : event_class_histograms)
        {
            if (hist.process_group == "Data")
            {
                has_data = true;
            }

            if (hist.process_group != "Data")
            {
                has_mc = true;
            }
        }

        return has_mc and has_data;
    }

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

        else if (analysis_name.find("z_to") != std::string::npos)
        {
            return {
                {"invariant_mass", true},
                {"met", true},
                {"sum_pt", true},
                {"lepton_1_pt", true},
                {"lepton_2_pt", true},
                {"lepton_1_eta", false},
                {"lepton_2_eta", false},
                {"lepton_1_phi", false},
                {"lepton_2_phi", false},
                {"lepton_1_jet_1_dPhi", false},
                {"lepton_1_jet_1_dR", false},
                {"jet_multiplicity", false},
                {"bjet_multiplicity", false},

            };
        }

        else if (analysis_name.find("w_to") != std::string::npos)
        {
            return {{"invariant_mass", true},
                    {"sum_pt", true},
                    {"lepton_pt", true},
                    {"lepton_eta", false},
                    {"lepton_phi", false},
                    {"met", true}};
        }

        else if (analysis_name.find("ttbar_to") != std::string::npos)
        {
            return {{"invariant_mass_jet0_jet1", true}, {"ht_had_lep", true}, {"transverse_mass_lep_MET", true}};
        }

        else if (analysis_name.find("gamma_plus_jets") != std::string::npos)
        {
            return {{"gamma_pt", true}, {"gamma_eta", false}, {"gamma_phi", false}};
        }

        fmt::print(stderr, "ERROR: Could not find distribution props for given analysis name ({}).", analysis_name);
        std::exit(EXIT_FAILURE);
    }

    static auto fold(const std::vector<std::string> &input_files,
                     const std::string &output_dir,
                     std::vector<std::string> &analyses_to_fold) -> void;

    static auto make_distributions(const std::string &input_file,
                                   const std::string &output_dir,
                                   std::string &analysis_to_plot,
                                   const std::optional<std::unordered_map<std::string, double>> &rescaling) -> bool;

    auto get_statistical_uncert() -> RVec<double>;
    auto get_systematics_uncert(const std::array<std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>,
                                                 total_variations> &unmerged_mc_histograms) -> RVec<double>;

    auto make_plot_props() -> PlotProps;
    auto make_integral_pvalue_props() -> IntegralPValueProps;

    auto print_me() const -> void
    {
        fmt::print("--> Event Class: {}\n", m_event_class_name);
        fmt::print("--> Distribution: {}\n", m_distribution_name);
        fmt::print("--> Year: {}\n", m_year_to_plot);
        fmt::print("--> Should scale to area? {}\n", m_scale_to_area);
        fmt::print("--> Number of bins: {}\n", m_n_bins);
        fmt::print("--> Statistical Uncert: [{}]\n", fmt::join(m_statistical_uncert, " - "));
        fmt::print("--> Systematatic Uncert: [{}]\n", fmt::join(m_systematics_uncert, " - "));
        fmt::print("--> Total uncert: [{}]\n", fmt::join(m_total_uncert, " - "));
        fmt::print("--> Total Data histogram:\n");
        m_total_data_histogram.Print("all");
        fmt::print("--> Total MC histogram:\n");
        m_total_mc_histogram.Print("all");
        fmt::print("--> Has MC: {}\n", has_mc());
        fmt::print("--> Has Data: {}\n", m_has_data);
        // m_data_graph.Print();
        // m_error_band.Print();
    }

    auto has_mc(double threshold = 0.) const -> bool
    {
        return ROOT::VecOps::Sum(ROOTHelpers::Counts(m_total_mc_histogram)) > threshold;
    }

    auto has_data() const -> bool
    {
        return m_has_data;
    }

    auto get_data_counts() const -> RVec<double>
    {
        return ROOTHelpers::Counts(m_total_data_histogram);
    }

    struct MCBinProps
    {
        std::unordered_map<std::string, double> mcEventsPerProcessGroup;
        std::unordered_map<std::string, double> mcStatUncertPerProcessGroup;
        float lowerEdge;
        float width;
        std::unordered_map<std::string, double> mcSysUncerts;
    };

    auto get_mcbins_props() const -> std::vector<MCBinProps>
    {
        auto props = std::vector<MCBinProps>(m_n_bins);
        auto has_edges_and_syst = false;

        for (std::size_t shift = 0; shift < static_cast<std::size_t>(Shifts::Variations::kTotalVariations); shift++)
        {
            for (const auto &[pg, histo] : m_histogram_per_process_group_and_shift[shift])
            {
                if (pg != "Data")
                {
                    if (shift == static_cast<std::size_t>(Shifts::Variations::Nominal))
                    {
                        if (not(has_edges_and_syst))
                        {
                            auto edges = ROOTHelpers::Edges(histo);
                            auto widths = ROOTHelpers::Widths(histo);
                            for (std::size_t idx = 0; idx < m_n_bins; idx++)
                            {
                                props[idx].lowerEdge = edges[idx];
                                props[idx].width = widths[idx];
                                for (const auto &[var, syst] : m_systematics_uncertainties)
                                {

                                    props[idx].mcSysUncerts[var] = syst[idx] ;
                                }

                                has_edges_and_syst = true;
                            }
                        }

                        auto counts = ROOTHelpers::Counts(histo);
                        auto errors = ROOTHelpers::Errors(histo);
                        for (std::size_t idx = 0; idx < m_n_bins; idx++)
                        {
                            props[idx].mcEventsPerProcessGroup[pg] = counts[idx];
                            props[idx].mcStatUncertPerProcessGroup[pg] = errors[idx];
                        }
                    }
                }
            }
        }
        return props;
    }
};

#endif // DISTRIBUTION_HPP
