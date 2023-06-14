#ifndef MAKE_MUONS_HPP
#define MAKE_MUONS_HPP

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

inline auto make_muons(const RVec<float> &Muon_pt,             //
                       const RVec<float> &Muon_eta,            //
                       const RVec<float> &Muon_phi,            //
                       const RVec<bool> &Muon_tightId,         //
                       const RVec<UChar_t> &Muon_highPtId,     //
                       const RVec<float> &Muon_pfRelIso04_all, //
                       const RVec<float> &Muon_tkRelIso,       //
                       const RVec<float> &Muon_tunepRelPt,     //
                       std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto muons = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Muon_pt.size(); i++)
    {
        bool is_good_low_pt_muon = (Muon_pt.at(i) >= ObjConfig::Muons[year].MinLowPt)                 //
                                   && (Muon_pt.at(i) < ObjConfig::Muons[year].MaxLowPt)               //
                                   && (std::fabs(Muon_eta.at(i)) <= ObjConfig::Muons[year].MaxAbsEta) //
                                   && (Muon_tightId.at(i))                                            //
                                   && (Muon_pfRelIso04_all.at(i) < ObjConfig::Muons[year].PFRelIso_WP);

        bool is_good_high_pt_muon = (Muon_pt.at(i) >= ObjConfig::Muons[year].MaxLowPt)                 //
                                    && (std::fabs(Muon_eta.at(i)) <= ObjConfig::Muons[year].MaxAbsEta) //
                                    && (Muon_highPtId.at(i) >= 2)                                      //
                                    && (Muon_tkRelIso.at(i) < ObjConfig::Muons[year].TkRelIso_WP);

        double pt_correction_factor = 1.;
        if (is_good_high_pt_muon)
        {
            // For some reason, the Relative pT Tune can yield very low corrected pT. Because of this,
            // they will be caped to 25., in order to not break the JSON SFs bound checks.
            // https://cms-nanoaod-integration.web.cern.ch/commonJSONSFs/summaries/MUO_2016postVFP_UL_muon_Z.html
            // leading_muon.SetPt(std::max(Muon_tunepRelPt[0] * leading_muon.pt(), 25.));
            pt_correction_factor = Muon_tunepRelPt.at(i);
        }

        if (is_good_low_pt_muon or is_good_high_pt_muon)
        {
            muons.push_back(Math::PtEtaPhiMVector(
                std::max(Muon_pt[i] * pt_correction_factor, 25.), Muon_eta[i], Muon_phi[i], PDG::Muon::Mass));
        }
    }
    const auto muon_reordering_mask = VecOps::Argsort(muons,
                                                      [](auto muon_1, auto muon_2) -> bool
                                                      {
                                                          return muon_1.pt() > muon_2.pt();
                                                      });

    return VecOps::Take(muons, muon_reordering_mask);

}

} // namespace ObjectFactories

#endif // !MAKE_MUONS_HPP