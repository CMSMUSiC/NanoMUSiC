#ifndef OUTPUTS
#define OUTPUTS

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

// ROOT stuff
#include "TFile.h"
#include "TH1.h"
#include "TObjString.h"
#include "TObject.h"
#include "TTree.h"

// On: 28.10.2022
// https://ericniebler.github.io/range-v3
// https://github.com/ericniebler/range-v3
#include <range/v3/all.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
// using fmt::print;

#include "Enumerate.hpp"
#include "MUSiCTools.hpp"
#include "NanoObjects.hpp"

using namespace ranges;

constexpr int MAX_JETS = 6; // SAME AS 2016 PAPER
constexpr int MAX_OBJECTS = 99;

// enum Shift
// {
//     Nominal,
//     Up,
//     Down,
//     kTotalShifts, // !!! should always be the last one !!!
// };

// enum Variation
// {
//     Default, // !!! should always be the first!!!
//     JEC,
//     JER,
//     MuonScale,
//     MuonResolution,
//     ElectronScale,
//     ElectronResolution,
//     kTotalVariations, // !!! should always be the last one !!!
// };

// enum Weight
// {
//     Generator,
//     PDF,
//     Alpha_S,
//     PileUp,
//     Lumi,
//     Trigger,
//     kTotalWeights, // !!! should always be the last one !!!
// };

namespace RangesHelpers
{

// Helper function to get a integer iterator
template <typename T = UInt_t>
constexpr auto index_range(const int &from, const int &to)
{
    using namespace ranges;
    return views::ints(from, to) | views::transform([](auto i) { return static_cast<T>(std::make_unsigned_t<int>(i)); });
}

template <typename T = UInt_t>
constexpr auto index_range(const int &to)
{
    return index_range<T>(0, to);
}

// const auto Variations = index_range<Variation>(Variation::kTotalVariations);
// const auto Shifts = index_range<Shift>(Shift::kTotalShifts);
// auto VariationsAndShifts = views::cartesian_product(Variations, Shifts) | views::remove_if([](auto variation_and_shift) {
//                                const auto [variation, shift] = variation_and_shift;
//                                return (variation == Variation::Default && (shift == Shift::Up || shift == Shift::Down));
//                            }) |
//                            views::remove_if([](auto variation_and_shift) {
//                                const auto [variation, shift] = variation_and_shift;
//                                return (variation == Variation::kTotalVariations);
//                            }) |
//                            views::remove_if([](auto variation_and_shift) {
//                                const auto [variation, shift] = variation_and_shift;
//                                return (shift == Shift::kTotalShifts);
//                            });

// const auto Weights = index_range<Weight>(Weight::kTotalWeights);
} // namespace RangesHelpers

// enum CutFlow
// {
//     // should be kept in order
//     NoCuts,
//     GeneratorWeight,
//     RunLumi,
//     nPV,
//     MetFilters,
//     TriggerCut,
//     TriggerMatch,
//     AtLeastOneClass,
//     kTotalCuts, // --> should be the last one
// };

//     void (const Multiplicity_t &multiplicity, const std::optional<NanoObjects::NanoAODObjects_t> nanoaod_objects)
//     {
//         if (nanoaod_objects)
//         {
//             // unpacking ...
//             const auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;
//             const auto [muons, electrons, photons, taus, bjets, jets, met_obj] = *nanoaod_objects;

//             // auto selected_muons = NanoObjects::Take(muons, i_muons);
//             // auto selected_electrons = NanoObjects::Take(electrons, i_electrons);
//             // auto selected_photons = NanoObjects::Take(photons, i_photons);
//             // // auto selected_taus = NanoObjects::Take(taus, i_taus);
//             // auto selected_bjets = NanoObjects::Take(bjets, i_bjets);
//             // auto selected_jets = NanoObjects::Take(jets, i_jets);
//             // auto selected_met = NanoObjects::Take(met_obj, i_met);

