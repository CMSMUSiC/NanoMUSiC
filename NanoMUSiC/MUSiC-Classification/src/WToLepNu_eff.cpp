#include "../include/WToLepNu_eff.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include "TEfficiency.h"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

#include "NanoEventClass.hpp"

WToLepNu_eff::WToLepNu_eff(const std::string &_analysis_name,
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

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_leptons_all");
    h_leptons_all = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_leptons_all.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_leptons_matched");
    h_leptons_matched = TH1F(histo_name.c_str(), histo_name.c_str(), limits_met.size() - 1, limits_met.data());
    h_leptons_matched.Sumw2();
}

auto WToLepNu_eff::fill_eff(const Math::PtEtaPhiMVector &lepton_1,
                            const RVec<Math::PtEtaPhiMVector> &met,
                            float weight,
                            bool &fake_tau) -> void
{

    h_leptons_all.Fill((lepton_1).pt());

    if (fake_tau == false)
    {
        h_leptons_matched.Fill((lepton_1).pt());
    }
}

auto WToLepNu_eff::dump_outputs_eff() -> void
{
    auto output_file = std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE"));
    output_file->cd();

    // h_leptons_all.Scale(min_bin_width, "width");
    h_leptons_all.SetDirectory(output_file.get());
    h_leptons_all.Write();

    // h_leptons_matched.Scale(min_bin_width, "width");
    h_leptons_matched.SetDirectory(output_file.get());
    h_leptons_matched.Write();

    output_file->Close();
}
