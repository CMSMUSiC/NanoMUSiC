#ifndef GENSELECTOR
#define GENSELECTOR

// Class to apply cuts on generator level, which can be given in a config file.
// Several different criteria can be cut on:
//   - generator binning, e.g. pT-hat (pT of the hard interaction)
//   - mass of the resonance particle
//   - pT of the resonance particle


#include <vector>

#include "Tools/MConfig.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

class GenSelector {
public:
   typedef std::vector< pxl::Particle* > pxlParticles;

   GenSelector( Tools::MConfig const &cfg );
   ~GenSelector() {}

   // Apply all Generator Cuts.
   bool passGeneratorCuts( pxl::EventView  *EvtView ) const;

private:
   // Apply cut on generator binning (usually pT-hat).
   bool passBinningCuts( pxl::EventView const *EvtView ) const;
   // Apply cuts on invariant mass of resonance particle.
   bool passMassCuts( std::vector< pxl::Particle* > genParts ) const;
   // Apply cut on HT in the event.
   bool passHtCuts( pxl::EventView const *EvtView ) const;
   // Apply cuts on transverse momentum of resonance particle.
   bool passTransverseMomentumCuts( std::vector< pxl::Particle* > genParts ) const;
   // Check if the given number of particles is sensible.
   bool CheckNumberOfParticles( pxlParticles const &s3_particlesSelected ) const;
   // Check if a gamma is found to close to a parton / lepton for Gjets / ZJets cleaning
   bool passGammaCuts( pxl::EventView const *EvtView,	
					   std::vector< pxl::Particle* > genParts,	
					   std::vector< pxl::Particle* > genJets ) const;
   //For DY vs Zy cleaning
   bool passDYGammaCuts( pxl::EventView const *EvtView,	
					   std::vector< pxl::Particle* > genParts,	
					   std::vector< pxl::Particle* > genJets ) const;
   //For Zy vs DY cleaning
   bool passZyGammaCuts( pxl::EventView const *EvtView,	
					   std::vector< pxl::Particle* > genParts,	
					   std::vector< pxl::Particle* > genJets ) const;

   std::vector< pxl::Particle* > getGenParticles( pxl::EventView* GenEvtView, std::string gen_label )const ;
   pxl::LorentzVector getGenCutObject( std::vector< pxl::Particle* > genParts, const std::string& type) const;

   std::string m_gen_label;	
   std::string m_gen_jet_label;

   // for cuts on single particle, what to do if particle not found
   bool m_ignore_missing_particle;

   // Cut on generator binning.
   double const m_binningValue_max;

   // Cuts on generator level HT of the event.
   double const m_ht_min;
   double const m_ht_max;

   // Cuts on invariant mass of resonance particle.
   double const m_mass_min;
   double const m_mass_max;
   bool m_mass_use_single;
   bool m_mass_use_double;
   std::vector< std::string> m_mass_single_types;
   std::vector< std::string> m_mass_double_types_first;
   std::vector< std::string> m_mass_double_types_second;

   // Cuts on transverse momentum of resonance particle.
   double const m_pt_min;
   double const m_pt_max;
   bool m_pt_use_single;
   bool m_pt_use_double;
   std::vector< std::string> m_pt_single_types;
   std::vector< std::string> m_pt_double_types_first;
   std::vector< std::string> m_pt_double_types_second;
	
   bool const m_gamma_use;
   bool const m_gammaDY_use;
   bool const m_gammaZy_use;
   std::string const m_gamma_cutmode;
   std::vector< int > const m_gamma_statuses;
   double const m_gamma_dR_max;
   double const m_gamma_genPtCut;
   std::vector< int > const m_gamma_gen_partner_id;
   bool m_has_gamma_quark_partner;
   

};

#endif /*GENSELECTOR*/
