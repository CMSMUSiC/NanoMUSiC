#ifndef MAKE_JETS_HPP
#define MAKE_JETS_HPP

// ROOT Stuff
#include "BTagEffMap.hpp"
#include "Math/Vector4Dfwd.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "JetCorrector.hpp"
#include "Shifts.hpp"
#include "TEfficiency.h"
#include "music_objects.hpp"

#include "NanoAODGenInfo.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fmt/core.h>
#include <format>
#include <memory>
#include <stdexcept>

namespace fs = std::filesystem;

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

class BTagEffMaps
{
    // Function to extract substring between "btag_eff_map_" and ".root"
    auto extract_process_group(std::string_view file_path) -> std::string
    {
        constexpr std::string_view prefix = "btag_eff_map_";
        constexpr std::string_view suffix = ".root";

        // Find the position of the prefix
        auto prefix_pos = file_path.find(prefix);
        if (prefix_pos == std::string_view::npos)
        {
            throw std::runtime_error(std::format("Cannot extract process group from file path: {}", file_path));
        }

        // Calculate start position after the prefix
        auto start_pos = prefix_pos + prefix.length();

        // Find the position of the suffix starting from after the prefix
        auto suffix_pos = file_path.find(suffix, start_pos);
        if (suffix_pos == std::string_view::npos)
        {
            throw std::runtime_error(std::format("Cannot extract process group from file path: {}", file_path));
        }

        // Extract the substring between prefix and suffix
        auto length = suffix_pos - start_pos;
        if (length == 0)
        {
            throw std::runtime_error(std::format("Cannot extract process group from file path: {}", file_path));
        }

        return std::string(file_path.substr(start_pos, length));
    }

  public:
    struct HadronFlavor
    {
        static constexpr unsigned int LIGHT = 0;
        static constexpr unsigned int C = 4;
        static constexpr unsigned int B = 5;
    };

    enum class IsDummy
    {
        Dummy,
        NotDummy,
    };

    std::unordered_map<std::string, std::unique_ptr<TEfficiency>> light_eff = {};
    std::unordered_map<std::string, std::unique_ptr<TEfficiency>> c_eff = {};
    std::unordered_map<std::string, std::unique_ptr<TEfficiency>> b_eff = {};
    std::string process_group;
    bool is_data = false;
    IsDummy is_dummy = IsDummy::NotDummy;

    BTagEffMaps(const std::string &process_group,
                const std::string &input_path,
                bool is_data = false,
                IsDummy is_dummy = IsDummy::NotDummy)
        : process_group(process_group),
          is_data(is_data),
          is_dummy(is_dummy)
    {
        if (is_dummy == IsDummy::Dummy or is_data)
        {
            return;
        }

        // Check if directory exists
        if (not(fs::exists(input_path)) or not(fs::is_directory(input_path)))
        {
            throw std::runtime_error(std::format("Directory does not exist: {}", input_path));
        }

        for (const auto &entry : fs::directory_iterator(input_path))
        {
            if (entry.is_regular_file())
            {
                const std::string file_path = entry.path().string();

                // Check if file ends with .root
                if (file_path.ends_with(".root"))
                {
                    // Open ROOT file
                    std::unique_ptr<TFile> file(TFile::Open(file_path.c_str(), "READ"));
                    if (!file || file->IsZombie())
                    {
                        throw std::runtime_error(std::format("Cannot open file: {}", file_path));
                    }

                    auto process_group = extract_process_group(file_path);

                    light_eff[process_group] = std::unique_ptr<TEfficiency>(
                        file->Get<TEfficiency>(std::format("{}_light_eff", process_group).c_str()));
                    if (not(light_eff[process_group]))
                    {
                        throw std::runtime_error(
                            std::format("Cannot load TEfficiency for light jets from {}", file_path));
                    }
                    light_eff[process_group]->SetDirectory(nullptr);
                    light_eff[process_group]->Print("all");

                    c_eff[process_group] = std::unique_ptr<TEfficiency>(
                        file->Get<TEfficiency>(std::format("{}_c_eff", process_group).c_str()));
                    if (not(c_eff[process_group]))
                    {
                        throw std::runtime_error(std::format("Cannot load TEfficiency for c jets from {}", file_path));
                    }
                    c_eff[process_group]->SetDirectory(nullptr);
                    c_eff[process_group]->Print("all");

                    b_eff[process_group] = std::unique_ptr<TEfficiency>(
                        file->Get<TEfficiency>(std::format("{}_b_eff", process_group).c_str()));
                    if (not(b_eff[process_group]))
                    {
                        throw std::runtime_error(std::format("Cannot load TEfficiency for b jets from {}", file_path));
                    }
                    b_eff[process_group]->SetDirectory(nullptr);
                    b_eff[process_group]->Print("all");
                }
            }
        }
    }

