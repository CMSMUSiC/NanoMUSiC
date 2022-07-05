#ifndef MultiIsolation_h
#define MultiIsolation_h
// stl includes
#include <iostream>
#include <tuple>
#include <vector>
// cmssw includes
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "PxlSkimmer/Skimming/interface/IsolationComputer.h"
#include <DataFormats/PatCandidates/interface/Muon.h>
#include <DataFormats/PatCandidates/interface/Electron.h>
#include <DataFormats/PatCandidates/interface/Jet.h>
// root includes
#include "TLorentzVector.h"

class MultiIsolation
{
public:
  MultiIsolation(const edm::ParameterSet & cfg, edm::ConsumesCollector && iC):
    m_pfCandToken(iC.consumes< std::vector<pat::PackedCandidate> >(cfg.getParameter<edm::InputTag>("candSource"))),
    m_rhoToken(iC.consumes<double>(cfg.getParameter<edm::InputTag>("rhoSource"))),
    isoComp_(heppy::IsolationComputer()),
    m_jetToken(iC.consumes< std::vector<pat::Jet> >(cfg.getParameter<edm::InputTag>("jetCollection"))) {}

  const void init(const edm::Event &ev);
  const double getAbsMiniIso(const pat::Muon &muon) const;
  const double getAbsMiniIso(const pat::Electron &electron) const;
  std::tuple<double, double, double, double, double, double, double, double> getPtRatioPtRel(const pat::Muon &muon) const;

private:
  const double getMiniIsoConeSize(const double pt) const;
  const double getEffArea(const pat::Muon &muon) const;
  const double getEffArea(const pat::Electron &electron) const;
  const double getPtRel(TLorentzVector &lepton, TLorentzVector &jet) const;
  void rescaleTLorentzVector(TLorentzVector& vector, double scaleFactor, bool massConst) const;

  //  pfCandidates used for miniIsolation
  edm::EDGetTokenT< std::vector<pat::PackedCandidate> > m_pfCandToken;
  // rho values used for conIsolation
  edm::EDGetTokenT<double> m_rhoToken;
  double rho_;
  // isolation computer used for miniIsolation
  heppy::IsolationComputer isoComp_;
  // jets used for multiIsolation
  edm::EDGetTokenT< std::vector<pat::Jet> > m_jetToken;
  std::vector< pat::Jet > recJets;
};

#endif

