#ifndef MUSIC_NANOMUSIC
#define MUSIC_NANOMUSIC

#include <algorithm>
#include <any>
#include <bitset>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <math.h>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include <typeinfo>
#include <unordered_set>

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/VectorUtil.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "TFile.h"
#include "TH1.h"
#include "TObjString.h"
#include "TTree.h"

// toml++ v3.1.0
// https://github.com/marzer/tomlplusplus
#include "toml.hpp"

#include "color.hpp"
#include "emoji.hpp"

// Comand line tools
// https://github.com/adishavit/argh
#include "argh.h"

#include <fmt/core.h>
// using fmt::print;

// Configurarion and filter
#include "MUSiCTools.hpp"
#include "TOMLConfig.hpp"

// Filters (lumi, gen phase-space, ...)
#include "RunLumiFilter.hpp"

// Corrections and weighters
#include "CorrectionSets.hpp"
// #include "PDFAlphaSWeights.hpp"

// MUSiC
#include "Configs.hpp"
#include "NanoObjects.hpp"
#include "Outputs.hpp"
// #include "ObjectCorrections.hpp"
#include "Enumerate.hpp"
#include "EventData.hpp"
#include "Trigger.hpp"

using namespace std::chrono_literals;
using namespace ranges;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

void PrintProcessInfo()
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

std::string_view get_data_stream(const std::string_view &dataset)
{
    auto s = std::string(dataset);
    std::string delimiter = "/";
    std::string token = s.substr(1, s.substr(1).find(delimiter));
    return token.c_str();
}

// (async) TFile download
using OptionalFuture_t = std::optional<std::future<std::unique_ptr<TFile>>>;
std::unique_ptr<TFile> file_loader(const std::string &file_path, const bool cacheread, const std::string &cache_dir,
                                   const bool verbose_load)
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

double getCpuTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0);
}

constexpr bool is_tenth(int &event_counter)
{
    return (event_counter < 10 || (event_counter < 100 && event_counter % 10 == 0) ||
            (event_counter < 1000 && event_counter % 100 == 0) || (event_counter < 10000 && event_counter % 1000 == 0) ||
            (event_counter >= 100000 && event_counter % 10000 == 0));
}

void save_class_storage(const std::unordered_set<unsigned long> &classes, std::string output_file_name)
{
    // expected number of elements: (7*2 + 1) * classes.size()
    // 7 types of objects
    // 2 digits per object
    // classes.size(): number of classes
    std::string str_class_storage =
        std::accumulate(classes.begin(), classes.end(), std::string(""),
                        [](std::string a, unsigned long b) { return std::move(a) + ',' + std::to_string(b); });

    // this will remove the leading comma in the begining of the string
    str_class_storage.erase(0, 1);

    output_file_name = output_file_name;
    std::ofstream out(output_file_name);
    out << str_class_storage;
    out.close();
}

template <typename T>
T get_and_check_future(std::future<T> &_ftr)
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
void prepare_output_buffer(const TaskConfiguration &configuration)
{
    const std::string startDir = getcwd(NULL, 0);

    // (Re)create output_directory dir and cd into it.
    system(("rm -rf " + configuration.output_directory).c_str());
    system(("mkdir -p " + configuration.output_directory).c_str());
    system(("cd " + configuration.output_directory).c_str());
    chdir(configuration.output_directory.c_str());

    if (!configuration.golden_json_file.empty())
        system(("cp " + configuration.golden_json_file + " . ").c_str());

    if (configuration.is_data)
        system("mkdir -p Event-lists");

    // save other configs with output
    system(("cp " + configuration.x_section_file + " . ").c_str());

    // copy rootlogon.C
    system(("cp " + MUSiCTools::parse_and_expand_music_base("$MUSIC_BASE/rootlogon.C") + " . ").c_str());
}

void print_final_report(const double &dTime1, const unsigned long &event_counter)
{
    double dTime2 = getCpuTime();
    std::cout << "[ Final Performance Report ] Analyzed " << event_counter << " events";
    std::cout << ", elapsed CPU time: " << dTime2 - dTime1 << "sec (" << double(event_counter) / (dTime2 - dTime1)
              << " evts per sec)" << std::endl;

    if (event_counter == 0)
    {
        std::cout << "Error: No event was analyzed!" << std::endl;
        throw std::runtime_error("No event was analyzed!");
    }
    std::cout << " " << std::endl;
}

#endif /*MUSIC_NANOMUSIC*/
