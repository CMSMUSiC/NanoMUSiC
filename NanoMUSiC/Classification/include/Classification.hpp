#ifndef CLASSIFICATION
#define CLASSIFICATION

// analysis classes

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
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
#include "TChain.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

#include "Configs.hpp"
#include "JetCorrector.hpp"
#include "MUSiCTools.hpp"
#include "NanoAODGenInfo.hpp"
#include "TOMLConfig.hpp"

#include "PDFAlphaSWeights.hpp"

#include "argh.h"
#include "emoji.hpp"
#include "fmt/format.h"
#include "processed_data_events.hpp"

#include "ObjectFactories/make_electrons.hpp"
#include "ObjectFactories/make_jets.hpp"
#include "ObjectFactories/make_met.hpp"
#include "ObjectFactories/make_muons.hpp"
#include "ObjectFactories/make_photons.hpp"
#include "ObjectFactories/make_taus.hpp"

#include "CorrectionLibUtils.hpp"
#include "TriggerMatch.hpp"

#include "Shifts.hpp"

#include "NanoEventClass.hpp"
#include "EventClassFactory.hpp"

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

    fmt::print("WARNING: Could not read branch: {}\n", leaf);
    return std::nullopt;
}

template <typename T>
auto make_array_reader(TTreeReader &tree_reader, const std::string &leaf) -> OptArrayReader_t<T>
{
    if (tree_reader.GetTree()->GetLeaf(leaf.c_str()) != nullptr)
    {
        return std::make_optional<TTreeReaderArray<T>>(tree_reader, leaf.c_str());
    }

    fmt::print("WARNING: Could not read branch: {}\n", leaf);
    return std::nullopt;
}

// helper macros
#define ADD_VALUE_READER(VAR, TYPE) auto VAR = make_value_reader<TYPE>(tree_reader, #VAR)
#define ADD_ARRAY_READER(VAR, TYPE) auto VAR = make_array_reader<TYPE>(tree_reader, #VAR)

