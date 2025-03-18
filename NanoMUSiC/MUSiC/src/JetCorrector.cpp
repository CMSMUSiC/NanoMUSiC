#include "JetCorrector.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

#include <ROOT/RVec.hxx>
#include <TRandom3.h>

#include "fmt/format.h"

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace std::string_literals;

JetCorrector::JetCorrector(const Year &_year, const std::string &_era, const bool _is_data)
    : year(_year),
      era(_era),
      is_data(_is_data),
      rand()
{
    std::string json_file = ""s;
    std::string scale_correction_key = ""s;
    std::string scale_uncertainty_correction_key = ""s;

    switch (year)
    {
    case Year::Run2016APV:
        json_file = "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016preVFP_UL/jet_jerc.json.gz"s;

        // JER
        pt_resolution_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at("Summer20UL16APV_JRV3_MC_PtResolution_AK4PFchs");
        scale_factor_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at("Summer20UL16APV_JRV3_MC_ScaleFactor_AK4PFchs");

        // JES
        if (_is_data)
        {
            if (era == "B" or era == "C" or era == "D")
            {
                scale_correction_key = "Summer19UL16APV_RunBCD_V7_DATA_L1L2L3Res_AK4PFchs";
            }
            else
            {
                scale_correction_key = "Summer19UL16APV_RunEF_V7_DATA_L1L2L3Res_AK4PFchs";
            }
        }
        else
        {
            scale_correction_key = "Summer19UL16APV_V7_MC_L1L2L3Res_AK4PFchs";
        }
        scale_uncertainty_correction_key = "Summer19UL16APV_V7_MC_Total_AK4PFchs";
        scale_correction_ref = correction::CorrectionSet::from_file(json_file)->compound().at(scale_correction_key);
        scale_uncertainty_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at(scale_uncertainty_correction_key);
        break;
    case Year::Run2016:
        json_file = "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016postVFP_UL/jet_jerc.json.gz"s;

        // JER
        pt_resolution_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at("Summer20UL16_JRV3_MC_PtResolution_AK4PFchs");
        scale_factor_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at("Summer20UL16_JRV3_MC_ScaleFactor_AK4PFchs");

        // JES
        if (_is_data)
        {
            scale_correction_key = "Summer19UL16_RunFGH_V7_DATA_L1L2L3Res_AK4PFchs";
        }
        else
        {
            scale_correction_key = "Summer19UL16_V7_MC_L1L2L3Res_AK4PFchs";
        }
        scale_uncertainty_correction_key = "Summer19UL16_V7_MC_Total_AK4PFchs";

        scale_correction_ref = correction::CorrectionSet::from_file(json_file)->compound().at(scale_correction_key);
        scale_uncertainty_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at(scale_uncertainty_correction_key);
        break;
    case Year::Run2017:
        json_file = "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2017_UL/jet_jerc.json.gz"s;

        // JER
        pt_resolution_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at("Summer19UL17_JRV2_MC_PtResolution_AK4PFchs");
        scale_factor_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at("Summer19UL17_JRV2_MC_ScaleFactor_AK4PFchs");

        // JES
        if (_is_data)
        {
            scale_correction_key = fmt::format("Summer19UL17_Run{}_V5_DATA_L1L2L3Res_AK4PFchs", era);
        }
        else
        {
            scale_correction_key = "Summer19UL17_V5_MC_L1L2L3Res_AK4PFchs";
        }
        scale_uncertainty_correction_key = "Summer19UL17_V5_MC_Total_AK4PFchs";
        scale_correction_ref = correction::CorrectionSet::from_file(json_file)->compound().at(scale_correction_key);
        scale_uncertainty_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at(scale_uncertainty_correction_key);
        break;
    case Year::Run2018:
        json_file = "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2018_UL/jet_jerc.json.gz"s;

        // JER
        pt_resolution_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at("Summer19UL18_JRV2_MC_PtResolution_AK4PFchs");
        scale_factor_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at("Summer19UL18_JRV2_MC_ScaleFactor_AK4PFchs");

        // JES
        if (_is_data)
        {
            scale_correction_key = fmt::format("Summer19UL18_Run{}_V5_DATA_L1L2L3Res_AK4PFchs", era);
        }
        else
        {
            scale_correction_key = "Summer19UL18_V5_MC_L1L2L3Res_AK4PFchs";
        }
        scale_uncertainty_correction_key = "Summer19UL18_V5_MC_Total_AK4PFchs";

        scale_correction_ref = correction::CorrectionSet::from_file(json_file)->compound().at(scale_correction_key);
        scale_uncertainty_correction_ref =
            correction::CorrectionSet::from_file(json_file)->at(scale_uncertainty_correction_key);
        break;
    default:
        throw std::runtime_error(
            fmt::format("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).\n", static_cast<int>(year)));
    }
}

