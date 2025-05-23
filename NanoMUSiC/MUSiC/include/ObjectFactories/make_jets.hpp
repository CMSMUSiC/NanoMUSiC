#ifndef MAKE_JETS_HPP
#define MAKE_JETS_HPP

// ROOT Stuff
#include "Math/Vector4Dfwd.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "JetCorrector.hpp"
#include "Shifts.hpp"
#include "music_objects.hpp"

#include "NanoAODGenInfo.hpp"
#include <algorithm>
#include <cmath>

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
                                       const NanoAODGenInfo::GenJets &gen_jets) -> double
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

inline auto make_jets(const RVec<float> &Jet_pt,               //
                      const RVec<float> &Jet_eta,              //
                      const RVec<float> &Jet_phi,              //
                      const RVec<float> &Jet_mass,             //
                      const RVec<Int_t> &Jet_jetId,            //
                      const RVec<float> &Jet_btagDeepFlavB,    //
                      const RVec<float> &Jet_rawFactor,        //
                      const RVec<float> &Jet_area,             //
                      const RVec<float> &Jet_chEmEF,           //
                      const RVec<Int_t> &Jet_puId,             //
                      const RVec<float> &Muon_eta,             //
                      const RVec<float> &Muon_phi,             //
                      const RVec<bool> &Muon_isPFcand,         //
                      const RVec<Int_t> &Jet_genJetIdx,        //
                      float fixedGridRhoFastjetAll,            //
                      JetCorrector &jet_corrections,           //
                      const CorrectionlibRef_t &btag_sf,       //
                      const NanoAODGenInfo::GenJets &gen_jets, //
                      const CorrectionlibRef_t &jet_veto_map,  //
                      bool is_data,                            //
                      const std::string &_year,                //
                      const Shifts::Variations shift)
    -> std::tuple<MUSiCObjects, MUSiCObjects, bool, RVec<int>, RVec<int>>
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
    auto jets_delta_met_x = RVec<double>{};
    auto bjets_delta_met_x = RVec<double>{};
    auto jets_delta_met_y = RVec<double>{};
    auto bjets_delta_met_y = RVec<double>{};
    auto jets_is_fake = RVec<bool>{};
    auto bjets_is_fake = RVec<bool>{};
    auto jets_id_score = RVec<unsigned int>{};
    auto bjets_id_score = RVec<unsigned int>{};

    bool has_vetoed_jets = false;
    auto selected_jet_indexes = RVec<int>{};
    auto selected_bjet_indexes = RVec<int>{};
    for (std::size_t i = 0; i < Jet_pt.size(); i++)
    {
        // check for vetoed jets
        // apply recemmended loose selection
        // https://cms-jerc.web.cern.ch/Recommendations/#jet-veto-maps
        // https://github.com/columnflow/columnflow/blob/1161b414ba536e93a9f532e8c6078e0b92b2a87e/columnflow/selection/cms/jets.py#L108
        if (shift == Shifts::Variations::Nominal)
        {
            if (Jet_pt[i] > 15. and Jet_jetId[i] >= 2 and Jet_chEmEF[i] < 0.9 and
                (Jet_puId[i] >= 4 or Jet_pt[i] >= 50.))
            {
                bool has_muon_overlap = false;
                for (std::size_t muon_idx = 0; muon_idx < Muon_eta.size(); muon_idx++)
                {
                    if (Muon_isPFcand[i])
                    {
                        if (ROOT::VecOps::DeltaR(Jet_eta[i], Muon_eta[muon_idx], Jet_phi[i], Muon_phi[muon_idx]) < 0.2)
                        {
                            has_muon_overlap = true;
                            break;
                        }
                    }
                }

                if (not(has_muon_overlap))
                {
                    if (jet_veto_map->evaluate({"jetvetomap",
                                                std::max(-5.1905f, std::min(Jet_eta[i], 5.1905f)),
                                                std::max(-3.14158f, std::min(Jet_phi[i], 3.14158f))}) != 0.)
                    {
                        has_vetoed_jets = true;
                        break;
                    }
                }
            }
        }

        auto is_good_jet_pre_filter =
            (std::fabs(Jet_eta[i]) <= ObjConfig::Jets[year].MaxAbsEta) //
            and (Jet_jetId[i] >= ObjConfig::Jets[year].MinJetID)       //
            and (0 <= Jet_btagDeepFlavB[i] and Jet_btagDeepFlavB[i] < ObjConfig::Jets[year].MaxBTagWPTight);

        auto is_good_bjet_pre_filter =
            (std::fabs(Jet_eta[i]) <= ObjConfig::Jets[year].MaxAbsEta) //
            and (Jet_jetId[i] >= ObjConfig::Jets[year].MinJetID)       //
            and (ObjConfig::Jets[year].MaxBTagWPTight <= Jet_btagDeepFlavB[i] and Jet_btagDeepFlavB[i] <= 1);

        auto jet_p4 = Math::PtEtaPhiMVector(Jet_pt[i], Jet_eta[i], Jet_phi[i], Jet_mass[i]);

        // first we accumulate the correction that was already aplied
        const double jets_delta_met_x_type_1 =
            (jet_p4.pt() - Jet_pt[i] * (1. - Jet_rawFactor[i])) * std::cos(Jet_phi[i]);
        const double jets_delta_met_y_type_1 =
            (jet_p4.pt() - Jet_pt[i] * (1. - Jet_rawFactor[i])) * std::sin(Jet_phi[i]);

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
        }

        if (is_good_jet_pre_filter or is_good_bjet_pre_filter)
        {
            auto is_good_jet = (jet_p4.pt() >= ObjConfig::Jets[year].MediumPt) and is_good_jet_pre_filter;
            auto is_good_bjet = (jet_p4.pt() >= ObjConfig::Jets[year].MediumPt) and is_good_bjet_pre_filter;

            if (is_good_jet)
            {
                jets_scale_factors.push_back(1.);
                jets_scale_factor_shift.push_back(0.);

                // then we accumulate the new correction
                jets_delta_met_x.push_back((jet_p4.pt() - Jet_pt[i]) * std::cos(Jet_phi[i]) - jets_delta_met_x_type_1);
                jets_delta_met_y.push_back((jet_p4.pt() - Jet_pt[i]) * std::sin(Jet_phi[i]) - jets_delta_met_y_type_1);

                jets_p4.push_back(jet_p4);
                jets_is_fake.push_back(is_data ? false : Jet_genJetIdx[i] < 0);
                if (jet_p4.pt() < ObjConfig::Jets[year].HighPt)
                {
                    jets_id_score.push_back(MUSiCObjects::IdScore::Medium);
                }
                else
                {
                    jets_id_score.push_back(MUSiCObjects::IdScore::Tight);
                }

                selected_jet_indexes.push_back(i);
            }

            if (is_good_bjet)
            {
                // TODO: Implement those scale factors!
                // Reference: BTagSFCorrector::BTagSFCorrector @ CorrectionSets.cpp
                bjets_scale_factors.push_back(1.);
                bjets_scale_factor_shift.push_back(0.);

                // then we accumulate the new correction
                bjets_delta_met_x.push_back((jet_p4.pt() - Jet_pt[i]) * std::cos(Jet_phi[i]) - jets_delta_met_x_type_1);
                bjets_delta_met_y.push_back((jet_p4.pt() - Jet_pt[i]) * std::sin(Jet_phi[i]) - jets_delta_met_y_type_1);

                bjets_p4.push_back(jet_p4);
                bjets_is_fake.push_back(is_data ? false : Jet_genJetIdx[i] < 0);
                if (jet_p4.pt() < ObjConfig::Jets[year].HighPt)
                {
                    bjets_id_score.push_back(MUSiCObjects::IdScore::Medium);
                }
                else
                {
                    bjets_id_score.push_back(MUSiCObjects::IdScore::Tight);
                }

                selected_bjet_indexes.push_back(i);
            }
        }
    }

    return {MUSiCObjects(jets_p4,
                         jets_scale_factors,
                         jets_scale_factor_shift,
                         jets_delta_met_x,
                         jets_delta_met_y,
                         jets_is_fake,
                         jets_id_score),
            MUSiCObjects(bjets_p4,
                         bjets_scale_factors,
                         bjets_scale_factor_shift,
                         bjets_delta_met_x,
                         bjets_delta_met_y,
                         bjets_is_fake,
                         bjets_id_score),
            has_vetoed_jets,
            selected_jet_indexes,
            selected_bjet_indexes};
}

} // namespace ObjectFactories

#endif // !MAKE_JETS_HPP
