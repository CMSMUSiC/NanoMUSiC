#include "Distribution.hpp"
#include "BS_thread_pool.hpp"

#include <algorithm>
#include <algorithm> // For std::shuffle
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fmt/core.h>
#include <fnmatch.h>
#include <future>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <random> // For std::random_device and std::mt19937
#include <regex>
#include <set>
#include <string>
#include <unordered_map>

#include "ROOT/RVec.hxx"
#include "TFile.h"
#include "TH1.h"
#include "TKey.h"
#include "TROOT.h"

#include "SerializationUtils.hpp"
#include "roothelpers.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;

Distribution::Distribution(const std::vector<ECHistogram> &event_class_histograms,
                           const std::string &event_class_name,
                           const std::string &distribution_name,
                           bool allow_rescale_by_width,
                           YearToPlot year_to_plot)
    : m_scale_to_area(allow_rescale_by_width),
      m_distribution_name(distribution_name),
      m_event_class_name(event_class_name),
      m_year_to_plot(year_to_string(year_to_plot))
{
    // split the histograms per process group and shift
    std::array<std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>, total_variations>
        unmerged_mc_histograms;
    for (const auto &histo : event_class_histograms)
    {
        unmerged_mc_histograms[histo.shift][histo.process_group].push_back(histo.histogram);
    }

    // merge unmerged_mc_histograms per process group and shift
    for (std::size_t shift = 0; shift < static_cast<std::size_t>(Shifts::Variations::kTotalVariations); shift++)
    {
        for (const auto &[pg, histos] : unmerged_mc_histograms[shift])
        {
            m_histogram_per_process_group_and_shift[shift][pg] = ROOTHelpers::SumAsTH1F(histos);
        }
    }

    // build Data histogram and grap
    m_total_data_histogram = TH1F(*(event_class_histograms.at(0).histogram));
    m_total_data_histogram.Reset();
    if (m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::Nominal))
            .find("Data") !=
        m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::Nominal)).end())
    {
        m_total_data_histogram =
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::Nominal))
                .at("Data");
    }

    // merge MC histograms
    m_total_mc_histogram = ROOTHelpers::SumAsTH1F(
        m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::Nominal)), true);

    // number of bins
    m_n_bins = m_total_mc_histogram.GetNbinsX();

    // statistical uncertainties - uncorrelated bewtween processes
    m_statistical_uncert = get_statistical_uncert();

    // systematic uncertainties
    m_systematics_uncert = get_systematics_uncert(unmerged_mc_histograms);

    // total uncertainties
    m_total_uncert =
        ROOT::VecOps::sqrt(ROOT::VecOps::pow(m_statistical_uncert, 2.) + ROOT::VecOps::pow(m_systematics_uncert, 2.));

    m_has_data = ROOT::VecOps::Sum(ROOTHelpers::Counts(m_total_data_histogram)) > 0.;
}

auto replace_all(std::string str, const std::string &from, const std::string &to) -> std::string
{
    if (from.empty())
    {
        return str;
    }
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Advance start_pos to the end of the replacement
    }

    return str;
}

