#ifndef MUSIC_CONFIG
#define MUSIC_CONFIG

#include <filesystem>

#include "Enumerate.hpp"
#include "MUSiCTools.hpp"
#include "TOMLConfig.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <limits>

using namespace std::literals;

namespace PDG
{
namespace Muon
{
constexpr int Id = 13;
constexpr float Mass = 105.6583755 / 1000.0;
} // namespace Muon
namespace Electron
{
constexpr int Id = 11;
constexpr float Mass = 0.51099895 / 1000.0;
} // namespace Electron
namespace Photon
{
constexpr int Id = 22;
constexpr float Mass = 0;
} // namespace Photon
namespace Tau
{
constexpr int Id = 15;
constexpr float Mass = 1776.86;
} // namespace Tau
} // namespace PDG

enum Year
{
    Run2016APV,
    Run2016,
    Run2017,
    Run2018,
    kTotalYears, // <-- should always be the last one!!
};

inline auto get_runyear(const std::string &year_str) -> Year
{
    // check year
    if (year_str != "2016APV" && year_str != "2016" && year_str != "2017" && year_str != "2018")
    {
        throw std::out_of_range{"ERROR: year should be 2016APV, 2016, 2017 or 2018"};
    }

    // return year as enum
    if (year_str == "2016APV")
    {
        return Year::Run2016APV;
    }
    if (year_str == "2016")
    {
        return Year::Run2016;
    }
    if (year_str == "2017")
    {
        return Year::Run2017;
    }
    return Year::Run2018;
}

namespace ObjConfig
{
// Muons
//  REF: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonUL2017
struct MuonConfig
{
    float MinLowPt = 25.;
    float MaxLowPt = 200.;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    // REF:
    // https://github.dev/cms-sw/cmssw/blob/34b164986d540977336f87c70b169efb2d5e478e/DataFormats/MuonReco/src/MuonSelectors.cc#L1043-L1044
    float TkRelIso_WP = 0.10; // smaller than this (DeltaR = 0.3)
    float PFRelIso_WP = 0.15; // smaller than this (DeltaR = 0.4)
};

constexpr auto Muon2016APV = MuonConfig{};
constexpr auto Muon2016 = MuonConfig{};
constexpr auto Muon2017 = MuonConfig{};
constexpr auto Muon2018 = MuonConfig{};
constexpr std::array<MuonConfig, Year::kTotalYears> Muons = {Muon2016APV, Muon2016, Muon2017, Muon2018};

// Electrons
struct ElectronConfig
{
    float MinLowPt = 25;
    float MaxLowPt = 100;
    float MaxAbsEta = 2.5;
    float MaxDeltaRTriggerMatch = 0.3;
    int cutBasedId = 4; // greater than this
};

constexpr auto Electron2016APV = ElectronConfig{};
constexpr auto Electron2016 = ElectronConfig{};
constexpr auto Electron2017 = ElectronConfig{};
constexpr auto Electron2018 = ElectronConfig{};
constexpr std::array<ElectronConfig, Year::kTotalYears> Electrons = {Electron2016APV, Electron2016, Electron2017,
                                                                     Electron2018};

// Photons
struct PhotonConfig
{
    float MinPt = 25;
};

constexpr auto Photon2016APV = PhotonConfig{};
constexpr auto Photon2016 = PhotonConfig{};
constexpr auto Photon2017 = PhotonConfig{};
constexpr auto Photon2018 = PhotonConfig{};
constexpr std::array<PhotonConfig, Year::kTotalYears> Photons = {Photon2016APV, Photon2016, Photon2017, Photon2018};

// Taus
struct TauConfig
{
    // FIXME: change this once we have Taus in the workflow.
    float MinPt = std::numeric_limits<float>::max();
};

constexpr auto Tau2016APV = TauConfig{};
constexpr auto Tau2016 = TauConfig{};
constexpr auto Tau2017 = TauConfig{};
constexpr auto Tau2018 = TauConfig{};
constexpr std::array<TauConfig, Year::kTotalYears> Taus = {Tau2016APV, Tau2016, Tau2017, Tau2018};

// Bjets
struct BjetsConfig
{
    float MinPt = 50;
    float MaxAbsEta = 2.4;
    int MinJetID = 2;              // equal or greater than this
    float MinBTagWPTight = 0.6502; // equal or greater than this
};

constexpr auto Bjets2016APV = BjetsConfig{.MinBTagWPTight = 0.6502};
constexpr auto Bjets2016 = BjetsConfig{.MinBTagWPTight = 0.6377};
constexpr auto Bjets2017 = BjetsConfig{.MinBTagWPTight = 0.7476};
constexpr auto Bjets2018 = BjetsConfig{.MinBTagWPTight = 0.71};
constexpr std::array<BjetsConfig, Year::kTotalYears> BJets = {Bjets2016APV, Bjets2016, Bjets2017, Bjets2018};

// Jets
// REF: https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation106XUL17#AK4_b_tagging
struct JetConfig
{
    float MinPt = 50;
    float MaxAbsEta = 2.4;
    int MinJetID = 2;              // equal or greater than this
    float MaxBTagWPTight = 0.6502; // smaller than this
};

constexpr auto Jet2016APV = JetConfig{.MaxBTagWPTight = 0.6502};
constexpr auto Jet2016 = JetConfig{.MaxBTagWPTight = 0.6377};
constexpr auto Jet2017 = JetConfig{.MaxBTagWPTight = 0.7476};
constexpr auto Jet2018 = JetConfig{.MaxBTagWPTight = 0.71};
constexpr std::array<JetConfig, Year::kTotalYears> Jets = {Jet2016APV, Jet2016, Jet2017, Jet2018};

// MET
// REF: https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation106XUL17#AK4_b_tagging
struct METConfig
{
    float MinPt = 100.;
};

constexpr auto MET2016APV = METConfig{};
constexpr auto MET2016 = METConfig{};
constexpr auto MET2017 = METConfig{};
constexpr auto MET2018 = METConfig{};
constexpr std::array<METConfig, Year::kTotalYears> MET = {MET2016APV, MET2016, MET2017, MET2018};
} // namespace ObjConfig

namespace RunConfig
{
// Here we should be careful. It is better to put here, only configurations that are invariant per sample.
struct RunConfig
{
    std::string_view golden_json;
};

constexpr auto Run2016APV = RunConfig{.golden_json = "$MUSIC_BASE/configs/golden_jsons/Run2016APV.txt"};
constexpr auto Run2016 = RunConfig{.golden_json = "$MUSIC_BASE/configs/golden_jsons/Run2016.txt"};
constexpr auto Run2017 = RunConfig{.golden_json = "$MUSIC_BASE/configs/golden_jsons/Run2017.txt"};
constexpr auto Run2018 = RunConfig{.golden_json = "$MUSIC_BASE/configs/golden_jsons/Run2018.txt"};
constexpr std::array<RunConfig, Year::kTotalYears> Runs = {Run2016APV, Run2016, Run2017, Run2018};

} // namespace RunConfig

#endif /*MUSIC_CONFIG*/