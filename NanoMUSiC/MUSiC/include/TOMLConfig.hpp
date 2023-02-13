#ifndef TOMLConfig_hh
#define TOMLConfig_hh

#include <iostream>
#include <string>

// toml++ v3.1.0
// https://github.com/marzer/tomlplusplus
#include "toml.hpp"

// Reads a TOML file with x-section information. Example:
//
// [DY1JetsToLL_M-50_13TeV_AM]
// XSec = 6104.0
// FilterEff = 1.0
// kFactor = 0.944
//
//

class TOMLConfig
{
  public:
    TOMLConfig(const toml::table &_toml_config, const std::string _toml_config_file);

    // Factory method to create a TOML reader that return a const TOMLConfig.
    const static TOMLConfig make_toml_config(const std::string &_toml_config_file);

    std::string config_file_path();

    bool check_item(const std::string &key);
    // Get a node as an specific type.
    template <typename T>
    T get_value(const std::string &path) const
    {
        return *(toml_config.at_path(path).value<T>());
    }

    // Get a node as an specific type.
    template <typename T>
    T get(const std::string &path) const
    {
        return get_value<T>(path);
    }

    // Get a node as vector of specific type.
    template <typename T>
    std::vector<T> get_vector(const std::string &path) const
    {
        auto _array = *(toml_config.at_path(path).as_array());

        auto _f = std::vector<T>();
        for (const auto &i : _array)
        {
            _f.push_back(*(i.template value<std::string>()));
        }
        return _f;
    }

  private:
    const toml::table toml_config;
    const std::string toml_config_file;
};

#endif /*TOMLConfig_hh*/
