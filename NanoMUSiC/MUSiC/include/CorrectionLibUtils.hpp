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

        // Muon ID SF - Low pT
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonIdLowPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},
        {{"MuonIdLowPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},
        {{"MuonIdLowPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},
        {{"MuonIdLowPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_TightID_DEN_TrackerMuons"}},

        // Muon ID SF - High pT
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonIdHighPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_HighPtID_DEN_TrackerMuons"}},
        {{"MuonIdHighPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_HighPtID_DEN_TrackerMuons"}},
        {{"MuonIdHighPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_HighPtID_DEN_TrackerMuons"}},
        {{"MuonIdHighPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_HighPtID_DEN_TrackerMuons"}},

        // Muon Iso SF - Low pT
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonIsoLowPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_TightRelIso_DEN_TightIDandIPCut"}},
        {{"MuonIsoLowPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_TightRelIso_DEN_TightIDandIPCut"}},
        {{"MuonIsoLowPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_TightRelIso_DEN_TightIDandIPCut"}},
        {{"MuonIsoLowPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_TightRelIso_DEN_TightIDandIPCut"}},

        // Muon Iso SF - High pT
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"MuonIsoHighPt"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_TightRelTkIso_DEN_HighPtIDandIPCut"}},
        {{"MuonIsoHighPt"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_TightRelTkIso_DEN_HighPtIDandIPCut"}},
        {{"MuonIsoHighPt"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_TightRelTkIso_DEN_HighPtIDandIPCut"}},
        {{"MuonIsoHighPt"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_TightRelTkIso_DEN_HighPtIDandIPCut"}},

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
        {{"MuonLowPt"sv, Year::Run2016APV},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016aUL.txt")),
          ""}},
        {{"MuonLowPt"sv, Year::Run2016},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016bUL.txt")),
          ""}},
        {{"MuonLowPt"sv, Year::Run2017},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2017UL.txt")),
          ""}},
        {{"MuonLowPt"sv, Year::Run2018},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2018UL.txt")),
          ""}},

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

        // PixelVeto
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{"PixelVetoSF"sv, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016preVFP_UL/photon.json.gz",
          "UL-Photon-PixVeto-SF"}},
        {{"PixelVetoSF"sv, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016postVFP_UL/photon.json.gz",
          "UL-Photon-PixVeto-SF"}},
        {{"PixelVetoSF"sv, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2017_UL/photon.json.gz",
          "UL-Photon-PixVeto-SF"}},
        {{"PixelVetoSF"sv, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2018_UL/photon.json.gz",
          "UL-Photon-PixVeto-SF"}},
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