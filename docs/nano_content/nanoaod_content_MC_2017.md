





NanoAOD File Content:
run - Value < UInt_t >
Description: run/i
----------------------------------------------------------------------------------------------------
luminosityBlock - Value < UInt_t >
Description: luminosityBlock/i
----------------------------------------------------------------------------------------------------
event - Value < ULong64_t >
Description: event/l
----------------------------------------------------------------------------------------------------
HTXS_Higgs_pt - Value < Float_t >
Description: pt of the Higgs boson as identified in HTXS
----------------------------------------------------------------------------------------------------
HTXS_Higgs_y - Value < Float_t >
Description: rapidity of the Higgs boson as identified in HTXS
----------------------------------------------------------------------------------------------------
HTXS_stage1_1_cat_pTjet25GeV - Value < Int_t >
Description: HTXS stage-1.1 category(jet pt>25 GeV)
----------------------------------------------------------------------------------------------------
HTXS_stage1_1_cat_pTjet30GeV - Value < Int_t >
Description: HTXS stage-1.1 category(jet pt>30 GeV)
----------------------------------------------------------------------------------------------------
HTXS_stage1_1_fine_cat_pTjet25GeV - Value < Int_t >
Description: HTXS stage-1.1-fine category(jet pt>25 GeV)
----------------------------------------------------------------------------------------------------
HTXS_stage1_1_fine_cat_pTjet30GeV - Value < Int_t >
Description: HTXS stage-1.1-fine category(jet pt>30 GeV)
----------------------------------------------------------------------------------------------------
HTXS_stage1_2_cat_pTjet25GeV - Value < Int_t >
Description: HTXS stage-1.2 category(jet pt>25 GeV)
----------------------------------------------------------------------------------------------------
HTXS_stage1_2_cat_pTjet30GeV - Value < Int_t >
Description: HTXS stage-1.2 category(jet pt>30 GeV)
----------------------------------------------------------------------------------------------------
HTXS_stage1_2_fine_cat_pTjet25GeV - Value < Int_t >
Description: HTXS stage-1.2-fine category(jet pt>25 GeV)
----------------------------------------------------------------------------------------------------
HTXS_stage1_2_fine_cat_pTjet30GeV - Value < Int_t >
Description: HTXS stage-1.2-fine category(jet pt>30 GeV)
----------------------------------------------------------------------------------------------------
HTXS_stage_0 - Value < Int_t >
Description: HTXS stage-0 category
----------------------------------------------------------------------------------------------------
HTXS_stage_1_pTjet25 - Value < Int_t >
Description: HTXS stage-1 category (jet pt>25 GeV)
----------------------------------------------------------------------------------------------------
HTXS_stage_1_pTjet30 - Value < Int_t >
Description: HTXS stage-1 category (jet pt>30 GeV)
----------------------------------------------------------------------------------------------------
HTXS_njets25 - Value < UChar_t >
Description: number of jets with pt>25 GeV as identified in HTXS
----------------------------------------------------------------------------------------------------
HTXS_njets30 - Value < UChar_t >
Description: number of jets with pt>30 GeV as identified in HTXS
----------------------------------------------------------------------------------------------------
nboostedTau - Value < UInt_t >
Description: slimmedBoostedTaus after basic selection (pt > 40 && tauID('decayModeFindingNewDMs') && (tauID('byVVLooseIsolationMVArun2017v2DBoldDMwLT2017') || tauID('byVVLooseIsolationMVArun2017v2DBoldDMdR0p3wLT2017') || tauID('byVVLooseIsolationMVArun2017v2DBnewDMwLT2017')))
----------------------------------------------------------------------------------------------------
boostedTau_chargedIso - Vector < Float_t >
Description: charged isolation
----------------------------------------------------------------------------------------------------
boostedTau_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
boostedTau_leadTkDeltaEta - Vector < Float_t >
Description: eta of the leading track, minus tau eta
----------------------------------------------------------------------------------------------------
boostedTau_leadTkDeltaPhi - Vector < Float_t >
Description: phi of the leading track, minus tau phi
----------------------------------------------------------------------------------------------------
boostedTau_leadTkPtOverTauPt - Vector < Float_t >
Description: pt of the leading track divided by tau pt
----------------------------------------------------------------------------------------------------
boostedTau_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
boostedTau_neutralIso - Vector < Float_t >
Description: neutral (photon) isolation
----------------------------------------------------------------------------------------------------
boostedTau_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
boostedTau_photonsOutsideSignalCone - Vector < Float_t >
Description: sum of photons outside signal cone
----------------------------------------------------------------------------------------------------
boostedTau_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
boostedTau_puCorr - Vector < Float_t >
Description: pileup correction
----------------------------------------------------------------------------------------------------
boostedTau_rawAntiEle2018 - Vector < Float_t >
Description: Anti-electron MVA discriminator V6 raw output discriminator (2018)
----------------------------------------------------------------------------------------------------
boostedTau_rawIso - Vector < Float_t >
Description: combined isolation (deltaBeta corrections)
----------------------------------------------------------------------------------------------------
boostedTau_rawIsodR03 - Vector < Float_t >
Description: combined isolation (deltaBeta corrections, dR=0.3)
----------------------------------------------------------------------------------------------------
boostedTau_rawMVAnewDM2017v2 - Vector < Float_t >
Description: byIsolationMVArun2017v2DBnewDMwLT raw output discriminator (2017v2)
----------------------------------------------------------------------------------------------------
boostedTau_rawMVAoldDM2017v2 - Vector < Float_t >
Description: byIsolationMVArun2017v2DBoldDMwLT raw output discriminator (2017v2)
----------------------------------------------------------------------------------------------------
boostedTau_rawMVAoldDMdR032017v2 - Vector < Float_t >
Description: byIsolationMVArun2017v2DBoldDMdR0p3wLT raw output discriminator (2017v2)
----------------------------------------------------------------------------------------------------
boostedTau_charge - Vector < Int_t >
Description: electric charge
----------------------------------------------------------------------------------------------------
boostedTau_decayMode - Vector < Int_t >
Description: decayMode()
----------------------------------------------------------------------------------------------------
boostedTau_jetIdx - Vector < Int_t >
Description: index of the associated jet (-1 if none)
----------------------------------------------------------------------------------------------------
boostedTau_rawAntiEleCat2018 - Vector < Int_t >
Description: Anti-electron MVA discriminator V6 category (2018)
----------------------------------------------------------------------------------------------------
boostedTau_idAntiEle2018 - Vector < UChar_t >
Description: Anti-electron MVA discriminator V6 (2018): bitmask 1 = VLoose, 2 = Loose, 4 = Medium, 8 = Tight, 16 = VTight
----------------------------------------------------------------------------------------------------
boostedTau_idAntiMu - Vector < UChar_t >
Description: Anti-muon discriminator V3: : bitmask 1 = Loose, 2 = Tight
----------------------------------------------------------------------------------------------------
boostedTau_idMVAnewDM2017v2 - Vector < UChar_t >
Description: IsolationMVArun2017v2DBnewDMwLT ID working point (2017v2): bitmask 1 = VVLoose, 2 = VLoose, 4 = Loose, 8 = Medium, 16 = Tight, 32 = VTight, 64 = VVTight
----------------------------------------------------------------------------------------------------
boostedTau_idMVAoldDM2017v2 - Vector < UChar_t >
Description: IsolationMVArun2017v2DBoldDMwLT ID working point (2017v2): bitmask 1 = VVLoose, 2 = VLoose, 4 = Loose, 8 = Medium, 16 = Tight, 32 = VTight, 64 = VVTight
----------------------------------------------------------------------------------------------------
boostedTau_idMVAoldDMdR032017v2 - Vector < UChar_t >
Description: IsolationMVArun2017v2DBoldDMdR0p3wLT ID working point (2017v2): bitmask 1 = VVLoose, 2 = VLoose, 4 = Loose, 8 = Medium, 16 = Tight, 32 = VTight, 64 = VVTight
----------------------------------------------------------------------------------------------------
btagWeight_CSVV2 - Value < Float_t >
Description: b-tag event weight for CSVV2
----------------------------------------------------------------------------------------------------
btagWeight_DeepCSVB - Value < Float_t >
Description: b-tag event weight for DeepCSVB
----------------------------------------------------------------------------------------------------
CaloMET_phi - Value < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
CaloMET_pt - Value < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
CaloMET_sumEt - Value < Float_t >
Description: scalar sum of Et
----------------------------------------------------------------------------------------------------
ChsMET_phi - Value < Float_t >
Description: raw chs PF MET phi
----------------------------------------------------------------------------------------------------
ChsMET_pt - Value < Float_t >
Description: raw chs PF MET pt
----------------------------------------------------------------------------------------------------
ChsMET_sumEt - Value < Float_t >
Description: raw chs PF scalar sum of Et
----------------------------------------------------------------------------------------------------
nCorrT1METJet - Value < UInt_t >
Description: Additional low-pt jets for Type-1 MET re-correction
----------------------------------------------------------------------------------------------------
CorrT1METJet_area - Vector < Float_t >
Description: jet catchment area, for JECs
----------------------------------------------------------------------------------------------------
CorrT1METJet_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
CorrT1METJet_muonSubtrFactor - Vector < Float_t >
Description: 1-(muon-subtracted raw pt)/(raw pt)
----------------------------------------------------------------------------------------------------
CorrT1METJet_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
CorrT1METJet_rawPt - Vector < Float_t >
Description: pt()*jecFactor('Uncorrected')
----------------------------------------------------------------------------------------------------
DeepMETResolutionTune_phi - Value < Float_t >
Description: DeepmET ResolutionTune phi
----------------------------------------------------------------------------------------------------
DeepMETResolutionTune_pt - Value < Float_t >
Description: DeepMET ResolutionTune pt
----------------------------------------------------------------------------------------------------
DeepMETResponseTune_phi - Value < Float_t >
Description: DeepMET ResponseTune phi
----------------------------------------------------------------------------------------------------
DeepMETResponseTune_pt - Value < Float_t >
Description: DeepMET ResponseTune pt
----------------------------------------------------------------------------------------------------
nElectron - Value < UInt_t >
Description: slimmedElectrons after basic selection (pt > 5 )
----------------------------------------------------------------------------------------------------
Electron_dEscaleDown - Vector < Float_t >
Description: ecal energy scale shifted 1 sigma down (adding gain/stat/syst in quadrature)
----------------------------------------------------------------------------------------------------
Electron_dEscaleUp - Vector < Float_t >
Description: ecal energy scale shifted 1 sigma up(adding gain/stat/syst in quadrature)
----------------------------------------------------------------------------------------------------
Electron_dEsigmaDown - Vector < Float_t >
Description: ecal energy smearing value shifted 1 sigma up
----------------------------------------------------------------------------------------------------
Electron_dEsigmaUp - Vector < Float_t >
Description: ecal energy smearing value shifted 1 sigma up
----------------------------------------------------------------------------------------------------
Electron_deltaEtaSC - Vector < Float_t >
Description: delta eta (SC,ele) with sign
----------------------------------------------------------------------------------------------------
Electron_dr03EcalRecHitSumEt - Vector < Float_t >
Description: Non-PF Ecal isolation within a delta R cone of 0.3 with electron pt > 35 GeV
----------------------------------------------------------------------------------------------------
Electron_dr03HcalDepth1TowerSumEt - Vector < Float_t >
Description: Non-PF Hcal isolation within a delta R cone of 0.3 with electron pt > 35 GeV
----------------------------------------------------------------------------------------------------
Electron_dr03TkSumPt - Vector < Float_t >
Description: Non-PF track isolation within a delta R cone of 0.3 with electron pt > 35 GeV
----------------------------------------------------------------------------------------------------
Electron_dr03TkSumPtHEEP - Vector < Float_t >
Description: Non-PF track isolation within a delta R cone of 0.3 with electron pt > 35 GeV used in HEEP ID
----------------------------------------------------------------------------------------------------
Electron_dxy - Vector < Float_t >
Description: dxy (with sign) wrt first PV, in cm
----------------------------------------------------------------------------------------------------
Electron_dxyErr - Vector < Float_t >
Description: dxy uncertainty, in cm
----------------------------------------------------------------------------------------------------
Electron_dz - Vector < Float_t >
Description: dz (with sign) wrt first PV, in cm
----------------------------------------------------------------------------------------------------
Electron_dzErr - Vector < Float_t >
Description: dz uncertainty, in cm
----------------------------------------------------------------------------------------------------
Electron_eCorr - Vector < Float_t >
Description: ratio of the calibrated energy/miniaod energy
----------------------------------------------------------------------------------------------------
Electron_eInvMinusPInv - Vector < Float_t >
Description: 1/E_SC - 1/p_trk
----------------------------------------------------------------------------------------------------
Electron_energyErr - Vector < Float_t >
Description: energy error of the cluster-track combination
----------------------------------------------------------------------------------------------------
Electron_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
Electron_hoe - Vector < Float_t >
Description: H over E
----------------------------------------------------------------------------------------------------
Electron_ip3d - Vector < Float_t >
Description: 3D impact parameter wrt first PV, in cm
----------------------------------------------------------------------------------------------------
Electron_jetPtRelv2 - Vector < Float_t >
Description: Relative momentum of the lepton with respect to the closest jet after subtracting the lepton
----------------------------------------------------------------------------------------------------
Electron_jetRelIso - Vector < Float_t >
Description: Relative isolation in matched jet (1/ptRatio-1, pfRelIso04_all if no matched jet)
----------------------------------------------------------------------------------------------------
Electron_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
Electron_miniPFRelIso_all - Vector < Float_t >
Description: mini PF relative isolation, total (with scaled rho*EA PU corrections)
----------------------------------------------------------------------------------------------------
Electron_miniPFRelIso_chg - Vector < Float_t >
Description: mini PF relative isolation, charged component
----------------------------------------------------------------------------------------------------
Electron_mvaFall17V2Iso - Vector < Float_t >
Description: MVA Iso ID V2 score
----------------------------------------------------------------------------------------------------
Electron_mvaFall17V2noIso - Vector < Float_t >
Description: MVA noIso ID V2 score
----------------------------------------------------------------------------------------------------
Electron_pfRelIso03_all - Vector < Float_t >
Description: PF relative isolation dR=0.3, total (with rho*EA PU corrections)
----------------------------------------------------------------------------------------------------
Electron_pfRelIso03_chg - Vector < Float_t >
Description: PF relative isolation dR=0.3, charged component
----------------------------------------------------------------------------------------------------
Electron_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
Electron_pt - Vector < Float_t >
Description: p_{T}
----------------------------------------------------------------------------------------------------
Electron_r9 - Vector < Float_t >
Description: R9 of the supercluster, calculated with full 5x5 region
----------------------------------------------------------------------------------------------------
Electron_scEtOverPt - Vector < Float_t >
Description: (supercluster transverse energy)/pt-1
----------------------------------------------------------------------------------------------------
Electron_sieie - Vector < Float_t >
Description: sigma_IetaIeta of the supercluster, calculated with full 5x5 region
----------------------------------------------------------------------------------------------------
Electron_sip3d - Vector < Float_t >
Description: 3D impact parameter significance wrt first PV, in cm
----------------------------------------------------------------------------------------------------
Electron_mvaTTH - Vector < Float_t >
Description: TTH MVA lepton ID score
----------------------------------------------------------------------------------------------------
Electron_charge - Vector < Int_t >
Description: electric charge
----------------------------------------------------------------------------------------------------
Electron_cutBased - Vector < Int_t >
Description: cut-based ID Fall17 V2 (0:fail, 1:veto, 2:loose, 3:medium, 4:tight)
----------------------------------------------------------------------------------------------------
Electron_jetIdx - Vector < Int_t >
Description: index of the associated jet (-1 if none)
----------------------------------------------------------------------------------------------------
Electron_pdgId - Vector < Int_t >
Description: PDG code assigned by the event reconstruction (not by MC truth)
----------------------------------------------------------------------------------------------------
Electron_photonIdx - Vector < Int_t >
Description: index of the associated photon (-1 if none)
----------------------------------------------------------------------------------------------------
Electron_tightCharge - Vector < Int_t >
Description: Tight charge criteria (0:none, 1:isGsfScPixChargeConsistent, 2:isGsfCtfScPixChargeConsistent)
----------------------------------------------------------------------------------------------------
Electron_vidNestedWPBitmap - Vector < Int_t >
Description: VID compressed bitmap (MinPtCut,GsfEleSCEtaMultiRangeCut,GsfEleDEtaInSeedCut,GsfEleDPhiInCut,GsfEleFull5x5SigmaIEtaIEtaCut,GsfEleHadronicOverEMEnergyScaledCut,GsfEleEInverseMinusPInverseCut,GsfEleRelPFIsoScaledCut,GsfEleConversionVetoCut,GsfEleMissingHitsCut), 3 bits per cut
----------------------------------------------------------------------------------------------------
Electron_vidNestedWPBitmapHEEP - Vector < Int_t >
Description: VID compressed bitmap (MinPtCut,GsfEleSCEtaMultiRangeCut,GsfEleDEtaInSeedCut,GsfEleDPhiInCut,GsfEleFull5x5SigmaIEtaIEtaWithSatCut,GsfEleFull5x5E2x5OverE5x5WithSatCut,GsfEleHadronicOverEMLinearCut,GsfEleTrkPtIsoCut,GsfEleEmHadD1IsoRhoCut,GsfEleDxyCut,GsfEleMissingHitsCut,GsfEleEcalDrivenCut), 1 bits per cut
----------------------------------------------------------------------------------------------------
Electron_convVeto - Vector < Bool_t >
Description: pass conversion veto
----------------------------------------------------------------------------------------------------
Electron_cutBased_HEEP - Vector < Bool_t >
Description: cut-based HEEP ID
----------------------------------------------------------------------------------------------------
Electron_isPFcand - Vector < Bool_t >
Description: electron is PF candidate
----------------------------------------------------------------------------------------------------
Electron_jetNDauCharged - Vector < UChar_t >
Description: number of charged daughters of the closest jet
----------------------------------------------------------------------------------------------------
Electron_lostHits - Vector < UChar_t >
Description: number of missing inner hits
----------------------------------------------------------------------------------------------------
Electron_mvaFall17V2Iso_WP80 - Vector < Bool_t >
Description: MVA Iso ID V2 WP80
----------------------------------------------------------------------------------------------------
Electron_mvaFall17V2Iso_WP90 - Vector < Bool_t >
Description: MVA Iso ID V2 WP90
----------------------------------------------------------------------------------------------------
Electron_mvaFall17V2Iso_WPL - Vector < Bool_t >
Description: MVA Iso ID V2 loose WP
----------------------------------------------------------------------------------------------------
Electron_mvaFall17V2noIso_WP80 - Vector < Bool_t >
Description: MVA noIso ID V2 WP80
----------------------------------------------------------------------------------------------------
Electron_mvaFall17V2noIso_WP90 - Vector < Bool_t >
Description: MVA noIso ID V2 WP90
----------------------------------------------------------------------------------------------------
Electron_mvaFall17V2noIso_WPL - Vector < Bool_t >
Description: MVA noIso ID V2 loose WP
----------------------------------------------------------------------------------------------------
Electron_seedGain - Vector < UChar_t >
Description: Gain of the seed crystal
----------------------------------------------------------------------------------------------------
nFatJet - Value < UInt_t >
Description: slimmedJetsAK8, i.e. ak8 fat jets for boosted analysis
----------------------------------------------------------------------------------------------------
FatJet_area - Vector < Float_t >
Description: jet catchment area, for JECs
----------------------------------------------------------------------------------------------------
FatJet_btagCSVV2 - Vector < Float_t >
Description:  pfCombinedInclusiveSecondaryVertexV2 b-tag discriminator (aka CSVV2)
----------------------------------------------------------------------------------------------------
FatJet_btagDDBvLV2 - Vector < Float_t >
Description: DeepDoubleX V2(mass-decorrelated) discriminator for H(Z)->bb vs QCD
----------------------------------------------------------------------------------------------------
FatJet_btagDDCvBV2 - Vector < Float_t >
Description: DeepDoubleX V2 (mass-decorrelated) discriminator for H(Z)->cc vs H(Z)->bb
----------------------------------------------------------------------------------------------------
FatJet_btagDDCvLV2 - Vector < Float_t >
Description: DeepDoubleX V2 (mass-decorrelated) discriminator for H(Z)->cc vs QCD
----------------------------------------------------------------------------------------------------
FatJet_btagDeepB - Vector < Float_t >
Description: DeepCSV b+bb tag discriminator
----------------------------------------------------------------------------------------------------
FatJet_btagHbb - Vector < Float_t >
Description: Higgs to BB tagger discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_H4qvsQCD - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger H->4q vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_HbbvsQCD - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger H->bb vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_TvsQCD - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger top vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_WvsQCD - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger W vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_ZHbbvsQCD - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger Z/H->bb vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_ZHccvsQCD - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger Z/H->cc vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_ZbbvsQCD - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger Z->bb vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_ZvsQCD - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger Z vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_bbvsLight - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger Z/H/gluon->bb vs light flavour discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTagMD_ccvsLight - Vector < Float_t >
Description: Mass-decorrelated DeepBoostedJet tagger Z/H/gluon->cc vs light flavour discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTag_H - Vector < Float_t >
Description: DeepBoostedJet tagger H(bb,cc,4q) sum
----------------------------------------------------------------------------------------------------
FatJet_deepTag_QCD - Vector < Float_t >
Description: DeepBoostedJet tagger QCD(bb,cc,b,c,others) sum
----------------------------------------------------------------------------------------------------
FatJet_deepTag_QCDothers - Vector < Float_t >
Description: DeepBoostedJet tagger QCDothers value
----------------------------------------------------------------------------------------------------
FatJet_deepTag_TvsQCD - Vector < Float_t >
Description: DeepBoostedJet tagger top vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTag_WvsQCD - Vector < Float_t >
Description: DeepBoostedJet tagger W vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_deepTag_ZvsQCD - Vector < Float_t >
Description: DeepBoostedJet tagger Z vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
FatJet_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
FatJet_msoftdrop - Vector < Float_t >
Description: Corrected soft drop mass with PUPPI
----------------------------------------------------------------------------------------------------
FatJet_n2b1 - Vector < Float_t >
Description: N2 with beta=1
----------------------------------------------------------------------------------------------------
FatJet_n3b1 - Vector < Float_t >
Description: N3 with beta=1
----------------------------------------------------------------------------------------------------
FatJet_particleNetMD_QCD - Vector < Float_t >
Description: Mass-decorrelated ParticleNet tagger raw QCD score
----------------------------------------------------------------------------------------------------
FatJet_particleNetMD_Xbb - Vector < Float_t >
Description: Mass-decorrelated ParticleNet tagger raw X->bb score. For X->bb vs QCD tagging, use Xbb/(Xbb+QCD)
----------------------------------------------------------------------------------------------------
FatJet_particleNetMD_Xcc - Vector < Float_t >
Description: Mass-decorrelated ParticleNet tagger raw X->cc score. For X->cc vs QCD tagging, use Xcc/(Xcc+QCD)
----------------------------------------------------------------------------------------------------
FatJet_particleNetMD_Xqq - Vector < Float_t >
Description: Mass-decorrelated ParticleNet tagger raw X->qq (uds) score. For X->qq vs QCD tagging, use Xqq/(Xqq+QCD). For W vs QCD tagging, use (Xcc+Xqq)/(Xcc+Xqq+QCD)
----------------------------------------------------------------------------------------------------
FatJet_particleNet_H4qvsQCD - Vector < Float_t >
Description: ParticleNet tagger H(->VV->qqqq) vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_particleNet_HbbvsQCD - Vector < Float_t >
Description: ParticleNet tagger H(->bb) vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_particleNet_HccvsQCD - Vector < Float_t >
Description: ParticleNet tagger H(->cc) vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_particleNet_QCD - Vector < Float_t >
Description: ParticleNet tagger QCD(bb,cc,b,c,others) sum
----------------------------------------------------------------------------------------------------
FatJet_particleNet_TvsQCD - Vector < Float_t >
Description: ParticleNet tagger top vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_particleNet_WvsQCD - Vector < Float_t >
Description: ParticleNet tagger W vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_particleNet_ZvsQCD - Vector < Float_t >
Description: ParticleNet tagger Z vs QCD discriminator
----------------------------------------------------------------------------------------------------
FatJet_particleNet_mass - Vector < Float_t >
Description: ParticleNet mass regression
----------------------------------------------------------------------------------------------------
FatJet_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
FatJet_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
FatJet_rawFactor - Vector < Float_t >
Description: 1 - Factor to get back to raw pT
----------------------------------------------------------------------------------------------------
FatJet_tau1 - Vector < Float_t >
Description: Nsubjettiness (1 axis)
----------------------------------------------------------------------------------------------------
FatJet_tau2 - Vector < Float_t >
Description: Nsubjettiness (2 axis)
----------------------------------------------------------------------------------------------------
FatJet_tau3 - Vector < Float_t >
Description: Nsubjettiness (3 axis)
----------------------------------------------------------------------------------------------------
FatJet_tau4 - Vector < Float_t >
Description: Nsubjettiness (4 axis)
----------------------------------------------------------------------------------------------------
FatJet_lsf3 - Vector < Float_t >
Description: Lepton Subjet Fraction (3 subjets)
----------------------------------------------------------------------------------------------------
FatJet_jetId - Vector < Int_t >
Description: Jet ID flags bit1 is loose (always false in 2017 since it does not exist), bit2 is tight, bit3 is tightLepVeto
----------------------------------------------------------------------------------------------------
FatJet_subJetIdx1 - Vector < Int_t >
Description: index of first subjet
----------------------------------------------------------------------------------------------------
FatJet_subJetIdx2 - Vector < Int_t >
Description: index of second subjet
----------------------------------------------------------------------------------------------------
FatJet_electronIdx3SJ - Vector < Int_t >
Description: index of electron matched to jet
----------------------------------------------------------------------------------------------------
FatJet_muonIdx3SJ - Vector < Int_t >
Description: index of muon matched to jet
----------------------------------------------------------------------------------------------------
FatJet_nConstituents - Vector < UChar_t >
Description: Number of particles in the jet
----------------------------------------------------------------------------------------------------
nFsrPhoton - Value < UInt_t >
Description: Final state radiation photons emitted by muons
----------------------------------------------------------------------------------------------------
FsrPhoton_dROverEt2 - Vector < Float_t >
Description: deltaR to associated muon divided by photon et2
----------------------------------------------------------------------------------------------------
FsrPhoton_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
FsrPhoton_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
FsrPhoton_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
FsrPhoton_relIso03 - Vector < Float_t >
Description: relative isolation in a 0.3 cone without CHS
----------------------------------------------------------------------------------------------------
FsrPhoton_muonIdx - Vector < Int_t >
Description: index of associated muon
----------------------------------------------------------------------------------------------------
nGenJetAK8 - Value < UInt_t >
Description: slimmedGenJetsAK8, i.e. ak8 Jets made with visible genparticles
----------------------------------------------------------------------------------------------------
GenJetAK8_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
GenJetAK8_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
GenJetAK8_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
GenJetAK8_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
nGenJet - Value < UInt_t >
Description: slimmedGenJets, i.e. ak4 Jets made with visible genparticles
----------------------------------------------------------------------------------------------------
GenJet_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
GenJet_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
GenJet_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
GenJet_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
nGenPart - Value < UInt_t >
Description: interesting gen particles 
----------------------------------------------------------------------------------------------------
GenPart_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
GenPart_mass - Vector < Float_t >
Description: Mass stored for all particles with the exception of quarks (except top), leptons/neutrinos, photons with mass < 1 GeV, gluons, pi0(111), pi+(211), D0(421), and D+(411). For these particles, you can lookup the value from PDG.
----------------------------------------------------------------------------------------------------
GenPart_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
GenPart_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
GenPart_genPartIdxMother - Vector < Int_t >
Description: index of the mother particle
----------------------------------------------------------------------------------------------------
GenPart_pdgId - Vector < Int_t >
Description: PDG id
----------------------------------------------------------------------------------------------------
GenPart_status - Vector < Int_t >
Description: Particle status. 1=stable
----------------------------------------------------------------------------------------------------
GenPart_statusFlags - Vector < Int_t >
Description: gen status flags stored bitwise, bits are: 0 : isPrompt, 1 : isDecayedLeptonHadron, 2 : isTauDecayProduct, 3 : isPromptTauDecayProduct, 4 : isDirectTauDecayProduct, 5 : isDirectPromptTauDecayProduct, 6 : isDirectHadronDecayProduct, 7 : isHardProcess, 8 : fromHardProcess, 9 : isHardProcessTauDecayProduct, 10 : isDirectHardProcessTauDecayProduct, 11 : fromHardProcessBeforeFSR, 12 : isFirstCopy, 13 : isLastCopy, 14 : isLastCopyBeforeFSR, 
----------------------------------------------------------------------------------------------------
nSubGenJetAK8 - Value < UInt_t >
Description: slimmedGenJetsAK8SoftDropSubJets, i.e. subjets of ak8 Jets made with visible genparticles
----------------------------------------------------------------------------------------------------
SubGenJetAK8_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
SubGenJetAK8_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
SubGenJetAK8_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
SubGenJetAK8_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
Generator_binvar - Value < Float_t >
Description: MC generation binning value
----------------------------------------------------------------------------------------------------
Generator_scalePDF - Value < Float_t >
Description: Q2 scale for PDF
----------------------------------------------------------------------------------------------------
Generator_weight - Value < Float_t >
Description: MC generator weight
----------------------------------------------------------------------------------------------------
Generator_x1 - Value < Float_t >
Description: x1 fraction of proton momentum carried by the first parton
----------------------------------------------------------------------------------------------------
Generator_x2 - Value < Float_t >
Description: x2 fraction of proton momentum carried by the second parton
----------------------------------------------------------------------------------------------------
Generator_xpdf1 - Value < Float_t >
Description: x*pdf(x) for the first parton
----------------------------------------------------------------------------------------------------
Generator_xpdf2 - Value < Float_t >
Description: x*pdf(x) for the second parton
----------------------------------------------------------------------------------------------------
Generator_id1 - Value < Int_t >
Description: id of first parton
----------------------------------------------------------------------------------------------------
Generator_id2 - Value < Int_t >
Description: id of second parton
----------------------------------------------------------------------------------------------------
GenVtx_x - Value < Float_t >
Description: gen vertex x
----------------------------------------------------------------------------------------------------
GenVtx_y - Value < Float_t >
Description: gen vertex y
----------------------------------------------------------------------------------------------------
GenVtx_z - Value < Float_t >
Description: gen vertex z
----------------------------------------------------------------------------------------------------
nGenVisTau - Value < UInt_t >
Description: gen hadronic taus 
----------------------------------------------------------------------------------------------------
GenVisTau_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
GenVisTau_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
GenVisTau_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
GenVisTau_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
GenVisTau_charge - Vector < Int_t >
Description: charge
----------------------------------------------------------------------------------------------------
GenVisTau_genPartIdxMother - Vector < Int_t >
Description: index of the mother particle
----------------------------------------------------------------------------------------------------
GenVisTau_status - Vector < Int_t >
Description: Hadronic tau decay mode. 0=OneProng0PiZero, 1=OneProng1PiZero, 2=OneProng2PiZero, 10=ThreeProng0PiZero, 11=ThreeProng1PiZero, 15=Other
----------------------------------------------------------------------------------------------------
genWeight - Value < Float_t >
Description: generator weight
----------------------------------------------------------------------------------------------------
LHEWeight_originalXWGTUP - Value < Float_t >
Description: Nominal event weight in the LHE file
----------------------------------------------------------------------------------------------------
nLHEPdfWeight - Value < UInt_t >
Description: 
----------------------------------------------------------------------------------------------------
LHEPdfWeight - Vector < Float_t >
Description: LHE pdf variation weights (w_var / w_nominal) for LHA IDs 325300 - 325402
----------------------------------------------------------------------------------------------------
nLHEReweightingWeight - Value < UInt_t >
Description: 
----------------------------------------------------------------------------------------------------
LHEReweightingWeight - Vector < Float_t >
Description: 
----------------------------------------------------------------------------------------------------
nLHEScaleWeight - Value < UInt_t >
Description: 
----------------------------------------------------------------------------------------------------
LHEScaleWeight - Vector < Float_t >
Description: LHE scale variation weights (w_var / w_nominal); [0] is MUF="0.5" MUR="0.5"; [1] is MUF="1.0" MUR="0.5"; [2] is MUF="2.0" MUR="0.5"; [3] is MUF="0.5" MUR="1.0"; [4] is MUF="1.0" MUR="1.0"; [5] is MUF="2.0" MUR="1.0"; [6] is MUF="0.5" MUR="2.0"; [7] is MUF="1.0" MUR="2.0"; [8] is MUF="2.0" MUR="2.0"
----------------------------------------------------------------------------------------------------
nPSWeight - Value < UInt_t >
Description: 
----------------------------------------------------------------------------------------------------
PSWeight - Vector < Float_t >
Description: PS weights (w_var / w_nominal);   [0] is ISR=2 FSR=1; [1] is ISR=1 FSR=2[2] is ISR=0.5 FSR=1; [3] is ISR=1 FSR=0.5;
----------------------------------------------------------------------------------------------------
nIsoTrack - Value < UInt_t >
Description: isolated tracks after basic selection (((pt>5 && (abs(pdgId) == 11 || abs(pdgId) == 13)) || pt > 10) && (abs(pdgId) < 15 || abs(eta) < 2.5) && ((abs(dxy) < 0.2 && abs(dz) < 0.1) || pt>15) && ((pfIsolationDR03().chargedHadronIso < 5 && pt < 25) || pfIsolationDR03().chargedHadronIso/pt < 0.2)) and lepton veto
----------------------------------------------------------------------------------------------------
IsoTrack_dxy - Vector < Float_t >
Description: dxy (with sign) wrt first PV, in cm
----------------------------------------------------------------------------------------------------
IsoTrack_dz - Vector < Float_t >
Description: dz (with sign) wrt first PV, in cm
----------------------------------------------------------------------------------------------------
IsoTrack_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
IsoTrack_pfRelIso03_all - Vector < Float_t >
Description: PF relative isolation dR=0.3, total (deltaBeta corrections)
----------------------------------------------------------------------------------------------------
IsoTrack_pfRelIso03_chg - Vector < Float_t >
Description: PF relative isolation dR=0.3, charged component
----------------------------------------------------------------------------------------------------
IsoTrack_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
IsoTrack_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
IsoTrack_miniPFRelIso_all - Vector < Float_t >
Description: mini PF relative isolation, total (with scaled rho*EA PU corrections)
----------------------------------------------------------------------------------------------------
IsoTrack_miniPFRelIso_chg - Vector < Float_t >
Description: mini PF relative isolation, charged component
----------------------------------------------------------------------------------------------------
IsoTrack_charge - Vector < Int_t >
Description: electric charge
----------------------------------------------------------------------------------------------------
IsoTrack_fromPV - Vector < Int_t >
Description: isolated track comes from PV
----------------------------------------------------------------------------------------------------
IsoTrack_pdgId - Vector < Int_t >
Description: PDG id of PF cand
----------------------------------------------------------------------------------------------------
IsoTrack_isHighPurityTrack - Vector < Bool_t >
Description: track is high purity
----------------------------------------------------------------------------------------------------
IsoTrack_isPFcand - Vector < Bool_t >
Description: if isolated track is a PF candidate
----------------------------------------------------------------------------------------------------
IsoTrack_isFromLostTrack - Vector < Bool_t >
Description: if isolated track comes from a lost track
----------------------------------------------------------------------------------------------------
nJet - Value < UInt_t >
Description: slimmedJets, i.e. ak4 PFJets CHS with JECs applied, after basic selection (pt > 15)
----------------------------------------------------------------------------------------------------
Jet_area - Vector < Float_t >
Description: jet catchment area, for JECs
----------------------------------------------------------------------------------------------------
Jet_btagCSVV2 - Vector < Float_t >
Description:  pfCombinedInclusiveSecondaryVertexV2 b-tag discriminator (aka CSVV2)
----------------------------------------------------------------------------------------------------
Jet_btagDeepB - Vector < Float_t >
Description: DeepCSV b+bb tag discriminator
----------------------------------------------------------------------------------------------------
Jet_btagDeepCvB - Vector < Float_t >
Description: DeepCSV c vs b+bb discriminator
----------------------------------------------------------------------------------------------------
Jet_btagDeepCvL - Vector < Float_t >
Description: DeepCSV c vs udsg discriminator
----------------------------------------------------------------------------------------------------
Jet_btagDeepFlavB - Vector < Float_t >
Description: DeepJet b+bb+lepb tag discriminator
----------------------------------------------------------------------------------------------------
Jet_btagDeepFlavCvB - Vector < Float_t >
Description: DeepJet c vs b+bb+lepb discriminator
----------------------------------------------------------------------------------------------------
Jet_btagDeepFlavCvL - Vector < Float_t >
Description: DeepJet c vs uds+g discriminator
----------------------------------------------------------------------------------------------------
Jet_btagDeepFlavQG - Vector < Float_t >
Description: DeepJet g vs uds discriminator
----------------------------------------------------------------------------------------------------
Jet_chEmEF - Vector < Float_t >
Description: charged Electromagnetic Energy Fraction
----------------------------------------------------------------------------------------------------
Jet_chFPV0EF - Vector < Float_t >
Description: charged fromPV==0 Energy Fraction (energy excluded from CHS jets). Previously called betastar.
----------------------------------------------------------------------------------------------------
Jet_chHEF - Vector < Float_t >
Description: charged Hadron Energy Fraction
----------------------------------------------------------------------------------------------------
Jet_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
Jet_hfsigmaEtaEta - Vector < Float_t >
Description: sigmaEtaEta for HF jets (noise discriminating variable)
----------------------------------------------------------------------------------------------------
Jet_hfsigmaPhiPhi - Vector < Float_t >
Description: sigmaPhiPhi for HF jets (noise discriminating variable)
----------------------------------------------------------------------------------------------------
Jet_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
Jet_muEF - Vector < Float_t >
Description: muon Energy Fraction
----------------------------------------------------------------------------------------------------
Jet_muonSubtrFactor - Vector < Float_t >
Description: 1-(muon-subtracted raw pt)/(raw pt)
----------------------------------------------------------------------------------------------------
Jet_neEmEF - Vector < Float_t >
Description: neutral Electromagnetic Energy Fraction
----------------------------------------------------------------------------------------------------
Jet_neHEF - Vector < Float_t >
Description: neutral Hadron Energy Fraction
----------------------------------------------------------------------------------------------------
Jet_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
Jet_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
Jet_puIdDisc - Vector < Float_t >
Description: Pileup ID discriminant with 106X (2017) training
----------------------------------------------------------------------------------------------------
Jet_qgl - Vector < Float_t >
Description: Quark vs Gluon likelihood discriminator
----------------------------------------------------------------------------------------------------
Jet_rawFactor - Vector < Float_t >
Description: 1 - Factor to get back to raw pT
----------------------------------------------------------------------------------------------------
Jet_bRegCorr - Vector < Float_t >
Description: pt correction for b-jet energy regression
----------------------------------------------------------------------------------------------------
Jet_bRegRes - Vector < Float_t >
Description: res on pt corrected with b-jet regression
----------------------------------------------------------------------------------------------------
Jet_cRegCorr - Vector < Float_t >
Description: pt correction for c-jet energy regression
----------------------------------------------------------------------------------------------------
Jet_cRegRes - Vector < Float_t >
Description: res on pt corrected with c-jet regression
----------------------------------------------------------------------------------------------------
Jet_electronIdx1 - Vector < Int_t >
Description: index of first matching electron
----------------------------------------------------------------------------------------------------
Jet_electronIdx2 - Vector < Int_t >
Description: index of second matching electron
----------------------------------------------------------------------------------------------------
Jet_hfadjacentEtaStripsSize - Vector < Int_t >
Description: eta size of the strips next to the central tower strip in HF (noise discriminating variable) 
----------------------------------------------------------------------------------------------------
Jet_hfcentralEtaStripSize - Vector < Int_t >
Description: eta size of the central tower strip in HF (noise discriminating variable) 
----------------------------------------------------------------------------------------------------
Jet_jetId - Vector < Int_t >
Description: Jet ID flags bit1 is loose (always false in 2017 since it does not exist), bit2 is tight, bit3 is tightLepVeto
----------------------------------------------------------------------------------------------------
Jet_muonIdx1 - Vector < Int_t >
Description: index of first matching muon
----------------------------------------------------------------------------------------------------
Jet_muonIdx2 - Vector < Int_t >
Description: index of second matching muon
----------------------------------------------------------------------------------------------------
Jet_nElectrons - Vector < Int_t >
Description: number of electrons in the jet
----------------------------------------------------------------------------------------------------
Jet_nMuons - Vector < Int_t >
Description: number of muons in the jet
----------------------------------------------------------------------------------------------------
Jet_puId - Vector < Int_t >
Description: Pileup ID flags with 106X (2017) training
----------------------------------------------------------------------------------------------------
Jet_nConstituents - Vector < UChar_t >
Description: Number of particles in the jet
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_Dn - Value < Float_t >
Description: L1 pre-firing event correction weight (1-probability), down var.
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_ECAL_Dn - Value < Float_t >
Description: ECAL L1 pre-firing event correction weight (1-probability), down var.
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_ECAL_Nom - Value < Float_t >
Description: ECAL L1 pre-firing event correction weight (1-probability)
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_ECAL_Up - Value < Float_t >
Description: ECAL L1 pre-firing event correction weight (1-probability), up var.
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_Muon_Nom - Value < Float_t >
Description: Muon L1 pre-firing event correction weight (1-probability)
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_Muon_StatDn - Value < Float_t >
Description: Muon L1 pre-firing event correction weight (1-probability), down var. stat.
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_Muon_StatUp - Value < Float_t >
Description: Muon L1 pre-firing event correction weight (1-probability), up var. stat.
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_Muon_SystDn - Value < Float_t >
Description: Muon L1 pre-firing event correction weight (1-probability), down var. syst.
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_Muon_SystUp - Value < Float_t >
Description: Muon L1 pre-firing event correction weight (1-probability), up var. syst.
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_Nom - Value < Float_t >
Description: L1 pre-firing event correction weight (1-probability)
----------------------------------------------------------------------------------------------------
L1PreFiringWeight_Up - Value < Float_t >
Description: L1 pre-firing event correction weight (1-probability), up var.
----------------------------------------------------------------------------------------------------
LHE_HT - Value < Float_t >
Description: HT, scalar sum of parton pTs at LHE step
----------------------------------------------------------------------------------------------------
LHE_HTIncoming - Value < Float_t >
Description: HT, scalar sum of parton pTs at LHE step, restricted to partons
----------------------------------------------------------------------------------------------------
LHE_Vpt - Value < Float_t >
Description: pT of the W or Z boson at LHE step
----------------------------------------------------------------------------------------------------
LHE_AlphaS - Value < Float_t >
Description: Per-event alphaS
----------------------------------------------------------------------------------------------------
LHE_Njets - Value < UChar_t >
Description: Number of jets (partons) at LHE step
----------------------------------------------------------------------------------------------------
LHE_Nb - Value < UChar_t >
Description: Number of b partons at LHE step
----------------------------------------------------------------------------------------------------
LHE_Nc - Value < UChar_t >
Description: Number of c partons at LHE step
----------------------------------------------------------------------------------------------------
LHE_Nuds - Value < UChar_t >
Description: Number of u,d,s partons at LHE step
----------------------------------------------------------------------------------------------------
LHE_Nglu - Value < UChar_t >
Description: Number of gluon partons at LHE step
----------------------------------------------------------------------------------------------------
LHE_NpNLO - Value < UChar_t >
Description: number of partons at NLO
----------------------------------------------------------------------------------------------------
LHE_NpLO - Value < UChar_t >
Description: number of partons at LO
----------------------------------------------------------------------------------------------------
nLHEPart - Value < UInt_t >
Description: 
----------------------------------------------------------------------------------------------------
LHEPart_pt - Vector < Float_t >
Description: Pt of LHE particles
----------------------------------------------------------------------------------------------------
LHEPart_eta - Vector < Float_t >
Description: Pseodorapidity of LHE particles
----------------------------------------------------------------------------------------------------
LHEPart_phi - Vector < Float_t >
Description: Phi of LHE particles
----------------------------------------------------------------------------------------------------
LHEPart_mass - Vector < Float_t >
Description: Mass of LHE particles
----------------------------------------------------------------------------------------------------
LHEPart_incomingpz - Vector < Float_t >
Description: Pz of incoming LHE particles
----------------------------------------------------------------------------------------------------
LHEPart_pdgId - Vector < Int_t >
Description: PDG ID of LHE particles
----------------------------------------------------------------------------------------------------
LHEPart_status - Vector < Int_t >
Description: LHE particle status; -1:incoming, 1:outgoing
----------------------------------------------------------------------------------------------------
LHEPart_spin - Vector < Int_t >
Description: Spin of LHE particles
----------------------------------------------------------------------------------------------------
nLowPtElectron - Value < UInt_t >
Description: slimmedLowPtElectrons after basic selection (pt > 1. && userFloat('ID') > -0.25)
----------------------------------------------------------------------------------------------------
LowPtElectron_ID - Vector < Float_t >
Description: New ID, BDT (raw) score
----------------------------------------------------------------------------------------------------
LowPtElectron_convVtxRadius - Vector < Float_t >
Description: conversion vertex radius (cm)
----------------------------------------------------------------------------------------------------
LowPtElectron_deltaEtaSC - Vector < Float_t >
Description: delta eta (SC,ele) with sign
----------------------------------------------------------------------------------------------------
LowPtElectron_dxy - Vector < Float_t >
Description: dxy (with sign) wrt first PV, in cm
----------------------------------------------------------------------------------------------------
LowPtElectron_dxyErr - Vector < Float_t >
Description: dxy uncertainty, in cm
----------------------------------------------------------------------------------------------------
LowPtElectron_dz - Vector < Float_t >
Description: dz (with sign) wrt first PV, in cm
----------------------------------------------------------------------------------------------------
LowPtElectron_dzErr - Vector < Float_t >
Description: dz uncertainty, in cm
----------------------------------------------------------------------------------------------------
LowPtElectron_eInvMinusPInv - Vector < Float_t >
Description: 1/E_SC - 1/p_trk
----------------------------------------------------------------------------------------------------
LowPtElectron_embeddedID - Vector < Float_t >
Description: ID, BDT (raw) score
----------------------------------------------------------------------------------------------------
LowPtElectron_energyErr - Vector < Float_t >
Description: energy error of the cluster-track combination
----------------------------------------------------------------------------------------------------
LowPtElectron_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
LowPtElectron_hoe - Vector < Float_t >
Description: H over E
----------------------------------------------------------------------------------------------------
LowPtElectron_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
LowPtElectron_miniPFRelIso_all - Vector < Float_t >
Description: mini PF relative isolation, total (with scaled rho*EA PU corrections)
----------------------------------------------------------------------------------------------------
LowPtElectron_miniPFRelIso_chg - Vector < Float_t >
Description: mini PF relative isolation, charged component
----------------------------------------------------------------------------------------------------
LowPtElectron_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
LowPtElectron_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
LowPtElectron_ptbiased - Vector < Float_t >
Description: ElectronSeed, pT- and dxy- dependent BDT (raw) score
----------------------------------------------------------------------------------------------------
LowPtElectron_r9 - Vector < Float_t >
Description: R9 of the SC, calculated with full 5x5 region
----------------------------------------------------------------------------------------------------
LowPtElectron_scEtOverPt - Vector < Float_t >
Description: (SC energy)/pt-1
----------------------------------------------------------------------------------------------------
LowPtElectron_sieie - Vector < Float_t >
Description: sigma_IetaIeta of the SC, calculated with full 5x5 region
----------------------------------------------------------------------------------------------------
LowPtElectron_unbiased - Vector < Float_t >
Description: ElectronSeed, pT- and dxy- agnostic BDT (raw) score
----------------------------------------------------------------------------------------------------
LowPtElectron_charge - Vector < Int_t >
Description: electric charge
----------------------------------------------------------------------------------------------------
LowPtElectron_convWP - Vector < Int_t >
Description: conversion flag bit map: 1=Veto, 2=Loose, 3=Tight
----------------------------------------------------------------------------------------------------
LowPtElectron_pdgId - Vector < Int_t >
Description: PDG code assigned by the event reconstruction (not by MC truth)
----------------------------------------------------------------------------------------------------
LowPtElectron_convVeto - Vector < Bool_t >
Description: pass conversion veto
----------------------------------------------------------------------------------------------------
LowPtElectron_lostHits - Vector < UChar_t >
Description: number of missing inner hits
----------------------------------------------------------------------------------------------------
GenMET_phi - Value < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
GenMET_pt - Value < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
MET_MetUnclustEnUpDeltaX - Value < Float_t >
Description: Delta (METx_mod-METx) Unclustered Energy Up
----------------------------------------------------------------------------------------------------
MET_MetUnclustEnUpDeltaY - Value < Float_t >
Description: Delta (METy_mod-METy) Unclustered Energy Up
----------------------------------------------------------------------------------------------------
MET_covXX - Value < Float_t >
Description: xx element of met covariance matrix
----------------------------------------------------------------------------------------------------
MET_covXY - Value < Float_t >
Description: xy element of met covariance matrix
----------------------------------------------------------------------------------------------------
MET_covYY - Value < Float_t >
Description: yy element of met covariance matrix
----------------------------------------------------------------------------------------------------
MET_phi - Value < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
MET_pt - Value < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
MET_significance - Value < Float_t >
Description: MET significance
----------------------------------------------------------------------------------------------------
MET_sumEt - Value < Float_t >
Description: scalar sum of Et
----------------------------------------------------------------------------------------------------
MET_sumPtUnclustered - Value < Float_t >
Description: sumPt used for MET significance
----------------------------------------------------------------------------------------------------
nMuon - Value < UInt_t >
Description: slimmedMuons after basic selection (pt > 15 || (pt > 3 && (passed('CutBasedIdLoose') || passed('SoftCutBasedId') || passed('SoftMvaId') || passed('CutBasedIdGlobalHighPt') || passed('CutBasedIdTrkHighPt'))))
----------------------------------------------------------------------------------------------------
Muon_dxy - Vector < Float_t >
Description: dxy (with sign) wrt first PV, in cm
----------------------------------------------------------------------------------------------------
Muon_dxyErr - Vector < Float_t >
Description: dxy uncertainty, in cm
----------------------------------------------------------------------------------------------------
Muon_dxybs - Vector < Float_t >
Description: dxy (with sign) wrt the beam spot, in cm
----------------------------------------------------------------------------------------------------
Muon_dz - Vector < Float_t >
Description: dz (with sign) wrt first PV, in cm
----------------------------------------------------------------------------------------------------
Muon_dzErr - Vector < Float_t >
Description: dz uncertainty, in cm
----------------------------------------------------------------------------------------------------
Muon_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
Muon_ip3d - Vector < Float_t >
Description: 3D impact parameter wrt first PV, in cm
----------------------------------------------------------------------------------------------------
Muon_jetPtRelv2 - Vector < Float_t >
Description: Relative momentum of the lepton with respect to the closest jet after subtracting the lepton
----------------------------------------------------------------------------------------------------
Muon_jetRelIso - Vector < Float_t >
Description: Relative isolation in matched jet (1/ptRatio-1, pfRelIso04_all if no matched jet)
----------------------------------------------------------------------------------------------------
Muon_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
Muon_miniPFRelIso_all - Vector < Float_t >
Description: mini PF relative isolation, total (with scaled rho*EA PU corrections)
----------------------------------------------------------------------------------------------------
Muon_miniPFRelIso_chg - Vector < Float_t >
Description: mini PF relative isolation, charged component
----------------------------------------------------------------------------------------------------
Muon_pfRelIso03_all - Vector < Float_t >
Description: PF relative isolation dR=0.3, total (deltaBeta corrections)
----------------------------------------------------------------------------------------------------
Muon_pfRelIso03_chg - Vector < Float_t >
Description: PF relative isolation dR=0.3, charged component
----------------------------------------------------------------------------------------------------
Muon_pfRelIso04_all - Vector < Float_t >
Description: PF relative isolation dR=0.4, total (deltaBeta corrections)
----------------------------------------------------------------------------------------------------
Muon_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
Muon_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
Muon_ptErr - Vector < Float_t >
Description: ptError of the muon track
----------------------------------------------------------------------------------------------------
Muon_segmentComp - Vector < Float_t >
Description: muon segment compatibility
----------------------------------------------------------------------------------------------------
Muon_sip3d - Vector < Float_t >
Description: 3D impact parameter significance wrt first PV
----------------------------------------------------------------------------------------------------
Muon_softMva - Vector < Float_t >
Description: soft MVA ID score
----------------------------------------------------------------------------------------------------
Muon_tkRelIso - Vector < Float_t >
Description: Tracker-based relative isolation dR=0.3 for highPt, trkIso/tunePpt
----------------------------------------------------------------------------------------------------
Muon_tunepRelPt - Vector < Float_t >
Description: TuneP relative pt, tunePpt/pt
----------------------------------------------------------------------------------------------------
Muon_mvaLowPt - Vector < Float_t >
Description: Low pt muon ID score
----------------------------------------------------------------------------------------------------
Muon_mvaTTH - Vector < Float_t >
Description: TTH MVA lepton ID score
----------------------------------------------------------------------------------------------------
Muon_charge - Vector < Int_t >
Description: electric charge
----------------------------------------------------------------------------------------------------
Muon_jetIdx - Vector < Int_t >
Description: index of the associated jet (-1 if none)
----------------------------------------------------------------------------------------------------
Muon_nStations - Vector < Int_t >
Description: number of matched stations with default arbitration (segment & track)
----------------------------------------------------------------------------------------------------
Muon_nTrackerLayers - Vector < Int_t >
Description: number of layers in the tracker
----------------------------------------------------------------------------------------------------
Muon_pdgId - Vector < Int_t >
Description: PDG code assigned by the event reconstruction (not by MC truth)
----------------------------------------------------------------------------------------------------
Muon_tightCharge - Vector < Int_t >
Description: Tight charge criterion using pterr/pt of muonBestTrack (0:fail, 2:pass)
----------------------------------------------------------------------------------------------------
Muon_fsrPhotonIdx - Vector < Int_t >
Description: Index of the associated FSR photon
----------------------------------------------------------------------------------------------------
Muon_highPtId - Vector < UChar_t >
Description: high-pT cut-based ID (1 = tracker high pT, 2 = global high pT, which includes tracker high pT)
----------------------------------------------------------------------------------------------------
Muon_highPurity - Vector < Bool_t >
Description: inner track is high purity
----------------------------------------------------------------------------------------------------
Muon_inTimeMuon - Vector < Bool_t >
Description: inTimeMuon ID
----------------------------------------------------------------------------------------------------
Muon_isGlobal - Vector < Bool_t >
Description: muon is global muon
----------------------------------------------------------------------------------------------------
Muon_isPFcand - Vector < Bool_t >
Description: muon is PF candidate
----------------------------------------------------------------------------------------------------
Muon_isStandalone - Vector < Bool_t >
Description: muon is a standalone muon
----------------------------------------------------------------------------------------------------
Muon_isTracker - Vector < Bool_t >
Description: muon is tracker muon
----------------------------------------------------------------------------------------------------
Muon_jetNDauCharged - Vector < UChar_t >
Description: number of charged daughters of the closest jet
----------------------------------------------------------------------------------------------------
Muon_looseId - Vector < Bool_t >
Description: muon is loose muon
----------------------------------------------------------------------------------------------------
Muon_mediumId - Vector < Bool_t >
Description: cut-based ID, medium WP
----------------------------------------------------------------------------------------------------
Muon_mediumPromptId - Vector < Bool_t >
Description: cut-based ID, medium prompt WP
----------------------------------------------------------------------------------------------------
Muon_miniIsoId - Vector < UChar_t >
Description: MiniIso ID from miniAOD selector (1=MiniIsoLoose, 2=MiniIsoMedium, 3=MiniIsoTight, 4=MiniIsoVeryTight)
----------------------------------------------------------------------------------------------------
Muon_multiIsoId - Vector < UChar_t >
Description: MultiIsoId from miniAOD selector (1=MultiIsoLoose, 2=MultiIsoMedium)
----------------------------------------------------------------------------------------------------
Muon_mvaId - Vector < UChar_t >
Description: Mva ID from miniAOD selector (1=MvaLoose, 2=MvaMedium, 3=MvaTight, 4=MvaVTight, 5=MvaVVTight)
----------------------------------------------------------------------------------------------------
Muon_mvaLowPtId - Vector < UChar_t >
Description: Low Pt Mva ID from miniAOD selector (1=LowPtMvaLoose, 2=LowPtMvaMedium)
----------------------------------------------------------------------------------------------------
Muon_pfIsoId - Vector < UChar_t >
Description: PFIso ID from miniAOD selector (1=PFIsoVeryLoose, 2=PFIsoLoose, 3=PFIsoMedium, 4=PFIsoTight, 5=PFIsoVeryTight, 6=PFIsoVeryVeryTight)
----------------------------------------------------------------------------------------------------
Muon_puppiIsoId - Vector < UChar_t >
Description: PuppiIsoId from miniAOD selector (1=Loose, 2=Medium, 3=Tight)
----------------------------------------------------------------------------------------------------
Muon_softId - Vector < Bool_t >
Description: soft cut-based ID
----------------------------------------------------------------------------------------------------
Muon_softMvaId - Vector < Bool_t >
Description: soft MVA ID
----------------------------------------------------------------------------------------------------
Muon_tightId - Vector < Bool_t >
Description: cut-based ID, tight WP
----------------------------------------------------------------------------------------------------
Muon_tkIsoId - Vector < UChar_t >
Description: TkIso ID (1=TkIsoLoose, 2=TkIsoTight)
----------------------------------------------------------------------------------------------------
Muon_triggerIdLoose - Vector < Bool_t >
Description: TriggerIdLoose ID
----------------------------------------------------------------------------------------------------
nPhoton - Value < UInt_t >
Description: slimmedPhotons after basic selection (pt > 5 )
----------------------------------------------------------------------------------------------------
Photon_dEscaleDown - Vector < Float_t >
Description: ecal energy scale shifted 1 sigma down (adding gain/stat/syst in quadrature)
----------------------------------------------------------------------------------------------------
Photon_dEscaleUp - Vector < Float_t >
Description: ecal energy scale shifted 1 sigma up (adding gain/stat/syst in quadrature)
----------------------------------------------------------------------------------------------------
Photon_dEsigmaDown - Vector < Float_t >
Description: ecal energy smearing value shifted 1 sigma up
----------------------------------------------------------------------------------------------------
Photon_dEsigmaUp - Vector < Float_t >
Description: ecal energy smearing value shifted 1 sigma up
----------------------------------------------------------------------------------------------------
Photon_eCorr - Vector < Float_t >
Description: ratio of the calibrated energy/miniaod energy
----------------------------------------------------------------------------------------------------
Photon_energyErr - Vector < Float_t >
Description: energy error of the cluster from regression
----------------------------------------------------------------------------------------------------
Photon_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
Photon_hoe - Vector < Float_t >
Description: H over E
----------------------------------------------------------------------------------------------------
Photon_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
Photon_mvaID - Vector < Float_t >
Description: MVA ID score, Fall17V2
----------------------------------------------------------------------------------------------------
Photon_mvaID_Fall17V1p1 - Vector < Float_t >
Description: MVA ID score, Fall17V1p1
----------------------------------------------------------------------------------------------------
Photon_pfRelIso03_all - Vector < Float_t >
Description: PF relative isolation dR=0.3, total (with rho*EA PU corrections)
----------------------------------------------------------------------------------------------------
Photon_pfRelIso03_chg - Vector < Float_t >
Description: PF relative isolation dR=0.3, charged component (with rho*EA PU corrections)
----------------------------------------------------------------------------------------------------
Photon_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
Photon_pt - Vector < Float_t >
Description: p_{T}
----------------------------------------------------------------------------------------------------
Photon_r9 - Vector < Float_t >
Description: R9 of the supercluster, calculated with full 5x5 region
----------------------------------------------------------------------------------------------------
Photon_sieie - Vector < Float_t >
Description: sigma_IetaIeta of the supercluster, calculated with full 5x5 region
----------------------------------------------------------------------------------------------------
Photon_charge - Vector < Int_t >
Description: electric charge
----------------------------------------------------------------------------------------------------
Photon_cutBased - Vector < Int_t >
Description: cut-based ID bitmap, Fall17V2, (0:fail, 1:loose, 2:medium, 3:tight)
----------------------------------------------------------------------------------------------------
Photon_cutBased_Fall17V1Bitmap - Vector < Int_t >
Description: cut-based ID bitmap, Fall17V1, 2^(0:loose, 1:medium, 2:tight).
----------------------------------------------------------------------------------------------------
Photon_electronIdx - Vector < Int_t >
Description: index of the associated electron (-1 if none)
----------------------------------------------------------------------------------------------------
Photon_jetIdx - Vector < Int_t >
Description: index of the associated jet (-1 if none)
----------------------------------------------------------------------------------------------------
Photon_pdgId - Vector < Int_t >
Description: PDG code assigned by the event reconstruction (not by MC truth)
----------------------------------------------------------------------------------------------------
Photon_vidNestedWPBitmap - Vector < Int_t >
Description: Fall17V2 VID compressed bitmap (MinPtCut,PhoSCEtaMultiRangeCut,PhoSingleTowerHadOverEmCut,PhoFull5x5SigmaIEtaIEtaCut,PhoGenericRhoPtScaledCut,PhoGenericRhoPtScaledCut,PhoGenericRhoPtScaledCut), 2 bits per cut
----------------------------------------------------------------------------------------------------
Photon_electronVeto - Vector < Bool_t >
Description: pass electron veto
----------------------------------------------------------------------------------------------------
Photon_isScEtaEB - Vector < Bool_t >
Description: is supercluster eta within barrel acceptance
----------------------------------------------------------------------------------------------------
Photon_isScEtaEE - Vector < Bool_t >
Description: is supercluster eta within endcap acceptance
----------------------------------------------------------------------------------------------------
Photon_mvaID_WP80 - Vector < Bool_t >
Description: MVA ID WP80, Fall17V2
----------------------------------------------------------------------------------------------------
Photon_mvaID_WP90 - Vector < Bool_t >
Description: MVA ID WP90, Fall17V2
----------------------------------------------------------------------------------------------------
Photon_pixelSeed - Vector < Bool_t >
Description: has pixel seed
----------------------------------------------------------------------------------------------------
Photon_seedGain - Vector < UChar_t >
Description: Gain of the seed crystal
----------------------------------------------------------------------------------------------------
Pileup_nTrueInt - Value < Float_t >
Description: the true mean number of the poisson distribution for this event from which the number of interactions each bunch crossing has been sampled
----------------------------------------------------------------------------------------------------
Pileup_pudensity - Value < Float_t >
Description: PU vertices / mm
----------------------------------------------------------------------------------------------------
Pileup_gpudensity - Value < Float_t >
Description: Generator-level PU vertices / mm
----------------------------------------------------------------------------------------------------
Pileup_nPU - Value < Int_t >
Description: the number of pileup interactions that have been added to the event in the current bunch crossing
----------------------------------------------------------------------------------------------------
Pileup_sumEOOT - Value < Int_t >
Description: number of early out of time pileup
----------------------------------------------------------------------------------------------------
Pileup_sumLOOT - Value < Int_t >
Description: number of late out of time pileup
----------------------------------------------------------------------------------------------------
PuppiMET_phi - Value < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
PuppiMET_phiJERDown - Value < Float_t >
Description: JER down phi
----------------------------------------------------------------------------------------------------
PuppiMET_phiJERUp - Value < Float_t >
Description: JER up phi
----------------------------------------------------------------------------------------------------
PuppiMET_phiJESDown - Value < Float_t >
Description: JES down phi
----------------------------------------------------------------------------------------------------
PuppiMET_phiJESUp - Value < Float_t >
Description: JES up phi
----------------------------------------------------------------------------------------------------
PuppiMET_phiUnclusteredDown - Value < Float_t >
Description: Unclustered down phi
----------------------------------------------------------------------------------------------------
PuppiMET_phiUnclusteredUp - Value < Float_t >
Description: Unclustered up phi
----------------------------------------------------------------------------------------------------
PuppiMET_pt - Value < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
PuppiMET_ptJERDown - Value < Float_t >
Description: JER down pt
----------------------------------------------------------------------------------------------------
PuppiMET_ptJERUp - Value < Float_t >
Description: JER up pt
----------------------------------------------------------------------------------------------------
PuppiMET_ptJESDown - Value < Float_t >
Description: JES down pt
----------------------------------------------------------------------------------------------------
PuppiMET_ptJESUp - Value < Float_t >
Description: JES up pt
----------------------------------------------------------------------------------------------------
PuppiMET_ptUnclusteredDown - Value < Float_t >
Description: Unclustered down pt
----------------------------------------------------------------------------------------------------
PuppiMET_ptUnclusteredUp - Value < Float_t >
Description: Unclustered up pt
----------------------------------------------------------------------------------------------------
PuppiMET_sumEt - Value < Float_t >
Description: scalar sum of Et
----------------------------------------------------------------------------------------------------
RawMET_phi - Value < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
RawMET_pt - Value < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
RawMET_sumEt - Value < Float_t >
Description: scalar sum of Et
----------------------------------------------------------------------------------------------------
RawPuppiMET_phi - Value < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
RawPuppiMET_pt - Value < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
RawPuppiMET_sumEt - Value < Float_t >
Description: scalar sum of Et
----------------------------------------------------------------------------------------------------
fixedGridRhoFastjetAll - Value < Float_t >
Description: rho from all PF Candidates, used e.g. for JECs
----------------------------------------------------------------------------------------------------
fixedGridRhoFastjetCentral - Value < Float_t >
Description: rho from all PF Candidates for central region, used e.g. for JECs
----------------------------------------------------------------------------------------------------
fixedGridRhoFastjetCentralCalo - Value < Float_t >
Description: rho from calo towers with |eta| < 2.5, used e.g. egamma PFCluster isolation
----------------------------------------------------------------------------------------------------
fixedGridRhoFastjetCentralChargedPileUp - Value < Float_t >
Description: rho from charged PF Candidates for central region, used e.g. for JECs
----------------------------------------------------------------------------------------------------
fixedGridRhoFastjetCentralNeutral - Value < Float_t >
Description: rho from neutral PF Candidates with |eta| < 2.5, used e.g. for rho corrections of some lepton isolations
----------------------------------------------------------------------------------------------------
nGenDressedLepton - Value < UInt_t >
Description: Dressed leptons from Rivet-based ParticleLevelProducer
----------------------------------------------------------------------------------------------------
GenDressedLepton_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
GenDressedLepton_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
GenDressedLepton_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
GenDressedLepton_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
GenDressedLepton_pdgId - Vector < Int_t >
Description: PDG id
----------------------------------------------------------------------------------------------------
GenDressedLepton_hasTauAnc - Vector < Bool_t >
Description: true if Dressed lepton has a tau as ancestor
----------------------------------------------------------------------------------------------------
nGenIsolatedPhoton - Value < UInt_t >
Description: Isolated photons from Rivet-based ParticleLevelProducer
----------------------------------------------------------------------------------------------------
GenIsolatedPhoton_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
GenIsolatedPhoton_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
GenIsolatedPhoton_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
GenIsolatedPhoton_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
nSoftActivityJet - Value < UInt_t >
Description: jets clustered from charged candidates compatible with primary vertex (charge()!=0 && pvAssociationQuality()>=5 && vertexRef().key()==0)
----------------------------------------------------------------------------------------------------
SoftActivityJet_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
SoftActivityJet_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
SoftActivityJet_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
SoftActivityJetHT - Value < Float_t >
Description: scalar sum of soft activity jet pt, pt>1
----------------------------------------------------------------------------------------------------
SoftActivityJetHT10 - Value < Float_t >
Description: scalar sum of soft activity jet pt , pt >10
----------------------------------------------------------------------------------------------------
SoftActivityJetHT2 - Value < Float_t >
Description: scalar sum of soft activity jet pt, pt >2
----------------------------------------------------------------------------------------------------
SoftActivityJetHT5 - Value < Float_t >
Description: scalar sum of soft activity jet pt, pt>5
----------------------------------------------------------------------------------------------------
SoftActivityJetNjets10 - Value < Int_t >
Description: number of soft activity jet pt, pt >2
----------------------------------------------------------------------------------------------------
SoftActivityJetNjets2 - Value < Int_t >
Description: number of soft activity jet pt, pt >10
----------------------------------------------------------------------------------------------------
SoftActivityJetNjets5 - Value < Int_t >
Description: number of soft activity jet pt, pt >5
----------------------------------------------------------------------------------------------------
nSubJet - Value < UInt_t >
Description: slimmedJetsAK8, i.e. ak8 fat jets for boosted analysis
----------------------------------------------------------------------------------------------------
SubJet_btagCSVV2 - Vector < Float_t >
Description:  pfCombinedInclusiveSecondaryVertexV2 b-tag discriminator (aka CSVV2)
----------------------------------------------------------------------------------------------------
SubJet_btagDeepB - Vector < Float_t >
Description: DeepCSV b+bb tag discriminator
----------------------------------------------------------------------------------------------------
SubJet_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
SubJet_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
SubJet_n2b1 - Vector < Float_t >
Description: N2 with beta=1
----------------------------------------------------------------------------------------------------
SubJet_n3b1 - Vector < Float_t >
Description: N3 with beta=1
----------------------------------------------------------------------------------------------------
SubJet_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
SubJet_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
SubJet_rawFactor - Vector < Float_t >
Description: 1 - Factor to get back to raw pT
----------------------------------------------------------------------------------------------------
SubJet_tau1 - Vector < Float_t >
Description: Nsubjettiness (1 axis)
----------------------------------------------------------------------------------------------------
SubJet_tau2 - Vector < Float_t >
Description: Nsubjettiness (2 axis)
----------------------------------------------------------------------------------------------------
SubJet_tau3 - Vector < Float_t >
Description: Nsubjettiness (3 axis)
----------------------------------------------------------------------------------------------------
SubJet_tau4 - Vector < Float_t >
Description: Nsubjettiness (4 axis)
----------------------------------------------------------------------------------------------------
nTau - Value < UInt_t >
Description: slimmedTaus after basic selection (pt > 18 && tauID('decayModeFindingNewDMs') && (tauID('byLooseCombinedIsolationDeltaBetaCorr3Hits') || (tauID('chargedIsoPtSumdR03')+max(0.,tauID('neutralIsoPtSumdR03')-0.072*tauID('puCorrPtSum'))<2.5) || tauID('byVVVLooseDeepTau2017v2p1VSjet')))
----------------------------------------------------------------------------------------------------
Tau_chargedIso - Vector < Float_t >
Description: charged isolation
----------------------------------------------------------------------------------------------------
Tau_dxy - Vector < Float_t >
Description: d_{xy} of lead track with respect to PV, in cm (with sign)
----------------------------------------------------------------------------------------------------
Tau_dz - Vector < Float_t >
Description: d_{z} of lead track with respect to PV, in cm (with sign)
----------------------------------------------------------------------------------------------------
Tau_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
Tau_leadTkDeltaEta - Vector < Float_t >
Description: eta of the leading track, minus tau eta
----------------------------------------------------------------------------------------------------
Tau_leadTkDeltaPhi - Vector < Float_t >
Description: phi of the leading track, minus tau phi
----------------------------------------------------------------------------------------------------
Tau_leadTkPtOverTauPt - Vector < Float_t >
Description: pt of the leading track divided by tau pt
----------------------------------------------------------------------------------------------------
Tau_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
Tau_neutralIso - Vector < Float_t >
Description: neutral (photon) isolation
----------------------------------------------------------------------------------------------------
Tau_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
Tau_photonsOutsideSignalCone - Vector < Float_t >
Description: sum of photons outside signal cone
----------------------------------------------------------------------------------------------------
Tau_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
Tau_puCorr - Vector < Float_t >
Description: pileup correction
----------------------------------------------------------------------------------------------------
Tau_rawDeepTau2017v2p1VSe - Vector < Float_t >
Description: byDeepTau2017v2p1VSe raw output discriminator (deepTau2017v2p1)
----------------------------------------------------------------------------------------------------
Tau_rawDeepTau2017v2p1VSjet - Vector < Float_t >
Description: byDeepTau2017v2p1VSjet raw output discriminator (deepTau2017v2p1)
----------------------------------------------------------------------------------------------------
Tau_rawDeepTau2017v2p1VSmu - Vector < Float_t >
Description: byDeepTau2017v2p1VSmu raw output discriminator (deepTau2017v2p1)
----------------------------------------------------------------------------------------------------
Tau_rawIso - Vector < Float_t >
Description: combined isolation (deltaBeta corrections)
----------------------------------------------------------------------------------------------------
Tau_rawIsodR03 - Vector < Float_t >
Description: combined isolation (deltaBeta corrections, dR=0.3)
----------------------------------------------------------------------------------------------------
Tau_charge - Vector < Int_t >
Description: electric charge
----------------------------------------------------------------------------------------------------
Tau_decayMode - Vector < Int_t >
Description: decayMode()
----------------------------------------------------------------------------------------------------
Tau_jetIdx - Vector < Int_t >
Description: index of the associated jet (-1 if none)
----------------------------------------------------------------------------------------------------
Tau_idAntiEleDeadECal - Vector < Bool_t >
Description: Anti-electron dead-ECal discriminator
----------------------------------------------------------------------------------------------------
Tau_idAntiMu - Vector < UChar_t >
Description: Anti-muon discriminator V3: : bitmask 1 = Loose, 2 = Tight
----------------------------------------------------------------------------------------------------
Tau_idDecayModeOldDMs - Vector < Bool_t >
Description: tauID('decayModeFinding')
----------------------------------------------------------------------------------------------------
Tau_idDeepTau2017v2p1VSe - Vector < UChar_t >
Description: byDeepTau2017v2p1VSe ID working points (deepTau2017v2p1): bitmask 1 = VVVLoose, 2 = VVLoose, 4 = VLoose, 8 = Loose, 16 = Medium, 32 = Tight, 64 = VTight, 128 = VVTight
----------------------------------------------------------------------------------------------------
Tau_idDeepTau2017v2p1VSjet - Vector < UChar_t >
Description: byDeepTau2017v2p1VSjet ID working points (deepTau2017v2p1): bitmask 1 = VVVLoose, 2 = VVLoose, 4 = VLoose, 8 = Loose, 16 = Medium, 32 = Tight, 64 = VTight, 128 = VVTight
----------------------------------------------------------------------------------------------------
Tau_idDeepTau2017v2p1VSmu - Vector < UChar_t >
Description: byDeepTau2017v2p1VSmu ID working points (deepTau2017v2p1): bitmask 1 = VLoose, 2 = Loose, 4 = Medium, 8 = Tight
----------------------------------------------------------------------------------------------------
TkMET_phi - Value < Float_t >
Description: raw track MET phi
----------------------------------------------------------------------------------------------------
TkMET_pt - Value < Float_t >
Description: raw track MET pt
----------------------------------------------------------------------------------------------------
TkMET_sumEt - Value < Float_t >
Description: raw track scalar sum of Et
----------------------------------------------------------------------------------------------------
nTrigObj - Value < UInt_t >
Description: 
----------------------------------------------------------------------------------------------------
TrigObj_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
TrigObj_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
TrigObj_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
TrigObj_l1pt - Vector < Float_t >
Description: pt of associated L1 seed
----------------------------------------------------------------------------------------------------
TrigObj_l1pt_2 - Vector < Float_t >
Description: pt of associated secondary L1 seed
----------------------------------------------------------------------------------------------------
TrigObj_l2pt - Vector < Float_t >
Description: pt of associated 'L2' seed (i.e. HLT before tracking/PF)
----------------------------------------------------------------------------------------------------
TrigObj_id - Vector < Int_t >
Description: ID of the object: 11 = Electron (PixelMatched e/gamma), 22 = Photon (PixelMatch-vetoed e/gamma), 13 = Muon, 15 = Tau, 1 = Jet, 6 = FatJet, 2 = MET, 3 = HT, 4 = MHT
----------------------------------------------------------------------------------------------------
TrigObj_l1iso - Vector < Int_t >
Description: iso of associated L1 seed
----------------------------------------------------------------------------------------------------
TrigObj_l1charge - Vector < Int_t >
Description: charge of associated L1 seed
----------------------------------------------------------------------------------------------------
TrigObj_filterBits - Vector < Int_t >
Description: extra bits of associated information: 1 = CaloIdL_TrackIdL_IsoVL, 2 = 1e (WPTight), 4 = 1e (WPLoose), 8 = OverlapFilter PFTau, 16 = 2e, 32 = 1e-1mu, 64 = 1e-1tau, 128 = 3e, 256 = 2e-1mu, 512 = 1e-2mu, 1024 = 1e (32_L1DoubleEG_AND_L1SingleEGOr), 2048 = 1e (CaloIdVT_GsfTrkIdT), 4096 = 1e (PFJet), 8192 = 1e (Photon175_OR_Photon200) for Electron (PixelMatched e/gamma); 1 = TrkIsoVVL, 2 = Iso, 4 = OverlapFilter PFTau, 8 = 1mu, 16 = 2mu, 32 = 1mu-1e, 64 = 1mu-1tau, 128 = 3mu, 256 = 2mu-1e, 512 = 1mu-2e, 1024 = 1mu (Mu50), 2048 = 1mu (Mu100) for Muon; 1 = LooseChargedIso, 2 = MediumChargedIso, 4 = TightChargedIso, 8 = TightID OOSC photons, 16 = HPS, 32 = single-tau + tau+MET, 64 = di-tau, 128 = e-tau, 256 = mu-tau, 512 = VBF+di-tau for Tau; Jet bits: bit 0 for VBF cross-cleaned from loose iso PFTau, bit 1 for hltBTagCaloCSVp087Triple, bit 2 for hltDoubleCentralJet90, bit 3 for hltDoublePFCentralJetLooseID90, bit 4 for hltL1sTripleJetVBFIorHTTIorDoubleJetCIorSingleJet, bit 5 for hltQuadCentralJet30, bit 6 for hltQuadPFCentralJetLooseID30, bit 7 for hltL1sQuadJetC50IorQuadJetC60IorHTT280IorHTT300IorHTT320IorTripleJet846848VBFIorTripleJet887256VBFIorTripleJet927664VBF or hltL1sQuadJetCIorTripleJetVBFIorHTT, bit 8 for hltQuadCentralJet45, bit 9 for hltQuadPFCentralJetLooseID45, bit 10  for hltL1sQuadJetC60IorHTT380IorHTT280QuadJetIorHTT300QuadJet or hltL1sQuadJetC50to60IorHTT280to500IorHTT250to340QuadJet bit 11 for hltBTagCaloCSVp05Double or hltBTagCaloDeepCSVp17Double, bit 12 for hltPFCentralJetLooseIDQuad30, bit 13 for hlt1PFCentralJetLooseID75, bit 14 for hlt2PFCentralJetLooseID60, bit 15 for hlt3PFCentralJetLooseID45, bit 16 for hlt4PFCentralJetLooseID40, bit 17 for hltBTagPFCSVp070Triple or hltBTagPFDeepCSVp24Triple or hltBTagPFDeepCSV4p5Triple  for Jet; HT bits: bit 0 for hltL1sTripleJetVBFIorHTTIorDoubleJetCIorSingleJet, bit 1 for hltL1sQuadJetC50IorQuadJetC60IorHTT280IorHTT300IorHTT320IorTripleJet846848VBFIorTripleJet887256VBFIorTripleJet927664VBF or hltL1sQuadJetCIorTripleJetVBFIorHTT, bit 2 for hltL1sQuadJetC60IorHTT380IorHTT280QuadJetIorHTT300QuadJet or hltL1sQuadJetC50to60IorHTT280to500IorHTT250to340QuadJet, bit 3 for hltCaloQuadJet30HT300 or hltCaloQuadJet30HT320, bit 4 for hltPFCentralJetsLooseIDQuad30HT300 or hltPFCentralJetsLooseIDQuad30HT330 for HT; MHT bits: bit 0 for hltCaloQuadJet30HT300 or hltCaloQuadJet30HT320, bit 1 for hltPFCentralJetsLooseIDQuad30HT300 or hltPFCentralJetsLooseIDQuad30HT330 for MHT; 
----------------------------------------------------------------------------------------------------
genTtbarId - Value < Int_t >
Description: ttbar categorization
----------------------------------------------------------------------------------------------------
nOtherPV - Value < UInt_t >
Description: 
----------------------------------------------------------------------------------------------------
OtherPV_z - Vector < Float_t >
Description: Z position of other primary vertices, excluding the main PV
----------------------------------------------------------------------------------------------------
PV_ndof - Value < Float_t >
Description: main primary vertex number of degree of freedom
----------------------------------------------------------------------------------------------------
PV_x - Value < Float_t >
Description: main primary vertex position x coordinate
----------------------------------------------------------------------------------------------------
PV_y - Value < Float_t >
Description: main primary vertex position y coordinate
----------------------------------------------------------------------------------------------------
PV_z - Value < Float_t >
Description: main primary vertex position z coordinate
----------------------------------------------------------------------------------------------------
PV_chi2 - Value < Float_t >
Description: main primary vertex reduced chi2
----------------------------------------------------------------------------------------------------
PV_score - Value < Float_t >
Description: main primary vertex score, i.e. sum pt2 of clustered objects
----------------------------------------------------------------------------------------------------
PV_npvs - Value < Int_t >
Description: total number of reconstructed primary vertices
----------------------------------------------------------------------------------------------------
PV_npvsGood - Value < Int_t >
Description: number of good reconstructed primary vertices. selection:!isFake && ndof > 4 && abs(z) <= 24 && position.Rho <= 2
----------------------------------------------------------------------------------------------------
nSV - Value < UInt_t >
Description: 
----------------------------------------------------------------------------------------------------
SV_dlen - Vector < Float_t >
Description: decay length in cm
----------------------------------------------------------------------------------------------------
SV_dlenSig - Vector < Float_t >
Description: decay length significance
----------------------------------------------------------------------------------------------------
SV_dxy - Vector < Float_t >
Description: 2D decay length in cm
----------------------------------------------------------------------------------------------------
SV_dxySig - Vector < Float_t >
Description: 2D decay length significance
----------------------------------------------------------------------------------------------------
SV_pAngle - Vector < Float_t >
Description: pointing angle, i.e. acos(p_SV * (SV - PV)) 
----------------------------------------------------------------------------------------------------
SV_charge - Vector < Int_t >
Description: sum of the charge of the SV tracks
----------------------------------------------------------------------------------------------------
boostedTau_genPartIdx - Vector < Int_t >
Description: Index into genParticle list for MC matching to status==2 taus
----------------------------------------------------------------------------------------------------
boostedTau_genPartFlav - Vector < UChar_t >
Description: Flavour of genParticle for MC matching to status==2 taus: 1 = prompt electron, 2 = prompt muon, 3 = tau->e decay, 4 = tau->mu decay, 5 = hadronic tau decay, 0 = unknown or unmatched
----------------------------------------------------------------------------------------------------
Electron_genPartIdx - Vector < Int_t >
Description: Index into genParticle list for MC matching to status==1 electrons or photons
----------------------------------------------------------------------------------------------------
Electron_genPartFlav - Vector < UChar_t >
Description: Flavour of genParticle (DressedLeptons for electrons) for MC matching to status==1 electrons or photons: 1 = prompt electron (including gamma*->mu mu), 15 = electron from prompt tau, 22 = prompt photon (likely conversion), 5 = electron from b, 4 = electron from c, 3 = electron from light or unknown, 0 = unmatched
----------------------------------------------------------------------------------------------------
FatJet_genJetAK8Idx - Vector < Int_t >
Description: index of matched gen AK8 jet
----------------------------------------------------------------------------------------------------
FatJet_hadronFlavour - Vector < Int_t >
Description: flavour from hadron ghost clustering
----------------------------------------------------------------------------------------------------
FatJet_nBHadrons - Vector < UChar_t >
Description: number of b-hadrons
----------------------------------------------------------------------------------------------------
FatJet_nCHadrons - Vector < UChar_t >
Description: number of c-hadrons
----------------------------------------------------------------------------------------------------
GenJetAK8_partonFlavour - Vector < Int_t >
Description: flavour from parton matching
----------------------------------------------------------------------------------------------------
GenJetAK8_hadronFlavour - Vector < UChar_t >
Description: flavour from hadron ghost clustering
----------------------------------------------------------------------------------------------------
GenJet_partonFlavour - Vector < Int_t >
Description: flavour from parton matching
----------------------------------------------------------------------------------------------------
GenJet_hadronFlavour - Vector < UChar_t >
Description: flavour from hadron ghost clustering
----------------------------------------------------------------------------------------------------
GenVtx_t0 - Value < Float_t >
Description: gen vertex t0
----------------------------------------------------------------------------------------------------
Jet_genJetIdx - Vector < Int_t >
Description: index of matched gen jet
----------------------------------------------------------------------------------------------------
Jet_hadronFlavour - Vector < Int_t >
Description: flavour from hadron ghost clustering
----------------------------------------------------------------------------------------------------
Jet_partonFlavour - Vector < Int_t >
Description: flavour from parton matching
----------------------------------------------------------------------------------------------------
LowPtElectron_genPartIdx - Vector < Int_t >
Description: Index into genParticle list for MC matching to status==1 electrons or photons
----------------------------------------------------------------------------------------------------
LowPtElectron_genPartFlav - Vector < UChar_t >
Description: Flavour of genParticle (DressedLeptons for electrons) for MC matching to status==1 electrons or photons: 1 = prompt electron (including gamma*->mu mu), 15 = electron from prompt tau, 22 = prompt photon (likely conversion), 5 = electron from b, 4 = electron from c, 3 = electron from light or unknown, 0 = unmatched
----------------------------------------------------------------------------------------------------
Muon_genPartIdx - Vector < Int_t >
Description: Index into genParticle list for MC matching to status==1 muons
----------------------------------------------------------------------------------------------------
Muon_genPartFlav - Vector < UChar_t >
Description: Flavour of genParticle for MC matching to status==1 muons: 1 = prompt muon (including gamma*->mu mu), 15 = muon from prompt tau, 5 = muon from b, 4 = muon from c, 3 = muon from light or unknown, 0 = unmatched
----------------------------------------------------------------------------------------------------
Photon_genPartIdx - Vector < Int_t >
Description: Index into genParticle list for MC matching to status==1 photons or electrons
----------------------------------------------------------------------------------------------------
Photon_genPartFlav - Vector < UChar_t >
Description: Flavour of genParticle for MC matching to status==1 photons or electrons: 1 = prompt photon, 11 = prompt electron, 0 = unknown or unmatched
----------------------------------------------------------------------------------------------------
MET_fiducialGenPhi - Value < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
MET_fiducialGenPt - Value < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
Electron_cleanmask - Vector < UChar_t >
Description: simple cleaning mask with priority to leptons
----------------------------------------------------------------------------------------------------
Jet_cleanmask - Vector < UChar_t >
Description: simple cleaning mask with priority to leptons
----------------------------------------------------------------------------------------------------
Muon_cleanmask - Vector < UChar_t >
Description: simple cleaning mask with priority to leptons
----------------------------------------------------------------------------------------------------
Photon_cleanmask - Vector < UChar_t >
Description: simple cleaning mask with priority to leptons
----------------------------------------------------------------------------------------------------
Tau_cleanmask - Vector < UChar_t >
Description: simple cleaning mask with priority to leptons
----------------------------------------------------------------------------------------------------
SubJet_hadronFlavour - Vector < Int_t >
Description: flavour from hadron ghost clustering
----------------------------------------------------------------------------------------------------
SubJet_nBHadrons - Vector < UChar_t >
Description: number of b-hadrons
----------------------------------------------------------------------------------------------------
SubJet_nCHadrons - Vector < UChar_t >
Description: number of c-hadrons
----------------------------------------------------------------------------------------------------
SV_chi2 - Vector < Float_t >
Description: reduced chi2, i.e. chi/ndof
----------------------------------------------------------------------------------------------------
SV_eta - Vector < Float_t >
Description: eta
----------------------------------------------------------------------------------------------------
SV_mass - Vector < Float_t >
Description: mass
----------------------------------------------------------------------------------------------------
SV_ndof - Vector < Float_t >
Description: number of degrees of freedom
----------------------------------------------------------------------------------------------------
SV_phi - Vector < Float_t >
Description: phi
----------------------------------------------------------------------------------------------------
SV_pt - Vector < Float_t >
Description: pt
----------------------------------------------------------------------------------------------------
SV_x - Vector < Float_t >
Description: secondary vertex X position, in cm
----------------------------------------------------------------------------------------------------
SV_y - Vector < Float_t >
Description: secondary vertex Y position, in cm
----------------------------------------------------------------------------------------------------
SV_z - Vector < Float_t >
Description: secondary vertex Z position, in cm
----------------------------------------------------------------------------------------------------
SV_ntracks - Vector < UChar_t >
Description: number of tracks
----------------------------------------------------------------------------------------------------
Tau_genPartIdx - Vector < Int_t >
Description: Index into genParticle list for MC matching to status==2 taus
----------------------------------------------------------------------------------------------------
Tau_genPartFlav - Vector < UChar_t >
Description: Flavour of genParticle for MC matching to status==2 taus: 1 = prompt electron, 2 = prompt muon, 3 = tau->e decay, 4 = tau->mu decay, 5 = hadronic tau decay, 0 = unknown or unmatched
----------------------------------------------------------------------------------------------------
L1_AlwaysTrue - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_AND_Ref1_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_AND_Ref3_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_AND_Ref4_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_BeamGas_B1_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_BeamGas_B2_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_BeamGas_Ref1_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_BeamGas_Ref2_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_NotOR_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_OR_Ref3_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_OR_Ref4_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BPTX_RefAND_VME - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BptxMinus - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BptxOR - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BptxPlus - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_BptxXOR - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_CDC_SingleMu_3_er1p2_TOP120_DPHI2p618_3p142 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG6_HTT240er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG6_HTT250er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG6_HTT255er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG6_HTT270er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG6_HTT300er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG8er2p6_HTT255er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG8er2p6_HTT270er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG8er2p6_HTT300er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_15_10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_18_17 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_20_18 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_22_10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_22_12 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_22_15 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_23_10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_24_17 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_25_12 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_25_13 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_25_14 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_LooseIso23_10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleEG_LooseIso24_10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleIsoTau28er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleIsoTau30er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleIsoTau32er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleIsoTau33er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleIsoTau34er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleIsoTau35er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleIsoTau36er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleIsoTau38er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet100er2p3_dEta_Max1p6 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet100er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet112er2p3_dEta_Max1p6 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet112er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet120er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet150er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet30_Mass_Min300_dEta_Max1p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet30_Mass_Min320_dEta_Max1p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet30_Mass_Min340_dEta_Max1p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet30_Mass_Min360_dEta_Max1p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet30_Mass_Min380_dEta_Max1p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet30_Mass_Min400_Mu10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet30_Mass_Min400_Mu6 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet30_Mass_Min400_dEta_Max1p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet35_rmovlp_IsoTau45_Mass_Min450 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet40er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet50er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet60er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet60er2p7_ETM100 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet60er2p7_ETM60 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet60er2p7_ETM70 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet60er2p7_ETM80 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet60er2p7_ETM90 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet80er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet_100_30_DoubleJet30_Mass_Min620 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet_100_35_DoubleJet35_Mass_Min620 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet_110_35_DoubleJet35_Mass_Min620 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet_110_40_DoubleJet40_Mass_Min620 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet_115_35_DoubleJet35_Mass_Min620 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet_115_40_DoubleJet40_Mass_Min620 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleJet_90_30_DoubleJet30_Mass_Min620 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleLooseIsoEG22er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleLooseIsoEG24er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0_ETM40 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0_ETM55 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0_ETM60 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0_ETM65 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0_ETM70 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0_SQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0_SQ_OS - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0er1p4_SQ_OS_dR_Max1p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0er1p4_dEta_Max1p8_OS - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0er1p5_SQ_OS - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0er1p5_SQ_OS_dR_Max1p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0er1p5_SQ_dR_Max1p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu0er2_SQ_dR_Max1p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu18er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu22er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_OS_DoubleEG7p5Upsilon - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_SQ_ETMHF40_Jet60_OR_DoubleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_SQ_ETMHF50_Jet60_OR_DoubleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_SQ_ETMHF60_Jet60_OR_DoubleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_SQ_ETMHF70_Jet60_OR_DoubleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_SQ_ETMHF80_Jet60_OR_DoubleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_SQ_HTT100er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_SQ_HTT200er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_SQ_HTT220er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu3_SQ_HTT240er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu4_OS_EG12 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu4_SQ_OS - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu4_SQ_OS_dR_Max1p2 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu4p5_SQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu4p5_SQ_OS - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu4p5_SQ_OS_dR_Max1p2 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu4p5er2p0_SQ_OS - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu4p5er2p0_SQ_OS_Mass7to18 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu5Upsilon_OS_DoubleEG3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu5_OS_EG12 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu5_SQ_OS - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu5_SQ_OS_Mass7to18 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu6_SQ_OS - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu7_EG7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu7_SQ_EG7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu8_SQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_10_0_dEta_Max1p8 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_11_4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_12_5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_12_8 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_13_6 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_15_5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_15_5_SQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_15_7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_15_7_SQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_15_7_SQ_Mass_Min4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleMu_20_2_SQ_Mass_Max20 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleTau50er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_DoubleTau70er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_EG25er2p1_HTT125er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_EG27er2p1_HTT200er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM100 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM100_Jet60_dPhi_Min0p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM105 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM110 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM110_Jet60_dPhi_Min0p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM115 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM120 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM150 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM40 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM50 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM60 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM70 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM75 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM75_Jet60_dPhi_Min0p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM80 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM80_Jet60_dPhi_Min0p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM85 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM90 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM90_Jet60_dPhi_Min0p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETM95 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF100 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF100_HTT60er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF100_Jet60_OR_DiJet30woTT28 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF100_Jet60_OR_DoubleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF100_Jet90_OR_DoubleJet45_OR_TripleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF110 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF110_HTT60er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF110_Jet60_OR_DiJet30woTT28 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF110_Jet90_OR_DoubleJet45_OR_TripleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF120 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF120_HTT60er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF120_Jet60_OR_DiJet30woTT28 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF150 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF70 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF70_Jet90_OR_DoubleJet45_OR_TripleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF80 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF80_HTT60er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF80_Jet90_OR_DoubleJet45_OR_TripleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF90 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF90_HTT60er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETMHF90_Jet90_OR_DoubleJet45_OR_TripleJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT100_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT110_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT40_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT50_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT60_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT70_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT75_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT80_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT85_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT90_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ETT95_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_FirstBunchAfterTrain - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_FirstBunchInTrain - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_FirstCollisionInOrbit - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_FirstCollisionInTrain - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT120er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT160er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT200er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT220er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT240er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT250er_QuadJet_70_55_40_35_er2p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT255er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT270er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT280er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT280er_QuadJet_70_55_40_35_er2p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT300er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT300er_QuadJet_70_55_40_35_er2p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT320er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT320er_QuadJet_70_55_40_40_er2p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT320er_QuadJet_70_55_40_40_er2p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT320er_QuadJet_70_55_45_45_er2p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT340er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT340er_QuadJet_70_55_40_40_er2p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT340er_QuadJet_70_55_45_45_er2p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT380er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT400er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT450er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_HTT500er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoEG33_Mt40 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoEG33_Mt44 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoEG33_Mt48 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETM100 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETM105 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETM110 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETM115 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETM120 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETM80 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETM85 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETM90 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETM95 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETMHF100 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETMHF110 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETMHF120 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETMHF80 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsoTau40er_ETMHF90 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_IsolatedBunch - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LastCollisionInTrain - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG22er2p1_IsoTau26er2p1_dR_Min0p3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG24er2p1_HTT100er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG24er2p1_IsoTau27er2p1_dR_Min0p3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG24er2p1_Jet26er2p7_dR_Min0p3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG24er2p1_TripleJet_26er2p7_26_26er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG26er2p1_HTT100er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG26er2p1_Jet34er2p7_dR_Min0p3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG28er2p1_HTT100er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG28er2p1_Jet34er2p7_dR_Min0p3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_LooseIsoEG30er2p1_Jet34er2p7_dR_Min0p3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_MU20_EG15 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_MinimumBiasHF0_AND_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_MinimumBiasHF0_OR_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu10er2p1_ETM30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu10er2p3_Jet32er2p3_dR_Max0p4_DoubleJet32er2p3_dEta_Max1p6 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu12_EG10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu12er2p3_Jet40er2p3_dR_Max0p4_DoubleJet40er2p3_dEta_Max1p6 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu14er2p1_ETM30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu15_HTT100er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu18_HTT100er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu18_Jet24er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu18er2p1_IsoTau26er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu18er2p1_Tau24er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu20_EG10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu20_EG17 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu20_LooseIsoEG6 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu20er2p1_IsoTau26er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu20er2p1_IsoTau27er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_IsoTau28er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_IsoTau30er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_IsoTau32er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_IsoTau33er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_IsoTau34er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_IsoTau35er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_IsoTau36er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_IsoTau38er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_IsoTau40er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_Tau50er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu22er2p1_Tau70er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu23_EG10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu23_LooseIsoEG10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu3_Jet120er2p7_dEta_Max0p4_dPhi_Max0p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu3_Jet16er2p7_dEta_Max0p4_dPhi_Max0p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu3_Jet30er2p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu3_Jet60er2p7_dEta_Max0p4_dPhi_Max0p4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu5_EG15 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu5_EG20 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu5_EG23 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu5_LooseIsoEG18 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu5_LooseIsoEG20 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu6_DoubleEG10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu6_DoubleEG17 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu6_HTT200er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu6_HTT240er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu6_HTT250er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu7_EG23 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu7_LooseIsoEG20 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu7_LooseIsoEG23 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_Mu8_HTT150er - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_NotBptxOR - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_QuadJet36er2p7_IsoTau52er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_QuadJet36er2p7_Tau52 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_QuadJet40er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_QuadJet50er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_QuadJet60er2p7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_QuadMu0 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG10 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG15 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG18 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG24 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG26 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG28 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG2_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG32 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG34 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG34er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG36 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG36er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG38 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG38er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG40 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG42 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG45 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleEG50 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG18 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG18er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG20 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG20er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG22 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG22er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG24 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG24er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG26 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG26er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG28 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG28er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG30er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG32 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG32er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG33er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG34 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG34er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG35 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG35er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG36 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG36er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG37 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG38 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG38er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG40 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleIsoEG40er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet120 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet120_FWD - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet12_BptxAND - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet140 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet150 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet16 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet160 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet170 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet180 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet20 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet200 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet20er2p7_NotBptxOR - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet20er2p7_NotBptxOR_3BX - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet35 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet35_FWD - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet35_HFm - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet35_HFp - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet43er2p7_NotBptxOR_3BX - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet46er2p7_NotBptxOR_3BX - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet60 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet60_FWD - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet60_HFm - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet60_HFp - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet90 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleJet90_FWD - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu0_BMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu0_EMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu0_OMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu10_LowQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu11_LowQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu12_LowQ_BMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu12_LowQ_EMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu12_LowQ_OMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu14er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu16 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu16er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu18 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu18er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu20 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu20er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu22 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu22_BMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu22_EMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu22_OMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu22er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu25 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu30 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMu7 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMuCosmics - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMuCosmics_BMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMuCosmics_EMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMuCosmics_OMTF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMuOpen - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMuOpen_NotBptxOR - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleMuOpen_NotBptxOR_3BX - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleTau100er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleTau120er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleTau130er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleTau140er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleTau20 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_SingleTau80er2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleEG_14_10_8 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleEG_18_17_8 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleEG_LooseIso20_10_5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleJet_100_85_72_VBF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleJet_105_85_76_VBF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleJet_84_68_48_VBF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleJet_88_72_56_VBF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleJet_92_76_64_VBF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleJet_98_83_71_VBF - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu0 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu0_OQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu3_SQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_4_4_4 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5OQ_3p5OQ_2p5OQ_DoubleMu_5_2p5_OQ_OS_Mass_5to17 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5OQ_3p5OQ_2p5OQ_DoubleMu_5_2p5_OQ_OS_Mass_8to14 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5SQ_3SQ_0OQ - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5SQ_3SQ_0OQ_DoubleMu_5_3_SQ_OS_Mass_Max9 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5SQ_3SQ_0_DoubleMu_5_3_SQ_OS_Mass_Max9 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5_0_0 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5_3_3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5_3p5_2p5 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5_3p5_2p5_DoubleMu_5_2p5_OS_Mass_5to17 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5_4_2p5_DoubleMu_5_2p5_OS_Mass_5to17 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_TripleMu_5_5_3 - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_UnpairedBunchBptxMinus - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_UnpairedBunchBptxPlus - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ZeroBias - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_ZeroBias_copy - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
L1_UnprefireableEvent - Value < Bool_t >
Description: Trigger/flag bit (process: NANO)
----------------------------------------------------------------------------------------------------
Flag_HBHENoiseFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_HBHENoiseIsoFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_CSCTightHaloFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_CSCTightHaloTrkMuUnvetoFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_CSCTightHalo2015Filter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_globalTightHalo2016Filter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_globalSuperTightHalo2016Filter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_HcalStripHaloFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_hcalLaserEventFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_EcalDeadCellTriggerPrimitiveFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_EcalDeadCellBoundaryEnergyFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_ecalBadCalibFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_goodVertices - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_eeBadScFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_ecalLaserCorrFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_trkPOGFilters - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_chargedHadronTrackResolutionFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_muonBadTrackFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_BadChargedCandidateFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_BadPFMuonFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_BadPFMuonDzFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_hfNoisyHitsFilter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_BadChargedCandidateSummer16Filter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_BadPFMuonSummer16Filter - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_trkPOG_manystripclus53X - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_trkPOG_toomanystripclus53X - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_trkPOG_logErrorTooManyClusters - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
Flag_METFilters - Value < Bool_t >
Description: Trigger/flag bit (process: PAT)
----------------------------------------------------------------------------------------------------
L1Reco_step - Value < Bool_t >
Description: Trigger/flag bit (process: RECO)
----------------------------------------------------------------------------------------------------
HLTriggerFirstPath - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet360_TrimMass30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet380_TrimMass30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet400_TrimMass30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet420_TrimMass30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFHT750_TrimMass50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFHT800_TrimMass50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFHT850_TrimMass50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFHT900_TrimMass50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloJet500_NoJetID - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloJet550_NoJetID - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Trimuon5_3p5_2_Upsilon_Muon - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleEle25_CaloIdL_MW - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleEle27_CaloIdL_MW - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleEle33_CaloIdL_MW - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleEle24_eta2p1_WPTight_Gsf - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_PFHT350 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele27_Ele37_CaloIdL_MW - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu27_Ele37_CaloIdL_MW - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu37_Ele27_CaloIdL_MW - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu37_TkMu27 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu4_3_Bs - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu4_3_Jpsi_Displaced - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu4_JpsiTrk_Displaced - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu4_LowMassNonResonantTrk_Displaced - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu3_Trk_Tau3mu - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu4_PsiPrimeTrk_Displaced - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu4_Mass8_DZ_PFHT350 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu8_Mass8_PFHT350 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu3_PFJet40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu7p5_L2Mu2_Jpsi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu7p5_L2Mu2_Upsilon - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu7p5_Track2_Jpsi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu7p5_Track3p5_Jpsi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu7p5_Track7_Jpsi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu7p5_Track2_Upsilon - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu7p5_Track3p5_Upsilon - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu7p5_Track7_Upsilon - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePhoton33_CaloIdL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePhoton70 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePhoton85 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele20_WPTight_Gsf - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele20_WPLoose_Gsf - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele20_eta2p1_WPLoose_Gsf - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiEle27_WPTightCaloOnly_L1DoubleEG - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele27_WPTight_Gsf - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele32_WPTight_Gsf - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele35_WPTight_Gsf - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele35_WPTight_Gsf_L1EGMT - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele38_WPTight_Gsf - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele40_WPTight_Gsf - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele32_WPTight_Gsf_L1DoubleEG - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT450_Beamspot - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT300_Beamspot - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu20_eta2p1_LooseChargedIsoPFTau27_eta2p1_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu20_eta2p1_MediumChargedIsoPFTau27_eta2p1_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu20_eta2p1_TightChargedIsoPFTau27_eta2p1_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu20_eta2p1_LooseChargedIsoPFTau27_eta2p1_TightID_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu20_eta2p1_MediumChargedIsoPFTau27_eta2p1_TightID_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu20_eta2p1_TightChargedIsoPFTau27_eta2p1_TightID_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_LooseChargedIsoPFTau20_SingleL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_MediumChargedIsoPFTau20_SingleL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_TightChargedIsoPFTau20_SingleL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_LooseChargedIsoPFTau20_TightID_SingleL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_MediumChargedIsoPFTau20_TightID_SingleL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_TightChargedIsoPFTau20_TightID_SingleL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu20 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu27 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_UncorrectedJetE30_NoBPTX - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_UncorrectedJetE30_NoBPTX3BX - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_UncorrectedJetE60_NoBPTX3BX - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_UncorrectedJetE70_NoBPTX3BX - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1SingleMu18 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1SingleMu25 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L2Mu10 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L2Mu10_NoVertex_NoBPTX3BX - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L2Mu10_NoVertex_NoBPTX - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L2Mu45_NoVertex_3Sta_NoBPTX3BX - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L2Mu40_NoVertex_3Sta_NoBPTX3BX - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L2Mu50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleL2Mu50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass8 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu25_TkMu0_Onia - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu30_TkMu0_Onia - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu20_TkMu0_Phi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu25_TkMu0_Phi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu20 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu27 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu55 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_OldMu100 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TkMu100 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJet15_NoCaloMatched - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJet25_NoCaloMatched - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJet15_FBEta3_NoCaloMatched - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJet25_FBEta3_NoCaloMatched - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve80 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve140 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve200 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve260 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve320 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve400 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve500 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve15_HFJEC - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve25_HFJEC - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve35_HFJEC - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve60_HFJEC - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve80_HFJEC - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve100_HFJEC - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve160_HFJEC - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve220_HFJEC - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiPFJetAve300_HFJEC - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet80 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet140 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet200 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet260 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet320 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet400 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet450 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet500 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet550 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet80 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet140 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet200 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet260 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet320 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet400 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet450 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet500 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJet550 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd80 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd140 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd200 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd260 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd320 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd400 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd450 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFJetFwd500 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd80 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd140 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd200 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd260 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd320 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd400 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd450 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJetFwd500 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT180 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT250 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT370 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT430 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT510 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT590 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT680 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT780 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT890 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT1050 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT500_PFMET100_PFMHT100_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT500_PFMET110_PFMHT110_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT700_PFMET85_PFMHT85_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT700_PFMET95_PFMHT95_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT800_PFMET75_PFMHT75_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT800_PFMET85_PFMHT85_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET110_PFMHT110_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET120_PFMHT120_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET130_PFMHT130_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET140_PFMHT140_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET100_PFMHT100_IDTight_CaloBTagCSV_3p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET110_PFMHT110_IDTight_CaloBTagCSV_3p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET120_PFMHT120_IDTight_CaloBTagCSV_3p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET130_PFMHT130_IDTight_CaloBTagCSV_3p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET140_PFMHT140_IDTight_CaloBTagCSV_3p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET120_PFMHT120_IDTight_PFHT60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_PFHT60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETTypeOne120_PFMHT120_IDTight_PFHT60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETTypeOne110_PFMHT110_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETTypeOne120_PFMHT120_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETTypeOne130_PFMHT130_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETTypeOne140_PFMHT140_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETNoMu110_PFMHTNoMu110_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETNoMu120_PFMHTNoMu120_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETNoMu130_PFMHTNoMu130_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETNoMu140_PFMHTNoMu140_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MonoCentralPFJet80_PFMETNoMu110_PFMHTNoMu110_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MonoCentralPFJet80_PFMETNoMu120_PFMHTNoMu120_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MonoCentralPFJet80_PFMETNoMu130_PFMHTNoMu130_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MonoCentralPFJet80_PFMETNoMu140_PFMHTNoMu140_IDTight - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1ETMHadSeeds - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMHT90 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET80_NotCleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET90_NotCleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET100_NotCleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET110_NotCleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET250_NotCleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET70_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET80_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET90_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET100_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET250_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET300_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_CaloMET350_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET200_NotCleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET200_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET250_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET300_HBHECleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET200_HBHE_BeamHaloCleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETTypeOne200_HBHE_BeamHaloCleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MET105_IsoTrk50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MET120_IsoTrk50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_SingleJet30_Mu12_SinglePFJet40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_DoublePFJets40_CaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_DoublePFJets100_CaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_DoublePFJets200_CaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_DoublePFJets350_CaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_DoublePFJets40MaxDeta1p6_DoubleCaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_DoublePFJets54MaxDeta1p6_DoubleCaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_DoublePFJets62MaxDeta1p6_DoubleCaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePFJets40_CaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePFJets100_CaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePFJets200_CaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePFJets350_CaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePFJets116MaxDeta1p6_DoubleCaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoublePFJets128MaxDeta1p6_DoubleCaloBTagCSV_p33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon300_NoHE - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu8_TrkIsoVVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu8_DiEle12_CaloIdL_TrackIdL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu17_TrkIsoVVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu19_TrkIsoVVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_BTagMu_AK4DiJet20_Mu5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_BTagMu_AK4DiJet40_Mu5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_BTagMu_AK4DiJet70_Mu5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_BTagMu_AK4DiJet110_Mu5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_BTagMu_AK4DiJet170_Mu5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_BTagMu_AK4Jet300_Mu5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_BTagMu_AK8DiJet170_Mu5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_BTagMu_AK8Jet300_Mu5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu12_DoublePhoton20 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TriplePhoton_20_20_20_CaloIdLV2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TriplePhoton_20_20_20_CaloIdLV2_R9IdVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TriplePhoton_30_30_10_CaloIdLV2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TriplePhoton_30_30_10_CaloIdLV2_R9IdVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TriplePhoton_35_35_5_CaloIdLV2_R9IdVL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon25 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon33 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon75 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon90 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon120 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon150 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon175 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon200 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon50_R9Id90_HE10_IsoM - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon75_R9Id90_HE10_IsoM - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon90_R9Id90_HE10_IsoM - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon120_R9Id90_HE10_IsoM - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon165_R9Id90_HE10_IsoM - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon90_CaloIdL_PFHT700 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass95 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Diphoton30PV_18PV_R9Id_AND_IsoCaloId_AND_HE_R9Id_PixelVeto_Mass55 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Diphoton30PV_18PV_R9Id_AND_IsoCaloId_AND_HE_R9Id_NoPixelVeto_Mass55 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Diphoton30EB_18EB_R9Id_OR_IsoCaloId_AND_HE_R9Id_NoPixelVeto_Mass55 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Diphoton30EB_18EB_R9Id_OR_IsoCaloId_AND_HE_R9Id_PixelVeto_Mass55 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Jpsi_L1_NoOS - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Jpsi_NoVertexing_NoOS - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Jpsi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Jpsi_NoVertexing - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Jpsi_L1_4R_0er1p5R - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Jpsi_NoVertexing_L1_4R_0er1p5R - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Jpsi3p5_Muon2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Upsilon_L1_4p5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Upsilon_L1_5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Upsilon_L1_4p5NoOS - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Upsilon_L1_4p5er2p0 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Upsilon_L1_4p5er2p0M - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Upsilon_NoVertexing - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Upsilon_L1_5M - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_LowMass_L1_0er1p5R - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_LowMass_L1_0er1p5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_LowMass - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_LowMass_L1_4 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_LowMass_L1_4R - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_LowMass_L1_TM530 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Upsilon_Muon_L1_TM0 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon0_Upsilon_Muon_NoL1Mass - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TripleMu_5_3_3_Mass3p8to60_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TripleMu_10_5_5_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TripleMu_12_10_5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Tau3Mu_Mu7_Mu1_TkMu1_Tau15 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Tau3Mu_Mu7_Mu1_TkMu1_Tau15_Charge1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Tau3Mu_Mu7_Mu1_TkMu1_IsoTau15 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Tau3Mu_Mu7_Mu1_TkMu1_IsoTau15_Charge1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu3_DZ_PFMET50_PFMHT60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu3_DZ_PFMET70_PFMHT70 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu3_DZ_PFMET90_PFMHT90 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu3_Trk_Tau3mu_NoL1Mass - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu4_Jpsi_Displaced - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu4_Jpsi_NoVertexing - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu4_JpsiTrkTrk_Displaced - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu43NoFiltersNoVtx - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu48NoFiltersNoVtx - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu43NoFiltersNoVtx_Photon43_CaloIdL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu48NoFiltersNoVtx_Photon48_CaloIdL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu20_7_Mass0to30_L1_DM4 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu20_7_Mass0to30_L1_DM4EG - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT425 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT430_DisplacedDijet40_DisplacedTrack - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT430_DisplacedDijet60_DisplacedTrack - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT430_DisplacedDijet80_DisplacedTrack - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT400_DisplacedDijet40_DisplacedTrack - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT650_DisplacedDijet60_Inclusive - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT550_DisplacedDijet80_Inclusive - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT550_DisplacedDijet60_Inclusive - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT650_DisplacedDijet80_Inclusive - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HT750_DisplacedDijet80_Inclusive - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiJet110_35_Mjj650_PFMET110 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiJet110_35_Mjj650_PFMET120 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiJet110_35_Mjj650_PFMET130 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TripleJet110_35_35_Mjj650_PFMET110 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TripleJet110_35_35_Mjj650_PFMET120 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TripleJet110_35_35_Mjj650_PFMET130 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_VBF_DoubleLooseChargedIsoPFTau20_Trk1_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_VBF_DoubleMediumChargedIsoPFTau20_Trk1_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_VBF_DoubleTightChargedIsoPFTau20_Trk1_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele30_eta2p1_WPTight_Gsf_CentralPFJet35_EleCleaned - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele28_eta2p1_WPTight_Gsf_HT150 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele28_HighEta_SC20_Mass55 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu20_7_Mass0to30_Photon23 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele15_IsoVVVL_PFHT450_CaloBTagCSV_4p5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele15_IsoVVVL_PFHT450_PFMET50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele15_IsoVVVL_PFHT450 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele50_IsoVVVL_PFHT450 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele15_IsoVVVL_PFHT600 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu8_TrkIsoVVL_DiPFJet40_DEta3p5_MJJ750_HTT300_PFMETNoMu60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu10_TrkIsoVVL_DiPFJet40_DEta3p5_MJJ750_HTT350_PFMETNoMu60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu15_IsoVVVL_PFHT450_CaloBTagCSV_4p5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu15_IsoVVVL_PFHT450_PFMET50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu15_IsoVVVL_PFHT450 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu50_IsoVVVL_PFHT450 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu15_IsoVVVL_PFHT600 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon10_PsiPrime_Barrel_Seagulls - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon20_Jpsi_Barrel_Seagulls - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon10_Upsilon_Barrel_Seagulls - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon12_Upsilon_eta1p5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon14_Phi_Barrel_Seagulls - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon18_PsiPrime - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon25_Jpsi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon18_PsiPrime_noCorrL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon24_Upsilon_noCorrL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon24_Phi_noCorrL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Dimuon25_Jpsi_noCorrL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiMu9_Ele9_CaloIdL_TrackIdL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleIsoMu20_eta2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleIsoMu24_eta2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TrkMu12_DoubleTrkMu5NoFiltersNoVtx - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TrkMu16_DoubleTrkMu6NoFiltersNoVtx - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TrkMu17_DoubleTrkMu8NoFiltersNoVtx - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu8 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu17 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu19 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu17_Photon30_IsoCaloId - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele8_CaloIdL_TrackIdL_IsoVL_PFJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele12_CaloIdL_TrackIdL_IsoVL_PFJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele23_CaloIdL_TrackIdL_IsoVL_PFJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele8_CaloIdM_TrackIdM_PFJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele17_CaloIdM_TrackIdM_PFJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele23_CaloIdM_TrackIdM_PFJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele50_CaloIdVT_GsfTrkIdT_PFJet165 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele115_CaloIdVT_GsfTrkIdT - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele135_CaloIdVT_GsfTrkIdT - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele145_CaloIdVT_GsfTrkIdT - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele200_CaloIdVT_GsfTrkIdT - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele250_CaloIdVT_GsfTrkIdT - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele300_CaloIdVT_GsfTrkIdT - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT300PT30_QuadPFJet_75_60_45_40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT300PT30_QuadPFJet_75_60_45_40_TriplePFBTagCSV_3p0 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT380_SixPFJet32_DoublePFBTagCSV_2p2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT380_SixPFJet32_DoublePFBTagDeepCSV_2p2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT380_SixPFJet32 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT430_SixPFJet40_PFBTagCSV_1p5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT430_SixPFJet40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT350 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFHT350MinPFJet15 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon60_R9Id90_CaloIdL_IsoL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon60_R9Id90_CaloIdL_IsoL_DisplacedIdL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon60_R9Id90_CaloIdL_IsoL_DisplacedIdL_PFHT350MinPFJet15 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_FullTrack_Multiplicity85 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_FullTrack_Multiplicity100 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_FullTrack_Multiplicity130 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_FullTrack_Multiplicity155 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ECALHT800 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DiSC30_18_EIso_AND_HE_Mass70 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Physics - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Physics_part0 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Physics_part1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Physics_part2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Physics_part3 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Physics_part4 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Physics_part5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Physics_part6 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Physics_part7 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Random - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_part0 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_part1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_part2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_part3 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_part4 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_part5 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_part6 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_part7 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4CaloJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4CaloJet40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4CaloJet50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4CaloJet80 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4CaloJet100 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4CaloJet120 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4PFJet30 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4PFJet50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4PFJet80 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4PFJet100 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK4PFJet120 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HISinglePhoton10_Eta3p1ForPPRef - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HISinglePhoton20_Eta3p1ForPPRef - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HISinglePhoton30_Eta3p1ForPPRef - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HISinglePhoton40_Eta3p1ForPPRef - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HISinglePhoton50_Eta3p1ForPPRef - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HISinglePhoton60_Eta3p1ForPPRef - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon20_HoverELoose - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon30_HoverELoose - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon40_HoverELoose - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon50_HoverELoose - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon60_HoverELoose - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_EcalCalibration - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HcalCalibration - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1UnpairedBunchBptxMinus - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1UnpairedBunchBptxPlus - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1NotBptxOR - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1MinimumBiasHF_OR - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1MinimumBiasHF0OR - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1_CDC_SingleMu_3_er1p2_TOP120_DPHI2p618_3p142 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HcalNZS - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HcalPhiSym - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_HcalIsolatedbunch - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoTrackHB - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoTrackHE - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_FirstCollisionAfterAbortGap - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_IsolatedBunches - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_FirstCollisionInTrain - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_LastCollisionInTrain - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_ZeroBias_FirstBXAfterTrain - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele24_eta2p1_WPTight_Gsf_LooseChargedIsoPFTau30_eta2p1_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele24_eta2p1_WPTight_Gsf_MediumChargedIsoPFTau30_eta2p1_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele24_eta2p1_WPTight_Gsf_TightChargedIsoPFTau30_eta2p1_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele24_eta2p1_WPTight_Gsf_LooseChargedIsoPFTau30_eta2p1_TightID_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele24_eta2p1_WPTight_Gsf_MediumChargedIsoPFTau30_eta2p1_TightID_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele24_eta2p1_WPTight_Gsf_TightChargedIsoPFTau30_eta2p1_TightID_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleLooseChargedIsoPFTau35_Trk1_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleLooseChargedIsoPFTau40_Trk1_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMediumChargedIsoPFTau35_Trk1_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMediumChargedIsoPFTau40_Trk1_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleTightChargedIsoPFTau35_Trk1_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleLooseChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleLooseChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMediumChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleTightChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_MediumChargedIsoPFTau50_Trk30_eta2p1_1pr - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MediumChargedIsoPFTau50_Trk30_eta2p1_1pr_MET90 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MediumChargedIsoPFTau50_Trk30_eta2p1_1pr_MET100 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MediumChargedIsoPFTau50_Trk30_eta2p1_1pr_MET110 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MediumChargedIsoPFTau50_Trk30_eta2p1_1pr_MET120 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MediumChargedIsoPFTau50_Trk30_eta2p1_1pr_MET130 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MediumChargedIsoPFTau50_Trk30_eta2p1_1pr - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1_1pr - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_LooseChargedIsoPFTau35_Trk1_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_LooseChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_MediumChargedIsoPFTau35_Trk1_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_MediumChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_TightChargedIsoPFTau35_Trk1_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_TightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_MediumChargedIsoPFTau40_Trk1_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_MediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_TightChargedIsoPFTau40_Trk1_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu24_eta2p1_TightChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg_CrossL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Rsq0p35 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Rsq0p40 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_RsqMR300_Rsq0p09_MR200 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_RsqMR320_Rsq0p09_MR200 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_RsqMR300_Rsq0p09_MR200_4jet - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_RsqMR320_Rsq0p09_MR200_4jet - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_L1_DoubleJet30_Mass_Min400_Mu10 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu27_LooseChargedIsoPFTau20_SingleL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu27_MediumChargedIsoPFTau20_SingleL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_IsoMu27_TightChargedIsoPFTau20_SingleL1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon50_R9Id90_HE10_IsoM_EBOnly_PFJetsMJJ300DEta3_PFMET50 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon75_R9Id90_HE10_IsoM_EBOnly_PFJetsMJJ300DEta3 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Photon75_R9Id90_HE10_IsoM_EBOnly_PFJetsMJJ600DEta3 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMET100_PFMHT100_IDTight_PFHT60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETNoMu100_PFMHTNoMu100_IDTight_PFHT60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_PFMETTypeOne100_PFMHT100_IDTight_PFHT60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu18_Mu9_SameSign - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu18_Mu9_SameSign_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu18_Mu9 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu18_Mu9_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu20_Mu10_SameSign - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu20_Mu10_SameSign_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu20_Mu10 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu20_Mu10_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu23_Mu12_SameSign - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu23_Mu12_SameSign_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu23_Mu12 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Mu23_Mu12_DZ - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu2_Jpsi_DoubleTrk1_Phi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu2_Jpsi_DoubleTkMu0_Phi - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_DoubleMu3_DCA_PFMET50_PFMHT60 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_TripleMu_5_3_3_Mass3p8to60_DCA - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet98_83_71_15_DoubleBTagCSV_p013_p08_VBF1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet103_88_75_15_DoubleBTagCSV_p013_p08_VBF1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet105_90_76_15_DoubleBTagCSV_p013_p08_VBF1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet111_90_80_15_DoubleBTagCSV_p013_p08_VBF1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet98_83_71_15_BTagCSV_p013_VBF2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet103_88_75_15_BTagCSV_p013_VBF2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet105_88_76_15_BTagCSV_p013_VBF2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet111_90_80_15_BTagCSV_p013_VBF2 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet98_83_71_15 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet103_88_75_15 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet105_88_76_15 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_QuadPFJet111_90_80_15 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet330_PFAK8BTagCSV_p17 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_AK8PFJet330_PFAK8BTagCSV_p1 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Diphoton30_18_PVrealAND_R9Id_AND_IsoCaloId_AND_HE_R9Id_PixelVeto_Mass55 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLT_Diphoton30_18_PVrealAND_R9Id_AND_IsoCaloId_AND_HE_R9Id_NoPixelVeto_Mass55 - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
HLTriggerFinalPath - Value < Bool_t >
Description: Trigger/flag bit (process: HLT)
----------------------------------------------------------------------------------------------------
L1simulation_step - Value < Bool_t >
Description: Trigger/flag bit (process: DIGI2RAW)
----------------------------------------------------------------------------------------------------






