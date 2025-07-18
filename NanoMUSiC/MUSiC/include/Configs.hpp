#ifndef MUSIC_CONFIG
#define MUSIC_CONFIG

#include <array>
#include <cstdlib>
#include <fmt/format.h>
#include <limits>
#include <stdexcept>

using namespace std::literals;

// Reference:
// https://pdg.lbl.gov/2019/reviews/rpp2019-rev-monte-carlo-numbering.pdf
namespace PDG
{
namespace Electron
{
constexpr int Id = 11;
constexpr float Mass = 0.51099895 / 1000.0;
} // namespace Electron

namespace ElectronNeutrino
{
constexpr int Id = 12;
constexpr float Mass = 0.;
} // namespace ElectronNeutrino

namespace Muon
{
constexpr int Id = 13;
constexpr float Mass = 105.6583755 / 1000.0;
} // namespace Muon

namespace MuonNeutrino
{
constexpr int Id = 14;
constexpr float Mass = 0.;
} // namespace MuonNeutrino

namespace Tau
{
constexpr int Id = 15;
constexpr float Mass = 1776.86 / 1000.;
} // namespace Tau

namespace TauNeutrino
{
constexpr int Id = 16;
constexpr float Mass = 0.;
} // namespace TauNeutrino

namespace Bottom
{
constexpr int Id = 5;
constexpr float Mass = 4.18;
} // namespace Bottom
namespace Top
{
constexpr int Id = 6;
constexpr float Mass = 172.76;
} // namespace Top

namespace Gluon
{
constexpr int Id = 21;
constexpr float Mass = 0.;
} // namespace Gluon

namespace Photon
{
constexpr int Id = 22;
constexpr float Mass = 0;
} // namespace Photon

namespace Z
{
constexpr int Id = 23;
constexpr float Mass = 91.1876;
} // namespace Z

namespace W
{
constexpr int Id = 24;
constexpr float Mass = 80.379; // NOT the CDF result
} // namespace W

inline auto get_mass_by_id(const int &id) -> float
{
    if (std::abs(id) == Electron::Id)
    {
        return Electron::Mass;
    }

    if (std::abs(id) == ElectronNeutrino::Id)
    {
        return ElectronNeutrino::Mass;
    }

    if (std::abs(id) == Muon::Id)
    {
        return Muon::Mass;
    }

    if (std::abs(id) == MuonNeutrino::Id)
    {
        return MuonNeutrino::Mass;
    }

    if (std::abs(id) == Tau::Id)
    {
        return Tau::Mass;
    }

    if (std::abs(id) == TauNeutrino::Id)
    {
        return TauNeutrino::Mass;
    }

    if (std::abs(id) == Gluon::Id)
    {
        return Gluon::Mass;
    }

    if (std::abs(id) == Photon::Id)
    {
        return Photon::Mass;
    }

    if (std::abs(id) == Z::Id)
    {
        return Z::Mass;
    }

    if (std::abs(id) == W::Id)
    {
        return W::Mass;
    }

    throw(std::runtime_error(fmt::format("Invalid PDG Id ({}).", id)));
}

} // namespace PDG

enum Year
{
    Run2016APV,
    Run2016,
    Run2017,
    Run2018,
    kTotalYears, // <-- should always be the last one!!
};

inline auto format_as(Year y)
{
    switch (y)
    {
    case Run2016APV:
        return "Run2016APV";
    case Run2016:
        return "Run2016";
    case Run2017:
        return "Run2017";
    case Run2018:
        return "Run2018";
    default:
        fmt::print(stderr, "ERROR: Could not format Year.");
        std::exit(EXIT_FAILURE);
    }
}

