#include "TTBarTo1Lep2Bjet2JetMET.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include "NanoEventClass.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

TTBarTo1Lep2Bjet2JetMET::TTBarTo1Lep2Bjet2JetMET(const std::string &_analysis_name,
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
    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_invariant_mass_jet0_jet1");
    h_invariant_mass_jet0_jet1 = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_invariant_mass_jet0_jet1.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_transverse_mass_lep_MET");
    h_transverse_mass_lep_MET = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_transverse_mass_lep_MET.Sumw2();

    histo_name = NanoEventClass::make_histogram_full_name(_analysis_name, //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_ht_had_lep");
    h_ht_had_lep = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_ht_had_lep.Sumw2();
}

auto TTBarTo1Lep2Bjet2JetMET::fill(Math::PtEtaPhiMVector lep,
                                   Math::PtEtaPhiMVector jet0,
                                   Math::PtEtaPhiMVector jet1,
                                   Math::PtEtaPhiMVector bjet0,
                                   Math::PtEtaPhiMVector bjet1,
                                   Math::PtEtaPhiMVector met,
                                   float weight) -> void
{

    if ((lep.Mt() + met.Pt()) == 60)
    {
        h_invariant_mass_jet0_jet1.Fill((jet0 + jet1).mass(), weight);
    }

    if (((jet0 + jet1).mass() < (PDG::W::Mass + 30)) and ((jet0 + jet1).mass() > (PDG::W::Mass - 30)))
    {
        h_transverse_mass_lep_MET.Fill((lep + met).Mt(), weight);
    }

    if (((lep.Mt() + met.Pt()) == 60) and ((jet0 + jet1).mass() < (PDG::W::Mass + 30)) and
        ((jet0 + jet1).mass() > (PDG::W::Mass - 30)))
    {
        h_ht_had_lep.Fill((jet0 + jet1 + bjet0 + bjet1).mass());
    }
}

auto TTBarTo1Lep2Bjet2JetMET::dump_outputs(std::unique_ptr<TFile> &output_file) -> void
{
    // auto output_file = std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE"));
    output_file->cd();

    // h_invariant_mass_jet0_jet1.Scale(min_bin_width, "width");
    h_invariant_mass_jet0_jet1.SetDirectory(output_file.get());
    h_invariant_mass_jet0_jet1.Write();

    // h_transverse_mass_lep_MET.Scale(min_bin_width, "width");
    h_transverse_mass_lep_MET.SetDirectory(output_file.get());
    h_transverse_mass_lep_MET.Write();

    // h_ht_had_lep.Scale(min_bin_width, "width");
    h_ht_had_lep.SetDirectory(output_file.get());
    h_ht_had_lep.Write();

    // output_file->Close();
}