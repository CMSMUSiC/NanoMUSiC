#include "EventClass.hpp"
#include "BinLimits.hpp"
#include "TFile.h"
#include "TH1F.h"
#include "fmt/core.h"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <string>
#include <unordered_map>

//////////////////////////
/// EventClassHistogram
//////////////////////////
auto EventClassHistogram::make_event_class_histogram(const std::string &name, bool weighted) -> EventClassHistogram
{
    auto h = EventClassHistogram{};
    h.name = name;
    h.weighted = weighted;
    h.counts.reserve(expected_max_bins);

    if (weighted)
    {
        h.squared_weights.reserve(expected_max_bins);
    }

    return h;
}

auto EventClassHistogram::push(float x, float w) -> void
{
    auto idx = bin_index(x);
    counts[idx] += w;
    if (weighted)
    {
        squared_weights[idx] += std::pow(w, 2);
    }
}

auto EventClassHistogram::count(float x) -> double
{
    auto idx = bounded_bin_index(x);
    if (idx)
    {
        return counts[*idx];
    }
    return 0.;
}

auto EventClassHistogram::error(float x) -> double
{
    auto idx = bounded_bin_index(x);
    if (idx)
    {
        return std::sqrt(squared_weights[*idx]);
    }
    return 0.;
}

auto EventClassHistogram::size() -> std::size_t
{
    return counts.size();
}

auto EventClassHistogram::bounded_bin_index(float x) -> std::optional<std::size_t>
{
    if (x < 0)
    {
        fmt::print(stderr, "ERROR: Could not get bin index for negative x: {}", x);
        std::exit(EXIT_FAILURE);
    }

    auto idx = static_cast<int>(x) / static_cast<int>(bin_size);
    if (counts.find(idx) == counts.end())
    {
        return std::nullopt;
    }
    return idx;
}

auto EventClassHistogram::bin_index(float x) -> std::size_t
{
    if (x < 0)
    {
        fmt::print(stderr, "ERROR: Could not get bin index for negative x: {}", x);
        std::exit(EXIT_FAILURE);
    }

    return static_cast<int>(x) / static_cast<int>(bin_size);
}

auto EventClassHistogram::merge_inplace(EventClassHistogram &other) -> void
{
    for (auto &&bin_idx : set_of_bins(*this, other))
    {
        counts[bin_idx] += other.counts[bin_idx];
        if (weighted)
        {
            squared_weights[bin_idx] += other.squared_weights[bin_idx];
        }
    }
}
auto EventClassHistogram::serialize_to_root(const std::unique_ptr<TFile> &output_root_file,
                                            const std::unordered_map<ObjectNames, int> &count_map,
                                            std::vector<double> &bins_limits,
                                            const std::string &event_class_name,
                                            const std::string &process_name,
                                            const std::string &process_group,
                                            const std::string &xsec_order,
                                            const std::string &year,
                                            bool is_data,
                                            const std::string &histogram_name,
                                            const std::string &variation_name) -> void
{
    if (not(is_data and variation_name != Shifts::variation_to_string(Shifts::Variations::Nominal)))
    {

        // auto histo = TH1F("histo",
        //                   "",
        //                   static_cast<int>(Histograms::max_energy / bin_size),
        //                   Histograms::min_energy,
        //                   Histograms::max_energy);

        auto histo_name = fmt::format("[{}]_[{}]_[{}]_[{}]_[{}]_[{}]_[{}]",
                                      event_class_name,
                                      process_group,
                                      xsec_order,
                                      process_name,
                                      year,
                                      variation_name,
                                      histogram_name);
        auto histo = TH1F(histo_name.c_str(), "", bins_limits.size() - 1, bins_limits.data());
        histo.Sumw2();

        auto total_count = 0.;
        auto total_squared_weight = 0.;
        for (auto &&[idx, count] : counts)
        {
            total_count += count;
            auto bin_center = idx * bin_size / 2.;
            auto root_bin_idx = histo.FindBin(bin_center);
            histo.SetBinContent(root_bin_idx, histo.GetBinContent(root_bin_idx) + count);
            if (weighted)
            {
                total_squared_weight += squared_weights[idx];
                histo.SetBinError(root_bin_idx,
                                  std::sqrt(std::pow(histo.GetBinError(root_bin_idx), 2) + squared_weights[idx]));
            }
        }

        output_root_file->WriteObject(&histo, histo_name.c_str());

        if (histogram_name == "h_sum_pt")
        {
            auto histo_name = fmt::format("[{}]_[{}]_[{}]_[{}]_[{}]_[{}]_[{}]",
                                          event_class_name,
                                          process_group,
                                          xsec_order,
                                          process_name,
                                          year,
                                          variation_name,
                                          "h_counts");
            auto histo = TH1F(histo_name.c_str(), "", 1, 0, 1);
            histo.Sumw2();

            histo.SetBinContent(1, total_count);
            histo.SetBinError(1, std::sqrt(total_squared_weight));

            output_root_file->WriteObject(&histo, histo_name.c_str());
        }
    }
}

