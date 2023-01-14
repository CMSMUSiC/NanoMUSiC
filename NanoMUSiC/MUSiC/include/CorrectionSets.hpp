
#ifndef CORRECTIONSET_H
#define CORRECTIONSET_H

// correctionlib
// More info: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagCalibration
// More info: https://github.com/cms-nanoAOD/correctionlib
// More info: https://cms-nanoaod.github.io/correctionlib/index.html
// Instructions:
// https://indico.cern.ch/event/1096988/contributions/4615134/attachments/2346047/4000529/Nov21_btaggingSFjsons.pdf
#include <correction.h>

// Rochester Corrections
// Ref: https://twiki.cern.ch/twiki/bin/viewauth/CMS/RochcorMuon
// Ref: https://gitlab.cern.ch/akhukhun/roccor/-/tree/Run2.v5
// Implementation example: https://github.com/UFLX2MuMu/Ntupliser/blob/master_2017_94X/DiMuons/src/PtCorrRoch.cc
#include "roccor/RoccoR.h"

#include "Configs.hpp"
#include "MUSiCTools.hpp"

using CorrectionlibRef_t = correction::Correction::Ref;
using RochesterCorrection_t = RoccoR;

enum class CorrectionTypes
{
    TriggerSFMuonLowPt,
    TriggerSFMuonHighPt,
    PU,
    MuonLowPt,
};

class Corrector
{
  public:
    const CorrectionTypes correction_type;
    const Year year;
    const bool is_data;
    std::variant<CorrectionlibRef_t, RochesterCorrection_t> correction_ref;

    const std::map<std::pair<CorrectionTypes, Year>, std::pair<std::string, std::string>> correction_keys = {
        // PU
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{CorrectionTypes::PU, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2016preVFP_UL/puWeights.json.gz",
          "Collisions16_UltraLegacy_goldenJSON"}},
        {{CorrectionTypes::PU, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2016postVFP_UL/puWeights.json.gz",
          "Collisions16_UltraLegacy_goldenJSON"}},
        {{CorrectionTypes::PU, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2017_UL/puWeights.json.gz",
          "Collisions17_UltraLegacy_goldenJSON"}},
        {{CorrectionTypes::PU, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2018_UL/puWeights.json.gz",
          "Collisions18_UltraLegacy_goldenJSON"}},

        // Muon Trigger SF  - Low Pt
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{CorrectionTypes::TriggerSFMuonLowPt, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight"}},
        {{CorrectionTypes::TriggerSFMuonLowPt, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight"}},
        {{CorrectionTypes::TriggerSFMuonLowPt, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight"}},
        {{CorrectionTypes::TriggerSFMuonLowPt, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight"}},

        // Muon Trigger SF  - High Pt
        // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
        {{CorrectionTypes::TriggerSFMuonHighPt, Year::Run2016APV},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
          "NUM_Mu50_or_TkMu50_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},
        {{CorrectionTypes::TriggerSFMuonHighPt, Year::Run2016},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
          "NUM_Mu50_or_TkMu50_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},
        {{CorrectionTypes::TriggerSFMuonHighPt, Year::Run2017},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
          "NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},
        {{CorrectionTypes::TriggerSFMuonHighPt, Year::Run2018},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
          "NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},

        // MuonLowPt
        // {{TYPE, YEAR}, {INPUT, DUMMY (leave empty)}},
        {{CorrectionTypes::MuonLowPt, Year::Run2016APV},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016UL.txt")),
          ""}},
        {{CorrectionTypes::MuonLowPt, Year::Run2016},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016UL.txt")),
          ""}},
        {{CorrectionTypes::MuonLowPt, Year::Run2017},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2017UL.txt")),
          ""}},
        {{CorrectionTypes::MuonLowPt, Year::Run2018},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2018UL.txt")),
          ""}},
    };

    Corrector(const CorrectionTypes _correction_type, const Year _year, bool _is_data)
        : correction_type(_correction_type), year(_year), is_data(_is_data)
    {

        switch (_correction_type)
        {
        case CorrectionTypes::MuonLowPt: {
            auto [input_file, _dummy_key] = correction_keys.at({correction_type, year});
            correction_ref = RoccoR(input_file);
            break;
        }
        default:
            if (!is_data)
            {
                auto [json_file, key] = correction_keys.at({correction_type, year});
                correction_ref = correction::CorrectionSet::from_file(json_file)->at(key);
            }
        }
    }

    // correctionlib
    template <class T = const std::vector<std::variant<int, double, std::string>>>
    double operator()(T &&values)
    {
        if (not is_data)
        {
            return std::get<CorrectionlibRef_t>(correction_ref)->evaluate(std::forward<T>(values));
        }
        return 1.;
    }

    // rochester corrections - Data
    double operator()(const int Q, const double pt, const double eta, const double phi, const int s = 0,
                      const int m = 0)
    {
        if (is_data)
        {
            return std::get<RochesterCorrection_t>(correction_ref).kScaleDT(Q, pt, eta, phi, s, m);
        }
        throw std::runtime_error("The signature of the used method is only valid for Data samples.");
    }

    // rochester corrections - MC (matched GenMuon - recommended)
    double operator()(const int Q, const double pt, const double eta, const double phi, const double genPt,
                      const int s = 0, const int m = 0)
    {
        if (not is_data)
        {
            return std::get<RochesterCorrection_t>(correction_ref).kSpreadMC(Q, pt, eta, phi, genPt, s, m);
        }
        throw std::runtime_error("The signature of the used method is only valid for MC samples.");
    }

    // rochester corrections - MC (unmatched GenMuon - not recommended)
    double operator()(const int Q, const double pt, const double eta, const double phi, const int n, const double u,
                      const int s = 0, const int m = 0)
    {
        if (not is_data)
        {
            return std::get<RochesterCorrection_t>(correction_ref).kSmearMC(Q, pt, eta, phi, n, u, s, m);
        }
        throw std::runtime_error("The signature of the used method is only valid for MC samples.");
    }
};

#endif /*CORRECTIONSET_H*/