auto Distribution::fold(const std::vector<std::string> &input_files,
                        const std::string &output_dir,
                        std::vector<std::string> &analyses_to_fold) -> void
{
    TH1::AddDirectory(false);
    TDirectory::AddDirectory(false);
    ROOT::EnableThreadSafety();

    fmt::print("[Distribution Folder] Starting pool: will launch {} threads ...\n", input_files.size());
    auto load_histograms_pool = BS::thread_pool(input_files.size());

    fmt::print("[Distribution Folder] Opening ROOT files and getting histograms...\n");
    std::vector<std::future<std::vector<ECHistogram>>> future_ec_histograms;
    std::atomic<int> file_counter = 0;
    for (std::size_t i = 0; i < input_files.size(); i++)
    {
        future_ec_histograms.push_back(load_histograms_pool.submit(
            [&](std::size_t i) -> std::vector<ECHistogram>
            {
                auto root_file = std::unique_ptr<TFile>(TFile::Open(input_files[i].c_str()));
                auto this_event_class_histograms = std::vector<ECHistogram>();
                this_event_class_histograms.reserve(root_file->GetListOfKeys()->GetSize());

                TIter keyList(root_file->GetListOfKeys());
                TKey *key;
                while ((key = (TKey *)keyList()))
                {
                    const std::string name = key->GetName();
                    const auto [analysis_name, //
                                process_group, //
                                xs_order,      //
                                sample,        //
                                year,          //
                                shift,         //
                                histo_name] = SerializationUtils::split_histo_name(name);

                    auto hist_ptr = std::shared_ptr<TH1F>(static_cast<TH1F *>(key->ReadObj()));
                    if (not(hist_ptr.get()) or hist_ptr->IsZombie())
                    {
                        fmt::print(
                            stderr, "ERROR: Could not load histogram for EventClass/Validation Analysis {}.\n", name);
                        std::exit(EXIT_FAILURE);
                    }

                    this_event_class_histograms.push_back(
                        ECHistogram{.analysis_name = analysis_name,
                                    .process_group = process_group,
                                    .xs_order = xs_order,
                                    .sample = sample,
                                    .year = year,
                                    .shift = static_cast<std::size_t>(Shifts::string_to_variation(shift)),
                                    .histo_name = histo_name,
                                    .histogram = hist_ptr});
                }

                file_counter++;
                fmt::print("Loaded ROOT file: {} - {}/{}\n", input_files[i], file_counter.load(), input_files.size());
                return this_event_class_histograms;
            },
            i));
    }

    fmt::print("[Distribution Folder] Harversting histograms ... \n");

    auto event_class_histograms = std::unordered_map<std::string, std::vector<ECHistogram>>();
    unsigned int collected_jobs = 0;
    for (auto &fut : future_ec_histograms)
    {
        auto this_histos = fut.get();
        for (const auto &ec_hist : this_histos)
        {
            event_class_histograms[ec_hist.analysis_name].push_back(ec_hist);
        }
        this_histos.clear();
        collected_jobs++;

        if (static_cast<std::size_t>(file_counter.load()) == input_files.size())
        {
            fmt::print("Harversted {} / {} files.\n", collected_jobs, future_ec_histograms.size());
        }
    }

    if (event_class_histograms.size() == 0)
    {
        fmt::print(stderr, "ERROR: Could not load any histogram for EventClass/Validation Analysis.\n");
        std::exit(EXIT_FAILURE);
    }

    unsigned int analysis_done = 0;
    for (const auto &[name, histograms] : event_class_histograms)
    {
        auto output_root_file = std::unique_ptr<TFile>(
            TFile::Open(fmt::format("{}/folded_histograms_{}.root", output_dir, replace_all(name, "+", "_")).c_str(),
                        "RECREATE",
                        name.c_str(),
                        0));

        for (auto &hist : histograms)
        {
            auto write_res = output_root_file->WriteObject(hist.histogram.get(), hist.histogram->GetName());
            if (write_res <= 0)
            {
                fmt::print(stderr, "ERROR: Could not write histogram to file: {}\n", hist.histogram->GetName());
                std::exit(EXIT_FAILURE);
            }
        }
        analysis_done++;
        fmt::print("Done: {} | {} / {}\n", name, analysis_done, event_class_histograms.size());
    }

    fmt::print("All done.\n");
}

