#include "JetCorrector.hpp"

#include <stdexcept>
#include <string>

#include "fmt/format.h"

using namespace std::string_literals;

JetCorrector::JetCorrector(const Year &_year,
                           const bool _is_data,
                           const std::string &_correction_file,
                           const std::string &_correction_key)
    : year(_year),
      is_data(_is_data)
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

//////////////////////////////////////////////////////////
/// References:
/// https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution#JER_Scaling_factors_and_Uncertai
/// https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution#Smearing_procedures
auto JetCorrector::get_resolution_correction(float pt, float eta, float rho, const std::string &variation) -> float
{
    if (not is_data)
    {
        double const recJetPt = pt;
        double recJetEta = eta;
        // no eta>5 possible set it to 5 times the sign of eta
        if (std::fabs(recJetEta) >= 5)
        {
            recJetEta = 4.99 * recJetEta / std::fabs(recJetEta);
        }

        double scaling_factor = 1.0;
        scaling_factor = get_resolution_scale_factor(recJetEta, variation);

        double jetCorrFactor = 1.0;

        // Found a match?
        if (genJet)
        {
            double const genJetPt = genJet->getPt();
            jetCorrFactor = std::max(0.0, 1 + (scaling_factor - 1) * (recJetPt - genJetPt) / recJetPt);

            // If not, just smear with a Gaussian.
        }
        else
        {
            double const sigma_MC = get_resolution(recJetPt, recJetEta, rho);

            jetCorrFactor = std::max(
                0.0, 1 + m_rand.Gaus(0, sigma_MC) * std::sqrt(std::max(scaling_factor * scaling_factor - 1.0, 0.0)));
        }

        // WARNING: 0 can be returned! Catch this case at the place this function is
        // used!
        return jetCorrFactor;
    }
    return 1.;
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