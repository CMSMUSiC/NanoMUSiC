#ifndef PROCESSED_DATA_EVENTS
#define PROCESSED_DATA_EVENTS

#include "fmt/format.h"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <glob.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ProcessedDataEvents
{

inline auto dump(const std::unordered_map<unsigned int, std::unordered_set<unsigned long>> &processed_data_events,
                 const std::string &output_path,
                 const std::string hash) -> void
{
    fmt::print("Dumping processed events to {}/processed_data_events_{}.bin ...\n", output_path, hash);
    std::ofstream outfile(fmt::format("{}/processed_data_events_{}.bin", output_path, hash), std::ios::binary);
    for (const auto &pair : processed_data_events)
    {
        outfile.write(reinterpret_cast<const char *>(&pair.first), sizeof(pair.first));
        const auto &values = pair.second;
        const size_t num_values = values.size();
        outfile.write(reinterpret_cast<const char *>(&num_values), sizeof(num_values));
        for (const auto &value : values)
        {
            outfile.write(reinterpret_cast<const char *>(&value), sizeof(value));
        }
    }
    outfile.close();
    fmt::print("... done.\n");
}

inline auto load(const std::string &input_file) -> std::unordered_map<unsigned int, std::unordered_set<unsigned long>>
{
    if (not(std::filesystem::exists(std::filesystem::path(input_file))))
    {
        throw std::runtime_error(
            fmt::format("ERROR: Could not load processed events files. Path does not exists ({}).", input_file));
    }

    fmt::print("Reading processed events from {} ...\n", input_file);

    // Create new unordered_map
    std::unordered_map<unsigned int, std::unordered_set<unsigned long>> processed_data_events;

    std::ifstream infile(input_file, std::ios::binary);
    while (!infile.eof())
    {
        unsigned int key;
        infile.read(reinterpret_cast<char *>(&key), sizeof(key));
        if (infile.eof())
        {
            break;
        }
        std::unordered_set<unsigned long> values;
        size_t num_values;
        infile.read(reinterpret_cast<char *>(&num_values), sizeof(num_values));
        for (size_t i = 0; i < num_values; ++i)
        {
            unsigned long value;
            infile.read(reinterpret_cast<char *>(&value), sizeof(value));
            values.insert(value);
        }
        processed_data_events[key] = values;
    }
    infile.close();
    fmt::print("... done.\n");

    return processed_data_events;
}

inline auto glob(const std::string &pattern) -> std::vector<std::string>
{
    std::vector<std::string> paths;

    glob_t glob_result;
    int ret = glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result);
    if (ret == 0)
    {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i)
        {
            paths.push_back(std::string(glob_result.gl_pathv[i]));
        }
    }
    globfree(&glob_result);

    return paths;
}

inline auto load_and_merge(const std::string &path_pattern)
    -> std::unordered_map<unsigned int, std::unordered_set<unsigned long>>
{
    const std::vector<std::string> processed_events_file_vec = glob(path_pattern);

    if (processed_events_file_vec.size() == 0)
    {
        throw std::runtime_error(fmt::format(
            "ERROR: Could not load and merge processed events files. The provided list of files is empty."));
    }

    if (processed_events_file_vec.size() == 1)
    {
        return load(processed_events_file_vec[0]);
    }

    // Create new map
    std::unordered_map<unsigned int, std::unordered_set<unsigned long>> processed_data_events =
        load(processed_events_file_vec[0]);

    for (std::size_t i = 1; i < processed_data_events.size(); i++)
    {
        for (auto &&[run, events_set] : load(processed_events_file_vec[i]))
        {
            if (processed_data_events.count(run) == 0)
            {
                processed_data_events.insert({run, events_set});
            }
            else
            {
                for (auto &&event : events_set)
                {
                    if (processed_data_events[run].count(event) == 0)
                    {
                        processed_data_events[run].insert(event);
                    }
                }
            }
        }
    }

    return processed_data_events;
}

} // namespace ProcessedDataEvents

#endif // !PROCESSED_DATA_EVENTS