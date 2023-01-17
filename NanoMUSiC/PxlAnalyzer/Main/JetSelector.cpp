#include "JetSelector.hpp"

JetSelector::JetSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap, const std::string &name)
    : // Add parent constructor
      ObjectSelector(cfg, globalOldNameMap, name, true),
      // general:
      // Jets:
      m_jet_use(cfg.GetItem<bool>(name + ".use")), m_jet_pt_min(cfg.GetItem<double>(name + ".pt.min")),
      m_jet_isPF(cfg.GetItem<bool>(name + ".isPF")), m_jet_ID_use(cfg.GetItem<bool>(name + ".ID.use")),
      m_jet_ID_type(cfg.GetItem<std::string>(name + ".ID.Type")),
      m_jet_gen_hadOverEm_min(cfg.GetItem<double>(name + ".Gen.HadOverEm.min")),
      m_jet_gen_hadEFrac_min(cfg.GetItem<double>(name + ".Gen.HadEFrac.min")),

      // In case we do the ID on our own:
      m_jet_nHadEFrac_max(cfg.GetItem<double>(name + ".NeutralHadronEnergyFraction.max", 0.99)),
      m_jet_nEMEFrac_max(cfg.GetItem<double>(name + ".NeutralEMEnergyFraction.max", 0.99)),
      m_jet_numConstituents_min(cfg.GetItem<unsigned long>(name + ".NumConstituents.min", 2)),
      // Only for |eta| > 2.4:
      m_jet_cHadEFrac_min(cfg.GetItem<double>(name + ".ChargedHadronEnergyFraction.min", 0.0)),
      m_jet_cEMEFrac_max(cfg.GetItem<double>(name + ".ChargedEMEnergyFraction.max", 0.99)),
      m_jet_cMultiplicity_min(cfg.GetItem<unsigned long>(name + ".chargedMultiplicity.min", 1))

{
}

JetSelector::~JetSelector()
{
}

int JetSelector::passObjectSelection(pxl::Particle *jet, double const jetRho, const std::string &idType,
                                     const bool isSyst // use alternative kinematic cuts for syst
) const
{
    bool passKin = ObjectSelector::passKinematics(jet, isSyst);
    bool passID = true;

    double const absEta = std::abs(jet->getEta());
    jet->setUserRecord("usedID", m_jet_ID_type);
    if (m_jet_ID_use)
    {
        if (not jet->getUserRecord(m_jet_ID_type).asBool())
            passID = false;
    }
    else
    {
        // We do it ourselves!
        if (jet->getUserRecord("neutralHadronEnergyFraction").toDouble() > m_jet_nHadEFrac_max)
            passID = false;
        if (jet->getUserRecord("neutralEmEnergyFraction").toDouble() > m_jet_nEMEFrac_max)
            passID = false;
        // This variable is unnecessarily stored as a double! Not any more!!
        if (jet->getUserRecord("nconstituents").asUInt32() < m_jet_numConstituents_min)
            passID = false;
        // Additional cuts if |eta|<2.4:
        if (absEta < 2.4)
        {
            if (jet->getUserRecord("chargedHadronEnergyFraction").toDouble() < m_jet_cHadEFrac_min)
                passID = false;
            if (jet->getUserRecord("chargedEmEnergyFraction").toDouble() > m_jet_cEMEFrac_max)
                passID = false;
            // This variable is unnecessarily stored as a double!
            if (jet->getUserRecord("chargedMultiplicity").asUInt32() < m_jet_cMultiplicity_min)
                passID = false;
        }
    }

    jet->setUserRecord("isPF", false);
    if (passKin && passID)
    {
        // TODO: Update to use new variable "isPFJet" implemented in Skimmer that
        // should be available in 5XY skimmed samples!
        // If any of the three is false, we don't count the jet as PF.
        jet->setUserRecord("isPF", m_jet_isPF);
        return 0;
    }
    else if (passKin && !passID)
        return 1;
    else if (!passKin && passID)
        return 2;
    return 3;
}
