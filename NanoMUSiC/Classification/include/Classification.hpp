#ifndef CLASSIFICATION
#define CLASSIFICATION

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <sys/time.h>
#include <unordered_map>

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "RtypesCore.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

#include "JetCorrector.hpp"
#include "MUSiCTools.hpp"
#include "NanoAODGenInfo.hpp"
#include "TOMLConfig.hpp"

#include "PDFAlphaSWeights.hpp"

#include "ValidationContainer.hpp"
#include "fmt/format.h"

#include "ObjectFactories/make_electrons.hpp"
#include "ObjectFactories/make_jets.hpp"
#include "ObjectFactories/make_met.hpp"
#include "ObjectFactories/make_muons.hpp"
#include "ObjectFactories/make_photons.hpp"
#include "ObjectFactories/make_taus.hpp"

#include "CorrectionLibUtils.hpp"
#include "TriggerMatch.hpp"

#include "PDFAlphaSWeight.hpp"
#include "Shifts.hpp"

#include "EventClass.hpp"

#include "json.hpp"
using json = nlohmann::json;

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

template <typename T>
using OptValueReader_t = std::optional<TTreeReaderValue<T>>;

template <typename T>
using OptArrayReader_t = std::optional<TTreeReaderArray<T>>;

template <typename T>
auto make_value_reader(TTreeReader &tree_reader, const std::string &leaf) -> OptValueReader_t<T>
{
    if (tree_reader.GetTree()->GetLeaf(leaf.c_str()) != nullptr)
    {
        return std::make_optional<TTreeReaderValue<T>>(tree_reader, leaf.c_str());
    }

    fmt::print(stderr, "WARNING: Could not read branch: {}\n", leaf);
    return std::nullopt;
}

template <typename T>
auto make_array_reader(TTreeReader &tree_reader, const std::string &leaf) -> OptArrayReader_t<T>
{
    if (tree_reader.GetTree()->GetLeaf(leaf.c_str()) != nullptr)
    {
        return std::make_optional<TTreeReaderArray<T>>(tree_reader, leaf.c_str());
    }

    fmt::print(stderr, "WARNING: Could not read branch: {}\n", leaf);
    return std::nullopt;
}

