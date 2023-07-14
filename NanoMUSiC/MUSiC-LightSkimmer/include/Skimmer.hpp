#ifndef SKIMMER_HPP
#define SKIMMER_HPP

#include <algorithm>
#include <any>
#include <bitset>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/time.h>
#include <thread>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/VectorUtil.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "RtypesCore.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TLegend.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include <ROOT/RSnapshotOptions.hxx>

#include "color.hpp"
#include "emoji.hpp"

// Comand line tools
// https://github.com/adishavit/argh
#include "argh.h"

#include <fmt/core.h>

// Configurarion and filter
#include "MUSiCTools.hpp"

// Filters (lumi, gen phase-space, ...)
#include "RunLumiFilter.hpp"

// MUSiC
#include "Enumerate.hpp"
#include "GeneratorFilters.hpp"
#include "NanoObjects.hpp"
#include "Outputs.hpp"
#include "TOMLConfig.hpp"
#include "TaskConfiguration.hpp"

// debug helpers
#include "debug.hpp"

using namespace std::chrono_literals;
// using namespace ranges;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

template <typename T>
auto make_value_reader(TTreeReader &tree_reader, const std::string &leaf) -> std::optional<TTreeReaderValue<T>>
{
    if (tree_reader.GetTree()->GetLeaf(leaf.c_str()) != nullptr)
    {
        return std::make_optional<TTreeReaderValue<T>>(tree_reader, leaf.c_str());
    }
    return std::nullopt;
}

template <typename T>
auto make_array_reader(TTreeReader &tree_reader, const std::string &leaf) -> std::optional<TTreeReaderArray<T>>
{
    if (tree_reader.GetTree()->GetLeaf(leaf.c_str()) != nullptr)
    {
        return std::make_optional<TTreeReaderArray<T>>(tree_reader, leaf.c_str());
    }
    return std::nullopt;
}

// helper macros
#define ADD_VALUE_READER(VAR, TYPE) auto VAR = make_value_reader<TYPE>(tree_reader, #VAR)
#define ADD_ARRAY_READER(VAR, TYPE) auto VAR = make_array_reader<TYPE>(tree_reader, #VAR)

template <typename T>
auto unwrap(std::optional<TTreeReaderValue<T>> &value) -> T
{
    if (value)
    {
        return **value;
    }
    return T();
}

template <typename T, typename Q>
auto unwrap(std::optional<TTreeReaderValue<T>> &value, Q &&default_value = Q()) -> T
{
    static_assert(std::is_arithmetic<T>::value, "The default type must be numeric.");

    if (value)
    {
        return **value;
    }
    return static_cast<T>(default_value);
}

template <typename T>
auto unwrap(std::optional<TTreeReaderArray<T>> &array) -> RVec<T>
{
    if (array)
    {
        return RVec<T>(static_cast<T *>((*array).GetAddress()), (*array).GetSize());
    }
    return RVec<T>();
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

// (async) TFile download
using OptionalFuture_t = std::optional<std::future<std::unique_ptr<TFile>>>;
inline auto file_loader(const std::string &file_path,
                        const bool cacheread,
                        const std::string &cache_dir,
                        const bool verbose_load) -> std::unique_ptr<TFile>
{
    std::cout << "Loading file [ " << file_path << " ]" << std::endl;

    if (cacheread)
    {
        const auto hash = std::to_string(std::hash<std::string>{}(file_path));
        const auto dest = cache_dir + "/" + hash + ".root";

        std::string silent_load = "--silent";
        if (verbose_load)
        {
            silent_load = "";
        }
        const std::string command_str = "xrdcp -f " + silent_load + " " + file_path + " " + dest;
        int download_return_code = std::system(command_str.c_str());

        if (download_return_code == 0)
        {
            std::unique_ptr<TFile> input_root_file(TFile::Open(dest.c_str()));
            return input_root_file;
        }

        return std::unique_ptr<TFile>{};
    }
    std::unique_ptr<TFile> input_root_file(TFile::Open(file_path.c_str()));
    return input_root_file;
}

inline auto download_file(const std::string &requested_file, const std::string &destination_file_path) -> bool
{
    const std::string command_str = fmt::format("xrdcp {} {}", requested_file, destination_file_path);
    int download_return_code = std::system(command_str.c_str());

    if (download_return_code == 0)
    {
        return true;
    }

    return false;
}

inline auto load_from_local_cache(const std::string &requested_file) -> std::string
{
    const std::string file_hash = std::to_string(std::hash<std::string>{}(requested_file));
    const std::string username = getlogin();
    const std::string cache_dir = fmt::format("/user/scratch/{}/cache_dir", username);
    const std::string file_path = fmt::format("{}/{}.root", cache_dir, file_hash);

    // check if file exists
    if (std::filesystem::exists(file_path))
    {
        return file_path;
    }

    // if file does not exists, will download it
    if (download_file(requested_file, file_path))
    {
        return load_from_local_cache(requested_file);
    }

    fmt::print("ERROR: Could not find requested file ({}).\n", requested_file);
    exit(-1);
}

inline auto getCpuTime() -> double
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0);
}

