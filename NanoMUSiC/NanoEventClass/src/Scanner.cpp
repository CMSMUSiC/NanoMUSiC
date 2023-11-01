#include "Scanner.hpp"

#include <algorithm>
#include <filesystem>
#include <unordered_map>

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace indicators;

Scanner::Scanner()
{
    // load lookup table
    auto looktable_file_path = fmt::format("{}/bin/lookuptable.bin", std::getenv("MUSIC_BASE"));
    if (std::filesystem::exists(looktable_file_path))
    {
        // fmt::print("[RoI Scanner] Loading lookup table ...\n");
        m_lookup_table.readFile(looktable_file_path);
        // fmt::print("[RoI Scanner] done.\n");
    }
    else
    {
        fmt::print(stderr, "ERROR: Could not load lookup table file. Did you \"ninja lut\"?");
        std::exit(EXIT_FAILURE);
    }
}

auto Scanner::make_region(const std::string &distribution_name,
                          const RVec<double> data_bins,
                          const RVec<double> mc_bins,
                          const RVec<double> sigma_bins,
                          std::size_t lower_index,
                          std::size_t upper_index) -> std::optional<Region>
{
    auto min_bin_size = 3;
    if (distribution_name == "invariant_mass" or distribution_name == "counts")
    {
        min_bin_size = 1;
    }
    if (upper_index - lower_index < static_cast<std::size_t>(min_bin_size))
    {
        return std::nullopt;
    }

    return Region{
        .data = std::reduce(data_bins.cbegin() + lower_index, data_bins.cbegin() + upper_index - 1),
        .mc = std::reduce(mc_bins.cbegin() + lower_index, mc_bins.cbegin() + upper_index - 1),
        .sigma = std::sqrt(std::transform_reduce(sigma_bins.cbegin() + lower_index,
                                                 sigma_bins.cbegin() + upper_index - 1,
                                                 0.,
                                                 std::plus<double>(),
                                                 [](double x) -> double
                                                 {
                                                     return std::pow(x, 2);
                                                 })) //
    };
}

auto Scanner::compute_p_value(const double data,
                              const double mc,
                              const double sigma,
                              const PriorMode prior,
                              const int debug_level) const -> double
{
    double p = -1.;
    constexpr double min_valid_p = 1E-8;

    // avoid caluclation if the difference between data and expecation is too large, just set it to a small p-value
    // because our assumptions might not be valid below there.
    if ((std::fabs(data - mc) / sigma) > 7.0)
    {
        // Set the value at  1e-8 (a little more than 5 sigma), because
        // our assumptions might not be valid below there.
        p = min_valid_p;
    }

    // try to look up p-value in lookup table
    p = m_lookup_table.lookup(data, mc, sigma);

    if (p < 0)
    {
        // p negative (either lookup-table-miss or lookup table skipped)
        // call p-value calculation from ConvolutionComputer.h
        p = compute_p_convolution(data, mc, sigma, prior, debug_level);
    }

    if (p >= 0)
    {
        // Don't allow p-values below 1e-8 (a little more than 5 sigma), because
        // our assumptions might not be valid there.
        // If multiple regions have the same score, we take the smallest.
        p = std::max(p, min_valid_p);
    }

    return p;
}

auto Scanner::roi_scan(const std::string &distribution_name, const RVec<double> &data_, DataSource data_source) const
    -> ScanResult
{
    // all temps ....
    const auto n_bins = 330;
    auto edges = RVec<double>(n_bins + 1);
    for (std::size_t i = 0; i < edges.size(); i++)
    {
        edges[i] = static_cast<double>(i);
    }
    const auto data = RVec<double>(n_bins, 10.);
    const auto mc = RVec<double>(n_bins, 11.);
    const auto sigma = RVec<double>(n_bins, 1.);
    // temps

    auto lowest_p = std::numeric_limits<double>::max();
    auto lower_edge = edges[0];
    auto upper_edge = edges[edges.size() - 1];
    auto status = ScanStatus::FAILED;
    for (std::size_t i = 0; i < edges.size() - 1; i++)
    {
        for (std::size_t j = i + 1; j < edges.size(); j++)
        {
            auto this_region = make_region(distribution_name, data, mc, sigma, i, j);
            if (this_region)
            {
                // auto this_p_value = compute_p_value(this_region->data,       //
                //                                     this_region->mc,         //
                //                                     this_region->sigma,      //
                //                                     PriorMode::NORMAL_PRIOR, //
                //                                     /*debug level*/ 1);

                auto this_p_value = compute_p_value(1.,                      //
                                                    1.01,                    //
                                                    0.1,                     //
                                                    PriorMode::NORMAL_PRIOR, //
                                                    /*debug level*/ 1);

                if (this_p_value < lowest_p and this_p_value > 0.)
                {
                    lowest_p = this_p_value;
                    lower_edge = edges[i];
                    upper_edge = edges[j];
                    status = ScanStatus::SUCCESS;
                }
            }
        }
    }

    return ScanResult{lowest_p, lower_edge, upper_edge, status, data_source};
}

