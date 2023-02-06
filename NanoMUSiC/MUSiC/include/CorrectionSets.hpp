
#ifndef CORRECTIONSET_H
#define CORRECTIONSET_H

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
    static auto read_xxd_dump(unsigned char arr[], unsigned int _size, const std::string &name = "_") -> TMemFile
    {
        std::unique_ptr<char[]> buffer(new char[_size]());

        for (unsigned int i = 0; i < _size; i++)
        {
            buffer[i] = static_cast<char>(arr[i]);
        }

        return {name.c_str(), buffer.get(), _size};
    }

    //////////////////////////////////////////
    /// Get limits for a given 2D SF histogram
    ///
    auto get_limits(const TH2F &histo) -> std::tuple<double, double, double, double>
    {

        auto eta_upper_limit = histo.GetXaxis()->GetBinUpEdge(histo.GetXaxis()->GetLast());
        auto eta_lower_limit =
            sf_histogram_barrel.GetXaxis()->GetBinLowEdge(sf_histogram_barrel.GetXaxis()->GetFirst());

        auto pt_upper_limit = histo.GetYaxis()->GetBinUpEdge(histo.GetYaxis()->GetLast());
        auto pt_lower_limit = histo.GetYaxis()->GetBinLowEdge(histo.GetYaxis()->GetFirst());

        return std::make_tuple(eta_upper_limit, //
                               eta_lower_limit, //
                               pt_upper_limit,  //
                               pt_lower_limit);
    }

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
    ElectronTriggerSF(const PtRegime &_pt_regime, const Year &_year)
        : pt_regime(_pt_regime),
          year(_year)
    {
        // histo->GetXaxis()->GetBinUpEdge(histo->GetXaxis()->GetLast())
        if (year == Year::Run2016APV)
        {
            if (pt_regime == PtRegime::LowPt)
            {
                auto memory_file = read_xxd_dump(EGTriggerLowPtRun2016APV::egammaEffi_txt_EGM2D_root,
                                                 EGTriggerLowPtRun2016APV::egammaEffi_txt_EGM2D_root_len);
                // get histograms
                sf_histogram_barrel = *(memory_file.Get<TH2F>("EGamma_SF2D"));
                sf_histogram_endcap = *(memory_file.Get<TH2F>("EGamma_SF2D"));

                // get limits
                auto barrel_limits = get_limits(sf_histogram_barrel);
                auto endcap_limits = get_limits(sf_histogram_endcap);
                // eta
                barrel_eta_upper_limit = std::get<0>(barrel_limits);
                barrel_eta_lower_limit = std::get<1>(barrel_limits);
                barrel_pt_upper_limit = std::get<2>(barrel_limits);
                barrel_pt_lower_limit = std::get<3>(barrel_limits);
                // pt
                endcap_eta_upper_limit = std::get<0>(endcap_limits);
                endcap_eta_lower_limit = std::get<1>(endcap_limits);
                endcap_pt_upper_limit = std::get<2>(endcap_limits);
                endcap_pt_lower_limit = std::get<3>(endcap_limits);
            }
            else
            {
                auto memory_file = read_xxd_dump(EGTriggerHighPtRun2016APV::egammaEffi_txt_EGM2D_root,
                                                 EGTriggerHighPtRun2016APV::egammaEffi_txt_EGM2D_root_len);
                sf_histogram_barrel = *(memory_file.Get<TH2F>("SF_TH2F_Barrel"));
                sf_histogram_endcap = *(memory_file.Get<TH2F>("SF_TH2F_EndCap"));

                // get limits
                auto barrel_limits = get_limits(sf_histogram_barrel);
                auto endcap_limits = get_limits(sf_histogram_endcap);
                // eta
                barrel_eta_upper_limit = std::get<0>(barrel_limits);
                barrel_eta_lower_limit = std::get<1>(barrel_limits);
                barrel_pt_upper_limit = std::get<2>(barrel_limits);
                barrel_pt_lower_limit = std::get<3>(barrel_limits);
                // pt
                endcap_eta_upper_limit = std::get<0>(endcap_limits);
                endcap_eta_lower_limit = std::get<1>(endcap_limits);
                endcap_pt_upper_limit = std::get<2>(endcap_limits);
                endcap_pt_lower_limit = std::get<3>(endcap_limits);
            }
        }
        else if (year == Year::Run2016)
        {
            if (pt_regime == PtRegime::LowPt)
            {
                auto memory_file = read_xxd_dump(EGTriggerLowPtRun2016::egammaEffi_txt_EGM2D_root,
                                                 EGTriggerLowPtRun2016::egammaEffi_txt_EGM2D_root_len);
                sf_histogram_barrel = *(memory_file.Get<TH2F>("EGamma_SF2D"));
                sf_histogram_endcap = *(memory_file.Get<TH2F>("EGamma_SF2D"));

                // get limits
                auto barrel_limits = get_limits(sf_histogram_barrel);
                auto endcap_limits = get_limits(sf_histogram_endcap);
                // eta
                barrel_eta_upper_limit = std::get<0>(barrel_limits);
                barrel_eta_lower_limit = std::get<1>(barrel_limits);
                barrel_pt_upper_limit = std::get<2>(barrel_limits);
                barrel_pt_lower_limit = std::get<3>(barrel_limits);
                // pt
                endcap_eta_upper_limit = std::get<0>(endcap_limits);
                endcap_eta_lower_limit = std::get<1>(endcap_limits);
                endcap_pt_upper_limit = std::get<2>(endcap_limits);
                endcap_pt_lower_limit = std::get<3>(endcap_limits);
            }
            else
            {
                auto memory_file = read_xxd_dump(EGTriggerHighPtRun2016::egammaEffi_txt_EGM2D_root,
                                                 EGTriggerHighPtRun2016::egammaEffi_txt_EGM2D_root_len);
                sf_histogram_barrel = *(memory_file.Get<TH2F>("SF_TH2F_Barrel"));
                sf_histogram_endcap = *(memory_file.Get<TH2F>("SF_TH2F_EndCap"));

                // get limits
                auto barrel_limits = get_limits(sf_histogram_barrel);
                auto endcap_limits = get_limits(sf_histogram_endcap);
                // eta
                barrel_eta_upper_limit = std::get<0>(barrel_limits);
                barrel_eta_lower_limit = std::get<1>(barrel_limits);
                barrel_pt_upper_limit = std::get<2>(barrel_limits);
                barrel_pt_lower_limit = std::get<3>(barrel_limits);
                // pt
                endcap_eta_upper_limit = std::get<0>(endcap_limits);
                endcap_eta_lower_limit = std::get<1>(endcap_limits);
                endcap_pt_upper_limit = std::get<2>(endcap_limits);
                endcap_pt_lower_limit = std::get<3>(endcap_limits);
            }
        }
        else if (year == Year::Run2017)
        {
            if (pt_regime == PtRegime::LowPt)
            {
                auto memory_file = read_xxd_dump(EGTriggerLowPtRun2017::egammaEffi_txt_EGM2D_root,
                                                 EGTriggerLowPtRun2017::egammaEffi_txt_EGM2D_root_len);
                sf_histogram_barrel = *(memory_file.Get<TH2F>("EGamma_SF2D"));
                sf_histogram_endcap = *(memory_file.Get<TH2F>("EGamma_SF2D"));

                // get limits
                auto barrel_limits = get_limits(sf_histogram_barrel);
                auto endcap_limits = get_limits(sf_histogram_endcap);
                // eta
                barrel_eta_upper_limit = std::get<0>(barrel_limits);
                barrel_eta_lower_limit = std::get<1>(barrel_limits);
                barrel_pt_upper_limit = std::get<2>(barrel_limits);
                barrel_pt_lower_limit = std::get<3>(barrel_limits);
                // pt
                endcap_eta_upper_limit = std::get<0>(endcap_limits);
                endcap_eta_lower_limit = std::get<1>(endcap_limits);
                endcap_pt_upper_limit = std::get<2>(endcap_limits);
                endcap_pt_lower_limit = std::get<3>(endcap_limits);
            }
            else
            {
                auto memory_file = read_xxd_dump(EGTriggerHighPtRun2017::egammaEffi_txt_EGM2D_root,
                                                 EGTriggerHighPtRun2017::egammaEffi_txt_EGM2D_root_len);
                sf_histogram_barrel = *(memory_file.Get<TH2F>("SF_TH2F_Barrel"));
                sf_histogram_endcap = *(memory_file.Get<TH2F>("SF_TH2F_EndCap"));

                // get limits
                auto barrel_limits = get_limits(sf_histogram_barrel);
                auto endcap_limits = get_limits(sf_histogram_endcap);
                // eta
                barrel_eta_upper_limit = std::get<0>(barrel_limits);
                barrel_eta_lower_limit = std::get<1>(barrel_limits);
                barrel_pt_upper_limit = std::get<2>(barrel_limits);
                barrel_pt_lower_limit = std::get<3>(barrel_limits);
                // pt
                endcap_eta_upper_limit = std::get<0>(endcap_limits);
                endcap_eta_lower_limit = std::get<1>(endcap_limits);
                endcap_pt_upper_limit = std::get<2>(endcap_limits);
                endcap_pt_lower_limit = std::get<3>(endcap_limits);
            }
        }
        else if (year == Year::Run2018)
        {
            if (pt_regime == PtRegime::LowPt)
            {
                auto memory_file = read_xxd_dump(EGTriggerLowPtRun2018::egammaEffi_txt_EGM2D_root,
                                                 EGTriggerLowPtRun2018::egammaEffi_txt_EGM2D_root_len);
                sf_histogram_barrel = *(memory_file.Get<TH2F>("EGamma_SF2D"));
                sf_histogram_endcap = *(memory_file.Get<TH2F>("EGamma_SF2D"));

                // get limits
                auto barrel_limits = get_limits(sf_histogram_barrel);
                auto endcap_limits = get_limits(sf_histogram_endcap);
                // eta
                barrel_eta_upper_limit = std::get<0>(barrel_limits);
                barrel_eta_lower_limit = std::get<1>(barrel_limits);
                barrel_pt_upper_limit = std::get<2>(barrel_limits);
                barrel_pt_lower_limit = std::get<3>(barrel_limits);
                // pt
                endcap_eta_upper_limit = std::get<0>(endcap_limits);
                endcap_eta_lower_limit = std::get<1>(endcap_limits);
                endcap_pt_upper_limit = std::get<2>(endcap_limits);
                endcap_pt_lower_limit = std::get<3>(endcap_limits);
            }
            else
            {
                auto memory_file = read_xxd_dump(EGTriggerHighPtRun2018::egammaEffi_txt_EGM2D_root,
                                                 EGTriggerHighPtRun2018::egammaEffi_txt_EGM2D_root_len);
                sf_histogram_barrel = *(memory_file.Get<TH2F>("SF_TH2F_Barrel"));
                sf_histogram_endcap = *(memory_file.Get<TH2F>("SF_TH2F_EndCap"));

                // get limits
                auto barrel_limits = get_limits(sf_histogram_barrel);
                auto endcap_limits = get_limits(sf_histogram_endcap);
                // eta
                barrel_eta_upper_limit = std::get<0>(barrel_limits);
                barrel_eta_lower_limit = std::get<1>(barrel_limits);
                barrel_pt_upper_limit = std::get<2>(barrel_limits);
                barrel_pt_lower_limit = std::get<3>(barrel_limits);
                // pt
                endcap_eta_upper_limit = std::get<0>(endcap_limits);
                endcap_eta_lower_limit = std::get<1>(endcap_limits);
                endcap_pt_upper_limit = std::get<2>(endcap_limits);
                endcap_pt_lower_limit = std::get<3>(endcap_limits);
            }
        }
        else
        {
            throw std::runtime_error(
                "[ Electron Trigger SF ] A proper year and pt_regime combination was not provided.");
        }

        // print configurations
        // PtRegime pt_regime;
        // Year year;
        // TH2F sf_histogram_barrel;
        // TH2F sf_histogram_endcap;
        // float barrel_eta_upper_limit;
        // float barrel_eta_lower_limit;
        // float endcap_eta_upper_limit;
        // float endcap_eta_lower_limit;
        // float barrel_pt_upper_limit;
        // float barrel_pt_lower_limit;
        // float endcap_pt_upper_limit;
        // float endcap_pt_lower_limit;
        fmt::print("\n=========================================================\n");
        fmt::print("------------      Electron Trigger SF    ----------------\n");
        fmt::print("Energy Regime: {}\n", (_pt_regime == PtRegime::LowPt) ? "Low pT"sv : "High pT"sv);
        fmt::print("Barrel eta: [{} - {}]\n", barrel_eta_lower_limit, barrel_eta_upper_limit);
        fmt::print("Barrel pT: [{} - {}]\n", barrel_pt_lower_limit, barrel_pt_upper_limit);
        fmt::print("Endcap eta: [{} - {}]\n", endcap_eta_lower_limit, endcap_eta_upper_limit);
        fmt::print("Endcap pT: [{} - {}]\n", endcap_pt_lower_limit, endcap_pt_upper_limit);
        fmt::print("=========================================================\n\n");
    }

    auto operator()(float eta_sc, float pt, std::string_view variation = "nominal") const -> double
    {
        // check for physical values
        if (pt < 0.)
        {
            throw std::runtime_error("[ Electron Trigger SF ] The value of pT should be positive.");
        }

        // check if is barrel or endcap
        if (std::fabs(eta_sc) < 1.442)
        {
            // check limits
            if (pt >= barrel_pt_upper_limit)
            {
                pt = barrel_pt_upper_limit - epsilon;
            }
            if (pt <= barrel_pt_lower_limit)
            {
                pt = barrel_pt_lower_limit + epsilon;
            }
            auto nominal = sf_histogram_barrel.GetBinContent(sf_histogram_barrel.FindFixBin(eta_sc, pt));
            auto uncert = 0.;

            if (variation == "up")
            {
                uncert = sf_histogram_barrel.GetBinErrorUp(sf_histogram_barrel.FindFixBin(eta_sc, pt));
            }
            if (variation == "down")
            {
                uncert = -1.0 * sf_histogram_barrel.GetBinErrorLow(sf_histogram_barrel.FindFixBin(eta_sc, pt));
            }

            return nominal + uncert;
        }
        if (std::fabs(eta_sc) >= 1.566 and std::fabs(eta_sc) < 2.5)
        {
            // check limits
            if (pt >= endcap_pt_upper_limit)
            {
                pt = endcap_pt_upper_limit - epsilon;
            }
            if (pt <= endcap_pt_lower_limit)
            {
                pt = endcap_pt_lower_limit + epsilon;
            }

            auto nominal = sf_histogram_endcap.GetBinContent(sf_histogram_endcap.FindFixBin(eta_sc, pt));
            auto uncert = 0.;

            if (variation == "up")
            {
                uncert = sf_histogram_barrel.GetBinErrorUp(sf_histogram_barrel.FindFixBin(eta_sc, pt));
            }
            if (variation == "down")
            {
                uncert = -1.0 * sf_histogram_barrel.GetBinErrorLow(sf_histogram_barrel.FindFixBin(eta_sc, pt));
            }

            return nominal + uncert;
        }
        throw std::runtime_error(fmt::format(
            "[ Electron Trigger SF ] The provided Super Cluster eta value ({}), is about of range.", eta_sc));
    }
};

