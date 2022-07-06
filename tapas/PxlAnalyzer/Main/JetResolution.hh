#ifndef JETRESOLUTION
#define JETRESOLUTION

#ifndef STANDALONE
#define STANDALONE
#endif

#include "TRandom3.h"

#include "Tools/MConfig.hh"
#include "BinnedMapping.hh"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
//#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/JetResolutionObject.h"
#include "JetMETCorrections/Modules/interface/JetResolution.h"
#pragma GCC diagnostic pop


namespace pxl {
    class Particle;
}

class JetResolution{
    public:
        JetResolution( Tools::MConfig const &config, std::string const& jetType );
        ~JetResolution() {}

        double getJetResolutionCorrFactor( pxl::Particle const *recJet,
                                            pxl::Particle const *genJet,
                                            double const npv,
                                            double const rho,
                                            int const updown
                                            );
        pxl::Particle* matchGenJet(const pxl::Particle* rec_jet, const std::vector<pxl::Particle*>& gen_jets, const double radius, const double npv) const;

    private:
        double getResolution( double const pt, double const eta, double const rho, double const npv ) const;
        double getResolutionSF( double const pt, double const eta, double const rho, double const npv, int const updown ) const;

        TRandom3 m_rand;

        JME::JetResolution m_resolutionPt;
        JME::JetResolutionScaleFactor m_resolutionPt_sf;
};

#endif /*JETRESOLUTION*/
