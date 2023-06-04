#ifndef VALIDATION
#define VALIDATION

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

#include "MUSiCTools.hpp"
#include "TOMLConfig.hpp"
#include "ZToLepLepX.hpp"

#include "PDFAlphaSWeights.hpp"

#include "argh.h"
#include "emoji.hpp"
#include "fmt/format.h"
#include "processed_data_events.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

// helper macros
#define ADD_VALUE_READER(VAR, TYPE) auto VAR = TTreeReaderValue<TYPE>(tree_reader, #VAR)
#define ADD_ARRAY_READER(VAR, TYPE) auto VAR = TTreeReaderArray<TYPE>(tree_reader, #VAR)

template <typename T>
auto unwrap(TTreeReaderValue<T> &value) -> T
{
    return *value;
}

template <typename T>
auto unwrap(TTreeReaderArray<T> &array) -> RVec<T>
{
    return RVec<T>(static_cast<T *>(array.GetAddress()), array.GetSize());
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
                       const RVec<float> &Muon_tunepRelPt,
                       bool pass_low_pt_muon_trigger,
                       bool pass_high_pt_muon_trigger) -> RVec<Math::PtEtaPhiMVector>
{
    auto muons = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Muon_pt.size(); i++)
    {
        if (i == 2)
        {
            break;
        }

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

        // if ((is_good_low_pt_muon) and not(is_good_high_pt_muon))
        // if (is_good_low_pt_muon)
        if ((pass_low_pt_muon_trigger and is_good_low_pt_muon) or (pass_high_pt_muon_trigger and is_good_high_pt_muon))
        {
            muons.push_back(ROOT::Math::PtEtaPhiMVector(
                std::max(Muon_pt[i] * pt_correction_factor, 25.), Muon_eta[i], Muon_phi[i], PDG::Muon::Mass));
            // muons.push_back(ROOT::Math::PtEtaPhiMVector(
            //     Muon_pt[i] * pt_correction_factor, Muon_eta[i], Muon_phi[i], PDG::Muon::Mass));
            // muons.push_back(ROOT::Math::PtEtaPhiMVector(Muon_pt[i], Muon_eta[i], Muon_phi[i], PDG::Muon::Mass));
        }
    }

    // const auto muon_reordering_mask = VecOps::Argsort(muons,
    //                                                   [](auto muon_1, auto muon_2) -> bool
    //                                                   {
    //                                                       return muon_1.pt() > muon_2.pt();
    //                                                   });

    // return VecOps::Take(muons, muon_reordering_mask);

    return muons;
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

#endif // VALIDATION