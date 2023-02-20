### HLT Bits

### 2017

```
1 = CaloIdL_TrackIdL_IsoVL
--  2 = 1e (WPTight)
--  4 = 1e (WPLoose)
--  8 = OverlapFilter PFTau
--  16 = 2e
--  32 = 1e-1mu
--  64 = 1e-1tau
--  128 = 3e
--  256 = 2e-1mu
--  512 = 1e-2mu
--  1024 = 1e (32_L1DoubleEG_AND_L1SingleEGOr)
--  2048 = 1e (CaloIdVT_GsfTrkIdT)
--  4096 = 1e (PFJet)
--  8192 = 1e (Photon175_OR_Photon200) for Electron (PixelMatched e/gamma)

 1 = TrkIsoVVL
--  2 = Iso
--  4 = OverlapFilter PFTau
--  8 = 1mu
--  16 = 2mu
--  32 = 1mu-1e
--  64 = 1mu-1tau
--  128 = 3mu
--  256 = 2mu-1e
--  512 = 1mu-2e
--  1024 = 1mu (Mu50)
--  2048 = 1mu (Mu100) for Muon

 1 = LooseChargedIso
--  2 = MediumChargedIso
--  4 = TightChargedIso
--  8 = TightID OOSC photons
--  16 = HPS
--  32 = single-tau + tau+MET
--  64 = di-tau
--  128 = e-tau
--  256 = mu-tau
--  512 = VBF+di-tau for Tau

 Jet bits: bit 0 for VBF cross-cleaned from loose iso PFTau
--  bit 1 for hltBTagCaloCSVp087Triple
--  bit 2 for hltDoubleCentralJet90
--  bit 3 for hltDoublePFCentralJetLooseID90
--  bit 4 for hltL1sTripleJetVBFIorHTTIorDoubleJetCIorSingleJet
--  bit 5 for hltQuadCentralJet30
--  bit 6 for hltQuadPFCentralJetLooseID30
--  bit 7 for hltL1sQuadJetC50IorQuadJetC60IorHTT280IorHTT300IorHTT320IorTripleJet846848VBFIorTripleJet887256VBFIorTripleJet927664VBF or hltL1sQuadJetCIorTripleJetVBFIorHTT
--  bit 8 for hltQuadCentralJet45
--  bit 9 for hltQuadPFCentralJetLooseID45
--  bit 10  for hltL1sQuadJetC60IorHTT380IorHTT280QuadJetIorHTT300QuadJet or hltL1sQuadJetC50to60IorHTT280to500IorHTT250to340QuadJet bit 11 for hltBTagCaloCSVp05Double or hltBTagCaloDeepCSVp17Double
--  bit 12 for hltPFCentralJetLooseIDQuad30
--  bit 13 for hlt1PFCentralJetLooseID75
--  bit 14 for hlt2PFCentralJetLooseID60
--  bit 15 for hlt3PFCentralJetLooseID45
--  bit 16 for hlt4PFCentralJetLooseID40
--  bit 17 for hltBTagPFCSVp070Triple or hltBTagPFDeepCSVp24Triple or hltBTagPFDeepCSV4p5Triple  for Jet

 HT bits: bit 0 for hltL1sTripleJetVBFIorHTTIorDoubleJetCIorSingleJet
--  bit 1 for hltL1sQuadJetC50IorQuadJetC60IorHTT280IorHTT300IorHTT320IorTripleJet846848VBFIorTripleJet887256VBFIorTripleJet927664VBF or hltL1sQuadJetCIorTripleJetVBFIorHTT
--  bit 2 for hltL1sQuadJetC60IorHTT380IorHTT280QuadJetIorHTT300QuadJet or hltL1sQuadJetC50to60IorHTT280to500IorHTT250to340QuadJet
--  bit 3 for hltCaloQuadJet30HT300 or hltCaloQuadJet30HT320
--  bit 4 for hltPFCentralJetsLooseIDQuad30HT300 or hltPFCentralJetsLooseIDQuad30HT330 for HT

 MHT bits: bit 0 for hltCaloQuadJet30HT300 or hltCaloQuadJet30HT320
--  bit 1 for hltPFCentralJetsLooseIDQuad30HT300 or hltPFCentralJetsLooseIDQuad30HT330 for MHT
```

#### Muons

```
hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p07 -> HLT_IsoMu27 - bit: 8
hltL3crIsoL1sSingleMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p07 -> HLT_IsoMu24 - bit: 8
hltL3fL1sMu22Or25L1f0L2f10QL3Filtered50Q -> HLT_Mu50 - bit: 1024
hltL3fL1sMu22Or25L1f0L2f10QL3Filtered100Q -> HLT_OldMu100 - bit: 2048
hltL3fL1sMu25f0TkFiltered100Q -> HLT_OldMu100 - bit: 2048
```
