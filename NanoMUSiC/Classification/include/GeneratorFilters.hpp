#ifndef GENERATOR_FILTERS_H
#define GENERATOR_FILTERS_H

#include <cstdlib>
#include <limits>

#include <functional>
#include <map>
#include <string>

#include <fmt/format.h>
#include <iostream>

#include "Configs.hpp"
#include "debug.hpp"

#include "NanoAODGenInfo.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace std::string_literals;

namespace LorentzVectorHelper
{

inline auto mass(const float &pt1,
                 const float &eta1,
                 const float &phi1,
                 const float &mass1,
                 const float &pt2,
                 const float &eta2,
                 const float &phi2,
                 const float &mass2) -> float
{
    // Conversion from (pt, eta, phi, mass) to (x, y, z, e) coordinate system
    const auto x1 = pt1 * std::cos(phi1);
    const auto y1 = pt1 * std::sin(phi1);
    const auto z1 = pt1 * std::sinh(eta1);
    const auto e1 = std::sqrt(x1 * x1 + y1 * y1 + z1 * z1 + mass1 * mass1);

    const auto x2 = pt2 * std::cos(phi2);
    const auto y2 = pt2 * std::sin(phi2);
    const auto z2 = pt2 * std::sinh(eta2);
    const auto e2 = std::sqrt(x2 * x2 + y2 * y2 + z2 * z2 + mass2 * mass2);

    // Addition of particle four-vector elements
    const auto e = e1 + e2;
    const auto x = x1 + x2;
    const auto y = y1 + y2;
    const auto z = z1 + z2;

    return std::sqrt(e * e - x * x - y * y - z * z);
}

inline auto pt(const float &pt1, const float &phi1, const float &pt2, const float &phi2) -> float
{
    const auto x1 = pt1 * std::cos(phi1);
    const auto y1 = pt1 * std::sin(phi1);

    const auto x2 = pt2 * std::cos(phi2);
    const auto y2 = pt2 * std::sin(phi2);

    // Addition of particle 2d-vector components
    const auto x = x1 + x2;
    const auto y = y1 + y2;

    return std::sqrt(x * x + y * y);
}

} // namespace LorentzVectorHelper

namespace GeneratorFilters
{
auto no_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, debugger_t &h_debug) -> bool;

auto dy_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
               const float &mass_min,
               const float &mass_max,
               const float &pt_min,
               const float &pt_max,
               debugger_t &h_debug) -> bool;

auto ttbar_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
                  const float &mass_min,
                  const float &mass_max,
                  debugger_t &h_debug) -> bool;

auto wg_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, const float &pt_max, debugger_t &h_debug) -> bool;

auto zg_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, const float &pt_max, debugger_t &h_debug) -> bool;

auto wwto2l2nu_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, const float &mass_max, debugger_t &h_debug)
    -> bool;

auto wlnujets_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
                     const float &mass_min,
                     const float &mass_max,
                     const float &pt_min,
                     const float &pt_max,
                     debugger_t &h_debug) -> bool;

auto wlnujets_mass_binned_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
                                 const NanoAODGenInfo::GenParticles &gen_particles,
                                 const Year &year,
                                 const float &mass_min,
                                 const float &mass_max,
                                 const float &pt_min,
                                 const float &pt_max,
                                 debugger_t &h_debug) -> bool;

auto wlnujets_mass_binned_sherpa_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
                                        const NanoAODGenInfo::GenParticles &gen_particles,
                                        const Year &year,
                                        const float &mass_min,
                                        const float &mass_max,
                                        const float &pt_min,
                                        const float &pt_max,
                                        debugger_t &h_debug) -> bool;

auto ww_2l2v_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, const float &mass_max, debugger_t &h_debug)
    -> bool;

auto gamma_jet_cleanner_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, float dr_max, debugger_t &h_debug)
    -> bool;

constexpr float MAX_FLOAT = std::numeric_limits<float>::max();

// dummy filter
// const std::map<std::string, std::function<bool(const NanoAODGenInfo::LHEParticles &)>> filters = {};

using Filter_t = std::function<bool(const NanoAODGenInfo::LHEParticles &lhe_particles,
                                    const NanoAODGenInfo::GenParticles &gen_particles,
                                     Year &year,
                                    debugger_t &h_debug)>;
auto get_filter(const std::string &filter_name) -> Filter_t;

