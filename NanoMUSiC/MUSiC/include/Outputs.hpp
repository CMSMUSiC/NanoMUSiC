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

#include "MUSiCTools.hpp"
#include "NanoObjects.hpp"

using namespace ranges;

constexpr int MAX_JETS = 6; // SAME AS 2016 PAPER
constexpr int MAX_OBJECTS = 99;

enum Shift
{
    Nominal,
    Up,
    Down,
    kTotalShifts, // !!! should always be the last one !!!
};

enum Variation
{
    Default, // !!! should always be the first!!!
    JEC,
    JER,
    MuonScale,
    MuonResolution,
    ElectronScale,
    ElectronResolution,
    kTotalVariations, // !!! should always be the last one !!!
};

unsigned int variation_and_shift_to_index(Variation variation, Shift shift)
{
    switch (shift)
    {
    case Shift::Up:
        return (2 * variation - 1);
    case Shift::Down:
        return (2 * variation);
    default: // nominal
        return 0;
    }
}

enum Weight
{
    Generator,
    PDF,
    Alpha_S,
    PileUp,
    Lumi,
    Trigger,
    kTotalWeights, // !!! should always be the last one !!!
};

// auto range_variations = MUSiCTools::index_range<Variation>(Variation::kTotalVariations);
// auto range_shifts = MUSiCTools::index_range<Shift>(Shift::kTotalShifts);
// auto range_cleanned_variation_and_shifts = views::cartesian_product(range_variations, range_shifts) |
//                                            views::remove_if([](auto variation_and_shift) {
//                                                const auto [variation, shift] = variation_and_shift;
//                                                return (variation == Variation::Default && shift == Shift::Up);
//                                            }) |
//                                            views::remove_if([](auto variation_and_shift) {
//                                                const auto [variation, shift] = variation_and_shift;
//                                                return (variation == Variation::Default && shift == Shift::Down);
//                                            });

// auto range_weights = MUSiCTools::index_range<Weight>(Weight::kTotalWeights);

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

const auto Variations = index_range<Variation>(Variation::kTotalVariations);
const auto Shifts = index_range<Shift>(Shift::kTotalShifts);
auto VariationsAndShifts = views::cartesian_product(Variations, Shifts) | views::remove_if([](auto variation_and_shift) {
                               const auto [variation, shift] = variation_and_shift;
                               return (variation == Variation::Default && (shift == Shift::Up || shift == Shift::Down));
                           }) |
                           views::remove_if([](auto variation_and_shift) {
                               const auto [variation, shift] = variation_and_shift;
                               return (variation == Variation::kTotalVariations);
                           }) |
                           views::remove_if([](auto variation_and_shift) {
                               const auto [variation, shift] = variation_and_shift;
                               return (shift == Shift::kTotalShifts);
                           });

const auto Weights = index_range<Weight>(Weight::kTotalWeights);
} // namespace RangesHelpers

enum CutFlow
{
    // should be kept in order
    NoCuts,
    GeneratorWeight,
    RunLumi,
    nPV,
    MetFilters,
    TriggerCut,
    TriggerMatch,
    AtLeastOneClass,
    kTotalCuts, // --> should be the last one
};

// TH1F imp_make_cutflow_histo(int index)
// {

//     std::string histo_name = "cutflow_histo";

//     if (index == -1)
//     {
//         histo_name = "temp_cutflow_histo";
//     }
//     else
//     {
//         histo_name = "cutflow_histo_" + std::to_string(index);
//     }

//     auto cutflow_histo = TH1F(histo_name.c_str(), histo_name.c_str(), CutFlow::kTotalCuts, -0.5, CutFlow::kTotalCuts + 0.5);
//     cutflow_histo.Sumw2();
//     return cutflow_histo;
// }

// template <typename T>
// std::array<TH1F, total_variations_and_shifts> make_cutflow_histos(T &&output_file)
// {
//     std::array<TH1F, total_variations_and_shifts> cutflow_histos;
//     for (size_t i = 0; i < cutflow_histos.size(); i++)
//     {
//         cutflow_histos.at(i) = imp_make_cutflow_histo(i);
//         cutflow_histos.at(i).SetDirectory(std::forward<T>(output_file));
//     }

//     return cutflow_histos;
// }

class EventWeight : public TObject
{
  public:
    std::array<float, Weight::kTotalWeights> weights_nominal;
    std::array<float, Weight::kTotalWeights> weights_up;
    std::array<float, Weight::kTotalWeights> weights_down;

    EventWeight()
    {
        for (auto &&idx_weight : RangesHelpers::Weights)
        {
            weights_nominal.at(idx_weight) = 1.;
            weights_up.at(idx_weight) = 1.;
            weights_down.at(idx_weight) = 1.;
        }
    }

