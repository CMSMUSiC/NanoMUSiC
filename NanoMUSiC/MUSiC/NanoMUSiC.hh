
#include <algorithm>
#include <bitset>
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
#include <sstream>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <unordered_set>

// toml++ v3.1.0
// https://github.com/marzer/tomlplusplus
#include "Tools/cpp_helper_libs/color.hh"
#include "Tools/cpp_helper_libs/emoji.hh"
// https://github.com/okdshin/PicoSHA2
#include "Tools/cpp_helper_libs/picosha2.hh"
#include "Tools/cpp_helper_libs/toml.h"

// Comand line Tools
// https://github.com/adishavit/argh
#include "Tools/cpp_helper_libs/argh.h"

#include "RunLumiFilter.hh"

#include "MConfig.hh"
#include "Main/TOMLConfig.hh"
#include "Tools/Tools.hh"

#include "NanoObjects.hh"
#include "event_class_hash.hh"

// ROOT Stuff
#include "Math/Vector4D.h"
#include "TFile.h"
#include "TH1.h"
#include "TObjString.h"
#include "TTree.h"

#include "CorrectionSets.hh"
#include "PDFAlphaSWeights.hh"

#include "Main/NanoAODReader.hh"

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
            (event_counter >= 10000 && event_counter % 10000 == 0));
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

struct ObjectCounter
{
    unsigned int n_muons = 0;
    unsigned int n_electrons = 0;
    unsigned int n_photons = 0;
    unsigned int n_taus = 0;
    unsigned int n_bjets = 0;
    unsigned int n_jets = 0;
    unsigned int n_met = 0;
};

struct EventContent
{
    unsigned int run = 0;
    unsigned int lumi_section = 0;
    unsigned long event_number = 0;
    unsigned int trigger_bits = 0;
    unsigned int event_class_hash = 0;
    float sum_pt = -99.0;
    float mass = -99.0;
    float met = -99.0;
    float weight = 1.0;
    float weight_pdf_up = 1.0;
    float weight_pdf_down = 1.0;
    float weight_alphas_up = 1.0;
    float weight_alphas_down = 1.0;
    float weight_pileup_up = 1.0;
    float weight_pileup_down = 1.0;
    float weight_lumi_up = 1.0;
    float weight_lumi_down = 1.0;

    void register_branches(std::unique_ptr<TTree> &output_tree)
    {
        output_tree->Branch("run", &(this->run));
        output_tree->Branch("lumi_section", &(this->lumi_section));
        output_tree->Branch("event_number", &(this->event_number));
        output_tree->Branch("trigger_bits", &(this->trigger_bits));
        output_tree->Branch("event_class_hash", &(this->event_class_hash));
        output_tree->Branch("sum_pt", &(this->sum_pt));
        output_tree->Branch("mass", &(this->mass));
        output_tree->Branch("met", &(this->met));
        output_tree->Branch("weight", &(this->weight));
        output_tree->Branch("weight", &(this->weight));
        output_tree->Branch("weight_pdf_up", &(this->weight_pdf_up));
        output_tree->Branch("weight_pdf_down", &(this->weight_pdf_down));
        output_tree->Branch("weight_alphas_up", &(this->weight_alphas_up));
        output_tree->Branch("weight_alphas_down", &(this->weight_alphas_down));
        output_tree->Branch("weight_pileup_up", &(this->weight_pileup_up));
        output_tree->Branch("weight_pileup_down", &(this->weight_pileup_down));
        output_tree->Branch("weight_lumi_up", &(this->weight_lumi_up));
        output_tree->Branch("weight_lumi_down", &(this->weight_lumi_down));
    }
};