//////////////////////////
/// EventClass
//////////////////////////
auto EventClass::make_event_class(const std::string &ec_name) -> EventClass
{
    auto ec = EventClass{};
    ec.ec_name = ec_name;
    for (std::size_t var = 0; var < total_variations; var++)
    {
        if (var == static_cast<std::size_t>(Shifts::Variations::Nominal))
        {
            ec.h_sum_pt[var] = EventClassHistogram::make_event_class_histogram(
                Shifts::variation_to_string(Shifts::Variations::Nominal), true);
            ec.h_invariant_mass[var] = EventClassHistogram::make_event_class_histogram(
                Shifts::variation_to_string(Shifts::Variations::Nominal), true);
            ec.h_met[var] = EventClassHistogram::make_event_class_histogram(
                Shifts::variation_to_string(Shifts::Variations::Nominal), true);
        }
        ec.h_sum_pt[var] = EventClassHistogram::make_event_class_histogram(Shifts::variation_to_string(var), false);
        ec.h_invariant_mass[var] =
            EventClassHistogram::make_event_class_histogram(Shifts::variation_to_string(var), false);
        ec.h_met[var] = EventClassHistogram::make_event_class_histogram(Shifts::variation_to_string(var), false);
    }
    return ec;
}

auto EventClass::sum_pt(std::size_t variation) -> EventClassHistogram &
{
    return h_sum_pt[variation];
}

auto EventClass::invariant_mass(std::size_t variation) -> EventClassHistogram &
{
    return h_invariant_mass[variation];
}

auto EventClass::met(std::size_t variation) -> EventClassHistogram &
{
    return h_met[variation];
}

auto EventClass::histogram(const std::string &observable, std::size_t variation) -> EventClassHistogram &
{
    if (observable == "sum_pt")
    {
        return sum_pt(variation);
    }
    if (observable == "invariant_mass")
    {
        return invariant_mass(variation);
    }
    if (observable == "met")
    {
        return met(variation);
    }

    fmt::print(stderr, "ERROR: Observable ({}) is not defined.", observable);
    std::exit(EXIT_FAILURE);
}

auto EventClass::merge_inplace(EventClass &other) -> void
{
    for (std::size_t var_idx = 0; var_idx < total_variations; var_idx++)
    {
        h_sum_pt[var_idx].merge_inplace(other.h_sum_pt[var_idx]);
        h_invariant_mass[var_idx].merge_inplace(other.h_invariant_mass[var_idx]);
        h_met[var_idx].merge_inplace(other.h_met[var_idx]);
    }
}

