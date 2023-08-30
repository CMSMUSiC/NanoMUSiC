#ifndef MAKE_ELECTRONS_HPP
#define MAKE_ELECTRONS_HPP

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

inline auto get_electron_energy_corrections(const std::string &shift,
                                            float dEscaleUp,
                                            float dEscaleDown,
                                            float dEsigmaUp,
                                            float dEsigmaDown,
                                            double energy) -> double
{
    if (shift == "ElectronScale_Up")
    {
        return (1.f - dEscaleUp / energy);
    }

    if (shift == "ElectronScale_Down")
    {
        return (1.f - dEscaleDown / energy);
    }

    if (shift == "ElectronResolution_Up")
    {
        return (1.f - dEsigmaUp / energy);
    }

    if (shift == "ElectronResolution_Down")
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
/// TODO: High Pt - Doesn't use the Correctionlib
/// RECO: Same as Low Pt.
/// ID: Example:
/// https://github.com/CMSLQ/rootNtupleAnalyzerV2/blob/2dd8f9415e7a9c3465c7e28916eb68866ff337ff/src/ElectronScaleFactors.C
/// 2016 prompt: 0.971±0.001 (stat) (EB), 0.983±0.001 (stat) (EE)
///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
///              uncertainty (syst?): EE ET < 90 GeV: 1% else min(1+(ET-90)*0.0143)%,4%)
///
/// 2016 legacy: 0.983±0.000 (stat) (EB), 0.991±0.001 (stat) (EE) (taken from slide 10 of [0])
///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
///              uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)
///
/// 2017 prompt: 0.968±0.001 (stat) (EB), 0.973±0.002 (stat) (EE)
///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
///              uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)
///
/// 2018 rereco (Autumn 18): 0.969 +/- 0.000 (stat) (EB), and 0.984 +/- 0.001 (stat) (EE).
///                          uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
///                          uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)

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
        // throw std::runtime_error(fmt::format("Eta SC value ({}) is out of range.", eta));
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
        throw std::runtime_error(fmt::format("Invalid variation parameter ({}).", variation));
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
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
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
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

inline auto make_electrons(const RVec<float> &Electron_pt,  //
                           const RVec<float> &Electron_eta, //
                           const RVec<float> &Electron_phi, //
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
                           const std::string &shift) -> MUSiCObjects
{
    auto year = get_runyear(_year);
    auto electrons_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = RVec<double>{};
    auto scale_factor_up = RVec<double>{};
    auto scale_factor_down = RVec<double>{};
    auto delta_met_x = 0.;
    auto delta_met_y = 0.;
    auto is_fake = RVec<bool>{};

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
        if (Electron_pt.at(i) >= ObjConfig::Electrons[year].MaxLowPt)
        {
            pt_correction_factor = Electron_scEtOverPt[i] + 1.f;
            eta_correction_factor = Electron_deltaEtaSC[i];
        }

        // define a new lorentz vector for a electron and apply the energy correction
        auto electron_p4 =
            // Math::PtEtaPhiMVector(std::max(Electron_pt[i] * pt_correction_factor,
            // ObjConfig::Electrons[year].MinLowPt),
            Math::PtEtaPhiMVector(Electron_pt[i] * pt_correction_factor,
                                  Electron_eta[i] + eta_correction_factor,
                                  Electron_phi[i],
                                  PDG::Electron::Mass);
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
            bool is_good_low_pt_electron = ((electron_p4.pt() >= ObjConfig::Electrons[year].MinLowPt) and
                                            (electron_p4.pt() < ObjConfig::Electrons[year].MaxLowPt)) and
                                           is_good_low_pt_electron_pre_filter;

            // High pT Electrons
            bool is_good_high_pt_electron =
                (electron_p4.pt() >= ObjConfig::Electrons[year].MaxLowPt) and is_good_high_pt_electron_pre_filter;

            // calculate scale factors per object (particle)
            // follow the previous MUSiC analysis, the SFs are set before the energy scale and resolution
            if (is_good_low_pt_electron)
            {
                scale_factors.push_back( //
                    MUSiCObjects::get_scale_factor(electron_sf,
                                                   is_data,
                                                   {get_year_for_electron_sf(year),
                                                    "sf",
                                                    "RecoAbove20",
                                                    Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                    electron_p4.pt()}) *
                    MUSiCObjects::get_scale_factor(electron_sf,
                                                   is_data,
                                                   {get_year_for_electron_sf(year),
                                                    "sf",
                                                    "Tight",
                                                    Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                    electron_p4.pt()}));

                scale_factor_up.push_back( //
                    MUSiCObjects::get_scale_factor(electron_sf,
                                                   is_data,
                                                   {get_year_for_electron_sf(year),
                                                    "sfup",
                                                    "RecoAbove20",
                                                    Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                    electron_p4.pt()}) *
                    MUSiCObjects::get_scale_factor(electron_sf,
                                                   is_data,
                                                   {get_year_for_electron_sf(year),
                                                    "sfup",
                                                    "Tight",
                                                    Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                    electron_p4.pt()}));

                scale_factor_down.push_back( //
                    MUSiCObjects::get_scale_factor(electron_sf,
                                                   is_data,
                                                   {get_year_for_electron_sf(year),
                                                    "sfdown",
                                                    "RecoAbove20",
                                                    Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                    electron_p4.pt()}) *
                    MUSiCObjects::get_scale_factor(electron_sf,
                                                   is_data,
                                                   {get_year_for_electron_sf(year),
                                                    "sfdown",
                                                    "Tight",
                                                    Electron_eta.at(i) + Electron_deltaEtaSC.at(i),
                                                    electron_p4.pt()}));
            }

            if (is_good_high_pt_electron)
            {
                scale_factors.push_back( //
                    MUSiCObjects::get_scale_factor(
                        electron_sf,
                        is_data,
                        {get_year_for_electron_sf(year), "sf", "RecoAbove20", Electron_eta.at(i), electron_p4.pt()}) *
                    get_high_pt_sf(is_data, year, "sf", electron_p4.pt(), electron_p4.eta()));

                scale_factor_up.push_back( //
                    MUSiCObjects::get_scale_factor(
                        electron_sf,
                        is_data,
                        {get_year_for_electron_sf(year), "sfup", "RecoAbove20", Electron_eta.at(i), electron_p4.pt()}) *
                    get_high_pt_sf(is_data, year, "sfup", electron_p4.pt(), electron_p4.eta()));

                scale_factor_down.push_back( //
                    MUSiCObjects::get_scale_factor(electron_sf,
                                                   is_data,
                                                   {get_year_for_electron_sf(year),
                                                    "sfdown",
                                                    "RecoAbove20",
                                                    Electron_eta.at(i),
                                                    electron_p4.pt()}) *
                    get_high_pt_sf(is_data, year, "sfdown", electron_p4.pt(), electron_p4.eta()));
            }

            if (is_good_low_pt_electron or is_good_high_pt_electron)
            {
                electrons_p4.push_back(electron_p4);

                is_fake.push_back(is_data ? false : Electron_genPartIdx[i] == -1);
            }
        }
    }

    return MUSiCObjects(electrons_p4,      //
                        scale_factors,     //
                        scale_factor_up,   //
                        scale_factor_down, //
                        delta_met_x,       //
                        delta_met_y,       //
                        is_fake);
}

} // namespace ObjectFactories

#endif // !MAKE_ELECTRONS_HPP
