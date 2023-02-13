#ifndef TASK_CONFIGURATION_H
#define TASK_CONFIGURATION_H

#include "Configs.hpp"
#include "TOMLConfig.hpp"
#include "Trigger.hpp"

class TaskConfiguration
{
  public:
    const std::string run_config_file;
    const TOMLConfig run_config;
    const std::string output_directory;
    const std::string process;
    const std::string dataset;
    const bool is_data;
    const bool is_crab_job;
    const std::string x_section_file;
    const std::string year_str;
    const std::vector<std::string> input_files;
    const Year year;
    const std::string golden_json_file;

    TaskConfiguration(const std::string _run_config_file);
};

#endif /*TASK_CONFIGURATION_H*/