// helper macros
#define ADD_VALUE_READER(VAR, TYPE)                                                                                    \
    input_ttree->SetBranchStatus(#VAR, true);                                                                          \
    auto VAR = make_value_reader<TYPE>(tree_reader, #VAR)

#define ADD_ARRAY_READER(VAR, TYPE)                                                                                    \
    input_ttree->SetBranchStatus(#VAR, true);                                                                          \
    auto VAR = make_array_reader<TYPE>(tree_reader, #VAR)

template <typename T>
auto unwrap(std::optional<TTreeReaderValue<T>> &value, bool allow_nan_or_inf = false) -> T
{
    if (value)
    {
        auto _this_value = **value;
        if (not(allow_nan_or_inf))
        {
            if (std::isnan(_this_value) or std::isinf(_this_value))
            {
                fmt::print(stderr,
                           "ERROR: Could not unwrap value: {}. NaN or INF found. Unwraped value: {}.\n",
                           (*value).GetBranchName(),
                           _this_value);
                std::exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (std::isnan(_this_value) or std::isinf(_this_value))
            {
                fmt::print(stderr,
                           "WARINING: Could not unwrap value: {}. NaN or INF found. Unwraped value: {}.\n",
                           (*value).GetBranchName(),
                           _this_value);
                return T();
            }
        }
        return _this_value;
    }
    return T();
}

template <typename T, typename Q>
auto unwrap_or(std::optional<TTreeReaderValue<T>> &value, Q &&default_value = Q(), bool allow_nan_or_inf = false) -> T
{
    static_assert(std::is_arithmetic<T>::value, "The default type must be numeric.");

    if (value)
    {
        auto _this_value = **value;
        if (not(allow_nan_or_inf))
        {
            if (std::isnan(_this_value) or std::isinf(_this_value))
            {
                fmt::print(stderr,
                           "ERROR: Could not unwrap value: {}. NaN or INF found. Unwraped value: {}.\n",
                           (*value).GetBranchName(),
                           _this_value);
                std::exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (std::isnan(_this_value) or std::isinf(_this_value))
            {
                fmt::print(stderr,
                           "WARINING: Could not unwrap value: {}. NaN or INF found. Unwraped value: {}.\n",
                           (*value).GetBranchName(),
                           _this_value);
                return static_cast<T>(default_value);
            }
        }
        return _this_value;
    }
    return static_cast<T>(default_value);
}

template <typename T>
auto unwrap(std::optional<TTreeReaderArray<T>> &array, bool allow_nan_or_inf = false) -> RVec<T>
{
    if (array)
    {
        auto _this_array = RVec<T>((*array).cbegin(), (*array).cend());
        if (not(allow_nan_or_inf))
        {
            for (const auto &_this_value : _this_array)
            {
                if (std::isnan(_this_value) or std::isinf(_this_value))
                {
                    fmt::print(stderr,
                               "ERROR: Could not unwrap array: {}. NaN or INF found. Unwraped array: [{}].\n",
                               (*array).GetBranchName(),
                               fmt::join(_this_array, ", "));
                    std::exit(EXIT_FAILURE);
                }
            }
        }
        return _this_array;
    }
    return RVec<T>();
}

template <typename T, typename Q, typename R>
auto unwrap_or(std::optional<TTreeReaderArray<T>> &array,
               Q &&default_value,
               R &&default_size,
               bool allow_nan_or_inf = false) -> T
{
    if (array)
    {
        auto _this_array = RVec<T>((*array).cbegin(), (*array).cend());
        if (not(allow_nan_or_inf))
        {
            for (const auto &_this_value : _this_array)
            {
                if (std::isnan(_this_value) or std::isinf(_this_value))
                {
                    fmt::print(stderr,
                               "ERROR: Could not unwrap array: {}. NaN or INF found. Unwraped array: [{}].\n",
                               (*array).GetBranchName(),
                               fmt::join(_this_array, ", "));
                    std::exit(EXIT_FAILURE);
                }
            }
        }
        return _this_array;
    }
    return RVec<T>(default_size, default_value);
}

inline auto PrintProcessInfo() -> void
{
    auto info = ProcInfo_t();
    gSystem->GetProcInfo(&info);
    std::cout.precision(1);
    std::cout << std::fixed;
    std::cout << "-------------" << std::endl;
    std::cout << "Process info:" << std::endl;
    std::cout << "-------------" << std::endl;
    std::cout << "CPU time elapsed: " << info.fCpuUser << " s" << std::endl;
    std::cout << "Sys time elapsed: " << info.fCpuSys << " s" << std::endl;
    std::cout << "Resident memory:  " << info.fMemResident / 1024. << " MB" << std::endl;
    std::cout << "Virtual memory:   " << info.fMemVirtual / 1024. << " MB" << std::endl;
}

inline auto getCpuTime() -> double
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0);
}

class LocalFile
{
  private:
    std::string input_file;
    std::string local_file;

  public:
    LocalFile(const std::string &_input_file)
        : input_file(_input_file)
    {
        std::hash<std::string> hasher;

        local_file = fmt::format("local_file_{}.root", hasher(input_file));

        // check if file already exists
        auto found_file = false;
        std::filesystem::path currentDir = std::filesystem::current_path();
        for (const auto &entry : std::filesystem::directory_iterator(currentDir))
        {
            if (entry.is_regular_file() && entry.path().filename() == local_file)
            {
                found_file = true;
            }
        }
        if (not(found_file))
        {
            std::system(fmt::format("rm -rf local_file_*.root").c_str());

            // download new file
            auto xrdcp_return_code = std::system(fmt::format("xrdcp --silent {} {} ", input_file, local_file).c_str());
            if (xrdcp_return_code != 0)
            {
                fmt::print(stderr, "ERROR: Could not download input file: {}\n", input_file);
                std::exit(EXIT_FAILURE);
            }
        }
    }

    auto get_local_file() const
    {
        return local_file;
    }

    auto get_input_file() const
    {
        return input_file;
    }
};

inline auto split_string(const std::string &input, const std::string &delimiter) -> std::vector<std::string>
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = input.find(delimiter);

    while (end != std::string::npos)
    {
        result.push_back(input.substr(start, end - start));
        start = end + delimiter.length();
        end = input.find(delimiter, start);
    }

    result.push_back(input.substr(start)); // Add the last token

    return result;
}

template <typename T>
inline auto save_as(T &histo, std::string &&filename) -> void
{
    system(fmt::format("rm {}.png", filename).c_str());
    system(fmt::format("rm {}.pdf", filename).c_str());

    auto c = TCanvas();

    // Set logarithmic scale on the y-axis
    c.SetLogy();

    histo.Draw("ep1");

    c.SaveAs((filename + ".png").c_str());
    c.SaveAs((filename + ".pdf").c_str());
}

inline auto get_era_from_process_name(const std::string &process, bool is_data) -> std::string
{
    if (is_data)
    {
        if (not(process.empty()))
        {
            return process.substr(process.length() - 1);
        }
        throw std::runtime_error(fmt::format("ERROR: Could not get era from process name ({}).\n", process));
    }
    return "_";
}

inline auto is_data_to_string(bool is_data) -> std::string
{
    if (is_data)
    {
        return "Data";
    }
    return "MC";
}

inline auto get_output_file_path(const std::string &prefix,
                                 const std::string &output_path,
                                 const std::string &process,
                                 const std::string &year,
                                 const std::string &process_group,
                                 const std::string &xs_order,
                                 bool is_data,
                                 const std::string &buffer_index,
                                 const std::string &suffix = ".root") -> std::string
{
    return fmt::format("{}/{}_{}_{}_{}_{}_{}_{}{}",
                       output_path,
                       prefix,
                       process,
                       year,
                       process_group,
                       xs_order,
                       is_data_to_string(is_data),
                       buffer_index,
                       suffix);
}

inline auto starts_with(const std::string &str, std::string &&prefix) -> bool
{
    return (str.rfind(prefix, 0) == 0);
}

// check if an event pass any trigger
inline auto trigger_filter(const std::string &process,         //
                           bool is_data,
                           Year year,                          //
                           bool pass_low_pt_muon_trigger,      //
                           bool pass_high_pt_muon_trigger,     //
                           bool pass_double_muon_trigger,      //
                           bool pass_low_pt_electron_trigger,  //
                           bool pass_high_pt_electron_trigger, //
                           bool pass_double_electron_trigger,  //
                           bool pass_high_pt_tau_trigger,      //
                           bool pass_double_tau_trigger,       //
                           bool pass_photon_trigger,
                           bool pass_double_photon_trigger) -> std::optional<std::unordered_map<std::string, bool>>
{
    std::optional<std::unordered_map<std::string, bool>> trigger_filter_res = std::nullopt;

    // Data
    if (is_data)
    {
        // SingleMuon dataset
        if (starts_with(process, "SingleMuon"))
        {
            if (pass_low_pt_muon_trigger or pass_high_pt_muon_trigger)
            {
                trigger_filter_res = {
                    {"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},           //
                    {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},         //
                    {"pass_double_muon_trigger", pass_double_muon_trigger},           //
                    {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},   //
                    {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}, //
                    {"pass_double_electron_trigger", pass_double_electron_trigger},   //
                    {"pass_high_pt_tau_trigger", pass_high_pt_tau_trigger},           //
                    {"pass_double_tau_trigger", pass_double_tau_trigger},             //
                    {"pass_photon_trigger", pass_photon_trigger},                     //
                    {"pass_double_photon_trigger", pass_double_photon_trigger}        //
                };
            }

            return trigger_filter_res;
        }

        // DoubleMuon dataset
        if (starts_with(process, "DoubleMuon"))
        {
            if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger) //
                and pass_double_muon_trigger)
            {
                trigger_filter_res = {
                    {"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},           //
                    {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},         //
                    {"pass_double_muon_trigger", pass_double_muon_trigger},           //
                    {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},   //
                    {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}, //
                    {"pass_double_electron_trigger", pass_double_electron_trigger},   //
                    {"pass_high_pt_tau_trigger", pass_high_pt_tau_trigger},           //
                    {"pass_double_tau_trigger", pass_double_tau_trigger},             //
                    {"pass_photon_trigger", pass_photon_trigger},                     //
                    {"pass_double_photon_trigger", pass_double_photon_trigger}        //
                };
            }

            return trigger_filter_res;
        }

        // Electron/Photon/EGamma dataset
        if (year != Year::Run2018)
        {
            if (starts_with(process, "SingleElectron"))
            {
                if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_double_muon_trigger) //
                    and (pass_low_pt_electron_trigger or pass_high_pt_electron_trigger))
                {
                    trigger_filter_res = {
                        {"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},           //
                        {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},         //
                        {"pass_double_muon_trigger", pass_double_muon_trigger},           //
                        {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},   //
                        {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}, //
                        {"pass_double_electron_trigger", pass_double_electron_trigger},   //
                        {"pass_high_pt_tau_trigger", pass_high_pt_tau_trigger},           //
                        {"pass_double_tau_trigger", pass_double_tau_trigger},             //
                        {"pass_photon_trigger", pass_photon_trigger},                     //
                        {"pass_double_photon_trigger", pass_double_photon_trigger}        //
                    };
                }

                return trigger_filter_res;
            }

            if (starts_with(process, "DoubleEG"))
            {
                if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_double_muon_trigger or
                        pass_low_pt_electron_trigger or pass_high_pt_electron_trigger) //
                    and (pass_double_electron_trigger or pass_double_photon_trigger))
                {
                    trigger_filter_res = {
                        {"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},           //
                        {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},         //
                        {"pass_double_muon_trigger", pass_double_muon_trigger},           //
                        {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},   //
                        {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}, //
                        {"pass_double_electron_trigger", pass_double_electron_trigger},   //
                        {"pass_high_pt_tau_trigger", pass_high_pt_tau_trigger},           //
                        {"pass_double_tau_trigger", pass_double_tau_trigger},             //
                        {"pass_photon_trigger", pass_photon_trigger},                     //
                        {"pass_double_photon_trigger", pass_double_photon_trigger}        //
                    };
                }

                return trigger_filter_res;
            }

            if (starts_with(process, "SinglePhoton"))
            {
                if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_double_muon_trigger or
                        pass_low_pt_electron_trigger or pass_high_pt_electron_trigger or pass_double_electron_trigger or
                        pass_double_photon_trigger) and
                    pass_photon_trigger)
                {
                    trigger_filter_res = {
                        {"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},           //
                        {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},         //
                        {"pass_double_muon_trigger", pass_double_muon_trigger},           //
                        {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},   //
                        {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}, //
                        {"pass_double_electron_trigger", pass_double_electron_trigger},   //
                        {"pass_high_pt_tau_trigger", pass_high_pt_tau_trigger},           //
                        {"pass_double_tau_trigger", pass_double_tau_trigger},             //
                        {"pass_photon_trigger", pass_photon_trigger},                     //
                        {"pass_double_photon_trigger", pass_double_photon_trigger}        //
                    };
                }

                return trigger_filter_res;
            }
        }
        else
        {
            if (starts_with(process, "EGamma"))
            {
                if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_double_muon_trigger) //
                    and (pass_low_pt_electron_trigger or pass_high_pt_electron_trigger or
                         pass_double_electron_trigger or pass_photon_trigger or pass_double_photon_trigger))
                {
                    trigger_filter_res = {
                        {"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},           //
                        {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},         //
                        {"pass_double_muon_trigger", pass_double_muon_trigger},           //
                        {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},   //
                        {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}, //
                        {"pass_double_electron_trigger", pass_double_electron_trigger},   //
                        {"pass_high_pt_tau_trigger", pass_high_pt_tau_trigger},           //
                        {"pass_double_tau_trigger", pass_double_tau_trigger},             //
                        {"pass_photon_trigger", pass_photon_trigger},                     //
                        {"pass_double_photon_trigger", pass_double_photon_trigger}        //
                    };
                }

                return trigger_filter_res;
            }
        }

        if (starts_with(process, "Tau"))
        {
            if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_double_muon_trigger or
                    pass_low_pt_electron_trigger or pass_high_pt_electron_trigger or pass_double_electron_trigger or
                    pass_photon_trigger or pass_double_photon_trigger) //
                and (pass_high_pt_tau_trigger or pass_double_tau_trigger))
            {
                trigger_filter_res = {
                    {"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},           //
                    {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},         //
                    {"pass_double_muon_trigger", pass_double_muon_trigger},           //
                    {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},   //
                    {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}, //
                    {"pass_double_electron_trigger", pass_double_electron_trigger},   //
                    {"pass_high_pt_tau_trigger", pass_high_pt_tau_trigger},           //
                    {"pass_double_tau_trigger", pass_double_tau_trigger},             //
                    {"pass_photon_trigger", pass_photon_trigger},                     //
                    {"pass_double_photon_trigger", pass_double_photon_trigger}        //
                };
            }

            return trigger_filter_res;
        }

        fmt::print(stderr,
                   "ERROR: Could not check trigger filter for Data file. The requested process ({}) does not "
                   "match any dataset pattern.",
                   process);
        std::exit(EXIT_FAILURE);
    }

    // MC
    if (                                 //
        pass_low_pt_muon_trigger         //
        or pass_high_pt_muon_trigger     //
        or pass_double_muon_trigger      //
        or pass_low_pt_electron_trigger  //
        or pass_high_pt_electron_trigger //
        or pass_double_electron_trigger  //
        or pass_high_pt_tau_trigger      //
        or pass_double_tau_trigger       //
        or pass_photon_trigger           //
        or pass_double_photon_trigger    //
    )
    {
        trigger_filter_res = {
            {"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},           //
            {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},         //
            {"pass_double_muon_trigger", pass_double_muon_trigger},           //
            {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},   //
            {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}, //
            {"pass_double_electron_trigger", pass_double_electron_trigger},   //
            {"pass_high_pt_tau_trigger", pass_high_pt_tau_trigger},           //
            {"pass_double_tau_trigger", pass_double_tau_trigger},             //
            {"pass_photon_trigger", pass_photon_trigger},                     //
            {"pass_double_photon_trigger", pass_double_photon_trigger}        //
        };
    }

    return trigger_filter_res;
};