inline auto get_runyear(const std::string &year_str) -> Year
{
    // check year
    if (year_str != "2016APV" and year_str != "2016" and year_str != "2017" and year_str != "2018")
    {
        fmt::print(stderr, "ERROR: year should be 2016APV, 2016, 2017 or 2018\n");
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
/// Muons
/// References:
/// https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonUL2017
/// https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonIdRun2#Tracker_based_Isolation
struct MuonConfig
{
    // float LowPt = 8.;
    float MediumPt = 25.;
    float HighPt = 200.;
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
    // float LowPt = 10.001;
    float MediumPt = 25.;
    // float MediumPt = 40.;
    float HighPt = 100.;
    float MaxAbsEta = 2.5;
    float MaxDeltaRTriggerMatch = 0.3;
    int cutBasedId = 4; // (tight) greater than this
};

constexpr auto Electron2016APV = ElectronConfig{};
constexpr auto Electron2016 = ElectronConfig{};
constexpr auto Electron2017 = ElectronConfig{};
constexpr auto Electron2018 = ElectronConfig{};
constexpr std::array<ElectronConfig, Year::kTotalYears> Electrons = {Electron2016APV,
                                                                     Electron2016,
                                                                     Electron2017,
                                                                     Electron2018};

// Photons
struct PhotonConfig
{
    // float LowPt = 25.;
    // float MediumPt = 40.;
    float MediumPt = 25.;
    float HighPt = 200.;
    int cutBasedId = 3; // (tight) greater than this
};

constexpr auto Photon2016APV = PhotonConfig{};
constexpr auto Photon2016 = PhotonConfig{};
constexpr auto Photon2017 = PhotonConfig{};
constexpr auto Photon2018 = PhotonConfig{};
constexpr std::array<PhotonConfig, Year::kTotalYears> Photons = {Photon2016APV, Photon2016, Photon2017, Photon2018};

// Taus
struct TauConfig
{
    float LowPt = 20.;
    float MediumPt = 40.;
    float HighPt = 142.;
};

constexpr auto Tau2016APV = TauConfig{};
constexpr auto Tau2016 = TauConfig{};
constexpr auto Tau2017 = TauConfig{};
constexpr auto Tau2018 = TauConfig{};
constexpr std::array<TauConfig, Year::kTotalYears> Taus = {Tau2016APV, Tau2016, Tau2017, Tau2018};

// Bjets
// REF: https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation106XUL17#AK4_b_tagging
struct BjetsConfig
{
    // float LowPt = 50.;
    float MediumPt = 50.;
    float HighPt = 500.;
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
struct JetConfig
{
    // float LowPt = 50.;
    float MediumPt = 50.;
    float HighPt = 500.;
    float MaxAbsEta = 2.4;
    // int MinJetID = 2;              // Tight - equal or greater than this
    int MinJetID = 4;              // tightLepVeto - equal or greater than this
    float MaxBTagWPTight = 0.6502; // smaller than this
};

constexpr auto Jet2016APV = JetConfig{.MaxBTagWPTight = 0.6502};
constexpr auto Jet2016 = JetConfig{.MaxBTagWPTight = 0.6377};
constexpr auto Jet2017 = JetConfig{.MaxBTagWPTight = 0.7476};
constexpr auto Jet2018 = JetConfig{.MaxBTagWPTight = 0.71};
constexpr std::array<JetConfig, Year::kTotalYears> Jets = {Jet2016APV, Jet2016, Jet2017, Jet2018};

// MET
struct METConfig
{
    float MediumPt = 100.;
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

constexpr auto Run2016APV = RunConfig{.golden_json = "$MUSIC_BASE/configs/data/golden_jsons/Run2016APV.txt"};
constexpr auto Run2016 = RunConfig{.golden_json = "$MUSIC_BASE/configs/data/golden_jsons/Run2016.txt"};
constexpr auto Run2017 = RunConfig{.golden_json = "$MUSIC_BASE/configs/data/golden_jsons/Run2017.txt"};
constexpr auto Run2018 = RunConfig{.golden_json = "$MUSIC_BASE/configs/data/golden_jsons/Run2018.txt"};
constexpr std::array<RunConfig, Year::kTotalYears> Runs = {Run2016APV, Run2016, Run2017, Run2018};

} // namespace RunConfig

#endif /*MUSIC_CONFIG*/
