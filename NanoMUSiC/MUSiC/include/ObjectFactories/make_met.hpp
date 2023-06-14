#ifndef MAKE_MEP_HPP
#define MAKE_MEP_HPP

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

namespace ObjectFactories
{

inline auto make_met(const float met_px, //
                     const float met_py, //
                     std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto this_met = Math::PxPyPzMVector(met_px, met_py, 0., 0.);
    auto met = RVec<Math::PtEtaPhiMVector>{};

    bool is_good_met = this_met.pt() >= ObjConfig::MET[year].MinPt;

    if (is_good_met)
    {
        met.push_back(Math::PtEtaPhiMVector(this_met.pt(), 0., this_met.phi(), 0.));
    }

    return met;
}

} // namespace ObjectFactories

#endif // !MAKE_MEP_HPP