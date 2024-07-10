#ifndef SERIALIZATION_UTILS
#define SERIALIZATION_UTILS

#include "fmt/core.h"
#include <string>
#include <tuple>
#include <vector>

constexpr unsigned int num_histo_name_parts = 7;

namespace SerializationUtils
{
inline auto make_histogram_full_name(const std::string &class_name,
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

inline auto split_histo_name(std::string histo_full_name, const std::string delimiter = "]_[")
    -> std::tuple<std::string, std::string, std::string, std::string, std::string, std::string, std::string>
{
    // remove leading and trailing bracket
    histo_full_name = histo_full_name.substr(1, histo_full_name.length() - 2);
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    auto parts = std::vector<std::string>();
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
            fmt::runtime("ERROR: Could not split the histogram full name ({}). The number of unpacked parts ({}) does "
                         "not match the expectation ({})."),
            histo_full_name,
            parts.size(),
            num_histo_name_parts); std::exit(EXIT_FAILURE);
    }

    return std::make_tuple(parts.at(0), parts.at(1), parts.at(2), parts.at(3), parts.at(4), parts.at(5), parts.at(6));
}
} // namespace SerializationUtils

#endif
