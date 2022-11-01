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
#include <math.h>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <thread>
#include <time.h>
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

// Comand line Tools
// https://github.com/adishavit/argh
#include "argh.h"

// Configurarion and filter
#include "MConfig.hpp"
#include "TOMLConfig.hpp"
#include "Tools.hpp"

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
#include "MUSiCEvent.hpp"
#include "NanoAODReader.hpp"
#include "NanoObjects.hpp"
#include "ObjectCorrections.hpp"

using namespace ranges;
using namespace ROOT::Math;
using OptionalFuture_t = std::optional<std::future<std::unique_ptr<TFile>>>;

// (async) TFile getter
std::unique_ptr<TFile> get_TFile(const std::string &file_path, const bool cacheread, const std::string &cache_dir)
{
    std::cout << "Loading file [ " << file_path << " ]" << std::endl;

    if (cacheread)
    {
        const auto hash = std::to_string(std::hash<std::string>{}(file_path));
        const auto dest = cache_dir + "/" + hash + ".root";
        if (TFile::Cp(file_path.c_str(), dest.c_str(), false))
        {
            std::unique_ptr<TFile> input_root_file(TFile::Open(dest.c_str()));
            return input_root_file;
        }

        return std::unique_ptr<TFile>{};
    }
    std::unique_ptr<TFile> input_root_file(TFile::Open(file_path.c_str()));
    return input_root_file;
}

// constexpr unsigned int getIntYear(const std::string_view &year)
// {
//     if (year == "2016APV" || year == "2016")
//     {
//         return 2016;
//     }
//     if (year == "2017")
//     {
//         return 2017;
//     }
//     if (year == "2018")
//     {
//         return 2018;
//     }
//     return 1; // dummy
// }

// This function will read a NanoAOD event from a tree and return a pxl::Event
// How to access data:
// nano_reader->getVal<UInt_t>("nMuon")
// nano_reader->getVec<Float_t>("Muon_pt")
// nano_reader->getVal<Bool_t>("HLT_Mu18_Mu9")

double getCpuTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0);
}

constexpr bool is_tenth(int &event_counter)
{
    return (event_counter < 10 || (event_counter < 100 && event_counter % 10 == 0) ||
            (event_counter < 1000 && event_counter % 100 == 0) ||
            (event_counter < 10000 && event_counter % 1000 == 0) ||
            (event_counter >= 100000 && event_counter % 10000 == 0));
}

std::string get_hash256(const std::string &input_string)
{
    return picosha2::hash256_hex_string(input_string);
}

enum HLTPaths
{
    SingleMuon,
    SingleElectron,
    DoubleMuon,
    DoubleElectron,
    Tau,
    BJet,
    MET,
    Photon,
};

struct TriggerBits
{
    // will have size = SIZE
    constexpr static size_t SIZE = sizeof(unsigned int) * 8;
    std::bitset<SIZE> trigger_bits;

    TriggerBits &set(unsigned int path, bool value)
    {
        trigger_bits.set(path, value);
        return *this;
    }

    auto as_ulong() const
    {
        return trigger_bits.to_ulong();
    }

    auto as_ulonglong() const
    {
        return trigger_bits.to_ullong();
    }

    auto as_uint() const
    {
        return static_cast<unsigned int>(trigger_bits.to_ullong());
    }

    std::string_view as_string() const
    {
        return std::string_view(std::to_string(this->as_ulong()));
    }
};

std::string make_class_storage(const std::set<unsigned long> &classes)
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

    return str_class_storage;
}

#endif /*MUSIC_NANOMUSIC*/