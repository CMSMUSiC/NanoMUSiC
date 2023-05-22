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
JetVal::JetVal(const std::string &output_path)
    : output_file(std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE")))
{
}

// fill0: fill >=0jet plots
// fill0(nBJet, nJet, met, weight)
auto JetVal::fill0(unsigned int nBJet, unsigned int nJet, std::optional<float> met, float weight) -> void
{
    if (met)
    {
        h_met.Fill(met.value(), weight);
    }
    h_jet_multiplicity.Fill(nJet, weight);
    h_bjet_multiplicity.Fill(nBJet, weight);
}

// fill1: fill >=1jet plots
// fill1(jet_1, weight)
auto JetVal::fill1(Math::PtEtaPhiMVector jet_1, float weight) -> void
{
    h_1jet_pt.Fill(jet_1.pt(), weight);
    h_1jet_eta.Fill(jet_1.eta(), weight);
    h_1jet_phi.Fill(jet_1.phi(), weight);
}

// fill2: fill >=2jet plots
// syntax fill histograms: fill2(jet_1, jet_2, weight)
auto JetVal::fill2(Math::PtEtaPhiMVector jet_1, Math::PtEtaPhiMVector jet_2, float weight) -> void
{
    h_2jet_invariant_mass.Fill((jet_1 + jet_2).mass(), weight);
    h_2jet_sum_pt.Fill(jet_1.pt() + jet_2.pt(), weight);
    h_2jet_1_pt.Fill(jet_1.pt(), weight);
    h_2jet_1_eta.Fill(jet_1.eta(), weight);
    h_2jet_1_phi.Fill(jet_1.phi(), weight);
    h_2jet_2_pt.Fill(jet_2.pt(), weight);
    h_2jet_2_eta.Fill(jet_2.eta(), weight);
    h_2jet_2_phi.Fill(jet_2.phi(), weight);
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
    save_histo(h_1jet_pt);
    save_histo(h_1jet_eta);
    save_histo(h_1jet_phi);
    save_histo(h_2jet_1_pt);
    save_histo(h_2jet_2_pt);
    save_histo(h_2jet_1_eta);
    save_histo(h_2jet_2_eta);
    save_histo(h_2jet_1_phi);
    save_histo(h_2jet_2_phi);
    save_histo(h_jet_multiplicity);
    save_histo(h_bjet_multiplicity);
    save_histo(h_2jet_Z_invariant_mass);
    output_file->Close();
}