namespace CorrectionHelpers
{

///////////////////////////////////////////////////////////////
/// For some reason, the Official Muon SFs requires a field of the requested year, with proper formating.
inline auto get_year_for_muon_sf(Year year) -> std::string
{
    switch (year)
    {
    case Year::Run2016APV:
        return "2016preVFP_UL"s;
    case Year::Run2016:
        return "2016postVFP_UL"s;
    case Year::Run2017:
        return "2017_UL"s;
    case Year::Run2018:
        return "2018_UL"s;
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

} // namespace CorrectionHelpers

using CorrectionlibRef_t = correction::Correction::Ref;
using RochesterCorrection_t = RoccoR;
using ElectronTriggerSF_t = ElectronTriggerSF;

enum class CorrectionTypes
{
    TriggerSFMuonLowPt,
    TriggerSFMuonHighPt,
    TriggerSFElectronLowPt,
    TriggerSFElectronHighPt,
    Photon,
    PU,
    MuonLowPt,
};

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

    Corrector(const std::string_view &_correction_type, const Year _year, bool _is_data)
        : correction_type(_correction_type),
          year(_year),
          is_data(_is_data)
    {
        // Rochester Correction
        if (_correction_type == "MuonLowPt")
        {
            auto [input_file, _dummy_key] = correction_keys.at({correction_type, year});
            correction_ref = RoccoR(input_file);
        }

        // Single Electron Trigger
        else if (_correction_type == "SingleElectronLowPt")
        {
            correction_ref = ElectronTriggerSF(ElectronTriggerSF::PtRegime::LowPt, year);
        }
        else if (_correction_type == "SingleElectronHighPt")
        {
            correction_ref = ElectronTriggerSF(ElectronTriggerSF::PtRegime::HighPt, year);
        }

        // default case is any correction that uses the correctionlib
        else
        {
            auto [json_file, key] = correction_keys.at({correction_type, year});
            correction_ref = correction::CorrectionSet::from_file(json_file)->at(key);
        }
    }

    auto operator()() const -> double
    {
        if (not is_data)
        {
            return 1.;
        }
        return 1.;
    }

    // Electron Trigger SF
    auto operator()(const float &eta_sc, const float &pt, const std::string_view &variation = "nominal") const -> double
    {
        if (not is_data)
        {
            return std::get<ElectronTriggerSF_t>(correction_ref)(eta_sc, pt, variation);
        }
        return 1.;
    }

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
    auto operator()(const Year &year, const RVec<T> &pt, const RVec<T> &eta, const std::string &variation,
                    BinOp Op = std::multiplies<T>(), float initial_value = 1.f) const -> float
    {
        static_assert(std::is_floating_point<T>::value, "The type must be floating point.");

        if (not is_data)
        {

            RVec<float> sfs = RVec<float>(pt.size(), initial_value);

            sfs = VecOps::Map(pt, eta, [&](const T &_pt, const T &_eta) {
                return (*this)({CorrectionHelpers::get_year_for_muon_sf(year), //
                                static_cast<double>(_eta),                     //
                                static_cast<double>(_pt),                      //
                                variation});
            });

            return std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);

            // auto weight = std::reduce(sfs.cbegin(), sfs.cend(), initial_value, Op);

            // fmt::print("Weight: {}\n", weight);

            // return weight;
        }
        return 1.;
    }

