#ifndef MUSIC_TRIGGER
#define MUSIC_TRIGGER

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

using namespace ranges;
using namespace ROOT::VecOps;

enum HLTPath
{
    SingleMuonLowPt,
    SingleMuonHighPt,
    SingleElectron,
    DoubleMuon,
    DoubleElectron,
    Tau,
    BJet,
    MET,
    Photon,
    kTotalPaths, // --> should be the last one!
};

struct TriggerBits
{
    // will have size = SIZE
    constexpr static size_t SIZE = sizeof(unsigned int) * 8;
    std::bitset<SIZE> trigger_bits;

    TriggerBits &set(unsigned int path, bool value)
    {
        trigger_bits.set(path, value);
        return *this;
    }

    bool pass(unsigned int path)
    {
        return trigger_bits.test(path);
    }

    bool any()
    {
        return trigger_bits.any();
    }

    auto as_ulong() const
    {
        return trigger_bits.to_ulong();
    }

    auto as_ulonglong() const
    {
        return trigger_bits.to_ullong();
    }

    auto as_uint() const
    {
        return static_cast<unsigned int>(trigger_bits.to_ullong());
    }

    std::string_view as_string() const
    {
        return std::string_view(std::to_string(this->as_ulong()));
    }

    template <typename T1, typename T2>
    static constexpr std::pair<RVec<float>, RVec<float>> get_matches(T1 &&trigger_objects, T2 &&nano_objects)
    {
        auto matches_distances = RVec<float>(nano_objects.size, std::numeric_limits<float>::max());
        auto matches_relative_pT = RVec<float>(nano_objects.size, std::numeric_limits<float>::max());

        auto combinations = Combinations(trigger_objects, nano_objects);
        auto trigger_idx = combinations[0];
        auto nano_idx = combinations[1];

        DeltaR(Take(trigger_objects.eta, trigger_idx), Take(trigger_objects.phi, trigger_idx), Take(nano_objects.eta, nano_idx),
               Take(nano_objects.phi, nano_idx));

        for (std::size_t trigger_idx = 0; trigger_idx < trigger_objects.size; trigger_idx++)
        {
            for (std::size_t nano_idx = 0; nano_idx < nano_objects.size; nano_idx++)
            {
                matches_distances.at(nano_idx) = std::min(DeltaR(trigger_objects.eta[trigger_idx], nano_objects.eta[nano_idx],
                                                                 trigger_objects.phi[trigger_idx], nano_objects.phi[nano_idx]),
                                                          matches_distances.at(nano_idx));
                matches_relative_pT.at(nano_idx) =
                    std::min(std::fabs(trigger_objects.pt[trigger_idx] - nano_objects.pt[nano_idx]) / (nano_objects.pt[nano_idx]),
                             matches_relative_pT.at(nano_idx));
            }
        }
        return std::make_pair(matches_distances, matches_relative_pT);
    }

    template <typename T1, typename T2>
    static constexpr std::tuple<bool, float, float, float> trigger_matcher(T1 trigger_objs, T2 nano_objects, Year &year)
    {
        bool has_match = false;
        float trigger_sf_nominal = 1.0;
        float trigger_sf_up = 1.0;
        float trigger_sf_down = 1.0;
        auto [matches_distances, matches_rel_pT] = TriggerBits::get_matches(trigger_objs, nano_objects);
        if (MUSiCTools::MinElem(matches_distances).value_or(std::numeric_limits<float>::max()) <
            ObjConfig::Muons[year].MaxDeltaRTriggerMatch)
        {
            has_match = true;
            //////////////////////////////////////////////////////////////
            // FIX ME: Here it should evaluate the trigger SF.
            //////////////////////////////////////////////////////////////
        }
        return std::make_tuple(has_match, trigger_sf_nominal, trigger_sf_up, trigger_sf_down);
    }

    // Run2017 configurations
    // hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p07 -> HLT_IsoMu27 - bit: 8
    // hltL3crIsoL1sSingleMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p07 -> HLT_IsoMu24 - bit: 8
    // hltL3fL1sMu22Or25L1f0L2f10QL3Filtered50Q -> HLT_Mu50 - bit: 1024
    // hltL3fL1sMu22Or25L1f0L2f10QL3Filtered100Q -> HLT_OldMu100 - bit: 2048
    // hltL3fL1sMu25f0TkFiltered100Q -> HLT_OldMu100 - bit: 2048

    // Single Muon - Low pT
    static constexpr bool SingleMuonLowPtBits(const int &trigger_bit, const Year &year)
    {
        switch (year)
        {
        case Year::Run2016APV:
            return (trigger_bit & 8);
        case Year::Run2016:
            return (trigger_bit & 8);
        case Year::Run2017:
            return (trigger_bit & 8);
        case Year::Run2018:
            return (trigger_bit & 8);
        default:
            throw std::runtime_error("Year (" + std::to_string(year) +
                                     ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
        }
    };

    // Single Muon - High pT
    static constexpr bool SingleMuonHighPtBits(const int &trigger_bit, const Year &year)
    {
        switch (year)
        {
        case Year::Run2016APV:
            return ((trigger_bit & 1024) || (trigger_bit & 2048));
        case Year::Run2016:
            return ((trigger_bit & 1024) || (trigger_bit & 2048));
        case Year::Run2017:
            return ((trigger_bit & 1024) || (trigger_bit & 2048));
        case Year::Run2018:
            return ((trigger_bit & 1024) || (trigger_bit & 2048));
        default:
            throw std::runtime_error("Year (" + std::to_string(year) +
                                     ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
        }
    };

    static constexpr bool check_bit(const int &trigger_bit, const HLTPath &path, const Year &year)
    {
        switch (path)
        {
        case HLTPath::SingleMuonLowPt:
            return SingleMuonLowPtBits(trigger_bit, year);

        case HLTPath::SingleMuonHighPt:
            return SingleMuonHighPtBits(trigger_bit, year);

        default:
            throw std::runtime_error("Year (" + std::to_string(year) +
                                     ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
        }
    }
};

#endif /*MUSIC_TRIGGER*/