auto Distribution::make_distributions(const std::string &input_file,
                                      const std::string &output_dir,
                                      std::string &analysis_to_plot,
                                      bool skip_per_year,
                                      const std::optional<std::unordered_map<std::string, double>> &rescaling) -> bool
{
    TH1::AddDirectory(false);
    TDirectory::AddDirectory(false);

    std::vector<std::future<std::vector<ECHistogram>>> future_ec_histograms;

    auto input_root_file = std::unique_ptr<TFile>(TFile::Open(input_file.c_str()));

    //  [distribution_name[year,ec_histograms]]]
    auto event_class_histograms =
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ECHistogram>>>();

    TIter keyList(input_root_file->GetListOfKeys());
    TKey *key;
    while ((key = (TKey *)keyList()))
    {
        const std::string name = key->GetName();
        const auto [analysis_name, //
                    process_group, //
                    xs_order,      //
                    sample,        //
                    year,          //
                    shift,         //
                    histo_name] = SerializationUtils::split_histo_name(name);

        auto hist_ptr = std::shared_ptr<TH1F>(static_cast<TH1F *>(key->ReadObj()));
        if (not(hist_ptr.get()))
        {
            fmt::print(stderr, "ERROR: Could not load histogram for EventClass/Validation Analysis {}.\n\n\n", name);
            return false;
        }

        const auto scaling = [&sample, &rescaling]() -> double
        {
            if (rescaling)
            {
                return (*rescaling).at(sample);
            }
            return 1.;
        }();

        if (scaling != 1.)
        {
            hist_ptr->Scale(scaling);
        }

        if (not(skip_per_year))
        {
            event_class_histograms[histo_name][year].push_back(
                ECHistogram{.analysis_name = analysis_name,
                            .process_group = process_group,
                            .xs_order = xs_order,
                            .sample = sample,
                            .year = year,
                            .shift = static_cast<std::size_t>(Shifts::string_to_variation(shift)),
                            .histo_name = histo_name,
                            .histogram = hist_ptr});
        }
        event_class_histograms[histo_name][year_to_string(YearToPlot::Run2)].push_back(
            ECHistogram{.analysis_name = analysis_name,
                        .process_group = process_group,
                        .xs_order = xs_order,
                        .sample = sample,
                        .year = year,
                        .shift = static_cast<std::size_t>(Shifts::string_to_variation(shift)),
                        .histo_name = histo_name,
                        .histogram = hist_ptr});
    }

    if (event_class_histograms.size() == 0)
    {
        fmt::print(stderr, "ERROR: Could not load any histogram for EventClass/Validation Analysis.\n\n\n");
        return false;
    }

    auto distributions = std::vector<Distribution>();
    for (const auto year_to_plot : years_to_plot())
    {
        for (const auto &[distribution_name, allow_rescale_by_width] : get_distribution_props(analysis_to_plot))
        {
            if (distribution_name == "met" and analysis_to_plot.find("1MET") == std::string::npos and
                analysis_to_plot.find("EC_") != std::string::npos)
            {
                continue;
            }

            if (event_class_histograms.contains("h_" + distribution_name))
            {
                if (event_class_histograms.at("h_" + distribution_name).contains(year_to_string(year_to_plot)))
                {
                    if (Distribution::is_valid(
                            event_class_histograms.at("h_" + distribution_name).at(year_to_string(year_to_plot))))
                    {
                        distributions.emplace_back(
                            event_class_histograms.at("h_" + distribution_name).at(year_to_string(year_to_plot)),
                            analysis_to_plot,
                            distribution_name,
                            allow_rescale_by_width,
                            year_to_plot);
                        // distributions.emplace_back(); }
                    }
                }
            }
        }
    }

    if (distributions.size() > 0)
    {
        auto output_root_file = std::unique_ptr<TFile>(TFile::Open(
            fmt::format(
                "{}/distribution_{}.root", output_dir, replace_all(distributions.at(0).m_event_class_name, "+", "_"))
                .c_str(),
            "RECREATE"));

        for (auto &dist : distributions)
        {
            auto write_res = output_root_file->WriteObject(
                &dist,
                fmt::format(
                    "distribution_{}_{}_{}", dist.m_event_class_name, dist.m_distribution_name, dist.m_year_to_plot)
                    .c_str());
            if (write_res <= 0)
            {
                fmt::print(stderr,
                           "ERROR: Could not write distribution to file: {} - {} - {}\n\n\n",
                           dist.m_event_class_name,
                           dist.m_distribution_name,
                           dist.m_year_to_plot);
                return false;
            }
        }
    }
    return true;
}

auto Distribution::get_statistical_uncert() -> RVec<double>
{
    auto statistical_uncertainties = RVec<double>(m_n_bins, 0.);

    for (const auto &[pg, histo] :
         m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::Nominal)))
    {
        if (pg != "Data")
        {
            statistical_uncertainties += ROOT::VecOps::pow(ROOTHelpers::Errors(histo), 2.);
        }
    }

    return ROOT::VecOps::sqrt(statistical_uncertainties);
}