constexpr auto is_tenth(int &event_counter) -> bool
{
    return (event_counter < 10 || (event_counter < 100 && event_counter % 10 == 0) ||
            (event_counter < 1000 && event_counter % 100 == 0) ||
            (event_counter < 10000 && event_counter % 1000 == 0) ||
            (event_counter >= 100000 && event_counter % 10000 == 0));
}

template <typename T>
auto get_and_check_future(std::future<T> &_ftr) -> T
{
    try
    {
        return _ftr.get();
    }
    catch (const std::exception &e)
    {
        std::cout << "[ERROR] Caught exception when trying to collect the object filter (async task)." << std::endl;
        std::cout << e.what() << std::endl;
        exit(1);
    }
}

inline auto prepare_output_buffer(const TaskConfiguration &configuration) -> void
{
    const std::string startDir = getcwd(nullptr, 0);

    // (Re)create output_directory dir and cd into it.
    system(("rm -rf " + configuration.output_directory).c_str());
    system(("mkdir -p " + configuration.output_directory).c_str());
    system(("cd " + configuration.output_directory).c_str());
    chdir(configuration.output_directory.c_str());

    if (!configuration.golden_json_file.empty())
        system(("cp " + configuration.golden_json_file + " . ").c_str());

    if (configuration.is_data)
        system("mkdir -p Event-lists");
}

inline auto print_report(const double &dTime1,
                         const unsigned long &event_counter,
                         TH1F &cutflow_histo,
                         bool is_final = false) -> void
{
    double dTime2 = getCpuTime();
    std::string final_str = is_final ? "Final " : "";
    std::cout << "\n[ " << final_str << "Process Report ] Analyzed " << event_counter << " events";
    std::cout << ", elapsed CPU time: " << dTime2 - dTime1 << "sec (" << double(event_counter) / (dTime2 - dTime1)
              << " evts per sec)" << std::endl;

    // print cutflow
    auto cutflow = cutflow_histo;
    fmt::print("\n=====================================\n");
    fmt::print("               Cutflow:              \n");
    fmt::print("-------------------------------------\n");
    fmt::print(" {:25}: {: >6.2f} %\n", "GeneratorFilter", cutflow.GetBinContent(2) / cutflow.GetBinContent(1) * 100);
    fmt::print(". . . . . . . . . . . . . . . . . . .\n");
    for (auto &&cut : IndexHelpers::make_index(2, Outputs::kTotalCuts - 1))
    {
        fmt::print(" {:25}: {: >6.2f} %\n",
                   Outputs::Cuts[cut],
                   cutflow.GetBinContent(cut + 1) / cutflow.GetBinContent(3) * 100);
    }
    fmt::print("=====================================\n");

    std::cout << " " << std::endl;
}