    // rochester corrections - Data
    auto operator()(const int Q, const double pt, const double eta, const double phi, const int s = 0,
                    const int m = 0) const -> double
    {
        if (is_data)
        {
            return std::get<RochesterCorrection_t>(correction_ref).kScaleDT(Q, pt, eta, phi, s, m);
        }
        throw std::runtime_error("The signature of the used method is only valid for Data samples.");
    }

    // rochester corrections - MC (matched GenMuon - recommended)
    auto operator()(const int Q, const double pt, const double eta, const double phi, const double genPt,
                    const int s = 0, const int m = 0) const -> double
    {
        if (not is_data)
        {
            return std::get<RochesterCorrection_t>(correction_ref).kSpreadMC(Q, pt, eta, phi, genPt, s, m);
        }
        throw std::runtime_error("The signature of the used method is only valid for MC samples.");
    }

    // rochester corrections - MC (unmatched GenMuon - not recommended)
    auto operator()(const int Q, const double pt, const double eta, const double phi, const int n, const double u,
                    const int s = 0, const int m = 0) const -> double
    {
        if (not is_data)
        {
            return std::get<RochesterCorrection_t>(correction_ref).kSmearMC(Q, pt, eta, phi, n, u, s, m);
        }
        throw std::runtime_error("The signature of the used method is only valid for MC samples.");
    }
};

#endif /*CORRECTIONSET_H*/
