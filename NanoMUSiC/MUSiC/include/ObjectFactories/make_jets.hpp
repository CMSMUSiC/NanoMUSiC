#ifndef MAKE_JETS_HPP
#define MAKE_JETS_HPP

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "CorrectionSets.hpp"
#include "JetCorrector.hpp"
#include "Shifts.hpp"
#include "music_objects.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;
namespace ObjectFactories
{

inline auto get_scale_resolution_shifts(const Shifts::Variations shift) -> std::pair<std::string, std::string>
{
    if (shift == Shifts::Variations::JetScale_Up)
    {
        return std::make_pair<std::string, std::string>("Up", "Nominal");
    }

    if (shift == Shifts::Variations::JetScale_Down)
    {
        return std::make_pair<std::string, std::string>("Down", "Nominal");
    }

    if (shift == Shifts::Variations::JetResolution_Up)
    {
        return std::make_pair<std::string, std::string>("Nominal", "Up");
    }

    if (shift == Shifts::Variations::JetResolution_Down)
    {
        return std::make_pair<std::string, std::string>("Nominal", "Down");
    }

    return std::make_pair<std::string, std::string>("Nominal", "Nominal");
}

inline auto get_jet_energy_corrections(const Shifts::Variations shift,
                                       float Jet_pt,                 //
                                       float Jet_eta,                //
                                       float Jet_phi,                //
                                       float Jet_rawFactor,          //
                                       float Jet_area,               //
                                       Int_t Jet_genJetIdx,          //
                                       float fixedGridRhoFastjetAll, //
                                       JetCorrector &jet_corrections,
                                       const NanoObjects::GenJets &gen_jets) -> double
{

    auto [scale_shift, resolution_shift] = get_scale_resolution_shifts(shift);

    // The JetCorrector already knows is_data (from the constructor)
    // JES: Nominal - JER: Nominal
    float scale_correction_nominal = jet_corrections.get_scale_correction(Jet_pt,                 //
                                                                          Jet_eta,                //
                                                                          Jet_phi,                //
                                                                          Jet_rawFactor,          //
                                                                          fixedGridRhoFastjetAll, //
                                                                          Jet_area,               //
                                                                          scale_shift);

    float new_pt_nominal = Jet_pt * scale_correction_nominal;

    float resolution_correction_nominal = jet_corrections.get_resolution_correction(new_pt_nominal,
                                                                                    Jet_eta,                //
                                                                                    Jet_phi,                //
                                                                                    fixedGridRhoFastjetAll, //
                                                                                    Jet_genJetIdx,          //
                                                                                    gen_jets,               //
                                                                                    resolution_shift);

    return scale_correction_nominal * resolution_correction_nominal;
}

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
                                                            //   BTagSFCorrector &btag_sf_Corrector,   //
                      const NanoObjects::GenJets &gen_jets, //
                      bool is_data,                         //
                      const std::string &_year,             //
                      const Shifts::Variations shift) -> std::pair<MUSiCObjects, MUSiCObjects>
{
    auto year = get_runyear(_year);
    auto jets = RVec<Math::PtEtaPhiMVector>{};
    auto bjets = RVec<Math::PtEtaPhiMVector>{};
    auto jets_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto bjets_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto jets_scale_factors = RVec<double>{};
    auto bjets_scale_factors = RVec<double>{};
    auto jets_scale_factor_shift = RVec<double>{};
    auto bjets_scale_factor_shift = RVec<double>{};
    auto jets_delta_met_x = 0.;
    auto bjets_delta_met_x = 0.;
    auto jets_delta_met_y = 0.;
    auto bjets_delta_met_y = 0.;
    auto jets_is_fake = RVec<bool>{};
    auto bjets_is_fake = RVec<bool>{};

    for (std::size_t i = 0; i < Jet_pt.size(); i++)
    {

        auto is_good_jet_pre_filter = (std::fabs(Jet_eta[i]) <= ObjConfig::Jets[year].MaxAbsEta) //
                                      and (Jet_jetId[i] >= ObjConfig::Jets[year].MinJetID)       //
                                      and (Jet_btagDeepFlavB[i] < ObjConfig::Jets[year].MaxBTagWPTight);

        auto is_good_bjet_pre_filter = (std::fabs(Jet_eta[i]) <= ObjConfig::Jets[year].MaxAbsEta) //
                                       and (Jet_jetId[i] >= ObjConfig::Jets[year].MinJetID)       //
                                       and (Jet_btagDeepFlavB[i] >= ObjConfig::Jets[year].MaxBTagWPTight);

        auto jet_p4 = Math::PtEtaPhiMVector(Jet_pt[i], Jet_eta[i], Jet_phi[i], Jet_mass[i]);

        if (jet_p4.pt() * (1. - Jet_rawFactor[i]) > 10. and std::fabs(jet_p4.eta()) < 5.2)
        {
            auto jet_energy_corrections = get_jet_energy_corrections(shift,
                                                                     Jet_pt[i],
                                                                     Jet_eta[i],
                                                                     Jet_phi[i],
                                                                     Jet_rawFactor[i],
                                                                     Jet_area[i],
                                                                     Jet_genJetIdx[i],
                                                                     fixedGridRhoFastjetAll,
                                                                     jet_corrections,
                                                                     gen_jets);
            jet_p4 = jet_p4 * jet_energy_corrections;

            jets_delta_met_x += (jet_p4.pt() - Jet_pt[i]) * std::cos(Jet_phi[i]);
            jets_delta_met_y += (jet_p4.pt() - Jet_pt[i]) * std::sin(Jet_phi[i]);
        }

        if (is_good_jet_pre_filter or is_good_bjet_pre_filter)
        {
            auto is_good_jet = (jet_p4.pt() >= ObjConfig::Jets[year].MinPt) and is_good_jet_pre_filter;
            auto is_good_bjet = (jet_p4.pt() >= ObjConfig::Jets[year].MinPt) and is_good_bjet_pre_filter;

            if (is_good_jet)
            {
                jets_scale_factors.push_back(1.);
                jets_scale_factor_shift.push_back(1.);

                jets_p4.push_back(jet_p4);

                jets_is_fake.push_back(is_data ? false : Jet_genJetIdx[i] < 0);
            }

            if (is_good_bjet)
            {
                // TODO: Implement those scale factors!
                // Reference: BTagSFCorrector::BTagSFCorrector @ CorrectionSets.cpp
                bjets_scale_factors.push_back(1.);
                bjets_scale_factor_shift.push_back(1.);

                bjets_p4.push_back(jet_p4);
                bjets_is_fake.push_back(is_data ? false : Jet_genJetIdx[i] < 0);
            }
        }
    }

    return std::make_pair(
        MUSiCObjects(
            jets_p4, jets_scale_factors, jets_scale_factor_shift, jets_delta_met_x, jets_delta_met_y, jets_is_fake),
        MUSiCObjects(bjets_p4,
                     bjets_scale_factors,
                     bjets_scale_factor_shift,
                     bjets_delta_met_x,
                     bjets_delta_met_y,
                     bjets_is_fake));
}

} // namespace ObjectFactories

#endif // !MAKE_JETS_HPP