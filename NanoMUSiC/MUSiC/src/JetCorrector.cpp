#include "JetCorrector.hpp"

#include <stdexcept>
#include <string>

#include <ROOT/RVec.hxx>
#include <TRandom3.h>

#include "fmt/format.h"

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace std::string_literals;

JetCorrector::JetCorrector(const Year &_year,
                           const bool _is_data,
                           const std::string &_correction_file,
                           const std::string &_correction_key)
    : year(_year),
      is_data(_is_data),
      rand(TRandom3())
{
    switch (year)
    {
    case Year::Run2016APV:
        pt_resolution_correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016preVFP_UL/jet_jerc.json.gz")
                ->at("Summer20UL16APV_JRV3_MC_PtResolution_AK4PFchs");
        scale_factor_correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016preVFP_UL/jet_jerc.json.gz")
                ->at("Summer20UL16APV_JRV3_MC_ScaleFactor_AK4PFchs");
    case Year::Run2016:
        pt_resolution_correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016postVFP_UL/jet_jerc.json.gz")
                ->at("Summer20UL16_JRV3_MC_PtResolution_AK4PFchs");
        scale_factor_correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016postVFP_UL/jet_jerc.json.gz")
                ->at("Summer20UL16_JRV3_MC_ScaleFactor_AK4PFchs");
    case Year::Run2017:
        pt_resolution_correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2017_UL/jet_jerc.json.gz")
                ->at("Summer19UL17_JRV2_MC_PtResolution_AK4PFchs");
        scale_factor_correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2017_UL/jet_jerc.json.gz")
                ->at("Summer19UL17_JRV2_MC_ScaleFactor_AK4PFchs");
    case Year::Run2018:
        pt_resolution_correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2018_UL/jet_jerc.json.gz")
                ->at("Summer19UL18_JRV2_MC_PtResolution_AK4PFchs");
        scale_factor_correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2018_UL/jet_jerc.json.gz")
                ->at("Summer19UL18_JRV2_MC_ScaleFactor_AK4PFchs");
    default:
        throw std::runtime_error(
            fmt::format("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).\n", year));
    }
}

auto JetCorrector::get_resolution(float pt, float eta, float rho) const -> float
{
    return pt_resolution_correction_ref->evaluate({eta, pt, rho});
}

auto JetCorrector::get_resolution_scale_factor(float eta, const std::string &variation) const -> float
{
    if (variation == "Nominal"s)
    {
        return scale_factor_correction_ref->evaluate({eta, "nom"});
    }
    if (variation == "Up"s)
    {
        return scale_factor_correction_ref->evaluate({eta, "up"});
    }
    if (variation == "Down"s)
    {
        return scale_factor_correction_ref->evaluate({eta, "down"});
    }
    throw(std::runtime_error(fmt::format("The requested variation ({}) is not available.", variation)));
}

//////////////////////////////////////////////////////////
/// Calculate JER factor using the Hybrid Mode.
/// References:
/// https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution#JER_Scaling_factors_and_Uncertai
/// https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution#Smearing_procedures
auto JetCorrector::get_resolution_correction(float pt,
                                             float eta,
                                             float phi,
                                             float rho,
                                             int genjet_idx,
                                             float gen_jet_pt,
                                             float gen_jet_eta,
                                             float gen_jet_phi,
                                             const std::string &variation) -> float
{
    if (not is_data)
    {
        // no eta>5 possible set it to 5 times the sign of eta
        if (std::fabs(eta) >= 5)
        {
            eta = 4.99 * eta / std::fabs(eta);
        }

        double scaling_factor = 1.0;
        scaling_factor = get_resolution_scale_factor(eta, variation);
        double const resolution = get_resolution(pt, eta, rho);

        // Found a match?
        auto is_good_match = [&]() -> bool
        {
            if (genjet_idx >= 0)
            {
                const double radius = 0.4;
                const double dr = VecOps::DeltaR(eta, gen_jet_eta, phi, gen_jet_phi);
                const double dpt = std::fabs(pt - gen_jet_pt);
                if ((dr < radius / 2.) and (dpt < 3 * resolution * pt))
                {
                    return true;
                }
            }
            return false;
        };

        if (is_good_match())
        {
            return std::max(min_correction_factor, 1 + (scaling_factor - 1) * (pt - gen_jet_pt) / pt);
        }

        // If not, just smear with a Gaussian.
        return std::max(min_correction_factor,
                        1 + rand.Gaus(0, resolution) * std::sqrt(std::max(std::pow(scaling_factor, 2) - 1.0, 0.0)));
    }
    return 1.;
}
