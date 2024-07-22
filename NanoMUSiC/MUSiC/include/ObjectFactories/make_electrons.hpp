#ifndef MAKE_ELECTRONS_HPP
#define MAKE_ELECTRONS_HPP

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "Shifts.hpp"
#include "music_objects.hpp"
#include <cstdlib>

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

namespace ObjectFactories
{

inline auto get_electron_energy_corrections(const Shifts::Variations shift,
                                            float dEscaleUp,
                                            float dEscaleDown,
                                            float dEsigmaUp,
                                            float dEsigmaDown,
                                            double energy) -> double
{
    if (shift == Shifts::Variations::ElectronScale_Up)
    {
        return (1.f - dEscaleUp / energy);
    }

    if (shift == Shifts::Variations::ElectronScale_Down)
    {
        return (1.f - dEscaleDown / energy);
    }

    if (shift == Shifts::Variations::ElectronResolution_Up)
    {
        return (1.f - dEsigmaUp / energy);
    }

    if (shift == Shifts::Variations::ElectronResolution_Down)
    {
        return (1.f - dEsigmaDown / energy);
    }

    return 1.;
}
/////////////////////////////////////////////////////////////////////////////////////////
/// Electron  SFs, in the correctionlib JSONs, are implemented in a single key: UL-Electron-ID-SF
/// inputs: year (string), variation (string), WorkingPoint (string), eta_SC (real), pt (real)
/// Examples:
/// - year: 2016preVFP, 2016postVFP, 2017, 2018
/// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
/// - WorkingPoint: Loose, Medium, RecoAbove20, RecoBelow20, Tight, Veto, wp80iso, wp80noiso, wp90iso, wp90noiso
/// - eta: [-inf, inf)
/// - pt [10., inf)
///
/// Low pT
/// RECO: RecoAbove20
/// ID: Tight
/// ISO: No recomendations (already incorporated).
///
/// For more details see here https://twiki.cern.ch/twiki/bin/view/CMS/HEEPElectronIdentificationRun2#Scale_Factor.
/// As always, HEEP ID SF are just two numbers, one for EB and one for EE.
///
/// [0] -
/// https://indico.cern.ch/event/831669/contributions/3485543/attachments/1871797/3084930/ApprovalSlides_EE_v3.pdf

//////////////////////////////////
/// Get HEEP Id SF
/// Full 2016:
/// Barrel : 0.980 +/- 0.001 (stat) +/- 0.005 (syst)
/// Endcap: 0.989 +/- 0.004 (stat) +/- 0.007 (syst)
/// 2017:
/// Barrel :  0.979 +/- 0.001 (stat) +/- 0.005 (syst)
/// Endcap: 0.987 +/- 0.002 (stat) +/- 0.010 (syst)
/// 2018:
/// Barrel : 0.973 +/- 0.001 (stat) +/- 0.004 (syst)
/// Endcap: 0.980 +/- 0.002 (stat) +/- 0.011 (syst)
/// For more details, see:
/// https://indico.cern.ch/event/1255216/contributions/5273071/attachments/2594851/4478919/HEEP%20ID%202016UL%20for%20EGamma.pdf
/// Reference: https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaUL2016To2018
inline auto get_high_pt_sf(bool is_data, const Year &year, const std::string &variation, float pt, float eta) -> double
{
    if (is_data)
    {
        return 1.;
    }

    bool is_EB = false;
    if (std::fabs(eta) < 1.566)
    {
        is_EB = true;
    }
    else if (std::fabs(eta) <= 2.5)
    {
        is_EB = false;
    }
    else
    {
        eta = std::min(std::fabs(eta), 2.5f) * eta / eta;
    }

    auto syst_multiplier = [&variation]() -> float
    {
        if (variation == "sf")
        {
            return 0.;
        }
        if (variation == "sfup")
        {
            return 1.;
        }
        if (variation == "sfdown")
        {
            return -1.;
        }
        fmt::print(stderr, "Invalid variation parameter ({}).", variation);
        std::exit(EXIT_FAILURE);
    };

    switch (year)
    {
    case Year::Run2016APV:
        if (is_EB)
        {
            return 0.98 + syst_multiplier() * std::sqrt(std::pow(0.001, 2) + std::pow(0.005, 2));
        }
        return 0.989 + syst_multiplier() * std::sqrt(std::pow(0.004, 2) + std::pow(0.007, 2));

    case Year::Run2016:
        if (is_EB)
        {
            return 0.98 + syst_multiplier() * std::sqrt(std::pow(0.001, 2) + std::pow(0.005, 2));
        }
        return 0.989 + syst_multiplier() * std::sqrt(std::pow(0.004, 2) + std::pow(0.007, 2));

    case Year::Run2017:
        if (is_EB)
        {
            return 0.979 + syst_multiplier() * std::sqrt(std::pow(0.001, 2) + std::pow(0.005, 2));
        }
        return 0.987 + syst_multiplier() * std::sqrt(std::pow(0.002, 2) + std::pow(0.010, 2));

    case Year::Run2018:
        if (is_EB)
        {
            return 0.973 + syst_multiplier() * std::sqrt(std::pow(0.001, 2) + std::pow(0.004, 2));
        }
        return 0.980 + syst_multiplier() * std::sqrt(std::pow(0.002, 2) + std::pow(0.011, 2));

    default:
        fmt::print("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).",
                   std::to_string(year));
        std::exit(EXIT_FAILURE);
    }
}

///////////////////////////////////////////////////////////////
/// For some reason, the Official Muon SFs requires a field of the requested year, with proper formating.
inline auto get_year_for_electron_sf(Year year) -> std::string
{
    switch (year)
    {
    case Year::Run2016APV:
        return "2016preVFP"s;
    case Year::Run2016:
        return "2016postVFP"s;
    case Year::Run2017:
        return "2017"s;
    case Year::Run2018:
        return "2018"s;
    default:
        fmt::print("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).",
                   std::to_string(year));
        std::exit(EXIT_FAILURE);
    }
}

inline auto make_electrons(const RVec<float> &Electron_pt,           //
                           const RVec<float> &Electron_eta,          //
                           const RVec<float> &Electron_phi,          //
                           const RVec<float> &Electron_mass,         //
                           const RVec<float> &Electron_deltaEtaSC,
                           const RVec<Int_t> &Electron_cutBased,     //
                           const RVec<bool> &Electron_cutBased_HEEP, //
                           const RVec<float> &Electron_scEtOverPt,   //
                           const RVec<float> &Electron_dEscaleUp,    //
                           const RVec<float> &Electron_dEscaleDown,  //
                           const RVec<float> &Electron_dEsigmaUp,    //
                           const RVec<float> &Electron_dEsigmaDown,  //
                           const RVec<int> &Electron_genPartIdx,     //
                           const CorrectionlibRef_t &electron_sf,    //
                           bool is_data,                             //
                           const std::string &_year,                 //
                           const Shifts::Variations shift) -> MUSiCObjects
{
    auto year = get_runyear(_year);
    auto electrons_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = RVec<double>{};
    auto scale_factor_shift = RVec<double>{};
    auto delta_met_x = 0.;
    auto delta_met_y = 0.;
    auto is_fake = RVec<bool>{};
    auto id_score = RVec<unsigned int>{};

    for (std::size_t i = 0; i < Electron_pt.size(); i++)
    {
        auto eta_SC = Electron_eta.at(i) + Electron_deltaEtaSC.at(i);

        // Low pT Electrons
        bool is_good_low_pt_electron_pre_filter =
            ((std::fabs(eta_SC) <= 1.442) or ((std::fabs(eta_SC) >= 1.566) and (std::fabs(eta_SC) <= 2.5))) //
            and (Electron_cutBased.at(i) >= ObjConfig::Electrons[year].cutBasedId);

        // High pT Electrons
        bool is_good_high_pt_electron_pre_filter =
            ((std::fabs(eta_SC) <= 1.442) or ((std::fabs(eta_SC) >= 1.566) and (std::fabs(eta_SC) <= 2.5))) //
            and (Electron_cutBased_HEEP.at(i));

        float pt_correction_factor = 1.f;
        float eta_correction_factor = 0.f;
        if (Electron_pt.at(i) >= ObjConfig::Electrons[year].HighPt)
        {
            pt_correction_factor = 1.f;
            if (not(std::isnan(Electron_scEtOverPt[i]) or std::isinf(Electron_scEtOverPt[i])))
            {
                pt_correction_factor += Electron_scEtOverPt[i];
            }
            eta_correction_factor = Electron_deltaEtaSC[i];
        }

        // define a new lorentz vector for a electron and apply the energy correction
        auto electron_p4 =
            // Math::PtEtaPhiMVector(std::max(Electron_pt[i] * pt_correction_factor,
            // ObjConfig::Electrons[year].MinLowPt),
            Math::PtEtaPhiMVector(Electron_pt[i] * pt_correction_factor,
                                  Electron_eta[i] + eta_correction_factor,
                                  Electron_phi[i],
                                  Electron_mass[i]);
        electron_p4 = electron_p4 * get_electron_energy_corrections(shift,
                                                                    Electron_dEscaleUp[i],
                                                                    Electron_dEscaleDown[i],
                                                                    Electron_dEsigmaUp[i],
                                                                    Electron_dEsigmaDown[i],
                                                                    electron_p4.energy());

        delta_met_x += (electron_p4.pt() - Electron_pt[i]) * std::cos(Electron_phi[i]);
        delta_met_y += (electron_p4.pt() - Electron_pt[i]) * std::sin(Electron_phi[i]);

        if (is_good_low_pt_electron_pre_filter or is_good_high_pt_electron_pre_filter)
        {
            // Low pT Electrons
            bool is_good_low_pt_electron = ((electron_p4.pt() >= ObjConfig::Electrons[year].MediumPt) and
                                            (electron_p4.pt() < ObjConfig::Electrons[year].HighPt)) and
                                           is_good_low_pt_electron_pre_filter;

            // High pT Electrons
            bool is_good_high_pt_electron =
                (electron_p4.pt() >= ObjConfig::Electrons[year].HighPt) and is_good_high_pt_electron_pre_filter;

            // calculate scale factors per object (particle)
            // follow the previous MUSiC analysis, the SFs are set before the energy scale and resolution
            if (is_good_low_pt_electron)
            {
                auto sf_reco = electron_p4.pt() >= 20.
                                   ? MUSiCObjects::get_scale_factor(electron_sf,
                                                                    is_data,
                                                                    {get_year_for_electron_sf(year),
                                                                     "sf",
                                                                     "RecoAbove20",
                                                                     Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                                     electron_p4.pt()})
                                   : MUSiCObjects::get_scale_factor(electron_sf,
                                                                    is_data,
                                                                    {get_year_for_electron_sf(year),
                                                                     "sf",
                                                                     "RecoBelow20",
                                                                     Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                                     electron_p4.pt()});
                auto sf_id = MUSiCObjects::get_scale_factor(electron_sf,
                                                            is_data,
                                                            {get_year_for_electron_sf(year),
                                                             "sf",
                                                             "Tight",
                                                             Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                             electron_p4.pt()});

                auto sf_reco_up = electron_p4.pt() >= 20.
                                      ? MUSiCObjects::get_scale_factor(electron_sf,
                                                                       is_data,
                                                                       {get_year_for_electron_sf(year),
                                                                        "sfup",
                                                                        "RecoAbove20",
                                                                        Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                                        electron_p4.pt()})
                                      : MUSiCObjects::get_scale_factor(electron_sf,
                                                                       is_data,
                                                                       {get_year_for_electron_sf(year),
                                                                        "sfup",
                                                                        "RecoBelow20",
                                                                        Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                                        electron_p4.pt()});

                auto sf_id_up = MUSiCObjects::get_scale_factor(electron_sf,
                                                               is_data,
                                                               {get_year_for_electron_sf(year),
                                                                "sfup",
                                                                "Tight",
                                                                Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                                electron_p4.pt()});

                auto sf_reco_down =
                    electron_p4.pt() >= 20.
                        ? MUSiCObjects::get_scale_factor(electron_sf,
                                                         is_data,
                                                         {get_year_for_electron_sf(year),
                                                          "sfdown",
                                                          "RecoAbove20",
                                                          Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                          electron_p4.pt()})
                        : MUSiCObjects::get_scale_factor(electron_sf,
                                                         is_data,
                                                         {get_year_for_electron_sf(year),
                                                          "sfdown",
                                                          "RecoBelow20",
                                                          Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                          electron_p4.pt()});

                auto sf_id_down = MUSiCObjects::get_scale_factor(electron_sf,
                                                                 is_data,
                                                                 {get_year_for_electron_sf(year),
                                                                  "sfdown",
                                                                  "Tight",
                                                                  Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                                  electron_p4.pt()});

                scale_factors.push_back(sf_reco * sf_id);
                scale_factor_shift.push_back(std::sqrt(                                                        //
                    std::pow(std::max(std::fabs(sf_reco - sf_reco_up), std::fabs(sf_reco - sf_reco_down)), 2.) //
                    + std::pow(std::max(std::fabs(sf_id - sf_id_up), std::fabs(sf_id - sf_id_down)), 2.)       //
                    ));
            }

            if (is_good_high_pt_electron)
            {
                auto sf_reco = MUSiCObjects::get_scale_factor(
                    electron_sf,
                    is_data,
                    {get_year_for_electron_sf(year), "sf", "RecoAbove20", Electron_eta.at(i), electron_p4.pt()});
                auto sf_id = get_high_pt_sf(is_data, year, "sf", electron_p4.pt(), electron_p4.eta());

                auto sf_reco_up = MUSiCObjects::get_scale_factor(
                    electron_sf,
                    is_data,
                    {get_year_for_electron_sf(year), "sfup", "RecoAbove20", Electron_eta.at(i), electron_p4.pt()});
                auto sf_id_up = get_high_pt_sf(is_data, year, "sfup", electron_p4.pt(), electron_p4.eta());

                auto sf_reco_down = MUSiCObjects::get_scale_factor(
                    electron_sf,
                    is_data,
                    {get_year_for_electron_sf(year), "sfdown", "RecoAbove20", Electron_eta.at(i), electron_p4.pt()});
                auto sf_id_down = get_high_pt_sf(is_data, year, "sfdown", electron_p4.pt(), electron_p4.eta());

                scale_factors.push_back(sf_reco * sf_id);
                scale_factor_shift.push_back(std::sqrt(                                                        //
                    std::pow(std::max(std::fabs(sf_reco - sf_reco_up), std::fabs(sf_reco - sf_reco_down)), 2.) //
                    + std::pow(std::max(std::fabs(sf_id - sf_id_up), std::fabs(sf_id - sf_id_down)), 2.)       //
                    ));
            }

            if (is_good_low_pt_electron or is_good_high_pt_electron)
            {
                electrons_p4.push_back(electron_p4);
                is_fake.push_back(is_data ? false : Electron_genPartIdx[i] < 0);
                if (is_good_low_pt_electron)
                {
                    if (electron_p4.pt() < ObjConfig::Electrons[year].MediumPt)
                    {
                        id_score.push_back(MUSiCObjects::IdScore::Loose);
                    }
                    else
                    {
                        id_score.push_back(MUSiCObjects::IdScore::Medium);
                    }
                }
                if (is_good_high_pt_electron)
                {
                    id_score.push_back(MUSiCObjects::IdScore::Tight);
                }
            }
        }
    }

    return MUSiCObjects(electrons_p4,       //
                        scale_factors,      //
                        scale_factor_shift, //
                        delta_met_x,        //
                        delta_met_y,        //
                        is_fake,
                        id_score);
}

} // namespace ObjectFactories

#endif // !MAKE_ELECTRONS_HPP
