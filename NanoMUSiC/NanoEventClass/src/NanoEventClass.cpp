#include "NanoEventClass.hpp"

#include <stdexcept>
#include <tuple>

auto NanoEventClass::split_histo_name(std::string histo_full_name, const std::string delimiter)
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

    if (parts.at(0) != m_class_name)
    {
        throw std::runtime_error(
            fmt::format("ERROR: Could not split the histogram full name ({}). The extracted class name ({}) does not "
                        "match the requested one ({}).",
                        histo_full_name,
                        parts.at(0),
                        m_class_name));
    }

    return std::make_tuple(parts.at(0), parts.at(1), parts.at(2), parts.at(3), parts.at(4), parts.at(5), parts.at(6));
}

auto NanoEventClass::make_nano_event_histo(const std::string &histo_full_name) -> NanoEventHisto
{
    const auto [class_name, process_group, xs_order, sample, year, shift, histo_name] =
        split_histo_name(histo_full_name);

    if (m_debug)
    {
        fmt::print(
            "Adding histogram: \nProcess group: {}\nXS Order:{}\nSample: {}\nYear: {}\nShift: {}\nHistogram: "
            "{}\n----------------------",
            process_group,
            xs_order,
            sample,
            year,
            shift,
            histo_name);
    }

    return NanoEventHisto{process_group,
                          xs_order,
                          sample,
                          year,
                          shift,
                          histo_name,
                          m_file->Get<TH1F>(histo_full_name.c_str()),
                          m_is_data};
}

// Function to check if a string matches a pattern with *
auto NanoEventClass::match_pattern(const std::string &str, const std::string &pattern) -> bool
{
    size_t pos = pattern.find('*');
    if (pos == std::string::npos)
    {
        return str == pattern;
    }
    else if (pos == 0)
    {
        std::string suffix = pattern.substr(1);
        return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
    else if (pos == pattern.size() - 1)
    {
        std::string prefix = pattern.substr(0, pos);
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }
    else
    {
        std::string prefix = pattern.substr(0, pos);
        std::string suffix = pattern.substr(pos + 1);
        return str.size() >= (prefix.size() + suffix.size()) && str.compare(0, prefix.size(), prefix) == 0 &&
               str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
}

NanoEventClass::NanoEventClass(const std::string &class_name, const std::string &file_path, bool is_data, bool debug)
    : m_class_name(class_name),
      m_file_path(file_path),
      m_file(TFile::Open(file_path.c_str())),
      m_is_data(is_data),
      m_debug(debug)
{
    for (auto &&histo : *m_file->GetListOfKeys())
    {
        const auto histo_full_name = std::string(histo->GetName());
        if (histo_full_name.find(fmt::format("[{}]", class_name)) == 0)
        {
            m_histograms.push_back(make_nano_event_histo(histo_full_name));
        }
    }
}

auto NanoEventClass::filter_histos(const std::string &process_group_pattern,
                                   const std::string &xs_order_pattern,
                                   const std::string &sample_pattern,
                                   const std::string &year_pattern,
                                   const std::string &shift_pattern,
                                   const std::string &histo_name_pattern) -> std::vector<NanoEventHisto>
{
    std::vector<NanoEventHisto> filtered_histos;
    std::copy_if(m_histograms.cbegin(),
                 m_histograms.cend(),
                 std::back_inserter(filtered_histos),
                 [&process_group_pattern,
                  &xs_order_pattern,
                  &sample_pattern,
                  &year_pattern,
                  &shift_pattern,
                  &histo_name_pattern](auto &histo)
                 {
                     return (match_pattern(histo.process_group, process_group_pattern)     //
                             and match_pattern(histo.process_group, process_group_pattern) //
                             and match_pattern(histo.xs_order, xs_order_pattern)           //
                             and match_pattern(histo.sample, sample_pattern)               //
                             and match_pattern(histo.year, year_pattern)                   //
                             and match_pattern(histo.shift, shift_pattern)                 //
                             and match_pattern(histo.histo_name, histo_name_pattern)       //
                     );
                 });

    return filtered_histos;
}

auto NanoEventClass::make_histogram_full_name(const std::string &class_name,
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