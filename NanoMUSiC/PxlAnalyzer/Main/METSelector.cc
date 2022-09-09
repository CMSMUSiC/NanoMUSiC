#include "METSelector.hh"

METSelector::METSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap)
    : // Add parent constructor
      ObjectSelector(cfg, globalOldNameMap, "MET", false),
      // general:
      // MET:
      m_met_use(cfg.GetItem<bool>("MET.use")), m_met_pt_min(cfg.GetItem<double>("MET.pt.min")),
      m_met_dphi_ele_min(cfg.GetItem<double>("MET.dPhi.Ele.min"))

{
}

METSelector::~METSelector()
{
}

int METSelector::passObjectSelection(pxl::Particle *met, double const metRho, const std::string &idType,
                                     const bool isSyst // use alternative kinematic cuts for syst
) const
{

    bool passKin = ObjectSelector::passKinematics(met, isSyst);
    bool passID = true;
    met->setUserRecord("usedID", idType);
    if (passKin && passID)
        return 0;
    else if (passKin && !passID)
        return 1;
    else if (!passKin && passID)
        return 2;
    return 4;
}
