
#include <algorithm>
#include <csignal>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <numeric>
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

// #include "Pxl/Pxl/interface/pxl/core.hh"
// #include "Pxl/Pxl/interface/pxl/hep.hh"

#include "MConfig.hh"
// #include "Main/EventAdaptor.hh"
// #include "Main/EventSelector.hh"
// #include "Main/JetTypeWriter.hh"
// #include "Main/ParticleMatcher.hh"
// #include "Main/ReWeighter.hh"
// #include "Main/RunLumiRanges.hh"
// #include "Main/SkipEvents.hh"
// #include "Main/Systematics.hh"
#include "Main/TOMLConfig.hh"
#include "Tools/Tools.hh"

#include "event_class_hash.hh"

// ROOT Stuff
#include "TFile.h"
#include "TH1.h"
#include "TObjString.h"
#include "TTree.h"

// #include "EventClassFactory/EventClassFactory.hh"

#include "Main/NanoAODReader.hh"
// #include "nano2pxl_utils.hh"

unsigned int getIntYear(std::string year)
{
    if (year == "2016APV" || year == "2016")
    {
        return 2016;
    }
    if (year == "2017")
    {
        return 2017;
    }
    if (year == "2018")
    {
        return 2018;
    }
    return 1; // dummy
}

enum class Cuts
{
    TotalUnweighted,
    TotalWeighted,
    Trigger,
    ObjectSelection
};

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

std::string get_hash256(const std::string &input_string)
{
    return picosha2::hash256_hex_string(input_string);
}

struct EventContent
{
    unsigned int run = 0;
    unsigned int lumi_section = 0;
    unsigned long event_number = 0;
    unsigned int event_class_hash = 0;
    float sum_pt = -99.0;
    float mass = -99.0;
    float met = -99.0;
    float weight = 1.0;
};

void register_branches(EventContent &event_content, std::unique_ptr<TTree> &output_tree)
{
    output_tree->Branch("run", &(event_content.run));
    output_tree->Branch("lumi_section", &(event_content.lumi_section));
    output_tree->Branch("event_number", &(event_content.event_number));
    output_tree->Branch("event_class_hash", &(event_content.event_class_hash));
    output_tree->Branch("sum_pt", &(event_content.sum_pt));
    output_tree->Branch("mass", &(event_content.mass));
    output_tree->Branch("met", &(event_content.met));
    output_tree->Branch("weight", &(event_content.weight));
}
