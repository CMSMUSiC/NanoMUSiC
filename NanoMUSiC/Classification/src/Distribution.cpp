#include "Distribution.hpp"
#include "BS_thread_pool.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fnmatch.h>
#include <future>
#include <memory>
#include <numeric>
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
    std::array<std::unordered_map<std::string, std::vector<std::shared_ptr<TKey>>>, total_variations>
        unmerged_mc_histograms;
    for (auto &&histo : event_class_histograms)
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
    auto total_data_histogram_ptr =
        std::unique_ptr<TH1F>(static_cast<TH1F *>(event_class_histograms.at(0).histogram->ReadObj()));
    m_total_data_histogram = TH1F(*(total_data_histogram_ptr));
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
    auto _systematics_uncert = get_systematics_uncert(unmerged_mc_histograms);
    m_systematics_uncert = std::get<0>(_systematics_uncert);
    m_systematics_uncert_for_plotting = std::get<1>(_systematics_uncert);

    // total uncertainties
    m_total_uncert =
        ROOT::VecOps::sqrt(ROOT::VecOps::pow(m_statistical_uncert, 2.) + ROOT::VecOps::pow(m_systematics_uncert, 2.));

    m_total_uncert_for_plotting = ROOT::VecOps::sqrt(ROOT::VecOps::pow(m_statistical_uncert, 2.) +
                                                     ROOT::VecOps::pow(m_systematics_uncert_for_plotting, 2.));

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
struct TempHist
{
    std::string analysis_name;
    std::string input_file;
    std::unique_ptr<TKey> hist_ptr;
};

