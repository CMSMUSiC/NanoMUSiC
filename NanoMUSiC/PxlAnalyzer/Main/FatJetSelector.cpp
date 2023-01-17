#include "FatJetSelector.hpp"

FatJetSelector::FatJetSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap)
    : // Add parent constructor
      JetSelector(cfg, globalOldNameMap, "FatJet")
{
}
