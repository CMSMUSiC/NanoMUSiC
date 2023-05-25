#include "JetVal.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

// Jet validation class

// constructor
JetVal::JetVal(const std::string &output_path, const std::map<int, std::map<std::string, int>> &countMap) // countmap contains count maps for 0,1,2 jets
    : output_file(std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE")))
{
    h_2jet_invariant_mass = rebin_histogram(h_2jet_invariant_mass, countMap.at(2)); // select correct count map for 2 or 1 jet histograms
    h_2jet_sum_pt = rebin_histogram(h_2jet_sum_pt, countMap.at(2));
    h_1jet_pt = rebin_histogram(h_1jet_pt, countMap.at(1));
    h_2jet_1_pt = rebin_histogram(h_2jet_1_pt, countMap.at(2));
    h_2jet_2_pt = rebin_histogram(h_2jet_2_pt, countMap.at(2));
    h_2jet_Z_invariant_mass = rebin_histogram(h_2jet_Z_invariant_mass, countMap.at(2), true);
    h_met = rebin_histogram(h_met, countMap.at(0), false, "MET");
    h_1jet_met = rebin_histogram(h_1jet_met, countMap.at(1), false, "MET");
    h_2jet_met = rebin_histogram(h_2jet_met, countMap.at(2), false, "MET");

    h_2jet_invariant_mass.Sumw2();
    h_2jet_sum_pt.Sumw2();
    h_met.Sumw2();
    h_1jet_pt.Sumw2();
    h_1jet_eta.Sumw2();
    h_1jet_phi.Sumw2();
    h_2jet_1_pt.Sumw2();
    h_2jet_2_pt.Sumw2();
    h_2jet_1_eta.Sumw2();
    h_2jet_2_eta.Sumw2();
    h_2jet_1_phi.Sumw2();
    h_2jet_2_phi.Sumw2();
    h_njet.Sumw2();
    h_nbjet.Sumw2();
    h_nelectron.Sumw2();
    h_nmuon.Sumw2();
    h_nfermion.Sumw2();
    h_1jet_nelectron.Sumw2();
    h_1jet_nmuon.Sumw2();
    h_1jet_nfermion.Sumw2();
    h_2jet_nelectron.Sumw2();
    h_2jet_nmuon.Sumw2();
    h_2jet_nfermion.Sumw2();
    h_2jet_Z_invariant_mass.Sumw2();
}

// fill0: fill >=0jet plots
// fill0(nBJet, nJet, nElectron, nMuon, met, weight)
auto JetVal::fill0(unsigned int nBJet, unsigned int nJet, unsigned int nElectron, unsigned int nMuon, std::optional<float> met, float weight) -> void
{
    if (met)
    {
        h_met.Fill(met.value(), weight);
    }
    h_njet.Fill(nJet, weight);
    h_nbjet.Fill(nBJet, weight);
    h_nelectron.Fill(nElectron, weight);
    h_nmuon.Fill(nMuon, weight);
    h_nfermion.Fill(nElectron + nMuon, weight);
}

// fill1: fill >=1jet plots
// fill1(jet_1, nElectron, nMuon, met, weight)
auto JetVal::fill1(Math::PtEtaPhiMVector jet_1, unsigned int nElectron, unsigned int nMuon, std::optional<float> met, float weight) -> void
{
    if (met)
    {
        h_1jet_met.Fill(met.value(), weight);
    }
    auto pt = jet_1.pt();
    if(pt > min_pt_1jet)
    {
        h_1jet_pt.Fill(pt, weight);
        h_1jet_eta.Fill(jet_1.eta(), weight);
        h_1jet_phi.Fill(jet_1.phi(), weight);
    }
    h_1jet_nelectron.Fill(nElectron, weight);
    h_1jet_nmuon.Fill(nMuon, weight);
    h_1jet_nfermion.Fill(nElectron + nMuon, weight);
}

// fill2: fill >=2jet plots
// fill2(jet_1, jet_2, nElectron, nMuon, met, weight)
auto JetVal::fill2(Math::PtEtaPhiMVector jet_1, Math::PtEtaPhiMVector jet_2, unsigned int nElectron, unsigned int nMuon, std::optional<float> met, float weight) -> void
{
    if (met)
    {
        h_2jet_met.Fill(met.value(), weight);
    }
    auto minv = (jet_1 + jet_2).mass();
    if(minv > min_minv_2jet)
    {
        h_2jet_invariant_mass.Fill(minv, weight);
        h_2jet_sum_pt.Fill(jet_1.pt() + jet_2.pt(), weight);
        h_2jet_1_pt.Fill(jet_1.pt(), weight);
        h_2jet_1_eta.Fill(jet_1.eta(), weight);
        h_2jet_1_phi.Fill(jet_1.phi(), weight);
        h_2jet_2_pt.Fill(jet_2.pt(), weight);
        h_2jet_2_eta.Fill(jet_2.eta(), weight);
        h_2jet_2_phi.Fill(jet_2.phi(), weight);
    }
    h_2jet_nelectron.Fill(nElectron, weight);
    h_2jet_nmuon.Fill(nMuon, weight);
    h_2jet_nfermion.Fill(nElectron + nMuon, weight);
}

// fill Z mass diagram
// fill2z: fill >=2jet plots
// syntax fill histograms: fill2z(jet_1, jet_2, weight)
auto JetVal::fill2z(Math::PtEtaPhiMVector jet_1, Math::PtEtaPhiMVector jet_2, float weight) -> void
{
    h_2jet_Z_invariant_mass.Fill((jet_1 + jet_2).mass(), weight);
}

// save histograms
auto JetVal::save_histo(TH1F &histo) -> void
{
    histo.SetDirectory(output_file.get());
    histo.Write();
}

// dump outputs
auto JetVal::dump_outputs() -> void
{
    // fmt::print("Saving outputs to: {}\n", output_file->GetPath());
    output_file->cd();
    save_histo(h_2jet_invariant_mass);
    save_histo(h_2jet_sum_pt);
    save_histo(h_met);
    save_histo(h_1jet_met);
    save_histo(h_2jet_met);
    save_histo(h_1jet_pt);
    save_histo(h_1jet_eta);
    save_histo(h_1jet_phi);
    save_histo(h_2jet_1_pt);
    save_histo(h_2jet_2_pt);
    save_histo(h_2jet_1_eta);
    save_histo(h_2jet_2_eta);
    save_histo(h_2jet_1_phi);
    save_histo(h_2jet_2_phi);
    save_histo(h_njet);
    save_histo(h_nbjet);
    save_histo(h_nelectron);
    save_histo(h_nmuon);
    save_histo(h_nfermion);
    save_histo(h_1jet_nelectron);
    save_histo(h_1jet_nmuon);
    save_histo(h_1jet_nfermion);
    save_histo(h_2jet_nelectron);
    save_histo(h_2jet_nmuon);
    save_histo(h_2jet_nfermion);
    save_histo(h_2jet_Z_invariant_mass);
    output_file->Close();
}