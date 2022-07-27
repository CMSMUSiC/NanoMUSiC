#ifndef TauSelector_hh
#define TauSelector_hh


/*

This class contains all the muon selections

*/
#include <string>
#include "Tools/MConfig.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "ObjectSelector.hh"


class TauSelector : public ObjectSelector{
public:
    // Constructor
    TauSelector(const Tools::MConfig &config, OldNameMapper *globalOldNameMap );
    // Destruktor
    ~TauSelector();
    int passObjectSelection( pxl::Particle *tau,
                                         double const tauRho,
                                         const std::string& idType,
                                         const bool isSyst // use alternative kinematic cuts for syst
                                         ) const;
    void tagJetsAsTau( std::vector< pxl::Particle* > &taus, std::vector< pxl::Particle* > &jets ) const;

private:

    // Discriminators:
    std::vector< std::string > const m_tau_discriminators;
    std::vector< std::string > const m_tau_syst_discriminators;

    double DeltaR(pxl::Particle* part_j, pxl::Particle* part_i) const;
};
#endif
