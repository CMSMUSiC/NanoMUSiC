#include "TaskConfiguration.hpp"

TaskConfiguration::TaskConfiguration(const std::string _run_config_file)
    : run_config_file(_run_config_file),
      run_config(TOMLConfig::make_toml_config(run_config_file)),
      output_directory(run_config.get<std::string>("output")),
      process(run_config.get<std::string>("process")),
      dataset(run_config.get<std::string>("dataset")),
      is_data(run_config.get<bool>("is_data")),
      is_crab_job(run_config.get<bool>("is_crab_job")),
      x_section_file(MUSiCTools::parse_and_expand_music_base(run_config.get<std::string>("x_section_file"))),
      year_str(run_config.get<std::string>("year")),
      input_files(run_config.get_vector<std::string>("input_files")),
      year(get_runyear(year_str)),
      era(run_config.get<std::string>("era")),
      golden_json_file(MUSiCTools::parse_and_expand_music_base(RunConfig::Runs[year].golden_json))
{
    if (is_data)
    {
        if (not std::filesystem::exists(golden_json_file))
        {
            std::stringstream error;
            error << "golden_json_file not found";
            throw MUSiCTools::config_error(error.str());
        }
    }
    if (!golden_json_file.empty())
    {
        std::cout << "INFO: Using Run/Lumi JSON file: " << golden_json_file << std::endl;
    }

    // print configuratiojn summmary
    fmt::print(fmt::emphasis::bold, "\n=====================================\n");
    fmt::print(fmt::emphasis::bold, "Task Configration:\n");
    fmt::print(fmt::emphasis::bold, "-------------------------------------\n");
    fmt::print(fmt::emphasis::bold, "Configuration file: {}\n", run_config_file);
    fmt::print(fmt::emphasis::bold, "Output Directory: {}\n", output_directory);
    fmt::print(fmt::emphasis::bold, "Process Name: {}\n", process);
    fmt::print(fmt::emphasis::bold, "Dataset: {}\n", dataset);
    fmt::print(fmt::emphasis::bold, "Is Data (?): {}\n", is_data);
    fmt::print(fmt::emphasis::bold, "Is a CRAB job (?): {}\n", is_crab_job);
    fmt::print(fmt::emphasis::bold, "Cross-sections File: {}\n", x_section_file);
    fmt::print(fmt::emphasis::bold, "Year: {}\n", year_str);
    if (is_data)
    {
        fmt::print(fmt::emphasis::bold, "Era: {}\n", year_str);
    }
    fmt::print(fmt::emphasis::bold, "-------------------------------------\n");
}