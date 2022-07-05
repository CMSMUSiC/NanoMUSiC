// -*- C++ -*-
// Copyright [2015] <RWTH Aachen, III. Phys. Inst. A>

#include "PxlSkimmer/Skimming/interface/MCSingleGenParticleFilter.h"

#include <iostream>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"


MCSingleGenParticleFilter::MCSingleGenParticleFilter(const edm::ParameterSet& iConfig) :
    label_(iConfig.getParameter< edm::InputTag >("genParSource")) {
		
    token_= consumes< std::vector<reco::GenParticle> >(label_);	
    
    // here do whatever other initialization is needed
    std::vector< int > defpid;
    defpid.push_back(0);
    particleID = iConfig.getUntrackedParameter< std::vector< int > >("ParticleID", defpid);

    std::vector< double > defptmin;
    defptmin.push_back(0.);
    ptMin = iConfig.getUntrackedParameter< std::vector< double > >("MinPt", defptmin);

    std::vector< double > defetamin;
    defetamin.push_back(-10.);
    etaMin = iConfig.getUntrackedParameter< std::vector< double > >("MinEta", defetamin);

    std::vector< double > defetamax;
    defetamax.push_back(10.);
    etaMax = iConfig.getUntrackedParameter< std::vector< double > >("MaxEta", defetamax);

    std::vector< int > defstat;
    defstat.push_back(0);
    status = iConfig.getUntrackedParameter< std::vector< int > >("Status", defstat);


    // check for same size
    if ((ptMin.size() > 1 &&  particleID.size() != ptMin.size())
         ||  (etaMin.size() > 1 && particleID.size() != etaMin.size())
         ||  (etaMax.size() > 1 && particleID.size() != etaMax.size())
         ||  (status.size() > 1 && particleID.size() != status.size())) {
        std::cout << "WARNING: MCPROCESSFILTER : size of MinPthat and/or MaxPthat not matching with ProcessID size!!" << std::endl;
    }

    // if ptMin size smaller than particleID , fill up further with defaults
    if (particleID.size() > ptMin.size()) {
        std::vector< double > defptmin2;
        for (unsigned int i = 0; i < particleID.size(); i++) { defptmin2.push_back(0.); }
        ptMin = defptmin2;
    }

    // if etaMin size smaller than particleID , fill up further with defaults
    if (particleID.size() > etaMin.size()) {
        std::vector< double > defetamin2;
        for (unsigned int i = 0; i < particleID.size(); i++) { defetamin2.push_back(-10.); }
        etaMin = defetamin2;
    }

    // if etaMax size smaller than particleID , fill up further with defaults
    if (particleID.size() > etaMax.size()) {
        std::vector< double > defetamax2;
        for (unsigned int i = 0; i < particleID.size(); i++) { defetamax2.push_back(10.); }
        etaMax = defetamax2;
    }

    // if status size smaller than particleID , fill up further with defaults
    if (particleID.size() > status.size()) {
        std::vector< int > defstat2;
        for (unsigned int i = 0; i < particleID.size(); i++) { defstat2.push_back(0); }
        status = defstat2;
    }
}


MCSingleGenParticleFilter::~MCSingleGenParticleFilter() {
}


// ------------ method called to skim the data  ------------
bool MCSingleGenParticleFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    bool accepted = false;
    edm::Handle< reco::GenParticleCollection > genParticleHandel;
    iEvent.getByToken(token_, genParticleHandel);

    for (reco::GenParticleCollection::const_iterator pa = genParticleHandel->begin(); pa != genParticleHandel->end(); ++pa) {
        const reco::GenParticle* p = (const reco::GenParticle*) &(*pa);
        const reco::Candidate::LorentzVector p4 = p->p4();

        for (unsigned int i = 0; i < particleID.size(); i++) {
            if (p->pdgId() == particleID.at(i) || particleID.at(i) == 0) {
                if (p4.Pt() > ptMin.at(i) && p4.Eta() > etaMin.at(i) && p4.Eta() < etaMax.at(i)
                     && (p->status() == status.at(i) || status.at(i) == 0)) {
                    accepted = true;
                }
            }
        }
    }

    return accepted;
}

#include "FWCore/Framework/interface/MakerMacros.h"

// define this as a plug-in
DEFINE_FWK_MODULE(MCSingleGenParticleFilter);

