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
      countMap(_countMap),
      shift(_shift)
{
    std::string histo_name = "";

    histo_name = NanoEventClass::make_histogram_full_name(_class_name,    //
                                                          _process_group, //
                                                          _xs_order,      //
                                                          _sample,        //
                                                          _year,          //
                                                          _shift,         //
                                                          "h_counts");
    h_counts = TH1F(histo_name.c_str(), histo_name.c_str(), 1, 0., 1.);
    h_counts.Sumw2();
}

auto EventClass::fill(double weight) -> void
{
    h_counts.Fill(0.5, weight);
}

auto EventClass::dump_outputs() -> void
{
    auto output_file = std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE"));
    output_file->cd();

    h_counts.SetDirectory(output_file.get());
    h_counts.Write();

    output_file->Close();
}

auto make_event_class_name(std::pair<std::size_t, std::size_t> muon_counts,
                           std::pair<std::size_t, std::size_t> electron_counts,
                           std::pair<std::size_t, std::size_t> tau_counts,
                           std::pair<std::size_t, std::size_t> photon_counts,
                           std::pair<std::size_t, std::size_t> jet_counts,
                           std::pair<std::size_t, std::size_t> bjet_counts,
                           std::pair<std::size_t, std::size_t> met_counts)
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

    std::string class_name = "EC";

    if (n_electrons > 0)
    {
        class_name = fmt::format("{}_{}Ele", class_name, n_electrons);
    }
    if (n_muons > 0)
    {
        class_name = fmt::format("{}_{}Muon", class_name, n_muons);
    }
    if (n_photons > 0)
    {
        class_name = fmt::format("{}_{}Gamma", class_name, n_photons);
    }
    if (n_taus > 0)
    {
        class_name = fmt::format("{}_{}Tau", class_name, n_taus);
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
    if (n_muons == total_muons and n_electrons == total_electrons and n_taus == total_taus and
        n_photons == total_photons and n_jets == total_jets and n_bjets == total_bjets and n_met == total_met)
    {
        exclusive_class_name = class_name;
    }

    std::optional<std::string> inclusive_class_name = fmt::format("{}+X", class_name);
    std::optional<std::string> jet_inclusive_class_name = fmt::format("{}+NJet", class_name);

    return std::make_tuple(exclusive_class_name, inclusive_class_name, jet_inclusive_class_name);
}
