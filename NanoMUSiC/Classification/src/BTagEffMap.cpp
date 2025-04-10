#include "BTagEffMap.hpp"
#include <TEfficiency.h>
#include <TFile.h>
#include <format>

BTagEffMap::BTagEffMap(const std::string &process_group)
{
    // Open the file
    auto file_path = std::format("btag_eff_maps/btag_eff_map_{}.root", process_group);
    auto root_file = std::unique_ptr<TFile>(TFile::Open(file_path.c_str(), "READ"));
    if (file->IsZombie() or not(file))
    {
        throw std::runtime_error(std::format("Error: Cannot open file {}", file_path));
    }

    // Get the object and cast to TEfficiency
    //
    TEfficiency *eff = dynamic_cast<TEfficiency *>(file.Get(objname.c_str()));
    auto p = file->Get<TEfficiency>("DrellYan_light_eff");

    if (!eff)
    {
        std::cerr << "Error: Object '" << objname << "' is not a TEfficiency or not found in file " << filename
                  << std::endl;
        return nullptr;
    }

    // Clone the object to create a unique_ptr (since the file will close)
    std::unique_ptr<TEfficiency> effCopy(static_cast<TEfficiency *>(eff->Clone()));
    return effCopy;
}

double BTagEffMap::get_eff(const Flavor &flavor, double pt, double eta) const
{
    return 0.0; // Default fallback if no bin matches
}
