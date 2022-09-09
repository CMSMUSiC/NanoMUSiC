#include "JetResolution.hh"

#include <cmath>

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Tools/Tools.hh"

JetResolution::JetResolution(Tools::MConfig const &config, std::string const &jetType)
    : m_rand(), m_resolution_correction_set(correction::CorrectionSet::from_file(
                    Tools::AbsolutePath(config.GetItem<std::string>(jetType + ".JSONFile"))))
{

    auto year = config.GetItem<std::string>("year");

    // jet algo (AK4 x AK8)
    std::string algo = [&]() {
        // if Jet
        if (jetType == "Jet")
        {
            return "AK4PFchs";
        }
        // if FatJet
        return "AK8PFPuppi";
    }();

    // Scale Factor parameters
    std::map<std::string, std::string> sf_key = {
        {"2016APV", "Summer20UL16APV_JRV3_MC_ScaleFactor_" + algo},
        {"2016", "Summer20UL16_JRV3_MC_ScaleFactor_" + algo},
        {"2017", "Summer19UL17_JRV2_MC_ScaleFactor_" + algo},
        {"2018", "Summer19UL18_JRV2_MC_ScaleFactor_" + algo},
    };

    // FIXME: Summer19UL17_JRV2_MC does not includes Scale Factors for AK8Puppi jets#30
    // https://github.com/CMSMUSiC/NanoMUSiC/issues/30
    if (year == "2017" && jetType == "FatJet")
    {
        sf_key["2017"] = "Summer19UL18_JRV2_MC_ScaleFactor_" + algo;
        auto json_file_2018 = Tools::AbsolutePath(config.GetItem<std::string>(jetType + ".JSONFile.2018"));
        m_resolution_correction_set = correction::CorrectionSet::from_file(json_file_2018);
    }

    m_resolution_scale_factor = m_resolution_correction_set->at(sf_key[year]);

    // Resolution parameters
    std::map<std::string, std::string> resolution_key = {
        {"2016APV", "Summer20UL16APV_JRV3_MC_PtResolution_" + algo},
        {"2016", "Summer20UL16_JRV3_MC_PtResolution_" + algo},
        {"2017", "Summer19UL17_JRV2_MC_PtResolution_" + algo},
        {"2018", "Summer19UL18_JRV2_MC_PtResolution_" + algo},
    };
    // FIXME: Summer19UL17_JRV2_MC does not includes Scale Factors for AK8Puppi jets#30
    // https://github.com/CMSMUSiC/NanoMUSiC/issues/30
    if (year == "2017" && jetType == "FatJet")
    {
        resolution_key["2017"] = "Summer19UL18_JRV2_MC_PtResolution_" + algo;
    }

    m_resolution = m_resolution_correction_set->at(resolution_key[year]);
}

double JetResolution::getResolution(double const jet_pt, double const jet_eta, double const rho) const
{
    return m_resolution->evaluate({jet_eta, jet_pt, rho});
}

double JetResolution::getResolutionSF(double const jet_eta, int const updown) const
{
    if (updown == 0)
    {
        return m_resolution_scale_factor->evaluate({jet_eta, "nom"});
    }
    else if (updown == -1)
    {
        return m_resolution_scale_factor->evaluate({jet_eta, "up"});
    }
    else if (updown == 1)
    {
        return m_resolution_scale_factor->evaluate({jet_eta, "down"});
    }
    else
    {
        std::stringstream err;
        err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
        err << "Variaton of jet resolution scale factor for " << updown << " not possible. Choose:" << std::endl;
        err << "\t 0 - no variation\n\t 1 - scale up\n\t-1 - scale down " << std::endl;
        throw Tools::value_error(err.str());
        return 0;
    }
}

double JetResolution::getJetResolutionCorrFactor(pxl::Particle const *recJet, pxl::Particle const *genJet,
                                                 double const npv = 0, double const rho = 0, int const updown = 0)
{
    double const recJetPt = recJet->getPt();
    double recJetEta = recJet->getEta();
    // no eta>5 possible set it to 5 times the sign of eta
    if (fabs(recJetEta) >= 5)
    {
        recJetEta = 4.99 * recJetEta / fabs(recJetEta);
    }

    double scaling_factor = 1.0;
    scaling_factor = getResolutionSF(recJetEta, updown);

    double jetCorrFactor = 1.0;

    // Formula: https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution#Smearing_procedures?rev=56
    // Found a match?
    if (genJet)
    {
        double const genJetPt = genJet->getPt();
        jetCorrFactor = std::max(0.0, 1 + (scaling_factor - 1) * (recJetPt - genJetPt) / recJetPt);

        // If not, just smear with a Gaussian.
    }
    else
    {
        double const sigma_MC = getResolution(recJetPt, recJetEta, rho);

        jetCorrFactor = std::max(0.0, 1 + m_rand.Gaus(0, sigma_MC) *
                                              std::sqrt(std::max(scaling_factor * scaling_factor - 1.0, 0.0)));
    }

    // WARNING: 0 can be returned! Catch this case at the place this function is
    // used!
    return jetCorrFactor;
}

pxl::Particle *JetResolution::matchGenJet(const pxl::Particle *rec_jet, const std::vector<pxl::Particle *> &gen_jets,
                                          const double radius, const double npv) const
{
    // Matching according to https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution?rev=62 (October 2018)
    for (auto &gen_jet : gen_jets)
    {
        const pxl::LorentzVector &gen_vec = gen_jet->getVector();
        const pxl::LorentzVector &rec_vec = rec_jet->getVector();
        const double dr = gen_vec.deltaR(&rec_vec);
        const double dpt = fabs(rec_vec.getPt() - gen_vec.getPt());
        if (dr < radius / 2 and dpt < 3 * getResolution(rec_jet->getPt(), rec_jet->getEta(), 0) * rec_jet->getPt())
            return gen_jet;
    }
    return nullptr;
}
