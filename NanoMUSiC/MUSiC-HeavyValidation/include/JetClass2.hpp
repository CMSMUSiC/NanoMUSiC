#ifndef JETCLASSVAL2
#define JETCLASSVAL2

#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include <TFile.h>
#include <TH1F.h>
#include <memory>
#include <optional>
#include <string_view>

using namespace ROOT;
using namespace ROOT::Math;

// Jet classification validation
class JetClass2
{
  public:
    std::unique_ptr<TFile> output_file;
    std::string c_name;
    unsigned int c_nJet = 0;
    unsigned int c_nBJet = 0;
    unsigned int c_nMET = 0;

    // histograms
    ADD_TH1F(h_m_inv, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_m_tr, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_sum_pt, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_pt_met, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_pt_1st_jet, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_pt_2nd_jet, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_eta_1st_jet, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_eta_2nd_jet, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_phi_1st_jet, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_phi_2nd_jet, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_pt_1st_bjet, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_pt_2nd_bjet, n_energy_bins, min_energy, max_energy);
    ADD_TH1F(h_eta_1st_bjet, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_eta_2nd_bjet, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_phi_1st_bjet, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_phi_2nd_bjet, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_phi_met, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_njet, n_multiplicity_bins, min_multiplicity, max_multiplicity);
    ADD_TH1F(h_nbjet, n_multiplicity_bins, min_multiplicity, max_multiplicity);
    ADD_TH1F(h_nelectron, n_multiplicity_bins, min_multiplicity, max_multiplicity);
    ADD_TH1F(h_nmuon, n_multiplicity_bins, min_multiplicity, max_multiplicity);
    ADD_TH1F(h_nemu, n_multiplicity_bins, min_multiplicity, max_multiplicity);
    ADD_TH1F(h_deltar_jetjet, n_dR_bins, min_dR, max_dR);
    ADD_TH1F(h_deltar_jetbjet, n_dR_bins, min_dR, max_dR);
    ADD_TH1F(h_deltar_bjetbjet, n_dR_bins, min_dR, max_dR);

    // constructor
    JetClass2(const std::string &output_path, const std::string c_name);

    // fill histograms
    auto fill(RVec<Math::PtEtaPhiMVector> jets,
              RVec<Math::PtEtaPhiMVector> bjets,
              unsigned int nElectron,
              unsigned int nMuon,
              RVec<Math::PtEtaPhiMVector> met,
              float weight) -> void;

    // save histograms
    auto save_histo(TH1F &histo) -> void;

    // dump outputs
    auto dump_outputs() -> void;
};

#endif