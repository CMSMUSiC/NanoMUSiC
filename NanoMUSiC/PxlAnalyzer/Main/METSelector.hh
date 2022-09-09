#ifndef METSelector_hh
#define METSelector_hh

#include "ObjectSelector.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Tools/MConfig.hh"
#include <map>
#include <string>

class METSelector : public ObjectSelector
{
  public:
    METSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap);
    ~METSelector();
    int passObjectSelection(pxl::Particle *met, double const metRho, const std::string &idType,
                            const bool isSyst // use alternative kinematic cuts for syst
    ) const;

  private:
    // MET:
    bool const m_met_use;
    double const m_met_pt_min;
    double const m_met_dphi_ele_min;
};
#endif
