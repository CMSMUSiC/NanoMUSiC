#include "GammaPlusJet.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include "NanoEventClass.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

GammaPlusJet::GammaPlusJet(const std::string &_analysis_name,
                           const std::string &_output_path,
                           const std::map<std::string, int> &_countMap,
                           bool dummy,
                           const std::string _shift,
                           const std::string &_sample,
                           const std::string &_year,
                           const std::string &_process_group,
                           const std::string &_xs_order)
    : output_path(_output_path),
      min_bin_width(10.),
      countMap(_countMap),
      shift(_shift)
{
    std::vector<double> limits =
        BinLimits::get_bin_limits("validation_plot", countMap, min_energy, max_energy, min_bin_width, 1);

    std::string histo_name = "";

    histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_gamma_pt");
    h_gamma_pt = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_gamma_pt.Sumw2();

    histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_gamma_eta");
    h_gamma_eta = TH1F(histo_name.c_str(), histo_name.c_str(), n_eta_bins, min_eta, max_eta);
    h_gamma_eta.Sumw2();

    histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_gamma_phi");
    h_gamma_phi = TH1F(histo_name.c_str(), histo_name.c_str(), n_phi_bins, min_phi, max_phi);
    h_gamma_phi.Sumw2();
}

auto GammaPlusJet::fill(Math::PtEtaPhiMVector gamma, float weight) -> void
{
    if (gamma.pt() > 200)
    {
        h_gamma_pt.Fill(gamma.pt(), weight);
        h_gamma_eta.Fill(gamma.eta(), weight);
        h_gamma_phi.Fill(gamma.phi(), weight);
    }
}

auto GammaPlusJet::dump_outputs(std::unique_ptr<TFile> &output_file) -> void
{
    // auto output_file = std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE"));
    output_file->cd();

    // h_gamma_pt.Scale(min_bin_width, "width");
    h_gamma_pt.SetDirectory(output_file.get());
    h_gamma_pt.Write();

    // h_gamma_eta.Scale(min_bin_width, "width");
    h_gamma_eta.SetDirectory(output_file.get());
    h_gamma_eta.Write();

    // h_gamma_phi.Scale(min_bin_width, "width");
    h_gamma_phi.SetDirectory(output_file.get());
    h_gamma_phi.Write();

    // output_file->Close();
}