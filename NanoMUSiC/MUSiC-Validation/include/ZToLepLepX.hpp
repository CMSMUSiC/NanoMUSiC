#ifndef ZTOLEPLEPX
#define ZTOLEPLEPX

#include "Histograms.hpp"
#include <TFile.h>
#include <TH1F.h>
#include <memory>
#include <string_view>

class ZToLepLepX
{
  public:
    std::unique_ptr<TFile> output_file;

    // histograms
    ADD_TH1F(h_invariant_mass, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_sum_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_met, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_lepton_1_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_lepton_2_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_lepton_1_eta, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_lepton_2_eta, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_lepton_1_phi, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_lepton_2_phi, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_lepton_1_jet_1_dPhi, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_lepton_1_jet_1_dR, n_dR_bins, min_dR, max_dR);
    ADD_TH1F(h_jet_multiplicity, n_multiplicity_bins, min_multiplicity, max_multiplicity);
    ADD_TH1F(h_bjet_multiplicity, n_multiplicity_bins, min_multiplicity, max_multiplicity);

    ZToLepLepX(const std::string &output_path);

    auto fill(float lepton_1_pt,
              float lepton_1_eta,
              float lepton_1_phi,
              float lepton_2_pt,
              float lepton_2_eta,
              float lepton_2_phi,
              int n_bjets,
              float bjet_1_pt,
              float bjet_1_eta,
              float bjet_1_phi,
              float bjet_2_pt,
              float bjet_2_eta,
              float bjet_2_phi,
              int n_jets,
              float jet_1_pt,
              float jet_1_eta,
              float jet_1_phi,
              float jet_2_pt,
              float jet_2_eta,
              float jet_2_phi,
              float met,
              float weight = 1.) -> void;

    auto dump_outputs() -> void;
};

#endif // !ZTOLEPLEPX