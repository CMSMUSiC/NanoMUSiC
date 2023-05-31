
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

class RunLumiFilter
{
  private:
    nlohmann::json good_runs_lumis_json;

  public:
    RunLumiFilter(const std::string &_input_json_file)
    {
        try
        {
            good_runs_lumis_json = nlohmann::json::parse(std::ifstream(_input_json_file));
        }
        catch (const nlohmann::json::parse_error &err)
        {
            std::cerr << "ERROR: Golden JSON file [" << _input_json_file << "] parsing failed." << std::endl;
            std::cerr << "Message: " << err.what() << '\n'
                      << "Exception id: " << err.id << '\n'
                      << "Byte position of error: " << err.byte << std::endl;
            throw std::runtime_error(fmt::format("ERROR: Could not parse input JSON file ({}).", _input_json_file));
        }
    }

    auto operator()(const unsigned long &run_number, const unsigned long &lumi, const bool &is_data) -> bool
    {
        if (!is_data)
        {
            return true;
        }

        // CMS standard is to have run number as string (WT*!!)
        const auto test_run = std::to_string(run_number);
        const auto test_lumi = lumi;

        if (good_runs_lumis_json.find(test_run) != good_runs_lumis_json.end())
        {
            for (auto const &interval : good_runs_lumis_json[test_run])
            {
                const unsigned long low = interval.front();
                const unsigned long high = interval.back();
                if (test_lumi >= low && test_lumi <= high)
                {
                    // is_good_run_lumi = true;
                    return true;
                }
            }
        }
        return false;
    }
};
