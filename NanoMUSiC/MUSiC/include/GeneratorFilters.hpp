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
#include "debug.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace std::string_literals;

namespace GeneratorFilters
{
auto no_filter(const NanoObjects::LHEParticles &lhe_particles, debugger_t &h_debug) -> bool;

auto dy_filter(const NanoObjects::LHEParticles &lhe_particles,
               const float &mass_min,
               const float &mass_max,
               const float &pt_min,
               const float &pt_max,
               debugger_t &h_debug) -> bool;

auto ttbar_filter(const NanoObjects::LHEParticles &lhe_particles,
                  const float &mass_min,
                  const float &mass_max,
                  debugger_t &h_debug) -> bool;
auto wg_filter(const NanoObjects::LHEParticles &lhe_particles, const float &pt_max, debugger_t &h_debug) -> bool;
auto zg_filter(const NanoObjects::LHEParticles &lhe_particles, const float &pt_max, debugger_t &h_debug) -> bool;
auto wwto2l2nu_filter(const NanoObjects::LHEParticles &lhe_particles, const float &mass_max, debugger_t &h_debug)
    -> bool;
auto wlnujets_filter(const NanoObjects::LHEParticles &lhe_particles,
                     const float &mass_min,
                     const float &mass_max,
                     const float &pt_min,
                     const float &pt_max,
                     debugger_t &h_debug) -> bool;
auto wlnujets_mass_binned_filter(const NanoObjects::LHEParticles &lhe_particles,
                                 const NanoObjects::GenParticles &gen_particles,
                                 const Year &year,
                                 const float &mass_min,
                                 const float &mass_max,
                                 debugger_t &h_debug) -> bool;
auto ww_2l2v_filter(const NanoObjects::LHEParticles &lhe_particles, const float &mass_max, debugger_t &h_debug) -> bool;
auto gamma_jet_cleanner_filter(const NanoObjects::LHEParticles &lhe_particles, float dr_max, debugger_t &h_debug)
    -> bool;

constexpr float MAX_FLOAT = std::numeric_limits<float>::max();

// dummy filter
// const std::map<std::string, std::function<bool(const NanoObjects::LHEParticles &)>> filters = {};

const std::map<std::string,
               std::function<bool(const NanoObjects::LHEParticles &lhe_particles,
                                  const NanoObjects::GenParticles &gen_particles,
                                  Year &year,
                                  debugger_t &h_debug)>>
    filters = {
        {"ttbar_mass_less_700"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return ttbar_filter(lhe_particles, 0., 700., h_debug);
         }},
        {"dyjets_pt_less_50"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return dy_filter(lhe_particles, 0., MAX_FLOAT, 0., 50., h_debug);
         }},
        {"dyjets_mass_less_120"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return dy_filter(lhe_particles, 0., 120., 0., MAX_FLOAT, h_debug);
         }},
        {"dyjets_pt_less_50_mass_less_120"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return dy_filter(lhe_particles, 0., 120., 0., 50, h_debug);
         }},
        {"ww_mLL_less_1800"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return ww_2l2v_filter(lhe_particles, 1800., h_debug);
         }},
        {"pt_gamma_less_130"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wg_filter(lhe_particles, 130., h_debug);
         }},
        {"pt_gamma_less_300"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wg_filter(lhe_particles, 300., h_debug);
         }},
        {"pt_gamma_less_500"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wg_filter(lhe_particles, 500., h_debug);
         }},
        {"gamma_plus_jets_cleanner"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return gamma_jet_cleanner_filter(lhe_particles, 0.4, h_debug);
         }},
        {"w_plus_jets_pt_less_100_mass_less_200"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wlnujets_filter(lhe_particles, 0., 200., 0., 100., h_debug);
         }},
        {"w_plus_jets_mass_less_200"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wlnujets_filter(lhe_particles, 0., 200., 0., MAX_FLOAT, h_debug);
         }},
        // Mass binned samples
        {"wjets_mass_less_500"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 500., h_debug);
         }},
        {"wjets_mass_less_1000"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 1000., h_debug);
         }},
        {"wjets_mass_less_2000"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 2000., h_debug);
         }},
        {"wjets_mass_less_3000"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 3000., h_debug);
         }},
        {"wjets_mass_less_4000"s,
         [](const NanoObjects::LHEParticles &lhe_particles,
            const NanoObjects::GenParticles &gen_particles,
            Year &year,
            debugger_t &h_debug) -> bool
         {
             return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 4000., h_debug);
         }},
        // {"DYJetsToLL_M-10To50_13TeV_AM"s,
        //  // {"DoLL_M-10To50_13TeV_AM"s,
        //  [](const NanoObjects::LHEParticles &lhe_particles, Year &year, debugger_t &h_debug) -> bool
        //  {
        //      return dy_filter(lhe_particles, 0., MAX_FLOAT, 0., 100.);
        //  }},
        // {"DYJetsToLL_M-50_13TeV_AM"s,
        //  [](const NanoObjects::LHEParticles &lhe_particles, Year &year, debugger_t &h_debug) -> bool
        //  {
        //      return dy_filter(lhe_particles, 0., 120., 0., 100.);
        //  }},
        // {"DYJetsToLL_LHEFilterPtZ-400To650"s,
        //  [](const NanoObjects::LHEParticles &lhe_particles, Year &year, debugger_t &h_debug) -> bool
        //  {
        //      return dy_filter(lhe_particles, 0., 120., 100., MAX_FLOAT);
        //  }},
        // {"DYToEE_M-120To200_TuneCP5_13TeV-powheg-pythia8"s,
        //  [](const NanoObjects::LHEParticles &lhe_particles, Year &year, debugger_t &h_debug) -> bool
        //  {
        //      return dy_filter(lhe_particles, 120., MAX_FLOAT, 0., MAX_FLOAT);
        //  }},
        // Not needed for now. Samples have to be requested/followed-up.
        // {"ZG"s, [](const NanoObjects::LHEParticles &lhe_particles) -> bool { return zg_filter(lhe_particles, 500.);
};
// }
// ,
//     {"WWTo2L2Nu_13TeV_PH"s,
//      [](const NanoObjects::LHEParticles &lhe_particles) -> bool
//      {
//          return wwto2l2nu_filter(lhe_particles, 200.);
//      }},
//     {"WJets"s,
//      [](const NanoObjects::LHEParticles &lhe_particles) -> bool
//      {
//          return wlnujets_filter(lhe_particles, 0., 120., 0., 500.);
//      }},
// }
// ;
} // namespace GeneratorFilters

#endif // GENERATOR_FILTERS_H