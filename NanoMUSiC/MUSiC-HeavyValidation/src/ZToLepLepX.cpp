#include "../include/ZToLepLepX.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include "TEfficiency.h"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

// define a new histogram with fixed bin size
#define ADD_TH1F(SHIFT, PROCESS, YEAR, HISTO, N_BINS, LOWER_BOUND, UPPER_BOUND)                                        \
    HISTO.insert(                                                                                                      \
        {SHIFT,                                                                                                        \
         TH1F(#HISTO "_" #PROCESS "_" #YEAR, #HISTO "_" #PROCESS "_" #YEAR, N_BINS, LOWER_BOUND, UPPER_BOUND)});       \
    HISTO.at(SHIFT).Sumw2()

ZToLepLepX::ZToLepLepX(const std::string &output_path,
                       const std::map<std::string, int> &_countMap,
                       bool _is_Z_mass_validation,
                       const std::vector<std::string> &_shifts,
                       const std::string &_process,
                       const std::string &_year)
    : output_file(std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE"))),
      min_bin_width(10.),
      countMap(_countMap),
      is_Z_mass_validation(_is_Z_mass_validation),
      shifts(_shifts)
{
    for (auto &&shift : shifts)
    {
        if (is_Z_mass_validation)
        {
            ADD_TH1F(shift, _process, _year, h_invariant_mass, n_energy_bins * 10, min_energy, max_energy);
        }
        else
        {
            ADD_TH1F(shift, _process, _year, h_invariant_mass, n_energy_bins, min_energy, max_energy);
        }
        ADD_TH1F(shift, _process, _year, h_sum_pt, n_energy_bins, min_energy, max_energy);
        ADD_TH1F(shift, _process, _year, h_met, n_energy_bins, min_energy, max_energy);
        ADD_TH1F(shift, _process, _year, h_lepton_1_pt, n_energy_bins, min_energy, max_energy);
        ADD_TH1F(shift, _process, _year, h_lepton_2_pt, n_energy_bins, min_energy, max_energy);
        ADD_TH1F(shift, _process, _year, h_lepton_1_eta, n_eta_bins, min_eta, max_eta);
        ADD_TH1F(shift, _process, _year, h_lepton_2_eta, n_eta_bins, min_eta, max_eta);
        ADD_TH1F(shift, _process, _year, h_lepton_1_phi, n_phi_bins, min_phi, max_phi);
        ADD_TH1F(shift, _process, _year, h_lepton_2_phi, n_phi_bins, min_phi, max_phi);
        ADD_TH1F(shift, _process, _year, h_lepton_1_jet_1_dPhi, n_phi_bins, min_phi, max_phi);
        ADD_TH1F(shift, _process, _year, h_lepton_1_jet_1_dR, n_dR_bins, min_dR, max_dR);
        ADD_TH1F(shift, _process, _year, h_jet_multiplicity, n_multiplicity_bins, min_multiplicity, max_multiplicity);
        ADD_TH1F(shift, _process, _year, h_bjet_multiplicity, n_multiplicity_bins, min_multiplicity, max_multiplicity);

        h_lepton_1_pt_eta.insert(
            {shift,
             TH2F("h_lepton_1_pt_eta", "h_lepton_1_pt_eta", 130, min_energy, 900, n_eta_bins, min_eta, max_eta)});
        h_lepton_1_pt_eta.at(shift).Sumw2();

        h_lepton_1_pt_phi.insert(
            {shift,
             TH2F("h_lepton_1_pt_eta", "h_lepton_1_pt_eta", 130, min_energy, 900, n_phi_bins, min_phi, max_phi)});
        h_lepton_1_pt_phi.at(shift).Sumw2();
    }
}

auto ZToLepLepX::fill(const Math::PtEtaPhiMVector &lepton_1,
                      const Math::PtEtaPhiMVector &lepton_2,
                      const RVec<Math::PtEtaPhiMVector> &bjets,
                      const RVec<Math::PtEtaPhiMVector> &jets,
                      const RVec<Math::PtEtaPhiMVector> &met,
                      float weight,
                      const std::string &shift) -> void
{
    h_invariant_mass[shift].Fill((lepton_1 + lepton_2).mass(), weight);
    h_sum_pt[shift].Fill(lepton_1.pt() + lepton_2.pt(), weight);
    if (met.size() > 0)
    {
        h_met[shift].Fill(met[0].pt(), weight);
    }
    h_lepton_1_pt[shift].Fill(lepton_1.pt(), weight);
    h_lepton_2_pt[shift].Fill(lepton_2.pt(), weight);
    h_lepton_1_eta[shift].Fill(lepton_1.eta(), weight);
    h_lepton_2_eta[shift].Fill(lepton_2.eta(), weight);
    h_lepton_1_phi[shift].Fill(lepton_1.phi(), weight);
    h_lepton_2_phi[shift].Fill(lepton_2.phi(), weight);
    if (jets.size() > 0 or bjets.size() > 0)
    {
        Math::PtEtaPhiMVector leading_jet = [&]() -> Math::PtEtaPhiMVector
        {
            if (jets.size() > 0 and not(bjets.size() > 0))
            {
                return jets[0];
            }
            if (not(jets.size() > 0) and bjets.size() > 0)
            {
                return bjets[0];
            }
            if ((jets[0]).pt() > (bjets[0]).pt())
            {
                return jets[0];
            }
            return bjets[0];
        }();
        h_lepton_1_jet_1_dPhi[shift].Fill(VectorUtil::DeltaPhi(lepton_1, leading_jet), weight);
        h_lepton_1_jet_1_dR[shift].Fill(VectorUtil::DeltaR(lepton_1, leading_jet), weight);
    }
    h_jet_multiplicity[shift].Fill(jets.size(), weight);
    h_bjet_multiplicity[shift].Fill(bjets.size(), weight);

    h_lepton_1_pt_eta[shift].Fill(lepton_1.pt(), lepton_1.eta(), weight);
    h_lepton_1_pt_phi[shift].Fill(lepton_1.pt(), lepton_1.phi(), weight);
}

auto ZToLepLepX::save_histo(TH1 histo) -> void
{
    histo.SetDirectory(output_file.get());
    histo.Write();
}

auto ZToLepLepX::save_histo(TH2 histo) -> void
{
    histo.SetDirectory(output_file.get());
    histo.Write();
}

auto ZToLepLepX::dump_outputs() -> void
{
    output_file->cd();

    for (auto &&shift : shifts)
    {

        // rebin energy-like histograms
        if (not(is_Z_mass_validation))
        {
            auto h_invariant_mass_rebinned = rebin_histogram(h_invariant_mass[shift], countMap);
            h_invariant_mass_rebinned->Sumw2();
            h_invariant_mass_rebinned->Scale(10., "width");
            h_invariant_mass_rebinned->SetDirectory(output_file.get());
            h_invariant_mass_rebinned->Write();
        }
        else
        {
            h_invariant_mass[shift].SetDirectory(output_file.get());
            h_invariant_mass[shift].Write();
        }

        auto h_sum_pt_rebinned = rebin_histogram(h_sum_pt[shift], countMap);
        h_sum_pt_rebinned->Sumw2();
        h_sum_pt_rebinned->Scale(min_bin_width, "width");
        h_sum_pt_rebinned->SetDirectory(output_file.get());
        h_sum_pt_rebinned->Write();

        auto h_met_rebinned = rebin_histogram(h_met[shift], countMap, "MET");
        h_met_rebinned->Sumw2();
        h_met_rebinned->Scale(min_bin_width, "width");
        h_met_rebinned->SetDirectory(output_file.get());
        h_met_rebinned->Write();

        auto h_lepton_1_pt_rebinned = rebin_histogram(h_lepton_1_pt[shift], countMap);
        h_lepton_1_pt_rebinned->Sumw2();
        h_lepton_1_pt_rebinned->Scale(min_bin_width, "width");
        h_lepton_1_pt_rebinned->SetDirectory(output_file.get());
        h_lepton_1_pt_rebinned->Write();

        auto h_lepton_2_pt_rebinned = rebin_histogram(h_lepton_2_pt[shift], countMap);
        h_lepton_2_pt_rebinned->Sumw2();
        h_lepton_2_pt_rebinned->Scale(min_bin_width, "width");
        h_lepton_2_pt_rebinned->SetDirectory(output_file.get());
        h_lepton_2_pt_rebinned->Write();

        h_lepton_1_eta[shift].SetDirectory(output_file.get());
        h_lepton_1_eta[shift].Write();

        h_lepton_2_eta[shift].SetDirectory(output_file.get());
        h_lepton_2_eta[shift].Write();

        h_lepton_1_phi[shift].SetDirectory(output_file.get());
        h_lepton_1_phi[shift].Write();

        h_lepton_2_phi[shift].SetDirectory(output_file.get());
        h_lepton_2_phi[shift].Write();

        h_lepton_1_jet_1_dPhi[shift].SetDirectory(output_file.get());
        h_lepton_1_jet_1_dPhi[shift].Write();

        h_lepton_1_jet_1_dR[shift].SetDirectory(output_file.get());
        h_lepton_1_jet_1_dR[shift].Write();

        h_jet_multiplicity[shift].SetDirectory(output_file.get());
        h_jet_multiplicity[shift].Write();

        h_bjet_multiplicity[shift].SetDirectory(output_file.get());
        h_bjet_multiplicity[shift].Write();

        h_lepton_1_pt_eta[shift].SetDirectory(output_file.get());
        h_lepton_1_pt_eta[shift].Write();

        h_lepton_1_pt_phi[shift].SetDirectory(output_file.get());
        h_lepton_1_pt_phi[shift].Write();
    }

    output_file->Close();
}
