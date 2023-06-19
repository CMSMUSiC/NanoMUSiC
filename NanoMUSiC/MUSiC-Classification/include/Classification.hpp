#ifndef CLASSIFICATION_HPP
#define CLASSIFICATION_HPP

#include <optional>
#include <stdexcept>
#include <sys/time.h>

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

// correctionlib
// More info: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagCalibration
// More info: https://github.com/cms-nanoAOD/correctionlib
// More info: https://cms-nanoaod.github.io/correctionlib/index.html
// Instructions:
// https://indico.cern.ch/event/1096988/contributions/4615134/attachments/2346047/4000529/Nov21_btaggingSFjsons.pdf
#include "correction.h"

#include "MUSiCTools.hpp"
#include "TOMLConfig.hpp"

#include "PDFAlphaSWeights.hpp"

#include "argh.h"
#include "emoji.hpp"
#include "fmt/format.h"

#include "CrossSectionOrderErrorMap.hpp"
#include "ObjectFactories/make_electrons.hpp"
#include "ObjectFactories/make_jets.hpp"
#include "ObjectFactories/make_met.hpp"
#include "ObjectFactories/make_muons.hpp"
#include "ObjectFactories/make_photons.hpp"
#include "ParticleMap.hpp"

#include "ClassFactory.hpp"

#include "Event.hpp"

using namespace ROOT;
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

inline auto getCpuTime() -> double
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0);
}

inline auto load_input_files(const std::string &filename) -> std::vector<std::string>
{
    std::vector<std::string> input_files;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error(fmt::format("ERROR: Could not open file: {}", filename));
    }

    std::string line;
    while (std::getline(file, line))
    {
        input_files.push_back(line);
    }
    file.close();

    return input_files;
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