struct FourVec
{
    double e;
    double et;
    double px;
    double py;
    double pz;
};

class TempEC
{
  public:
    static constexpr int max_allowed_jets_per_class = 6;

    const int max_muon_idx;
    const int max_electron_idx;
    const int max_tau_idx;
    const int max_photon_idx;
    const int max_bjet_idx;
    const int max_jet_idx;
    const int max_met_idx;

    const int num_muons;
    const int num_electrons;
    const int num_taus;
    const int num_photons;
    const int num_bjets;
    const int num_jets;
    const int num_met;

    const bool has_exclusive;
    const bool has_jet_inclusive;
    const bool has_met;

    double sum_pt;
    double met;
    FourVec four_vec;

    TempEC(int max_muon_idx,
           int max_electron_idx,
           int max_tau_idx,
           int max_photon_idx,
           int max_bjet_idx,
           int max_jet_idx,
           int max_met_idx,
           bool has_exclusive,
           bool has_jet_inclusive)
        : max_muon_idx(max_muon_idx),
          max_electron_idx(max_electron_idx),
          max_tau_idx(max_tau_idx),
          max_photon_idx(max_photon_idx),
          max_bjet_idx(max_bjet_idx),
          max_jet_idx(max_jet_idx),
          max_met_idx(max_met_idx),
          num_muons(max_muon_idx + 1),
          num_electrons(max_electron_idx + 1),
          num_taus(max_tau_idx + 1),
          num_photons(max_photon_idx + 1),
          num_bjets(max_bjet_idx + 1),
          num_jets(max_jet_idx + 1),
          num_met(max_met_idx + 1),
          has_exclusive(has_exclusive),
          has_jet_inclusive(has_jet_inclusive),
          has_met(max_met_idx >= 0),
          sum_pt(0),
          met(0),
          four_vec({})
    {
    }

