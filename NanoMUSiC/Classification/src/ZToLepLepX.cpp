#include "ZToLepLepX.hpp"
#include "EventClass.hpp"
#include "Math/VectorUtil.h"
#include "SerializationUtils.hpp"
#include <cstdlib>
#include <fmt/format.h>
#include <memory>

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
        analysis_name = around_to_Z_mass ? "z_to_muon_muon_x_z_mass" : "z_to_muon_muon_x";

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
        analysis_name = around_to_Z_mass ? "z_to_electron_electron_x_z_mass" : "z_to_electron_electron_x";

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
        analysis_name = around_to_Z_mass ? "z_to_tau_tau_x_z_mass" : "z_to_tau_tau_x";

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

    std::vector<double> bin_limits_multiplicity = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<double> bins_limits_dR = {0,   0.3, 0.6, 0.9, 1.2, 1.5, 1.8, 2.1, 2.4, 2.7, 3,
                                          3.3, 3.6, 3.9, 4.2, 4.5, 4.8, 5.1, 5.4, 5.7, 6,   6.3,
                                          6.6, 6.9, 7.2, 7.5, 7.8, 8.1, 8.4, 8.7, 9};
    std::vector<double> bin_limits_eta = {-1.5, -1.25, -1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1, 1.25, 1.5};
    std::vector<double> bin_limits_phi = {-3.25, -3,    -2.75, -2.5,  -2.25, -2,   -1.75, -1.5, -1.25,
                                          -1,    -0.75, -0.5,  -0.25, 0,     0.25, 0.5,   0.75, 1,
                                          1.25,  1.5,   1.75,  2,     2.25,  2.5,  2.75,  3,    3.25};

    count_map[ObjectNames::MET] = 1;
    auto bins_limits_MET = BinLimits::limits(
        count_map, true, Histograms::min_energy, Histograms::max_energy, Histograms::min_bin_size, Histograms::fudge);

    // Loop over all variations
    for (std::size_t idx_var = 0; idx_var < total_variations; idx_var++)
    {
        std::string histo_name = "";
        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_invariant_mass");

        h_invariant_mass[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_invariant_mass[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_met");

        h_met[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits_MET.size() - 1, bins_limits_MET.data());
        h_met[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_sum_pt");
        h_sum_pt[idx_var] = TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_sum_pt[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_1_pt");
        h_lepton_1_pt[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_lepton_1_pt[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_2_pt");
        h_lepton_2_pt[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_lepton_2_pt[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_1_eta");
        h_lepton_1_eta[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bin_limits_eta.size() - 1, bin_limits_eta.data());
        h_lepton_1_eta[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_2_eta");
        h_lepton_2_eta[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bin_limits_eta.size() - 1, bin_limits_eta.data());
        h_lepton_2_eta[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_1_phi");
        h_lepton_1_phi[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bin_limits_phi.size() - 1, bin_limits_phi.data());
        h_lepton_1_phi[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_2_phi");
        h_lepton_2_phi[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bin_limits_phi.size() - 1, bin_limits_phi.data());
        h_lepton_2_phi[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_1_jet_1_dPhi");
        h_lepton_1_jet_1_dPhi[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bin_limits_phi.size() - 1, bin_limits_phi.data());
        h_lepton_1_jet_1_dPhi[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_1_jet_1_dR");
        h_lepton_1_jet_1_dR[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits_dR.size() - 1, bins_limits_dR.data());
        h_lepton_1_jet_1_dR[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_jet_multiplicity");
        h_jet_multiplicity[idx_var] = TH1F(
            histo_name.c_str(), histo_name.c_str(), bin_limits_multiplicity.size() - 1, bin_limits_multiplicity.data());
        h_jet_multiplicity[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_bjet_multiplicity");
        h_bjet_multiplicity[idx_var] = TH1F(
            histo_name.c_str(), histo_name.c_str(), bin_limits_multiplicity.size() - 1, bin_limits_multiplicity.data());
        h_bjet_multiplicity[idx_var].Sumw2();
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

        h_sum_pt[idx_var].Fill((leptons.p4.at(0) + leptons.p4.at(1)).pt(), weight);
        h_lepton_1_pt[idx_var].Fill(leptons.p4.at(0).pt(), weight);
        h_lepton_2_pt[idx_var].Fill(leptons.p4.at(1).pt(), weight);
        h_lepton_1_eta[idx_var].Fill(leptons.p4.at(0).eta(), weight);
        h_lepton_2_eta[idx_var].Fill(leptons.p4.at(1).eta(), weight);
        h_lepton_1_phi[idx_var].Fill(leptons.p4.at(0).phi(), weight);
        h_lepton_2_phi[idx_var].Fill(leptons.p4.at(1).phi(), weight);

        if (jets.size() > 0 or bjets.size() > 0)
        {
            Math::PtEtaPhiMVector leading_jet = [&]() -> Math::PtEtaPhiMVector
            {
                if (jets.size() > 0 and not(bjets.size() > 0))
                {
                    return jets.p4.at(0);
                }
                if (not(jets.size() > 0) and bjets.size() > 0)
                {
                    return bjets.p4.at(0);
                }
                if (jets.p4.at(0).pt() > bjets.p4.at(0).pt())
                {
                    return jets.p4.at(0);
                }
                return bjets.p4.at(0);
            }();

            h_lepton_1_jet_1_dPhi[idx_var].Fill(VectorUtil::DeltaPhi(leptons.p4.at(0), leading_jet), weight);
            h_lepton_1_jet_1_dR[idx_var].Fill(VectorUtil::DeltaR(leptons.p4.at(0), leading_jet), weight);
        }

        h_jet_multiplicity[idx_var].Fill(jets.size(), weight);
        h_bjet_multiplicity[idx_var].Fill(bjets.size(), weight);
    }
}

auto ZToLepLepX::serialize_to_root(const std::unique_ptr<TFile> &output_file) -> void
{
    for (auto &hist : {h_invariant_mass,
                       h_met,
                       h_sum_pt,
                       h_lepton_1_pt,
                       h_lepton_2_pt,
                       h_lepton_1_eta,
                       h_lepton_2_eta,
                       h_lepton_1_phi,
                       h_lepton_2_phi,
                       h_lepton_1_jet_1_dPhi,
                       h_lepton_1_jet_1_dR,
                       h_jet_multiplicity,
                       h_bjet_multiplicity})
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
    MERGE(h_sum_pt)
    MERGE(h_lepton_1_pt)
    MERGE(h_lepton_2_pt)
    MERGE(h_lepton_1_eta)
    MERGE(h_lepton_2_eta)
    MERGE(h_lepton_1_phi)
    MERGE(h_lepton_2_phi)
    MERGE(h_lepton_1_jet_1_dPhi)
    MERGE(h_lepton_1_jet_1_dR)
    MERGE(h_jet_multiplicity)
    MERGE(h_bjet_multiplicity)
}
