#include "ZToLepLepX.hpp"
#include <cstdlib>
#include <fmt/format.h>
#include <memory>

#include "EventClass.hpp"
#include "NanoEventClass.hpp"

ZToLepLepX::ZToLepLepX(enum Leptons lepton,
                       bool around_to_Z_mass,
                       const std::string &process_group,
                       const std::string &xs_order,
                       const std::string &sample,
                       const std::string &year)
    : lepton(lepton),
      around_to_Z_mass(around_to_Z_mass)
{
    TH1::AddDirectory(kFALSE);

    auto count_map = std::unordered_map<ObjectNames, int>{};
    if (lepton == Leptons::MUONS)
    {
        analysis_name = around_to_Z_mass ? "z_to_muon_muon_x" : "z_to_muon_muon_x_z_mass";

        count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 2},
                                                         {ObjectNames::Electron, 0},
                                                         {ObjectNames::Photon, 0},
                                                         {ObjectNames::Tau, 0},
                                                         {ObjectNames::bJet, 0},
                                                         {ObjectNames::Jet, 0},
                                                         {ObjectNames::MET, 0}};
    }
    else if (lepton == Leptons::ELECTRONS)
    {
        analysis_name = around_to_Z_mass ? "z_to_electron_electron_x" : "z_to_electron_electron_x_z_mass";

        count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 0},
                                                         {ObjectNames::Electron, 2},
                                                         {ObjectNames::Photon, 0},
                                                         {ObjectNames::Tau, 0},
                                                         {ObjectNames::bJet, 0},
                                                         {ObjectNames::Jet, 0},
                                                         {ObjectNames::MET, 0}};
    }
    else if (lepton == Leptons::TAUS)
    {
        analysis_name = around_to_Z_mass ? "z_to_tau_tau_x" : "z_to_tau_tau_x_z_mass";

        count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 0},
                                                         {ObjectNames::Electron, 0},
                                                         {ObjectNames::Photon, 0},
                                                         {ObjectNames::Tau, 2},
                                                         {ObjectNames::bJet, 0},
                                                         {ObjectNames::Jet, 0},
                                                         {ObjectNames::MET, 0}};
    }
    else
    {

        fmt::print(stderr, "ERROR: Could not set analysis name and bim limits. Lepton flavor not found.\n");
        std::exit(EXIT_FAILURE);
    }

    auto bins_limits = BinLimits::limits(
        count_map, false, Histograms::min_energy, Histograms::max_energy, Histograms::min_bin_size, Histograms::fudge);
    if (around_to_Z_mass)
    {
        bins_limits =
            BinLimits::limits(count_map, false, PDG::Z::Mass - 20., PDG::Z::Mass + 20., 1., Histograms::fudge);
    }

    count_map[ObjectNames::MET] = 1;
    auto bins_limits_MET = BinLimits::limits(
        count_map, true, Histograms::min_energy, Histograms::max_energy, Histograms::min_bin_size, Histograms::fudge);

    // Loop over all variations
    for (std::size_t idx_var = 0; idx_var < total_variations; idx_var++)
    {
        std::string histo_name = "";
        histo_name = NanoEventHisto::make_histogram_full_name(analysis_name,                        //
                                                              process_group,                        //
                                                              xs_order,                             //
                                                              sample,                               //
                                                              year,                                 //
                                                              Shifts::variation_to_string(idx_var), //
                                                              "h_invariant_mass");

        h_invariant_mass[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_invariant_mass[idx_var].Sumw2();

        histo_name = NanoEventHisto::make_histogram_full_name(analysis_name,                        //
                                                              process_group,                        //
                                                              xs_order,                             //
                                                              sample,                               //
                                                              year,                                 //
                                                              Shifts::variation_to_string(idx_var), //
                                                              "h_met");

        h_met[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits_MET.size() - 1, bins_limits_MET.data());
        h_met[idx_var].Sumw2();

        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_sum_pt");
        // h_sum_pt = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
        // h_sum_pt.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_met");
        // h_met = TH1F(histo_name.c_str(), histo_name.c_str(), limits_met.size() - 1, limits_met.data());
        // h_met.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_1_pt");
        // h_lepton_1_pt = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
        // h_lepton_1_pt.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_2_pt");
        // h_lepton_2_pt = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
        // h_lepton_2_pt.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_1_eta");
        // h_lepton_1_eta = TH1F(histo_name.c_str(), histo_name.c_str(), n_eta_bins, min_eta, max_eta);
        // h_lepton_1_eta.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_2_eta");
        // h_lepton_2_eta = TH1F(histo_name.c_str(), histo_name.c_str(), n_eta_bins, min_eta, max_eta);
        // h_lepton_2_eta.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_1_phi");
        // h_lepton_1_phi = TH1F(histo_name.c_str(), histo_name.c_str(), n_phi_bins, min_phi, max_phi);
        // h_lepton_1_phi.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_2_phi");
        // h_lepton_2_phi = TH1F(histo_name.c_str(), histo_name.c_str(), n_phi_bins, min_phi, max_phi);
        // h_lepton_2_phi.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_1_jet_1_dPhi");
        // h_lepton_1_jet_1_dPhi = TH1F(histo_name.c_str(), histo_name.c_str(), n_phi_bins, min_phi, max_phi);
        // h_lepton_1_jet_1_dPhi.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_1_jet_1_dR");
        // h_lepton_1_jet_1_dR = TH1F(histo_name.c_str(), histo_name.c_str(), n_dR_bins, min_dR, max_dR);
        // h_lepton_1_jet_1_dR.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_jet_multiplicity");
        // h_jet_multiplicity =
        //     TH1F(histo_name.c_str(), histo_name.c_str(), n_multiplicity_bins, min_multiplicity, max_multiplicity);
        // h_jet_multiplicity.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_bjet_multiplicity");
        // h_bjet_multiplicity =
        //     TH1F(histo_name.c_str(), histo_name.c_str(), n_multiplicity_bins, min_multiplicity, max_multiplicity);
        // h_bjet_multiplicity.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_1_pt_eta");
        // h_lepton_1_pt_eta = TH2F(histo_name.c_str(),
        //                          histo_name.c_str(),
        //                          130,
        //                          min_energy,
        //                          900,
        //                          n_multiplicity_bins,
        //                          min_multiplicity,
        //                          max_multiplicity);
        // h_lepton_1_pt_eta.Sumw2();
        //
        // histo_name = NanoEventHisto::make_histogram_full_name(_analysis_name, //
        //                                                       _process_group, //
        //                                                       _xs_order,      //
        //                                                       _sample,        //
        //                                                       _year,          //
        //                                                       _shift,         //
        //                                                       "h_lepton_1_pt_phi");
        // h_lepton_1_pt_phi =
        //     TH2F(histo_name.c_str(), histo_name.c_str(), 130, min_energy, 900, n_phi_bins, min_phi, max_phi);
        // h_lepton_1_pt_phi.Sumw2();
    }
}

