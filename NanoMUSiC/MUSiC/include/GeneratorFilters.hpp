#ifndef GENERATOR_FILTERS_H
#define GENERATOR_FILTERS_H

#include <limits>
#include <optional>

#include <functional>
#include <map>
#include <string>
#include <variant>

#include <fmt/format.h>

#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "NanoObjects.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace std::string_literals;

namespace GeneratorFilters
{
auto no_filter(const NanoObjects::LHEParticles &lhe_particles) -> bool;

auto dy_filter(const NanoObjects::LHEParticles &lhe_particles,
               const float &mass_min,
               const float &mass_max,
               const float &pt_min,
               const float &pt_max) -> bool;

auto ttbar_filter(const NanoObjects::LHEParticles &lhe_particles, const float &mass_min, const float &mass_max) -> bool;
auto wg_filter(const NanoObjects::LHEParticles &lhe_particles, const float &pt_max) -> bool;
auto zg_filter(const NanoObjects::LHEParticles &lhe_particles, const float &pt_max) -> bool;
auto wwtro2l2nu_filter(const NanoObjects::LHEParticles &lhe_particles, const float &mass_max) -> bool;

constexpr float max_float = std::numeric_limits<float>::max();

const std::map<std::string, std::function<bool(const NanoObjects::LHEParticles &)>> filters = {
    // {"QCD_SOMETHING"s, no_filter}, //
    {"DYJetsToLL_M-10To50_13TeV_AM"s,
     [](const NanoObjects::LHEParticles &lhe_particles) -> bool {
         return dy_filter(lhe_particles, 0., max_float, 0., 100.);
     }}, //
    {"DYJetsToLL_M-50_13TeV_AM"s,
     [](const NanoObjects::LHEParticles &lhe_particles) -> bool {
         return dy_filter(lhe_particles, 0., 120., 0., 100.);
     }}, //
    {"DYJetsToLL_LHEFilterPtZ-400To650"s,
     [](const NanoObjects::LHEParticles &lhe_particles) -> bool {
         return dy_filter(lhe_particles, 0., 120., 100., max_float);
     }}, //
    {"DYToEE_M-120To200_TuneCP5_13TeV-powheg-pythia8"s,
     [](const NanoObjects::LHEParticles &lhe_particles) -> bool {
         return dy_filter(lhe_particles, 120., max_float, 0., max_float);
     }}, //
    {"TTToSemiLeptonic_2018"s,
     [](const NanoObjects::LHEParticles &lhe_particles) -> bool { return ttbar_filter(lhe_particles, 0., 700.); }}, //
    {"WG"s, [](const NanoObjects::LHEParticles &lhe_particles) -> bool { return wg_filter(lhe_particles, 500.); }}, //
    // Not needed for now. Samples have to be requested/followed-up.
    // {"ZG"s, [](const NanoObjects::LHEParticles &lhe_particles) -> bool { return zg_filter(lhe_particles, 500.); }},
    // //
    {"WWTo2L2Nu_13TeV_PH"s,
     [](const NanoObjects::LHEParticles &lhe_particles) -> bool { return wwtro2l2nu_filter(lhe_particles, 200.); }}, //
};
} // namespace GeneratorFilters

#endif // GENERATOR_FILTERS_H