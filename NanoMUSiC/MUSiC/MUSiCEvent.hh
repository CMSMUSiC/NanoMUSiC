#include <array>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <tuple>

#include "TObjString.h"
#include "TObject.h"
#include "TTree.h"

// On: 28.10.2022
// https://ericniebler.github.io/range-v3
// https://github.com/ericniebler/range-v3
#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/remove_if.hpp>

using namespace ranges;

constexpr int MAX_JETS = 6; // SAME AS 2016 PAPER

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
    kTotalWeights, // !!! should always be the last one !!!
};

class EventWeight : public TObject
{
  public:
    std::array<float, Weight::kTotalWeights> weights_nominal;
    std::array<float, Weight::kTotalWeights> weights_up;
    std::array<float, Weight::kTotalWeights> weights_down;

    EventWeight()
    {
    }

    void set_weight(Weight weight, Shift shift, float value)
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

    float get_weight(Weight weight = Weight::kTotalWeights, Shift shift = Shift::Nominal)
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
  public:
    std::vector<EventWeight> event_weight;
    std::vector<unsigned int> event_class_hash;
    std::vector<float> sum_pt;
    std::vector<float> mass;
    std::vector<float> met;

    EventContent()
    {
    }

    static unsigned int get_class_hash(const std::tuple<int, int, int, int, int, int, int> &multiplicity)
    {
        auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;
        return std::stoul(std::to_string(i_muons) + std::to_string(i_electrons) + std::to_string(i_photons) +
                          std::to_string(i_taus) + std::to_string(i_bjets) + std::to_string(i_jets) +
                          std::to_string(i_met));
    }

    static auto get_multiplicities(const int &n_muons, const int &n_electrons, const int &n_photons, const int &n_taus,
                                   const int &n_bjets, const int &n_jets, const int &n_met)
    {
        return views::cartesian_product(views::ints(0, n_muons + 1), views::ints(0, n_electrons + 1),
                                        views::ints(0, n_photons + 1), views::ints(0, n_taus + 1),
                                        views::ints(0, n_bjets + 1), views::ints(0, n_jets + 1),
                                        views::ints(0, n_met + 1)) |
               views::remove_if([&](const auto &multiplicity) {
                   const auto [i_muons, i_electrons, i_photons, i_taus, i_bjets, i_jets, i_met] = multiplicity;

                   // MET filter
                   if (i_met > 1)
                   {
                       return true;
                   }

                   // at least one muon or one electron
                   if (i_muons == 0 || i_electrons == 0)
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

    ClassDef(EventContent, 1)
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

class MUSiCEvent : public TObject
{
  public:
    unsigned int run = 0;
    unsigned int lumi_section = 0;
    unsigned long event_number = 0;
    unsigned int trigger_bits = 0;
    char n_muons = 10;
    char n_electrons = 0;
    char n_photons = 0;
    char n_taus = 0;
    char n_bjets = 0;
    char n_jets = 0;
    bool n_met = 0;
    unsigned long n_classes = 0;

    std::array<EventContent, Variation::kTotalVariations> event_content_nominal;
    std::array<EventContent, Variation::kTotalVariations> event_content_up;
    std::array<EventContent, Variation::kTotalVariations> event_content_down;

    MUSiCEvent()
    {
    }

    void set_content(Variation variation, Shift shift, EventContent &&event_content)
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
    EventContent get_content(Variation variation = Variation::Default, Shift shift = Shift::Nominal)
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
