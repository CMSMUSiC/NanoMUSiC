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
#include "Shifts.hpp"
#include "music_objects.hpp"

namespace ObjectFactories
{

// 2016 - https://twiki.cern.ch/twiki/bin/view/CMS/MuonUL2016
// p (GeV)	50-100	100-150	150-200	200-300	300-400	400-600	600-1500	1500-3500
// abs(η)<1.6	0.9914 (0.0008)	0.9936 (0.0009)	0.993 (0.001)	0.993 (0.002)	0.990 (0.004)	0.990 (0.003)	0.989
// (0.004) 0.8 (0.3) 1.6<abs(η)<2.4	-	0.993 (0.001)	0.991 (0.001)	0.985 (0.001)	0.981 (0.002)	0.979 (0.004)
// 0.978 (0.005)	0.9 (0.2)

// 2017 - https://twiki.cern.ch/twiki/bin/view/CMS/MuonUL2017
// p (GeV)	50-100	100-150	150-200	200-300	300-400	400-600	600-1500	1500-3500
// abs(η)<1.6	0.9938 (0.0006)	0.9950 (0.0007)	0.996 (0.001)	0.996 (0.001)	0.994 (0.001)	1.003 (0.006)	0.987
// (0.003) 0.9 (0.1) 1.6<abs(η)<2.4	-	0.993 (0.001)	0.989 (0.001)	0.986 (0.001)	0.989 (0.001)	0.983 (0.003)
// 0.986 (0.006)	1.01 (0.01)

// 2018 - https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonUL2018
// p (GeV)	50-100	100-150	150-200	200-300	300-400	400-600	600-1500	1500-3500
// abs(η)<1.6	0.9943 (0.0007)	0.9948 (0.0007)	0.9950 (0.0009)	0.994 (0.001)	0.9914 (0.0009)	0.993 (0.002)	0.991
// (0.004)	1.0 (0.1)
// 1.6<abs(η)<2.4	-	0.993 (0.001)	0.990 (0.001)	0.988 (0.001)	0.981 (0.002)	0.983 (0.003)	0.978 (0.006)
// 0.98 (0.03)

inline auto high_pt_reco_scale_factor(Year year, double momentum, double eta, const std::string &shift) -> double
{
    auto sf = 1.;
    auto delta = 0.;
    auto modifier = 0.;

    if (year == Year::Run2016APV or year == Year::Run2016)
    {
        if (50. < momentum and momentum <= 100.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.9914;
                delta = 0.0008;
            }
            else
            {
                sf = 1;
                delta = 0.;
            }
        }
        if (100. < momentum and momentum <= 150.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.9936;
                delta = 0.0009;
            }
            else
            {
                sf = 0.993;
                delta = 0.001;
            }
        }
        if (150. < momentum and momentum <= 200.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.993;
                delta = 0.001;
            }
            else
            {
                sf = 0.991;
                delta = 0.001;
            }
        }
        if (200. < momentum and momentum <= 300.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.993;
                delta = 0.002;
            }
            else
            {
                sf = 0.985;
                delta = 0.001;
            }
        }
        if (300. < momentum and momentum <= 400.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.990;
                delta = 0.004;
            }
            else
            {
                sf = 0.981;
                delta = 0.002;
            }
        }
        if (400. < momentum and momentum <= 600.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.990;
                delta = 0.003;
            }
            else
            {
                sf = 0.979;
                delta = 0.004;
            }
        }
        if (600. < momentum and momentum <= 1500.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.989;
                delta = 0.004;
            }
            else
            {
                sf = 0.978;
                delta = 0.005;
            }
        }
        if (1500. < momentum and momentum <= 3500.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.8;
                delta = 0.3;
            }
            else
            {
                sf = 0.9;
                delta = 0.2;
            }
        }
        if (momentum > 3500.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 1;
                delta = 0.;
            }
            else
            {
                sf = 1;
                delta = 0.;
            }
        }
    }
    if (year == Year::Run2017)
    {
        if (50. < momentum and momentum <= 100.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.9938;
                delta = 0.0006;
            }
            else
            {
                sf = 1.0;
                delta = 0.0;
            }
        }
        if (100. < momentum and momentum <= 150.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.9950;
                delta = 0.0007;
            }
            else
            {
                sf = 0.993;
                delta = 0.001;
            }
        }
        if (150. < momentum and momentum <= 200.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.996;
                delta = 0.001;
            }
            else
            {
                sf = 0.989;
                delta = 0.001;
            }
        }
        if (200. < momentum and momentum <= 300.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.996;
                delta = 0.001;
            }
            else
            {
                sf = 0.986;
                delta = 0.001;
            }
        }
        if (300. < momentum and momentum <= 400.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.994;
                delta = 0.001;
            }
            else
            {
                sf = 0.989;
                delta = 0.001;
            }
        }
        if (400. < momentum and momentum <= 600.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 1.003;
                delta = 0.006;
            }
            else
            {
                sf = 0.983;
                delta = 0.003;
            }
        }
        if (600. < momentum and momentum <= 1500.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.987;
                delta = 0.003;
            }
            else
            {
                sf = 0.986;
                delta = 0.006;
            }
        }
        if (1500. < momentum and momentum <= 3500.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.9;
                delta = 0.1;
            }
            else
            {
                sf = 1.01;
                delta = 0.01;
            }
        }
        if (momentum > 3500.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 1;
                delta = 0.;
            }
            else
            {
                sf = 1;
                delta = 0.;
            }
        }
    }

    if (year == Year::Run2018)
    {
        if (50. < momentum and momentum <= 100.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.9943;
                delta = 0.0007;
            }
            else
            {
                sf = 1.0;
                delta = 0.;
            }
        }
        if (100. < momentum and momentum <= 150.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.9948;
                delta = 0.0007;
            }
            else
            {
                sf = 0.993;
                delta = 0.001;
            }
        }
        if (150. < momentum and momentum <= 200.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.9950;
                delta = 0.0009;
            }
            else
            {
                sf = 0.990;
                delta = 0.001;
            }
        }
        if (200. < momentum and momentum <= 300.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.994;
                delta = 0.001;
            }
            else
            {
                sf = 0.988;
                delta = 0.001;
            }
        }
        if (300. < momentum and momentum <= 400.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.9914;
                delta = 0.0009;
            }
            else
            {
                sf = 0.981;
                delta = 0.002;
            }
        }
        if (400. < momentum and momentum <= 600.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.993;
                delta = 0.002;
            }
            else
            {
                sf = 0.983;
                delta = 0.003;
            }
        }
        if (600. < momentum and momentum <= 1500.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 0.991;
                delta = 0.004;
            }
            else
            {
                sf = 0.978;
                delta = 0.006;
            }
        }
        if (1500. < momentum and momentum <= 3500.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 1.0;
                delta = 0.1;
            }
            else
            {
                sf = 0.98;
                delta = 0.03;
            }
        }
        if (momentum > 3500.)
        {
            if (std::fabs(eta) < 1.6)
            {
                sf = 1.0;
                delta = 0.;
            }
            else
            {
                sf = 1.0;
                delta = 0.;
            }
        }
    }

    if (shift == "up")
    {
        modifier = 1.;
    }

    if (shift == "down")
    {
        modifier = -1.;
    }

    return sf + modifier * delta;
}

