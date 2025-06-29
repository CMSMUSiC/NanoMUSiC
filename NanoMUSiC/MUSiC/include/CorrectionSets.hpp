
#ifndef CORRECTIONSET_H
#define CORRECTIONSET_H

#include <functional>
#include <stdexcept>
#include <string>

// correctionlib
// More info: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagCalibration
// More info: https://github.com/cms-nanoAOD/correctionlib
// More info: https://cms-nanoaod.github.io/correctionlib/index.html
// Instructions:
// https://indico.cern.ch/event/1096988/contributions/4615134/attachments/2346047/4000529/Nov21_btaggingSFjsons.pdf
#include <correction.h>

#include "ROOT/RVec.hxx"
#include "TH2.h"
#include "TMemFile.h"
using namespace ROOT;
using namespace ROOT::VecOps;

#include "fmt/core.h"

// Rochester Corrections
// Ref: https://twiki.cern.ch/twiki/bin/viewauth/CMS/RochcorMuon
// Ref: https://gitlab.cern.ch/akhukhun/roccor/-/tree/Run2.v5
// Implementation example: https://github.com/UFLX2MuMu/Ntupliser/blob/master_2017_94X/DiMuons/src/PtCorrRoch.cc
#include "roccor/RoccoR.h"

#include "Configs.hpp"
#include "MUSiCTools.hpp"

// Electron Trigger SF
// Low Pt
#include "ElectronTriggerSF/ElectronLowPt/Run2016/egammaEffi_txt_EGM2D.hpp"
#include "ElectronTriggerSF/ElectronLowPt/Run2016APV/egammaEffi_txt_EGM2D.hpp"
#include "ElectronTriggerSF/ElectronLowPt/Run2017/egammaEffi_txt_EGM2D.hpp"
#include "ElectronTriggerSF/ElectronLowPt/Run2018/egammaEffi_txt_EGM2D.hpp"

// High Pt
#include "ElectronTriggerSF/ElectronHighPt/Run2016/egammaEffi_txt_EGM2D.hpp"
#include "ElectronTriggerSF/ElectronHighPt/Run2016APV/egammaEffi_txt_EGM2D.hpp"
#include "ElectronTriggerSF/ElectronHighPt/Run2017/egammaEffi_txt_EGM2D.hpp"
#include "ElectronTriggerSF/ElectronHighPt/Run2018/egammaEffi_txt_EGM2D.hpp"

using namespace std::string_view_literals;

class ElectronTriggerSF
{
  public:
    enum PtRegime
    {
        LowPt,
        HighPt,
        kTotalRegimes
    };

  private:
    ///////////////////////////////////////////////////////
    /// Reads a xxd dump and returns a TMemFile.
    /// xxd -i root_file.root > foo.h
    static auto read_xxd_dump(const unsigned char arr[], const unsigned int _size, const std::string &name = "_")
        -> TMemFile;

    //////////////////////////////////////////
    /// Get limits for a given 2D SF histogram
    ///
    auto get_limits(const TH2F &histo) -> std::tuple<double, double, double, double>;

    PtRegime pt_regime;
    Year year;
    TH2F sf_histogram_barrel;
    TH2F sf_histogram_endcap;
    float barrel_eta_upper_limit;
    float barrel_eta_lower_limit;
    float endcap_eta_upper_limit;
    float endcap_eta_lower_limit;
    float barrel_pt_upper_limit;
    float barrel_pt_lower_limit;
    float endcap_pt_upper_limit;
    float endcap_pt_lower_limit;

    constexpr static float epsilon = 0.0001;

  public:
    ElectronTriggerSF(const PtRegime &_pt_regime, const Year &_year);

    auto operator()(float eta_sc, float pt, std::string_view variation = "nominal") const -> double;
};

namespace CorrectionHelpers
{

///////////////////////////////////////////////////////////////
/// For some reason, the Official Muon SFs requires a field of the requested year, with proper formating.
auto get_year_for_muon_sf(Year year) -> std::string;

} // namespace CorrectionHelpers

using CorrectionlibRef_t = correction::Correction::Ref;
using RochesterCorrection_t = RoccoR;
using ElectronTriggerSF_t = ElectronTriggerSF;

class Corrector
{
  public:
    const std::string_view correction_type;
    const Year year;
    const bool is_data;
    std::variant<CorrectionlibRef_t, RochesterCorrection_t, ElectronTriggerSF_t> correction_ref;

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

