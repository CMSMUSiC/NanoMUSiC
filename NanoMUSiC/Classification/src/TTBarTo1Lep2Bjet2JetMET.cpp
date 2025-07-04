#include "TTBarTo1Lep2Bjet2JetMET.hpp"
#include <cstdlib>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>

#include "EventClass.hpp"
#include "SerializationUtils.hpp"

TTBarTo1Lep2Bjet2JetMET::TTBarTo1Lep2Bjet2JetMET(enum Leptons lepton,
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
        analysis_name = "ttbar_to_1muon_2bjet_2jet_met";

        count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 1},
                                                         {ObjectNames::Electron, 0},
                                                         {ObjectNames::Photon, 0},
                                                         {ObjectNames::Tau, 0},
                                                         {ObjectNames::bJet, 2},
                                                         {ObjectNames::Jet, 2},
                                                         {ObjectNames::MET, 0}};
    }
    else if (lepton == Leptons::ELECTRONS)
    {
        analysis_name = "ttbar_to_1electron_2bjet_2jet_met";

        count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 0},
                                                         {ObjectNames::Electron, 1},
                                                         {ObjectNames::Photon, 0},
                                                         {ObjectNames::Tau, 0},
                                                         {ObjectNames::bJet, 2},
                                                         {ObjectNames::Jet, 2},
                                                         {ObjectNames::MET, 0}};
    }
    else if (lepton == Leptons::TAUS)
    {
        analysis_name = "ttbar_to_1tau_2bjet_2jet_met";

        count_map = std::unordered_map<ObjectNames, int>{{ObjectNames::Muon, 0},
                                                         {ObjectNames::Electron, 0},
                                                         {ObjectNames::Photon, 0},
                                                         {ObjectNames::Tau, 1},
                                                         {ObjectNames::bJet, 2},
                                                         {ObjectNames::Jet, 2},
                                                         {ObjectNames::MET, 0}};
    }
    else
    {

	    std::runtime_error("Could not set analysis name and bim limits. Lepton flavor not found.");
    }

    auto bins_limits = BinLimits::limits(
        count_map, false, Histograms::min_energy, Histograms::max_energy, Histograms::min_bin_size, Histograms::fudge);

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
                                                                  "h_invariant_mass_jet0_jet1");

        h_invariant_mass_jet0_jet1[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_invariant_mass_jet0_jet1[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_ht_had_lep");

        h_ht_had_lep[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits.size() - 1, bins_limits.data());
        h_ht_had_lep[idx_var].Sumw2();

        histo_name = SerializationUtils::make_histogram_full_name(analysis_name,                        //
                                                                  process_group,                        //
                                                                  xs_order,                             //
                                                                  sample,                               //
                                                                  year,                                 //
                                                                  Shifts::variation_to_string(idx_var), //
                                                                  "h_transverse_mass_lep_MET");

        h_transverse_mass_lep_MET[idx_var] =
            TH1F(histo_name.c_str(), histo_name.c_str(), bins_limits_MET.size() - 1, bins_limits_MET.data());
        h_transverse_mass_lep_MET[idx_var].Sumw2();
    }
}

inline auto transverse_mass(double et, double px, double py) -> double
{
    return std::sqrt(std::pow(et, 2) - std::pow(px, 2) - std::pow(py, 2));
}

auto TTBarTo1Lep2Bjet2JetMET::fill(const MUSiCObjects &leptons,
                                   const MUSiCObjects &bjets,
                                   const MUSiCObjects &jets,
                                   const MUSiCObjects &met,
                                   double weight,
                                   Shifts::Variations shift) -> void
{
    auto idx_var = static_cast<std::size_t>(shift);
    if ((leptons.size() >= 1) and (bjets.size() >= 2) and (jets.size() >= 2) and (met.size() >= 1))
    {
        auto jet_12_mass = (jets.p4.at(0) + jets.p4.at(1)).mass();
        auto tranv_mass_lep_met = transverse_mass(leptons.p4.at(0).Et() + met.p4.at(0).Et(),
                                                  leptons.p4.at(0).px() + met.p4.at(0).px(),
                                                  leptons.p4.at(0).py() + met.p4.at(0).py());

        if (tranv_mass_lep_met >= 60.)
        {
            h_invariant_mass_jet0_jet1[idx_var].Fill(jet_12_mass, weight);
        }

        if (PDG::W::Mass - 30. < jet_12_mass and jet_12_mass < PDG::W::Mass + 30.)
        {
            h_transverse_mass_lep_MET[idx_var].Fill(tranv_mass_lep_met, weight);
        }

        if (tranv_mass_lep_met >= 60. and (PDG::W::Mass - 30. < jet_12_mass and jet_12_mass < PDG::W::Mass + 30.))
        {
            h_ht_had_lep[idx_var].Fill(jets.p4.at(0).pt() + jets.p4.at(1).pt() + bjets.p4.at(0).pt() +
                                       bjets.p4.at(1).pt());
        }
    }
}

auto TTBarTo1Lep2Bjet2JetMET::serialize_to_root(const std::unique_ptr<TFile> &output_file) -> void
{
    for (auto &hist : {h_invariant_mass_jet0_jet1, h_ht_had_lep, h_transverse_mass_lep_MET})
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

auto TTBarTo1Lep2Bjet2JetMET::merge_inplace(const TTBarTo1Lep2Bjet2JetMET &other) -> void
{
    MERGE(h_invariant_mass_jet0_jet1)
    MERGE(h_ht_had_lep)
    MERGE(h_transverse_mass_lep_MET)
}