auto Distribution::get_systematics_uncert(
    const std::array<std::unordered_map<std::string, std::vector<std::shared_ptr<TH1F>>>, total_variations>
        &unmerged_mc_histograms) -> RVec<double>
{
    std::unordered_map<std::string, RVec<double>> xsec_order_uncert_LO_samples;
    std::unordered_map<std::string, RVec<double>> xsec_order_uncert_non_LO_samples;
    std::unordered_map<std::string, RVec<double>> xsec_order_uncert_non_LO_samples_for_plotting;

    for (const auto &[pg, unmerged_histos] :
         unmerged_mc_histograms[static_cast<std::size_t>(Shifts::Variations::Nominal)])
    {
        if (pg != "Data")
        {
            if (not(xsec_order_uncert_LO_samples.contains(pg)))
            {
                xsec_order_uncert_LO_samples.insert({pg, RVec<double>(m_n_bins, 0.)});
                xsec_order_uncert_non_LO_samples.insert({pg, RVec<double>(m_n_bins, 0.)});
                xsec_order_uncert_non_LO_samples_for_plotting.insert({pg, RVec<double>(m_n_bins, 0.)});
            }

            for (std::size_t i = 0; i < unmerged_histos.size(); i++)
            {
                const auto [class_name,    //
                            process_group, //
                            xs_order,      //
                            sample,        //
                            year,          //
                            shift,         //
                            histo_name] = SerializationUtils::split_histo_name(unmerged_histos[i]->GetName());

                if (xs_order == "LO")
                {
                    xsec_order_uncert_LO_samples.at(pg) += ROOTHelpers::Counts(*(unmerged_histos[i])) * 0.5;
                }
                // NLO case
                else
                {
                    auto uncert_qcd_scale = Uncertanties::AbsDiffAndSymmetrize(
                        *(unmerged_histos[i]),
                        *unmerged_mc_histograms[static_cast<std::size_t>(Shifts::Variations::QCDScale_Up)].at(pg).at(i),
                        *unmerged_mc_histograms[static_cast<std::size_t>(Shifts::Variations::QCDScale_Down)].at(pg).at(
                            i));
                    auto uncert_qcd_scale_for_plotting = Uncertanties::AbsDiffAndSymmetrizeForPlots(
                        *(unmerged_histos[i]),
                        *unmerged_mc_histograms[static_cast<std::size_t>(Shifts::Variations::QCDScale_Up)].at(pg).at(i),
                        *unmerged_mc_histograms[static_cast<std::size_t>(Shifts::Variations::QCDScale_Down)].at(pg).at(
                            i));
                    auto nominal_counts = ROOTHelpers::Counts(*(unmerged_histos[i]));

                    for (std::size_t i = 0; i < uncert_qcd_scale.size(); i++)
                    {
                        constexpr double qcd_scale_uncert_upper_limit = 0.15;
                        uncert_qcd_scale[i] =
                            std::min(uncert_qcd_scale[i], std::fabs(nominal_counts[i] * qcd_scale_uncert_upper_limit));
                        uncert_qcd_scale_for_plotting[i] =
                            std::min(uncert_qcd_scale_for_plotting[i],
                                     std::fabs(nominal_counts[i] * qcd_scale_uncert_upper_limit));
                    }

                    xsec_order_uncert_non_LO_samples.at(pg) += uncert_qcd_scale;
                    xsec_order_uncert_non_LO_samples_for_plotting.at(pg) += uncert_qcd_scale_for_plotting;
                }
            }
        }
    }

    // auto xsec_uncert = RVec<double>(m_n_bins, 0.);
    // for (const auto &[pg, uncert] : xsec_order_uncert_LO_samples)
    // {
    //     xsec_uncert += ROOT::VecOps::pow(xsec_order_uncert_LO_samples.at(pg), 2.) +
    //                    ROOT::VecOps::pow(xsec_order_uncert_non_LO_samples.at(pg), 2.);
    //     // fmt::print("=== EC: {} - DIST: {} - PG: {} -- [{}]\n",
    //     //    m_event_class_name,
    //     //    m_distribution_name,
    //     //    pg,
    //     //    fmt::join(xsec_order_uncert_non_LO_samples.at(pg), ", "));
    // }
    // xsec_uncert = ROOT::VecOps::sqrt(xsec_uncert);

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
    //                                                       unmerged_mc_histograms.at("Nominal").at(pg).at(i))
    //                                                       /
    //                                     (ROOTHelpers::Counts(unmerged_mc_histograms.at("Nominal").at(pg).at(i))
    //                                     + 1e-6),
    //                                 ", "));
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    // will cap the PDF+As uncertainties
    constexpr auto pdf_as_upper_limit = 0.3;
    auto capped_pdf_as_uncert = ROOTHelpers::Counts(m_total_mc_histogram) * pdf_as_upper_limit;
    auto pdf_as_uncert = Uncertanties::AbsDiff(m_total_mc_histogram,
                                               ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                   static_cast<std::size_t>(Shifts::Variations::PDF_As_Up))));

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

    auto extra_jets_uncert = [](const std::string &str) -> double
    {
        std::regex bjet_regex(R"((\d+)bJet)");
        std::regex jet_regex(R"((\d+)Jet(?!_))");

        std::smatch match;
        int bjet_value = 0;
        int jet_value = 0;

        // Extract bJet value
        if (std::regex_search(str, match, bjet_regex))
        {
            bjet_value = std::stoi(match[1].str());
        }

        // Extract Jet value (but not bJet)
        if (std::regex_search(str, match, jet_regex))
        {
            jet_value = std::stoi(match[1].str());
        }

        // return std::make_pair(bjet_value, jet_value);
        auto extra_jets = 0;
        if (bjet_value >= 2)
        {
            extra_jets += bjet_value - 1;
        }
        if (jet_value >= 4)
        {
            extra_jets += jet_value - 3;
        }

        return extra_jets * 0.1;
    };

    m_systematics_uncertainties = {
        ////////////////////////////////////
        // Integrals
        ////////////////////////////////////
        // trigger
        {"trigger", Uncertanties::IntegralUncert(m_total_mc_histogram, 0.05)},

        // luminosity
        // https://twiki.cern.ch/twiki/bin/view/CMS/LumiRecommendationsRun2#Quick_summary_table
        {"luminosity", Uncertanties::IntegralUncert(m_total_mc_histogram, 0.016)},

        // extra_jets
        // {"extra_jets", Uncertanties::IntegralUncert(m_total_mc_histogram, extra_jets_uncert(m_event_class_name))},

        ////////////////////////////////////
        // Constants
        ////////////////////////////////////
        // fakes
        {"fakes",
         Uncertanties::AbsDiff(m_total_mc_histogram,
                               ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                   static_cast<std::size_t>(Shifts::Variations::Fakes_Up))))},

        // PDF + Alpha_S
        // Uncertanties::AbsDiff(m_total_mc_histogram,
        //   ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PDF_As_Up"))),
        // {"pdf_alphas", ROOTHelpers::Counts(m_total_mc_histogram) * 0.1},
        {"pdf_alphas", capped_pdf_as_uncert},

        // Prefiring
        {"prefiring",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PreFiring_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PreFiring_Down))))},

        // Scale Factors
        {"scale_factors",
         Uncertanties::AbsDiff(m_total_mc_histogram,
                               ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                   static_cast<std::size_t>(Shifts::Variations::ScaleFactor_Up))))},

        // Pile up
        {"pile_up",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PU_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PU_Down))))},

        ////////////////////////////////////
        // Differential
        ////////////////////////////////////
        // Electron Resolution
        {"electron_resolution",
         Uncertanties::AbsDiffAndSymmetrize(
             m_total_mc_histogram,
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::ElectronResolution_Up))),
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::ElectronResolution_Down))))},

        // Electron Scale
        {"electron_scale",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::ElectronScale_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::ElectronScale_Down))))},

        // Photon Resolution
        {"photon_resolution",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonResolution_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonResolution_Down))))},

        // Photon Scale
        {"photon_scale",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonScale_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonScale_Down))))},

        // Tau Energy
        {"tau_energy",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauEnergy_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauEnergy_Down))))},

        // Jet Resolution
        {"jet_resolution",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::JetResolution_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::JetResolution_Down))))},

        // Jet Scale
        {"jet_scale",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::JetScale_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::JetScale_Down))))},

        // MET Unclustered Energy
        {"met_unclustered_energy",
         Uncertanties::AbsDiffAndSymmetrize(
             m_total_mc_histogram,
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::UnclusteredEnergy_Up))),
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::UnclusteredEnergy_Down))))},
    };

    // XSecOrder + QCD Scale
    for (const auto &[pg, uncert] : xsec_order_uncert_LO_samples)
    {
        m_systematics_uncertainties[fmt::format("xsec_LO_{}", pg)] = xsec_order_uncert_LO_samples.at(pg);
    }

    for (const auto &[pg, uncert] : xsec_order_uncert_non_LO_samples)
    {
        m_systematics_uncertainties[fmt::format("xsec_non_LO_{}", pg)] = xsec_order_uncert_non_LO_samples.at(pg);
    }

    // build systematics for plotting (taking the symmatriation conservative approach)
    auto systematics_for_plots_and_integral_p_value = m_systematics_uncertainties;
    systematics_for_plots_and_integral_p_value["prefiring"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PreFiring_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PreFiring_Down))));

    systematics_for_plots_and_integral_p_value["pile_up"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PU_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PU_Down))));

    systematics_for_plots_and_integral_p_value["electron_resolution"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::ElectronResolution_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::ElectronResolution_Down))));

    systematics_for_plots_and_integral_p_value["electron_scale"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::ElectronScale_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::ElectronScale_Down))));

    systematics_for_plots_and_integral_p_value["photon_resolution"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::PhotonResolution_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::PhotonResolution_Down))));

    systematics_for_plots_and_integral_p_value["photon_scale"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PhotonScale_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::PhotonScale_Down))));

    systematics_for_plots_and_integral_p_value["tau_energy"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::TauEnergy_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::TauEnergy_Down))));

    systematics_for_plots_and_integral_p_value["jet_resolution"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::JetResolution_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::JetResolution_Down))));

    systematics_for_plots_and_integral_p_value["jet_scale"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::JetScale_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::JetScale_Down))));

    systematics_for_plots_and_integral_p_value["met_unclustered_energy"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::UnclusteredEnergy_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::UnclusteredEnergy_Down))));

    for (const auto &[pg, uncert] : xsec_order_uncert_non_LO_samples_for_plotting)
    {
        m_systematics_uncertainties[fmt::format("xsec_non_LO_{}", pg)] =
            xsec_order_uncert_non_LO_samples_for_plotting.at(pg);
    }

    return ROOT::VecOps::sqrt(std::accumulate(
        systematics_for_plots_and_integral_p_value.cbegin(),
        systematics_for_plots_and_integral_p_value.cend(),
        RVec<double>(m_n_bins, 0.),
        [](const RVec<double> &acc_vec, const std::pair<std::string, RVec<double>> &next_key_val) -> RVec<double>
        {
            const auto &[src, next_vec] = next_key_val;

            // if (m_year_to_plot == "Run2" and m_distribution_name == "counts")
            // {
            //     fmt::print("--- {} - {} - {} - {}: [{}]\n",
            //                m_distribution_name,
            //                m_event_class_name,
            //                m_year_to_plot,
            //                src,
            //                fmt::join(next_vec, " - "));
            // }

            return acc_vec + ROOT::VecOps::pow(next_vec, 2.);
        }));
}