    void set(Weight weight, Shift shift, float value)
    {
        switch (shift)
        {
        case Shift::Up:
            weights_up.at(weight) = value;
            break;
        case Shift::Down:
            weights_down.at(weight) = value;
            break;
        default: // nominal
            weights_nominal.at(weight) = value;
            break;
        }
    }

    void set(Weight weight, float value)
    {
        weights_up.at(weight) = value;
        weights_down.at(weight) = value;
        weights_nominal.at(weight) = value;
    }

    float get(Weight weight = Weight::kTotalWeights, Shift shift = Shift::Nominal)
    {
        auto nominal_weight = std::reduce(weights_nominal.cbegin(), weights_nominal.cend(), 1., std::multiplies<float>());

        switch (shift)
        {
        case Shift::Up:
            return weights_up.at(weight) / weights_nominal.at(weight) * nominal_weight;
        case Shift::Down:
            return weights_down.at(weight) / weights_nominal.at(weight) * nominal_weight;
        default: // nominal
            return nominal_weight;
        }
    }

    ClassDef(EventWeight, 1)
};

// class EventContent : public TObject
// {
//   public:
//     EventWeight event_weight;
//     std::vector<unsigned long> event_class_hash;
//     std::vector<float> sum_pt;
//     std::vector<float> mass;
//     std::vector<float> met;

//     EventContent()
//     {
//     }

//     template <typename T>
//     static std::string to_string_with_zero_padding(T &value, std::size_t total_length = 2)
//     {
//         std::string _str = std::to_string(value);
//         if (_str.length() < total_length)
//         {
//             _str.insert(0, "0");
//         }
//         return _str;
//     }

//     static unsigned long make_class_hash(const Multiplicity_t &multiplicity)
//     {
//         const auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;

//         return std::stoul("9" + to_string_with_zero_padding(i_muons) + to_string_with_zero_padding(i_electrons) +
//                           to_string_with_zero_padding(i_photons) + to_string_with_zero_padding(i_taus) +
//                           to_string_with_zero_padding(i_bjets) + to_string_with_zero_padding(i_jets) +
//                           to_string_with_zero_padding(i_met));
//     }

//     static auto get_multiplicities(const int &n_muons, const int &n_electrons, const int &n_photons, const int &n_taus,
//                                    const int &n_bjets, const int &n_jets, const int &n_met)
//     {
//         return views::cartesian_product(
//                    views::ints(0, std::min(n_muons, MAX_OBJECTS) + 1), views::ints(0, std::min(n_electrons, MAX_OBJECTS) + 1),
//                    views::ints(0, std::min(n_photons, MAX_OBJECTS) + 1), views::ints(0, std::min(n_taus, MAX_OBJECTS) + 1),
//                    views::ints(0, std::min(n_bjets, MAX_OBJECTS) + 1), views::ints(0, std::min(n_jets, MAX_OBJECTS) + 1),
//                    views::ints(0, std::min(n_met, MAX_OBJECTS) + 1)) |
//                views::remove_if([&](const auto &multiplicity) {
//                    const auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;

//                    // no taus (for now)
//                    if (i_taus > 0)
//                    {
//                        return true;
//                    }

//                    // MET filter
//                    if (i_met > 1)
//                    {
//                        return true;
//                    }

//                    // at least one muon or one electron
//                    if (i_muons == 0 && i_electrons == 0)
//                    {
//                        return true;
//                    }

//                    // no more than MAX_JETS
//                    if (i_jets > MAX_JETS)
//                    {
//                        return true;
//                    }

//                    // default --> accepted
//                    return false;
//                });
//     }

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
//         }
//     }

//     ClassDef(EventContent, 1)
// };

// class MUSiCEvent : public TObject
// {
//   public:
//     unsigned int run = 0;
//     unsigned int lumi_section = 0;
//     unsigned long event_number = 0;
//     unsigned int trigger_bits = 0;
//     std::array<EventContent, total_variations_and_shifts> event_contents;

//     MUSiCEvent()
//     {
//     }

//     void fill(const EventContent &event_content, const Variation variation, const Shift shift)
//     {
//         event_contents.at(variation_shift_to_index(variation, shift)) = event_content;
//     }

//     void fill(const EventContent &event_content, unsigned int index)
//     {
//         event_contents.at(index) = event_content;
//     }

//     EventContent get(const Variation variation = Variation::Default, const Shift shift = Shift::Nominal)
//     {
//         return event_contents.at(variation_shift_to_index(variation, shift));
//     }
//     ClassDef(MUSiCEvent, 1)
// };

