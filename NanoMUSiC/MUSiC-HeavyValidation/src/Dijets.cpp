#include "Dijets.hpp"

#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include "TEfficiency.h"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

Dijets::Dijets(const std::string &output_path, const std::map<std::string, int> &countMap)
    : output_file(std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE"))),
      min_bin_width(10.)
{
    h_invariant_mass = rebin_histogram(h_invariant_mass, countMap);
    h_sum_pt = rebin_histogram(h_sum_pt, countMap);
    h_met = rebin_histogram(h_met, countMap, false, "MET");
    h_jet_1_pt = rebin_histogram(h_jet_1_pt, countMap);
    h_jet_2_pt = rebin_histogram(h_jet_2_pt, countMap);

    h_invariant_mass.Sumw2();
    h_sum_pt.Sumw2();
    h_met.Sumw2();
    h_jet_1_pt.Sumw2();
    h_jet_2_pt.Sumw2();
    h_jet_1_eta.Sumw2();
    h_jet_2_eta.Sumw2();
    h_jet_1_phi.Sumw2();
    h_jet_2_phi.Sumw2();
    // h_jet_1_jet_1_dPhi.Sumw2();
    // h_jet_1_jet_1_dR.Sumw2();
    // h_jet_multiplicity.Sumw2();
    // h_bjet_multiplicity.Sumw2();

    // h_jet_1_pt_eta.Sumw2();
    // h_jet_1_pt_phi.Sumw2();
}

auto Dijets::fill(const Math::PtEtaPhiMVector &jet_1,
                  const Math::PtEtaPhiMVector &jet_2,
                  //   unsigned int nBJet,
                  //   std::optional<Math::PtEtaPhiMVector> bjet,
                  //   unsigned int nJet,
                  //   std::optional<Math::PtEtaPhiMVector> jet,
                  std::optional<float> met,
                  float weight) -> void
{
    h_invariant_mass.Fill((jet_1 + jet_2).mass(), weight);
    h_sum_pt.Fill(jet_1.pt() + jet_2.pt(), weight);
    if (met)
    {
        h_met.Fill(met.value(), weight);
    }
    h_jet_1_pt.Fill(jet_1.pt(), weight);
    h_jet_2_pt.Fill(jet_2.pt(), weight);
    h_jet_1_eta.Fill(jet_1.eta(), weight);
    h_jet_2_eta.Fill(jet_2.eta(), weight);
    h_jet_1_phi.Fill(jet_1.phi(), weight);
    h_jet_2_phi.Fill(jet_2.phi(), weight);

    // if (jet or bjet)
    // {
    //     Math::PtEtaPhiMVector leading_jet = [&]() -> Math::PtEtaPhiMVector
    //     {
    //         if (jet and not(bjet))
    //         {
    //             return *jet;
    //         }
    //         if (not(jet) and bjet)
    //         {
    //             return *bjet;
    //         }
    //         if ((*jet).pt() > (*bjet).pt())
    //         {
    //             return *jet;
    //         }
    //         return *bjet;
    //     }();
    //     h_jet_1_jet_1_dPhi.Fill(VectorUtil::DeltaPhi(jet_1, leading_jet), weight);
    //     h_jet_1_jet_1_dR.Fill(VectorUtil::DeltaR(jet_1, leading_jet), weight);
    // }
    // h_jet_multiplicity.Fill(nJet, weight);
    // h_bjet_multiplicity.Fill(nBJet, weight);

    // h_jet_1_pt_eta.Fill(jet_1.pt(), jet_1.eta(), weight);
    // h_jet_1_pt_phi.Fill(jet_1.pt(), jet_1.phi(), weight);
}

auto Dijets::save_histo(TH1F &histo) -> void
{
    histo.Scale(min_bin_width, "width");
    histo.SetDirectory(output_file.get());
    histo.Write();
}

auto Dijets::save_histo(TH2F &histo) -> void
{
    histo.Scale(min_bin_width, "width");
    histo.SetDirectory(output_file.get());
    histo.Write();
}

auto Dijets::dump_outputs() -> void
{
    // fmt::print("Saving outputs to: {}\n", output_file->GetPath());
    output_file->cd();
    save_histo(h_invariant_mass);
    save_histo(h_sum_pt);
    save_histo(h_met);
    save_histo(h_jet_1_pt);
    save_histo(h_jet_2_pt);
    save_histo(h_jet_1_eta);
    save_histo(h_jet_2_eta);
    save_histo(h_jet_1_phi);
    save_histo(h_jet_2_phi);
    // save_histo(h_jet_1_jet_1_dPhi);
    // save_histo(h_jet_1_jet_1_dR);
    // save_histo(h_jet_multiplicity);
    // save_histo(h_bjet_multiplicity);
    // save_histo(h_jet_1_pt_eta);
    // save_histo(h_jet_1_pt_phi);

    output_file->Close();
}