    auto get_efficiency(int hadron_flavor, float pt, float eta) const -> double
    {
        if (is_data)
        {
            return 1.;
        }

        eta = std::fabs(eta);

        switch (hadron_flavor)
        {
        case HadronFlavor::LIGHT:
            return light_eff.at(process_group)->GetEfficiency(light_eff.at(process_group)->FindFixBin(pt, eta));
        case HadronFlavor::C:
            return c_eff.at(process_group)->GetEfficiency(c_eff.at(process_group)->FindFixBin(pt, eta));
        case HadronFlavor::B:
            return b_eff.at(process_group)->GetEfficiency(b_eff.at(process_group)->FindFixBin(pt, eta));
        default:
            throw std::runtime_error(std::format("Invalid hadron flavor: {}", hadron_flavor));
        }
    }
};

namespace ObjectFactories
{

inline auto get_scale_resolution_shifts(const Shifts::Variations shift) -> std::pair<std::string, std::string>
{
    if (shift == Shifts::Variations::JetScale_Up)
    {
        return std::make_pair<std::string, std::string>("Up", "Nominal");
    }

    if (shift == Shifts::Variations::JetScale_Down)
    {
        return std::make_pair<std::string, std::string>("Down", "Nominal");
    }

    if (shift == Shifts::Variations::JetResolution_Up)
    {
        return std::make_pair<std::string, std::string>("Nominal", "Up");
    }

    if (shift == Shifts::Variations::JetResolution_Down)
    {
        return std::make_pair<std::string, std::string>("Nominal", "Down");
    }

    return std::make_pair<std::string, std::string>("Nominal", "Nominal");
}

inline auto get_jet_energy_corrections(const Shifts::Variations shift,
                                       float Jet_pt,                 //
                                       float Jet_eta,                //
                                       float Jet_phi,                //
                                       float Jet_rawFactor,          //
                                       float Jet_area,               //
                                       Int_t Jet_genJetIdx,          //
                                       float fixedGridRhoFastjetAll, //
                                       JetCorrector &jet_corrections,
                                       const NanoAODGenInfo::GenJets &gen_jets) -> double
{

    auto [scale_shift, resolution_shift] = get_scale_resolution_shifts(shift);

    // The JetCorrector already knows is_data (from the constructor)
    // JES: Nominal - JER: Nominal
    float scale_correction_nominal = jet_corrections.get_scale_correction(Jet_pt,                 //
                                                                          Jet_eta,                //
                                                                          Jet_phi,                //
                                                                          Jet_rawFactor,          //
                                                                          fixedGridRhoFastjetAll, //
                                                                          Jet_area,               //
                                                                          scale_shift);

    float new_pt_nominal = Jet_pt * scale_correction_nominal;

    float resolution_correction_nominal = jet_corrections.get_resolution_correction(new_pt_nominal,
                                                                                    Jet_eta,                //
                                                                                    Jet_phi,                //
                                                                                    fixedGridRhoFastjetAll, //
                                                                                    Jet_genJetIdx,          //
                                                                                    gen_jets,               //
                                                                                    resolution_shift);

    return scale_correction_nominal * resolution_correction_nominal;
}
inline auto should_remove_jet(const Math::PtEtaPhiMVector &jet,
                              const RVec<float> &Muon_pt,
                              const RVec<float> &Muon_eta,
                              const RVec<float> &Muon_phi,
                              const RVec<bool> &Muon_looseId,
                              const RVec<float> &Electron_pt,
                              const RVec<float> &Electron_eta,
                              const RVec<float> &Electron_phi,
                              const RVec<int> &Electron_cutBased,
                              const RVec<float> &Tau_pt,
                              const RVec<float> &Tau_eta,
                              const RVec<float> &Tau_phi,
                              const RVec<UChar_t> &Tau_idDeepTau2017v2p1VSjet,
                              const RVec<float> &Photon_pt,
                              const RVec<float> &Photon_eta,
                              const RVec<float> &Photon_phi,
                              const RVec<int> &Photon_cutBased) -> bool
{
    // Helper lambda for deltaR calculation
    auto deltaR = [](float eta1, float phi1, float eta2, float phi2)
    {
        float deta = eta1 - eta2;
        float dphi = phi1 - phi2;
        while (dphi > M_PI)
            dphi -= 2 * M_PI;
        while (dphi < -M_PI)
            dphi += 2 * M_PI;
        return std::sqrt(deta * deta + dphi * dphi);
    };

    constexpr float DR_THRESHOLD = 0.4;

    // Check overlap with muons
    for (size_t i = 0; i < Muon_pt.size(); ++i)
    {
        if (Muon_looseId[i] && Muon_pt[i] > 10 && std::abs(Muon_eta[i]) < 2.4)
        {
            if (deltaR(jet.eta(), jet.phi(), Muon_eta[i], Muon_phi[i]) < DR_THRESHOLD)
            {
                return true; // Remove this jet
            }
        }
    }

    // Check overlap with electrons
    for (size_t i = 0; i < Electron_pt.size(); ++i)
    {
        if (Electron_cutBased[i] >= 1 && // Veto
            Electron_pt[i] > 10 && std::abs(Electron_eta[i]) < 2.5)
        {
            if (deltaR(jet.eta(), jet.phi(), Electron_eta[i], Electron_phi[i]) < DR_THRESHOLD)
            {
                return true; // Remove this jet
            }
        }
    }

    // Check overlap with taus
    for (size_t i = 0; i < Tau_pt.size(); ++i)
    {
        if (Tau_idDeepTau2017v2p1VSjet[i] >= 1 && // VVVLoose
            Tau_pt[i] > 20 && std::abs(Tau_eta[i]) < 2.3)
        {
            if (deltaR(jet.eta(), jet.phi(), Tau_eta[i], Tau_phi[i]) < DR_THRESHOLD)
            {
                return true; // Remove this jet
            }
        }
    }

    // Check overlap with photons
    for (size_t i = 0; i < Photon_pt.size(); ++i)
    {
        if (Photon_cutBased[i] >= 1 && // Loose
            Photon_pt[i] > 15 && std::abs(Photon_eta[i]) < 2.5)
        {
            if (deltaR(jet.eta(), jet.phi(), Photon_eta[i], Photon_phi[i]) < DR_THRESHOLD)
            {
                return true; // Remove this jet
            }
        }
    }

    return false; // Keep this jet
}

inline auto make_jets(const RVec<float> &Jet_pt,                       //
                      const RVec<float> &Jet_eta,                      //
                      const RVec<float> &Jet_phi,                      //
                      const RVec<float> &Jet_mass,                     //
                      const RVec<Int_t> &Jet_jetId,                    //
                      const RVec<float> &Jet_btagDeepFlavB,            //
                      const RVec<float> &Jet_rawFactor,                //
                      const RVec<float> &Jet_area,                     //
                      const RVec<float> &Jet_chEmEF,                   //
                      const RVec<Int_t> &Jet_puId,                     //
                      const RVec<Int_t> &Jet_hadronFlavour,            //
                      const RVec<float> &Muon_pt,                      //
                      const RVec<float> &Muon_eta,                     //
                      const RVec<float> &Muon_phi,                     //
                      const RVec<bool> &Muon_looseId,                  //
                      const RVec<bool> &Muon_isPFcand,                 //
                      const RVec<Int_t> &Muon_jetIdx,                  //
                      const RVec<float> &Electron_pt,                  //
                      const RVec<float> &Electron_eta,                 //
                      const RVec<float> &Electron_phi,                 //
                      const RVec<int> &Electron_cutBased,              //
                      const RVec<Int_t> &Electron_jetIdx,              //
                      const RVec<float> &Tau_pt,                       //
                      const RVec<float> &Tau_eta,                      //
                      const RVec<float> &Tau_phi,                      //
                      const RVec<UChar_t> &Tau_idDeepTau2017v2p1VSjet, //
                      const RVec<Int_t> &Tau_jetIdx,                   //
                      const RVec<float> &Photon_pt,                    //
                      const RVec<float> &Photon_eta,                   //
                      const RVec<float> &Photon_phi,                   //
                      const RVec<int> &Photon_cutBased,                //
                      const RVec<Int_t> &Jet_genJetIdx,                //
                      float fixedGridRhoFastjetAll,                    //
                      JetCorrector &jet_corrections,                   //
                      const CorrectionlibRef_t &btag_sf_bc,            //
                      const CorrectionlibRef_t &btag_sf_light,         //
                      const NanoAODGenInfo::GenJets &gen_jets,         //
                      const CorrectionlibRef_t &jet_veto_map,          //
                      const BTagEffMaps &btag_eff_maps,                //
                      bool is_data,                                    //
                      const std::string &_year,                        //
                      const Shifts::Variations shift)
    -> std::tuple<MUSiCObjects, MUSiCObjects, bool, RVec<int>, RVec<int>>
{
    auto year = get_runyear(_year);
    auto jets = RVec<Math::PtEtaPhiMVector>{};
    auto bjets = RVec<Math::PtEtaPhiMVector>{};
    auto jets_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto bjets_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto jets_scale_factors = RVec<double>{};
    auto bjets_scale_factors = RVec<double>{};
    auto jets_scale_factor_shift = RVec<double>{};
    auto bjets_scale_factor_shift = RVec<double>{};
    auto jets_delta_met_x = RVec<double>{};
    auto bjets_delta_met_x = RVec<double>{};
    auto jets_delta_met_y = RVec<double>{};
    auto bjets_delta_met_y = RVec<double>{};
    auto jets_is_fake = RVec<bool>{};
    auto bjets_is_fake = RVec<bool>{};

    bool has_vetoed_jets = false;
    auto selected_jet_indexes = RVec<int>{};
    auto selected_bjet_indexes = RVec<int>{};

    auto btag_weight_jets = RVec<double>{};
    auto btag_weight_bjets = RVec<double>{};

    for (std::size_t i = 0; i < Jet_pt.size(); i++)
    {
        bool is_matched_to_tau = std::find(Tau_jetIdx.cbegin(), Tau_jetIdx.cend(), i) != Tau_jetIdx.cend();
        if (is_matched_to_tau)
        {
            continue;
        }

        bool is_matched_to_electron =
            std::find(Electron_jetIdx.cbegin(), Electron_jetIdx.cend(), i) != Electron_jetIdx.cend();
        if (is_matched_to_electron)
        {
            continue;
        }

        bool is_matched_to_muon = std::find(Muon_jetIdx.cbegin(), Muon_jetIdx.cend(), i) != Muon_jetIdx.cend();
        if (is_matched_to_muon)
        {
            continue;
        }

        // check for vetoed jets
        // apply recemmended loose selection
        // https://cms-jerc.web.cern.ch/Recommendations/#jet-veto-maps
        // https://github.com/columnflow/columnflow/blob/1161b414ba536e93a9f532e8c6078e0b92b2a87e/columnflow/selection/cms/jets.py#L108
        if (shift == Shifts::Variations::Nominal)
        {
            if (Jet_pt[i] > 15. and Jet_jetId[i] >= 2 and Jet_chEmEF[i] < 0.9 and
                (Jet_puId[i] >= 4 or Jet_pt[i] >= 50.))
            {
                bool has_muon_overlap = false;
                for (std::size_t muon_idx = 0; muon_idx < Muon_eta.size(); muon_idx++)
                {
                    if (Muon_isPFcand[i])
                    {
                        if (ROOT::VecOps::DeltaR(Jet_eta[i], Muon_eta[muon_idx], Jet_phi[i], Muon_phi[muon_idx]) < 0.2)
                        {
                            has_muon_overlap = true;
                            break;
                        }
                    }
                }

                if (not(has_muon_overlap))
                {
                    if (jet_veto_map->evaluate({"jetvetomap",
                                                std::max(-5.1905f, std::min(Jet_eta[i], 5.1905f)),
                                                std::max(-3.14158f, std::min(Jet_phi[i], 3.14158f))}) != 0.)
                    {
                        has_vetoed_jets = true;
                        break;
                    }
                }
            }
        }

        auto is_good_jet_pre_filter =
            (std::fabs(Jet_eta[i]) <= ObjConfig::Jets[year].MaxAbsEta) //
            and (Jet_jetId[i] >= ObjConfig::Jets[year].MinJetID)       //
            and (0 <= Jet_btagDeepFlavB[i] and Jet_btagDeepFlavB[i] < ObjConfig::Jets[year].MaxBTagWPTight);

        auto is_good_bjet_pre_filter =
            (std::fabs(Jet_eta[i]) <= ObjConfig::Jets[year].MaxAbsEta) //
            and (Jet_jetId[i] >= ObjConfig::Jets[year].MinJetID)       //
            and (ObjConfig::Jets[year].MaxBTagWPTight <= Jet_btagDeepFlavB[i] and Jet_btagDeepFlavB[i] <= 1);

        auto jet_p4 = Math::PtEtaPhiMVector(Jet_pt[i], Jet_eta[i], Jet_phi[i], Jet_mass[i]);

        // first we accumulate the correction that was already aplied
        const double jets_delta_met_x_type_1 =
            (jet_p4.pt() - Jet_pt[i] * (1. - Jet_rawFactor[i])) * std::cos(Jet_phi[i]);
        const double jets_delta_met_y_type_1 =
            (jet_p4.pt() - Jet_pt[i] * (1. - Jet_rawFactor[i])) * std::sin(Jet_phi[i]);

        if (jet_p4.pt() * (1. - Jet_rawFactor[i]) > 10. and std::fabs(jet_p4.eta()) < 5.2)
        {
            auto jet_energy_corrections = get_jet_energy_corrections(shift,
                                                                     Jet_pt[i],
                                                                     Jet_eta[i],
                                                                     Jet_phi[i],
                                                                     Jet_rawFactor[i],
                                                                     Jet_area[i],
                                                                     Jet_genJetIdx[i],
                                                                     fixedGridRhoFastjetAll,
                                                                     jet_corrections,
                                                                     gen_jets);

            jet_p4 = jet_p4 * jet_energy_corrections;
        }

        if (is_good_jet_pre_filter or is_good_bjet_pre_filter)
        {
            auto is_good_jet = (jet_p4.pt() >= ObjConfig::Jets[year].MediumPt) and is_good_jet_pre_filter;
            auto is_good_bjet = (jet_p4.pt() >= ObjConfig::Jets[year].MediumPt) and is_good_bjet_pre_filter;

            if (is_good_jet or is_good_bjet)
            {
                auto should_remove = should_remove_jet(jet_p4,
                                                       Muon_pt,
                                                       Muon_eta,
                                                       Muon_phi,
                                                       Muon_looseId,
                                                       Electron_pt,
                                                       Electron_eta,
                                                       Electron_phi,
                                                       Electron_cutBased,
                                                       Tau_pt,
                                                       Tau_eta,
                                                       Tau_phi,
                                                       Tau_idDeepTau2017v2p1VSjet,
                                                       Photon_pt,
                                                       Photon_eta,
                                                       Photon_phi,
                                                       Photon_cutBased);

                if (should_remove)
                {
                    continue;
                }
            }

            if (is_good_jet)
            {
                // then we accumulate the new correction
                jets_delta_met_x.push_back((jet_p4.pt() - Jet_pt[i]) * std::cos(Jet_phi[i]) - jets_delta_met_x_type_1);
                jets_delta_met_y.push_back((jet_p4.pt() - Jet_pt[i]) * std::sin(Jet_phi[i]) - jets_delta_met_y_type_1);

                jets_p4.push_back(jet_p4);
                jets_is_fake.push_back(is_data ? false : Jet_genJetIdx[i] < 0);

                selected_jet_indexes.push_back(i);

                if (btag_eff_maps.is_dummy == BTagEffMaps::IsDummy::NotDummy and not(is_data))
                {
                    auto btag_eff = btag_eff_maps.get_efficiency(Jet_hadronFlavour[i], jet_p4.pt(), jet_p4.eta());

                    auto sf = 1.;
                    auto sf_up = 1.;
                    auto sf_down = 1.;

                    switch (Jet_hadronFlavour[i])
                    {
                    case BTagEffMaps::HadronFlavor::LIGHT:
                        sf = MUSiCObjects::get_scale_factor(
                            btag_sf_light,
                            is_data,
                            {"central", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        sf_up = MUSiCObjects::get_scale_factor(
                            btag_sf_light,
                            is_data,
                            {"up_correlated", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        sf_down = MUSiCObjects::get_scale_factor(
                            btag_sf_light,
                            is_data,
                            {"down_correlated", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});

                        break;
                    case BTagEffMaps::HadronFlavor::C:
                    case BTagEffMaps::HadronFlavor::B:
                        sf = MUSiCObjects::get_scale_factor(
                            btag_sf_bc,
                            is_data,
                            {"central", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        sf_up = MUSiCObjects::get_scale_factor(
                            btag_sf_bc,
                            is_data,
                            {"up_correlated", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        sf_down = MUSiCObjects::get_scale_factor(
                            btag_sf_bc,
                            is_data,
                            {"down_correlated", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        break;
                    default:
                        throw std::runtime_error(std::format("Invalid hadron flavor: {}", Jet_hadronFlavour[i]));
                    }
                    auto scale_factor = (1 - btag_eff * sf) / (1 - btag_eff);
                    auto scale_factor_up = (1 - btag_eff * sf_up) / (1 - btag_eff);
                    auto scale_factor_down = (1 - btag_eff * sf_down) / (1 - btag_eff);

                    jets_scale_factors.push_back(scale_factor);
                    jets_scale_factor_shift.push_back(std::max(std::fabs(scale_factor - scale_factor_up),
                                                               std::fabs(scale_factor - scale_factor_down)));
                }
                else
                {
                    jets_scale_factors.push_back(1.);
                    jets_scale_factor_shift.push_back(0.);
                }
            }

            if (is_good_bjet)
            {
                // then we accumulate the new correction
                bjets_delta_met_x.push_back((jet_p4.pt() - Jet_pt[i]) * std::cos(Jet_phi[i]) - jets_delta_met_x_type_1);
                bjets_delta_met_y.push_back((jet_p4.pt() - Jet_pt[i]) * std::sin(Jet_phi[i]) - jets_delta_met_y_type_1);

                bjets_p4.push_back(jet_p4);
                bjets_is_fake.push_back(is_data ? false : Jet_genJetIdx[i] < 0);

                selected_bjet_indexes.push_back(i);

                if (btag_eff_maps.is_dummy == BTagEffMaps::IsDummy::NotDummy and not(is_data))
                {
                    auto sf = 1.;
                    auto sf_up = 1.;
                    auto sf_down = 1.;

                    switch (Jet_hadronFlavour[i])
                    {
                    case BTagEffMaps::HadronFlavor::LIGHT:
                        sf = MUSiCObjects::get_scale_factor(
                            btag_sf_light,
                            is_data,
                            {"central", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        sf_up = MUSiCObjects::get_scale_factor(
                            btag_sf_light,
                            is_data,
                            {"up_correlated", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        sf_down = MUSiCObjects::get_scale_factor(
                            btag_sf_light,
                            is_data,
                            {"down_correlated", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        break;
                    case BTagEffMaps::HadronFlavor::C:
                    case BTagEffMaps::HadronFlavor::B:
                        sf = MUSiCObjects::get_scale_factor(
                            btag_sf_bc,
                            is_data,
                            {"central", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        sf_up = MUSiCObjects::get_scale_factor(
                            btag_sf_bc,
                            is_data,
                            {"up_correlated", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        sf_down = MUSiCObjects::get_scale_factor(
                            btag_sf_bc,
                            is_data,
                            {"down_correlated", "T", Jet_hadronFlavour[i], std::fabs(jet_p4.eta()), jet_p4.pt()});
                        break;
                    default:
                        throw std::runtime_error(std::format("Invalid hadron flavor: {}", Jet_hadronFlavour[i]));
                    }

                    auto scale_factor = sf;
                    auto scale_factor_up = sf_up;
                    auto scale_factor_down = sf_down;

                    bjets_scale_factors.push_back(scale_factor);
                    bjets_scale_factor_shift.push_back(std::max(std::fabs(scale_factor - scale_factor_up),
                                                                std::fabs(scale_factor - scale_factor_down)));
                }
                else
                {
                    bjets_scale_factors.push_back(1.);
                    bjets_scale_factor_shift.push_back(0.);
                }
            }
        }
    }

    return {MUSiCObjects(
                jets_p4, jets_scale_factors, jets_scale_factor_shift, jets_delta_met_x, jets_delta_met_y, jets_is_fake),
            MUSiCObjects(bjets_p4,
                         bjets_scale_factors,
                         bjets_scale_factor_shift,
                         bjets_delta_met_x,
                         bjets_delta_met_y,
                         bjets_is_fake),
            has_vetoed_jets,
            selected_jet_indexes,
            selected_bjet_indexes};
}

} // namespace ObjectFactories

#endif // !MAKE_JETS_HPP