    auto to_string() const -> std::string
    {
        return fmt::format(
            "Muons: {} - Electrons: {} - Taus: {} - Photons: {} - bJet: {} - "
            "Jet: {} - MET: {} - Excl: {}",
            max_muon_idx,
            max_electron_idx,
            max_tau_idx,
            max_photon_idx,
            max_bjet_idx,
            max_jet_idx,
            max_met_idx,
            has_exclusive);
    }

    auto push(const RVec<Math::PtEtaPhiMVector> &p4, const int max_idx) -> void
    {
        if (max_idx > -1)
        {
            for (std::size_t i = 0; i <= static_cast<std::size_t>(max_idx); i++)
            {
                sum_pt += p4[i].pt();
                met += has_met ? p4[i].pt() : 0.;
                four_vec.e += p4[i].e();
                four_vec.et += p4[i].Et();
                four_vec.px += p4[i].px();
                four_vec.py += p4[i].py();
                four_vec.pz += p4[i].pz();
            }
        }
    }

    auto get_sum_pt() const -> double
    {
        return sum_pt;
    }

    auto get_met() const -> std::optional<double>
    {
        if (has_met)
        {
            return met;
        }
        return std::nullopt;
    }

    auto get_mass() const -> double
    {
        if (has_met)
        {
            return std::sqrt(std::pow(four_vec.et, 2) - std::pow(four_vec.px, 2) - std::pow(four_vec.py, 2));
        }

        return std::sqrt(std::pow(four_vec.e, 2) - std::pow(four_vec.px, 2) - std::pow(four_vec.py, 2) -
                         std::pow(four_vec.pz, 2));
    }