auto Distribution::fold(const std::vector<std::string> &input_files, const std::string &output_dir) -> bool
{
    TH1::AddDirectory(false);
    TDirectory::AddDirectory(false);
    ROOT::EnableThreadSafety();

    fmt::print("[Distribution Folder] Loading input files ...\n", input_files.size());

    std::vector<std::future<std::pair<std::string, std::unique_ptr<TFile>>>> future_loaded_files;
    std::atomic<unsigned int> file_loader_counter = 0;
    {
        fmt::print("[Distribution Folder] Starting pool: will launch {} threads ...\n", input_files.size());
        auto load_root_files_pool = BS::thread_pool(input_files.size());
        for (const auto &input_file : input_files)
        {
            future_loaded_files.push_back(load_root_files_pool.submit(
                [&]() -> std::pair<std::string, std::unique_ptr<TFile>>
                {
                    auto file_ptr = std::unique_ptr<TFile>(TFile::Open(input_file.c_str()));
                    fmt::print("Loaded: {} - {}/{}\n", input_file, file_loader_counter.load(), input_files.size());
                    file_loader_counter++;
                    return {input_file, std::move(file_ptr)};
                }));
        }
    }

    std::unordered_map<std::string, std::unique_ptr<TFile>> root_files;
    for (auto &&fut : future_loaded_files)
    {
        auto [input_file, file_ptr] = fut.get();

        root_files[input_file] = std::move(file_ptr);
    }

    fmt::print("[Distribution Folder] Opening ROOT files and getting histograms...\n");
    std::vector<std::future<std::vector<TempHist>>> future_ec_histograms;
    std::atomic<int> file_counter = 0;

    {
        fmt::print("[Distribution Folder] Starting pool: will launch {} threads ...\n", input_files.size());
        auto load_histograms_pool = BS::thread_pool(input_files.size());
        for (std::size_t i = 0; i < input_files.size(); i++)
        {
            future_ec_histograms.push_back(load_histograms_pool.submit(
                [&](std::size_t i) -> std::vector<TempHist>
                {
                    auto this_event_class_histograms = std::vector<TempHist>();

                    TIter keyList(root_files[input_files[i]]->GetListOfKeys());
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

                        auto hist_ptr = std::unique_ptr<TKey>(key);
                        if (not(hist_ptr.get()) or hist_ptr->IsZombie())
                        {
                            throw std::runtime_error(
                                fmt::format("Could not load histogram for EventClass/Validation Analysis {}.\n", name));
                        }

                        this_event_class_histograms.push_back({.analysis_name = analysis_name,
                                                               .input_file = input_files[i],
                                                               .hist_ptr = std::move(hist_ptr)});
                    }

                    file_counter++;
                    fmt::print("Collected histograms for: {} - {}/{}\n",
                               input_files[i],
                               file_counter.load(),
                               input_files.size());
                    return this_event_class_histograms;
                },
                i));
        }
    }

    fmt::print("[Distribution Folder] Harversting histograms ... \n");

    auto event_class_histograms = std::unordered_map<std::string, std::vector<TempHist>>();
    unsigned int collected_jobs = 0;
    for (auto &&fut : future_ec_histograms)
    {
        auto this_histos = fut.get();
        for (auto &&temp_hist : this_histos)
        {
            event_class_histograms[temp_hist.analysis_name].push_back(std::move(temp_hist));
        }
        collected_jobs++;

        if (static_cast<std::size_t>(file_counter.load()) == input_files.size())
        {
            fmt::print("Harversted {} / {} files.\n", collected_jobs, future_ec_histograms.size());
        }
    }

    if (event_class_histograms.size() == 0)
    {
        throw std::runtime_error(" Could not load any histogram for EventClass/Validation Analysis.\n");
    }

    fmt::print("[Distribution Folder] Opening ROOT files and getting histograms...\n");
    unsigned int analysis_done = 0;
    for (auto &&[name, histograms] : event_class_histograms)
    {
        auto output_root_file = std::unique_ptr<TFile>(
            TFile::Open(fmt::format("{}/folded_histograms_{}.root", output_dir, replace_all(name, "+", "_")).c_str(),
                        "RECREATE",
                        name.c_str(),
                        0));

        auto total_histograms_per_class = 0;
        for (std::size_t i = 0; i < histograms.size(); i++)
        {
            auto this_hist = std::unique_ptr<TH1F>(static_cast<TH1F *>(histograms[i].hist_ptr->ReadObj()));
            auto write_res = output_root_file->WriteObject(this_hist.get(), this_hist->GetName());
            if (write_res <= 0)
            {
                throw std::runtime_error(fmt::format("Could not write histogram to file: {}\n", this_hist->GetName()));
            }

            [[maybe_unused]] volatile auto _ = histograms[i].hist_ptr.release(); // prevents optimization

            total_histograms_per_class++;
        }

        analysis_done++;

        constexpr auto extra_padding = [](const auto &name) -> std::string
        {
            if (name.find("+NJet") != std::string::npos)
            {
                return "";
            }
            else if (name.find("+X") != std::string::npos)
            {
                return "   ";
            }
            else
            {
                return "     ";
            }
        };

        fmt::print("Done: {}{} ({} histograms) | {} / {}\n",
                   name,
                   extra_padding(name),
                   total_histograms_per_class,
                   analysis_done,
                   event_class_histograms.size());
    }

    fmt::print("All done.\n");

    return true;
}

