#ifndef MUSIC_NANOMUSIC
#define MUSIC_NANOMUSIC

#include <algorithm>
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
#include "TFile.h"
#include "TH1.h"
#include "TObjString.h"
#include "TTree.h"

// toml++ v3.1.0
// https://github.com/marzer/tomlplusplus
#include "color.hpp"
#include "emoji.hpp"

// https://github.com/okdshin/PicoSHA2
#include "picosha2.hpp"
#include "toml.hpp"

// Comand line tools
// https://github.com/adishavit/argh
#include "argh.h"

// http:://github.com/bshoshany/thread-pool
#include "BS_thread_pool.hpp"

// Configurarion and filter
// #include "MConfig.hpp"
#include "MUSiCTools.hpp"
#include "TOMLConfig.hpp"

// Filters (lumi, gen phase-space, ...)
#include "RunLumiFilter.hpp"

// ROOT Stuff
#include "Math/Vector4D.h"
#include "TFile.h"
#include "TH1.h"
#include "TObjString.h"
#include "TTree.h"

// Corrections and weighters
#include "CorrectionSets.hpp"
// #include "PDFAlphaSWeights.hpp"

// MUSiC data models
#include "Configs.hpp"
#include "MUSiCEvent.hpp"
#include "NanoAODReader.hpp"
#include "NanoObjects.hpp"
#include "ObjectCorrections.hpp"
#include "Trigger.hpp"

using namespace ranges;
using namespace ROOT::Math;
using OptionalFuture_t = std::optional<std::future<std::unique_ptr<TFile>>>;

std::string_view get_data_stream(const std::string_view &dataset)
{
    auto s = std::string(dataset);
    std::string delimiter = "/";
    std::string token = s.substr(1, s.substr(1).find(delimiter));
    return token.c_str();
}

// (async) TFile download
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

std::string get_hash256(const std::string &input_string)
{
    return picosha2::hash256_hex_string(input_string);
}

void save_class_storage(const std::set<unsigned long> &classes, std::string output_file_name)
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

    output_file_name = std::regex_replace(std::string(output_file_name), std::regex("root"),
                                          "classes"); // "this is an example string." (1)
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

#endif /*MUSIC_NANOMUSIC*/
