#ifndef MUSIC_CONFIG
#define MUSIC_CONFIG

#include <filesystem>

#include "Enumerate.hpp"
#include "MUSiCTools.hpp"
#include "TOMLConfig.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

#define BOOST_HANA_CONFIG_ENABLE_STRING_UDL 1
#include <boost/hana.hpp>
using namespace boost::hana::literals;
using namespace std::literals;

// useful hana macros
#define ADD_TO_TRIGGER_RED_LIST(DATA_STREAM, ...)                                                                      \
    boost::hana::make_pair(DATA_STREAM, boost::hana::make_tuple(__VA_ARGS__))
#define HANA_TO_SV(DATA) std::string_view(DATA.c_str())
#define HANA_FIRST(DATA) boost::hana::first(DATA)
#define HANA_SECOND(DATA) boost::hana::second(DATA)
#define HANA_FIRST_TO_SV(DATA) std::string_view(boost::hana::first(DATA).c_str())
#define HANA_SECOND_TO_SV(DATA) std::string_view(boost::hana::second(DATA).c_str())

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

auto get_runyear(const std::string &year_str) -> Year
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
    float MinLowPt = 26.;
    float MaxLowPt = 120.;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10; // smaller than this
    float PFRelIso_WP = 0.15; // smaller than this
};

constexpr auto Muon2016APV = MuonConfig{};
constexpr auto Muon2016 = MuonConfig{};
constexpr auto Muon2017 = MuonConfig{.MinLowPt = 29.};
constexpr auto Muon2018 = MuonConfig{};
constexpr std::array<MuonConfig, Year::kTotalYears> Muons = {Muon2016APV, Muon2016, Muon2017, Muon2018};

// Electrons
struct ElectronConfig
{
    float PreSelPt = 20;
    float MinLowPt = 25;
    float MaxLowPt = 120;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10; // smaller than this
    float PFRelIso_WP = 0.15; // smaller than this
};

constexpr auto Electron2016APV = ElectronConfig{};
constexpr auto Electron2016 = ElectronConfig{};
constexpr auto Electron2017 = ElectronConfig{.MinLowPt = 28.};
constexpr auto Electron2018 = ElectronConfig{};
constexpr std::array<ElectronConfig, Year::kTotalYears> Electrons = {Electron2016APV, Electron2016, Electron2017,
                                                                     Electron2018};

// Photons
struct PhotonConfig
{
    float PreSelPt = 20;
    float MinLowPt = 25;
    float MaxLowPt = 120;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10; // smaller than this
    float PFRelIso_WP = 0.15; // smaller than this
};

constexpr auto Photon2016APV = PhotonConfig{};
constexpr auto Photon2016 = PhotonConfig{};
constexpr auto Photon2017 = PhotonConfig{.MinLowPt = 28.};
constexpr auto Photon2018 = PhotonConfig{};
constexpr std::array<PhotonConfig, Year::kTotalYears> Photons = {Photon2016APV, Photon2016, Photon2017, Photon2018};

// Taus
struct TauConfig
{
    float PreSelPt = 20;
    float MinLowPt = 25;
    float MaxLowPt = 120;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10; // smaller than this
    float PFRelIso_WP = 0.15; // smaller than this
};

constexpr auto Tau2016APV = TauConfig{};
constexpr auto Tau2016 = TauConfig{};
constexpr auto Tau2017 = TauConfig{.MinLowPt = 28.};
constexpr auto Tau2018 = TauConfig{};
constexpr std::array<TauConfig, Year::kTotalYears> Taus = {Tau2016APV, Tau2016, Tau2017, Tau2018};

// Bjets
struct BjetsConfig
{
    float PreSelPt = 20;
    float MinLowPt = 25;
    float MaxLowPt = 120;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10; // smaller than this
    float PFRelIso_WP = 0.15; // smaller than this
};

