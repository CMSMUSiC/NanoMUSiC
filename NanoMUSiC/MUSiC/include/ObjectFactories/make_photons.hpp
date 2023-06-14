#ifndef MAKE_PHOTONS_HPP
#define MAKE_PHOTONS_HPP

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

inline auto make_photons(const RVec<float> &Photon_pt,  //
                         const RVec<float> &Photon_eta, //
                         const RVec<float> &Photon_phi, //
                         const RVec<bool> &Photon_isScEtaEB,
                         const RVec<bool> &Photon_isScEtaEE,
                         const RVec<Int_t> &Photon_cutBased, //
                         const RVec<bool> &Photon_pixelSeed, //
                         std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto photons = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Photon_pt.size(); i++)
    {

        bool is_good_photon = (Photon_pt.at(i) >= ObjConfig::Photons[year].MinPt)               //
                              && (Photon_isScEtaEB.at(i))                                       //
                              && (not Photon_isScEtaEE.at(i))                                   // only EB photons
                              && (Photon_cutBased.at(i) >= ObjConfig::Photons[year].cutBasedId) //
                              && (Photon_pixelSeed.at(i) == false);

        if (is_good_photon)
        {
            photons.push_back(Math::PtEtaPhiMVector(Photon_pt[i], Photon_eta[i], Photon_phi[i], PDG::Photon::Mass));
        }
    }
    const auto photon_reordering_mask = VecOps::Argsort(photons,
                                                        [](auto photon_1, auto photon_2) -> bool
                                                        {
                                                            return photon_1.pt() > photon_2.pt();
                                                        });

    return VecOps::Take(photons, photon_reordering_mask);
}

} // namespace ObjectFactories

#endif // !MAKE_PHOTONS_HPP