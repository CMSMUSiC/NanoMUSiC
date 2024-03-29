#include "Distribution.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fmt/core.h>
#include <fnmatch.h>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>

#include "ROOT/RVec.hxx"
#include "TCollection.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TKey.h"
#include "TLegend.h"

#include "NanoEventClass.hpp"
#include "fmt/format.h"
#include "indicators.hpp"
#include "roothelpers.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;

Distribution::Distribution(const NanoEventClass &ec, const std::string &distribution_name, bool allow_rescale_by_width)
    : m_scale_to_area(distribution_name != "counts" and allow_rescale_by_width),
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
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>>
        unmerged_mc_histograms;
    for (auto &&histo : *event_class_histograms)
    {
        if (unmerged_mc_histograms.find(histo.shift) == unmerged_mc_histograms.end())
        {
            unmerged_mc_histograms[histo.shift] = std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>();
        }
        unmerged_mc_histograms[histo.shift][histo.process_group].push_back(histo.histogram);
    }

    // merge unmerged_mc_histograms per process group and shift
    for (auto &&[shift, histos_per_process_group] : unmerged_mc_histograms)
    {
        if (m_histogram_per_process_group_and_shift.find(shift) == m_histogram_per_process_group_and_shift.end())
        {
            m_histogram_per_process_group_and_shift[shift] = std::unordered_map<std::string, std::shared_ptr<TH1F>>();
        }

        for (auto &&[pg, histos] : unmerged_mc_histograms[shift])
        {
            m_histogram_per_process_group_and_shift[shift][pg] = ROOTHelpers::SumAsTH1F(histos);
        }
    }

    // build Data histogram and graph
    m_total_data_histogram =
        std::shared_ptr<TH1F>(static_cast<TH1F *>(event_class_histograms->at(0).histogram->Clone()));
    m_total_data_histogram->Reset();
    if (m_histogram_per_process_group_and_shift.at("Nominal").find("Data") !=
        m_histogram_per_process_group_and_shift.at("Nominal").end())
    {
        m_total_data_histogram = m_histogram_per_process_group_and_shift.at("Nominal").at("Data");
    }

    // merge MC histograms
    m_total_mc_histogram = ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("Nominal"));

    // number of bins
    m_n_bins = m_total_mc_histogram->GetNbinsX();

    // statistical uncertainties - uncorrelated bewtween processes
    m_statistical_uncert = get_statistical_uncert();

    // systematic uncertainties
    m_systematics_uncert = get_systematics_uncert(unmerged_mc_histograms);

    // total uncertainties
    m_total_uncert =
        ROOT::VecOps::sqrt(ROOT::VecOps::pow(m_statistical_uncert, 2.) + ROOT::VecOps::pow(m_systematics_uncert, 2.));
}

auto Distribution::get_statistical_uncert() const -> RVec<double>
{
    auto statistical_uncertainties = RVec<double>(m_n_bins, 0.);

    for (const auto &[pg, histo] : m_histogram_per_process_group_and_shift.at("Nominal"))
    {
        if (pg != "Data")
        {
            statistical_uncertainties += ROOT::VecOps::pow(ROOTHelpers::Errors(histo), 2.);
        }
    }

    return ROOT::VecOps::sqrt(statistical_uncertainties);
}

