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

#include "Configs.hpp"
#include "JetCorrector.hpp"
#include "MUSiCTools.hpp"
#include "TOMLConfig.hpp"

#include "PDFAlphaSWeights.hpp"

#include "argh.h"
#include "emoji.hpp"
#include "fmt/format.h"

#include "ClassFactory.hpp"
#include "CrossSectionOrderErrorMap.hpp"
#include "ParticleMap.hpp"

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

inline auto make_muons(const RVec<float> &Muon_pt,             //
                       const RVec<float> &Muon_eta,            //
                       const RVec<float> &Muon_phi,            //
                       const RVec<bool> &Muon_tightId,         //
                       const RVec<UChar_t> &Muon_highPtId,     //
                       const RVec<float> &Muon_pfRelIso04_all, //
                       const RVec<float> &Muon_tkRelIso,       //
                       const RVec<float> &Muon_tunepRelPt) -> RVec<Math::PtEtaPhiMVector>
{
    auto muons = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Muon_pt.size(); i++)
    {
        bool is_good_low_pt_muon = (Muon_pt.at(i) >= 25.)                //
                                   && (Muon_pt.at(i) < 200.)             //
                                   && (std::fabs(Muon_eta.at(i)) <= 2.4) //
                                   && (Muon_tightId.at(i))               //
                                   && (Muon_pfRelIso04_all.at(i) < 0.15);

        bool is_good_high_pt_muon = (Muon_pt.at(i) >= 200.) //
                                                            // bool is_good_high_pt_muon = (Muon_pt.at(i) >= 60.) //
                                    && (std::fabs(Muon_eta.at(i)) <= 2.4) //
                                    && (Muon_highPtId.at(i) >= 2)         //
                                    && (Muon_tkRelIso.at(i) < 0.10);

        double pt_correction_factor = 1.;
        if (is_good_high_pt_muon)
        {
            //    //     // for some reason, the Relative pT Tune can yield very low corrected pT. Because of this,
            //     //     // they will be caped to 25., in order to not break the JSON SFs bound checks.
            //     //     //
            //     https://cms-nanoaod-integration.web.cern.ch/commonJSONSFs/summaries/MUO_2016postVFP_UL_muon_Z.html
            //     //     // leading_muon.SetPt(std::max(Muon_tunepRelPt[0] * leading_muon.pt(), 25.));
            pt_correction_factor = Muon_tunepRelPt.at(i);
        }

        if (is_good_low_pt_muon or is_good_high_pt_muon)
        {
            muons.push_back(ROOT::Math::PtEtaPhiMVector(
                std::max(Muon_pt[i] * pt_correction_factor, 25.), Muon_eta[i], Muon_phi[i], PDG::Muon::Mass));
            // muons.push_back(ROOT::Math::PtEtaPhiMVector(
            //     Muon_pt[i] * pt_correction_factor, Muon_eta[i], Muon_phi[i], PDG::Muon::Mass));
            // muons.push_back(ROOT::Math::PtEtaPhiMVector(Muon_pt[i], Muon_eta[i], Muon_phi[i], PDG::Muon::Mass));}
        }
    }
    const auto muon_reordering_mask = VecOps::Argsort(muons,
                                                      [](auto muon_1, auto muon_2) -> bool
                                                      {
                                                          return muon_1.pt() > muon_2.pt();
                                                      });

    return VecOps::Take(muons, muon_reordering_mask);

    return muons;
}

