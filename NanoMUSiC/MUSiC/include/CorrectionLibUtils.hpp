#ifndef CORRECTIONLIB_UTILS_HPP
#define CORRECTIONLIB_UTILS_HPP

#include <map>
#include <string>
#include <string_view>

#include "Configs.hpp"

// correctionlib
// More info: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagCalibration
// More info: https://github.com/cms-nanoAOD/correctionlib
// More info: https://cms-nanoaod.github.io/correctionlib/index.html
// Instructions:
// https://indico.cern.ch/event/1096988/contributions/4615134/attachments/2346047/4000529/Nov21_btaggingSFjsons.pdf
#include <correction.h>

class CorrectionLibUtils
{
    using CorrectionlibRef_t = correction::Correction::Ref;

  public:
    const std::map<std::pair<std::string_view, Year>, std::pair<std::string, std::string>> correction_keys = {
        // PU
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"PU"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2016preVFP_UL/puWeights.json.gz",
          "Collisions16_UltraLegacy_goldenJSON"}},
        {{"PU"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2016postVFP_UL/puWeights.json.gz",
          "Collisions16_UltraLegacy_goldenJSON"}},
        {{"PU"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2017_UL/puWeights.json.gz",
          "Collisions17_UltraLegacy_goldenJSON"}},
        {{"PU"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2018_UL/puWeights.json.gz",
          "Collisions18_UltraLegacy_goldenJSON"}},

        // Muon Reconstruction SF
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonReco"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_TrackerMuons_DEN_genTracks"}},
        {{"MuonReco"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_TrackerMuons_DEN_genTracks"}},
        {{"MuonReco"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_TrackerMuons_DEN_genTracks"}},
        {{"MuonReco"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_TrackerMuons_DEN_genTracks"}},

        // Muon Reconstruction SF - HighPt
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonRecoHighPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_HighPt.json.gz",
          "NUM_GlobalMuons_DEN_TrackerMuonProbes"}},
        {{"MuonRecoHighPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_HighPt.json.gz",
          "NUM_GlobalMuons_DEN_TrackerMuonProbes"}},
        {{"MuonRecoHighPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_HighPt.json.gz",
          "NUM_GlobalMuons_DEN_TrackerMuonProbes"}},
        {{"MuonRecoHighPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_HighPt.json.gz",
          "NUM_GlobalMuons_DEN_TrackerMuonProbes"}},

        // Muon ID SF -  Low pT
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonIdLowPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_JPsi.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},
        {{"MuonIdLowPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_JPsi.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},
        {{"MuonIdLowPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_JPsi.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},
        {{"MuonIdLowPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_JPsi.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},

        // Muon ID SF - Medium pT
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonIdMediumPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},
        {{"MuonIdMediumPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},
        {{"MuonIdMediumPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},
        {{"MuonIdMediumPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},

        // Muon ID SF - High pT
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonIdHighPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_HighPt.json.gz",
          "NUM_HighPtID_DEN_GlobalMuonProbes"}},
        {{"MuonIdHighPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_HighPt.json.gz",
          "NUM_HighPtID_DEN_GlobalMuonProbes"}},
        {{"MuonIdHighPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_HighPt.json.gz",
          "NUM_HighPtID_DEN_GlobalMuonProbes"}},
        {{"MuonIdHighPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_HighPt.json.gz",
          "NUM_HighPtID_DEN_GlobalMuonProbes"}},

        // Muon Iso SF - Medium pT
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonIsoMediumPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_TightRelIso_DEN_TightIDandIPCut"}},
        {{"MuonIsoMediumPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_TightRelIso_DEN_TightIDandIPCut"}},
        {{"MuonIsoMediumPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_TightRelIso_DEN_TightIDandIPCut"}},
        {{"MuonIsoMediumPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_TightRelIso_DEN_TightIDandIPCut"}},

        // Muon Iso SF - High pT
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonIsoHighPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_HighPt.json.gz",
          "NUM_HighPtID_DEN_GlobalMuonProbes"}},
        {{"MuonIsoHighPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_HighPt.json.gz",
          "NUM_HighPtID_DEN_GlobalMuonProbes"}},
        {{"MuonIsoHighPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_HighPt.json.gz",
          "NUM_HighPtID_DEN_GlobalMuonProbes"}},
        {{"MuonIsoHighPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_HighPt.json.gz",
          "NUM_HighPtID_DEN_GlobalMuonProbes"}},

        // Muon Trigger SF  - Low Pt
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"SingleMuonLowPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight"}},
        {{"SingleMuonLowPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight"}},
        {{"SingleMuonLowPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight"}},
        {{"SingleMuonLowPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight"}},

        // Muon Trigger SF  - High Pt
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"SingleMuonHighPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_Mu50_or_TkMu50_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},
        {{"SingleMuonHighPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_Mu50_or_TkMu50_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},
        {{"SingleMuonHighPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},
        {{"SingleMuonHighPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},

        // MuonLowPt - Rochester Corrections
        // {{TYPE, YEAR}, {INPUT, DUMMY (leave empty)}},
        // {{"MuonLowPt"sv, Year::Run2016APV},
        //  {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016aUL.txt")),
        //   ""}},
        // {{"MuonLowPt"sv, Year::Run2016},
        //  {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016bUL.txt")),
        //   ""}},
        // {{"MuonLowPt"sv, Year::Run2017},
        //  {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2017UL.txt")),
        //   ""}},
        // {{"MuonLowPt"sv, Year::Run2018},
        //  {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2018UL.txt")),
        //   ""}},

        // Electrons
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"ElectronSF"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016preVFP_UL/electron.json.gz",
          "UL-Electron-ID-SF"}},
        {{"ElectronSF"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016postVFP_UL/electron.json.gz",
          "UL-Electron-ID-SF"}},
        {{"ElectronSF"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2017_UL/electron.json.gz",
          "UL-Electron-ID-SF"}},
        {{"ElectronSF"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2018_UL/electron.json.gz",
          "UL-Electron-ID-SF"}},

        // Photons
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"PhotonSF"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016preVFP_UL/photon.json.gz",
          "UL-Photon-ID-SF"}},
        {{"PhotonSF"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016postVFP_UL/photon.json.gz",
          "UL-Photon-ID-SF"}},
        {{"PhotonSF"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2017_UL/photon.json.gz",
          "UL-Photon-ID-SF"}},
        {{"PhotonSF"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2018_UL/photon.json.gz",
          "UL-Photon-ID-SF"}},

        // // PixelVeto
        // // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        // {{"PixelVetoSF"sv, Year::Run2016APV},
        //  {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016preVFP_UL/photon.json.gz",
        //   "UL-Photon-PixVeto-SF"}},
        // {{"PixelVetoSF"sv, Year::Run2016},
        //  {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016postVFP_UL/photon.json.gz",
        //   "UL-Photon-PixVeto-SF"}},
        // {{"PixelVetoSF"sv, Year::Run2017},
        //  {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2017_UL/photon.json.gz",
        //   "UL-Photon-PixVeto-SF"}},
        // {{"PixelVetoSF"sv, Year::Run2018},
        //  {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2018_UL/photon.json.gz",
        //   "UL-Photon-PixVeto-SF"}},

        // CSEV Veto
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"CSEVSF"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016preVFP_UL/photon.json.gz",
          "UL-Photon-CSEV-SF"}},
        {{"CSEVSF"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016postVFP_UL/photon.json.gz",
          "UL-Photon-CSEV-SF"}},
        {{"CSEVSF"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2017_UL/photon.json.gz",
          "UL-Photon-CSEV-SF"}},
        {{"CSEVSF"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2018_UL/photon.json.gz",
          "UL-Photon-CSEV-SF"}},

        // Tau Corrections
        // DeepTau ID VS Electron
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"TauVSe"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2016preVFP_UL/tau.json.gz",
          "DeepTau2017v2p1VSe"}},
        {{"TauVSe"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2016postVFP_UL/tau.json.gz",
          "DeepTau2017v2p1VSe"}},
        {{"TauVSe"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2017_UL/tau.json.gz",
          "DeepTau2017v2p1VSe"}},
        {{"TauVSe"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2018_UL/tau.json.gz",
          "DeepTau2017v2p1VSe"}},

        // DeepTau ID VS Jet
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"TauVSjet"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2016preVFP_UL/tau.json.gz",
          "DeepTau2017v2p1VSjet"}},
        {{"TauVSjet"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2016postVFP_UL/tau.json.gz",
          "DeepTau2017v2p1VSjet"}},
        {{"TauVSjet"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2017_UL/tau.json.gz",
          "DeepTau2017v2p1VSjet"}},
        {{"TauVSjet"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2018_UL/tau.json.gz",
          "DeepTau2017v2p1VSjet"}},

        // DeepTau ID VS Muon
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"TauVSmu"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2016preVFP_UL/tau.json.gz",
          "DeepTau2017v2p1VSmu"}},
        {{"TauVSmu"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2016postVFP_UL/tau.json.gz",
          "DeepTau2017v2p1VSmu"}},
        {{"TauVSmu"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2017_UL/tau.json.gz",
          "DeepTau2017v2p1VSmu"}},
        {{"TauVSmu"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2018_UL/tau.json.gz",
          "DeepTau2017v2p1VSmu"}},

        // Tau Energy Scale
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"TauEnergyScale"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2016preVFP_UL/tau.json.gz",
          "tau_energy_scale"}},
        {{"TauEnergyScale"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2016postVFP_UL/tau.json.gz",
          "tau_energy_scale"}},
        {{"TauEnergyScale"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2017_UL/tau.json.gz", "tau_energy_scale"}},
        {{"TauEnergyScale"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/TAU/2018_UL/tau.json.gz", "tau_energy_scale"}},

        // MET xy Pt Correction - Data
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"METXYCorrDataPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016preVFP_UL/met.json.gz",
          "pt_metphicorr_pfmet_data"}},
        {{"METXYCorrDataPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016postVFP_UL/met.json.gz",
          "pt_metphicorr_pfmet_data"}},
        {{"METXYCorrDataPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2017_UL/met.json.gz",
          "pt_metphicorr_pfmet_data"}},
        {{"METXYCorrDataPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2018_UL/met.json.gz",
          "pt_metphicorr_pfmet_data"}},

        // MET xy Phi Correction - Data
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"METXYCorrDataPhi"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016preVFP_UL/met.json.gz",
          "phi_metphicorr_pfmet_data"}},
        {{"METXYCorrDataPhi"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016postVFP_UL/met.json.gz",
          "phi_metphicorr_pfmet_data"}},
        {{"METXYCorrDataPhi"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2017_UL/met.json.gz",
          "phi_metphicorr_pfmet_data"}},
        {{"METXYCorrDataPhi"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2018_UL/met.json.gz",
          "phi_metphicorr_pfmet_data"}},

        // MET xy Pt Correction - MC
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"METXYCorrMCPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016preVFP_UL/met.json.gz",
          "pt_metphicorr_pfmet_mc"}},
        {{"METXYCorrMCPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016postVFP_UL/met.json.gz",
          "pt_metphicorr_pfmet_mc"}},
        {{"METXYCorrMCPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2017_UL/met.json.gz",
          "pt_metphicorr_pfmet_mc"}},
        {{"METXYCorrMCPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2018_UL/met.json.gz",
          "pt_metphicorr_pfmet_mc"}},

        // MET xy Phi Correction - MC
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"METXYCorrMCPhi"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016preVFP_UL/met.json.gz",
          "phi_metphicorr_pfmet_mc"}},
        {{"METXYCorrMCPhi"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016postVFP_UL/met.json.gz",
          "phi_metphicorr_pfmet_mc"}},
        {{"METXYCorrMCPhi"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2017_UL/met.json.gz",
          "phi_metphicorr_pfmet_mc"}},
        {{"METXYCorrMCPhi"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2018_UL/met.json.gz",
          "phi_metphicorr_pfmet_mc"}},

        // Jet Veto Map
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"JetVetoMap"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016preVFP_UL/jetvetomaps.json.gz",
          "Summer19UL16_V1"}},
        {{"JetVetoMap"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2016postVFP_UL/jetvetomaps.json.gz",
          "Summer19UL16_V1"}},
        {{"JetVetoMap"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2017_UL/jetvetomaps.json.gz",
          "Summer19UL17_V1"}},
        {{"JetVetoMap"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/2018_UL/jetvetomaps.json.gz",
          "Summer19UL18_V1"}},

        // btag scale factors - heavy jets
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"BTagSFbc"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2016preVFP_UL/btagging.json.gz",
          "deepJet_mujets"}},
        {{"BTagSFbc"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2016postVFP_UL/btagging.json.gz",
          "deepJet_mujets"}},
        {{"BTagSFbc"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2017_UL/btagging.json.gz",
          "deepJet_mujets"}},
        {{"BTagSFbc"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2018_UL/btagging.json.gz",
          "deepJet_mujets"}},

        // btag scale factors - light jets
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"BTagSFlight"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2016preVFP_UL/btagging.json.gz",
          "deepJet_incl"}},
        {{"BTagSFlight"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2016postVFP_UL/btagging.json.gz",
          "deepJet_incl"}},
        {{"BTagSFlight"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2017_UL/btagging.json.gz", "deepJet_incl"}},
        {{"BTagSFlight"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2018_UL/btagging.json.gz", "deepJet_incl"}},
    };

    auto make_correctionlib_ref(const std::string &key, const std::string &year) -> const CorrectionlibRef_t
    {
        if (correction_keys.count(std::make_pair<std::string_view, Year>(key, get_runyear(year))) == 0)
        {
            throw std::runtime_error(fmt::format("Could not find correction key: {}\n", key));
        }

        auto [sf_file, sf_key] = correction_keys.at({key, get_runyear(year)});
        return correction::CorrectionSet::from_file(sf_file)->at(sf_key);
    }
};

#endif // !CORRECTIONLIB_UTILS_HPP