        // MuonLowPt
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
    };

    Corrector(const std::string_view &_correction_type, const Year _year, bool _is_data);

    auto operator()() const -> double;

    // Electron Trigger SF
    auto operator()(const float &eta_sc, const float &pt, const std::string_view &variation = "nominal") const
        -> double;

    // correctionlib
    template <class T = const std::vector<std::variant<int, double, std::string>>>
    auto operator()(T &&values) const -> double
    {
        if (not is_data)
        {
            return std::get<CorrectionlibRef_t>(correction_ref)->evaluate(std::forward<T>(values));
        }
        return 1.;
    }

    ////////////////////////////////////////////////////////
    /// correctionlib
    /// For a given collection of objects, get its total scale factor, according to a binary operator and a initial
    /// value (should be the null value of the given operator)
    template <typename T, typename BinOp = std::multiplies<T>>
    auto operator()(const Year &year,
                    const RVec<T> &pt,
                    const RVec<T> &eta,
                    const std::string &variation,
                    BinOp Op = std::multiplies<T>(),
                    float initial_value = 1.f) const -> float
    {
        static_assert(std::is_floating_point<T>::value, "The type must be floating point.");

        if (not is_data)
        {
            if (correction_type == "MuonReco"sv        //
                or correction_type == "MuonIdLowPt"sv  //
                or correction_type == "MuonIdHighPt"sv //
                or correction_type == "MuonIsoLowPt"sv //
                or correction_type == "MuonIsoHighPt"sv)
            {

                RVec<float> sfs = RVec<float>(pt.size(), initial_value);

                sfs = VecOps::Map(pt,
                                  eta,
                                  [&](const T &_pt, const T &_eta)
                                  {
                                      return (*this)({CorrectionHelpers::get_year_for_muon_sf(year), //
                                                      static_cast<double>(_eta),                     //
                                                      static_cast<double>(_pt),                      //
                                                      variation});
                                  });

                return std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);
            }

            // default case
            throw std::runtime_error(
                fmt::format("No matching was found for this correction type: {}.", correction_type));
        }
        return 1.;
    }

    // rochester corrections - Data
    auto operator()(const int Q, const double pt, const double eta, const double phi, const int s = 0, const int m = 0)
        const -> double;

    // rochester corrections - MC (matched GenMuon - recommended)
    auto operator()(const int Q,
                    const double pt,
                    const double eta,
                    const double phi,
                    const double genPt,
                    const int s = 0,
                    const int m = 0) const -> double;

    // rochester corrections - MC (unmatched GenMuon - not recommended)
    auto operator()(const int Q,
                    const double pt,
                    const double eta,
                    const double phi,
                    const int n,
                    const double u,
                    const int s = 0,
                    const int m = 0) const -> double;
};

class ElectronSFCorrector
{
  public:
    const Year year;
    const bool is_data;
    CorrectionlibRef_t correction_ref;
    std::string year_str;

    ElectronSFCorrector(const Year _year, bool _is_data);

    //////////////////////////////////
    /// Get HEEP Id SF
    /// Ref:
    /// https://twiki.cern.ch/twiki/bin/view/CMS/HEEPElectronIdentificationRun2#Scale_Factor
    auto get_high_pt_sf(const Year &year, const std::string &variation, const float &pt, const float &eta) const
        -> float;

    // correctionlib default call
    template <class T = const std::vector<std::variant<int, double, std::string>>>
    auto get_low_pt_sf(T &&values) const -> double
    {
        if (not is_data)
        {
            return correction_ref->evaluate(std::forward<T>(values));
        }
        return 1.;
    }

    ////////////////////////////////////////////////////////
    /// correctionlib
    /// For a given collection of objects, get its total scale factor, according to a binary operator and a initial
    /// value (should be the null value of the given operator)
    template <typename T, typename BinOp = std::multiplies<T>>
    auto operator()(const std::string &variation,
                    const std::string &working_point,
                    const RVec<T> &pt,
                    const RVec<T> &eta,
                    BinOp Op = std::multiplies<T>(),
                    float initial_value = 1.f) const -> float
    {
        static_assert(std::is_floating_point<T>::value, "The type must be floating point.");

        if (not is_data)
        {
            // check if it is HEEP Id
            if (working_point == "HEEPId")
            {
                RVec<float> sfs = RVec<float>(pt.size(), initial_value);

                sfs = VecOps::Map(pt,
                                  eta,
                                  [&](const T &_pt, const T &_eta)
                                  {
                                      return this->get_high_pt_sf(year,                     //
                                                                  variation,                //
                                                                  static_cast<double>(_pt), //
                                                                  static_cast<double>(_eta));
                                  });

                return std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);

                // auto weight = std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);
                // if (pt.size() > 0)
                // {
                //     fmt::print("Weight: {}\n", weight);
                // }
                // return weight;
            }

            // if not HEEP ID, then it is Low Pt
            RVec<float> sfs = RVec<float>(pt.size(), initial_value);

            sfs = VecOps::Map(pt,
                              eta,
                              [&](const T &_pt, const T &_eta)
                              {
                                  return this->get_low_pt_sf({year_str,                  //
                                                              variation,                 //
                                                              working_point,             //
                                                              static_cast<double>(_eta), //
                                                              static_cast<double>(_pt)});
                              });

            return std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);

