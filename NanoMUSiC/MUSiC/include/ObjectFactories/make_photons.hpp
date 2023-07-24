#ifndef MAKE_PHOTONS_HPP
#define MAKE_PHOTONS_HPP

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

inline auto get_photon_energy_corrections(const std::string &shift,
                                          float dEscaleUp,
                                          float dEscaleDown,
                                          float dEsigmaUp,
                                          float dEsigmaDown,
                                          double energy) -> double
{
    if (shift == "PhotonScale_Up")
    {
        return (1.f - dEscaleUp / energy);
    }

    if (shift == "PhotonScale_Down")
    {
        return (1.f - dEscaleDown / energy);
    }

    if (shift == "PhotonResolution_Up")
    {
        return (1.f - dEsigmaUp / energy);
    }

    if (shift == "PhotonResolution_Up")
    {
        return (1.f - dEsigmaDown / energy);
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
inline auto get_year_for_photon_sf(Year year) -> std::string
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

inline auto make_photons(const RVec<float> &Photon_pt,  //
                         const RVec<float> &Photon_eta, //
                         const RVec<float> &Photon_phi, //
                         const RVec<bool> &Photon_isScEtaEB,
                         const RVec<bool> &Photon_isScEtaEE,
                         const RVec<Int_t> &Photon_cutBased,             //
                         const RVec<bool> &Photon_pixelSeed,             //
                         const RVec<float> &Photon_dEscaleUp,            //
                         const RVec<float> &Photon_dEscaleDown,          //
                         const RVec<float> &Photon_dEsigmaUp,            //
                         const RVec<float> &Photon_dEsigmaDown,          //
                         const RVec<int> &Photon_genPartIdx,             //
                         const CorrectionlibRef_t &photon_sf,            //
                         const CorrectionlibRef_t &photon_pixel_veto_sf, //
                         bool is_data,                                   //
                         const std::string &_year,                       //
                         const std::string &shift) -> MUSiCObjects
{
    auto year = get_runyear(_year);

    auto photons_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = RVec<double>{};
    auto scale_factor_up = RVec<double>{};
    auto scale_factor_down = RVec<double>{};
    auto delta_met_x = 0.;
    auto delta_met_y = 0.;
    auto is_fake = RVec<bool>{};

    for (std::size_t i = 0; i < Photon_pt.size(); i++)
    {
        bool is_good_photon_pre_filter = (Photon_isScEtaEB[i])         //
                                         and (not Photon_isScEtaEE[i]) // only EB photons
                                         and (Photon_cutBased[i] >= ObjConfig::Photons[year].cutBasedId) //
                                         and (Photon_pixelSeed[i] == false);

        if (is_good_photon_pre_filter)
        {
            auto photon_p4 = Math::PtEtaPhiMVector(Photon_pt[i], Photon_eta[i], Photon_phi[i], PDG::Photon::Mass);
            photon_p4 = photon_p4 * get_photon_energy_corrections(shift,
                                                                  Photon_dEscaleUp[i],
                                                                  Photon_dEscaleDown[i],
                                                                  Photon_dEsigmaUp[i],
                                                                  Photon_dEsigmaDown[i],
                                                                  photon_p4.energy());

            bool is_good_photon = (photon_p4.pt() >= ObjConfig::Photons[year].MinPt) and is_good_photon_pre_filter;

            if (is_good_photon)
            {

                scale_factors.push_back( //
                    MUSiCObjects::get_scale_factor(
                        photon_sf,
                        is_data,
                        {get_year_for_photon_sf(year), "sf", "Tight", photon_p4.eta(), photon_p4.pt()}) *
                    MUSiCObjects::get_scale_factor(
                        photon_pixel_veto_sf, is_data, {get_year_for_photon_sf(year), "sf", "Tight", "EBInc"}));

                scale_factor_up.push_back( //
                    MUSiCObjects::get_scale_factor(
                        photon_sf,
                        is_data,
                        {get_year_for_photon_sf(year), "sfup", "Tight", photon_p4.eta(), photon_p4.pt()}) *
                    MUSiCObjects::get_scale_factor(
                        photon_pixel_veto_sf, is_data, {get_year_for_photon_sf(year), "sfup", "Tight", "EBInc"}));

                scale_factor_down.push_back( //
                    MUSiCObjects::get_scale_factor(
                        photon_sf,
                        is_data,
                        {get_year_for_photon_sf(year), "sfdown", "Tight", photon_p4.eta(), photon_p4.pt()}) *
                    MUSiCObjects::get_scale_factor(
                        photon_pixel_veto_sf, is_data, {get_year_for_photon_sf(year), "sfdown", "Tight", "EBInc"}));

                photons_p4.push_back(photon_p4);

                delta_met_x += (photon_p4.pt() - Photon_pt[i]) * std::cos(Photon_phi[i]);
                delta_met_y += (photon_p4.pt() - Photon_pt[i]) * std::sin(Photon_phi[i]);

                is_fake.push_back(is_data ? false : Photon_genPartIdx[i] == -1);
            }
        }
    }

    return MUSiCObjects(photons_p4,        //
                        scale_factors,     //
                        scale_factor_up,   //
                        scale_factor_down, //
                        delta_met_x,       //
                        delta_met_y,       //
                        is_fake);
}

} // namespace ObjectFactories

#endif // !MAKE_PHOTONS_HPP