//             // FIX ME: add taus
//             sum_pt.push_back(10.);
//             // sum_pt.push_back(ranges::accumulate(
//             //     views::concat(selected_muons, selected_electrons, selected_photons, selected_bjets, selected_jets,
//             //     selected_met) |
//             //         views::transform([](const auto _muon) { return _muon.pt(); }),
//             //     0));
//             mass.push_back(20.);
//             met.push_back(30.);
//             event_class_hash.push_back(EventContent::make_class_hash(multiplicity));

class Outputs
{
  private:
    const std::string filename;

  public:
    // variations, shifts, weights and cuts
    static constexpr auto Cuts = make_enumerate("NoCuts", "GeneratorWeight", "RunLumi", "nPV", "MetFilters", "TriggerCut",
                                                "TriggerMatch", "AtLeastOneClass");
    static constexpr auto Weights = make_enumerate("Generator", "PDF", "Alpha_S", "PileUp", "Lumi", "Trigger");
    static constexpr auto Variations =
        make_enumerate("Default", "JEC", "JER", "MuonScale", "MuonResolution", "ElectronScale", "ElectronResolution");
    static constexpr auto Shifts = make_enumerate("Nominal", "Up", "Down");

    static constexpr auto kTotalCuts = Outputs::Cuts.size();
    static constexpr auto kTotalWeights = Outputs::Weights.size();
    static constexpr auto kTotalVariations = Outputs::Variations.size();
    static constexpr auto kTotalshifts = Outputs::Shifts.size();

    static unsigned int variation_to_index(std::string_view variation, std::string_view shift)
    {
        // default case
        if (variation == "Default")
        {
            return 0;
        }

        // general case
        return 2 * Outputs::Variations.index_of(variation) - 2 + Outputs::Shifts.index_of(shift);
    }

    static std::pair<std::string_view, std::string_view> index_to_variation(std::size_t index)
    {
        // default case
        if (index == 0)
        {
            return std::make_pair(Outputs::Variations[0], Outputs::Shifts[0]);
        }

        // general case
        std::size_t idx_variation = (index + 1) / 2;
        std::size_t idx_shift = index - 2 * idx_variation + 2;
        return std::make_pair(Outputs::Variations[idx_variation], Outputs::Shifts[idx_shift]);
    }

    static inline auto VariationsAndShiftsRange = views::cartesian_product(Outputs::Variations, Outputs::Shifts) |
                                                  views::remove_if([](auto variation_and_shift) {
                                                      const auto [variation, shift] = variation_and_shift;
                                                      return (variation == "Default" && (shift == "Up" || shift == "Down"));
                                                  }) |
                                                  views::remove_if([](auto variation_and_shift) {
                                                      const auto [variation, shift] = variation_and_shift;
                                                      return (variation != "Default" && shift == "Nominal");
                                                  });

    static constexpr unsigned int kTotalVariationsAndShifts = (Outputs::kTotalVariations - 1) * 2 + 1;
    static inline const auto VariationsAndShiftsIndexRange = RangesHelpers::index_range<unsigned long>(kTotalVariationsAndShifts);

    // output file
    std::unique_ptr<TFile> output_file;

    // output event trees
    std::array<std::unique_ptr<TTree>, kTotalVariationsAndShifts> output_trees;

    // output data per event
    std::array<unsigned int, kTotalVariationsAndShifts> run;
    std::array<unsigned int, kTotalVariationsAndShifts> lumi_section;
    std::array<unsigned long, kTotalVariationsAndShifts> event_number;
    std::array<unsigned long, kTotalVariationsAndShifts> trigger_bits;
    std::array<std::array<float, kTotalWeights>, kTotalVariationsAndShifts> weights_nominal;
    std::array<std::array<float, kTotalWeights>, kTotalVariationsAndShifts> weights_up;
    std::array<std::array<float, kTotalWeights>, kTotalVariationsAndShifts> weights_down;
    std::array<unsigned long, kTotalVariationsAndShifts> nClasses;
    std::array<std::vector<unsigned long>, kTotalVariationsAndShifts> classes;
    std::array<std::vector<float>, kTotalVariationsAndShifts> sum_pt;
    std::array<std::vector<float>, kTotalVariationsAndShifts> invariant_mass;
    std::array<std::vector<float>, kTotalVariationsAndShifts> met;