auto Distribution::make_integral_pvalue_props() -> IntegralPValueProps
{
    //  Sanity check
    if (m_distribution_name != "counts")
    {
        fmt::print(stderr,
                   "WARNING: Could not get P-Value props for a histogram that is not \"Counts\". Using the returned "
                   "values "
                   "will cause undefined behavior.\n");
        return IntegralPValueProps{
            .total_data = -1., .total_mc = -1., .sigma_total = -1., .sigma_stat = -1., .total_per_process_group = {}};
    }

    // auto total_per_process_group = std::vector<double>();
    // for (const auto &[pg, hist] :
    //      m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::Nominal)))
    // {
    //     if (pg != "Data")
    //     {
    //         total_per_process_group.push_back(hist.GetBinContent(1));
    //     }
    // }

    return IntegralPValueProps{
        .total_data = m_total_data_histogram.GetBinContent(1), //
        .total_mc = m_total_mc_histogram.GetBinContent(1),     //
        .sigma_total = m_total_uncert.at(0),                   //
        .sigma_stat = m_statistical_uncert.at(0),              //
        .total_per_process_group = {}                          //
    };
}

auto Distribution::make_plot_props() -> PlotProps
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

    std::unordered_map<std::string, TH1F> mc_histograms;
    for (auto &[pg, hist] :
         m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::Nominal)))
    {
        if (m_scale_to_area)
        {
            hist.Scale(min_bin_width, "width");
        }
        mc_histograms[pg] = hist;
    }

    return PlotProps{
        .class_name = m_event_class_name,
        .distribution_name = m_distribution_name,
        .year_to_plot = m_year_to_plot,
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
