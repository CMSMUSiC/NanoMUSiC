#ifndef BTAG_EFF_MAP_H
#define BTAG_EFF_MAP_H

#include <string>

class BTagEffMap
{
  public:
    // Enum for jet flavor
    enum class Flavor
    {
        B,
        C,
        Light,
        Unknown
    };

    BTagEffMap(const std::string & process_group);

    double get_eff(const Flavor &flavor, double pt, double eta) const;
};

#endif // BTAG_EFF_MAP_H
