#ifndef EleSelector_hh
#define EleSelector_hh


/*

This class contains all the muon selections

*/

#include <string>
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Main/EffectiveArea.hh"
#include "ObjectSelector.hh"


class EleSelector : public ObjectSelector {
public:
   // Constructor
   EleSelector(const Tools::MConfig &config, OldNameMapper *globalOldNameMap );
   // Destruktor
   ~EleSelector();
   //~ int passObjectSelection(pxl::Particle *object,
   int passObjectSelection(pxl::Particle *ele,
                                double const objectRho,
                                const std::string& idType,
                                const bool isSyst // use alternative kinematic cuts for syst
                                ) const;

private:
   // methods:
   bool passlooseCBID( pxl::Particle const *ele, double const elePt, double const eleAbsEta, bool const eleBarrel, bool const eleEndcap ) const;
   bool passtightCBID( pxl::Particle const *ele, double const elePt, double const eleAbsEta, bool const eleBarrel, bool const eleEndcap ) const;
   bool passlooseHEEPID( pxl::Particle const *ele, double const eleEt, bool const eleBarrel, bool const eleEndcap ) const;
   bool passtightHEEPID( pxl::Particle const *ele, double const eleEt, bool const eleBarrel, bool const eleEndcap ) const;
   bool passHEEP_Isolation( pxl::Particle const *ele, double const eleEt, bool const eleBarrel, bool const eleEndcap, double const eleRho) const;
   bool passCBID_Isolation( pxl::Particle const *ele, double const &eleRho , bool const &eleBarrel, bool const &eleEndcap ) const;


   // ID:
   //bool const m_ele_ID_use;
   //std::string const m_ele_ID_name;
   std::string const m_ele_id_type;
   double const  m_ele_id_ptswitch;

   // variables and constants:
   // Electrons:
   double const m_ele_pt_min;
   double const m_ele_eta_barrel_max;
   double const m_ele_eta_endcap_min;
   double const m_ele_eta_endcap_max;
   bool const   m_ele_invertIso;

   // CutBasedID:
   //bool const   m_ele_cbid_use;
   bool const   m_ele_cbid_usebool;
   std::string const m_ele_cbid_boolname;

   double const m_ele_cbid_lowEta_EoP_min; // lowEta: |eta| < 1.0
   double const m_ele_cbid_fBrem_min;
   bool const m_use_fBrem;

   double const m_ele_cbid_barrel_DEtaIn_max;
   double const m_ele_cbid_barrel_DPhiIn_max;
   double const m_ele_cbid_barrel_sigmaIetaIeta_max;
   double const m_ele_cbid_barrel_HoE_max;
   double const m_ele_cbid_barrel_Dxy_max;
   double const m_ele_cbid_barrel_HoEIterm_max;
   double const m_ele_cbid_barrel_HoEIIterm_max;
   double const m_ele_cbid_barrel_HoEIIIterm_max;
   double const m_ele_cbid_barrel_Iterm_PFIsoRel_max;
   double const m_ele_cbid_barrel_IIterm_PFIsoRel_max;
   double const m_ele_cbid_endcap_HoEIterm_max;
   double const m_ele_cbid_endcap_HoEIIterm_max;
   double const m_ele_cbid_endcap_HoEIIIterm_max;
   double const m_ele_cbid_endcap_Iterm_PFIsoRel_max;
   double const m_ele_cbid_endcap_IIterm_PFIsoRel_max;
   double const m_ele_cbid_barrel_Dz_max;
   double const m_ele_cbid_barrel_RelInvEpDiff_max;
   double const m_ele_cbid_barrel_PFIsoRel_max;
   int const    m_ele_cbid_barrel_NInnerLayerLostHits_max;
   double const m_ele_cbid_barrel_Conversion_reject;


   double const m_ele_cbid_endcap_DEtaIn_max;
   double const m_ele_cbid_endcap_DPhiIn_max;
   double const m_ele_cbid_endcap_sigmaIetaIeta_max;
   double const m_ele_cbid_endcap_HoE_max;
   double const m_ele_cbid_endcap_Dxy_max;
   double const m_ele_cbid_endcap_Dz_max;
   double const m_ele_cbid_endcap_RelInvEpDiff_max;
   double const m_ele_cbid_endcap_PFIsoRel_max;
   int const    m_ele_cbid_endcap_NInnerLayerLostHits_max;
   double const m_ele_cbid_endcap_Conversion_reject;

   // HEEP:
   //bool const   m_ele_heepid_use;
   bool const   m_ele_heepid_usebool;
   std::string const m_ele_heepid_boolname;
   bool const   m_ele_heepid_requireEcalDriven;

   double const m_ele_heepid_barrel_deltaEta_max;
   double const m_ele_heepid_barrel_deltaPhi_max;
   double const m_ele_heepid_barrel_HoEM_max;
   double const m_ele_heepid_barrel_HoEM_slope;
   double const m_ele_heepid_barrel_trackiso_max;
   double const m_ele_heepid_barrel_HcalD1_offset;
   double const m_ele_heepid_barrel_HcalD1_slope;
   double const m_ele_heepid_barrel_HcalD1_rhoSlope;
   double const m_ele_heepid_barrel_TrkIso_rhoSlope;
   int const    m_ele_heepid_barrel_NInnerLayerLostHits_max;
   double const m_ele_heepid_barrel_dxy_max;
   double const m_ele_heepid_barrel_e1x5_min;
   double const m_ele_heepid_barrel_e2x5_min;

   double const m_ele_heepid_endcap_deltaEta_max;
   double const m_ele_heepid_endcap_deltaPhi_max;
   double const m_ele_heepid_endcap_HoEM_max;
   double const m_ele_heepid_endcap_HoEM_slope;
   double const m_ele_heepid_endcap_trackiso_max;
   double const m_ele_heepid_endcap_HcalD1_offset;
   double const m_ele_heepid_endcap_HcalD1_slope;
   double const m_ele_heepid_endcap_HcalD1_rhoSlope;
   double const m_ele_heepid_endcap_TrkIso_rhoSlope;
   int const    m_ele_heepid_endcap_NInnerLayerLostHits_max;
   double const m_ele_heepid_endcap_dxy_max;
   double const m_ele_heepid_endcap_sigmaIetaIeta_max;

   EffectiveArea const m_ele_EA;
   bool mutable        m_useAlternative;
   std::map<std::string,std::string> mutable m_alternativeUserVariables;
};
#endif
