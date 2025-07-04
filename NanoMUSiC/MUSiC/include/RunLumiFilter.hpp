#ifndef RUNLUMIFILTER_HPP
#define RUNLUMIFILTER_HPP

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "fmt/core.h"

#include "json.hpp"
using json = nlohmann::json;

class RunLumiFilter
{
  private:
    json good_runs_lumis_json;

  public:
    RunLumiFilter(const std::string &input_json_file)
    {

        if (input_json_file == "")
        {
            throw std::runtime_error("No Golden JSON file provided.");
        }
        auto _input_json_file = input_json_file;
        size_t pos = input_json_file.find("$MUSIC_BASE");
        if (pos != std::string::npos)
        {
            _input_json_file.replace(pos, 11, std::getenv("MUSIC_BASE"));
        }

        try
        {
            good_runs_lumis_json = json::parse(std::ifstream(_input_json_file));
        }
        catch (const json::parse_error &err)
        {
            std::cerr << "ERROR: Golden JSON file [" << _input_json_file << "] parsing failed." << std::endl;
            std::cerr << "Message: " << err.what() << '\n'
                      << "Exception id: " << err.id << '\n'
                      << "Byte position of error: " << err.byte << std::endl;

            std::ifstream file(_input_json_file); // Open the file
            if (!file.is_open())
            {
                fmt::print(stderr, "ERROR: Could not open file. Does the file exists?\n");
            }

            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>())); // Read file content into a string

            throw std::runtime_error(fmt::format("Golden JSON file content:\n{}\n", content));
        }
    }

    auto operator()(const unsigned long &run_number, const unsigned long &lumi, const bool &is_data) const
    {
        if (!is_data)
        {
            return true;
        }

        // CMS standard is to have run number as string
        const auto test_run = std::to_string(run_number);
        const auto test_lumi = lumi;

        auto is_good_run_lumi = false;

        if (good_runs_lumis_json.find(test_run) != good_runs_lumis_json.end())
        {
            for (auto const &interval : good_runs_lumis_json[test_run])
            {
                const unsigned long low = interval.front();
                const unsigned long high = interval.back();
                if (test_lumi >= low && test_lumi <= high)
                {
                    is_good_run_lumi = true;
                    break;
                }
            }
        }
        return is_good_run_lumi;
    }
};
#endif // !RUNLUMIFILTER_HPP
