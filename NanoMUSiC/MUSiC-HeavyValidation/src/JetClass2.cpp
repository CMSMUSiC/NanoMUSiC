#include "JetClass2.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

// Jet classification validation

// constructor
JetClass2::JetClass2(const std::string &output_path, const std::string c_name)
    : output_file(std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE"))), c_name(c_name)
{
    // extract nJet and nBJet from classname
    c_nJet = std::stoi(c_name.substr(0, c_name.find("J")));
    c_nBJet = std::stoi(c_name.substr(c_name.find("+") + 1, c_name.find("BJ")));

    // hist rebinning with nJet, nBJet countmap
    h_m_inv = rebin_histogram(h_m_inv, return_jet_countmap(c_nJet, c_nBJet));
    h_sum_pt = rebin_histogram(h_sum_pt, return_jet_countmap(c_nJet, c_nBJet));
    h_pt_1st_jet = rebin_histogram(h_pt_1st_jet, return_jet_countmap(1, 0));
    h_pt_2nd_jet = rebin_histogram(h_pt_2nd_jet, return_jet_countmap(1, 0));
    h_pt_1st_bjet = rebin_histogram(h_pt_1st_bjet, return_jet_countmap(0, 1));
    h_pt_2nd_bjet = rebin_histogram(h_pt_2nd_bjet, return_jet_countmap(0, 1));
    h_met = rebin_histogram(h_met, return_jet_countmap(c_nJet, c_nBJet), false, "MET");
    
    // Sumw2
    h_m_inv.Sumw2();
    h_sum_pt.Sumw2();
    h_met.Sumw2();
    h_pt_1st_jet.Sumw2();
    h_eta_1st_jet.Sumw2();
    h_phi_1st_jet.Sumw2();
    h_pt_2nd_jet.Sumw2();
    h_eta_2nd_jet.Sumw2();
    h_phi_2nd_jet.Sumw2();
    h_pt_1st_bjet.Sumw2();
    h_eta_1st_bjet.Sumw2();
    h_phi_1st_bjet.Sumw2();
    h_pt_2nd_bjet.Sumw2();
    h_eta_2nd_bjet.Sumw2();
    h_phi_2nd_bjet.Sumw2();
    h_njet.Sumw2();
    h_nbjet.Sumw2();
    h_nelectron.Sumw2();
    h_nmuon.Sumw2();
    h_nemu.Sumw2();
    h_deltar_jetjet.Sumw2();
    h_deltar_jetbjet.Sumw2();
    h_deltar_bjetbjet.Sumw2();
    
}

// fill histogram for an event in the class
// fill(jets, bjets, nElectron, nMuon, met, weight)
// jets is a RVec of 4-vectors including all jets in the event sorted (highest pt first)
auto JetClass2::fill(RVec<Math::PtEtaPhiMVector> jets, RVec<Math::PtEtaPhiMVector> bjets,
                    unsigned int nElectron, unsigned int nMuon, std::optional<float> met, float weight) -> void
{
    // validate nJet/nBJet with class name
    if(c_nJet > jets.size())
    {
        throw std::runtime_error(fmt::format("ERROR: JetClass {}: jet vector (size {}) set can not be smaller as nJet taken from the class name (value {}).",c_name,jets.size(),c_nJet).c_str());
    }
    if(c_nBJet > bjets.size())
    {
        throw std::runtime_error(fmt::format("ERROR: JetClass {}: bjet vector (size {}) set can not be smaller as nBJet taken from the class name (value {}).",c_name,bjets.size(),c_nBJet).c_str());
    }
    // met
    if (met)
    {
        h_met.Fill(met.value(), weight);
    }
    // sum_pt and m_inv
    auto jetsum = Math::PtEtaPhiMVector(0, 0, 0, 0);
    float sumpt = 0;
    for(unsigned int i = 0; i < c_nJet; i++)
    {
        sumpt += jets.at(i).pt();
        jetsum += jets.at(i);
    }
    for(unsigned int i = 0; i < c_nBJet; i++)
    {
        sumpt += bjets.at(i).pt();
        jetsum += bjets.at(i);
    }
    if(c_nJet >= 1 or c_nBJet >= 1)
    {
        h_m_inv.Fill(jetsum.mass(), weight);
        h_sum_pt.Fill(sumpt, weight);
    }
    // leading jet
    if(c_nJet >= 1)
    {
        h_pt_1st_jet.Fill(jets.at(0).pt(), weight);
        h_eta_1st_jet.Fill(jets.at(0).eta(), weight);
        h_phi_1st_jet.Fill(jets.at(0).phi(), weight);
    }
    // 2nd leading jet
    if(c_nJet >= 2)
    {
        h_pt_2nd_jet.Fill(jets.at(1).pt(), weight);
        h_eta_2nd_jet.Fill(jets.at(1).eta(), weight);
        h_phi_2nd_jet.Fill(jets.at(1).phi(), weight);
    }
    // leading bjet
    if(c_nBJet >= 1)
    {
        h_pt_1st_bjet.Fill(bjets.at(0).pt(), weight);
        h_eta_1st_bjet.Fill(bjets.at(0).eta(), weight);
        h_phi_1st_bjet.Fill(bjets.at(0).phi(), weight);
    }
    // 2nd leading bjet
    if(c_nBJet >= 2)
    {
        h_pt_2nd_bjet.Fill(bjets.at(1).pt(), weight);
        h_eta_2nd_bjet.Fill(bjets.at(1).eta(), weight);
        h_phi_2nd_bjet.Fill(bjets.at(1).phi(), weight);
    }
    // deltar
    if(c_nJet >= 2)
    {
        h_deltar_jetjet.Fill(Math::VectorUtil::DeltaR(jets.at(0), jets.at(1)), weight);
    }
    if(c_nBJet >= 2)
    {
        h_deltar_bjetbjet.Fill(Math::VectorUtil::DeltaR(bjets.at(0), bjets.at(1)), weight);
    }
    if(c_nJet >= 1 and c_nBJet >= 1)
    {
        h_deltar_jetbjet.Fill(Math::VectorUtil::DeltaR(jets.at(0), bjets.at(0)), weight);
    }
    // multiplicities
    h_njet.Fill(jets.size(), weight);
    h_nbjet.Fill(bjets.size(), weight);
    h_nelectron.Fill(nElectron, weight);
    h_nmuon.Fill(nMuon, weight);
    h_nemu.Fill(nElectron + nMuon, weight);
}

// save histograms
auto JetClass2::save_histo(TH1F &histo) -> void
{
    histo.SetDirectory(output_file.get());
    histo.Write();
}

// dump outputs
auto JetClass2::dump_outputs() -> void
{
    // fmt::print("Saving outputs to: {}\n", output_file->GetPath());
    output_file->cd();
    save_histo(h_m_inv);
    save_histo(h_sum_pt);
    save_histo(h_met);
    save_histo(h_pt_1st_jet);
    save_histo(h_eta_1st_jet);
    save_histo(h_phi_1st_jet);
    save_histo(h_pt_2nd_jet);
    save_histo(h_eta_2nd_jet);
    save_histo(h_phi_2nd_jet);
    save_histo(h_pt_1st_bjet);
    save_histo(h_eta_1st_bjet);
    save_histo(h_phi_1st_bjet);
    save_histo(h_pt_2nd_bjet);
    save_histo(h_eta_2nd_bjet);
    save_histo(h_phi_2nd_bjet);
    save_histo(h_njet);
    save_histo(h_nbjet);
    save_histo(h_nelectron);
    save_histo(h_nmuon);
    save_histo(h_nemu);
    save_histo(h_deltar_jetjet);
    save_histo(h_deltar_jetbjet);
    save_histo(h_deltar_bjetbjet);
    output_file->Close();
}