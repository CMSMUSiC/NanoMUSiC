#include "../include/TauGammaX.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include "TEfficiency.h"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

#include "NanoEventClass.hpp"

TauGammaX::TauGammaX(const std::string &_analysis_name,
                   const std::string &_output_path,
                   const std::map<std::string, int> &_countMap,
                   bool _is_Z_mass_validation,
                   const std::string _shift,
                   const std::string &_sample,
                   const std::string &_year,
                   const std::string &_process_group,
                   const std::string &_xs_order)
    : output_path(_output_path),
      min_bin_width(10.),
      countMap(_countMap),
      is_Z_mass_validation(_is_Z_mass_validation),
      shift(_shift)
{
    std::vector<double> limits =
        BinLimits::get_bin_limits("validation_plot", countMap, min_energy, max_energy, min_bin_width, 1);
    std::vector<double> limits_Z_val =
        BinLimits::get_bin_limits("validation_plot", countMap, PDG::Z::Mass - 20., PDG::Z::Mass + 20., 1, 1);
    std::vector<double> limits_met =
        BinLimits::get_bin_limits("MET", countMap, min_energy, max_energy, min_bin_width, 1);

    std::string histo_name = "";
    if (is_Z_mass_validation)
    {

        histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                              _process_group, //
                                                              _xs_order,      //
                                                              _sample,        //
                                                              _year,          //
                                                              _shift,         //
                                                              "h_invariant_mass");
        h_invariant_mass = TH1F(histo_name.c_str(), histo_name.c_str(), limits_Z_val.size() - 1, limits_Z_val.data());
        h_invariant_mass.Sumw2();
    }
    else
    {
        histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                              _process_group, //
                                                              _xs_order,      //
                                                              _sample,        //
                                                              _year,          //
                                                              _shift,         //
                                                              "h_invariant_mass");
        h_invariant_mass = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
        h_invariant_mass.Sumw2();
    }

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_sum_pt");
    h_sum_pt = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_sum_pt.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_met");
    h_met = TH1F(histo_name.c_str(), histo_name.c_str(), limits_met.size() - 1, limits_met.data());
    h_met.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_tau_pt");
    h_tau_pt = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_tau_pt.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_gamma_pt");
    h_gamma_pt = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_gamma_pt.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_tau_eta");
    h_tau_eta = TH1F(histo_name.c_str(), histo_name.c_str(), n_eta_bins, min_eta, max_eta);
    h_tau_eta.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_gamma_eta");
    h_gamma_eta = TH1F(histo_name.c_str(), histo_name.c_str(), n_eta_bins, min_eta, max_eta);
    h_gamma_eta.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_tau_phi");
    h_tau_phi = TH1F(histo_name.c_str(), histo_name.c_str(), n_phi_bins, min_phi, max_phi);
    h_tau_phi.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_gamma_phi");
    h_gamma_phi = TH1F(histo_name.c_str(), histo_name.c_str(), n_phi_bins, min_phi, max_phi);
    h_gamma_phi.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_tau_pt_eta");
    h_tau_pt_eta = TH2F(histo_name.c_str(),
                             histo_name.c_str(),
                             130,
                             min_energy,
                             900,
                             n_multiplicity_bins,
                             min_multiplicity,
                             max_multiplicity);
    h_tau_pt_eta.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_tau_pt_phi");
    h_tau_pt_phi =
        TH2F(histo_name.c_str(), histo_name.c_str(), 130, min_energy, 900, n_phi_bins, min_phi, max_phi);
    h_tau_pt_phi.Sumw2();
}

auto TauGammaX::fill(const Math::PtEtaPhiMVector &tau,
                    const RVec<Math::PtEtaPhiMVector> &met,
                    const Math::PtEtaPhiMVector &photon,
                    float weight) -> void
{
    h_invariant_mass.Fill((tau + photon).mass(), weight);
    h_sum_pt.Fill(tau.pt() + photon.pt(), weight);

    if (met.size() > 0)
    {
        h_met.Fill(met[0].pt(), weight);
    }

    h_tau_pt.Fill(tau.pt(), weight);
    h_gamma_pt.Fill(photon.pt(), weight);
    h_tau_eta.Fill(tau.eta(), weight);
    h_gamma_eta.Fill(photon.eta(), weight);
    h_tau_phi.Fill(tau.phi(), weight);
    h_gamma_phi.Fill(photon.phi(), weight);

    h_tau_pt_eta.Fill(tau.pt(), tau.eta(), weight);
    h_tau_pt_phi.Fill(tau.pt(), tau.phi(), weight);
}

auto TauGammaX::dump_outputs() -> void
{

    auto output_file = std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE"));
    output_file->cd();

    // rebin energy-like histograms
    if (not(is_Z_mass_validation))
    {
        h_invariant_mass.Scale(10., "width");
        h_invariant_mass.SetDirectory(output_file.get());
        h_invariant_mass.Write();
    }
    else
    {
        h_invariant_mass.Scale(10., "width");
        h_invariant_mass.SetDirectory(output_file.get());
        h_invariant_mass.Write();
    }

    h_sum_pt.Scale(min_bin_width, "width");
    h_sum_pt.SetDirectory(output_file.get());
    h_sum_pt.Write();

    h_met.Scale(min_bin_width, "width");
    h_met.SetDirectory(output_file.get());
    h_met.Write();

    h_tau_pt.Scale(min_bin_width, "width");
    h_tau_pt.SetDirectory(output_file.get());
    h_tau_pt.Write();

    h_gamma_pt.Scale(min_bin_width, "width");
    h_gamma_pt.SetDirectory(output_file.get());
    h_gamma_pt.Write();

    h_tau_eta.SetDirectory(output_file.get());
    h_tau_eta.Write();

    h_gamma_eta.SetDirectory(output_file.get());
    h_gamma_eta.Write();

    h_tau_phi.SetDirectory(output_file.get());
    h_tau_phi.Write();

    h_gamma_phi.SetDirectory(output_file.get());
    h_gamma_phi.Write();

    h_tau_pt_eta.SetDirectory(output_file.get());
    h_tau_pt_eta.Write();

    h_tau_pt_phi.SetDirectory(output_file.get());
    h_tau_pt_phi.Write();

    output_file->Close();
}