// TODO implement muon corrections
inline auto get_muon_energy_corrections(const Shifts::Variations shift) -> double
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
                       const Shifts::Variations shift) -> MUSiCObjects
{
    auto year = get_runyear(_year);

    auto muons_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = RVec<double>{};
    auto scale_factor_shift = RVec<double>{};
    auto delta_met_x = 0.;
    auto delta_met_y = 0.;
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
            pt_correction_factor = Muon_tunepRelPt.at(i);
        }

        // build a muon and apply energy corrections
        auto muon_p4 =
            Math::PtEtaPhiMVector(Muon_pt[i] * pt_correction_factor, Muon_eta[i], Muon_phi[i], PDG::Muon::Mass) *
            get_muon_energy_corrections(shift);

        delta_met_x += (muon_p4.pt() - Muon_pt[i]) * std::cos(Muon_phi[i]);
        delta_met_y += (muon_p4.pt() - Muon_pt[i]) * std::sin(Muon_phi[i]);

        if (is_good_low_pt_muon_pre_filter or is_good_high_pt_muon_pre_filter)
        {
            auto is_good_low_pt_muon = (muon_p4.pt() >= ObjConfig::Muons[year].MinLowPt) and
                                       (muon_p4.pt() < ObjConfig::Muons[year].MaxLowPt) and
                                       is_good_low_pt_muon_pre_filter;
            auto is_good_high_pt_muon =
                (muon_p4.pt() >= ObjConfig::Muons[year].MaxLowPt) and is_good_high_pt_muon_pre_filter;

            // calculate scale factors per object (particle)
            if (is_good_low_pt_muon)
            {

                auto sf_reco = muon_p4.pt() > 40.
                                   ? MUSiCObjects::get_scale_factor(
                                         muon_sf_reco, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "nominal"})
                                   : 1.0;
                auto sf_id = MUSiCObjects::get_scale_factor(
                    muon_sf_id_low_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "nominal"});
                auto sf_iso = MUSiCObjects::get_scale_factor(
                    muon_sf_iso_low_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "nominal"});

                auto sf_reco_up = muon_p4.pt() > 40.
                                      ? MUSiCObjects::get_scale_factor(
                                            muon_sf_reco, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"})
                                      : 1.0;
                auto sf_id_up = MUSiCObjects::get_scale_factor(
                    muon_sf_id_low_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"});
                auto sf_iso_up = MUSiCObjects::get_scale_factor(
                    muon_sf_iso_low_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"});

                auto sf_reco_down =
                    muon_p4.pt() > 40.
                        ? MUSiCObjects::get_scale_factor(
                              muon_sf_reco, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"})
                        : 1.0;
                auto sf_id_down = MUSiCObjects::get_scale_factor(
                    muon_sf_id_low_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"});
                auto sf_iso_down = MUSiCObjects::get_scale_factor(
                    muon_sf_iso_low_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"});

                scale_factors.push_back(sf_reco * sf_id * sf_iso);
                scale_factor_shift.push_back(std::sqrt(                                                       //
                    std::pow(std::max(std::fabs(sf_reco - sf_reco_up), std::fabs(sf_reco - sf_reco_down)), 2) //
                    + std::pow(std::max(std::fabs(sf_id - sf_id_up), std::fabs(sf_id - sf_id_down)), 2)       //
                    + std::pow(std::max(std::fabs(sf_iso - sf_iso_up), std::fabs(sf_iso - sf_iso_down)), 2)   //
                    ));
            }

            if (is_good_high_pt_muon)
            {
                auto sf_reco = high_pt_reco_scale_factor(year, muon_p4.P(), muon_p4.eta(), "nom");

                auto sf_id = MUSiCObjects::get_scale_factor(
                    muon_sf_id_high_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "nominal"});

                auto sf_iso = MUSiCObjects::get_scale_factor(
                    muon_sf_iso_high_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "nominal"});

                auto sf_reco_up = high_pt_reco_scale_factor(year, muon_p4.P(), muon_p4.eta(), "up");

                auto sf_id_up = MUSiCObjects::get_scale_factor(
                    muon_sf_id_high_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"});

                auto sf_iso_up = MUSiCObjects::get_scale_factor(
                    muon_sf_iso_high_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systup"});

                auto sf_reco_down = high_pt_reco_scale_factor(year, muon_p4.P(), muon_p4.eta(), "down");

                auto sf_id_down = MUSiCObjects::get_scale_factor(
                    muon_sf_id_high_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"});

                auto sf_iso_down = MUSiCObjects::get_scale_factor(
                    muon_sf_iso_high_pt, is_data, {std::fabs(muon_p4.eta()), muon_p4.pt(), "systdown"});

                scale_factors.push_back(sf_reco * sf_id * sf_iso);
                scale_factor_shift.push_back(std::sqrt(                                                        //
                    std::pow(std::max(std::fabs(sf_reco - sf_reco_up), std::fabs(sf_reco - sf_reco_down)), 2) //
                    + std::pow(std::max(std::fabs(sf_id - sf_id_up), std::fabs(sf_id - sf_id_down)), 2)       //
                    + std::pow(std::max(std::fabs(sf_iso - sf_iso_up), std::fabs(sf_iso - sf_iso_down)), 2)   //
                    ));
            }

            if (is_good_low_pt_muon or is_good_high_pt_muon)
            {
                muons_p4.push_back(muon_p4);
                is_fake.push_back(is_data ? false : Muon_genPartIdx[i] < 0);
            }
        }
    }

    return MUSiCObjects(muons_p4,           //
                        scale_factors,      //
                        scale_factor_shift, //
                        delta_met_x,        //
                        delta_met_y,        //
                        is_fake);
}

} // namespace ObjectFactories

#endif // !MAKE_MUONS_HPP
