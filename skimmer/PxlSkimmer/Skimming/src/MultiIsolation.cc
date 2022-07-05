#include "PxlSkimmer/Skimming/interface/MultiIsolation.h"

const void MultiIsolation::init(const edm::Event &ev ) {
  //  pfCandidates used for miniIsolation
  edm::Handle< pat::PackedCandidateCollection > pfCandidates;
  ev.getByToken(m_pfCandToken, pfCandidates);
  isoComp_.setPackedCandidates(*pfCandidates);
  // rho values used for PU-correction in miniIsolation
  edm::Handle<double> m_rhoHandle;
  ev.getByToken(m_rhoToken, m_rhoHandle);
  rho_ = *(m_rhoHandle.product());
  // jets used for multiIsolation
  edm::Handle< std::vector< pat::Jet > > jetHandle;
  ev.getByToken(m_jetToken, jetHandle);
  recJets = *jetHandle;
}

const double MultiIsolation::getAbsMiniIso(const pat::Muon &muon) const {
  double miniIsoConeSize = getMiniIsoConeSize(muon.pt());
  double miniAbsIsoCharged = isoComp_.chargedAbsIso(muon, miniIsoConeSize, 0.0001, 0.0);
  double miniAbsIsoNeutral = isoComp_.neutralAbsIsoRaw(muon, miniIsoConeSize, 0.01, 0.5);
  double effArea = getEffArea(muon);
  miniAbsIsoNeutral = std::max(0.0, miniAbsIsoNeutral - rho_ * effArea * std::pow(miniIsoConeSize/0.3, 2.));
  return miniAbsIsoCharged + miniAbsIsoNeutral;
}

const double MultiIsolation::getAbsMiniIso(const pat::Electron &electron) const {
  double miniIsoConeSize = getMiniIsoConeSize(electron.pt());
  double miniAbsIsoCharged = 0.;
  double miniAbsIsoPho = 0.;
  if (std::abs(electron.eta()) < 1.479) {
    miniAbsIsoCharged = isoComp_.chargedAbsIso(electron, miniIsoConeSize, 0.0, 0.0, isoComp_.selfVetoNone);
    miniAbsIsoPho = isoComp_.photonAbsIsoRaw(electron, miniIsoConeSize, 0.0, 0.0, isoComp_.selfVetoNone);
  } else {
    miniAbsIsoCharged = isoComp_.chargedAbsIso(electron, miniIsoConeSize, 0.015, 0.0, isoComp_.selfVetoNone);
    miniAbsIsoPho = isoComp_.photonAbsIsoRaw(electron, miniIsoConeSize, 0.08, 0.0, isoComp_.selfVetoNone);
  }
  double miniAbsIsoNHad = isoComp_.neutralHadAbsIsoRaw(electron, miniIsoConeSize, 0.0, 0.0, isoComp_.selfVetoNone);
  double miniAbsIsoNeutral = miniAbsIsoPho + miniAbsIsoNHad;
  double effArea = getEffArea(electron);
  miniAbsIsoNeutral = std::max(0.0, miniAbsIsoNeutral - rho_ * effArea * std::pow(miniIsoConeSize/0.3, 2.));
  return miniAbsIsoCharged + miniAbsIsoNeutral;
}

const double MultiIsolation::getMiniIsoConeSize(const double pt) const {
  return 10.0/(std::min(std::max(pt, 50.),200.));
}

const double MultiIsolation::getEffArea(const pat::Muon &muon) const {
  // helper function to get effective areas for muons
  double etaAbs = std::fabs(muon.eta());
  // Spring15 as recommended for Moriond 17
  // https://twiki.cern.ch/twiki/bin/view/CMS/SUSLeptonSF?rev=169
  double AEff = 0.0735;
  if (etaAbs > 0.8 && etaAbs <= 1.3) AEff =  0.0619;
  if (etaAbs > 1.3 && etaAbs <= 2.0) AEff =  0.0465;
  if (etaAbs > 2.0 && etaAbs <= 2.2) AEff = 0.0433;
  if (etaAbs > 2.2) AEff = 0.0577;
  return AEff;
}

