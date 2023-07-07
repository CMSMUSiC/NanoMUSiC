#ifndef DIJETS
#define DIJETS

#include "Histograms.hpp"
#include "Math/Vector4D.h"
#include "TEfficiency.h"
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <memory>
#include <optional>
#include <string_view>

using namespace ROOT;
using namespace ROOT::Math;

class Dijets
{
  public:
    std::unique_ptr<TFile> output_file;

    // histograms
    ADD_TH1F(h_invariant_mass, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_sum_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_met, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_jet_1_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_jet_2_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_jet_1_eta, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_jet_2_eta, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_jet_1_phi, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_jet_2_phi, n_phi_bins, min_phi, max_phi);

    ADD_TH1F(h_dr_jet_1_jet_2, n_dR_bins, min_dR, max_dR);
    ADD_TH1F(h_jet_1_jet_2_dPhi, n_dR_bins, min_dR, max_dR);

    // ADD_TH1F(h_jet_1_jet_1_dPhi, n_phi_bins, min_phi, max_phi);
    // ADD_TH1F(h_jet_1_jet_1_dR, n_dR_bins, min_dR, max_dR);
    // ADD_TH1F(h_jet_multiplicity, n_multiplicity_bins, min_multiplicity, max_multiplicity);
    // ADD_TH1F(h_bjet_multiplicity, n_multiplicity_bins, min_multiplicity, max_multiplicity);

    TH2F h_dr_mass = TH2F("h_dr_mass", "h_dr_mass", n_dR_bins, min_dR, max_dR, 100, 0, 8000);
    TH2F h_dr_sum_pt = TH2F("h_dr_sum_pt", "h_dr_sum_pt", n_dR_bins, min_dR, max_dR, 100, 0, 6000);
    TH2F h_dr_pt1 = TH2F("h_dr_pt1", "h_dr_pt1", n_dR_bins, min_dR, max_dR, 80, 0, 3000);
    TH2F h_eta1_mass = TH2F("h_eta1_mass", "h_eta1_mass", n_eta_bins, min_eta, max_eta, 80, 0, 3000);

    double min_bin_width = 10;

    Dijets(const std::string &output_path, const std::map<std::string, int> &countMap);

    auto fill(const Math::PtEtaPhiMVector &jet_1,
              const Math::PtEtaPhiMVector &jet_2,
              //   unsigned int nBJet,
              //   std::optional<Math::PtEtaPhiMVector> bjet,
              //   unsigned int nJet,
              //   std::optional<Math::PtEtaPhiMVector> jet,
              std::optional<float> met,
              float weight = 1.) -> void;

    auto save_histo(TH1F &histo) -> void;
    auto save_histo(TH2F &histo) -> void;

    auto dump_outputs() -> void;
};

#endif // !DIJETS