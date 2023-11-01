#include "NanoEventClass.hpp"

#include <cstdlib>
#include <fnmatch.h>
#include <iostream>
#include <set>
#include <stdexcept>
#include <tuple>

#include "TCollection.h"
#include "TFile.h"
#include "TKey.h"

#include "BS_thread_pool.hpp"
#include "fmt/format.h"

#include "roothelpers.hpp"

auto NanoEventHisto::split_histo_name(std::string histo_full_name, const std::string delimiter)
    -> std::tuple<std::string, std::string, std::string, std::string, std::string, std::string, std::string>
{
    // remove leading and trailing bracket
    histo_full_name = histo_full_name.substr(1, histo_full_name.length() - 2);

    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> parts;
    parts.reserve(7);

    while ((pos_end = histo_full_name.find(delimiter, pos_start)) != std::string::npos)
    {
        token = histo_full_name.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        parts.push_back(token);
    }

    parts.push_back(histo_full_name.substr(pos_start));

    if (parts.size() != num_histo_name_parts)
    {
        fmt::print(
            stderr,
            fmt::format("ERROR: Could not split the histogram full name ({}). The number of unpacked parts ({}) does "
                        "not match the expectation ({}).",
                        histo_full_name,
                        parts.size(),
                        num_histo_name_parts));
        std::exit(EXIT_FAILURE);
    }

    return std::make_tuple(parts.at(0), parts.at(1), parts.at(2), parts.at(3), parts.at(4), parts.at(5), parts.at(6));
}

auto NanoEventHisto::make_nano_event_histo(const std::string &histo_full_name, TH1F *histo_ptr) -> NanoEventHisto
{

    const auto [class_name, process_group, xs_order, sample, year, shift, histo_name] =
        NanoEventHisto::split_histo_name(histo_full_name);

    return NanoEventHisto{
        class_name,                       //
        process_group,                    //
        xs_order,                         //
        sample,                           //
        year,                             //
        shift,                            //
        histo_name,                       //
        std::shared_ptr<TH1F>(histo_ptr), //
        (process_group == "Data")         //
    };
}

auto NanoEventHisto::to_string() const -> std::string
{
    return fmt::format(
        "{} - {} - {} - {} - {} - {} - {}", class_name, process_group, xs_order, sample, year, shift, histo_name);
}

auto NanoEventHisto::make_histogram_full_name(const std::string &class_name,
                                              const std::string &process_group,
                                              const std::string &xs_order,
                                              const std::string &sample,
                                              const std::string &year,
                                              const std::string &shift,
                                              const std::string &histo_name) -> std::string
{
    return fmt::format(
        "[{}]_[{}]_[{}]_[{}]_[{}]_[{}]_[{}]", class_name, process_group, xs_order, sample, year, shift, histo_name);
}

// NanoEventHisto::~NanoEventHisto()
// {
//     fmt::print("Destructor called ... \n");
//     delete histogram;
// }

// // Function to check if a string matches a pattern with *
// auto NanoEventClass::match_pattern(const std::string &str, const std::string &pattern) -> bool
// {
//     size_t pos = pattern.find('*');
//     if (pos == std::string::npos)
//     {
//         return str == pattern;
//     }
//     else if (pos == 0)
//     {
//         std::string suffix = pattern.substr(1);
//         return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
//     }
//     else if (pos == pattern.size() - 1)
//     {
//         std::string prefix = pattern.substr(0, pos);
//         return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
//     }
//     else
//     {
//         std::string prefix = pattern.substr(0, pos);
//         std::string suffix = pattern.substr(pos + 1);
//         return str.size() >= (prefix.size() + suffix.size()) && str.compare(0, prefix.size(), prefix) == 0 &&
//                str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
//     }
// }

