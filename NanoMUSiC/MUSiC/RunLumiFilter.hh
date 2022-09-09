#include "Tools/cpp_helper_libs/json.hpp"
#include <string>
using json = nlohmann::json;

class RunLumiFilter
{
  private:
    json good_runs_lumis_json;
    bool dummy_json = false;

  public:
    RunLumiFilter(const std::string &_input_json_file)
    {
        good_runs_lumis_json = [&]() {
            if (_input_json_file == "")
            {
                // no golden json provided --> should always return true
                dummy_json = true;
                return json();
            }

            try
            {
                auto golden_json = json::parse(std::ifstream(_input_json_file));
                return golden_json;
            }
            catch (const json::parse_error &err)
            {
                std::cerr << "ERROR: Golden JSON file [" << _input_json_file << "] parsing failed." << std::endl;
                std::cerr << "Message: " << err.what() << '\n'
                          << "Exception id: " << err.id << '\n'
                          << "Byte position of error: " << err.byte << std::endl;
                exit(-1);
            }
        }();
    }

    auto operator()(unsigned long run_number, unsigned long lumi) const
    {
        if (dummy_json)
        {
            // no golden json provided --> should always return true
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
                }
            }
        }
        return is_good_run_lumi;
    }
};