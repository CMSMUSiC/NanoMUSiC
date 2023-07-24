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
                                       float dEscaleUp,
                                       float dEscaleDown,
                                       float dEsigmaUp,
                                       float dEsigmaDown,
                                       double energy) -> double
{
    if (shift == "Tau_Up")
    {
        return 1.;
    }

    if (shift == "Tau_Down")
    {
        return 1.;
    }

    return 1.;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Photons ID SFs, in the correctionlib JSONs, are implemented in: UL-Photon-ID-SF
/// inputs: year (string), variation (string), WorkingPoint (string), eta_SC (real), pt (real)
/// - year: 2016preVFP, 2016postVFP, 2017, 2018
/// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
/// - WorkingPoint: Loose, Medium, Tight, wp80, wp90
/// - eta: [-inf, inf)
/// - pt [20., inf)
///
/// Low pT
/// RECO: From Twiki [0]: "The scale factor to reconstruct a supercluster with H/E<0.5 is assumed to be 100%."
/// ID: Tight
/// ISO: No recomendations (already incorporated).
///
/// [0] - https://twiki.cern.ch/twiki/bin/view/CMS/EgammaRunIIRecommendations#E_gamma_RECO
///
/// Photons PixelSeed SFs, in the correctionlib JSONs, are implemented in:  UL-Photon-PixVeto-SF
/// These are the Photon Pixel Veto Scale Factors (nominal, up or down) for 2018 Ultra Legacy dataset.
/// - year: 2016preVFP, 2016postVFP, 2017, 2018
/// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
/// - WorkingPoint (SFs available for the cut-based and MVA IDs): Loose, MVA, Medium, Tight
/// - HasPixBin: For each working point of choice, they are dependent on the photon pseudorapidity and R9: Possible
/// bin choices: ['EBInc','EBHighR9','EBLowR9','EEInc','EEHighR9','EELowR9']
///////////////////////////////////////////////////////////////
/// For some reason, the Official Muon SFs requires a field of the requested year, with proper formating.

// inline auto get_year_for_tau_sf(Year year) -> std::string
// {
//     switch (year)
//     {
//     case Year::Run2016APV:
//         return "2016preVFP"s;
//     case Year::Run2016:
//         return "2016postVFP"s;
//     case Year::Run2017:
//         return "2017"s;
//     case Year::Run2018:
//         return "2018"s;
//     default:
//         throw std::runtime_error("Year (" + std::to_string(year) +
//                                  ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
//     }
// }

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
    // auto year = get_runyear(_year);

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

        if (is_good_tau_pre_filter)
        {
            auto tau_p4 = Math::PtEtaPhiMVector(
                Tau_pt[i],
                Tau_eta[i],
                Tau_phi[i],
                Tau_mass[i]); // regard Tau_mass right here ("PDG::Tau::Mass" is only literature value)
            tau_p4 = tau_p4 * MUSiCObjects::get_scale_factor(tau_energy_scale,
                                                             is_data,
                                                             {tau_p4.pt(),
                                                              std::fabs(tau_p4.eta()),
                                                              Tau_decayMode[i],
                                                              Tau_genPartFlav[i],
                                                              "DeepTau2017v2p1",
                                                              "nom"});

            bool is_good_tau =
                (tau_p4.pt() >= 20.) and is_good_tau_pre_filter; // 25 our studies 20 official recommondation

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
                        is_data,
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
                        is_data,
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
                        is_data,
                        {tau_p4.pt(), Tau_decayMode[i], Tau_genPartFlav[i], "Tight", "Tight", "down", "pt"}));

                taus_p4.push_back(tau_p4);

                delta_met_x += (tau_p4.pt() - Tau_pt[i]) * std::cos(Tau_phi[i]);
                delta_met_y += (tau_p4.pt() - Tau_pt[i]) * std::sin(Tau_phi[i]);

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