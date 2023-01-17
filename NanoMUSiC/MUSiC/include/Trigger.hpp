#ifndef MUSIC_TRIGGER
#define MUSIC_TRIGGER

#include <string>
#include <string_view>

#include "CorrectionSets.hpp"
#include "Enumerate.hpp"

#include <fmt/core.h>

#include "ROOT/RVec.hxx"
using namespace ROOT;
using namespace ROOT::VecOps;
// using namespace std::literals::string_view_literals;
using namespace std::literals;

namespace Trigger
{
constexpr auto HLTPath = make_enumerate( //
    "SingleMuonLowPt"sv,                 //
    "SingleMuonHighPt"sv,                //
    "SingleElectron"sv,                  //
    "DoubleMuon"sv,                      //
    "DoubleElectron"sv,                  //
    "Photon"sv,                          //
    "Tau"sv,                             //
    "BJet"sv,                            //
    "Jet"sv,                             //
    "MET"sv);
constexpr auto kTotalPaths = HLTPath.size();

std::string get_year_for_muon_sf(Year year)
{
    switch (year)
    {
    case Year::Run2016APV:
        return "2016preVFP_UL"s;
    case Year::Run2016:
        return "2016postVFP_UL"s;
    case Year::Run2017:
        return "2017_UL"s;
    case Year::Run2018:
        return "2018_UL"s;
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

template <typename T1, typename T2>
std::pair<RVec<float>, RVec<float>> get_matches(const T1 &trigger_objects_pt,  //
                                                const T1 &trigger_objects_eta, //
                                                const T1 &trigger_objects_phi, //
                                                const T2 &nano_objects_pt,     //
                                                const T2 &nano_objects_eta,    //
                                                const T2 &nano_objects_phi)
{
    auto matches_distances = RVec<float>(nano_objects_pt.size(), std::numeric_limits<float>::max());
    auto matches_relative_pT = RVec<float>(nano_objects_pt.size(), std::numeric_limits<float>::max());

    for (std::size_t idx = 0; idx < nano_objects_pt.size(); idx++)
    {
        // DeltaR
        matches_distances.at(idx) = VecOps::Min(VecOps::sqrt(                               //
            VecOps::pow((trigger_objects_eta - nano_objects_eta[idx]), 2.)                  //
            + VecOps::pow(VecOps::DeltaPhi(trigger_objects_phi, nano_objects_phi[idx]), 2.) //
            ));

        // Relative pT diff
        matches_relative_pT.at(idx) =
            VecOps::Min(VecOps::abs(nano_objects_pt[idx] - trigger_objects_phi) / (nano_objects_pt[idx]));
    }

    return std::make_pair(matches_distances, matches_relative_pT);
}

template <typename T1, typename T2>
constexpr std::tuple<bool, float, float> trigger_matcher(const T1 &trigger_objects_pt,  //
                                                         const T1 &trigger_objects_eta, //
                                                         const T1 &trigger_objects_phi, //
                                                         const T2 &nano_objects_pt,     //
                                                         const T2 &nano_objects_eta,    //
                                                         const T2 &nano_objects_phi,    //
                                                         const float max_delta_r)
{
    bool has_trigger_match = false;
    float matched_nanoobject_pt = 0.;
    float matched_nanoobject_eta = 0.;

    if (nano_objects_pt.size() > 0 and trigger_objects_pt.size() > 0)
    {
        auto [matches_distances, matches_rel_pT] = get_matches(trigger_objects_pt,  //
                                                               trigger_objects_eta, //
                                                               trigger_objects_phi, //
                                                               nano_objects_pt,     //
                                                               nano_objects_eta,    //
                                                               nano_objects_phi);

        auto good_delta_r = matches_distances <= max_delta_r;
        if (VecOps::Any(good_delta_r))
        {
            has_trigger_match = true;

            // if there is any match, them we take the first object.
            // it should be the largest pT
            matched_nanoobject_pt = nano_objects_pt[0];
            matched_nanoobject_eta = nano_objects_eta[0];
        }
    }

    //////////////////////////////////////////////////////
    // DEBUG
    // fmt::print("###### DEBUG ############\n");
    // fmt::print("Has a match? : {}\n", has_trigger_match);
    // fmt::print("Distances: : {}\n", matches_distances);
    // fmt::print("Rel_pT: : {}\n", matches_rel_pT);
    ///////////////////////////////////////

    return std::make_tuple(has_trigger_match, matched_nanoobject_pt, matched_nanoobject_eta);
}

RVec<int> check_bit(const RVec<int> &trigger_bits, const int &bit)
{
    return (trigger_bits & bit) / bit;
}

///////////////////////////////////////////////////////////
/// Check the `trigger_bit` of a given TrigObj for Single Muon - Low pT
// Run2017 configurations
// hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p07 -> HLT_IsoMu27 - bit: 8
// hltL3crIsoL1sSingleMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p07 -> HLT_IsoMu24 - bit: 8
///
RVec<int> SingleMuonLowPtBits(const RVec<int> &triggerobj_bit, const Year &year)
{
    switch (year)
    {
    case Year::Run2016APV:
        return check_bit(triggerobj_bit, 8);
    case Year::Run2016:
        return check_bit(triggerobj_bit, 8);
    case Year::Run2017:
        return check_bit(triggerobj_bit, 8);
    case Year::Run2018:
        return check_bit(triggerobj_bit, 8);
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
};

///////////////////////////////////////////////////////////
/// Check the `trigger_bit` of a given TrigObj for Single Muon - High pT
// Run2017 configurations
// hltL3fL1sMu22Or25L1f0L2f10QL3Filtered50Q -> HLT_Mu50 - bit: 1024
// hltL3fL1sMu22Or25L1f0L2f10QL3Filtered100Q -> HLT_OldMu100 - bit: 2048
// hltL3fL1sMu25f0TkFiltered100Q -> HLT_OldMu100 - bit: 2048
///
RVec<int> SingleMuonHighPtBits(const RVec<int> &triggerobj_bit, const Year &year)
{
    switch (year)
    {
    case Year::Run2016APV:
        return (check_bit(triggerobj_bit, 1024) | check_bit(triggerobj_bit, 2048));
    case Year::Run2016:
        return (check_bit(triggerobj_bit, 1024) | check_bit(triggerobj_bit, 2048));
    case Year::Run2017:
        return (check_bit(triggerobj_bit, 1024) | check_bit(triggerobj_bit, 2048));
    case Year::Run2018:
        return (check_bit(triggerobj_bit, 1024) | check_bit(triggerobj_bit, 2048));
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
};

////////////////////////////////////////////////////////////////////////
/// Check the `trigger_bit` of a given TrigObj, `path` and `year`.
///
RVec<int> check_trigger_bit(const RVec<int> &triggerobj_bit, const std::string_view &path, const Year &year)
{
    if (path == std::string_view("SingleMuonLowPt"))
    {
        return SingleMuonLowPtBits(triggerobj_bit, year);
    }
    else if (path == std::string_view("SingleMuonHighPt"))
    {
        return SingleMuonHighPtBits(triggerobj_bit, year);
    }
    else
    {
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

} // namespace Trigger

struct TriggerBits
{

    std::bitset<Trigger::kTotalPaths> trigger_bits;

    TriggerBits &set(unsigned int path, bool value)
    {
        trigger_bits.set(path, value);
        return *this;
    }

    TriggerBits &set(const std::string_view &path, bool value)
    {
        trigger_bits.set(Trigger::HLTPath.index_of(path), value);
        return *this;
    }

    bool pass(const std::string_view &path) const
    {
        return trigger_bits.test(Trigger::HLTPath.index_of(path));
    }

    bool pass(unsigned int path) const
    {
        return trigger_bits.test(path);
    }

    bool any() const
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
        return trigger_bits.to_string();
    }
};

#endif /*MUSIC_TRIGGER*/
