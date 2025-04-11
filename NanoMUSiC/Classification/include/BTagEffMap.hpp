#ifndef BTAG_EFF_MAP_H
#define BTAG_EFF_MAP_H

#include "TEfficiency.h"
#include "TFile.h"
#include <memory>
#include <string>

class BTagEffMap
{
    std::string file_path;
    // std::unique_ptr<TFile> root_file;
    std::unique_ptr<TEfficiency> eff_light;
    std::unique_ptr<TEfficiency> eff_b;

  public:
    // Enum for jet flavor
    enum class Flavor
    {
        B,
        C,
        Light,
        Unknown
    };

    BTagEffMap(const std::string &process_group);

    double get_eff(const Flavor &flavor, double pt, double eta) const;
};

#endif // BTAG_EFF_MAP_H