    struct ClassesNames
    {
        const std::optional<std::string> exclusive_class_name;
        const std::optional<std::string> inclusive_class_name;
        const std::optional<std::string> jet_inclusive_class_name;
    };

    auto make_event_class_name(const RVec<unsigned int> &muons_id_score,
                               const RVec<unsigned int> &electrons_id_score,
                               const RVec<unsigned int> &taus_id_score,
                               const RVec<unsigned int> &photons_id_score,
                               const RVec<unsigned int> &bjets_id_score,
                               const RVec<unsigned int> &jets_id_score,
                               const RVec<unsigned int> &met_id_score) -> TempEC::ClassesNames
    {
        /////////////////////////////////
        // CUSTOM
        // if (not((num_muons <= 2 or num_muons == 4) and num_electrons == 0 and num_taus == 0 and num_photons == 0 and
        //         num_bjets == 0 and num_jets == 0))
        // {
        //     return ClassesNames{std::nullopt, std::nullopt, std::nullopt};
        // }
        /////////////////////////////////

        if (num_muons == 0 and num_electrons == 0 and num_photons == 0 and num_taus == 0)
        {
            return ClassesNames{std::nullopt, std::nullopt, std::nullopt};
        }

        if (num_bjets + num_jets > max_allowed_jets_per_class)
        {
            return ClassesNames{std::nullopt, std::nullopt, std::nullopt};
        }

        // auto total_id_score = MUSiCObjects::IdScore::accum_score(muons_id_score, num_muons) +
        //                       MUSiCObjects::IdScore::accum_score(electrons_id_score, num_electrons) +
        //                       MUSiCObjects::IdScore::accum_score(taus_id_score, num_taus) +
        //                       MUSiCObjects::IdScore::accum_score(photons_id_score, num_photons) +
        //                       MUSiCObjects::IdScore::accum_score(bjets_id_score, num_bjets) +
        //                       MUSiCObjects::IdScore::accum_score(jets_id_score, num_jets) +
        //                       MUSiCObjects::IdScore::accum_score(met_id_score, num_met);

        // if (total_id_score.num_medium >= 1 or total_id_score.num_loose >= 3)
        // if (total_id_score.num_medium >= 1)
        // {
        std::string class_name = fmt::format("EC_{}Muon_{}Electron_{}Tau_{}Photon_{}bJet_{}Jet_{}MET",
                                             num_muons,
                                             num_electrons,
                                             num_taus,
                                             num_photons,
                                             num_bjets,
                                             num_jets,
                                             num_met);

        std::optional<std::string> exclusive_class_name = std::nullopt;
        if (has_exclusive)
        {
            exclusive_class_name = class_name;
        }

        std::optional<std::string> inclusive_class_name = fmt::format("{}+X", class_name);

        std::optional<std::string> jet_inclusive_class_name = std::nullopt;
        if (has_jet_inclusive)
        {
            jet_inclusive_class_name = fmt::format("{}+NJet", class_name);
        }

        return ClassesNames{exclusive_class_name, inclusive_class_name, jet_inclusive_class_name};
        // }

        return ClassesNames{std::nullopt, std::nullopt, std::nullopt};
    }

