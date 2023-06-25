#include "JetClass2D.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

// Jet classification validation

// histo.Scale(min_bin_width, "width");

// constructor
JetClass2D::JetClass2D(const std::string &output_path, const std::string c_name)
    : output_file(std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE"))),
      c_name(c_name)
{
    // extract nJet and nBJet from classname
    c_nJet = std::stoi(c_name.substr(0, c_name.find("J")));
    c_nBJet = std::stoi(c_name.substr(c_name.find("+") + 1, c_name.find("BJ")));
    c_nMET = 0; // default (+0MET or +XMET): no MET considered in the distributions
    if (c_name.find("+1MET") != std::string::npos)
    {
        c_nMET = 1; // consider MET
    }

    // Sumw2
    h_deltar_jetjet.Sumw2();
    h_deltaphi_jetjet.Sumw2();
    h_deltaeta_jetjet.Sumw2();
    h_nmuon.Sumw2();
}

// fill histogram for an event in the class
// fill(jets, bjets, nElectron, nMuon, met, weight)
// jets is a RVec of 4-vectors including all jets in the event sorted (highest pt first)
auto JetClass2D::fill(RVec<Math::PtEtaPhiMVector> jets,
                     RVec<Math::PtEtaPhiMVector> bjets,
                     unsigned int nElectron,
                     unsigned int nMuon,
                     RVec<Math::PtEtaPhiMVector> met,
                     float weight) -> void
{
    // quality control
    if (c_nJet > jets.size())
    {
        throw std::runtime_error(fmt::format("ERROR: JetClass {}: jet vector (size {}) set can not be smaller as nJet "
                                             "taken from the class name (value {}).",
                                             c_name,
                                             jets.size(),
                                             c_nJet)
                                     .c_str());
    }
    if (c_nBJet > bjets.size())
    {
        throw std::runtime_error(fmt::format("ERROR: JetClass {}: bjet vector (size {}) set can not be smaller as "
                                             "nBJet taken from the class name (value {}).",
                                             c_name,
                                             bjets.size(),
                                             c_nBJet)
                                     .c_str());
    }
    if (c_nMET == 0 and met.size() >= 1)
    {
        throw std::runtime_error(
            fmt::format("ERROR: JetClass {}: found MET in the event although it was not required by the class name.",
                        c_name)
                .c_str());
    }
    if (c_nMET == 1 and met.size() < 1)
    {
        throw std::runtime_error(
            fmt::format("ERROR: JetClass {}: found no MET in the event although it was required by the class name.",
                        c_name)
                .c_str());
    }
    // deltar, deltaphi, deltaeta
    if (c_nJet >= 2)
    {
        h_deltar_jetjet.Fill(Math::VectorUtil::DeltaR(jets.at(0), jets.at(1)), weight);
        h_deltaphi_jetjet.Fill(std::abs(jets.at(0).phi() - jets.at(1).phi()), weight);
        h_deltaeta_jetjet.Fill(std::abs(jets.at(0).eta() - jets.at(1).eta()), weight);
    }
    // multiplicities
    h_nmuon.Fill(nMuon, weight);
}

// save histograms
auto JetClass2D::save_histo(TH1F &histo) -> void
{
    histo.Scale(10, "width"); // FIXES STEPS IN DISTRIBUTIONS
    histo.SetDirectory(output_file.get());
    histo.Write();
}

// dump outputs
auto JetClass2D::dump_outputs() -> void
{
    // fmt::print("Saving outputs to: {}\n", output_file->GetPath());
    output_file->cd();
    save_histo(h_nmuon);
    save_histo(h_deltar_jetjet);
    save_histo(h_deltaphi_jetjet);
    save_histo(h_deltaeta_jetjet);
    output_file->Close();
}