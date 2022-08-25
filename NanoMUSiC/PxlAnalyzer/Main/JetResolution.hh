#ifndef JETRESOLUTION
#define JETRESOLUTION

#include <string>

#include "TRandom3.h"

#include "Tools/MConfig.hh"
#include "BinnedMapping.hh"

// correction lib
#include "correction.h"

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
        double getResolution(double const jet_pt, double const jet_eta, double const rho) const;
        double getResolutionSF(double const jet_eta, int const updown) const;

        TRandom3 m_rand;

        std::unique_ptr<correction::CorrectionSet> m_resolution_correction_set;
        correction::Correction::Ref  m_resolution_scale_factor;
        correction::Correction::Ref  m_resolution;
};

#endif /*JETRESOLUTION*/
