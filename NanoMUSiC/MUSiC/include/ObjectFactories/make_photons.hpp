#ifndef MAKE_PHOTONS_HPP
#define MAKE_PHOTONS_HPP

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

inline auto get_photon_energy_corrections(const Shifts::Variations shift,
                                          float dEscaleUp,
                                          float dEscaleDown,
                                          float dEsigmaUp,
                                          float dEsigmaDown,
                                          double energy) -> double
{
    if (shift == Shifts::Variations::PhotonDiffScale_Up)
    {
        return (1.f - dEscaleUp / energy);
    }

    if (shift == Shifts::Variations::PhotonDiffScale_Down)
    {
        return (1.f - dEscaleDown / energy);
    }

    if (shift == Shifts::Variations::PhotonDiffResolution_Up)
    {
        return (1.f - dEsigmaUp / energy);
    }

    if (shift == Shifts::Variations::PhotonDiffResolution_Down)
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

inline auto make_photons(const RVec<float> &Photon_pt,             //
                         const RVec<float> &Photon_eta,            //
                         const RVec<float> &Photon_phi,            //
                         const RVec<float> &Photon_mass,           //
                         const RVec<bool> &Photon_isScEtaEB,       //
                         const RVec<bool> &Photon_isScEtaEE,       //
                         const RVec<Int_t> &Photon_mvaID_WP90,     //
                         const RVec<bool> &Photon_electronVeto,    //
                         const RVec<float> &Photon_dEscaleUp,      //
                         const RVec<float> &Photon_dEscaleDown,    //
                         const RVec<float> &Photon_dEsigmaUp,      //
                         const RVec<float> &Photon_dEsigmaDown,    //
                         const RVec<int> &Photon_genPartIdx,       //
                         const RVec<int> &Photon_electronIdx,      //
                         const RVec<int> &Photon_jetIdx,           //
                         const CorrectionlibRef_t &photon_sf,      //
                         const CorrectionlibRef_t &photon_csev_sf, //
                         bool is_data,                             //
                         const std::string &_year,                 //
                         const Shifts::Variations shift) -> MUSiCObjects
{
    auto year = get_runyear(_year);

    auto photons_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = std::unordered_map<Shifts::Variations, RVec<double>>{};
    auto delta_met_x = RVec<double>{};
    auto delta_met_y = RVec<double>{};
    auto is_fake = RVec<bool>{};

    for (std::size_t i = 0; i < Photon_pt.size(); i++)
    {
        bool is_good_photon_pre_filter = (Photon_isScEtaEB[i])           //
                                         and (not(Photon_isScEtaEE[i]))  // only EB photons
                                         and (Photon_mvaID_WP90[i])      //
                                         and (Photon_electronVeto[i])    //
                                         and (Photon_electronIdx[i] < 0) //
                                                                         // and (Photon_jetIdx[i] < 0)
            ;

        auto photon_p4 = Math::PtEtaPhiMVector(Photon_pt[i], Photon_eta[i], Photon_phi[i], Photon_mass[i]);
        photon_p4 = photon_p4 * get_photon_energy_corrections(shift,
                                                              Photon_dEscaleUp[i],
                                                              Photon_dEscaleDown[i],
                                                              Photon_dEsigmaUp[i],
                                                              Photon_dEsigmaDown[i],
                                                              photon_p4.energy());

        if (is_good_photon_pre_filter)
        {
            bool is_good_photon = (photon_p4.pt() >= ObjConfig::Photons[year].MediumPt) and is_good_photon_pre_filter;

            if (is_good_photon)
            {
                auto sf_id = MUSiCObjects::get_scale_factor(
                    photon_sf, is_data, {get_year_for_photon_sf(year), "sf", "wp90", photon_p4.eta(), photon_p4.pt()});

                auto sf_veto = MUSiCObjects::get_scale_factor(
                    photon_csev_sf, is_data, {get_year_for_photon_sf(year), "sf", "MVA", "EBInc"});

                auto sf_id_up = MUSiCObjects::get_scale_factor(
                    photon_sf,
                    is_data,
                    {get_year_for_photon_sf(year), "sfup", "wp90", photon_p4.eta(), photon_p4.pt()});

                auto sf_veto_up = MUSiCObjects::get_scale_factor(
                    photon_csev_sf, is_data, {get_year_for_photon_sf(year), "sfup", "MVA", "EBInc"});

                auto sf_id_down = MUSiCObjects::get_scale_factor(
                    photon_sf,
                    is_data,
                    {get_year_for_photon_sf(year), "sfdown", "wp90", photon_p4.eta(), photon_p4.pt()});

                auto sf_veto_down = MUSiCObjects::get_scale_factor(
                    photon_csev_sf, is_data, {get_year_for_photon_sf(year), "sfdown", "MVA", "EBInc"});

                if (shift == Shifts::Variations::Nominal)
                {
                    MUSiCObjects::push_sf_inplace(scale_factors, Shifts::Variations::Nominal, sf_veto * sf_id);
                    MUSiCObjects::push_sf_inplace(scale_factors, Shifts::Variations::PhotonId_Up, sf_veto_up * sf_id);
                    MUSiCObjects::push_sf_inplace(
                        scale_factors, Shifts::Variations::PhotonId_Down, sf_veto_down * sf_id);

                    MUSiCObjects::push_sf_inplace(scale_factors, Shifts::Variations::PhotonVeto_Up, sf_veto * sf_id_up);
                    MUSiCObjects::push_sf_inplace(
                        scale_factors, Shifts::Variations::PhotonVeto_Down, sf_veto * sf_id_down);
                }

                if (Shifts::is_diff(shift))
                {
                    MUSiCObjects::push_sf_inplace(scale_factors, shift, sf_veto * sf_id);
                }

                delta_met_x.push_back((photon_p4.pt() - Photon_pt[i]) * std::cos(Photon_phi[i]));
                delta_met_y.push_back((photon_p4.pt() - Photon_pt[i]) * std::sin(Photon_phi[i]));

                photons_p4.push_back(photon_p4);

                is_fake.push_back(is_data ? false : Photon_genPartIdx[i] < 0);
            }
        }
    }

    return MUSiCObjects(photons_p4,    //
                        scale_factors, //
                        delta_met_x,   //
                        delta_met_y,   //
                        is_fake);
}

} // namespace ObjectFactories

#endif // !MAKE_PHOTONS_HPP
