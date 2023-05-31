#include "RtypesCore.h"
#include "TCanvas.h"
#include "fmt/format.h"

#include "Math/Vector4D.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "TH1.h"
#include "TH2.h"

#include "input_files.hpp"
#include <chrono>
#include <cstddef>
#include <mutex>
#include <numeric>
#include <thread>

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

constexpr float muon_mass = 105.6583755 / 1000.0;

// define columns to be processed
inline auto cols() -> std::vector<std::string>
{
    return {
        "luminosityBlock", //
        "run",             //
        "PV_npvsGood",     //
        "Flag_goodVertices",
        "Flag_globalSuperTightHalo2016Filter",
        "Flag_HBHENoiseFilter",
        "Flag_HBHENoiseIsoFilter",
        "Flag_EcalDeadCellTriggerPrimitiveFilter",
        "Flag_BadPFMuonFilter",
        "Flag_BadPFMuonDzFilter",
        "Flag_eeBadScFilter",
        "Flag_ecalBadCalibFilter",
        "Muon_pt",             //
        "Muon_eta",            //
        "Muon_phi",            //
        "Muon_tightId",        //
        "Muon_highPtId",       //
        "Muon_pfRelIso04_all", //
        "Muon_tkRelIso",       //
        "Muon_tunepRelPt",     //
        "HLT_IsoMu24",         //
        "HLT_Mu50",            //
        "HLT_TkMu100",         //
        "HLT_OldMu100"         //
    };
}

inline auto save_as(std::vector<TH1F> &histo, std::string &&filename) -> void
{
    system(fmt::format("rm {}.png", filename).c_str());
    system(fmt::format("rm {}.pdf", filename).c_str());

    auto c = TCanvas();

    // Set logarithmic scale on the y-axis
    c.SetLogy();

    TH1F sum_histo = histo[0];
    for (std::size_t i = 1; i < histo.size(); i++)
    {
        sum_histo.Add(&histo[i]);
    }

    sum_histo.Draw("ep1");

    c.SaveAs((filename + ".png").c_str());
    c.SaveAs((filename + ".pdf").c_str());
}

inline auto save_as(std::vector<TH2F> &histo, std::string &&filename) -> void
{
    system(fmt::format("rm {}.png", filename).c_str());
    system(fmt::format("rm {}.pdf", filename).c_str());

    auto c = TCanvas();

    TH2F sum_histo = histo[0];
    for (std::size_t i = 1; i < histo.size(); i++)
    {
        sum_histo.Add(&histo[i]);
    }

    sum_histo.Draw("colz");

    c.SaveAs((filename + ".png").c_str());
    c.SaveAs((filename + ".pdf").c_str());
}

inline auto save(std::vector<TH1F> &histo, std::string &&filename) -> void
{
    system(fmt::format("rm {}.root", filename).c_str());

    TH1F sum_histo = histo[0];
    for (std::size_t i = 1; i < histo.size(); i++)
    {
        sum_histo.Add(&histo[i]);
    }

    // Create a ROOT file
    TFile file = TFile(fmt::format("{}.root", filename).c_str(), "RECREATE");

    // Write the histogram to the file
    sum_histo.Write();

    // Close the file
    file.Close();
}

inline auto save(std::vector<TH2F> &histo, std::string &&filename) -> void
{
    system(fmt::format("rm {}.root", filename).c_str());

    TH2F sum_histo = histo[0];
    for (std::size_t i = 1; i < histo.size(); i++)
    {
        sum_histo.Add(&histo[i]);
    }

    // Create a ROOT file
    TFile file = TFile(fmt::format("{}.root", filename).c_str(), "RECREATE");

    // Write the histogram to the file
    sum_histo.Write();

    // Close the file
    file.Close();
}

inline auto event_counter(std::mutex &mtx, bool &lopp_is_done, std::vector<unsigned long> &event_counters) -> void
{
    while (not(lopp_is_done))
    {
        // Acquire the mutex
        // std::unique_lock<std::mutex> lock(mtx);

        // Perform the computation
        std::cout << "Processed events: " << std::accumulate(event_counters.cbegin(), event_counters.cend(), 0)
                  << std::endl;

        // Release the mutex
        // lock.unlock();

        // Sleep for 10 seconds
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}