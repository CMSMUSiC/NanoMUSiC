#ifndef MUSIC_CONFIG
#define MUSIC_CONFIG

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

auto get_runyear(const std::string &year_str)
{
    // check year
    if (year_str != "2016APV" && year_str != "2016" && year_str != "2017" && year_str != "2018")
    {
        auto error_msg = "ERROR: year should be 2016APV, 2016, 2017 or 2018";
        throw std::out_of_range{error_msg};
    }

    // return year as enum
    if (year_str == "2016APV")
    {
        return Year::Run2016APV;
    }
    else if (year_str == "2016")
    {
        return Year::Run2016;
    }
    else if (year_str == "2017")
    {
        return Year::Run2017;
    }
    else
    {
        return Year::Run2018;
    }
}

namespace ObjConfig
{
// Muons
//  REF: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonUL2017
struct MuonConfig
{
    float MinLowPt = 25;
    float MaxLowPt = 120;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10; // smaller than this
    float PFRelIso_WP = 0.15; // smaller than this
};

constexpr auto Muon2016APV = MuonConfig{};
constexpr auto Muon2016 = MuonConfig{};
constexpr auto Muon2017 = MuonConfig{.MinLowPt = 28.};
constexpr auto Muon2018 = MuonConfig{};
constexpr std::array<MuonConfig, Year::kTotalYears> Muons = {Muon2016APV, Muon2016, Muon2017, Muon2018};

// Jets
// REF: https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation106XUL17#AK4_b_tagging
struct JetConfig
{
    std::string_view btag_algo = "btagDeepFlavB"; // DeepJet=DeepFlavour
    float btag_wp_tight = 0.7476;
};

constexpr auto Jet2016APV = JetConfig{};
constexpr auto Jet2016 = JetConfig{};
constexpr auto Jet2017 = JetConfig{};
constexpr auto Jet2018 = JetConfig{};
constexpr std::array<JetConfig, Year::kTotalYears> Jets = {Jet2016APV, Jet2016, Jet2017, Jet2018};

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

// constexpr auto my_muon_filter = [](const auto &muon) {
//     if (muon > 25.)
//     {
//         return true;
//     }
//     return false;
// };

} // namespace RunConfig

#endif /*MUSIC_CONFIG*/