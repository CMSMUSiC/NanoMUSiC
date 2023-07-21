#ifndef MAKE_MUONS_HPP
#define MAKE_MUONS_HPP

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

#include "Configs.hpp"
#include "CorrectionSets.hpp"
#include "music_objects.hpp"

namespace ObjectFactories
{

// TODO implement muon corrections
inline auto get_muon_energy_corrections(const std::string &shift) -> double
{
    return 1.;
}

///////////////////////////////////////////////////////////////
/// For some reason, the Official Muon SFs requires a field of the requested year, with proper formating.
inline auto get_year_for_muon_sf(Year year) -> std::string
{
    switch (year)
    {
    case Year::Run2016APV:
        return "2016preVFP_UL"s;
    case Year::Run2016:
        return "2016postVFP_UL"s;
    case Year::Run2017:
        return "2017_UL"s;
    case Year::Run2018:
        return "2018_UL"s;
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

inline auto make_muons(const RVec<float> &Muon_pt,                   //
                       const RVec<float> &Muon_eta,                  //
                       const RVec<float> &Muon_phi,                  //
                       const RVec<bool> &Muon_tightId,               //
                       const RVec<UChar_t> &Muon_highPtId,           //
                       const RVec<float> &Muon_pfRelIso04_all,       //
                       const RVec<float> &Muon_tkRelIso,             //
                       const RVec<float> &Muon_tunepRelPt,           //
                       const RVec<float> &Muon_highPurity,           //
                       const RVec<int> &Muon_genPartIdx,             //
                       const CorrectionlibRef_t &muon_sf_reco,       //
                       const CorrectionlibRef_t &muon_sf_id_low_pt,  //
                       const CorrectionlibRef_t &muon_sf_id_high_pt, //
                       const CorrectionlibRef_t &muon_sf_iso_low_pt, //
                       const CorrectionlibRef_t &muon_sf_iso_high_pt,
                       bool is_data,             //
                       const std::string &_year, //
                       const std::string &shift) -> MUSiCObjects
{
    auto year = get_runyear(_year);

    auto muons_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = RVec<double>{};
    auto scale_factor_up = RVec<double>{};
    auto scale_factor_down = RVec<double>{};
    auto delta_met_x = RVec<double>{};
    auto delta_met_y = RVec<double>{};
    auto is_fake = RVec<bool>{};

    for (std::size_t i = 0; i < Muon_pt.size(); i++)
    {

        bool is_good_low_pt_muon_pre_filter = (std::fabs(Muon_eta.at(i)) <= ObjConfig::Muons[year].MaxAbsEta) //
                                              and (Muon_tightId.at(i))                                        //
                                              and (Muon_pfRelIso04_all.at(i) < ObjConfig::Muons[year].PFRelIso_WP);

        bool is_good_high_pt_muon_pre_filter = (std::fabs(Muon_eta.at(i)) <= ObjConfig::Muons[year].MaxAbsEta) //
                                               and (Muon_highPtId.at(i) >= 2)                                  //
                                               and (Muon_tkRelIso.at(i) < ObjConfig::Muons[year].TkRelIso_WP   //
                                                    and Muon_highPurity.at(i));

        float pt_correction_factor = 1.f;
        if (Muon_pt.at(i) >= ObjConfig::Muons[year].MaxLowPt)
        {
            // For some reason, the Relative pT Tune can yield very low corrected pT. Because of this,
            // they will be caped to 25., in order to not break the JSON SFs bound checks.
            // https://cms-nanoaod-integration.web.cern.ch/commonJSONSFs/summaries/MUO_2016postVFP_UL_muon_Z.html
            // leading_muon.SetP t(std::max(Muon_tunepRelPt[0] * leading_muon.pt(), 25.));
            pt_correction_factor = Muon_tunepRelPt.at(i);
        }

        if (is_good_low_pt_muon_pre_filter or is_good_high_pt_muon_pre_filter)
        {
            // build a muon and apply energy corrections
            auto muon_p4 =
                Math::PtEtaPhiMVector(std::max(Muon_pt[i] * pt_correction_factor, ObjConfig::Muons[year].MinLowPt),
                                      Muon_eta[i],
                                      Muon_phi[i],
                                      PDG::Muon::Mass) *
                get_muon_energy_corrections(shift);

            auto is_good_low_pt_muon = (muon_p4.pt() >= ObjConfig::Muons[year].MinLowPt) and
                                       (muon_p4.pt() < ObjConfig::Muons[year].MaxLowPt) and
                                       is_good_low_pt_muon_pre_filter;
            auto is_good_high_pt_muon =
                (muon_p4.pt() >= ObjConfig::Muons[year].MaxLowPt) and is_good_low_pt_muon_pre_filter;

            // calculate scale factors per object (particle)
            if (is_good_low_pt_muon)
            {
                scale_factors.push_back(
                    MUSiCObjects::get_scale_factor(
                        muon_sf_reco,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "sf"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_id_low_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "sf"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_iso_low_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "sf"}));

                scale_factor_up.push_back(
                    MUSiCObjects::get_scale_factor(
                        muon_sf_reco,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_id_low_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_iso_low_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"}));

                scale_factor_down.push_back(
                    MUSiCObjects::get_scale_factor(
                        muon_sf_reco,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_id_low_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_iso_low_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"}));
            }

            if (is_good_high_pt_muon)
            {
                scale_factors.push_back(
                    MUSiCObjects::get_scale_factor(
                        muon_sf_reco,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "sf"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_id_low_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "sf"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_iso_low_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "sf"}));

                scale_factor_up.push_back(
                    MUSiCObjects::get_scale_factor(
                        muon_sf_reco,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_id_high_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_iso_high_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"}));

                scale_factor_down.push_back(
                    MUSiCObjects::get_scale_factor(
                        muon_sf_reco,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_id_high_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"}) *
                    MUSiCObjects::get_scale_factor(
                        muon_sf_iso_high_pt,
                        is_data,
                        {get_year_for_muon_sf(year), std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"}));
            }

            if (is_good_low_pt_muon or is_good_high_pt_muon)
            {
                muons_p4.push_back(muon_p4);

                delta_met_x.push_back((muon_p4.pt() - Muon_pt[i]) * std::cos(Muon_phi[i]));
                delta_met_y.push_back((muon_p4.pt() - Muon_pt[i]) * std::sin(Muon_phi[i]));

                is_fake.push_back(is_data ? false : Muon_genPartIdx[i] == -1);
            }
        }
    }

    return MUSiCObjects(muons_p4,          //
                        scale_factors,     //
                        scale_factor_up,   //
                        scale_factor_down, //
                        delta_met_x,       //
                        delta_met_y,       //
                        is_fake);
}

} // namespace ObjectFactories

#endif // !MAKE_MUONS_HPP