#ifndef MAKE_MEP_HPP
#define MAKE_MEP_HPP

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "music_objects.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

namespace ObjectFactories
{

inline auto make_met(const double met_pt,                      //
                     const double met_phi,                     //
                     const double delta_met_px_from_muons,     //
                     const double delta_met_py_from_muons,     //
                     const double delta_met_px_from_electrons, //
                     const double delta_met_py_from_electrons, //
                     const double delta_met_px_from_taus,      //
                     const double delta_met_py_from_taus,      //
                     const double delta_met_px_from_photons,   //
                     const double delta_met_py_from_photons,   //
                     const double delta_met_px_from_jets,      //
                     const double delta_met_py_from_jets,      //
                     const double delta_met_px_from_bjets,     //
                     const double delta_met_py_from_bjets,     //
                     bool is_data,                             //
                     const std::string &_year,                 //
                     const std::string &shift) -> MUSiCObjects
{

    auto year = get_runyear(_year);
    auto met_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = RVec<double>{};
    auto scale_factor_up = RVec<double>{};
    auto scale_factor_down = RVec<double>{};
    auto delta_met_x = 0.;
    auto delta_met_y = 0.;
    auto is_fake = RVec<bool>{};

    auto met_px = met_pt * std::cos(met_phi)    //
                  + delta_met_px_from_muons     //
                  + delta_met_px_from_electrons //
                  + delta_met_px_from_taus      //
                  + delta_met_px_from_photons   //
                  + delta_met_px_from_jets      //
                  + delta_met_px_from_bjets;

    auto met_py = met_pt * std::sin(met_phi)    //
                  + delta_met_py_from_muons     //
                  + delta_met_py_from_electrons //
                  + delta_met_py_from_taus      //
                  + delta_met_py_from_photons   //
                  + delta_met_py_from_jets      //
                  + delta_met_py_from_bjets;

    auto this_met = Math::PxPyPzMVector(met_px, met_py, 0., 0.);

    // TODO: Implement the Unclustered energy shifts

    bool is_good_met = this_met.pt() >= ObjConfig::MET[year].MinPt;

    if (is_good_met)
    {
        scale_factors.push_back(1.);
        scale_factor_up.push_back(1.);
        scale_factor_down.push_back(1.);
        met_p4.push_back(Math::PtEtaPhiMVector(this_met));

        is_fake.push_back(false);
    }

    return MUSiCObjects(met_p4,            //
                        scale_factors,     //
                        scale_factor_up,   //
                        scale_factor_down, //
                        delta_met_x,       //
                        delta_met_y,       //
                        is_fake);
}

} // namespace ObjectFactories

#endif // !MAKE_MEP_HPP