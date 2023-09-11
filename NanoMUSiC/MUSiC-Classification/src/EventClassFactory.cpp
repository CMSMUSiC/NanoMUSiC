#include "../include/EventClassFactory.hpp"
#include "Configs.hpp"
#include "Histograms.hpp"
#include "Math/GenVector/VectorUtil.h"
#include "Math/VectorUtil.h"
#include "TEfficiency.h"
#include <filesystem>
#include <fmt/format.h>
#include <optional>
#include <string_view>

#include "NanoEventClass.hpp"

EventClass::EventClass(const std::string &_class_name,
                       const std::string &_output_path,
                       const std::map<std::string, int> &_countMap,
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
    has_met = (_class_name.find("MET") != std::string::npos);

    std::vector<double> limits =
        BinLimits::get_bin_limits("ec_plot", countMap, min_energy, max_energy, min_bin_width, 1);
    std::vector<double> limits_met =
        BinLimits::get_bin_limits("ec_plot_met", countMap, min_energy, max_energy, min_bin_width, 1);

    std::string histo_name = "";

    histo_name = NanoEventHisto::make_histogram_full_name(_class_name,    //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_counts");
    h_counts = TH1F(histo_name.c_str(), histo_name.c_str(), 1, 0., 1.);
    h_counts.Sumw2();

    histo_name = NanoEventHisto::make_histogram_full_name(_class_name,    //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_invariant_mass");
    h_invariant_mass = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_invariant_mass.Sumw2();

    histo_name = NanoEventHisto::make_histogram_full_name(_class_name,    //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_sum_pt");
    h_sum_pt = TH1F(histo_name.c_str(), histo_name.c_str(), limits.size() - 1, limits.data());
    h_sum_pt.Sumw2();

    histo_name = NanoEventHisto::make_histogram_full_name(_class_name,    //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_met");
    h_met = TH1F(histo_name.c_str(), histo_name.c_str(), limits_met.size() - 1, limits_met.data());
    h_met.Sumw2();
}

auto EventClass::fill(std::pair<std::size_t, const MUSiCObjects &> this_muons,
                      std::pair<std::size_t, const MUSiCObjects &> this_electrons,
                      std::pair<std::size_t, const MUSiCObjects &> this_taus,
                      std::pair<std::size_t, const MUSiCObjects &> this_photons,
                      std::pair<std::size_t, const MUSiCObjects &> this_bjets,
                      std::pair<std::size_t, const MUSiCObjects &> this_jets,
                      std::pair<std::size_t, const MUSiCObjects &> this_met,
                      double weight) -> void
{
    auto [n_muons, muons] = this_muons;
    auto [n_electrons, electrons] = this_electrons;
    auto [n_taus, taus] = this_taus;
    auto [n_photons, photons] = this_photons;
    auto [n_bjets, bjets] = this_bjets;
    auto [n_jets, jets] = this_jets;
    auto [n_met, met] = this_met;

    if (has_met)
    {
        h_met.Fill(met.p4.at(0).pt(), weight);
    }

    auto lorentz_vec = Math::PtEtaPhiMVector();
    lorentz_vec += std::reduce(muons.p4.cbegin(), muons.p4.cbegin() + n_muons, Math::PtEtaPhiMVector(), std::plus{});
    lorentz_vec +=
        std::reduce(electrons.p4.cbegin(), electrons.p4.cbegin() + n_electrons, Math::PtEtaPhiMVector(), std::plus{});
    lorentz_vec += std::reduce(taus.p4.cbegin(), taus.p4.cbegin() + n_taus, Math::PtEtaPhiMVector(), std::plus{});
    lorentz_vec +=
        std::reduce(photons.p4.cbegin(), photons.p4.cbegin() + n_photons, Math::PtEtaPhiMVector(), std::plus{});
    lorentz_vec += std::reduce(bjets.p4.cbegin(), bjets.p4.cbegin() + n_bjets, Math::PtEtaPhiMVector(), std::plus{});
    lorentz_vec += std::reduce(jets.p4.cbegin(), jets.p4.cbegin() + n_jets, Math::PtEtaPhiMVector(), std::plus{});
    lorentz_vec += std::reduce(met.p4.cbegin(), met.p4.cbegin() + n_met, Math::PtEtaPhiMVector(), std::plus{});

    if (has_met)
    {
        h_invariant_mass.Fill(MUSiCObjects::transverse_mass(lorentz_vec), weight);
    }
    else
    {
        h_invariant_mass.Fill(lorentz_vec.mass(), weight);
    }

    auto sum_pt = 0.f;
    sum_pt += std::transform_reduce(muons.p4.cbegin(), muons.p4.cbegin() + n_muons, 0.f, std::plus{}, get_pt);
    sum_pt +=
        std::transform_reduce(electrons.p4.cbegin(), electrons.p4.cbegin() + n_electrons, 0.f, std::plus{}, get_pt);
    sum_pt += std::transform_reduce(taus.p4.cbegin(), taus.p4.cbegin() + n_taus, 0.f, std::plus{}, get_pt);
    sum_pt += std::transform_reduce(photons.p4.cbegin(), photons.p4.cbegin() + n_photons, 0.f, std::plus{}, get_pt);
    sum_pt += std::transform_reduce(bjets.p4.cbegin(), bjets.p4.cbegin() + n_bjets, 0.f, std::plus{}, get_pt);
    sum_pt += std::transform_reduce(jets.p4.cbegin(), jets.p4.cbegin() + n_jets, 0.f, std::plus{}, get_pt);
    sum_pt += std::transform_reduce(met.p4.cbegin(), met.p4.cbegin() + n_met, 0.f, std::plus{}, get_pt);
    h_sum_pt.Fill(sum_pt, weight);

    h_counts.Fill(count_bin_center, weight);
}

auto EventClass::dump_outputs(std::unique_ptr<TFile> &output_file) -> void
{
    output_file->cd();

    h_counts.SetDirectory(output_file.get());
    h_counts.Write();

    h_invariant_mass.SetDirectory(output_file.get());
    h_invariant_mass.Write();

    h_sum_pt.SetDirectory(output_file.get());
    h_sum_pt.Write();

    h_met.SetDirectory(output_file.get());
    h_met.Write();
}

constexpr std::size_t max_allowed_jets_per_class = 6;

auto make_event_class_name(std::pair<std::size_t, std::size_t> muon_counts,
                           std::pair<std::size_t, std::size_t> electron_counts,
                           std::pair<std::size_t, std::size_t> tau_counts,
                           std::pair<std::size_t, std::size_t> photon_counts,
                           std::pair<std::size_t, std::size_t> jet_counts,
                           std::pair<std::size_t, std::size_t> bjet_counts,
                           std::pair<std::size_t, std::size_t> met_counts,
                           const std::unordered_map<std::string, std::optional<TriggerMatch>> &trigger_matches)
    -> std::tuple<std::optional<std::string>, std::optional<std::string>, std::optional<std::string>>
{

    auto [n_muons, total_muons] = muon_counts;
    auto [n_electrons, total_electrons] = electron_counts;
    auto [n_taus, total_taus] = tau_counts;
    auto [n_photons, total_photons] = photon_counts;
    auto [n_jets, total_jets] = jet_counts;
    auto [n_bjets, total_bjets] = bjet_counts;
    auto [n_met, total_met] = met_counts;

    if (n_muons == 0 and n_electrons == 0 and n_photons == 0)
    {
        return std::make_tuple(std::nullopt, std::nullopt, std::nullopt);
    }

    if (n_bjets + n_jets > max_allowed_jets_per_class)
    {
        return std::make_tuple(std::nullopt, std::nullopt, std::nullopt);
    }

    if (not(has_good_match(trigger_matches, n_muons, n_electrons, n_photons)))
    {
        return std::make_tuple(std::nullopt, std::nullopt, std::nullopt);
    }

    std::string class_name = "EC";

    if (n_muons > 0)
    {
        class_name = fmt::format("{}_{}Muon", class_name, n_muons);
    }
    if (n_electrons > 0)
    {
        class_name = fmt::format("{}_{}Electron", class_name, n_electrons);
    }
    if (n_taus > 0)
    {
        class_name = fmt::format("{}_{}Tau", class_name, n_taus);
    }
    if (n_photons > 0)
    {
        class_name = fmt::format("{}_{}Photon", class_name, n_photons);
    }
    if (n_jets > 0)
    {
        class_name = fmt::format("{}_{}Jet", class_name, n_jets);
    }
    if (n_bjets > 0)
    {
        class_name = fmt::format("{}_{}bJet", class_name, n_bjets);
    }
    if (n_met > 0)
    {
        class_name = fmt::format("{}_{}MET", class_name, n_met);
    }

    if (n_muons == 0         //
        and n_electrons == 0 //
        and n_taus == 0      //
        and n_photons == 0   //
        and n_jets == 0      //
        and n_bjets == 0     //
        and n_met == 0)
    {
        class_name = "EC_Empty";
    }

    std::optional<std::string> exclusive_class_name = std::nullopt;
    if (n_muons == total_muons             //
        and n_electrons == total_electrons //
        and n_taus == total_taus           //
        and n_photons == total_photons     //
        and n_jets == total_jets           //
        and n_bjets == total_bjets         //
        and n_met == total_met)
    {
        exclusive_class_name = class_name;
    }

    std::optional<std::string> inclusive_class_name = fmt::format("{}+X", class_name);

    std::optional<std::string> jet_inclusive_class_name = std::nullopt;
    if (n_muons == total_muons             //
        and n_electrons == total_electrons //
        and n_taus == total_taus           //
        and n_photons == total_photons     //
        and n_met == total_met)
    {
        jet_inclusive_class_name = fmt::format("{}+NJet", class_name);
    }

    return std::make_tuple(exclusive_class_name, inclusive_class_name, jet_inclusive_class_name);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       h_counts, replace_substring(h_counts.GetName(), "Nominal", Shifts::variation_to_string(shift)).c_str());
    output_file->WriteObject(
        &h_invariant_mass,
        replace_substring(h_invariant_mass.GetName(), "Nominal", Shifts::variation_to_string(shift)).c_str());
    output_file->WriteObject(
        &h_sum_pt, replace_substring(h_sum_pt.GetName(), "Nominal", Shifts::variation_to_string(shift)).c_str());
    output_file->WriteObject(&h_met,
                             replace_substring(h_met.GetName(), "Nominal", Shifts::variation_to_string(shift)).c_str());
}

constexpr std::size_t max_allowed_jets_per_class = 6;

auto make_event_class_name(
    std::pair<std::size_t, std::size_t> muon_counts,
    std::pair<std::size_t, std::size_t> electron_counts,
    std::pair<std::size_t, std::size_t> tau_counts,
    std::pair<std::size_t, std::size_t> photon_counts,
    std::pair<std::size_t, std::size_t> jet_counts,
    std::pair<std::size_t, std::size_t> bjet_counts,
    std::pair<std::size_t, std::size_t> met_counts,
    const std::optional<std::unordered_map<std::string, std::optional<TriggerMatch>>> &trigger_matches)
    -> std::tuple<std::optional<std::string>, std::optional<std::string>, std::optional<std::string>>
{

    auto [n_muons, total_muons] = muon_counts;
    auto [n_electrons, total_electrons] = electron_counts;
    auto [n_taus, total_taus] = tau_counts;
    auto [n_photons, total_photons] = photon_counts;
    auto [n_jets, total_jets] = jet_counts;
    auto [n_bjets, total_bjets] = bjet_counts;
    auto [n_met, total_met] = met_counts;

    if (n_muons == 0 and n_electrons == 0 and n_photons == 0)
    {
        return std::make_tuple(std::nullopt, std::nullopt, std::nullopt);
    }

    if (n_bjets + n_jets > max_allowed_jets_per_class)
    {
        return std::make_tuple(std::nullopt, std::nullopt, std::nullopt);
    }

    if (trigger_matches)
    {
        if (not(has_good_match(*trigger_matches, n_muons, n_electrons, n_photons)))
        {
            return std::make_tuple(std::nullopt, std::nullopt, std::nullopt);
        }
    }

    std::string class_name = "EC";

    if (n_muons > 0)
    {
        class_name = fmt::format("{}_{}Muon", class_name, n_muons);
    }
    if (n_electrons > 0)
    {
        class_name = fmt::format("{}_{}Electron", class_name, n_electrons);
    }
    if (n_taus > 0)
    {
        class_name = fmt::format("{}_{}Tau", class_name, n_taus);
    }
    if (n_photons > 0)
    {
        class_name = fmt::format("{}_{}Photon", class_name, n_photons);
    }
    if (n_jets > 0)
    {
        class_name = fmt::format("{}_{}Jet", class_name, n_jets);
    }
    if (n_bjets > 0)
    {
        class_name = fmt::format("{}_{}bJet", class_name, n_bjets);
    }
    if (n_met > 0)
    {
        class_name = fmt::format("{}_{}MET", class_name, n_met);
    }

    if (n_muons == 0         //
        and n_electrons == 0 //
        and n_taus == 0      //
        and n_photons == 0   //
        and n_jets == 0      //
        and n_bjets == 0     //
        and n_met == 0)
    {
        class_name = "EC_Empty";
    }

    std::optional<std::string> exclusive_class_name = std::nullopt;
    if (n_muons == total_muons             //
        and n_electrons == total_electrons //
        and n_taus == total_taus           //
        and n_photons == total_photons     //
        and n_jets == total_jets           //
        and n_bjets == total_bjets         //
        and n_met == total_met)
    {
        exclusive_class_name = class_name;
    }

    std::optional<std::string> inclusive_class_name = fmt::format("{}+X", class_name);

    std::optional<std::string> jet_inclusive_class_name = std::nullopt;
    if (n_muons == total_muons             //
        and n_electrons == total_electrons //
        and n_taus == total_taus           //
        and n_photons == total_photons     //
        and n_met == total_met)
    {
        jet_inclusive_class_name = fmt::format("{}+NJet", class_name);
    }

    return std::make_tuple(exclusive_class_name, inclusive_class_name, jet_inclusive_class_name);
}