NanoEventClass::NanoEventClass(const std::string &class_name,
                               const std::vector<NanoEventHisto> &counts,
                               const std::vector<NanoEventHisto> &invariant_mass,
                               const std::vector<NanoEventHisto> &sum_pt,
                               const std::vector<NanoEventHisto> &met)
    : m_class_name(class_name),
      m_counts(counts),
      m_invariant_mass(invariant_mass),
      m_sum_pt(sum_pt),
      m_met(met)
{
    m_data_count = 0.;
    m_mc_count = 0.;
    for (const auto &count : m_counts)
    {
        if (count.shift == "Nominal")
        {
            if (count.is_data)
            {
                m_data_count += count.histogram->Integral();
            }
            else
            {
                m_mc_count += count.histogram->Integral();
            }
        }
    }

    m_is_valid = ((m_class_name.find("Muon") != std::string::npos)        //
                  or (m_class_name.find("Electron") != std::string::npos) //
                  or (m_class_name.find("Photon") != std::string::npos))  //
                 and m_mc_count >= min_mc_count;
    //  and m_data_count >= min_data_count and m_mc_count >= min_mc_count;
}

auto NanoEventClass::to_string() const -> std::string
{
    auto n_data = std::count_if(m_counts.begin(),
                                m_counts.end(),
                                [](const auto &h)
                                {
                                    return h.process_group == "Data";
                                });
    auto n_mc = m_counts.size() - n_data;

    return fmt::format(
        "Event Class: {}\nData: {} events - {} "
        "histograms\nMC: {} events - {} histograms\n",
        m_class_name,
        m_data_count,
        n_data,
        m_mc_count,
        n_mc);
}

auto NanoEventClass::get_class_name(std::string histo_full_name, const std::string delimiter) -> std::string
{
    // remove leading and trailing bracket
    histo_full_name = histo_full_name.substr(1, histo_full_name.length() - 2);

    std::vector<std::string> parts;
    std::size_t startPos = 0;
    std::size_t endPos = histo_full_name.find(delimiter);

    while (endPos != std::string::npos)
    {
        return histo_full_name.substr(startPos, endPos - startPos);
    }

    fmt::print(stderr,
               fmt::format("ERROR: Could not get the class name from histogram fulll name ({}).", histo_full_name));
    std::exit(EXIT_FAILURE);
}

// auto NanoEventClass::filter_histos(const std::string &process_group_pattern,
//                                    const std::string &xs_order_pattern,
//                                    const std::string &sample_pattern,
//                                    const std::string &year_pattern,
//                                    const std::string &shift_pattern,
//                                    const std::string &histo_name_pattern) -> std::vector<NanoEventHisto>
// {
//     std::vector<NanoEventHisto> filtered_histos;
//     std::copy_if(m_histograms.cbegin(),
//                  m_histograms.cend(),
//                  std::back_inserter(filtered_histos),
//                  [&process_group_pattern,
//                   &xs_order_pattern,
//                   &sample_pattern,
//                   &year_pattern,
//                   &shift_pattern,
//                   &histo_name_pattern](auto &histo)
//                  {
//                      return (match_pattern(histo.process_group, process_group_pattern)     //
//                              and match_pattern(histo.process_group, process_group_pattern) //
//                              and match_pattern(histo.xs_order, xs_order_pattern)           //
//                              and match_pattern(histo.sample, sample_pattern)               //
//                              and match_pattern(histo.year, year_pattern)                   //
//                              and match_pattern(histo.shift, shift_pattern)                 //
//                              and match_pattern(histo.histo_name, histo_name_pattern)       //
//                      );
//                  });

//     return filtered_histos;
// }

auto NanoEventClassCollection::ClassesWithData(const std::vector<std::string> &root_file_paths) -> std::set<std::string>
{
    fmt::print("[NanoEventClass Collection] Building list of classes with Data ...\n");
    auto classes = std::set<std::string>();
    for (auto &&file_path : root_file_paths)
    {
        std::unique_ptr<TFile> root_file(TFile::Open(file_path.c_str()));
        TIter keyList(root_file->GetListOfKeys());
        TKey *key;
        while ((key = (TKey *)keyList()))
        {
            const std::string name = key->GetName();
            if (name.find("[Data]") != std::string::npos)
            {
                classes.insert(NanoEventClass::get_class_name(key->GetName()));
            }
        }
    }
    fmt::print("[NanoEventClass Collection] Found {} classes with at least one data event.\n", classes.size());

    return classes;
}