auto Distribution::make_distributions(const std::string &input_file,
                                      const std::string &output_dir,
                                      std::string &analysis_to_plot,
                                      bool skip_per_year) -> bool
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

        auto hist_ptr = std::shared_ptr<TKey>(key);
        if (not(hist_ptr.get()))
        {
            throw std::runtime_error(
                fmt::format("Could not load histogram for EventClass/Validation Analysis {}.\n\n\n", name));
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
        throw std::runtime_error(" Could not load any histogram for EventClass/Validation Analysis.\n\n\n");
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

                    for (auto &&hist :
                         event_class_histograms.at("h_" + distribution_name).at(year_to_string(year_to_plot)))
                    {
                        hist.histogram.reset();
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
    const std::array<std::unordered_map<std::string, std::vector<std::shared_ptr<TKey>>>, total_variations>
        &unmerged_mc_histograms) -> std::tuple<RVec<double>, RVec<double>>
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

    // will cap the PDF+As uncertainties
    constexpr auto pdf_as_upper_limit = 0.3;
    auto capped_pdf_as_uncert = ROOTHelpers::Counts(m_total_mc_histogram) * pdf_as_upper_limit;
    auto pdf_as_uncert = Uncertanties::AbsDiff(m_total_mc_histogram,
                                               ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                   static_cast<std::size_t>(Shifts::Variations::PDF_As_Up))));

    // sanity check
    if (capped_pdf_as_uncert.size() != pdf_as_uncert.size())
    {
        throw std::runtime_error("Capped and nominal PDF+As uncertainties have different sizes.\n");
    }

    for (std::size_t i = 0; i < capped_pdf_as_uncert.size(); i++)
    {
        capped_pdf_as_uncert[i] = std::min(capped_pdf_as_uncert[i], pdf_as_uncert[i]);
    }

    constexpr auto extra_jets_uncert = [](const std::string &str) -> double
    {
        std::optional<int> bJet_count;
        std::optional<int> jet_count;

        auto extract_number_before = [&](std::string_view key) -> std::optional<int>
        {
            auto pos = str.find(key);
            if (pos == std::string_view::npos || pos == 0)
                return std::nullopt;

            // Walk backwards to find the number
            size_t end = pos;
            size_t start = end;
            while (start > 0 && std::isdigit(str[start - 1]))
            {
                --start;
            }

            if (start == end)
                return std::nullopt;

            int value;
            auto result = std::from_chars(str.data() + start, str.data() + end, value);
            if (result.ec == std::errc())
            {
                return value;
            }
            return std::nullopt;
        };

        bJet_count = extract_number_before("bJet");

        // To avoid double-counting the same number for Jet and bJet
        // We search for "Jet" only when it's NOT preceded by 'b'
        size_t pos = 0;
        while ((pos = str.find("Jet", pos)) != std::string_view::npos)
        {
            if (pos == 0 || str[pos - 1] != 'b')
            {
                // Found "Jet" not preceded by 'b'
                size_t end = pos;
                size_t start = end;
                while (start > 0 && std::isdigit(str[start - 1]))
                {
                    --start;
                }

                if (start != end)
                {
                    int value;
                    auto result = std::from_chars(str.data() + start, str.data() + end, value);
                    if (result.ec == std::errc())
                    {
                        jet_count = value;
                    }
                }
            }
            ++pos;
        }

        auto extra_jets = 0;
        if (bJet_count)
        {
            if (*bJet_count >= 2)
            {
                extra_jets += *bJet_count - 1;
            }
        }

        if (jet_count)
        {
            if (*jet_count >= 4)
            {
                extra_jets += *jet_count - 3;
            }
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
        {"jet_extra", Uncertanties::IntegralUncert(m_total_mc_histogram, extra_jets_uncert(m_event_class_name))},

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
        {"muon_reco",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::MuonReco_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::MuonReco_Down))))},

        {"muon_id",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::MuonId_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::MuonId_Down))))},

        {"muon_iso",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::MuonIso_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::MuonIso_Down))))},

        {"electron_reco",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::ElectronReco_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::ElectronReco_Down))))},

        {"electron_id",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::ElectronId_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::ElectronId_Down))))},

        {"photon_id",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonId_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonId_Down))))},

        {"photon_veto",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonVeto_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonVeto_Down))))},

        {"tau_vs_e",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauVsE_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauVsE_Down))))},

        {"tau_vs_mu",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauVsMu_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauVsMu_Down))))},

        {"tau_vs_jet",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauVsJet_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauVsJet_Down))))},

        {"jet_btag",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::JetBTag_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::JetBTag_Down))))},

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
                 static_cast<std::size_t>(Shifts::Variations::ElectronDiffResolution_Up))),
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::ElectronDiffResolution_Down))))},

        // Electron Scale
        {"electron_scale",
         Uncertanties::AbsDiffAndSymmetrize(
             m_total_mc_histogram,
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::ElectronDiffScale_Up))),
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::ElectronDiffScale_Down))))},

        // Photon Resolution
        {"photon_resolution",
         Uncertanties::AbsDiffAndSymmetrize(
             m_total_mc_histogram,
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::PhotonDiffResolution_Up))),
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::PhotonDiffResolution_Down))))},

        // Photon Scale
        {"photon_scale",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonDiffScale_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::PhotonDiffScale_Down))))},

        // Tau Energy
        {"tau_energy",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauDiffEnergy_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::TauDiffEnergy_Down))))},

        // Jet Resolution
        {"jet_resolution",
         Uncertanties::AbsDiffAndSymmetrize(
             m_total_mc_histogram,
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::JetDiffResolution_Up))),
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::JetDiffResolution_Down))))},

        // Jet Scale
        {"jet_scale",
         Uncertanties::AbsDiffAndSymmetrize(m_total_mc_histogram,
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::JetDiffScale_Up))),
                                            ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                                                static_cast<std::size_t>(Shifts::Variations::JetDiffScale_Down))))},

        // MET Unclustered Energy
        {"met_unclustered_energy",
         Uncertanties::AbsDiffAndSymmetrize(
             m_total_mc_histogram,
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::METDiffUnclusteredEnergy_Up))),
             ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
                 static_cast<std::size_t>(Shifts::Variations::METDiffUnclusteredEnergy_Down))))},
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

    // build systematics for plotting (taking the symmetrization conservative approach)
    // this will only affect uncertainties which are assymetric
    m_systematics_for_plots_and_integral_p_value = m_systematics_uncertainties;
    m_systematics_for_plots_and_integral_p_value["prefiring"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PreFiring_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PreFiring_Down))));

    m_systematics_for_plots_and_integral_p_value["muon_reco"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::MuonReco_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::MuonReco_Down))));

    m_systematics_for_plots_and_integral_p_value["muon_id"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::MuonId_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::MuonId_Down))));

    m_systematics_for_plots_and_integral_p_value["muon_iso"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::MuonIso_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::MuonIso_Down))));

    m_systematics_for_plots_and_integral_p_value["electron_reco"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::ElectronReco_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::ElectronReco_Down))));

    m_systematics_for_plots_and_integral_p_value["electron_id"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::ElectronId_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::ElectronId_Down))));

    m_systematics_for_plots_and_integral_p_value["photon_id"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PhotonId_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PhotonId_Down))));

    m_systematics_for_plots_and_integral_p_value["photon_veto"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PhotonVeto_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PhotonVeto_Down))));

    m_systematics_for_plots_and_integral_p_value["tau_vs_e"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::TauVsE_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::TauVsE_Down))));

    m_systematics_for_plots_and_integral_p_value["tau_vs_mu"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::TauVsMu_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::TauVsMu_Down))));

    m_systematics_for_plots_and_integral_p_value["tau_vs_jet"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::TauVsJet_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::TauVsJet_Down))));

    m_systematics_for_plots_and_integral_p_value["jet_btag"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::JetBTag_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::JetBTag_Down))));

    m_systematics_for_plots_and_integral_p_value["pile_up"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PU_Up))),
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::PU_Down))));

    m_systematics_for_plots_and_integral_p_value["electron_resolution"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::ElectronDiffResolution_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::ElectronDiffResolution_Down))));

    m_systematics_for_plots_and_integral_p_value["electron_scale"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::ElectronDiffScale_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::ElectronDiffScale_Down))));

    m_systematics_for_plots_and_integral_p_value["photon_resolution"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::PhotonDiffResolution_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::PhotonDiffResolution_Down))));

    m_systematics_for_plots_and_integral_p_value["photon_scale"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::PhotonDiffScale_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::PhotonDiffScale_Down))));

    m_systematics_for_plots_and_integral_p_value["tau_energy"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::TauDiffEnergy_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::TauDiffEnergy_Down))));

    m_systematics_for_plots_and_integral_p_value["jet_resolution"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::JetDiffResolution_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::JetDiffResolution_Down))));

    m_systematics_for_plots_and_integral_p_value["jet_scale"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(
            m_histogram_per_process_group_and_shift.at(static_cast<std::size_t>(Shifts::Variations::JetDiffScale_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::JetDiffScale_Down))));

    m_systematics_for_plots_and_integral_p_value["met_unclustered_energy"] = Uncertanties::AbsDiffAndSymmetrizeForPlots(
        m_total_mc_histogram,
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::METDiffUnclusteredEnergy_Up))),
        ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at(
            static_cast<std::size_t>(Shifts::Variations::METDiffUnclusteredEnergy_Down))));

    for (const auto &[pg, uncert] : xsec_order_uncert_non_LO_samples_for_plotting)
    {
        m_systematics_for_plots_and_integral_p_value[fmt::format("xsec_non_LO_{}", pg)] =
            xsec_order_uncert_non_LO_samples_for_plotting.at(pg);
    }

    return {
        ROOT::VecOps::sqrt(std::accumulate(
            m_systematics_uncertainties.cbegin(),
            m_systematics_uncertainties.cend(),
            RVec<double>(m_n_bins, 0.),
            [](const RVec<double> &acc_vec, const std::pair<std::string, RVec<double>> &next_key_val) -> RVec<double>
            {
                const auto &[src, next_vec] = next_key_val;
                return acc_vec + ROOT::VecOps::pow(next_vec, 2.);
            })),
        ROOT::VecOps::sqrt(std::accumulate(
            m_systematics_for_plots_and_integral_p_value.cbegin(),
            m_systematics_for_plots_and_integral_p_value.cend(),
            RVec<double>(m_n_bins, 0.),
            [](const RVec<double> &acc_vec, const std::pair<std::string, RVec<double>> &next_key_val) -> RVec<double>
            {
                const auto &[src, next_vec] = next_key_val;
                return acc_vec + ROOT::VecOps::pow(next_vec, 2.);
            }))};
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

    return IntegralPValueProps{
        .total_data = m_total_data_histogram.GetBinContent(1), //
        .total_mc = m_total_mc_histogram.GetBinContent(1),     //
        .sigma_total = m_total_uncert_for_plotting.at(0),      //
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

    auto mc_uncert = ROOTHelpers::MakeErrorBand(m_total_mc_histogram, m_total_uncert_for_plotting, m_scale_to_area);

    auto [ratio_graph, ratio_mc_err_graph] =
        ROOTHelpers::MakeRatioGraph(m_total_data_histogram, m_total_mc_histogram, m_total_uncert_for_plotting, min_max);

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

    auto mc_counts = ROOTHelpers::Counts(m_total_mc_histogram);
    auto uncertainties = std::unordered_map<std::string, RVec<double>>{};
    for (const auto &[uncert, values] : m_systematics_for_plots_and_integral_p_value)
    {
        uncertainties[uncert] = {};
        uncertainties[uncert].reserve(values.size());
        for (std::size_t i = 0; i < values.size(); i++)
        {
            if (mc_counts[i] != 0.)
            {
                uncertainties[uncert].push_back(values[i] / std::fabs(mc_counts[i]));
            }
            else
            {
                uncertainties[uncert].push_back(0.);
            }
        }
    }

    for (std::size_t i = 0; i < m_total_uncert_for_plotting.size(); i++)
    {
        if (mc_counts[i] != 0.)
        {
            uncertainties["total"].push_back(m_total_uncert_for_plotting[i] / std::fabs(mc_counts[i]));
        }
        else
        {
            uncertainties["total"].push_back(0.);
        }
    }

    for (std::size_t i = 0; i < m_statistical_uncert.size(); i++)
    {
        if (mc_counts[i] != 0.)
        {
            uncertainties["stat"].push_back(m_statistical_uncert[i] / std::fabs(mc_counts[i]));
        }
        else
        {
            uncertainties["stat"].push_back(0.);
        }
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
        .uncert_props =
            UncertPlotProps{
                .class_name = m_event_class_name,
                .distribution_name = m_distribution_name,
                .year_to_plot = m_year_to_plot,
                .bins = ROOTHelpers::Edges(m_total_data_histogram),
                .uncertanties = uncertainties,
                .x_min = min,
                .x_max = max,
            },
    };
}