const double MultiIsolation::getEffArea(const pat::Electron &electron) const {
  // helper function to get effective areas for electrons
  double etaAbs = std::fabs(electron.eta());
  // Spring15 as recommended for Moriond 17
  // https://twiki.cern.ch/twiki/bin/view/CMS/SUSLeptonSF?rev=169
  double AEff = 0.1752;
  if (etaAbs > 1.0 && etaAbs <= 1.479) AEff = 0.1862;
  if (etaAbs > 1.479 && etaAbs <= 2.0) AEff = 0.1411;
  if (etaAbs > 2.0 && etaAbs <= 2.2) AEff = 0.1534;
  if (etaAbs > 2.2 && etaAbs <= 2.3) AEff = 0.1903;
  if (etaAbs > 2.3 && etaAbs <= 2.4) AEff = 0.2243;
  if (etaAbs > 2.4) AEff = 0.2687;
  return AEff;
}


std::tuple<double, double, double, double, double, double, double, double> MultiIsolation::getPtRatioPtRel(const pat::Muon &muon) const {
  // initialize return variables with default values
  double ptRatio = 1.;
  double ptRel = 0.;
  double lepawareJetPt = muon.pt();
  double lepawareJetEta = muon.eta();
  double lepawareJetPhi = muon.phi();
  double lepawareJetM = muon.mass();
  double lepawareJetJECToRawFactor = 1.;
  double lepawareJetJECToL1Factor = 1.;

  // CASE 1:
  // - there are no jets
  // - use muon as lepaware jet
  if (recJets.size() == 0) {
    return std::make_tuple(ptRatio, ptRel, lepawareJetPt, lepawareJetEta, lepawareJetPhi, lepawareJetM, lepawareJetJECToRawFactor, lepawareJetJECToL1Factor);
  }
  // find closest jet
  pat::Jet closestJet;
  double dr2min = 999999.;
  for (auto jet : recJets) {
    if (jet.pt() < 10.) continue;
    double dr2 = deltaR2(jet, muon);
    if (dr2 < dr2min) {
      closestJet = jet;
      dr2min = dr2;
    }
  }
  if (dr2min < 0.4*0.4) {
    // CASE 2:
    // - dR(muon,closestJet) < 0.4
    // - use closestJet as lepaware jet

    TLorentzVector muonVec = TLorentzVector(muon.px(),
                                            muon.py(),
                                            muon.pz(),
                                            muon.energy());
    TLorentzVector jetVec = TLorentzVector(closestJet.px(),
                                           closestJet.py(),
                                           closestJet.pz(),
                                           closestJet.energy());
    double jecFactorFullJECToRaw = closestJet.jecFactor("Uncorrected");

    if (((jetVec * jecFactorFullJECToRaw - muonVec)).Rho() < 0.0001) {
      // CASE 2a:
      // - jet contains ~only muon
      // - switch to muon as lepaware jet
      return std::make_tuple(ptRatio, ptRel, lepawareJetPt, lepawareJetEta, lepawareJetPhi, lepawareJetM, lepawareJetJECToRawFactor, lepawareJetJECToL1Factor);
    }

    // CASE 2b:
    // - there is a nearby jet which is not ~only the muon
    // - if there is a nearby jet apply JEC 'only to hadronic part'
    // - formula from RA5:
    // - jet_muonAware = (jet_fullJEC * rawFactor - (muon / L1CorrFactor)) * fullCorrFactor + muon
    // - rawFactor: from fullJECJet -> rawJet
    // - L1CorrFactor: from rawJet -> L1CorrJet; [jecFactorL1/jecFactorUncorrected]
    // - fullCorrFactor: from rawJet -> fullJECecJet; [1/rawFactor]
    double jecFactorFullJECToL1 = closestJet.jecFactor("L1FastJet");
    if (jecFactorFullJECToRaw > 0 && jecFactorFullJECToL1 > 0) {
      lepawareJetPt = jetVec.Pt();
      lepawareJetEta = jetVec.Eta();
      lepawareJetPhi = jetVec.Phi();
      lepawareJetM = jetVec.M();
      lepawareJetJECToRawFactor = jecFactorFullJECToRaw;
      lepawareJetJECToL1Factor = jecFactorFullJECToL1;
      double jecFactorRawToL1 = jecFactorFullJECToL1 / jecFactorFullJECToRaw;
      // from fully corrected (JEC & JER) jet to raw jet
      rescaleTLorentzVector(jetVec,jecFactorFullJECToRaw, false);
      // from muon to (muon / L1CorrFactor)
      rescaleTLorentzVector(muonVec, (1/jecFactorRawToL1), true);
      // jetVec = (jet_fullJEC * rawFactor - (muon / L1CorrFactor))
      jetVec = jetVec - muonVec;
      // from (muon / L1CorrFactor) back to muon
      rescaleTLorentzVector(muonVec, jecFactorRawToL1, true);
      // jetVec = (jet_fullJEC * rawFactor - (muon / L1CorrFactor)) * fullCorrFactor
      rescaleTLorentzVector(jetVec,1/jecFactorFullJECToRaw, false);
      // (jet_fullJEC * rawFactor - (muon / L1CorrFactor)) * fullCorrFactor + muon
      jetVec = jetVec + muonVec;

      ptRatio = muonVec.Pt() / jetVec.Pt();
      ptRel = getPtRel(muonVec, jetVec);
      return std::make_tuple(ptRatio, ptRel, lepawareJetPt, lepawareJetEta, lepawareJetPhi, lepawareJetM, lepawareJetJECToRawFactor, lepawareJetJECToL1Factor);
    }
  } else {
    // CASE 3:
    // - dR(muon,closestJet) > 0.4
    // - use muon as lepaware jet
    return std::make_tuple(ptRatio, ptRel, lepawareJetPt, lepawareJetEta, lepawareJetPhi, lepawareJetM, lepawareJetJECToRawFactor, lepawareJetJECToL1Factor);
  }
  // CASE X:
  // - something went wrong
  // - above cases should catch eveything
  // - set variables to dummy values failing very cut
  ptRatio = -999.;
  ptRel = -999.;
  lepawareJetPt = -999.;
  lepawareJetEta = -999.;
  lepawareJetPhi = -999.;
  lepawareJetM = -999.;
  lepawareJetJECToRawFactor = -999.;
  lepawareJetJECToL1Factor = -999.;
  return std::make_tuple(ptRatio, ptRel, lepawareJetPt, lepawareJetEta, lepawareJetPhi, lepawareJetM, lepawareJetJECToRawFactor, lepawareJetJECToL1Factor);
}

const double MultiIsolation::getPtRel(TLorentzVector &lepton, TLorentzVector &jet) const {
  // helper function for lepton multiisolation
  // calculates ptRel from lepton and jet vector
  if ((jet-lepton).Rho() < 0.0001) {
    // jet containing only the lepton or lep.jet==lep (no match)
    return 0;
  } else {
    return lepton.Perp((jet-lepton).Vect());
  }
}

void MultiIsolation::rescaleTLorentzVector(TLorentzVector& vector, double scaleFactor, bool massConst = true) const {
  // helper function for lepton multiisolation
  // rescales vector with / without keeping mass constnt
  if (massConst) {
    vector.SetPtEtaPhiM(vector.Pt()*scaleFactor,
                        vector.Eta(),
                        vector.Phi(),
                        vector.M());
  } else {
    vector = vector * scaleFactor;
  }
}

