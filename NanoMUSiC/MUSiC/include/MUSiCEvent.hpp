#ifndef MUSIC_EVENTS
#define MUSIC_EVENTS

#include <array>
#include <functional>
#include <iostream>
#include <numeric>
#include <optional>
#include <string>
#include <tuple>

// ROOT stuff
#include "TH1.h"
#include "TObjString.h"
#include "TObject.h"
#include "TTree.h"

// On: 28.10.2022
// https://ericniebler.github.io/range-v3
// https://github.com/ericniebler/range-v3
// #include <range/v3/algorithm/for_each.hpp>
// #include <range/v3/numeric/accumulate.hpp>
// #include <range/v3/view/cartesian_product.hpp>
// #include <range/v3/view/iota.hpp>
// #include <range/v3/view/remove_if.hpp>
// #include <range/v3/view/take.hpp>
// #include <range/v3/view/transform.hpp>
#include <range/v3/all.hpp>

#include "MUSiCTools.hpp"
#include "NanoObjects.hpp"

using namespace ranges;

constexpr int MAX_JETS = 6; // SAME AS 2016 PAPER
constexpr int MAX_OBJECTS = 99;

using Multiplicity_t = std::tuple<int, int, int, int, int, int, int>;

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

constexpr unsigned int total_variations_and_shifts = Variation::kTotalVariations * 2 + 1;

unsigned int variation_shift_to_index(Variation variation, Shift shift)
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

auto range_variations = MUSiCTools::index_range<Variation>(Variation::kTotalVariations);
auto range_shifts = MUSiCTools::index_range<Shift>(Shift::kTotalShifts);
auto range_cleanned_variation_and_shifts = views::cartesian_product(range_variations, range_shifts) |
                                           views::remove_if([](auto variation_and_shift) {
                                               const auto [variation, shift] = variation_and_shift;
                                               return (variation == Variation::Default && shift == Shift::Up);
                                           }) |
                                           views::remove_if([](auto variation_and_shift) {
                                               const auto [variation, shift] = variation_and_shift;
                                               return (variation == Variation::Default && shift == Shift::Down);
                                           });

auto range_weights = MUSiCTools::index_range<Weight>(Weight::kTotalWeights);

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

TH1F imp_make_cutflow_histo(unsigned int index)
{
    std::string histo_name = "cutflow_histo" + std::to_string(index);
    auto cutflow_histo = TH1F(histo_name.c_str(), histo_name.c_str(), CutFlow::kTotalCuts, -0.5, CutFlow::kTotalCuts + 0.5);
    cutflow_histo.Sumw2();
    return cutflow_histo;
}

template <typename T>
std::array<TH1F, total_variations_and_shifts> make_cutflow_histos(T &&output_file, const std::stringstream &output_tree_title)
{
    std::array<TH1F, total_variations_and_shifts> cutflow_histos;
    for (size_t i = 0; i < cutflow_histos.size(); i++)
    {
        cutflow_histos.at(i) = imp_make_cutflow_histo(i);
        cutflow_histos.at(i).SetDirectory(std::forward<T>(output_file));
    }

    return cutflow_histos;
}

class EventWeight : public TObject
{
  public:
    std::array<float, Weight::kTotalWeights> weights_nominal;
    std::array<float, Weight::kTotalWeights> weights_up;
    std::array<float, Weight::kTotalWeights> weights_down;

    EventWeight()
    {
        for (auto &&idx_weight : range_weights)
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
        auto nominal_weight = std::accumulate(weights_nominal.cbegin(), weights_nominal.cend(), 1., std::multiplies<float>());

        switch (shift)
        {
        case Shift::Up:
            return nominal_weight / weights_nominal.at(weight) * weights_up.at(weight);
        case Shift::Down:
            return nominal_weight / weights_nominal.at(weight) * weights_down.at(weight);
        default: // nominal
            return nominal_weight;
        }
    }

    ClassDef(EventWeight, 1)
};

class EventContent : public TObject
{
  public:
    EventWeight event_weight;
    std::vector<unsigned long> event_class_hash;
    std::vector<float> sum_pt;
    std::vector<float> mass;
    std::vector<float> met;

    EventContent()
    {
    }

