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
    float PreSelPt = 20;
    float MinLowPt = 25;
    float MaxLowPt = 120;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10;                   // smaller than this
    float PFRelIso_WP = 0.15;                   // smaller than this
    std::string_view TightIdVar = "TightIdVar"; // dummy
    std::string_view HEEPIdVar = "HEEPIdVar";   // dummy
    std::string_view IsoVar = "IsoVar";         // dummy
};

constexpr auto Muon2016APV = MuonConfig{};
constexpr auto Muon2016 = MuonConfig{};
constexpr auto Muon2017 = MuonConfig{.MinLowPt = 28.};
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
    float TkRelIso_WP = 0.10;                   // smaller than this
    float PFRelIso_WP = 0.15;                   // smaller than this
    std::string_view TightIdVar = "TightIdVar"; // dummy
    std::string_view HEEPIdVar = "HEEPIdVar";   // dummy
    std::string_view IsoVar = "IsoVar";         // dummy
};

constexpr auto Electron2016APV = ElectronConfig{};
constexpr auto Electron2016 = ElectronConfig{};
constexpr auto Electron2017 = ElectronConfig{.MinLowPt = 28.};
constexpr auto Electron2018 = ElectronConfig{};
constexpr std::array<ElectronConfig, Year::kTotalYears> Electrons = {Electron2016APV, Electron2016, Electron2017, Electron2018};

// Photons
struct PhotonConfig
{
    float PreSelPt = 20;
    float MinLowPt = 25;
    float MaxLowPt = 120;
    float MaxAbsEta = 2.4;
    float MaxDeltaRTriggerMatch = 0.1;
    float TkRelIso_WP = 0.10;                   // smaller than this
    float PFRelIso_WP = 0.15;                   // smaller than this
    std::string_view TightIdVar = "TightIdVar"; // dummy
    std::string_view HEEPIdVar = "HEEPIdVar";   // dummy
    std::string_view IsoVar = "IsoVar";         // dummy
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
    float TkRelIso_WP = 0.10;                   // smaller than this
    float PFRelIso_WP = 0.15;                   // smaller than this
    std::string_view TightIdVar = "TightIdVar"; // dummy
    std::string_view HEEPIdVar = "HEEPIdVar";   // dummy
    std::string_view IsoVar = "IsoVar";         // dummy
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
    float TkRelIso_WP = 0.10;                   // smaller than this
    float PFRelIso_WP = 0.15;                   // smaller than this
    std::string_view TightIdVar = "TightIdVar"; // dummy
    std::string_view HEEPIdVar = "HEEPIdVar";   // dummy
    std::string_view IsoVar = "IsoVar";         // dummy
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
    std::string_view TightIdVar = "TightIdVar"; // dummy
    std::string_view HEEPIdVar = "HEEPIdVar";   // dummy
    std::string_view IsoVar = "IsoVar";         // dummy
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
    std::string_view TightIdVar = "TightIdVar"; // dummy
    std::string_view HEEPIdVar = "HEEPIdVar";   // dummy
    std::string_view IsoVar = "IsoVar";         // dummy
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

class TaskConfiguration
{
  public:
    const std::string run_config_file;
    const TOMLConfig run_config;
    const std::string output_directory;
    const std::string process;
    const std::string dataset;
    const bool is_data;
    const bool is_crab_job;
    const std::string x_section_file;
    const std::string run_hash;
    const std::string year_str;
    const std::vector<std::string> input_files;
    const int _n_threads;
    const std::size_t n_threads;
    const Year year;
    const std::string golden_json_file;

    TaskConfiguration(const std::string _run_config_file)
        : run_config_file(_run_config_file), run_config(TOMLConfig::make_toml_config(run_config_file)),
          output_directory(run_config.get<std::string>("output")), process(run_config.get<std::string>("process")),
          dataset(run_config.get<std::string>("dataset")), is_data(run_config.get<bool>("is_data")),
          is_crab_job(run_config.get<bool>("is_crab_job")),
          x_section_file(MUSiCTools::parse_and_expand_music_base(run_config.get<std::string>("x_section_file"))),
          run_hash(run_config.get<std::string>("hash")), year_str(run_config.get<std::string>("year")),
          input_files(run_config.get_vector<std::string>("input_files")), _n_threads(run_config.get<int>("n_threads")),
          n_threads(std::min(_n_threads, static_cast<int>(input_files.size()))), year(get_runyear(year_str)),
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
    }
};

#endif /*MUSIC_CONFIG*/