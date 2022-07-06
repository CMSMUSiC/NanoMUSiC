#ifndef JetSelector_hh
#define JetSelector_hh

#include <string>
#include <map>
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Tools/MConfig.hh"
#include "ObjectSelector.hh"

class JetSelector : public ObjectSelector {
    public:
        JetSelector( const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap, const std::string& name="Jet" );
        ~JetSelector();
    int passObjectSelection( pxl::Particle *jet,
                   double const jetRho,
                   const std::string& idType,
                   const bool isSyst // use alternative kinematic cuts for syst
                   ) const;

    protected:
        // Jets:
        bool const          m_jet_use;
        double const        m_jet_pt_min;
        bool const          m_jet_isPF;
        bool const          m_jet_ID_use;
        std::string const m_jet_ID_type;
        double const        m_jet_gen_hadOverEm_min;
        double const        m_jet_gen_hadEFrac_min;

        // In case we do the ID on our own:
        double const m_jet_nHadEFrac_max;
        double const m_jet_nEMEFrac_max;
        unsigned long const     m_jet_numConstituents_min;
        double const m_jet_cHadEFrac_min;
        double const m_jet_cEMEFrac_max;
        unsigned long const     m_jet_cMultiplicity_min;
};
#endif
