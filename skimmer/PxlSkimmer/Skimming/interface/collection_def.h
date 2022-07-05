// -*- C++ -*-
// Copyright [2015] <RWTH Aachen, III. Phys. Inst. A>

#ifndef SKIMMING_INTERFACE_COLLECTION_DEF_H_
#define SKIMMING_INTERFACE_COLLECTION_DEF_H_

namespace pat {
class Jet;
}

template< class T > class Selector;

#include <string>
#include <vector>
#include <utility>

#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/PatCandidates/interface/Jet.h"


struct jet_def{
    std::string   name;
    edm::EDGetTokenT< reco::GenJetCollection > MCToken;
    edm::EDGetTokenT< std::vector< pat::Jet > > RecoToken;
    bool          isPF;
    bool          recoOnly;
    std::vector< std::pair< std::string, Selector<pat::Jet>* > >   IDs;
};

#endif  // SKIMMING_INTERFACE_COLLECTION_DEF_H_