inline auto get_output_branches(const TaskConfiguration &configuration) -> std::vector<std::string>
{
    std::vector<std::string> output_branches = {
        "pass_low_pt_muon_trigger",
        "pass_high_pt_muon_trigger",
        "pass_low_pt_electron_trigger",
        "pass_high_pt_electron_trigger",
        "pass_jet_ht_trigger",
        "pass_jet_pt_trigger",
        "run",
        "luminosityBlock",
        "event",
        "fixedGridRhoFastjetAll",
        "Muon_pt",
        "Muon_eta",
        "Muon_phi",
        "Muon_tightId",
        "Muon_highPtId",
        "Muon_pfRelIso04_all",
        "Muon_tkRelIso",
        "Muon_highPurity",
        "Muon_tunepRelPt",
        "Electron_pt",
        "Electron_eta",
        "Electron_phi",
        "Electron_cutBased",
        "Electron_cutBased_HEEP",
        "Electron_deltaEtaSC",
        "Electron_dEscaleDown",
        "Electron_dEscaleUp",
        "Electron_dEsigmaDown",
        "Electron_dEsigmaUp",
        "Electron_eCorr",
        "Electron_scEtOverPt",
        "Photon_pt",
        "Photon_eta",
        "Photon_phi",
        "Photon_cutBased",
        "Photon_pixelSeed",
        "Photon_isScEtaEB",
        "Photon_isScEtaEE",
        "Photon_dEscaleUp",
        "Photon_dEscaleDown",
        "Photon_dEsigmaUp",
        "Photon_dEsigmaDown",
        "Tau_decayMode",
        "Tau_dz",
        "Tau_idDeepTau2017v2p1VSe",
        "Tau_idDeepTau2017v2p1VSjet",
        "Tau_idDeepTau2017v2p1VSmu",
        "Tau_leadTkDeltaEta",
        "Tau_leadTkDeltaPhi",
        "Tau_leadTkPtOverTauPt",
        "Tau_pt",
        "Tau_eta",
        "Tau_phi",
        "Tau_mass",
        "Jet_pt",
        "Jet_eta",
        "Jet_phi",
        "Jet_mass",
        "Jet_jetId",
        "Jet_btagDeepFlavB",
        "Jet_rawFactor",
        "Jet_area",
        "MET_pt",
        "MET_phi",
        "MET_MetUnclustEnUpDeltaX",
        "MET_MetUnclustEnUpDeltaY" //
    };
    const std::vector<std::string> output_branches_mc_only = {
        "mc_weight",
        "Jet_hadronFlavour",
        "Jet_genJetIdx",
        "Pileup_nTrueInt",
        "genWeight",
        "L1PreFiringWeight_Nom",
        "L1PreFiringWeight_Up",
        "L1PreFiringWeight_Dn",
        "Generator_binvar",
        "Generator_scalePDF",
        "Generator_weight",
        "Generator_x1",
        "Generator_x2",
        "Generator_xpdf1",
        "Generator_xpdf2",
        "Generator_id1",
        "Generator_id2",
        // "GenPart_eta",
        // "GenPart_mass",
        // "GenPart_phi",
        // "GenPart_pt",
        // "GenPart_genPartIdxMother",
        // "GenPart_pdgId",
        // "GenPart_status",
        // "GenPart_statusFlags",
        "LHEPdfWeight",
        "LHEScaleWeight",
        "LHEWeight_originalXWGTUP",
        "LHE_HT",
        "LHE_HTIncoming",
        // "LHEPart_pt",
        // "LHEPart_eta",
        // "LHEPart_phi",
        // "LHEPart_mass",
        // "LHEPart_incomingpz",
        // "LHEPart_pdgId",
        // "LHEPart_status",
        "GenJet_pt",
        "GenJet_eta",
        "GenJet_phi",
        "GenJet_mass",
        "Muon_genPartIdx",
        "Electron_genPartIdx",
        "Photon_genPartIdx",
        "Tau_genPartFlav",
        "Tau_genPartIdx" //
    };

    if (!configuration.is_data)
    {
        output_branches.insert(output_branches.end(), output_branches_mc_only.cbegin(), output_branches_mc_only.cend());
    }
    return output_branches;
}

inline auto get_hash(const std::string &str) -> std::string
{
    std::hash<std::string> hasher;

    return std::to_string(hasher(str));
}

#endif /*SKIMMER_HPP*/
