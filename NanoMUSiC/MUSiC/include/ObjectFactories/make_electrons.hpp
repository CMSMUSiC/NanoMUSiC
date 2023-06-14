#ifndef MAKE_ELECTRONS_HPP
#define MAKE_ELECTRONS_HPP

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

inline auto make_electrons(const RVec<float> &Electron_pt,  //
                           const RVec<float> &Electron_eta, //
                           const RVec<float> &Electron_phi, //
                           const RVec<float> &Electron_deltaEtaSC,
                           const RVec<Int_t> &Electron_cutBased,     //
                           const RVec<bool> &Electron_cutBased_HEEP, //
                           std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto electrons = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Electron_pt.size(); i++)
    {

        // Low pT Electrons
        bool is_good_low_pt_electron = ((Electron_pt.at(i) >= ObjConfig::Electrons[year].MinLowPt) &&
                                        (Electron_pt.at(i) < ObjConfig::Electrons[year].MaxLowPt)) //
                                       && ((std::fabs(Electron_eta.at(i) + Electron_deltaEtaSC.at(i)) <= 1.442) ||
                                           ((std::fabs(Electron_eta.at(i) + Electron_deltaEtaSC.at(i)) >= 1.566) &&
                                            (std::fabs(Electron_eta.at(i) + Electron_deltaEtaSC.at(i)) <= 2.5))) //
                                       && (Electron_cutBased.at(i) >= ObjConfig::Electrons[year].cutBasedId);

        // High pT Electrons
        bool is_good_high_pt_electron = (Electron_pt.at(i) >= ObjConfig::Electrons[year].MaxLowPt) //
                                        && ((std::fabs(Electron_eta.at(i) + Electron_deltaEtaSC.at(i)) <= 1.442) ||
                                            ((std::fabs(Electron_eta.at(i) + Electron_deltaEtaSC.at(i)) >= 1.566) &&
                                             (std::fabs(Electron_eta.at(i) + Electron_deltaEtaSC.at(i)) <= 2.5))) //
                                        && (Electron_cutBased_HEEP.at(i));

        if (is_good_low_pt_electron or is_good_high_pt_electron)
        {
            electrons.emplace_back(Electron_pt[i], Electron_eta[i], Electron_phi[i], PDG::Electron::Mass);
        }
    }
    const auto electron_reordering_mask = VecOps::Argsort(electrons,
                                                          [](auto electron_1, auto electron_2) -> bool
                                                          {
                                                              return electron_1.pt() > electron_2.pt();
                                                          });

    return VecOps::Take(electrons, electron_reordering_mask);

    return electrons;
}

} // namespace ObjectFactories

#endif // !MAKE_ELECTRONS_HPP