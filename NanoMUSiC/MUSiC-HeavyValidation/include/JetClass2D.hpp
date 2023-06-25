#ifndef JETCLASSVAL2D
#define JETCLASSVAL2D

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
class JetClass2D
{
  public:
    std::unique_ptr<TFile> output_file;
    std::string c_name;
    unsigned int c_nJet = 0;
    unsigned int c_nBJet = 0;
    unsigned int c_nMET = 0;

    // histograms
    ADD_TH1F(h_deltar_jetjet, n_dR_bins, min_dR, max_dR);
    ADD_TH1F(h_deltar_jetbjet, n_dR_bins, min_dR, max_dR);
    ADD_TH1F(h_deltar_bjetbjet, n_dR_bins, min_dR, max_dR);
    ADD_TH1F(h_deltaphi_jetjet, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_deltaphi_jetbjet, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_deltaphi_bjetbjet, n_phi_bins, min_phi, max_phi);
    ADD_TH1F(h_deltaeta_jetjet, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_deltaeta_jetbjet, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_deltaeta_bjetbjet, n_eta_bins, min_eta, max_eta);
    ADD_TH1F(h_nmuon, n_multiplicity_bins, min_multiplicity, max_multiplicity);

    // constructor
    JetClass2D(const std::string &output_path, const std::string c_name);

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