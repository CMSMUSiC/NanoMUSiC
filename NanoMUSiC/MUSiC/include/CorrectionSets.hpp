
// correctionlib
// More info: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagCalibration
// More info: https://github.com/cms-nanoAOD/correctionlib
// More info: https://cms-nanoaod.github.io/correctionlib/index.html
// Instructions:
// https://indico.cern.ch/event/1096988/contributions/4615134/attachments/2346047/4000529/Nov21_btaggingSFjsons.pdf
#include "correction.h"

// Rochester Corrections
// Ref: https://twiki.cern.ch/twiki/bin/viewauth/CMS/RochcorMuon
// Ref: https://gitlab.cern.ch/akhukhun/roccor/-/tree/Run2.v5
// Implementation example: https://github.com/UFLX2MuMu/Ntupliser/blob/master_2017_94X/DiMuons/src/PtCorrRoch.cc
#include "roccor/RoccoR.h"

#include "Configs.hpp"

using CorrectionlibRef_t = correction::Correction::Ref;
using RochesterCorrection_t = RoccoR;

enum class CorrectionTypes
{
    PU,
    MuonLowPt,
};

class Corrector
{
  public:
    const CorrectionTypes correction_type;
    const Year year;
    const bool isData;
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

        // MuonLowPt
        // {{TYPE, YEAR}, {INPUT, DUMMY (leave empty)}},
        {{CorrectionTypes::MuonLowPt, Year::Run2016APV},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016UL.txt")), ""}},
        {{CorrectionTypes::MuonLowPt, Year::Run2016},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016UL.txt")), ""}},
        {{CorrectionTypes::MuonLowPt, Year::Run2017},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2017UL.txt")), ""}},
        {{CorrectionTypes::MuonLowPt, Year::Run2018},
         {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2018UL.txt")), ""}},
    };

    Corrector(const CorrectionTypes _correction_type, const Year _year, bool _isData)
        : correction_type(_correction_type), year(_year), isData(_isData)
    {

        switch (_correction_type)
        {
        case CorrectionTypes::MuonLowPt: {
            auto [input_file, _dummy_key] = correction_keys.at({correction_type, year});
            correction_ref = RoccoR(input_file);
            break;
        }
        default:
            if (!isData)
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
        if (!isData)
        {
            return std::get<CorrectionlibRef_t>(correction_ref)->evaluate(std::forward<T>(values));
        }
        return 1.;
    }

    // rochester corrections - Data
    double operator()(int Q, double pt, double eta, double phi, int s = 0, int m = 0)
    {
        if (isData)
        {
            return std::get<RochesterCorrection_t>(correction_ref).kScaleDT(Q, pt, eta, phi, s, m);
        }
        throw std::runtime_error("The signature of the used method is only valid for Data samples.");
    }

    // rochester corrections - MC (matched GenMuon - recommended)
    double operator()(int Q, double pt, double eta, double phi, double genPt, int s = 0, int m = 0)
    {
        if (isData)
        {
            return std::get<RochesterCorrection_t>(correction_ref).kSpreadMC(Q, pt, eta, phi, genPt, s, m);
        }
        throw std::runtime_error("The signature of the used method is only valid for Data samples.");
    }

    // rochester corrections - MC (unmatched GenMuon - not recommended)
    double operator()(int Q, double pt, double eta, double phi, int n, double u, int s = 0, int m = 0)
    {
        if (isData)
        {
            return std::get<RochesterCorrection_t>(correction_ref).kSmearMC(Q, pt, eta, phi, n, u, s, m);
        }
        throw std::runtime_error("The signature of the used method is only valid for Data samples.");
    }
};