const std::map<std::string, Filter_t> filters = {
    // TTBar Samples
    {"ttbar_mass_less_700"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return ttbar_filter(lhe_particles, 0., 700., h_debug);
     }},

    // DY Jets - Low Mass sample
    {"dyjets_pt_less_50"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return dy_filter(lhe_particles, 0., MAX_FLOAT, 0., 50., h_debug);
     }},

    //  DY Jets - pT binned
    {"dyjets_mass_less_120_mass_greater_50"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return dy_filter(lhe_particles, 50., 120., 0., MAX_FLOAT, h_debug);
     }},

    //  DY Jets - Very Low Mass
    {"dyjets_mass_less_10"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return dy_filter(lhe_particles, 0., 10., 0., MAX_FLOAT, h_debug);
     }},

    // DY Jets - Inclusive
    {"dyjets_pt_less_50_mass_less_120"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return dy_filter(lhe_particles, 0., 120., 0., 50, h_debug);
     }},

    // WW full-leptonic sample - inclusive
    {"ww_mLL_less_200"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return ww_2l2v_filter(lhe_particles, 200., h_debug);
     }},
    // WW full-leptonic sample - mass binned
    {"ww_mLL_less_1800"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return ww_2l2v_filter(lhe_particles, 1800., h_debug);
     }},

    // W Gamma
    {"pt_gamma_less_130"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wg_filter(lhe_particles, 130., h_debug);
     }},
    {"pt_gamma_less_300"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wg_filter(lhe_particles, 300., h_debug);
     }},
    {"pt_gamma_less_500"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wg_filter(lhe_particles, 500., h_debug);
     }},

    //  QCD gamma cleanner
    {"gamma_plus_jets_cleanner"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return gamma_jet_cleanner_filter(lhe_particles, 0.4, h_debug);
     }},

    //  W Jets - Inclusive - amc@nlo
    {"w_plus_jets_pt_less_100_mass_less_200"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wlnujets_filter(lhe_particles, 0., 200., 0., 100., h_debug);
     }},

    // WJets - Inclusive - SHERPA
    {"wjets_mass_less_200_pt_less_100_sherpa"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wlnujets_mass_binned_sherpa_filter(lhe_particles, gen_particles, year, 0, 200., 0., 100., h_debug);
     }},

    //  W Jets - pT binned
    {"w_plus_jets_mass_less_200"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wlnujets_filter(lhe_particles, 0., 200., 0., MAX_FLOAT, h_debug);
     }},

    // WJets - Mass binned samples
    {"wjets_mass_less_500"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         if (year == Year::Run2016 or year == Year::Run2016APV)
         {
             return true;
         }
         return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 500., 0, MAX_FLOAT, h_debug);
     }},
    {"wjets_mass_less_1000"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 1000., 0, MAX_FLOAT, h_debug);
     }},
    {"wjets_mass_less_2000"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 2000., 0, MAX_FLOAT, h_debug);
     }},
    {"wjets_mass_less_3000"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 3000., 0, MAX_FLOAT, h_debug);
     }},
    {"wjets_mass_less_4000"s,
     [](const NanoAODGenInfo::LHEParticles &lhe_particles,
        const NanoAODGenInfo::GenParticles &gen_particles,
        Year &year,
        debugger_t &h_debug) -> bool
     {
         return wlnujets_mass_binned_filter(lhe_particles, gen_particles, year, 0., 4000., 0, MAX_FLOAT, h_debug);
     }},

    // {"DYJetsToLL_M-10To50_13TeV_AM"s,
    //  // {"DoLL_M-10To50_13TeV_AM"s,
    //  [](const NanoAODGenInfo::LHEParticles &lhe_particles, Year &year, debugger_t &h_debug) -> bool
    //  {
    //      return dy_filter(lhe_particles, 0., MAX_FLOAT, 0., 100.);
    //  }},
    // {"DYJetsToLL_M-50_13TeV_AM"s,
    //  [](const NanoAODGenInfo::LHEParticles &lhe_particles, Year &year, debugger_t &h_debug) -> bool
    //  {
    //      return dy_filter(lhe_particles, 0., 120., 0., 100.);
    //  }},
    // {"DYJetsToLL_LHEFilterPtZ-400To650"s,
    //  [](const NanoAODGenInfo::LHEParticles &lhe_particles, Year &year, debugger_t &h_debug) -> bool
    //  {
    //      return dy_filter(lhe_particles, 0., 120., 100., MAX_FLOAT);
    //  }},
    // {"DYToEE_M-120To200_TuneCP5_13TeV-powheg-pythia8"s,
    //  [](const NanoAODGenInfo::LHEParticles &lhe_particles, Year &year, debugger_t &h_debug) -> bool
    //  {
    //      return dy_filter(lhe_particles, 120., MAX_FLOAT, 0., MAX_FLOAT);
    //  }},
    // Not needed for now. Samples have to be requested/followed-up.
    // {"ZG"s, [](const NanoAODGenInfo::LHEParticles &lhe_particles) -> bool { return zg_filter(lhe_particles,
    // 500.);
};

// }
// ,
//     {"WWTo2L2Nu_13TeV_PH"s,
//      [](const NanoAODGenInfo::LHEParticles &lhe_particles) -> bool
//      {
//          return wwto2l2nu_filter(lhe_particles, 200.);
//      }},
//     {"WJets"s,
//      [](const NanoAODGenInfo::LHEParticles &lhe_particles) -> bool
//      {
//          return wlnujets_filter(lhe_particles, 0., 120., 0., 500.);
//      }},
// }
// ;

} // namespace GeneratorFilters

#endif // GENERATOR_FILTERS_H
