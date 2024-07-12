#include "GammaPlusJet.hpp"
#include <cstdlib>
#include <fmt/format.h>
#include <memory>

#include "EventClass.hpp"
#include "SerializationUtils.hpp"

GammaPlusJet::GammaPlusJet(const std::string &process_group,
                           const std::string &xs_order,
                           const std::string &sample,
                           const std::string &year)
{
    TH1::AddDirectory(kFALSE);

    auto count_map = std::unordered_map<ObjectNames, int>{};
    analysis_name = "gamma_plus_jets";

    count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 0},
                                                     {ObjectNames::Electron, 0},
                                                     {ObjectNames::Photon, 1},
                                                     {ObjectNames::Tau, 0},
                                                     {ObjectNames::bJet, 0},
                                                     {ObjectNames::Jet, 1},
                                                     {ObjectNames::MET, 0}};

    auto bins_limits = BinLimits::limits(
        count_map, false, Histograms::min_energy, Histograms::max_energy, Histograms::min_bin_size, Histograms::fudge);

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
                                                                  "h_gamma_pt");

        h_gamma_pt[idx_var] = TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_gamma_pt[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_gamma_eta");

        h_gamma_eta[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits_eta.size() - 1, bins_limits_eta.data());
        h_gamma_eta[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_gamma_phi");

        h_gamma_phi[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits_phi.size() - 1, bins_limits_phi.data());
        h_gamma_phi[idx_var].Sumw2();
    }
}

auto GammaPlusJet::fill(const MUSiCObjects &muons,
                        const MUSiCObjects &electrons,
                        const MUSiCObjects &taus,
                        const MUSiCObjects &photons,
                        const MUSiCObjects &bjets,
                        const MUSiCObjects &jets,
                        const MUSiCObjects &met,
                        double weight,
                        Shifts::Variations shift) -> void
{
    auto idx_var = static_cast<std::size_t>(shift);

    if (photons.size() == 1 and (jets.size() + bjets.size()) == 1 and
        (muons.size() + electrons.size() + taus.size()) == 0 and met.size() == 0)
    {
        if (photons.p4.at(0).pt() > 220.)
        {
            h_gamma_pt[idx_var].Fill(photons.p4.at(0).pt(), weight);
            h_gamma_eta[idx_var].Fill(photons.p4.at(0).eta(), weight);
            h_gamma_phi[idx_var].Fill(photons.p4.at(0).phi(), weight);
        }
    }
}

auto GammaPlusJet::serialize_to_root(const std::unique_ptr<TFile> &output_file) -> void
{
    for (auto &hist : {h_gamma_pt, h_gamma_eta, h_gamma_phi})
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

auto GammaPlusJet::merge_inplace(const GammaPlusJet &other) -> void
{
    MERGE(h_gamma_pt) MERGE(h_gamma_eta) MERGE(h_gamma_phi)
}
