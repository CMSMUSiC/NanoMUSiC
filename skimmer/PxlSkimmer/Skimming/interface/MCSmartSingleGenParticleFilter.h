// -*- C++ -*-
// Copyright [2015] <RWTH Aachen, III. Phys. Inst. A>

#ifndef SKIMMING_INTERFACE_MCSMARTSINGLEGENPARTICLEFILTER_H_
#define SKIMMING_INTERFACE_MCSMARTSINGLEGENPARTICLEFILTER_H_

//
// Filter based on the CMSSW "MCSmartSingleParticleFilter".
//
// This module filters events based on Pythia particleID, the pt, eta,
// production vertex. In contrast to MCSmartSingleParticleFilter it takes the
// reco::GenParticle information rather than the HepMC::GenEvent information.
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
// class declaration
//

class MCSmartSingleGenParticleFilter : public edm::EDFilter {
  public:
    explicit MCSmartSingleGenParticleFilter(const edm::ParameterSet&);
    ~MCSmartSingleGenParticleFilter();

    virtual bool filter(edm::Event&, const edm::EventSetup&);

  private:
    // ----------member data ---------------------------
    edm::InputTag label_;
    edm::EDGetTokenT< std::vector<reco::GenParticle> > token_;
    std::vector< int > particleID;
    std::vector< double > pMin;
    std::vector< double > ptMin;
    std::vector< double > etaMin;
    std::vector< double > etaMax;
    std::vector< int > status;
    std::vector< double > decayRadiusMin;
    std::vector< double > decayRadiusMax;
    std::vector< double > decayZMin;
    std::vector< double > decayZMax;
};

#endif  // SKIMMING_INTERFACE_MCSMARTSINGLEGENPARTICLEFILTER_H_

