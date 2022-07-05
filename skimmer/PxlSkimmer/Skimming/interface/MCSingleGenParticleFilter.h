// -*- C++ -*-
// Copyright [2015] <RWTH Aachen, III. Phys. Inst. A>

#ifndef SKIMMING_INTERFACE_MCSINGLEGENPARTICLEFILTER_H_
#define SKIMMING_INTERFACE_MCSINGLEGENPARTICLEFILTER_H_

//
// Filter based on the CMSSW "MCSingleParticleFilter".
//
// This module filters events based on Pythia particleID, the pt, eta. In
// contrast to MCSingleParticleFilter it takes the reco::GenParticle
// information rather than the HepMC::GenEvent information.
//
// Inherits from EDFilter
//

namespace edm {
class Event;
class EventSetup;
class ParameterSet;
}

namespace reco {
class Candidate;
class GenParticle;
class Vertex;
}

#include <vector>

#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/Utilities/interface/InputTag.h"

//
// class decleration
//

class MCSingleGenParticleFilter : public edm::EDFilter {
  public:
    explicit MCSingleGenParticleFilter(const edm::ParameterSet&);
    ~MCSingleGenParticleFilter();


    virtual bool filter(edm::Event&, const edm::EventSetup&);
  private:
    // ----------member data ---------------------------
    edm::InputTag label_;
    edm::EDGetTokenT< std::vector<reco::GenParticle> > token_;
    std::vector< int > particleID;
    std::vector< double > ptMin;
    std::vector< double > etaMin;
    std::vector< double > etaMax;
    std::vector< int > status;
};

#endif  // SKIMMING_INTERFACE_MCSINGLEGENPARTICLEFILTER_H_