constexpr auto Bjets2016APV = BjetsConfig{};
constexpr auto Bjets2016 = BjetsConfig{};
constexpr auto Bjets2017 = BjetsConfig{.MinLowPt = 28.};
constexpr auto Bjets2018 = BjetsConfig{};
constexpr std::array<BjetsConfig, Year::kTotalYears> BJets = {Bjets2016APV, Bjets2016, Bjets2017, Bjets2018};

// Jets
// REF: https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation106XUL17#AK4_b_tagging
struct JetConfig
{
    float PreSelPt = 20;
    float MinLowPt = 25;
    float MaxLowPt = 120;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10;                     // smaller than this
    float PFRelIso_WP = 0.15;                     // smaller than this
    std::string_view btag_algo = "btagDeepFlavB"; // DeepJet=DeepFlavour
    float btag_wp_tight = 0.7476;
};

constexpr auto Jet2016APV = JetConfig{};
constexpr auto Jet2016 = JetConfig{};
constexpr auto Jet2017 = JetConfig{};
constexpr auto Jet2018 = JetConfig{};
constexpr std::array<JetConfig, Year::kTotalYears> Jets = {Jet2016APV, Jet2016, Jet2017, Jet2018};

// MET
// REF: https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation106XUL17#AK4_b_tagging
struct METConfig
{
    float PreSelPt = 20;
    float MinLowPt = 25;
    float MaxLowPt = 120;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10;                     // smaller than this
    float PFRelIso_WP = 0.15;                     // smaller than this
    std::string_view btag_algo = "btagDeepFlavB"; // DeepMET=DeepFlavour
    float btag_wp_tight = 0.7476;
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

namespace TriggerConfig
{
////////////////////////////////////////////////////////
/// List of unwanted Trigger paths mapped to data sample stream
///
constexpr auto TriggerStreamRedList = //
    boost::hana::make_map(
        ADD_TO_TRIGGER_RED_LIST("MC"_s),                                                                      //
        ADD_TO_TRIGGER_RED_LIST("SingleMuon"_s),                                                              //
        ADD_TO_TRIGGER_RED_LIST("SingleElectron"_s, "SingleMuonLowPt"sv, "SingleMuonHighPt"sv),               //
        ADD_TO_TRIGGER_RED_LIST("DoubleMuon"_s, "SingleMuonLowPt"sv, "SingleMuonHighPt"sv, "SingleElectron"), //
        ADD_TO_TRIGGER_RED_LIST("DoubleEG"_s, "SingleMuonLowPt"sv, "SingleMuonHighPt"sv, "SingleElectron"sv,
                                "DoubleMuon"), //
        ADD_TO_TRIGGER_RED_LIST("SinglePhoton"_s, "SingleMuonLowPt"sv, "SingleMuonHighPt"sv, "SingleElectron"sv,
                                "DoubleMuon"sv, "DoubleElectron"), //
        ADD_TO_TRIGGER_RED_LIST("Tau"_s, "SingleMuonLowPt"sv, "SingleMuonHighPt"sv, "SingleElectron"sv, "DoubleMuon"sv,
                                "DoubleElectron"sv, "Photon"), //
        ADD_TO_TRIGGER_RED_LIST("BTagCSV"_s, "SingleMuonLowPt"sv, "SingleMuonHighPt"sv, "SingleElectron"sv,
                                "DoubleMuon"sv, "DoubleElectron"sv, "Photon"sv, "Tau"), //
        ADD_TO_TRIGGER_RED_LIST("JetHT"_s, "SingleMuonLowPt"sv, "SingleMuonHighPt"sv, "SingleElectron"sv,
                                "DoubleMuon"sv, "DoubleElectron"sv, "Photon"sv, "Tau"sv, "BJet"), //
        ADD_TO_TRIGGER_RED_LIST("MET"_s, "SingleMuonLowPt"sv, "SingleMuonHighPt"sv, "SingleElectron"sv, "DoubleMuon"sv,
                                "DoubleElectron"sv, "Photon"sv, "Tau"sv, "BJet"sv, "Jet"sv) //
    );

// get data_stream from process
std::string_view get_trigger_stream(bool is_data, const std::string &process)
{
    using namespace std::literals;
    if (not is_data)
    {
        return "MC"sv;
    }
    else
    {
        std::string_view _buffer = "";
        boost::hana::for_each(TriggerStreamRedList, [&](const auto &stream) {
            if (process.rfind(HANA_FIRST_TO_SV(stream), 0) == 0 and _buffer == "")
            {
                _buffer = HANA_FIRST_TO_SV(stream);
            }
        });

        if (_buffer != "")
        {
            return _buffer;
        }
    }
    throw std::runtime_error("[ ERROR ] Data Stream not found for this process name: \""s + process + "\"."s);
}
} // namespace TriggerConfig

class TaskConfiguration
{
  public:
    const std::string run_config_file;
    const TOMLConfig run_config;
    const std::string output_directory;
    const std::string process;
    const std::string trigger_stream;
    const std::string dataset;
    const bool is_data;
    const bool is_crab_job;
    const std::string x_section_file;
    const std::string run_hash;
    const std::string year_str;
    const std::vector<std::string> input_files;
    const Year year;
    const std::string golden_json_file;

