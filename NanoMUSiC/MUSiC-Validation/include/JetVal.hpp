#ifndef ZTOLEPLEPX
#define ZTOLEPLEPX

#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/Vector4D.h"
#include <TFile.h>
#include <TH1F.h>
#include <memory>
#include <optional>
#include <string_view>

using namespace ROOT;
using namespace ROOT::Math;

// Jet validation class
class JetVal
{
  public:
    std::unique_ptr<TFile> output_file;

    // histograms
    ADD_TH1F(h_2jet_invariant_mass, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_2jet_sum_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_met, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_1jet_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_1jet_eta, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_1jet_phi, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_2jet_1_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_2jet_2_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_2jet_1_eta, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_2jet_2_eta, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_2jet_1_phi, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_2jet_2_phi, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_jet_multiplicity, n_multiplicity_bins, min_multiplicity, max_multiplicity);
    ADD_TH1F(h_bjet_multiplicity, n_multiplicity_bins, min_multiplicity, max_multiplicity);
    ADD_TH1F(h_2jet_Z_invariant_mass, n_energy_bins, PDG::Z::Mass - 20., PDG::Z::Mass + 20.);

    // constructor
    JetVal(const std::string &output_path);

    // fill0: fill >=0jet plots
    // fill1(nBJet, nJet, met, weight)
    auto fill0(unsigned int nBJet, unsigned int nJet, std::optional<float> met, float weight) -> void;
    // fill1: fill >=1jet plots
    // fill1(jet_1, weight)
    auto fill1(Math::PtEtaPhiMVector jet_1, float weight) -> void;
    // fill2: fill >=2jet plots
    // fill2(jet_1, jet_2, weight)
    auto fill2(Math::PtEtaPhiMVector jet_1, Math::PtEtaPhiMVector jet_2, float weight) -> void;
    // fill Z resonance plot
    // fill2z: fill >=2jet plots
    // fill2z(jet_1, jet_2, weight)
    auto fill2z(Math::PtEtaPhiMVector jet_1, Math::PtEtaPhiMVector jet_2, float weight) -> void;

    // save histograms
    auto save_histo(TH1F &histo) -> void;

    // dump outputs
    auto dump_outputs() -> void;
};

#endif // !ZTOLEPLEPX