inline auto make_jets(const RVec<float> &Jet_pt,            //
                      const RVec<float> &Jet_eta,           //
                      const RVec<float> &Jet_phi,           //
                      const RVec<float> &Jet_mass,          //
                      const RVec<Int_t> &Jet_jetId,         //
                      const RVec<float> &Jet_btagDeepFlavB, //
                      const RVec<float> &Jet_rawFactor,     //
                      const RVec<float> &Jet_area,          //
                      const RVec<Int_t> &Jet_genJetIdx,     //
                      float fixedGridRhoFastjetAll,         //
                      JetCorrector &jet_corrections,        //
                      const NanoObjects::GenJets &gen_jets, //
                      std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto bjets = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Jet_pt.size(); i++)
    {
        // JES: Nominal - JER: Nominal
        float scale_correction_nominal = jet_corrections.get_scale_correction(Jet_pt[i],              //
                                                                              Jet_eta[i],             //
                                                                              Jet_phi[i],             //
                                                                              Jet_rawFactor[i],       //
                                                                              fixedGridRhoFastjetAll, //
                                                                              Jet_area[i],            //
                                                                              "Nominal"s);

        float new_pt_nominal = Jet_pt[i] * scale_correction_nominal;

        float resolution_correction_nominal = jet_corrections.get_resolution_correction(new_pt_nominal,
                                                                                        Jet_eta[i],             //
                                                                                        Jet_phi[i],             //
                                                                                        fixedGridRhoFastjetAll, //
                                                                                        Jet_genJetIdx[i],       //
                                                                                        gen_jets,               //
                                                                                        "Nominal"s);
        auto is_good_jet =
            (Jet_pt.at(i) * scale_correction_nominal * resolution_correction_nominal >= ObjConfig::Jets[year].MinPt) //
            && (std::fabs(Jet_eta.at(i)) <= ObjConfig::Jets[year].MaxAbsEta)                                         //
            && (Jet_jetId.at(i) >= ObjConfig::Jets[year].MinJetID)                                                   //
            && (Jet_btagDeepFlavB.at(i) < ObjConfig::Jets[year].MaxBTagWPTight);

        if (is_good_jet)
        {
            bjets.push_back(ROOT::Math::PtEtaPhiMVector(                                   //
                Jet_pt.at(i) * scale_correction_nominal * resolution_correction_nominal,   //
                Jet_eta.at(i),                                                             //
                Jet_phi.at(i),                                                             //
                Jet_mass.at(i) * scale_correction_nominal * resolution_correction_nominal) //
            );
        }
    }

    const auto jets_reordering_mask = VecOps::Argsort(bjets,
                                                      [](auto jet_1, auto jet_2) -> bool
                                                      {
                                                          return jet_1.pt() > jet_2.pt();
                                                      });

    return VecOps::Take(bjets, jets_reordering_mask);

    return bjets;
}

inline auto make_bjets(const RVec<float> &Jet_pt,            //
                       const RVec<float> &Jet_eta,           //
                       const RVec<float> &Jet_phi,           //
                       const RVec<float> &Jet_mass,          //
                       const RVec<Int_t> &Jet_jetId,         //
                       const RVec<float> &Jet_btagDeepFlavB, //
                       const RVec<float> &Jet_rawFactor,     //
                       const RVec<float> &Jet_area,          //
                       const RVec<Int_t> &Jet_genJetIdx,     //
                       float fixedGridRhoFastjetAll,         //
                       JetCorrector &jet_corrections,        //
                       const NanoObjects::GenJets &gen_jets, //
                       std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto jets = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Jet_pt.size(); i++)
    {
        // JES: Nominal - JER: Nominal
        float scale_correction_nominal = jet_corrections.get_scale_correction(Jet_pt[i],              //
                                                                              Jet_eta[i],             //
                                                                              Jet_phi[i],             //
                                                                              Jet_rawFactor[i],       //
                                                                              fixedGridRhoFastjetAll, //
                                                                              Jet_area[i],            //
                                                                              "Nominal"s);

        float new_pt_nominal = Jet_pt[i] * scale_correction_nominal;

        float resolution_correction_nominal = jet_corrections.get_resolution_correction(new_pt_nominal,
                                                                                        Jet_eta[i],             //
                                                                                        Jet_phi[i],             //
                                                                                        fixedGridRhoFastjetAll, //
                                                                                        Jet_genJetIdx[i],       //
                                                                                        gen_jets,               //
                                                                                        "Nominal"s);
        auto is_good_jet =
            (Jet_pt.at(i) * scale_correction_nominal * resolution_correction_nominal >= ObjConfig::Jets[year].MinPt) //
            && (std::fabs(Jet_eta.at(i)) <= ObjConfig::Jets[year].MaxAbsEta)                                         //
            && (Jet_jetId.at(i) >= ObjConfig::Jets[year].MinJetID)                                                   //
            && (Jet_btagDeepFlavB.at(i) >= ObjConfig::Jets[year].MaxBTagWPTight);
        if (is_good_jet)
        {
            jets.push_back(ROOT::Math::PtEtaPhiMVector(                                    //
                Jet_pt.at(i) * scale_correction_nominal * resolution_correction_nominal,   //
                Jet_eta.at(i),                                                             //
                Jet_phi.at(i),                                                             //
                Jet_mass.at(i) * scale_correction_nominal * resolution_correction_nominal) //
            );
        }
    }

    const auto jets_reordering_mask = VecOps::Argsort(jets,
                                                      [](auto jet_1, auto jet_2) -> bool
                                                      {
                                                          return jet_1.pt() > jet_2.pt();
                                                      });

    return VecOps::Take(jets, jets_reordering_mask);

    return jets;
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

#endif // CLASSIFICATION_HPP