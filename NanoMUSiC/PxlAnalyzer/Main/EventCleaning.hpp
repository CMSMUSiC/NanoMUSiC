#ifndef EVENTCLEANING
#define EVENTCLEANING

#include <functional>
#include <list>
#include <map>
#include <string>
#include <vector>

// Class to check for duplicate reconstructed objects and to decide for only one of them.
// Must be applied AFTER final physics object selection and BEFORE check of event topology and redoing the gen-rec
// matching. The actual pxl::Particles are removed from the containers and from the EventView. More information on Event
// Cleaning (please read): https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATCrossCleaning

namespace pxl
{
class Particle;
}

namespace Tools
{
class MConfig;
}

class EventCleaning
{
  public:
    EventCleaning(Tools::MConfig const &cfg);
    ~EventCleaning()
    {
    }

    // Main function to be called from outside with given set of particles.
    void cleanEvent(std::map<std::string, std::vector<pxl::Particle *>> &particleMap, bool const isRec) const;

  private:
    // Remove object pointed at by iterator from the vector.
    // Also remove the object from the EventView.
    // The iterator is decremented, so it is still valid.
    std::vector<pxl::Particle *>::iterator removeParticle(std::vector<pxl::Particle *> &particles,
                                                          std::vector<pxl::Particle *>::iterator &particle) const;

    // Clean Gammas from Eles (uses seed id)
    void removeGammaEleOverlap(std::vector<pxl::Particle *> &gams, std::vector<pxl::Particle *> const &eles,
                               bool const isRec) const;

    // Take two particle collections and remove "duplicates" from the first one.
    void removeOverlappingParticles(std::vector<pxl::Particle *> &toBeCleanedCollection,
                                    std::vector<pxl::Particle *> const &inputCollection, double const DeltaR_max) const;

    // Remove duplicate muons (ghosts).
    void cleanMuos(std::vector<pxl::Particle *> &muons, bool const isRec) const;

    // Remove electron duplicates, clean muons from electrons.
    void cleanEles(std::vector<pxl::Particle *> &eles, bool const isRec) const;

    // Remove gamma duplicates, clean electrons and muons from gammas.
    void cleanGams(std::vector<pxl::Particle *> &gams, bool const isRec) const;

    // Clean electrons, muons and gammas from taus.
    // Since taus are treated similarly to jets, no tau-tau cleaning atm.

    // In Particle Flow "everything" is potentially a jet, so check against all
    // objects and remove every jet that is in close proximity to anything.
    // ATTENTION: jet-jet not checked atm., jets should be non-overlapping by
    // design in kt algorithm and for anti-kt it is OK.
    // For details on anti-kt, see also:
    // http://iopscience.iop.org/1126-6708/2008/04/063

    // Checks mu-mu overlap and returns -1,0,1 to indicate the removal of the
    // first, none or the second particle, respectively.
    // Chooses the mu with lower chi2/ndf.
    // For Gen: Choose the mu with higher pT.
    int checkMuonOverlap(pxl::Particle const *paI, pxl::Particle const *paJ) const;

    // Checks ele-ele overlap and returns -1,0,1 to indicate the removal of the
    // first, none or the second particle, respectively.
    // Chooses ele with higher energy.
    int checkElectronOverlap(pxl::Particle const *paI, pxl::Particle const *paJ, bool const isRec) const;

    // Checks gamma-gamma overlap and returns -1,0,1 to indicate the removal of
    // the first, none or the second particle, respectively.
    // Chooses gamma with higher energy.
    int checkGammaOverlap(pxl::Particle const *paI, pxl::Particle const *paJ, bool const isRec) const;

    // Check which particle has the lower chi2/ndf and return -1,0,1 to indicate
    // the removal of the first, none or the second particle, respectively.
    int checkNormChi2(pxl::Particle const *p1, pxl::Particle const *p2) const;

    // Alternative to simply checking for smallest chi2/ndof.
    int checkProbability(pxl::Particle const *p1, pxl::Particle const *p2) const;

    // Returns true if particle2 is within DeltaR_max of particle1.
    bool checkParticleOverlap(pxl::Particle const *p1, pxl::Particle const *p2, double const DeltaR_max) const;

    // Returns true if seedID is same for both particles.
    bool checkSeedOverlap(pxl::Particle const *p1, pxl::Particle const *p2) const;

    // Returns true if TrackerP is the same for both particles.
    bool checkTrack(pxl::Particle const *p1, pxl::Particle const *p2) const;

    // Parameters controlling overlaps.
    std::vector<std::string> m_order;
    const std::vector<std::string> m_self_cleaning;
    std::map<std::string, bool> m_particleUseMap;

    std::list<std::function<void(std::map<std::string, std::vector<pxl::Particle *>> &)>> m_splitting_funcs;
    std::list<std::function<void(std::map<std::string, std::vector<pxl::Particle *>> &)>> m_merge_funcs;

    const std::map<std::string, std::function<void(const EventCleaning &, std::vector<pxl::Particle *> &, const bool)>>
        m_self_cleaning_functions;

    const std::map<std::string, double> m_deltaR_map;
};

#endif /*EVENTCLEANING*/
