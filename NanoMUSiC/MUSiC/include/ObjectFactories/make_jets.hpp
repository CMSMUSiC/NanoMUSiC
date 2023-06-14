#ifndef MAKE_JETS_HPP
#define MAKE_JETS_HPP

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "JetCorrector.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

namespace ObjectFactories
{
inline auto make_jets(const RVec<float> &Jet_pt,            //
                      const RVec<float> &Jet_eta,           //
                      const RVec<float> &Jet_phi,           //
                      const RVec<float> &Jet_mass,          //
                      const RVec<Int_t> &Jet_jetId,         //
                      const RVec<float> &Jet_btagDeepFlavB, //
                      const RVec<float> &Jet_rawFactor,     //
                      const RVec<float> &Jet_area,          //
                      const RVec<Int_t> &Jet_genJetIdx,     //
                      float fixedGridRhoFastjetAll,         //
                      JetCorrector &jet_corrections,        //
                      const NanoObjects::GenJets &gen_jets, //
                      std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto bjets = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Jet_pt.size(); i++)
    {
        // JES: Nominal - JER: Nominal
        float scale_correction_nominal = jet_corrections.get_scale_correction(Jet_pt[i],              //
                                                                              Jet_eta[i],             //
                                                                              Jet_phi[i],             //
                                                                              Jet_rawFactor[i],       //
                                                                              fixedGridRhoFastjetAll, //
                                                                              Jet_area[i],            //
                                                                              "Nominal"s);

        float new_pt_nominal = Jet_pt[i] * scale_correction_nominal;

        float resolution_correction_nominal = jet_corrections.get_resolution_correction(new_pt_nominal,
                                                                                        Jet_eta[i],             //
                                                                                        Jet_phi[i],             //
                                                                                        fixedGridRhoFastjetAll, //
                                                                                        Jet_genJetIdx[i],       //
                                                                                        gen_jets,               //
                                                                                        "Nominal"s);
        auto is_good_jet =
            (Jet_pt.at(i) * scale_correction_nominal * resolution_correction_nominal >= ObjConfig::Jets[year].MinPt) //
            && (std::fabs(Jet_eta.at(i)) <= ObjConfig::Jets[year].MaxAbsEta)                                         //
            && (Jet_jetId.at(i) >= ObjConfig::Jets[year].MinJetID)                                                   //
            && (Jet_btagDeepFlavB.at(i) < ObjConfig::Jets[year].MaxBTagWPTight);

        if (is_good_jet)
        {
            bjets.emplace_back(                                                          //
                Jet_pt.at(i) * scale_correction_nominal * resolution_correction_nominal, //
                Jet_eta.at(i),                                                           //
                Jet_phi.at(i),                                                           //
                Jet_mass.at(i) * scale_correction_nominal * resolution_correction_nominal);
        }
    }

    const auto jets_reordering_mask = VecOps::Argsort(bjets,
                                                      [](auto jet_1, auto jet_2) -> bool
                                                      {
                                                          return jet_1.pt() > jet_2.pt();
                                                      });

    return VecOps::Take(bjets, jets_reordering_mask);

    return bjets;
}

inline auto make_bjets(const RVec<float> &Jet_pt,            //
                       const RVec<float> &Jet_eta,           //
                       const RVec<float> &Jet_phi,           //
                       const RVec<float> &Jet_mass,          //
                       const RVec<Int_t> &Jet_jetId,         //
                       const RVec<float> &Jet_btagDeepFlavB, //
                       const RVec<float> &Jet_rawFactor,     //
                       const RVec<float> &Jet_area,          //
                       const RVec<Int_t> &Jet_genJetIdx,     //
                       float fixedGridRhoFastjetAll,         //
                       JetCorrector &jet_corrections,        //
                       const NanoObjects::GenJets &gen_jets, //
                       std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto bjets = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Jet_pt.size(); i++)
    {
        // JES: Nominal - JER: Nominal
        float scale_correction_nominal = jet_corrections.get_scale_correction(Jet_pt[i],              //
                                                                              Jet_eta[i],             //
                                                                              Jet_phi[i],             //
                                                                              Jet_rawFactor[i],       //
                                                                              fixedGridRhoFastjetAll, //
                                                                              Jet_area[i],            //
                                                                              "Nominal"s);

        float new_pt_nominal = Jet_pt[i] * scale_correction_nominal;

        float resolution_correction_nominal = jet_corrections.get_resolution_correction(new_pt_nominal,
                                                                                        Jet_eta[i],             //
                                                                                        Jet_phi[i],             //
                                                                                        fixedGridRhoFastjetAll, //
                                                                                        Jet_genJetIdx[i],       //
                                                                                        gen_jets,               //
                                                                                        "Nominal"s);
        auto is_good_jet =
            (Jet_pt.at(i) * scale_correction_nominal * resolution_correction_nominal >= ObjConfig::Jets[year].MinPt) //
            && (std::fabs(Jet_eta.at(i)) <= ObjConfig::Jets[year].MaxAbsEta)                                         //
            && (Jet_jetId.at(i) >= ObjConfig::Jets[year].MinJetID)                                                   //
            && (Jet_btagDeepFlavB.at(i) >= ObjConfig::Jets[year].MaxBTagWPTight);
        if (is_good_jet)
        {
            bjets.emplace_back(                                                          //
                Jet_pt.at(i) * scale_correction_nominal * resolution_correction_nominal, //
                Jet_eta.at(i),                                                           //
                Jet_phi.at(i),                                                           //
                Jet_mass.at(i) * scale_correction_nominal * resolution_correction_nominal);
        }
    }

    const auto bjets_reordering_mask = VecOps::Argsort(bjets,
                                                       [](auto jet_1, auto jet_2) -> bool
                                                       {
                                                           return jet_1.pt() > jet_2.pt();
                                                       });

    return VecOps::Take(bjets, bjets_reordering_mask);

    return bjets;
}
} // namespace ObjectFactories

#endif // !MAKE_JETS_HPP