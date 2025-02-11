#ifndef CONFIG_PARAMS_HPP
#define CONFIG_PARAMS_HPP

#include <optional>
#include <string>
#include <vector>

class ConfigParams
{
  public:
    std::string name;
};

class JetParams : public ConfigParams
{
  public:
    double jet_config_a;
};

class GlobalParams : public ConfigParams
{
  public:
    std::string output_file;
    std::string process;
    std::string year;
    bool is_data;
    float x_section;
    float filter_eff;
    float k_factor;
    float luminosity;
    std::string xs_order;
    std::string process_group;
    std::string sum_weights_json_filepath;
    std::vector<std::string> input_files;
    std::string generator_filter;
    std::optional<int> first_event;
    std::optional<int> last_event;
    bool is_dev_job;
    bool do_btag_efficiency;
    bool debug;

    // Constructor
    // ConfigParams(const std::string &name= "", float v = 0.0f);
};

#endif // CONFIG_PARAMS_HPP