#define INITIALIZE_ANALYSIS(TYPE, ANALYSIS, COUNT_MAP)                                                                 \
    ANALYSIS.insert({shift,                                                                                            \
                     TYPE(#ANALYSIS,                                                                                   \
                          get_output_file_path(                                                                        \
                              #ANALYSIS, output_path, process, year, process_group, xs_order, is_data, buffer_index),  \
                          COUNT_MAP,                                                                                   \
                          false,                                                                                       \
                          shift,                                                                                       \
                          process,                                                                                     \
                          year,                                                                                        \
                          process_group,                                                                               \
                          xs_order)})

template <typename T>
auto unwrap(std::optional<TTreeReaderValue<T>> &value) -> T
{
    if (value)
    {
        auto _this_value = **value;
        if (std::isnan(_this_value) or std::isinf(_this_value))
        {
            fmt::print("ERROR: NaN or INF value was unwraped.\n");
            std::exit(EXIT_FAILURE);
        }
        return _this_value;
    }
    return T();
}

template <typename T, typename Q>
auto unwrap_or(std::optional<TTreeReaderValue<T>> &value, Q &&default_value = Q()) -> T
{
    static_assert(std::is_arithmetic<T>::value, "The default type must be numeric.");

    if (value)
    {
        auto _this_value = **value;
        if (std::isnan(_this_value) or std::isinf(_this_value))
        {
            fmt::print("ERROR: NaN or INF value was unwraped.\n");
            std::exit(EXIT_FAILURE);
        }
        return _this_value;
    }
    return static_cast<T>(default_value);
}

template <typename T>
auto unwrap(std::optional<TTreeReaderArray<T>> &array) -> RVec<T>
{
    if (array)
    {
        auto _this_array = RVec<T>(static_cast<T *>((*array).GetAddress()), (*array).GetSize());
        for (const auto &_this_value : _this_array)
        {
            if (std::isnan(_this_value) or std::isinf(_this_value))
            {
                fmt::print("ERROR: NaN or INF array was unwraped.\n");
                std::exit(EXIT_FAILURE);
            }
        }
        return _this_array;
    }
    return RVec<T>();
}

template <typename T, typename Q, typename R>
auto unwrap_or(std::optional<TTreeReaderArray<T>> &array, Q &&default_value, R &&default_size) -> T
{
    if (array)
    {
        auto _this_array = RVec<T>(static_cast<T *>((*array).GetAddress()), (*array).GetSize());
        for (const auto &_this_value : _this_array)
        {
            if (std::isnan(_this_value) or std::isinf(_this_value))
            {
                fmt::print("ERROR: NaN or INF array was unwraped.\n");
                std::exit(EXIT_FAILURE);
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
inline auto trigger_filter(const std::string &process, //
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
                           bool pass_photon_trigger) -> std::optional<std::unordered_map<std::string, bool>>
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
                    {"pass_photon_trigger", pass_photon_trigger}                      //
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
                    {"pass_photon_trigger", pass_photon_trigger}                      //
                };
            }

            return trigger_filter_res;
        }

        // Electron/Photon/EGamma dataset
        if (year != Year::Run2018)
        {
            if (starts_with(process, "SingleElectron"))
            {
                if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger) //
                    and not(pass_double_muon_trigger)                          //
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
                        {"pass_photon_trigger", pass_photon_trigger}                      //
                    };
                }

                return trigger_filter_res;
            }

            if (starts_with(process, "DoubleEG"))
            {
                if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger)             //
                    and not(pass_double_muon_trigger)                                      //
                    and not(pass_low_pt_electron_trigger or pass_high_pt_electron_trigger) //
                    and pass_double_electron_trigger)
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
                        {"pass_photon_trigger", pass_photon_trigger}                      //
                    };
                }

                return trigger_filter_res;
            }

            if (starts_with(process, "SinglePhoton"))
            {
                if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger)             //
                    and not(pass_double_muon_trigger)                                      //
                    and not(pass_low_pt_electron_trigger or pass_high_pt_electron_trigger) //
                    and not(pass_double_electron_trigger)                                  //
                    and pass_photon_trigger)
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
                        {"pass_photon_trigger", pass_photon_trigger}                      //
                    };
                }

                return trigger_filter_res;
            }
        }

        if (year == Year::Run2018)
        {
            if (starts_with(process, "EGamma"))
            {
                if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger) //
                    and not(pass_double_muon_trigger)                          //
                    and (pass_low_pt_electron_trigger or pass_high_pt_electron_trigger or
                         pass_double_electron_trigger or pass_photon_trigger))
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
                        {"pass_photon_trigger", pass_photon_trigger}                      //
                    };
                }

                return trigger_filter_res;
            }
        }

        if (starts_with(process, "Tau"))
        {
            if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger)             //
                and not(pass_double_muon_trigger)                                      //
                and not(pass_low_pt_electron_trigger or pass_high_pt_electron_trigger) //
                and not(pass_double_electron_trigger)                                  //
                and not(pass_photon_trigger)                                           //
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
                    {"pass_photon_trigger", pass_photon_trigger}                      //
                };
            }

            return trigger_filter_res;
        }

        throw std::runtime_error(
            fmt::format("ERROR: Could not check trigger filter for Data file. The requested process ({}) does not "
                        "match any dataset pattern.",
                        process));
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
            {"pass_photon_trigger", pass_photon_trigger}                      //
        };
    }

    return trigger_filter_res;
};

template <typename F>
inline auto loop_over_objects(F f,
                              std::size_t muons_size,
                              std::size_t electrons_size,
                              std::size_t taus_size,
                              std::size_t photons_size,
                              std::size_t bjets_size,
                              std::size_t jets_size,
                              std::size_t met_size) -> void
{
    for (std::size_t idx_muon = 0; idx_muon <= muons_size; idx_muon++)
    {
        for (std::size_t idx_electron = 0; idx_electron <= electrons_size; idx_electron++)
        {
            for (std::size_t idx_tau = 0; idx_tau <= taus_size; idx_tau++)
            {
                for (std::size_t idx_photon = 0; idx_photon <= photons_size; idx_photon++)
                {
                    for (std::size_t idx_bjet = 0; idx_bjet <= bjets_size; idx_bjet++)
                    {
                        for (std::size_t idx_jet = 0; idx_jet <= jets_size; idx_jet++)
                        {
                            for (std::size_t idx_met = 0; idx_met <= met_size; idx_met++)
                            {
                                f(idx_muon, idx_electron, idx_tau, idx_photon, idx_bjet, idx_jet, idx_met);
                            }
                        }
                    }
                }
            }
        }
    }
}

#endif // CLASSIFICATION