const std::map<std::pair<std::string_view, Year>, std::pair<std::string, std::string>> correction_keys = {
    // PU
    // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
    {{"PU"sv, Year::Run2016APV},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2016preVFP_UL/puWeights.json.gz",
      "Collisions16_UltraLegacy_goldenJSON"}},
    {{"PU"sv, Year::Run2016},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2016postVFP_UL/puWeights.json.gz",
      "Collisions16_UltraLegacy_goldenJSON"}},
    {{"PU"sv, Year::Run2017},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2017_UL/puWeights.json.gz",
      "Collisions17_UltraLegacy_goldenJSON"}},
    {{"PU"sv, Year::Run2018},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2018_UL/puWeights.json.gz",
      "Collisions18_UltraLegacy_goldenJSON"}},

    // Muon Reconstruction SF
    // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
    {{"MuonReco"sv, Year::Run2016APV},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
      "NUM_TrackerMuons_DEN_genTracks"}},
    {{"MuonReco"sv, Year::Run2016},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
      "NUM_TrackerMuons_DEN_genTracks"}},
    {{"MuonReco"sv, Year::Run2017},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
      "NUM_TrackerMuons_DEN_genTracks"}},
    {{"MuonReco"sv, Year::Run2018},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
      "NUM_TrackerMuons_DEN_genTracks"}},

    // Muon ID SF - Low pT
    // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
    {{"MuonIdLowPt"sv, Year::Run2016APV},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
      "NUM_TightID_DEN_TrackerMuons"}},
    {{"MuonIdLowPt"sv, Year::Run2016},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
      "NUM_TightID_DEN_TrackerMuons"}},
    {{"MuonIdLowPt"sv, Year::Run2017},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
      "NUM_TightID_DEN_TrackerMuons"}},
    {{"MuonIdLowPt"sv, Year::Run2018},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
      "NUM_TightID_DEN_TrackerMuons"}},

    // Muon ID SF - High pT
    // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
    {{"MuonIdHighPt"sv, Year::Run2016APV},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
      "NUM_HighPtID_DEN_TrackerMuons"}},
    {{"MuonIdHighPt"sv, Year::Run2016},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
      "NUM_HighPtID_DEN_TrackerMuons"}},
    {{"MuonIdHighPt"sv, Year::Run2017},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
      "NUM_HighPtID_DEN_TrackerMuons"}},
    {{"MuonIdHighPt"sv, Year::Run2018},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
      "NUM_HighPtID_DEN_TrackerMuons"}},

    // Muon Iso SF - Low pT
    // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
    {{"MuonIsoLowPt"sv, Year::Run2016APV},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
      "NUM_TightRelIso_DEN_TightIDandIPCut"}},
    {{"MuonIsoLowPt"sv, Year::Run2016},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
      "NUM_TightRelIso_DEN_TightIDandIPCut"}},
    {{"MuonIsoLowPt"sv, Year::Run2017},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
      "NUM_TightRelIso_DEN_TightIDandIPCut"}},
    {{"MuonIsoLowPt"sv, Year::Run2018},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
      "NUM_TightRelIso_DEN_TightIDandIPCut"}},

    // Muon Iso SF - High pT
    // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
    {{"MuonIsoHighPt"sv, Year::Run2016APV},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
      "NUM_TightRelTkIso_DEN_HighPtIDandIPCut"}},
    {{"MuonIsoHighPt"sv, Year::Run2016},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
      "NUM_TightRelTkIso_DEN_HighPtIDandIPCut"}},
    {{"MuonIsoHighPt"sv, Year::Run2017},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
      "NUM_TightRelTkIso_DEN_HighPtIDandIPCut"}},
    {{"MuonIsoHighPt"sv, Year::Run2018},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
      "NUM_TightRelTkIso_DEN_HighPtIDandIPCut"}},

    // Muon Trigger SF  - Low Pt
    // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
    {{"SingleMuonLowPt"sv, Year::Run2016APV},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
      "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight"}},
    {{"SingleMuonLowPt"sv, Year::Run2016},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
      "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight"}},
    {{"SingleMuonLowPt"sv, Year::Run2017},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
      "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight"}},
    {{"SingleMuonLowPt"sv, Year::Run2018},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
      "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight"}},

    // Muon Trigger SF  - High Pt
    // {{TYPE, YEAR}, {JSON, CORRECTION_KEY}},
    {{"SingleMuonHighPt"sv, Year::Run2016APV},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016preVFP_UL/muon_Z.json.gz",
      "NUM_Mu50_or_TkMu50_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},
    {{"SingleMuonHighPt"sv, Year::Run2016},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2016postVFP_UL/muon_Z.json.gz",
      "NUM_Mu50_or_TkMu50_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},
    {{"SingleMuonHighPt"sv, Year::Run2017},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2017_UL/muon_Z.json.gz",
      "NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},
    {{"SingleMuonHighPt"sv, Year::Run2018},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/MUO/2018_UL/muon_Z.json.gz",
      "NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose"}},

    // MuonLowPt
    // {{TYPE, YEAR}, {INPUT, DUMMY (leave empty)}},
    {{"MuonLowPt"sv, Year::Run2016APV},
     {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016aUL.txt")), ""}},
    {{"MuonLowPt"sv, Year::Run2016},
     {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2016bUL.txt")), ""}},
    {{"MuonLowPt"sv, Year::Run2017},
     {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2017UL.txt")), ""}},
    {{"MuonLowPt"sv, Year::Run2018},
     {MUSiCTools::parse_and_expand_music_base(("$MUSIC_BASE/NanoMUSiC/MUSiC/external/roccor/RoccoR2018UL.txt")), ""}},

    {{"ElectronSF", Year::Run2016APV},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016preVFP_UL/electron.json.gz",
      "UL-Electron-ID-SF"}},
    {{"ElectronSF", Year::Run2016},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016postVFP_UL/electron.json.gz",
      "UL-Electron-ID-SF"}},
    {{"ElectronSF", Year::Run2017},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2017_UL/electron.json.gz",
      "UL-Electron-ID-SF"}},
    {{"ElectronSF", Year::Run2018},
     {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2018_UL/electron.json.gz",
      "UL-Electron-ID-SF"}},
};

#endif // CLASSIFICATION_HPP