auto ZToLepLepX::fill(const MUSiCObjects &leptons,
                      const MUSiCObjects &bjets,
                      const MUSiCObjects &jets,
                      const MUSiCObjects &met,
                      double weight,
                      Shifts::Variations shift) -> void
{
    if (leptons.size() >= 2)
    {
        auto idx_var = static_cast<std::size_t>(shift);

        h_invariant_mass[idx_var].Fill((leptons.p4.at(0) + leptons.p4.at(1)).mass(), weight);

        if (met.size() > 0)
        {
            h_met[idx_var].Fill(met.p4.at(0).pt(), weight);
        }

        // h_sum_pt.Fill(lepton_1.pt() + lepton_2.pt(), weight);
        // h_lepton_1_pt.Fill(lepton_1.pt(), weight);
        // h_lepton_2_pt.Fill(lepton_2.pt(), weight);
        // h_lepton_1_eta.Fill(lepton_1.eta(), weight);
        // h_lepton_2_eta.Fill(lepton_2.eta(), weight);
        // h_lepton_1_phi.Fill(lepton_1.phi(), weight);
        // h_lepton_2_phi.Fill(lepton_2.phi(), weight);
        //
        // if (jets.size() > 0 or bjets.size() > 0)
        // {
        //     Math::PtEtaPhiMVector leading_jet = [&]() -> Math::PtEtaPhiMVector
        //     {
        //         if (jets.size() > 0 and not(bjets.size() > 0))
        //         {
        //             return jets[0];
        //         }
        //         if (not(jets.size() > 0) and bjets.size() > 0)
        //         {
        //             return bjets[0];
        //         }
        //         if ((jets[0]).pt() > (bjets[0]).pt())
        //         {
        //             return jets[0];
        //         }
        //         return bjets[0];
        //     }();
        //
        //     h_lepton_1_jet_1_dPhi.Fill(VectorUtil::DeltaPhi(lepton_1, leading_jet), weight);
        //     h_lepton_1_jet_1_dR.Fill(VectorUtil::DeltaR(lepton_1, leading_jet), weight);
        // }
        //
        // h_jet_multiplicity.Fill(jets.size(), weight);
        // h_bjet_multiplicity.Fill(bjets.size(), weight);
        //
        // h_lepton_1_pt_eta.Fill(lepton_1.pt(), lepton_1.eta(), weight);
        // h_lepton_1_pt_phi.Fill(lepton_1.pt(), lepton_1.phi(), weight);
    }
}

auto ZToLepLepX::serialize_to_root(const std::unique_ptr<TFile> &output_file) -> void
{
    for (auto &hist : {h_invariant_mass, h_met})
    {
        for (std::size_t idx_var = 0; idx_var < total_variations; idx_var++)
        {
            output_file->WriteObject(&hist[idx_var], hist[idx_var].GetName());
        }
    }
}

#define MERGE(h)                                                                                                       \
    for (std::size_t idx_var = 0; idx_var < total_variations; idx_var++)                                               \
    {                                                                                                                  \
        h[idx_var].Add(&other.h[idx_var]);                                                                             \
    }

auto ZToLepLepX::merge_inplace(const ZToLepLepX &other) -> void
{
    MERGE(h_invariant_mass)
    MERGE(h_met)
}
