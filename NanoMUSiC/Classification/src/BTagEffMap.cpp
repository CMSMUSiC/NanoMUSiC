#include "BTagEffMap.hpp"
#include <TEfficiency.h>
#include <TFile.h>
#include <format>

BTagEffMap::BTagEffMap(const std::string &process_group)
{
    // Open the file
    file_path = std::format("btag_eff_maps/btag_eff_map_{}.root", process_group);
    auto root_file = std::unique_ptr<TFile>(TFile::Open(file_path.c_str(), "READ"));
    if (root_file->IsZombie() or not(root_file))
    {
        throw std::runtime_error(std::format("Error: Cannot open file {}", file_path));
    }

    auto light_map_name = std::format("{}_light_eff", process_group);
    eff_light = std::unique_ptr<TEfficiency>(root_file->Get<TEfficiency>(light_map_name.c_str()));
    if (not(eff_light))
    {
        throw std::runtime_error(
            std::format("Object '{}' is not a TEfficiency or not found in file {}", light_map_name, file_path));
    }
    root_file->Remove(eff_light.get());

    auto b_map_name = std::format("{}_b_eff", process_group);
    eff_b = std::unique_ptr<TEfficiency>(root_file->Get<TEfficiency>(b_map_name.c_str()));
    if (not(eff_b))
    {
        throw std::runtime_error(
            std::format("Object '{}' is not a TEfficiency or not found in file {}", b_map_name, file_path));
    }
    root_file->Remove(eff_b.get());
}

double BTagEffMap::get_eff(const Flavor &flavor, double pt, double eta) const
{
    switch (flavor)
    {
    case Flavor::Light:
        return 1.;
    case Flavor::B:
        return 1.;
    case Flavor::C:
        throw std::runtime_error(std::format("c Eff not supported"));
    default:
        throw std::runtime_error(std::format("Unkown flavor"));
    }
}