    static auto make_temp_event_classes(const int total_muons,
                                        const int total_electrons,
                                        const int total_taus,
                                        const int total_photons,
                                        const int total_bjets,
                                        const int total_jets,
                                        const int total_met) -> std::vector<TempEC>
    {
        auto temp_event_classes = std::vector<TempEC>{};
        for (int idx_muon = -1; idx_muon < total_muons; idx_muon++)
        {
            for (int idx_electron = -1; idx_electron < total_electrons; idx_electron++)
            {
                for (int idx_tau = -1; idx_tau < total_taus; idx_tau++)
                {
                    for (int idx_photon = -1; idx_photon < total_photons; idx_photon++)
                    {
                        for (int idx_bjet = -1; idx_bjet < total_bjets; idx_bjet++)
                        {
                            for (int idx_jet = -1; idx_jet < total_jets; idx_jet++)
                            {
                                for (int idx_met = -1; idx_met < total_met; idx_met++)
                                {
                                    bool has_exclusive = false;
                                    if (idx_muon + 1 == total_muons and idx_electron + 1 == total_electrons and
                                        idx_tau + 1 == total_taus and idx_photon + 1 == total_photons and
                                        idx_bjet + 1 == total_bjets and idx_jet + 1 == total_jets and
                                        idx_met + 1 == total_met)
                                    {
                                        has_exclusive = true;
                                    }

                                    bool has_jet_inclusive = false;
                                    if (idx_muon + 1 == total_muons and idx_electron + 1 == total_electrons and
                                        idx_tau + 1 == total_taus and idx_photon + 1 == total_photons and
                                        idx_met + 1 == total_met)
                                    {
                                        has_jet_inclusive = true;
                                    }

                                    if (idx_bjet + 1 + idx_jet + 1 < max_allowed_jets_per_class and
                                        (idx_muon > -1 or idx_electron > -1 or idx_tau > -1 or idx_photon > -1))
                                    {
                                        temp_event_classes.emplace_back(idx_muon,
                                                                        idx_electron,
                                                                        idx_tau,
                                                                        idx_photon,
                                                                        idx_bjet,
                                                                        idx_jet,
                                                                        idx_met,
                                                                        has_exclusive,
                                                                        has_jet_inclusive);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return temp_event_classes;
    }
};

auto classification(const std::string process,
                    const std::string year,
                    const bool is_data,
                    const double x_section,
                    const double filter_eff,
                    const double k_factor,
                    const double luminosity,
                    const std::string xs_order,
                    const std::string process_group,
                    const std::string sum_weights_json_filepath,
                    const std::string input_file,
                    const std::string &generator_filter,
                    // [EVENT_CLASS_NAME, [SHIFT, EVENT_CLASS_OBJECT] ]
                    EventClassContainer &event_classes,
                    ValidationContainer &validation_container,
                    std::optional<unsigned long> first_event = std::nullopt,
                    std::optional<long> last_event = std::nullopt,
                    const bool debug = false) -> void;

#endif // CLASSIFICATION
