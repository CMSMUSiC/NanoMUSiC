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

enum CutFlow
{
    // ideally, should be kept in order
    NoCuts,
    nPV,
    MetFilters,
    TriggerCut,
    TriggerMatch,
    AtLeastOneClass,
    kTotalCuts, // --> should be the last one
};

template <typename T>
TH1F make_cutflow_histo(T &&output_file, const std::stringstream &output_tree_title)
{
    auto cutflow_histo =
        TH1F("cutflow_histo", output_tree_title.str().c_str(), CutFlow::kTotalCuts, -0.5, CutFlow::kTotalCuts + 0.5);
    cutflow_histo.Sumw2();
    cutflow_histo.SetDirectory(std::forward<T>(output_file));
    return cutflow_histo;
}

constexpr int MAX_JETS = 6;     // SAME AS 2016 PAPER
constexpr int MAX_OBJECTS = 99; // SAME AS 2016 PAPER

using Multiplicity_t = std::tuple<int, int, int, int, int, int, int>;

enum Shift
{
    Nominal,
    Up,
    Down,
    kTotalShifts, // !!! should always be the last one !!!
};

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

enum Variation
{
    Default,
    JEC,
    JER,
    MuonScale,
    MuonResolution,
    ElectronScale,
    ElectronResolution,
    kTotalVariations, // !!! should always be the last one !!!
};

auto range_variations = MUSiCTools::index_range<Variation>(Variation::kTotalVariations);
auto range_shifts = MUSiCTools::index_range<Shift>(Shift::kTotalShifts);
auto range_weights = MUSiCTools::index_range<Weight>(Weight::kTotalWeights);

class EventWeight : public TObject
{
  public:
    std::array<float, Weight::kTotalWeights> weights_nominal;
    std::array<float, Weight::kTotalWeights> weights_up;
    std::array<float, Weight::kTotalWeights> weights_down;

    EventWeight()
    {
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

    float get(Weight weight = Weight::kTotalWeights, Shift shift = Shift::Nominal)
    {
        auto nominal_weight =
            std::accumulate(weights_nominal.cbegin(), weights_nominal.cend(), 1, std::multiplies<float>());

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
    // mutable std::mutex _mtx;

  public:
    // std::vector<EventWeight> event_weight;
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

    static unsigned long get_class_hash(const Multiplicity_t &multiplicity)
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
        return views::cartesian_product(views::ints(0, std::min(n_muons, MAX_OBJECTS) + 1),
                                        views::ints(0, std::min(n_electrons, MAX_OBJECTS) + 1),
                                        views::ints(0, std::min(n_photons, MAX_OBJECTS) + 1),
                                        views::ints(0, std::min(n_taus, MAX_OBJECTS) + 1),
                                        views::ints(0, std::min(n_bjets, MAX_OBJECTS) + 1),
                                        views::ints(0, std::min(n_jets, MAX_OBJECTS) + 1),
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
        // std::lock_guard<std::mutex> l(_mtx);

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
            sum_pt.emplace_back(ranges::accumulate(views::concat(selected_muons, selected_electrons, selected_photons,
                                                                 selected_bjets, selected_jets, selected_met) |
                                                       views::transform([](const auto _muon) { return _muon.pt(); }),
                                                   0));
            mass.emplace_back(20.);
            met.emplace_back(30.);
            auto event_weight_buffer = EventWeight{};
            // for (const auto &weight : range_weights)
            // {
            //     event_weight_buffer.set_weight(weight, Shift::Nominal, 1.0);
            //     event_weight_buffer.set_weight(weight, Shift::Up, 1.1);
            //     event_weight_buffer.set_weight(weight, Shift::Down, 0.9);
            // }
            // event_weight.emplace_back(event_weight_buffer);
            event_class_hash.emplace_back(EventContent::get_class_hash(multiplicity));
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
    char n_muons = 0;
    char n_electrons = 0;
    char n_photons = 0;
    char n_taus = 0;
    char n_bjets = 0;
    char n_jets = 0;
    bool n_met = 0;
    unsigned long n_classes = 0;
    EventWeight event_weight;

    std::array<EventContent, Variation::kTotalVariations> event_content_nominal;
    std::array<EventContent, Variation::kTotalVariations> event_content_up;
    std::array<EventContent, Variation::kTotalVariations> event_content_down;

    MUSiCEvent()
    {
    }

    void fill(const Variation variation, const Shift shift, const EventContent &&event_content)
    {
        switch (shift)
        {
        case Shift::Up:
            event_content_up.at(variation) = event_content;
            break;
        case Shift::Down:
            event_content_down.at(variation) = event_content;
            break;
        default: // nominal
            event_content_nominal.at(variation) = event_content;
            break;
        }
    }

    EventContent get(const Variation variation = Variation::Default, const Shift shift = Shift::Nominal)
    {
        switch (shift)
        {
        case Shift::Up:
            return event_content_up.at(variation);
        case Shift::Down:
            return event_content_down.at(variation);
        default: // nominal
            return event_content_nominal.at(variation);
        }
    }
    ClassDef(MUSiCEvent, 1)
};

#endif /*MUSIC_EVENTS*/
