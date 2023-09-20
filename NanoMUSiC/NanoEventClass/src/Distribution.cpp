#include "Distribution.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fnmatch.h>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>

#include "ROOT/RVec.hxx"
#include "TCollection.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TKey.h"

#include "TLegend.h"
#include "fmt/format.h"

#include "roothelpers.hpp"

#include "NanoEventClass.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;

Distribution::Distribution(const NanoEventClass &ec, const std::string &distribution_name)
    : m_scale_to_area(distribution_name != "counts"),
      m_distribution_name(distribution_name),
      m_event_class_name(ec.m_class_name)
{
    const std::vector<NanoEventHisto> *event_class_histograms;
    if (distribution_name == "counts")
    {
        event_class_histograms = &ec.m_counts;
    }
    else if (distribution_name == "invariant_mass")
    {
        event_class_histograms = &ec.m_invariant_mass;
    }
    else if (distribution_name == "sum_pt")
    {
        event_class_histograms = &ec.m_sum_pt;
    }
    else if (distribution_name == "met")
    {
        event_class_histograms = &ec.m_met;
    }
    else
    {
        fmt::print(stderr, "ERROR: Could not build Distribution for the requested name.");
        std::exit(EXIT_FAILURE);
    }

    // split the histograms per process group and shift
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>> histograms;
    for (auto &&histo : *event_class_histograms)
    {
        if (histograms.find(histo.shift) == histograms.end())
        {
            histograms[histo.shift] = std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>();
        }
        histograms[histo.shift][histo.process_group].push_back(histo.histogram);
    }

    // merge histograms per process group and shift
    for (auto &&[shift, histos_per_process_group] : histograms)
    {
        if (m_histogram_per_process_group_and_shift.find(shift) == m_histogram_per_process_group_and_shift.end())
        {
            m_histogram_per_process_group_and_shift[shift] = std::unordered_map<std::string, std::shared_ptr<TH1F>>();
        }

        for (auto &&[pg, histos] : histograms[shift])
        {
            m_histogram_per_process_group_and_shift[shift][pg] = ROOTHelpers::SumAsTH1F(histos);
        }
    }

    // build Data histogram and graph
    m_total_data_histogram = m_histogram_per_process_group_and_shift["Nominal"]["Data"];

    // merge MC histograms
    std::vector<std::shared_ptr<TH1F>> mc_histos;
    for (auto &&[pg, histo] : m_histogram_per_process_group_and_shift["Nominal"])
    {
        if (pg != "Data")
        {
            mc_histos.push_back(histo);
        }
    }
    m_total_mc_histogram = ROOTHelpers::SumAsTH1F(mc_histos);

    // statistical uncertainties
    m_statistical_uncert = ROOTHelpers::Errors(m_total_mc_histogram);

    // systemtic uncertainties
    m_systematics_uncert = ROOTHelpers::Counts(m_total_mc_histogram) * 0.05;

    // total uncertainties
    m_total_uncert =
        ROOT::VecOps::sqrt(ROOT::VecOps::pow(m_statistical_uncert, 2.) + ROOT::VecOps::pow(m_systematics_uncert, 2.));

    // build MC Stack --> ideally, should be done when plotting
    // compute systematics uncert
    // compute statistical uncert
    // compute data and its uncert for ratio plot
    // compute MC uncertanties for ratio plot
}

auto Distribution::get_pvalue_props() const -> PValueProps
{
    return PValueProps{
        .total_data = 2, .total_mc = 3., .sigma_total = 1., .sigma_stat = 3, .total_per_process_group = {2, 3, 5, 9}};
}

auto Distribution::get_plot_props() const -> PlotProps
{
    auto min_max = ROOTHelpers::GetMinMax(m_total_data_histogram);
    auto [_min, _max] = min_max;
    auto [idx_min, min] = _min;
    auto [idx_max, max] = _max;

    auto data_graph = ROOTHelpers::MakeDataGraph(m_total_data_histogram, m_scale_to_area, min_max);

    auto mc_uncert = ROOTHelpers::MakeErrorBand(m_total_mc_histogram, m_total_uncert, m_scale_to_area);

    auto [ratio_graph, ratio_mc_err_graph] =
        ROOTHelpers::MakeRatioGraph(m_total_data_histogram, m_total_mc_histogram, m_total_uncert, min_max);

    auto y_min = ROOTHelpers::GetYMin(m_total_mc_histogram, m_scale_to_area, min_max);
    auto y_max = ROOTHelpers::GetYMax(m_total_data_histogram, m_total_mc_histogram, m_scale_to_area, min_max);

    auto mc_histograms = m_histogram_per_process_group_and_shift.at("Nominal");
    if (m_scale_to_area)
    {
        for (auto &[pg, hist] : mc_histograms)
        {
            hist->Scale(min_bin_width, "width");
        }
    }

    return PlotProps{
        .class_name = m_event_class_name,
        .distribution_name = m_distribution_name,
        .x_min = min,
        .x_max = max,
        .y_min = y_min,
        .y_max = y_max,
        .total_data_histogram = m_total_data_histogram,
        .data_graph = data_graph,
        .mc_histograms = mc_histograms,
        .mc_uncertainty = mc_uncert,
        .ratio_graph = ratio_graph,
        .ratio_mc_error_band = ratio_mc_err_graph,
    };
}