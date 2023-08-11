#ifndef MAKE_TAUS_HPP
#define MAKE_TAUS_HPP

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

inline auto get_tau_energy_corrections(const std::string &shift,
                                       const CorrectionlibRef_t &tau_energy_scale,
                                       float tau_pt,
                                       float tau_eta,
                                       int tau_decayMode,
                                       int tau_genPartFlav) -> double
{
    if (shift == "Tau_Up")
    {
        return tau_energy_scale->evaluate({tau_pt, tau_eta, tau_decayMode, tau_genPartFlav, "DeepTau2017v2p1", "up"});
    }

    if (shift == "Tau_Down")
    {
        return tau_energy_scale->evaluate({tau_pt, tau_eta, tau_decayMode, tau_genPartFlav, "DeepTau2017v2p1", "down"});
    }

    return tau_energy_scale->evaluate({tau_pt, tau_eta, tau_decayMode, tau_genPartFlav, "DeepTau2017v2p1", "nom"});
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
                      const std::string &shift) -> MUSiCObjects
{

    auto taus_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = RVec<double>{};
    auto scale_factor_up = RVec<double>{};
    auto scale_factor_down = RVec<double>{};
    auto delta_met_x = 0.;
    auto delta_met_y = 0.;
    auto is_fake = RVec<bool>{};

    for (std::size_t i = 0; i < Tau_pt.size(); i++)
    {
        bool is_good_tau_pre_filter =
            ((Tau_idDeepTau2017v2p1VSe[i] & 32) == 32) and ((Tau_idDeepTau2017v2p1VSjet[i] & 32) == 32) and
            ((Tau_idDeepTau2017v2p1VSmu[i] & 8) == 8) and (Tau_decayMode[i] != 5) and (Tau_decayMode[i] != 6) and
            (std::fabs(Tau_eta[i]) <= 2.1) and (std::fabs(Tau_dz[i]) < 0.2);

        auto tau_p4 = Math::PtEtaPhiMVector(
            Tau_pt[i],
            Tau_eta[i],
            Tau_phi[i],
            Tau_mass[i]); // regard Tau_mass right here ("PDG::Tau::Mass" is only literature value)

        auto energy_correction = 1.;
        if (Tau_decayMode[i] != 5 and Tau_decayMode[i] != 6)
        {
            energy_correction = get_tau_energy_corrections(shift,
                tau_energy_scale,
                tau_p4.pt(), std::fabs(tau_p4.eta()), Tau_decayMode[i], Tau_genPartFlav[i]);
            tau_p4 = tau_p4 * energy_correction;
        }

        delta_met_x += (tau_p4.pt() - Tau_pt[i]) * std::cos(Tau_phi[i]);
        delta_met_y += (tau_p4.pt() - Tau_pt[i]) * std::sin(Tau_phi[i]);

        if (is_good_tau_pre_filter)
        {

            bool is_good_tau =
                (tau_p4.pt() >= 25.) and is_good_tau_pre_filter; // 25 our studies 20 official recommondation

            if (is_good_tau)
            {

                // fmt::print("\n Mass of Taus: {}\n", Tau_mass[i]);
                // fmt::print("\n Reference Mass of Taus: {}\n", PDG::Tau::Mass);
                // fmt::print("\n Tau_genPartFlav is: {}\n", Tau_genPartFlav[i]);
                // fmt::print("\n Tau_decayMode is: {}\n", Tau_decayMode[i]);
                // fmt::print("\n Tau_genPartIdx is: {}\n", Tau_genPartIdx[i]);

                scale_factors.push_back(
                    MUSiCObjects::get_scale_factor(deep_tau_2017_v2_p1_vs_e,
                                                   is_data,
                                                   {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "nom"}) *
                    MUSiCObjects::get_scale_factor(deep_tau_2017_v2_p1_vs_mu,
                                                   is_data,
                                                   {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "nom"}) *
                    MUSiCObjects::get_scale_factor(
                        deep_tau_2017_v2_p1_vs_jet,
                        is_data,                                            // WP of VSJet here considered 
                        {tau_p4.pt(), Tau_decayMode[i], Tau_genPartFlav[i], "Tight", "Tight", "default", "pt"}));

                scale_factor_up.push_back(
                    MUSiCObjects::get_scale_factor(deep_tau_2017_v2_p1_vs_e,
                                                   is_data,
                                                   {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "up"}) *
                    MUSiCObjects::get_scale_factor(deep_tau_2017_v2_p1_vs_mu,
                                                   is_data,
                                                   {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "up"}) *
                    MUSiCObjects::get_scale_factor(
                        deep_tau_2017_v2_p1_vs_jet,
                        is_data,                                            // WP of VSJet here considered
                        {tau_p4.pt(), Tau_decayMode[i], Tau_genPartFlav[i], "Tight", "Tight", "up", "pt"}));

                scale_factor_down.push_back(
                    MUSiCObjects::get_scale_factor(deep_tau_2017_v2_p1_vs_e,
                                                   is_data,
                                                   {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "down"}) *
                    MUSiCObjects::get_scale_factor(deep_tau_2017_v2_p1_vs_mu,
                                                   is_data,
                                                   {std::fabs(tau_p4.eta()), Tau_genPartFlav[i], "Tight", "down"}) *
                    MUSiCObjects::get_scale_factor(
                        deep_tau_2017_v2_p1_vs_jet,
                        is_data,                                            // WP of VSJet here considered
                        {tau_p4.pt(), Tau_decayMode[i], Tau_genPartFlav[i], "Tight", "Tight", "down", "pt"}));

                taus_p4.push_back(tau_p4);

                is_fake.push_back(is_data ? false : Tau_genPartIdx[i] < 0);
            }
        }
    }

    return MUSiCObjects(taus_p4,           //
                        scale_factors,     //
                        scale_factor_up,   //
                        scale_factor_down, //
                        delta_met_x,       //
                        delta_met_y,       //
                        is_fake);
}

} // namespace ObjectFactories

#endif // !MAKE_TAUS_HPP