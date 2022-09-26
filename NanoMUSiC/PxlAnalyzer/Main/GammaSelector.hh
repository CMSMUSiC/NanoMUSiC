#ifndef GammaSelector_hh
#define GammaSelector_hh

#include "EffectiveArea.hh"
#include "ObjectSelector.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Tools/MConfig.hh"
#include <map>
#include <string>

class GammaSelector : public ObjectSelector
{
  public:
    GammaSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap);
    ~GammaSelector();
    int passObjectSelection(pxl::Particle *gam, double const gamRho, const std::string &idType,
                            const bool isSyst // use alternative kinematic cuts for syst
    ) const;

  private:
    // member functions
    bool passCBID(pxl::Particle const *gam) const;
    bool passMVAID(pxl::Particle const *gam) const;

    //     bool passCutBased2012(pxl::Particle const *gam) const;
    bool passPhys14Loose(pxl::Particle const *gam) const;
    bool passPhys14Medium(pxl::Particle const *gam) const;
    bool passPhys14Tight(pxl::Particle const *gam) const;
    bool passSpring15Loose(pxl::Particle const *gam) const;
    bool passSpring15Medium(pxl::Particle const *gam) const;
    bool passSpring15Tight(pxl::Particle const *gam) const;
    bool passSpring15Loose25nsBool(pxl::Particle const *gam) const;
    bool passSpring15Medium25nsBool(pxl::Particle const *gam) const;
    bool passSpring15Tight25nsBool(pxl::Particle const *gam) const;
    bool passMVAIDBool(pxl::Particle const *gam) const;

    // member variables

    bool const m_gam_useConverted;
    bool const m_gam_useElectronVeto;
    bool const m_gam_usePixelSeed;
    bool const m_gam_rejectOutOfTime;
    double const m_gam_corrFactor_max;

    double const m_gam_barrel_sigmaIetaIeta_min;
    double const m_gam_barrel_sigmaIetaIeta_max;
    double const m_gam_endcap_sigmaIetaIeta_max;

    double const m_gam_SpikeCleaning;
    // CutBasedID
    EffectiveArea const m_gam_EA;

    // ID:
    std::string const m_gam_ID_Type;

    mutable bool m_idErrReported;

    bool m_gam_cb_use_bool;
    std::string m_gam_cb_boolname;
    double m_gam_barrel_HoEm_max;
    double m_gam_barrel_PFIsoChargedHadron_max;
    double m_gam_barrel_PFIsoNeutralHadron_offset;
    double m_gam_barrel_PFIsoNeutralHadron_linscale;
    double m_gam_barrel_PFIsoNeutralHadron_quadscale;
    double m_gam_barrel_PFIsoPhoton_offset;
    double m_gam_barrel_PFIsoPhoton_linscale;
    double m_gam_endcap_HoEm_max;
    double m_gam_endcap_PFIsoChargedHadron_max;
    double m_gam_endcap_PFIsoNeutralHadron_offset;
    double m_gam_endcap_PFIsoNeutralHadron_linscale;
    double m_gam_endcap_PFIsoNeutralHadron_quadscale;
    double m_gam_endcap_PFIsoPhoton_offset;
    double m_gam_endcap_PFIsoPhoton_linscale;

    bool m_gam_mva_use_bool;
    std::string m_gam_mva_boolname;
    double m_gam_barrel_mva_min;
    double m_gam_endcap_mva_min;
};
#endif