    // set of classes (unique classes)
    std::array<std::unordered_set<unsigned long>, kTotalVariationsAndShifts> set_of_classes;

    // cutflow
    std::array<TH1F, kTotalVariationsAndShifts> cutflow_histos;

    Outputs(const std::string _filename)
        : filename(_filename), output_file(std::unique_ptr<TFile>(TFile::Open(filename.c_str(), "RECREATE")))
    {
        // fmt::print("DEBUG - TotalVariationsAndShifts: {}\n", kTotalVariationsAndShifts);
        for (auto &&idx_var : VariationsAndShiftsIndexRange)
        {
            const auto [variation, shift] = index_to_variation(idx_var);
            // make event trees
            const std::string output_title = fmt::format("Variation: {} - Shift: {}", variation, shift);
            output_trees.at(idx_var) =
                std::make_unique<TTree>(("nano_music_" + std::to_string(idx_var)).c_str(), output_title.c_str());
            output_trees.at(idx_var)->SetDirectory(output_file.get());

            // tree branches
            output_trees.at(idx_var)->Branch("run", &run.at(idx_var));
            output_trees.at(idx_var)->Branch("lumi_section", &lumi_section.at(idx_var));
            output_trees.at(idx_var)->Branch("event_number", &event_number.at(idx_var));
            output_trees.at(idx_var)->Branch("trigger_bits", &trigger_bits.at(idx_var));
            output_trees.at(idx_var)->Branch("weights_nominal", &weights_nominal.at(idx_var));
            output_trees.at(idx_var)->Branch("weights_up", &weights_up.at(idx_var));
            output_trees.at(idx_var)->Branch("weights_down", &weights_down.at(idx_var));
            output_trees.at(idx_var)->Branch("nClasses", &nClasses.at(idx_var));
            output_trees.at(idx_var)->Branch("classes", &classes.at(idx_var));
            output_trees.at(idx_var)->Branch("sum_pt", &sum_pt.at(idx_var));
            output_trees.at(idx_var)->Branch("invariant_mass", &invariant_mass.at(idx_var));
            output_trees.at(idx_var)->Branch("met", &met.at(idx_var));

            // make cutflow histos
            const std::string histo_name = "cutflow_" + std::to_string(idx_var);
            cutflow_histos.at(idx_var) = TH1F(histo_name.c_str(), output_title.c_str(), kTotalCuts, -0.5, kTotalCuts + 0.5);
            cutflow_histos.at(idx_var).Sumw2();
            cutflow_histos.at(idx_var).SetDirectory(output_file.get());
        }
    }

    void set_event_weight(unsigned int idx_var, std::string_view weight, std::string_view shift, float value)
    {
        if (shift == "Up")
        {
            weights_up.at(idx_var).at(Outputs::Weights.index_of(weight)) = value;
        }
        else if (shift == "Down")
        {

            weights_down.at(idx_var).at(Outputs::Weights.index_of(weight)) = value;
        }
        else
        {
            weights_nominal.at(idx_var).at(Outputs::Weights.index_of(weight)) = value;
        }
    }

    void set_event_weight(std::string_view variation, std::string_view var_shift, std::string_view weight, std::string_view shift,
                          float value)
    {
        return set_event_weight(variation_to_index(variation, var_shift), weight, shift, value);
    }

    void set_event_weight(unsigned int idx_var, std::string_view weight, float value)
    {
        weights_up.at(idx_var).at(Outputs::Weights.index_of(weight)) = value;
        weights_down.at(idx_var).at(Outputs::Weights.index_of(weight)) = value;
        weights_nominal.at(idx_var).at(Outputs::Weights.index_of(weight)) = value;
    }

    void set_event_weight(std::string_view variation, std::string_view var_shift, std::string_view weight, float value)
    {
        return set_event_weight(variation_to_index(variation, var_shift), weight, value);
    }