class Outputs
{
  private:
    const std::string filename;

  public:
    // variations
    static constexpr unsigned int kTotalVariationsAndShifts = (Variation::kTotalVariations - 1) * 2 + 1;
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
    std::array<EventWeight, kTotalVariationsAndShifts> event_weight;
    std::array<unsigned long, kTotalVariationsAndShifts> nClasses;
    std::array<std::vector<unsigned long>, kTotalVariationsAndShifts> classes;
    std::array<std::vector<float>, kTotalVariationsAndShifts> sum_pt;
    std::array<std::vector<float>, kTotalVariationsAndShifts> invariant_mass;
    std::array<std::vector<float>, kTotalVariationsAndShifts> met;

    // set of classes (unique classes)
    std::array<std::set<unsigned long>, kTotalVariationsAndShifts> set_of_classes;

    // cutflow
    std::array<TH1F, kTotalVariationsAndShifts> cutflow_histos;

    Outputs(const std::string _filename)
        : filename(_filename), output_file(std::unique_ptr<TFile>(TFile::Open(filename.c_str(), "RECREATE")))
    {
        // fmt::print("DEBUG - TotalVariationsAndShifts: {}\n", kTotalVariationsAndShifts);
        for (auto &&idx_var : VariationsAndShiftsIndexRange)
        {
            // make event trees
            output_trees.at(idx_var) = std::make_unique<TTree>(("nano_music_" + std::to_string(idx_var)).c_str(), "");
            output_trees.at(idx_var)->SetDirectory(output_file.get());

            // tree branches
            output_trees.at(idx_var)->Branch("run", &run.at(idx_var));
            output_trees.at(idx_var)->Branch("lumi_section", &lumi_section.at(idx_var));
            output_trees.at(idx_var)->Branch("event_number", &event_number.at(idx_var));
            output_trees.at(idx_var)->Branch("trigger_bits", &trigger_bits.at(idx_var));
            // output_trees.at(idx_var)->Branch("event_weight", &event_weight.at(idx_var));
            output_trees.at(idx_var)->Branch("nClasses", &nClasses.at(idx_var));
            output_trees.at(idx_var)->Branch("classes", &classes.at(idx_var));
            // output_trees.at(idx_var)->Branch("sum_pt", &sum_pt.at(idx_var));
            // output_trees.at(idx_var)->Branch("invariant_mass", &invariant_mass.at(idx_var));
            // output_trees.at(idx_var)->Branch("met", &met.at(idx_var));

            // make cutflow histos
            const std::string histo_name = "cutflow_" + std::to_string(idx_var);
            cutflow_histos.at(idx_var) =
                TH1F(histo_name.c_str(), histo_name.c_str(), CutFlow::kTotalCuts, -0.5, CutFlow::kTotalCuts + 0.5);
            cutflow_histos.at(idx_var).Sumw2();
            cutflow_histos.at(idx_var).SetDirectory(output_file.get());
        }
    }

    // clear event tree
    void clear_event_tree(unsigned long idx_var)
    {
        run.at(idx_var) = 0;
        lumi_section.at(idx_var) = 0;
        event_number.at(idx_var) = 0;
        trigger_bits.at(idx_var) = 0;
        event_weight.at(idx_var) = EventWeight();
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
    void fill_cutflow_histo(unsigned long idx_var, unsigned int cut, float weight)
    {
        cutflow_histos.at(idx_var).Fill(cut, weight);
    }

    // fill Default cutflow histograms
    void fill_default_cutflow_histos(unsigned int cut, float weight)
    {
        for (auto &&idx_var : VariationsAndShiftsIndexRange)
        {
            cutflow_histos.at(idx_var).Fill(cut, weight);
        }
    }

    // save storaged data
    void write_data()
    {
        output_file->cd();
        for (auto &&idx_var : VariationsAndShiftsIndexRange)
        {
            output_trees.at(idx_var)->Write();
            output_file->WriteObject(&set_of_classes.at(idx_var), ("set_of_classes_" + std::to_string(idx_var)).c_str());
            cutflow_histos.at(idx_var).Write();
        }
    }

    // write many outputs to disk
    static void write_data(std::vector<Outputs> &outputs_vec)
    {
        std::vector<std::thread> output_writers;
        for (auto &&slot : RangesHelpers::index_range(outputs_vec.size()))
        {
            output_writers.push_back(std::thread([&](auto slot) { outputs_vec.at(slot).write_data(); }, slot));
        }

        for (auto &writer : output_writers)
        {
            writer.join();
        }
    }
};

#endif /*OUTPUTS*/
