#include "TOMLConfig.hpp"

TOMLConfig::TOMLConfig(const toml::table &_toml_config, const std::string _toml_config_file)
    : toml_config(_toml_config), toml_config_file(_toml_config_file)
{
}
const TOMLConfig TOMLConfig::make_toml_config(const std::string &_toml_config_file)
{
    try
    {
        return TOMLConfig(toml::parse_file(_toml_config_file), _toml_config_file);
    }
    catch (const toml::parse_error &err)
    {
        std::cerr << "ERROR: Config file [" << _toml_config_file << "] parsing failed.\n" << err << "\n";
        exit(-1);
    }
}

std::string TOMLConfig::config_file_path()
{
    return toml_config_file;
}

bool TOMLConfig::check_item(const std::string &path)
{
    if (toml_config.at_path(path))
    {
        return true;
    }
    return false;
}
