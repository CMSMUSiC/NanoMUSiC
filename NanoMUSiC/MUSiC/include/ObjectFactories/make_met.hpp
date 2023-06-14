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

inline auto make_met(const float MET_pt,  //
                     const float MET_phi, //
                     std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto met = RVec<Math::PtEtaPhiMVector>{};

    bool is_good_met = MET_pt >= ObjConfig::MET[year].MinPt;

    if (is_good_met)
    {
        met.emplace_back(MET_pt, 0., MET_phi, 0.);
    }

    return met;
}

} // namespace ObjectFactories

#endif // !MAKE_MEP_HPP