auto JetCorrector::get_resolution(float pt, float eta, float rho) const -> double
{
    return pt_resolution_correction_ref->evaluate({eta, pt, rho});
}

auto JetCorrector::get_resolution_scale_factor(float eta, const std::string &variation) const -> double
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

auto is_good_match(float pt,
                   float eta,
                   float phi,
                   int genjet_idx,
                   const NanoAODGenInfo::GenJets &gen_jets,
                   double resolution) -> bool
{
    if (genjet_idx >= 0 and static_cast<std::size_t>(genjet_idx) < gen_jets.size)
    {
        const double radius = 0.4;
        const double dr = ROOT::VecOps::DeltaR(eta, gen_jets.eta.at(genjet_idx), phi, gen_jets.phi.at(genjet_idx));
        const double dpt = std::fabs(pt - gen_jets.pt.at(genjet_idx));
        if ((dr < radius / 2.) and (dpt < 3 * resolution * pt))
        {
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////
/// Calculate JER factor using the Hybrid Mode.
/// References:
/// https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution#JER_Scaling_factors_and_Uncertai
/// https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution#Smearing_procedures
/// https://cms-jerc.web.cern.ch/JEC/
auto JetCorrector::get_resolution_correction(float pt,
                                             float eta,
                                             float phi,
                                             float rho,
                                             int genjet_idx,
                                             const NanoAODGenInfo::GenJets &gen_jets,
                                             const std::string &variation) -> double
{
    if (not is_data)
    {
        // no eta>5.2 possible set it to 5.2 times the sign of eta
        if (std::fabs(eta) >= 5.2)
        {
            eta = 5.199 * eta / std::fabs(eta);
        }

        const double scaling_factor = get_resolution_scale_factor(eta, variation);
        const double resolution = get_resolution(pt, eta, rho);

        // Found a match?
        if (is_good_match(pt, eta, phi, genjet_idx, gen_jets, resolution))
        {
            return std::max(min_correction_factor, 1 + (scaling_factor - 1) * (pt - gen_jets.pt.at(genjet_idx)) / pt);
        }

        // If not, just smear with a Gaussian.
        return std::max(min_correction_factor,
                        1 + rand.Gaus(0, resolution) * std::sqrt(std::max(std::pow(scaling_factor, 2.) - 1.0, 0.0)));
    }
    return 1.;
}

/////////////////////////////////////////////
/// JESType
/// Reference:
/// https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#JetCorUncertainties
/// https://twiki.cern.ch/twiki/bin/view/CMS/JECUncertaintySources
/// https://cms-jerc.web.cern.ch/JEC/
auto JetCorrector::get_scale_correction(float pt,
                                        float eta,
                                        float phi,
                                        float raw_factor,
                                        float rho,
                                        float area,
                                        const std::string &variation) const -> double
{
    double correction_factor =
        (1 - raw_factor) * scale_correction_ref->evaluate({area, eta, pt * (1. - raw_factor), rho});

    // if MC
    if (not is_data)
    {
        double correction_uncertainty = scale_uncertainty_correction_ref->evaluate({eta, pt * correction_factor});

        if (variation == "Nominal"s)
        {
            return correction_factor;
        }
        if (variation == "Up"s)
        {
            return correction_factor * (1. + correction_uncertainty);
        }
        if (variation == "Down"s)
        {
            return correction_factor * (1. - correction_uncertainty);
        }

	fmt::print(stderr, "The requested variation ({}) is not available.", variation);
	std::exit(EXIT_FAILURE);
    }
    return correction_factor;
}
