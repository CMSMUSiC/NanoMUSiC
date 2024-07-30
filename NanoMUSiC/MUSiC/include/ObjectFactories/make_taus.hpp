#ifndef MAKE_TAUS_HPP
#define MAKE_TAUS_HPP

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "Shifts.hpp"
#include "music_objects.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

namespace ObjectFactories
{

inline auto get_tau_energy_variation(const Shifts::Variations shift) -> std::string
{
    if (shift == Shifts::Variations::TauEnergy_Up)
    {
        return "up";
    }

    if (shift == Shifts::Variations::TauEnergy_Down)
    {
        return "down";
    }

    return "nom";
}

inline auto make_taus(const RVec<float> &Tau_pt,                            //
                      const RVec<float> &Tau_eta,                           //
                      const RVec<float> &Tau_phi,                           //
                      const RVec<float> &Tau_dz,                            //
                      const RVec<float> &Tau_mass,                          //
                      const RVec<UChar_t> &Tau_genPartFlav,                 //
                      const RVec<int> &Tau_genPartIdx,                      //
                      const RVec<int> &Tau_decayMode,                       //
                      const RVec<UChar_t> &Tau_idDeepTau2017v2p1VSe,        //
                      const RVec<UChar_t> &Tau_idDeepTau2017v2p1VSmu,       //
                      const RVec<UChar_t> &Tau_idDeepTau2017v2p1VSjet,      //
                      const CorrectionlibRef_t &deep_tau_2017_v2_p1_vs_e,   //
                      const CorrectionlibRef_t &deep_tau_2017_v2_p1_vs_mu,  //
                      const CorrectionlibRef_t &deep_tau_2017_v2_p1_vs_jet, //
                      const CorrectionlibRef_t &tau_energy_scale,           //
                      bool is_data,                                         //
                      const std::string &_year,                             //
                      const Shifts::Variations shift) -> MUSiCObjects
{
    auto year = get_runyear(_year);

    auto taus_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = RVec<double>{};
    auto scale_factor_shift = RVec<double>{};
    auto delta_met_x = RVec<double>{};
    auto delta_met_y = RVec<double>{};
    auto is_fake = RVec<bool>{};
    auto id_score = RVec<unsigned int>{};

    for (std::size_t i = 0; i < Tau_pt.size(); i++)
    {
        bool is_good_tau_pre_filter =
            ((Tau_idDeepTau2017v2p1VSe[i] & 64) == 64) and ((Tau_idDeepTau2017v2p1VSjet[i] & 64) == 64) and
            ((Tau_idDeepTau2017v2p1VSmu[i] & 8) == 8) and (Tau_decayMode[i] != 5) and (Tau_decayMode[i] != 6) and
            (std::fabs(Tau_eta[i]) <= 2.1) and (std::fabs(Tau_dz[i]) < 0.2);

        auto tau_p4 = Math::PtEtaPhiMVector(Tau_pt[i], Tau_eta[i], Tau_phi[i], Tau_mass[i]);

        auto energy_correction = 1.;
        if (Tau_decayMode[i] != 5 and Tau_decayMode[i] != 6)
        {
            auto variation = get_tau_energy_variation(shift);
            energy_correction = MUSiCObjects::get_scale_factor(tau_energy_scale,
                                                               is_data,
                                                               {tau_p4.pt(),
                                                                std::fabs(tau_p4.eta()),
                                                                Tau_decayMode[i],
                                                                Tau_genPartFlav[i],
                                                                "DeepTau2017v2p1",
                                                                variation});
            tau_p4 = tau_p4 * energy_correction;
        }

        if (is_good_tau_pre_filter)
        {
            // bool is_good_tau = (tau_p4.pt() >= ObjConfig::Taus[year].LowPt) and is_good_tau_pre_filter;
            bool is_good_tau = (tau_p4.pt() >= ObjConfig::Taus[year].MediumPt) and is_good_tau_pre_filter;

            if (is_good_tau)
            {

                auto sf_vs_e = MUSiCObjects::get_scale_factor(
                    deep_tau_2017_v2_p1_vs_e, is_data, {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "nom"});
                auto sf_vs_mu = MUSiCObjects::get_scale_factor(
                    deep_tau_2017_v2_p1_vs_mu, is_data, {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "nom"});
                auto sf_vs_jet = MUSiCObjects::get_scale_factor(
                    deep_tau_2017_v2_p1_vs_jet,
                    is_data,
                    {tau_p4.pt(), Tau_decayMode[i], Tau_genPartFlav[i], "Tight", "Tight", "default", "pt"});

                auto sf_vs_e_up = MUSiCObjects::get_scale_factor(
                    deep_tau_2017_v2_p1_vs_e, is_data, {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "up"});
                auto sf_vs_mu_up = MUSiCObjects::get_scale_factor(
                    deep_tau_2017_v2_p1_vs_mu, is_data, {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "up"});
                auto sf_vsjet_up = MUSiCObjects::get_scale_factor(
                    deep_tau_2017_v2_p1_vs_jet,
                    is_data,
                    {tau_p4.pt(), Tau_decayMode[i], Tau_genPartFlav[i], "Tight", "Tight", "up", "pt"});

                auto sf_vs_e_down = MUSiCObjects::get_scale_factor(
                    deep_tau_2017_v2_p1_vs_e, is_data, {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "down"});
                auto sf_vs_mu_down = MUSiCObjects::get_scale_factor(
                    deep_tau_2017_v2_p1_vs_mu, is_data, {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "down"});
                auto sf_vsjet_down = MUSiCObjects::get_scale_factor(
                    deep_tau_2017_v2_p1_vs_jet,
                    is_data,
                    {tau_p4.pt(), Tau_decayMode[i], Tau_genPartFlav[i], "Tight", "Tight", "down", "pt"});

                scale_factors.push_back(sf_vs_e * sf_vs_mu * sf_vs_jet);
                scale_factor_shift.push_back(std::sqrt(                                                              //
                    std::pow(std::max(std::fabs(sf_vs_e - sf_vs_e_up), std::fabs(sf_vs_e - sf_vs_e_down)), 2.)       //
                    + std::pow(std::max(std::fabs(sf_vs_mu - sf_vs_mu_up), std::fabs(sf_vs_mu - sf_vs_mu_down)), 2.) //
                    +
                    std::pow(std::max(std::fabs(sf_vs_jet - sf_vsjet_up), std::fabs(sf_vs_jet - sf_vsjet_down)), 2.) //
                    ));

                delta_met_x.push_back((tau_p4.pt() - Tau_pt[i]) * std::cos(Tau_phi[i]));
                delta_met_y.push_back((tau_p4.pt() - Tau_pt[i]) * std::sin(Tau_phi[i]));

                taus_p4.push_back(tau_p4);

                is_fake.push_back(is_data ? false : Tau_genPartIdx[i] < 0);

                if (tau_p4.pt() < ObjConfig::Taus[year].MediumPt)
                {
                    id_score.push_back(MUSiCObjects::IdScore::Loose);
                }
                else if (tau_p4.pt() >= ObjConfig::Taus[year].MediumPt and tau_p4.pt() < ObjConfig::Taus[year].HighPt)
                {
                    id_score.push_back(MUSiCObjects::IdScore::Medium);
                }
                else
                {
                    id_score.push_back(MUSiCObjects::IdScore::Tight);
                }
            }
        }
    }

    return MUSiCObjects(taus_p4,            //
                        scale_factors,      //
                        scale_factor_shift, //
                        delta_met_x,        //
                        delta_met_y,        //
                        is_fake,
                        id_score);
}

} // namespace ObjectFactories

#endif // !MAKE_TAUS_HPP
