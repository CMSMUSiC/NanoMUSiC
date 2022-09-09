#ifndef TOMLConfig_hh
#define TOMLConfig_hh

#include <iostream>
#include <string>

// toml++ v3.1.0
// https://github.com/marzer/tomlplusplus
#include "Tools/cpp_helper_libs/toml.h"

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

    // Get a node as an specific type.
    template <typename T>
    T get(const std::string &path) const
    {
        return *(toml_config.at_path(path).value<T>());
    }
    std::string config_file_path();
    bool check_item(const std::string &key);

  private:
    const toml::table toml_config;
    const std::string toml_config_file;
};

#endif /*TOMLConfig_hh*/