    TaskConfiguration(const std::string _run_config_file)
        : run_config_file(_run_config_file), run_config(TOMLConfig::make_toml_config(run_config_file)),
          output_directory(run_config.get<std::string>("output")), process(run_config.get<std::string>("process")),
          trigger_stream(TriggerConfig::get_trigger_stream(run_config.get<bool>("is_data"), process)),
          dataset(run_config.get<std::string>("dataset")), is_data(run_config.get<bool>("is_data")),
          is_crab_job(run_config.get<bool>("is_crab_job")),
          x_section_file(MUSiCTools::parse_and_expand_music_base(run_config.get<std::string>("x_section_file"))),
          run_hash(run_config.get<std::string>("hash")), year_str(run_config.get<std::string>("year")),
          input_files(run_config.get_vector<std::string>("input_files")), year(get_runyear(year_str)),
          golden_json_file(MUSiCTools::parse_and_expand_music_base(RunConfig::Runs[year].golden_json))

    {
        if (is_data)
        {
            if (not std::filesystem::exists(golden_json_file))
            {
                std::stringstream error;
                error << "golden_json_file not found";
                throw MUSiCTools::config_error(error.str());
            }
        }
        if (!golden_json_file.empty())
        {
            std::cout << "INFO: Using Run/Lumi JSON file: " << golden_json_file << std::endl;
        }

        // print configuratiojn summmary
        fmt::print(fmt::emphasis::bold, "\n=====================================\n");
        fmt::print(fmt::emphasis::bold, "Task Configration:\n");
        fmt::print(fmt::emphasis::bold, "-------------------------------------\n");
        fmt::print(fmt::emphasis::bold, "Configuration file: {}\n", run_config_file);
        fmt::print(fmt::emphasis::bold, "Output Directory: {}\n", output_directory);
        fmt::print(fmt::emphasis::bold, "Process Name: {}\n", process);
        fmt::print(fmt::emphasis::bold, "Trigger Stream: {}\n", trigger_stream);
        fmt::print(fmt::emphasis::bold, "Dataset: {}\n", dataset);
        fmt::print(fmt::emphasis::bold, "Is Data (?): {}\n", is_data);
        fmt::print(fmt::emphasis::bold, "Is a CRAB job (?): {}\n", is_crab_job);
        fmt::print(fmt::emphasis::bold, "Cross-sections File: {}\n", x_section_file);
        fmt::print(fmt::emphasis::bold, "Year: {}\n", year_str);
        fmt::print(fmt::emphasis::bold, "-------------------------------------\n");
    }
};

#endif /*MUSIC_CONFIG*/