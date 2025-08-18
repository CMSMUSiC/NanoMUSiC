#include "ZGammaGamma.hpp"
#include <cstdlib>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>

#include "EventClass.hpp"
#include "SerializationUtils.hpp"

ZGammaGamma::ZGammaGamma(const std::string &process_group,
               const std::string &xs_order,
               const std::string &sample,
               const std::string &year)
{
    TH1::AddDirectory(kFALSE);

    auto count_map = std::unordered_map<ObjectNames, int>{};
    analysis_name = "zgammagamma";

    count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 0},
                                                     {ObjectNames::Electron, 0},
                                                     {ObjectNames::Photon, 1}, // will be used only for leptons
                                                     {ObjectNames::Tau, 0},
                                                     {ObjectNames::bJet, 0},
                                                     {ObjectNames::Jet, 0},
                                                     {ObjectNames::MET, 0}};

    auto bins_limits = BinLimits::limits(
        count_map, false, Histograms::min_energy, Histograms::max_energy, Histograms::min_bin_size, Histograms::fudge);

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
                                                                  "h_deltaR_gamma_lepton");

        h_deltaR_gamma_lepton[idx_var] = TH1F(histo_name.c_str(), histo_name.c_str(), 50, 0., 5.);
        h_deltaR_gamma_lepton[idx_var].Sumw2();
    }
}

auto ZGammaGamma::fill(const MUSiCObjects &muons,
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

    if (photons.size() == 1
        // and (jets.size() + bjets.size()) == 1                                     //
        and (muons.size() >= 2 or electrons.size() >= 2 or taus.size() >= 2) == 0 //
        and met.size() == 0                                                       //
    )
    {
        RVec<Math::PtEtaPhiMVector> dileptons = {};

        if (muons.size() >= 2)
        {
            dileptons = {muons.p4.at(0), muons.p4.at(1)};
        }
        else if (electrons.size() >= 2)
        {
            dileptons = {electrons.p4.at(0), electrons.p4.at(1)};
        }
        else if (taus.size() >= 2)
        {
            dileptons = {taus.p4.at(0), taus.p4.at(1)};
        }
        else
        {
            throw std::runtime_error("ZGammaGamma Validation: invalid number of leptons");
        }

        auto z_cand = dileptons[0] + dileptons[1];
        if (70. <= z_cand.mass() and z_cand.mass() >= 110.)
        {
            h_deltaR_gamma_lepton[idx_var].Fill(Math::VectorUtil::DeltaR(photons.p4.at(0), dileptons[0]), weight);
            h_gamma_pt[idx_var].Fill(photons.p4.at(0).pt(), weight);
        }
    }
}

auto ZGammaGamma::serialize_to_root(const std::unique_ptr<TFile> &output_file) -> void
{
    for (auto &hist : {h_gamma_pt, h_deltaR_gamma_lepton})
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

auto ZGammaGamma::merge_inplace(const ZGammaGamma &other) -> void
{
    MERGE(h_gamma_pt)
    MERGE(h_deltaR_gamma_lepton)
}