auto EventClass::serialize_to_root(const std::unique_ptr<TFile> &output_root_file,
                                   const std::string &event_class_name,
                                   const std::string &process_name,
                                   const std::string &process_group,
                                   const std::string &xsec_order,
                                   const std::string &year,
                                   bool is_data) -> void
{
    auto count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 0},
                                                          {ObjectNames::Electron, 0},
                                                          {ObjectNames::Photon, 0},
                                                          {ObjectNames::Tau, 0},
                                                          {ObjectNames::bJet, 0},
                                                          {ObjectNames::Jet, 0},
                                                          {ObjectNames::MET, 0}};

    std::vector<std::string> tokens;
    tokens.reserve(8);
    std::istringstream iss(event_class_name);
    std::string token;
    while (std::getline(iss, token, '_'))
    {
        tokens.push_back(token);
    }

    for (auto &&tok : tokens)
    {
        auto pos = tok.find("Muon");
        if (pos != std::string::npos)
        {
            count_map.at(ObjectNames::Muon) = std::stoi(tok.substr(0, pos));
            continue;
        }
        pos = tok.find("Electron");
        if (pos != std::string::npos)
        {
            count_map.at(ObjectNames::Electron) = std::stoi(tok.substr(0, pos));
            continue;
        }
        pos = tok.find("Tau");
        if (pos != std::string::npos)
        {
            count_map.at(ObjectNames::Tau) = std::stoi(tok.substr(0, pos));
            continue;
        }
        pos = tok.find("Photon");
        if (pos != std::string::npos)
        {
            count_map.at(ObjectNames::Photon) = std::stoi(tok.substr(0, pos));
            continue;
        }
        pos = tok.find("bJet");
        if (pos != std::string::npos)
        {
            count_map.at(ObjectNames::bJet) = std::stoi(tok.substr(0, pos));
            continue;
        }
        pos = tok.find("Jet");
        if (pos != std::string::npos)
        {
            count_map.at(ObjectNames::Jet) = std::stoi(tok.substr(0, pos));
            continue;
        }
        pos = tok.find("MET");
        if (pos != std::string::npos)
        {
            count_map.at(ObjectNames::MET) = std::stoi(tok.substr(0, pos));
            continue;
        }
    }
    auto bins_limits = BinLimits::limits(
        count_map, false, Histograms::min_energy, Histograms::max_energy, Histograms::min_bin_size, Histograms::fudge);

    for (std::size_t var_idx = 0; var_idx < total_variations; var_idx++)
    {
        h_sum_pt[var_idx].serialize_to_root(output_root_file,
                                            count_map,
                                            bins_limits,
                                            event_class_name,
                                            process_name,
                                            process_group,
                                            xsec_order,
                                            year,
                                            is_data,
                                            "h_sum_pt",
                                            Shifts::variation_to_string(var_idx));
        h_invariant_mass[var_idx].serialize_to_root(output_root_file,
                                                    count_map,
                                                    bins_limits,
                                                    event_class_name,
                                                    process_name,
                                                    process_group,
                                                    xsec_order,
                                                    year,
                                                    is_data,
                                                    "h_invariant_mass",
                                                    Shifts::variation_to_string(var_idx));
        if (count_map[ObjectNames::MET] > 0)
        {
    auto bins_limits_MET = BinLimits::limits(
        count_map, true, Histograms::min_energy, Histograms::max_energy, Histograms::min_bin_size, Histograms::fudge);
            h_met[var_idx].serialize_to_root(output_root_file,
                                             count_map,
                                             bins_limits_MET,
                                             event_class_name,
                                             process_name,
                                             process_group,
                                             xsec_order,
                                             year,
                                             is_data,
                                             "h_met",
                                             Shifts::variation_to_string(var_idx));
        }
    }
}

/////////////////////////
/// EventClassContainer
/////////////////////////
auto EventClassContainer::merge_inplace(EventClassContainer &other) -> void
{
    for (auto &&ec_name : set_of_classes(*this, other))
    {
        classes[ec_name].merge_inplace(other.classes[ec_name]);
    }
}

auto EventClassContainer::serialize_to_root(EventClassContainer &cont,
                                            const std::string &ouput_file_path,
                                            const std::string &process_name,
                                            const std::string &process_group,
                                            const std::string &xsec_order,
                                            const std::string &year,
                                            bool is_data) -> void
{
    std::unique_ptr<TFile> output_root_file(TFile::Open(ouput_file_path.c_str(), "RECREATE"));
    for (auto &&[event_class_name, event_class] : cont.classes)
    {
        event_class.serialize_to_root(
            output_root_file, event_class_name, process_name, process_group, xsec_order, year, is_data);
    }
}