auto Scanner::sample(const std::unordered_map<std::string, std::vector<double>> &shifts, std::size_t shift_index) const
    -> RVec<double>
{
    return {1., 2., 3.};
}

auto Scanner::scan(const std::shared_ptr<Distribution> &distribution,
                   const std::unordered_map<std::string, std::vector<double>> &shifts,
                   unsigned int n_shifts) -> std::vector<ScanResult>
{
    //   COUNTS
    auto data_bins = ROOTHelpers::Counts(distribution->m_total_data_histogram);

    //  [ PG : vec::COUNTS ]
    auto mc_bins = std::unordered_map<std::string, RVec<double>>();

    //  [ PG : vec::DIFFS ]
    auto stats_bins = std::unordered_map<std::string, RVec<double>>();

    //  [ SHIFT : vec::DIFFS ]
    auto sigma_bins = std::unordered_map<std::string, RVec<double>>();

    // "m_histogram_per_process_group_and_shift" maps  [SHIFT: [ PG : COUNTS ] ]
    for (auto &&[shift, histogram_per_process_group] : distribution->m_histogram_per_process_group_and_shift)
    {
        if (shift == "Nominal")
        {
            for (auto &&[pg, histogram] : histogram_per_process_group)
            {
                if (pg != "Data")
                {
                    mc_bins.insert({pg, ROOTHelpers::Counts(histogram)});
                    stats_bins.insert({pg, ROOTHelpers::Errors(histogram)});
                }
            }
        }
        else
        {
            for (auto &&[pg, histogram] : histogram_per_process_group)
            {
                // this check, in principle, is irrelevant, but... we never know....
                if (pg != "Data")
                {
                    // Uncertanties::AbsDiffAndSymmetrize(
                    //     m_total_mc_histogram,
                    //     ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PreFiring_Up")),
                    //     ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("PreFiring_Down"))),
                    //
                    // // Scale Factors
                    // Uncertanties::AbsDiff(m_total_mc_histogram,
                    //                       ROOTHelpers::SumAsTH1F(m_histogram_per_process_group_and_shift.at("ScaleFactor_Up"))),
                    sigma_bins.insert({pg, ROOTHelpers::Counts(histogram)});
                }
            }
        }
    }

    fmt::print("[RoI Scanner] Starting scan for {} - {}.\n",
               distribution->m_event_class_name,
               distribution->m_distribution_name);
    auto results = std::vector<ScanResult>();
    results.reserve(n_shifts + 1);

    // RoI Scan for Data
    results.push_back(roi_scan(distribution->m_distribution_name, {1., 1., 1.}, DataSource::DATA));
    fmt::print("\33[2K\r[RoI Scanner] Data 🗃️... done.\n");

    // Hide cursor
    // indicators::show_console_cursor(true);

    auto bar = indicators::ProgressBar{
        option::BarWidth{50},
        option::Start{" ["},
        option::Fill{"█"},
        option::Lead{"█"},
        option::Remainder{"-"},
        option::End{"]"},
        option::MaxProgress{n_shifts},
        option::PrefixText{fmt::format("\33[2K\r[RoI Scanner] Toys 👀")},
        // option::ForegroundColor{Color::yellow},
        option::ShowElapsedTime{true},
        option::ShowRemainingTime{true}
        // option::FontStyles{std::vector<FontStyle>{FontStyle::bold}} //
    };

    // toy SM data
    for (std::size_t i = 0; i < static_cast<std::size_t>(n_shifts); i++)
    {
        results.push_back(roi_scan(distribution->m_distribution_name, sample(shifts, i), DataSource::TOY));

        // update status bar
        bar.tick();
        // Show iteration as postfix text
        bar.set_option(option::PostfixText{std::to_string(i) + "/" + std::to_string(n_shifts) + " scans"});
    }

    bar.mark_as_completed();

    // Show cursor
    indicators::show_console_cursor(true);
    fmt::print("\033[A\33[2K\r\n");

    return results;
}