    template <typename T>
    static std::string to_string_with_zero_padding(T &value, std::size_t total_length = 2)
    {
        std::string _str = std::to_string(value);
        if (_str.length() < total_length)
        {
            _str.insert(0, "0");
        }
        return _str;
    }

    static unsigned long make_class_hash(const Multiplicity_t &multiplicity)
    {
        const auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;

        return std::stoul("9" + to_string_with_zero_padding(i_muons) + to_string_with_zero_padding(i_electrons) +
                          to_string_with_zero_padding(i_photons) + to_string_with_zero_padding(i_taus) +
                          to_string_with_zero_padding(i_bjets) + to_string_with_zero_padding(i_jets) +
                          to_string_with_zero_padding(i_met));
    }

    static auto get_multiplicities(const int &n_muons, const int &n_electrons, const int &n_photons, const int &n_taus,
                                   const int &n_bjets, const int &n_jets, const int &n_met)
    {
        return views::cartesian_product(
                   views::ints(0, std::min(n_muons, MAX_OBJECTS) + 1), views::ints(0, std::min(n_electrons, MAX_OBJECTS) + 1),
                   views::ints(0, std::min(n_photons, MAX_OBJECTS) + 1), views::ints(0, std::min(n_taus, MAX_OBJECTS) + 1),
                   views::ints(0, std::min(n_bjets, MAX_OBJECTS) + 1), views::ints(0, std::min(n_jets, MAX_OBJECTS) + 1),
                   views::ints(0, std::min(n_met, MAX_OBJECTS) + 1)) |
               views::remove_if([&](const auto &multiplicity) {
                   const auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;

                   // no taus (for now)
                   if (i_taus > 0)
                   {
                       return true;
                   }

                   // MET filter
                   if (i_met > 1)
                   {
                       return true;
                   }

                   // at least one muon or one electron
                   if (i_muons == 0 && i_electrons == 0)
                   {
                       return true;
                   }

                   // no more than MAX_JETS
                   if (i_jets > MAX_JETS)
                   {
                       return true;
                   }

                   // default --> accepted
                   return false;
               });
    }

    void fill(const Multiplicity_t &multiplicity, const std::optional<NanoObject::NanoAODObjects_t> nanoaod_objects)
    {
        if (nanoaod_objects)
        {
            // unpacking ...
            const auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;
            const auto [muons, electrons, photons, taus, bjets, jets, met_obj] = *nanoaod_objects;

            auto selected_muons = muons | views::take(i_muons);
            auto selected_electrons = electrons | views::take(i_electrons);
            auto selected_photons = photons | views::take(i_photons);
            // auto selected_taus = taus | views::take(i_taus);
            auto selected_bjets = bjets | views::take(i_bjets);
            auto selected_jets = jets | views::take(i_jets);
            auto selected_met = views::single(met_obj) | views::take(i_met);

            // FIX ME: add taus
            sum_pt.emplace_back(ranges::accumulate(
                views::concat(selected_muons, selected_electrons, selected_photons, selected_bjets, selected_jets, selected_met) |
                    views::transform([](const auto _muon) { return _muon.pt(); }),
                0));
            mass.emplace_back(20.);
            met.emplace_back(30.);
            event_class_hash.emplace_back(EventContent::make_class_hash(multiplicity));
        }
    }

    ClassDef(EventContent, 1)
};

class MUSiCEvent : public TObject
{
  public:
    unsigned int run = 0;
    unsigned int lumi_section = 0;
    unsigned long event_number = 0;
    unsigned int trigger_bits = 0;
    std::array<EventContent, total_variations_and_shifts> event_contents;

    MUSiCEvent()
    {
    }

    void fill(EventContent &&event_content, const Variation variation, const Shift shift)
    {
        event_contents.at(variation_shift_to_index(variation, shift)) = event_content;
    }

    void fill(EventContent &&event_content, unsigned int index)
    {
        event_contents.at(index) = event_content;
    }

    EventContent get(const Variation variation = Variation::Default, const Shift shift = Shift::Nominal)
    {
        return event_contents.at(variation_shift_to_index(variation, shift));
    }
    ClassDef(MUSiCEvent, 1)
};

#endif /*MUSIC_EVENTS*/
