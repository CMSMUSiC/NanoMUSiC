#ifndef ParticleMatcher_hh
#define ParticleMatcher_hh

/*
Class which perform the matching between generator level particle with
reconstructed particles. The matching is based on a delta R algo. Each gen
particle points to the best matching rec particle and vice versa. If the best
matching particle has a distance large than the given limits for DeltaR, DeltaPtoPt or DeltaCharge the particle is
declared to have no match. For unmatched particles Match UserRecord is set to -1.
*/

#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"
#include "TMatrixT.h"
#include "Tools/PXL/JetSubtypeCriterion.hpp"
#include <string>
#include <vector>

#include "Main/GenRecNameMap.hpp"

namespace Tools
{
class MConfig;
}

class ParticleMatcher
{
  public:
    // Construktor
    ParticleMatcher(Tools::MConfig const &cfg, int const debug = 0);
    // Destructor
    ~ParticleMatcher()
    {
        ;
    };
    // Match method
    void matchObjects(pxl::EventView const *GenEvtView, pxl::EventView const *RecEvtView,
                      std::string const &defaultLinkName = "priv-gen-rec", bool const CustomMatch = false) const;
    void makeMatching(std::vector<pxl::Particle *> &gen_particles, std::vector<pxl::Particle *> &rec_particles,
                      const std::string &Match = "Match", const std::string &hctaM = "hctaM",
                      const std::string &linkname = "priv-gen-rec") const;

  private:
    // Some helper methods
    int SmallestRowElement(TMatrixT<double> *matrixDR, TMatrixT<double> *matrixDp, TMatrixT<double> *matrixDC,
                           const unsigned int &row, const double &DeltaRMatching, const double &DeltaChargeMatching,
                           const double &DeltaPtoPtMatching) const;
    int SmallestColumnElement(TMatrixT<double> *matrixDR, TMatrixT<double> *matrixDp, TMatrixT<double> *matrixDC,
                              const unsigned int &col, const double &DeltaRMatching, const double &DeltaChargeMatching,
                              const double &DeltaPtoPtMatching) const;
    // variable to define dR which decides matching
    double const m_DeltaR_Particles;
    double const m_DeltaR_MET;
    double const m_DeltaPtoPt;
    double const m_DeltaCharge;

    bool const m_jet_bJets_use;
    std::string const m_jet_bJets_algo;
    std::string const m_jet_bJets_gen_label;

    GenRecNameMap const m_gen_rec_map;

    int const m_debug;
};
#endif