auto Distribution::get_systematics_uncert(
    const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>>
        &unmerged_mc_histograms) const -> RVec<double>
{
    std::unordered_map<std::string, RVec<double>> xsec_order_uncert_LO_samples;
    std::unordered_map<std::string, RVec<double>> xsec_order_uncert_non_LO_samples;

    for (const auto &[shift, histo_per_pg] : unmerged_mc_histograms)
    {
        if (shift == "Nominal")
        {
            for (const auto &[pg, unmerged_histos] : histo_per_pg)
            {
                if (pg != "Data")
                {
                    if (xsec_order_uncert_LO_samples.find(pg) == xsec_order_uncert_LO_samples.end())
                    {
                        xsec_order_uncert_LO_samples.insert({pg, RVec<double>(m_n_bins, 0.)});
                        xsec_order_uncert_non_LO_samples.insert({pg, RVec<double>(m_n_bins, 0.)});
                    }

                    for (std::size_t i = 0; i < unmerged_histos.size(); i++)
                    {
                        const auto [class_name, process_group, xs_order, sample, year, shift, histo_name] =
                            NanoEventHisto::split_histo_name(unmerged_histos[i]->GetName());
                        if (xs_order == "LO")
                        {
                            xsec_order_uncert_LO_samples.at(pg) += ROOTHelpers::Counts(unmerged_histos[i]) * 0.5;
                        }
                        else
                        {
                            auto uncert_qcd_scale = Uncertanties::AbsDiffAndSymmetrize(
                                unmerged_histos[i],
                                unmerged_mc_histograms.at("QCDScale_Up").at(pg).at(i),
                                unmerged_mc_histograms.at("QCDScale_Down").at(pg).at(i));
                            auto nominal_counts = ROOTHelpers::Counts(unmerged_histos[i]);

                            for (std::size_t i = 0; i < uncert_qcd_scale.size(); i++)
                            {
                                uncert_qcd_scale[i] = std::min(uncert_qcd_scale[i], std::fabs(nominal_counts[i] * 0.3));
                            }

                            xsec_order_uncert_non_LO_samples.at(pg) += uncert_qcd_scale;
                        }

                        //////////////
                        //////////////
                        ///// DEBUG!!!
                        // if (histo_name == "h_counts")
                        // {
                        //     fmt::print(
                        //         "-- {} - {} - [{}]\n",
                        //         sample,
                        //         year,
                        //         fmt::join(Uncertanties::AbsDiff(unmerged_histos[i],
                        //                                         unmerged_mc_histograms.at("PDF_As_Up").at(pg).at(i))
                        //                                         /
                        //                       ROOTHelpers::Counts(unmerged_histos[i]),
                        //                   ", "));
                        // }
                        //////////////
                        //////////////
                    }
                }
            }
        }
    }

    auto xsec_uncert = RVec<double>(m_n_bins, 0.);
    for (const auto &[pg, uncert] : xsec_order_uncert_LO_samples)
    {
        xsec_uncert += ROOT::VecOps::pow(xsec_order_uncert_LO_samples.at(pg), 2.) +
                       ROOT::VecOps::pow(xsec_order_uncert_non_LO_samples.at(pg), 2.);
        // fmt::print("=== EC: {} - DIST: {} - PG: {} -- [{}]\n",
        //    m_event_class_name,
        //    m_distribution_name,
        //    pg,
        //    fmt::join(xsec_order_uncert_non_LO_samples.at(pg), ", "));
    }
    xsec_uncert = ROOT::VecOps::sqrt(xsec_uncert);

    // std::cout << "Fakes error: "
    //           << Uncertanties::AbsDiff(m_total_mc_histogram,
    //                                    ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("Fakes_Up")))
    //           << std::endl;
    // std::cout << "PDF+As error: "
    //           << Uncertanties::AbsDiff(m_total_mc_histogram,
    //                                    ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PDF_As_Up")))
    //           << std::endl;
    // for (auto &&[pg, h] : m_histogram_per_process_group_and_shift.at("PDF_As_Up"))
    // {
    //     fmt::print("PG:{} - [{}]\n", pg, fmt::join(ROOTHelpers::Counts(h), ", "));
    // }
    // if (m_distribution_name == "counts")
    // {
    //     for (const auto &[shift, histo_per_pg] : unmerged_mc_histograms)
    //     {
    //         if (shift == "Nominal")
    //         {
    //             for (const auto &[pg, unmerged_histos] : histo_per_pg)
    //             {
    //                 if (pg != "Data")
    //                 {
    //                     for (std::size_t i = 0; i < unmerged_histos.size(); i++)
    //                     {
    //                         fmt::print(
    //                             "{} \n[{}]\n",
    //                             unmerged_mc_histograms.at("PDF_As_Up").at(pg).at(i)->GetName(),
    //                             fmt::join(
    //                                 Uncertanties::AbsDiff(unmerged_mc_histograms.at("PDF_As_Up").at(pg).at(i),
    //                                                       unmerged_mc_histograms.at("Nominal").at(pg).at(i)) /
    //                                     (ROOTHelpers::Counts(unmerged_mc_histograms.at("Nominal").at(pg).at(i)) +
    //                                     1e-6),
    //                                 ", "));
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    // will cap the PDF+As uncertainties
    constexpr auto pdf_as_upper_limit = 0.7;
    auto capped_pdf_as_uncert = ROOTHelpers::Counts(m_total_mc_histogram) * pdf_as_upper_limit;
    auto pdf_as_uncert = Uncertanties::AbsDiff(
        m_total_mc_histogram, ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PDF_As_Up")));

    // sanity check
    if (capped_pdf_as_uncert.size() != pdf_as_uncert.size())
    {
        fmt::print(stderr, "ERROR: Capped and nominal PDF+As uncertainties have different sizes.\n");
        std::exit(EXIT_FAILURE);
    }

    for (std::size_t i = 0; i < capped_pdf_as_uncert.size(); i++)
    {
        capped_pdf_as_uncert[i] = std::min(capped_pdf_as_uncert[i], pdf_as_uncert[i]);
    }

    auto systematics_uncertainties = {
        ////////////////////////////////////
        // Integrals
        ////////////////////////////////////
        // trigger
        Uncertanties::IntegralUncert(m_total_mc_histogram, 0.03),

        // luminosity
        Uncertanties::IntegralUncert(m_total_mc_histogram, 0.025),

        // XSecOrder + QCD Scale
        xsec_uncert,

        ////////////////////////////////////
        // Constants
        ////////////////////////////////////
        // fakes
        Uncertanties::AbsDiff(m_total_mc_histogram,
                              ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("Fakes_Up"))),

        // PDF + Alpha_S
        // Uncertanties::AbsDiff(m_total_mc_histogram,
        //   ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PDF_As_Up"))),
        ROOTHelpers::Counts(m_total_mc_histogram) * 0.1,
        // capped_pdf_as_uncert,

        // Prefiring
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PreFiring_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PreFiring_Down"))),

        // Scale Factors
        Uncertanties::AbsDiff(m_total_mc_histogram,
                              ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("ScaleFactor_Up"))),

        // Pile up
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PU_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PU_Down"))),

        ////////////////////////////////////
        // Differential
        ////////////////////////////////////
        // Electron Resolution
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("ElectronResolution_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("ElectronResolution_Down"))),

        // Electron Scale
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("ElectronScale_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("ElectronScale_Down"))),

        // Photon Resolution
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PhotonResolution_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PhotonResolution_Down"))),

        // Photon Scale
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PhotonScale_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PhotonScale_Down"))),

        // Tau Energy
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("TauEnergy_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("TauEnergy_Down"))),

        // Jet Resolution
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("JetResolution_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("JetResolution_Down"))),

        // Jet Scale
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("JetScale_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("JetScale_Down"))),

        // MET Unclustered Energy
        Uncertanties::AbsDiffAndSymmetrize(
            m_total_mc_histogram,
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("UnclusteredEnergy_Up")),
            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("UnclusteredEnergy_Down"))),
    };

    return ROOT::VecOps::sqrt( //
        std::accumulate(systematics_uncertainties.begin(),
                        systematics_uncertainties.end(),
                        RVec<double>(m_n_bins, 0.),
                        [](const RVec<double> &acc_vec, const RVec<double> &next_vec) -> RVec<double>
                        {
                            return acc_vec + ROOT::VecOps::pow(next_vec, 2.);
                        }) //
    );
}

auto Distribution::serialize() const -> std::string
{
    return fmt::format("EC: {}\nDist: {}", m_event_class_name, m_distribution_name);
}

auto Distribution::get_integral_pvalue_props() const -> IntegralPValueProps
{
    //  Sanity check
    if (m_distribution_name != "counts")
    {
        fmt::print(
            "WARNING: Could not get P-Value props for a histogram that is not \"Counts\". Using the returned values "
            "will cause undefined behavior.\n");
        return IntegralPValueProps{
            .total_data = -1., .total_mc = -1., .sigma_total = -1., .sigma_stat = -1., .total_per_process_group = {}};
    }

    auto total_per_process_group = std::vector<double>();
    for (const auto &[pg, hist] : m_histogram_per_process_group_and_shift.at("Nominal"))
    {
        if (pg != "Data")
        {
            total_per_process_group.push_back(hist->GetBinContent(1));
        }
    }
    return IntegralPValueProps{
        .total_data = m_total_data_histogram->GetBinContent(1), //
        .total_mc = m_total_mc_histogram->GetBinContent(1),     //
        .sigma_total = m_total_uncert.at(0),                    //
        .sigma_stat = m_statistical_uncert.at(0),               //
        .total_per_process_group = {}                           //
    };
}

auto Distribution::get_plot_props() const -> PlotProps
{
    auto min_max = ROOTHelpers::GetMinMax(m_total_data_histogram, m_total_mc_histogram);
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