NanoEventClassCollection::NanoEventClassCollection(const std::vector<std::string> &root_file_paths,
                                                   const std::vector<std::string> &class_patterns)
    : m_classes({})
{
    auto h_counts_per_class_buffer = std::vector<std::unordered_map<std::string, std::vector<NanoEventHisto>>>(
        root_file_paths.size(), std::unordered_map<std::string, std::vector<NanoEventHisto>>());
    auto h_invariant_mass_per_class_buffer = std::vector<std::unordered_map<std::string, std::vector<NanoEventHisto>>>(
        root_file_paths.size(), std::unordered_map<std::string, std::vector<NanoEventHisto>>());
    auto h_sum_pt_per_class_buffer = std::vector<std::unordered_map<std::string, std::vector<NanoEventHisto>>>(
        root_file_paths.size(), std::unordered_map<std::string, std::vector<NanoEventHisto>>());
    auto h_met_per_class_buffer = std::vector<std::unordered_map<std::string, std::vector<NanoEventHisto>>>(
        root_file_paths.size(), std::unordered_map<std::string, std::vector<NanoEventHisto>>());

    fmt::print("[NanoEventClass Collection] Reading input files ...\n");
    auto pool = BS::thread_pool(root_file_paths.size());
    // auto pool = BS::thread_pool(1);

    std::atomic<int> file_counter{0};
    auto make_collection = [&](std::size_t idx) -> void
    {
        auto file_path = root_file_paths.at(idx);
        std::unique_ptr<TFile> root_file(TFile::Open(file_path.c_str()));

        TIter keyList(root_file->GetListOfKeys());
        TKey *key;
        while ((key = (TKey *)keyList()))
        {
            auto full_name = std::string(key->GetName());
            auto event_class_name = NanoEventClass::get_class_name(full_name);
            bool has_match = false;
            for (auto &&pattern : class_patterns)
            {
                if (fnmatch(pattern.c_str(), event_class_name.c_str(), FNM_EXTMATCH) != FNM_NOMATCH)
                {
                    has_match = true;
                    break;
                }
            }

            if (full_name.find("[EC_") == 0 and has_match)
            {
                auto nano_histo = NanoEventHisto::make_nano_event_histo(full_name, (TH1F *)key->ReadObj());
                if (full_name.find("h_counts") != std::string::npos)
                {
                    h_counts_per_class_buffer.at(idx)[NanoEventClass::get_class_name(full_name)].emplace_back(
                        std::move(nano_histo));
                }

                if (full_name.find("h_invariant_mass") != std::string::npos)
                {
                    h_invariant_mass_per_class_buffer.at(idx)[NanoEventClass::get_class_name(full_name)].emplace_back(
                        std::move(nano_histo));
                }

                if (full_name.find("h_sum_pt") != std::string::npos)
                {
                    h_sum_pt_per_class_buffer.at(idx)[NanoEventClass::get_class_name(full_name)].emplace_back(
                        std::move(nano_histo));
                }

                if (full_name.find("h_met") != std::string::npos)
                {
                    h_met_per_class_buffer.at(idx)[NanoEventClass::get_class_name(full_name)].emplace_back(
                        std::move(nano_histo));
                }
            }
        }
        ++file_counter;
        fmt::print("[NanoEventClass Collection] Done [{} / {}]: {}\n", file_counter, root_file_paths.size(), file_path);
    };

    std::vector<std::future<void>> future_collections;
    fmt::print("[NanoEventClass Collection] Launching threads ...\n");
    for (std::size_t idx_file = 0; idx_file < root_file_paths.size(); idx_file++)
    {

        future_collections.push_back(pool.submit(make_collection, idx_file));
    }

    fmt::print("[NanoEventClass Collection] Waiting ...\n");
    for (auto &&fut : future_collections)
    {
        fut.wait();
    }

    fmt::print("[NanoEventClass Collection] Checking for exceptions ...\n");
    for (auto &&fut : future_collections)
    {
        fut.get();
    }

    auto map_merger = [](const std::vector<std::unordered_map<std::string, std::vector<NanoEventHisto>>> &buffer)
        -> std::unordered_map<std::string, std::vector<NanoEventHisto>>
    {
        auto merged_map = std::unordered_map<std::string, std::vector<NanoEventHisto>>();

        for (const auto &partial_map : buffer)
        {
            for (const auto &[event_class_name, nano_event_histograms] : partial_map)
            {
                if (merged_map.find(event_class_name) == merged_map.end())
                {
                    merged_map.insert({event_class_name, std::vector<NanoEventHisto>()});
                }
                merged_map.at(event_class_name)
                    .reserve(merged_map.at(event_class_name).size() + nano_event_histograms.size());
                std::move(nano_event_histograms.cbegin(),
                          nano_event_histograms.cend(),
                          std::back_inserter(merged_map.at(event_class_name)));
            }
        }

        return merged_map;
    };

    fmt::print("[NanoEventClass Collection] Merging buffers ...\n");
    fmt::print("[NanoEventClass Collection] Launching threads ...\n");

    auto merger_pool = BS::thread_pool(4);
    auto merged_counts_future = merger_pool.submit(map_merger, h_counts_per_class_buffer);
    auto merged_invariant_mass_future = merger_pool.submit(map_merger, h_invariant_mass_per_class_buffer);
    auto merged_sum_pt_future = merger_pool.submit(map_merger, h_sum_pt_per_class_buffer);
    auto merged_met_future = merger_pool.submit(map_merger, h_met_per_class_buffer);

    fmt::print("[NanoEventClass Collection] Waiting ...\n");
    merged_counts_future.wait();
    merged_invariant_mass_future.wait();
    merged_sum_pt_future.wait();
    merged_met_future.wait();

    fmt::print("[NanoEventClass Collection] Getting results ...\n");
    auto h_counts_per_class = merged_counts_future.get();
    auto h_invariant_mass_per_class = merged_invariant_mass_future.get();
    auto h_sum_pt_per_class = merged_sum_pt_future.get();
    auto h_met_per_class = merged_met_future.get();

    fmt::print("[NanoEventClass Collection] Building event classes ...\n");
    for (auto &&[event_class_name, _] : h_counts_per_class)
    {
        // fmt::print("{} [{}]\n", event_class_name, h_counts_per_class.size());

        if (h_invariant_mass_per_class.find(event_class_name) != h_invariant_mass_per_class.cend() //
            and h_sum_pt_per_class.find(event_class_name) != h_sum_pt_per_class.cend()             //
            and h_met_per_class.find(event_class_name) != h_met_per_class.cend())                  //
        {
            auto ec = NanoEventClass(event_class_name,
                                     h_counts_per_class.at(event_class_name),
                                     h_invariant_mass_per_class.at(event_class_name),
                                     h_sum_pt_per_class.at(event_class_name),
                                     h_met_per_class.at(event_class_name));

            if (ec.m_is_valid)
            {
                m_classes.insert({event_class_name, ec});
            }
        }
    }

    fmt::print("\n");
}

auto NanoEventClassCollection::get_classes() const -> std::vector<std::string>
{
    std::vector<std::string> classes;

    // Iterate over the map and extract keys
    for (const auto &pair : m_classes)
    {
        classes.push_back(pair.first);
    }
    return classes;
}

auto NanoEventClassCollection::get_class(const std::string &class_name) -> NanoEventClass &
{
    return m_classes.at(class_name);
}
