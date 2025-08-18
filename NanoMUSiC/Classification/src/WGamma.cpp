#include "WGamma.hpp"
#include <cstdlib>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>

#include "EventClass.hpp"
#include "SerializationUtils.hpp"

WGamma::WGamma(const std::string &process_group,
               const std::string &xs_order,
               const std::string &sample,
               const std::string &year)
{
    TH1::AddDirectory(kFALSE);

    auto count_map = std::unordered_map<ObjectNames, int>{};
    analysis_name = "wgamma";

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

enum EWBosonChildren
{
    MUONS,
    ELECTRONS,
    TAUS,
    UNDEFINED,
};

inline auto transverse_mass(double et, double px, double py) -> double
{
    return std::sqrt(std::pow(et, 2) - std::pow(px, 2) - std::pow(py, 2));
}

auto WGamma::fill(const MUSiCObjects &muons,
                  const MUSiCObjects &electrons,
                  const MUSiCObjects &taus,
                  const MUSiCObjects &photons,
                  const MUSiCObjects &bjets,
                  const MUSiCObjects &jets,
                  const MUSiCObjects &met,
                  double weight,
                  Shifts::Variations shift) -> void
{
    constexpr double W_MASS = 75.; // approx. for typical tranverse mass

    auto idx_var = static_cast<std::size_t>(shift);

    if (photons.size() == 1
        // and (jets.size() + bjets.size()) == 1                                     //
        and (muons.size() >= 1 or electrons.size() >= 1 or taus.size() >= 1) //
        and met.size() == 1                                                  //
    )
    {

        std::optional<double> dimuon_mass = std::nullopt;
        if (muons.size() >= 1)
        {
            auto transv_mass_lep_met = transverse_mass(muons.p4.at(0).Et() + met.p4.at(0).Et(),
                                                       muons.p4.at(0).px() + met.p4.at(0).px(),
                                                       muons.p4.at(0).py() + met.p4.at(0).py());
            if (50. <= transv_mass_lep_met and transv_mass_lep_met <= 100.)
            {
                dimuon_mass = transv_mass_lep_met;
            }
        }

        std::optional<double> dielectron_mass = std::nullopt;
        if (electrons.size() >= 1)
        {
            auto transv_mass_lep_met = transverse_mass(electrons.p4.at(0).Et() + met.p4.at(0).Et(),
                                                       electrons.p4.at(0).px() + met.p4.at(0).px(),
                                                       electrons.p4.at(0).py() + met.p4.at(0).py());
            if (50. <= transv_mass_lep_met and transv_mass_lep_met <= 100.)
            {
                dielectron_mass = transv_mass_lep_met;
            }
        }

        std::optional<double> ditau_mass = std::nullopt;
        if (taus.size() >= 1)
        {
            auto transv_mass_lep_met = transverse_mass(taus.p4.at(0).Et() + met.p4.at(0).Et(),
                                                       taus.p4.at(0).px() + met.p4.at(0).px(),
                                                       taus.p4.at(0).py() + met.p4.at(0).py());
            if (50. <= transv_mass_lep_met and transv_mass_lep_met <= 100.)
            {
                ditau_mass = transv_mass_lep_met;
            }
        }

        auto closest_to_W = [](std::array<std::optional<double>, 3> values) -> std::optional<std::size_t>
        {
            double best_diff = std::numeric_limits<double>::infinity();
            std::optional<std::size_t> best_index = std::nullopt;

            std::size_t i = 0;
            for (const auto &v : values)
            {
                if (v)
                {
                    double diff = std::abs(*v - W_MASS);
                    if (diff < best_diff)
                    {
                        best_diff = diff;
                        best_index = i;
                    }
                }
                ++i;
            }

            return best_index;
        }({dimuon_mass, dielectron_mass, ditau_mass});

        if (closest_to_W)
        {
            if (closest_to_W == EWBosonChildren::MUONS)
            {
                h_deltaR_gamma_lepton[idx_var].Fill(Math::VectorUtil::DeltaR(photons.p4.at(0), muons.p4[0]), weight);
                h_gamma_pt[idx_var].Fill(photons.p4.at(0).pt(), weight);
                return;
            }
            if (closest_to_W == EWBosonChildren::ELECTRONS)
            {
                h_deltaR_gamma_lepton[idx_var].Fill(Math::VectorUtil::DeltaR(photons.p4.at(0), electrons.p4[0]),
                                                    weight);
                h_gamma_pt[idx_var].Fill(photons.p4.at(0).pt(), weight);
                return;
            }
            if (closest_to_W == EWBosonChildren::TAUS)
            {
                h_deltaR_gamma_lepton[idx_var].Fill(Math::VectorUtil::DeltaR(photons.p4.at(0), taus.p4[0]), weight);
                h_gamma_pt[idx_var].Fill(photons.p4.at(0).pt(), weight);
                return;
            }

            if (closest_to_W >= EWBosonChildren::UNDEFINED)
            {
                throw std::runtime_error("Z+NGamma Validation: invalid index");
            }
        }
    }
}

auto WGamma::serialize_to_root(const std::unique_ptr<TFile> &output_file) -> void
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

auto WGamma::merge_inplace(const WGamma &other) -> void
{
    MERGE(h_gamma_pt)
    MERGE(h_deltaR_gamma_lepton)
}