    float get_event_weight(unsigned int idx_var, std::string_view weight = "", std::string_view shift = "Nominal")
    {
        auto nominal_weight =
            std::reduce(weights_nominal.at(idx_var).cbegin(), weights_nominal.at(idx_var).cend(), 1., std::multiplies<float>());

        if (shift == "Up")
        {
            return weights_up.at(idx_var).at(Outputs::Weights.index_of(weight)) /
                   weights_nominal.at(idx_var).at(Outputs::Weights.index_of(weight)) * nominal_weight;
        }
        else if (shift == "Down")
        {
            return weights_down.at(idx_var).at(Outputs::Weights.index_of(weight)) /
                   weights_nominal.at(idx_var).at(Outputs::Weights.index_of(weight)) * nominal_weight;
        }
        else
        {
            return nominal_weight;
        }
    }

    float get_event_weight(std::string_view variation, std::string_view var_shift, std::string_view weight = "",
                           std::string_view shift = "Nominal")
    {
        return get_event_weight(variation_to_index(variation, var_shift), weight, shift);
    }

    // clear event tree
    void clear_event_tree(unsigned int idx_var)
    {
        run.at(idx_var) = 0;
        lumi_section.at(idx_var) = 0;
        event_number.at(idx_var) = 0;
        trigger_bits.at(idx_var) = 0;

        for (auto &&idx_weight : Outputs::Weights)
        {
            weights_nominal.at(idx_var).at(Outputs::Weights.index_of(idx_weight)) = 1.;
            weights_up.at(idx_var).at(Outputs::Weights.index_of(idx_weight)) = 1.;
            weights_down.at(idx_var).at(Outputs::Weights.index_of(idx_weight)) = 1.;
        }

        nClasses.at(idx_var) = 0;
        classes.at(idx_var).clear();
        sum_pt.at(idx_var).clear();
        invariant_mass.at(idx_var).clear();
        met.at(idx_var).clear();
    }

    // fill event trees
    void clear_event_trees()
    {
        for (auto &&idx_var : VariationsAndShiftsIndexRange)
        {
            clear_event_tree(idx_var);
        }
    }

    // fill event tree
    void fill_event_tree(unsigned long idx_var)
    {
        output_trees.at(idx_var)->Fill();
        for (const auto &reconstructed_class : classes.at(idx_var))
        {
            set_of_classes.at(idx_var).insert(reconstructed_class);
        }
    }

    // fill event trees
    void fill_event_trees()
    {
        for (auto &&idx_var : VariationsAndShiftsIndexRange)
        {
            fill_event_tree(idx_var);
        }
    }

    // fill (unique) classes
    void fill_classes(unsigned long idx_var, unsigned long _class)
    {
        set_of_classes.at(idx_var).insert(_class);
    }

    // fill cutflow
    void fill_cutflow_histo(unsigned long idx_var, std::string_view cut, float weight)
    {
        cutflow_histos.at(idx_var).Fill(Outputs::Cuts.index_of(cut), weight);
    }

    // fill Default cutflow histograms
    void fill_default_cutflow_histos(std::string_view cut, float weight)
    {
        for (auto &&idx_var : VariationsAndShiftsIndexRange)
        {
            cutflow_histos.at(idx_var).Fill(Outputs::Cuts.index_of(cut), weight);
        }
    }

    // save storaged data
    void write_data()
    {
        output_file->cd();
        std::string variations_str = "";
        for (auto &&idx_var : VariationsAndShiftsIndexRange)
        {
            output_trees.at(idx_var)->Write();
            const auto [variation, shift] = index_to_variation(idx_var);
            variations_str += fmt::format("Index: {} - Variation: {} - Shift: {}; ", idx_var, variation, shift);
            output_file->WriteObject(&set_of_classes.at(idx_var), ("set_of_classes_" + std::to_string(idx_var)).c_str());
            cutflow_histos.at(idx_var).Write();
        }
        auto variations_obj_str = TObjString(variations_str.c_str());
        output_file->WriteObject(&variations_obj_str, "indexes_variations_shifts");
    }
};

#endif /*OUTPUTS*/
