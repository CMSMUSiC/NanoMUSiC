#ifndef FatJetSelector_hh
#define FatJetSelector_hh

#include "JetSelector.hpp"

class FatJetSelector : public JetSelector
{
  public:
    FatJetSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap);
};
#endif