            // auto weight = std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);
            // if (pt.size() > 0)
            // {
            //     fmt::print("Weight: {}\n", weight);
            // }
            // return weight;
        }
        return 1.;
    }
};

class PhotonSFCorrector
{
  public:
    std::string correction_type;
    const Year year;
    const bool is_data;
    CorrectionlibRef_t correction_ref;
    std::string year_str;

    PhotonSFCorrector(const std::string &_correction_type, const Year _year, bool _is_data);

    // correctionlib default call
    template <class T = const std::vector<std::variant<int, double, std::string>>>
    auto get_sf(T &&values) const -> double
    {
        if (not is_data)
        {
            return correction_ref->evaluate(std::forward<T>(values));
        }
        return 1.;
    }

    ////////////////////////////////////////////////////////
    /// correctionlib
    /// For a given collection of objects, get its total scale factor, according to a binary operator and a initial
    /// value (should be the null value of the given operator)
    template <typename T = float, typename BinOp = std::multiplies<T>>
    auto operator()(const std::string &variation,
                    const RVec<T> &pt,
                    const RVec<T> &eta = {},
                    BinOp Op = std::multiplies<T>(),
                    float initial_value = 1.f) const -> float
    {
        static_assert(std::is_floating_point<T>::value, "The type must be floating point.");

        if (not is_data)
        {
            // check if correction type is PhotonID
            if (correction_type == "PhotonID"s)
            {
                RVec<float> sfs = RVec<float>(pt.size(), initial_value);

                sfs = VecOps::Map(pt,
                                  eta,
                                  [&](const T &_pt, const T &_eta)
                                  {
                                      return this->get_sf({year_str,                  //
                                                           variation,                 //
                                                           "Tight",                   //
                                                           static_cast<double>(_eta), //
                                                           static_cast<double>(_pt)});
                                  });

                return std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);

                // auto weight = std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);
                // if (pt.size() > 0)
                // {
                //     fmt::print("Weight (PhotonID): {}\n", weight);
                // }
                // return weight;
            }

            // if not PhotonID, then it is PixelSeed
            RVec<float> sfs = RVec<float>(pt.size(), initial_value);

            sfs = VecOps::Map(pt,
                              [&](const T &_pt)
                              {
                                  return this->get_sf({year_str,  //
                                                       variation, //
                                                       "Tight",   //
                                                       "EBInc"});
                              });

            return std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);

            // auto weight = std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);
            // if (pt.size() > 0)
            // {
            //     fmt::print("Weight (PixelSeed): {}\n", weight);
            // }
            // return weight;
        }
        return 1.;
    }
};

class BTagSFCorrector
{
  public:
    const Year year;
    const bool is_data;
    CorrectionlibRef_t correction_ref_bjet;
    CorrectionlibRef_t correction_ref_light_jet;

    BTagSFCorrector(const Year _year, bool _is_data);

    // correctionlib default call
    template <class T = const std::vector<std::variant<int, double, std::string>>>
    auto get_sf_bjet(T &&values) const -> double
    {
        if (not is_data)
        {
            return correction_ref_bjet->evaluate(std::forward<T>(values));
        }
        return 1.;
    }

    // correctionlib default call
    template <class T = const std::vector<std::variant<int, double, std::string>>>
    auto get_sf_light_jet(T &&values) const -> double
    {
        if (not is_data)
        {
            return correction_ref_light_jet->evaluate(std::forward<T>(values));
        }
        return 1.;
    }

    //////////////////////////////////////
    /// get btag efficiencies
    /// For now we are taking the mean value of what was used for the 2016 papper
    auto get_eff(const float &_pt, const float &_eta, const int &_flavour) const -> double;

    ////////////////////////////////////////////////////////
    /// Using Method 1A - Per event weight
    /// References:
    /// - https://twiki.cern.ch/twiki/bin/view/CMS/BTagSFMethods#1a_Event_reweighting_using_scale
    /// - https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideCMSDataAnalysisSchoolLPC2023TaggingExercise
    /// - https://github.com/IreneZoi/CMSDAS2023-BTV/tree/master/BTaggingExercise
    /// - https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation#UltraLegacy_scale_factor_uncerta
    ///
    /// systematic (string): central, down, down_correlated, down_uncorrelated, up, up_correlated,
    /// working_point (string): L, M, T
    /// flavor (int): 5=b, 4=c, 0=udsg
    /// abseta (real)
    /// pt (real)

    auto operator()(const std::string &variation_bjet,          //
                    const std::string &variation_light_jet,     //
                    const RVec<float> &tagged_jets_pt,          //
                    const RVec<float> &tagged_jets_abseta,      //
                    const RVec<int> &tagged_jets_hadronFlavour, //
                    const RVec<float> &untagged_jets_pt,        //
                    const RVec<float> &untagged_jets_abseta,    //
                    const RVec<int> &untagged_jets_hadronFlavour) const -> float;
};

#endif /*CORRECTIONSET_H*/
