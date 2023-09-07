#include "NanoEventClass.hpp"

#include <iostream>
#include <stdexcept>
#include <tuple>

#include "TCollection.h"
#include "TFile.h"
#include "TKey.h"

#include "fmt/format.h"

auto NanoEventHisto::split_histo_name(std::string histo_full_name, const std::string delimiter)
    -> std::tuple<std::string, std::string, std::string, std::string, std::string, std::string, std::string>
{
    // remove leading and trailing bracket
    histo_full_name = histo_full_name.substr(1, histo_full_name.length() - 2);

    std::vector<std::string> parts;
    std::size_t startPos = 0;
    std::size_t endPos = histo_full_name.find(delimiter);

    while (endPos != std::string::npos)
    {
        std::string part = histo_full_name.substr(startPos, endPos - startPos);
        parts.push_back(part);
        startPos = endPos + 1;
        endPos = histo_full_name.find(delimiter, startPos);
    }

    std::string lastPart = histo_full_name.substr(startPos);
    parts.push_back(lastPart);

    if (parts.size() != num_histo_name_parts)
    {
        throw std::runtime_error(
            fmt::format("ERROR: Could not split the histogram full name ({}). The number of unpacked parts ({}) does "
                        "not match the expectation ({}).",
                        histo_full_name,
                        parts.size(),
                        num_histo_name_parts));
    }

    return std::make_tuple(parts.at(0), parts.at(1), parts.at(2), parts.at(3), parts.at(4), parts.at(5), parts.at(6));
}

auto NanoEventHisto::make_nano_event_histo(const std::string &histo_full_name, TH1F *histo_ptr) -> NanoEventHisto
{
    const auto [class_name, process_group, xs_order, sample, year, shift, histo_name] =
        NanoEventHisto::split_histo_name(histo_full_name);

    return NanoEventHisto{
        class_name,                 //
        process_group,              //
        xs_order,                   //
        sample,                     //
        year,                       //
        shift,                      //
        histo_name,                 //
        (TH1F *)histo_ptr->Clone(), //
        (process_group == "Data")   //
    };
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
    m_data_count = 0;
    m_mc_count = 0;
    for (auto count : m_counts)
    {
        fmt::print("{}\n", count.shift);
        if (count.shift == "Nominal")
        {
            fmt::print("Passei por aqui \n");
            if (count.is_data)
            {
                fmt::print("Passei  de novo\n");
                m_data_count += count.histogram->Integral();
            }
            else
            {
                fmt::print("Passei  de novo\n");
                m_mc_count += count.histogram->Integral();
            }
        }
    }

    m_is_valid = ((m_class_name.find("Muon") != std::string::npos)        //
                  or (m_class_name.find("Electron") != std::string::npos) //
                  or (m_class_name.find("Photon") != std::string::npos))  //
                 and m_data_count >= min_data_count and m_mc_count >= min_mc_count;

    // m_is_valid = true;
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
        "histograms\nMC:{} events - {} histograms\n",
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

    throw std::runtime_error(
        fmt::format("ERROR: Could not get the class name from histogram fulll name ({}).", histo_full_name));
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

NanoEventClassCollection::NanoEventClassCollection(const std::vector<std::string> &root_file_paths)
    : m_classes({}),
      m_root_files({})
{
    auto h_counts_per_class = std::unordered_map<std::string, std::vector<NanoEventHisto>>();
    auto h_invariant_mass_per_class = std::unordered_map<std::string, std::vector<NanoEventHisto>>();
    auto h_sum_pt_per_class = std::unordered_map<std::string, std::vector<NanoEventHisto>>();
    auto h_met_per_class = std::unordered_map<std::string, std::vector<NanoEventHisto>>();

    fmt::print("Reading input files ...\n");
    auto i = 0;
    for (auto &&file_path : root_file_paths)
    {
        i++;
        std::cout << fmt::format("File: {} / {}\n", i, root_file_paths.size());
        auto root_file = TFile::Open(file_path.c_str());
        m_root_files.push_back(root_file);

        TIter keyList(root_file->GetListOfKeys());
        TKey *key;
        while ((key = (TKey *)keyList()))
        {
            auto full_name = std::string(key->GetName());
            if (full_name.find("[EC_") == 0)
            {
                if (full_name.find("h_counts") != std::string::npos)
                {
                    h_counts_per_class[NanoEventClass::get_class_name(full_name)].push_back(
                        NanoEventHisto::make_nano_event_histo(full_name, (TH1F *)key->ReadObj()));
                };

                if (full_name.find("h_invariant_mass") != std::string::npos)
                {
                    h_invariant_mass_per_class[NanoEventClass::get_class_name(full_name)].push_back(
                        NanoEventHisto::make_nano_event_histo(full_name, (TH1F *)key->ReadObj()));
                };

                if (full_name.find("h_sum_pt") != std::string::npos)
                {
                    h_sum_pt_per_class[NanoEventClass::get_class_name(full_name)].push_back(
                        NanoEventHisto::make_nano_event_histo(full_name, (TH1F *)key->ReadObj()));
                };

                if (full_name.find("h_met") != std::string::npos)
                {
                    h_met_per_class[NanoEventClass::get_class_name(full_name)].push_back(
                        NanoEventHisto::make_nano_event_histo(full_name, (TH1F *)key->ReadObj()));
                };
            }
        }
    }

    fmt::print("Building event classes ...\n");

    auto j = 0;
    for (auto &&[event_class_name, _] : h_counts_per_class)
    {
        j++;
        std::cout << fmt::format("EC: {} / {}\n", j, h_counts_per_class.size());

        if (h_invariant_mass_per_class.find(event_class_name) != h_invariant_mass_per_class.end() //
            and h_sum_pt_per_class.find(event_class_name) != h_sum_pt_per_class.cend()            //
            and h_met_per_class.find(event_class_name) != h_met_per_class.cend()                  //
        )
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

NanoEventClassCollection::~NanoEventClassCollection()
{
    for (auto &&f : m_root_files)
    {
        f->Close();
    }
}
