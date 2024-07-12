#include "WToLepNuX.hpp"
#include <cstdlib>
#include <fmt/format.h>
#include <memory>

#include "EventClass.hpp"
#include "SerializationUtils.hpp"

WToLepNuX::WToLepNuX(enum Leptons lepton,
                     const std::string &process_group,
                     const std::string &xs_order,
                     const std::string &sample,
                     const std::string &year)
    : lepton(lepton)
{
    TH1::AddDirectory(kFALSE);

    auto count_map = std::unordered_map<ObjectNames, int>{};
    if (lepton == Leptons::MUONS)
    {
        analysis_name = "w_to_muon_neutrino_x";

        count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 1},
                                                         {ObjectNames::Electron, 0},
                                                         {ObjectNames::Photon, 0},
                                                         {ObjectNames::Tau, 0},
                                                         {ObjectNames::bJet, 0},
                                                         {ObjectNames::Jet, 0},
                                                         {ObjectNames::MET, 1}};
    }
    else if (lepton == Leptons::ELECTRONS)
    {
        analysis_name = "w_to_electron_neutrino_x";

        count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 0},
                                                         {ObjectNames::Electron, 1},
                                                         {ObjectNames::Photon, 0},
                                                         {ObjectNames::Tau, 0},
                                                         {ObjectNames::bJet, 0},
                                                         {ObjectNames::Jet, 0},
                                                         {ObjectNames::MET, 1}};
    }
    else if (lepton == Leptons::TAUS)
    {
        analysis_name = "w_to_tau_neutrino_x";

        count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 0},
                                                         {ObjectNames::Electron, 0},
                                                         {ObjectNames::Photon, 0},
                                                         {ObjectNames::Tau, 1},
                                                         {ObjectNames::bJet, 0},
                                                         {ObjectNames::Jet, 0},
                                                         {ObjectNames::MET, 1}};
    }
    else
    {

        fmt::print(stderr, "ERROR: Could not set analysis name and bim limits. Lepton flavor not found.\n");
        std::exit(EXIT_FAILURE);
    }

    auto bins_limits = BinLimits::limits(
        count_map, true, Histograms::min_energy, Histograms::max_energy, Histograms::min_bin_size, Histograms::fudge);

    std::vector<double> bins_limits_eta = {-1.5, -1.25, -1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1, 1.25, 1.5};
    std::vector<double> bins_limits_phi = {-3.25, -3,    -2.75, -2.5,  -2.25, -2,   -1.75, -1.5, -1.25,
                                           -1,    -0.75, -0.5,  -0.25, 0,     0.25, 0.5,   0.75, 1,
                                           1.25,  1.5,   1.75,  2,     2.25,  2.5,  2.75,  3,    3.25};

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
                                                                  "h_transverse_mass");

        h_transverse_mass[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_transverse_mass[idx_var].Sumw2();

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
                                                                  "h_lepton_pt");

        h_lepton_pt[idx_var] = TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_lepton_pt[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_eta");

        h_lepton_eta[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits_eta.size() - 1, bins_limits_eta.data());
        h_lepton_eta[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_lepton_phi");

        h_lepton_phi[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits_phi.size() - 1, bins_limits_phi.data());
        h_lepton_phi[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_met");

        h_met[idx_var] = TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_met[idx_var].Sumw2();
    }
}

auto WToLepNuX::fill(const MUSiCObjects &leptons,
                     const MUSiCObjects &bjets,
                     const MUSiCObjects &jets,
                     const MUSiCObjects &met,
                     double weight,
                     Shifts::Variations shift) -> void
{
    if (leptons.size() >= 1 and met.size() >= 1)
    {
        auto idx_var = static_cast<std::size_t>(shift);

        h_transverse_mass[idx_var].Fill(std::sqrt(std::pow(leptons.p4.at(0).px() + met.p4.at(0).px(), 2) +
                                                  std::pow(leptons.p4.at(0).py() + met.p4.at(0).py(), 2)),
                                        weight);
        h_sum_pt[idx_var].Fill((leptons.p4.at(0) + met.p4.at(0)).pt(), weight);
        h_lepton_pt[idx_var].Fill((leptons.p4.at(0)).pt(), weight);
        h_lepton_eta[idx_var].Fill((leptons.p4.at(0)).eta(), weight);
        h_lepton_phi[idx_var].Fill((leptons.p4.at(0)).phi(), weight);
        h_met[idx_var].Fill(met.p4.at(0).pt(), weight);
    }
}

auto WToLepNuX::serialize_to_root(const std::unique_ptr<TFile> &output_file) -> void
{
    for (auto &hist : {h_transverse_mass, h_sum_pt, h_lepton_pt, h_lepton_eta, h_lepton_phi, h_met})
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

auto WToLepNuX::merge_inplace(const WToLepNuX &other) -> void
{
    MERGE(h_transverse_mass)
    MERGE(h_sum_pt)
    MERGE(h_lepton_pt)
    MERGE(h_lepton_eta)
    MERGE(h_lepton_phi)
    MERGE(h_met)
}
