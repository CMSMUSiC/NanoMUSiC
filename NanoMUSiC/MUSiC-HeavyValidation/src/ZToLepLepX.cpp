#include "../include/ZToLepLepX.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include "TEfficiency.h"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

ZToLepLepX::ZToLepLepX(const std::string &output_path,
                       const std::map<std::string, int> &countMap,
                       bool is_Z_mass_validation)
    : output_file(std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE")))
{
    // h_invariant_mass = rebin_histogram(h_invariant_mass, countMap, is_Z_mass_validation, "validation_plot", 0.,
    // 1000.);
    h_sum_pt = rebin_histogram(h_sum_pt, countMap);
    h_met = rebin_histogram(h_met, countMap, false, "MET");
    h_lepton_1_pt = rebin_histogram(h_lepton_1_pt, countMap);
    h_lepton_2_pt = rebin_histogram(h_lepton_2_pt, countMap);

    h_invariant_mass.Sumw2();
    h_sum_pt.Sumw2();
    h_met.Sumw2();
    h_lepton_1_pt.Sumw2();
    h_lepton_2_pt.Sumw2();
    h_lepton_1_eta.Sumw2();
    h_lepton_2_eta.Sumw2();
    h_lepton_1_phi.Sumw2();
    h_lepton_2_phi.Sumw2();
    h_lepton_1_jet_1_dPhi.Sumw2();
    h_lepton_1_jet_1_dR.Sumw2();
    h_jet_multiplicity.Sumw2();
    h_bjet_multiplicity.Sumw2();

    h_lepton_1_pt_eta.Sumw2();
    h_lepton_1_pt_phi.Sumw2();
}

auto ZToLepLepX::fill(const Math::PtEtaPhiMVector &lepton_1,
                      const Math::PtEtaPhiMVector &lepton_2,
                      unsigned int nBJet,
                      std::optional<Math::PtEtaPhiMVector> bjet,
                      unsigned int nJet,
                      std::optional<Math::PtEtaPhiMVector> jet,
                      std::optional<float> met,
                      float weight) -> void
{
    fmt::print("lepton_1: {}\n", lepton_1);
    fmt::print("lepton_2: {}\n", lepton_2);
    fmt::print("h_invariant_mass V2: {}\n", (lepton_1 + lepton_2).mass());

    h_invariant_mass.Fill((lepton_1 + lepton_2).mass(), weight);
    h_sum_pt.Fill(lepton_1.pt() + lepton_2.pt(), weight);
    if (met)
    {
        h_met.Fill(met.value(), weight);
    }
    h_lepton_1_pt.Fill(lepton_1.pt(), weight);
    h_lepton_2_pt.Fill(lepton_2.pt(), weight);
    h_lepton_1_eta.Fill(lepton_1.eta(), weight);
    h_lepton_2_eta.Fill(lepton_2.eta(), weight);
    h_lepton_1_phi.Fill(lepton_1.phi(), weight);
    h_lepton_2_phi.Fill(lepton_2.phi(), weight);
    if (jet or bjet)
    {
        Math::PtEtaPhiMVector leading_jet = [&]() -> Math::PtEtaPhiMVector
        {
            if (jet and not(bjet))
            {
                return *jet;
            }
            if (not(jet) and bjet)
            {
                return *bjet;
            }
            if ((*jet).pt() > (*bjet).pt())
            {
                return *jet;
            }
            return *bjet;
        }();
        h_lepton_1_jet_1_dPhi.Fill(VectorUtil::DeltaPhi(lepton_1, leading_jet), weight);
        h_lepton_1_jet_1_dR.Fill(VectorUtil::DeltaR(lepton_1, leading_jet), weight);
    }
    h_jet_multiplicity.Fill(nJet, weight);
    h_bjet_multiplicity.Fill(nBJet, weight);

    h_lepton_1_pt_eta.Fill(lepton_1.pt(), lepton_1.eta(), weight);
    h_lepton_1_pt_phi.Fill(lepton_1.pt(), lepton_1.phi(), weight);
}

auto ZToLepLepX::save_histo(TH1F &histo) -> void
{
    histo.SetDirectory(output_file.get());
    histo.Write();
}

auto ZToLepLepX::save_histo(TH2F &histo) -> void
{
    histo.SetDirectory(output_file.get());
    histo.Write();
}

auto ZToLepLepX::dump_outputs() -> void
{
    // fmt::print("Saving outputs to: {}\n", output_file->GetPath());
    output_file->cd();
    save_histo(h_invariant_mass);
    save_histo(h_sum_pt);
    save_histo(h_met);
    save_histo(h_lepton_1_pt);
    save_histo(h_lepton_2_pt);
    save_histo(h_lepton_1_eta);
    save_histo(h_lepton_2_eta);
    save_histo(h_lepton_1_phi);
    save_histo(h_lepton_2_phi);
    save_histo(h_lepton_1_jet_1_dPhi);
    save_histo(h_lepton_1_jet_1_dR);
    save_histo(h_jet_multiplicity);
    save_histo(h_bjet_multiplicity);
    save_histo(h_lepton_1_pt_eta);
    save_histo(h_lepton_1_pt_phi);

    output_file->Close();
}

auto ZToLepLepX::dump_outputs(TEfficiency &efficiency) -> void
{
    // fmt::print("Saving outputs to: {}\n", output_file->GetPath());
    output_file->cd();
    save_histo(h_invariant_mass);
    save_histo(h_sum_pt);
    save_histo(h_met);
    save_histo(h_lepton_1_pt);
    save_histo(h_lepton_2_pt);
    save_histo(h_lepton_1_eta);
    save_histo(h_lepton_2_eta);
    save_histo(h_lepton_1_phi);
    save_histo(h_lepton_2_phi);
    save_histo(h_lepton_1_jet_1_dPhi);
    save_histo(h_lepton_1_jet_1_dR);
    save_histo(h_jet_multiplicity);
    save_histo(h_bjet_multiplicity);
    save_histo(h_lepton_1_pt_eta);
    save_histo(h_lepton_1_pt_phi);

    efficiency.SetDirectory(output_file.get());
    efficiency.Write();

    output_file->Close();
}