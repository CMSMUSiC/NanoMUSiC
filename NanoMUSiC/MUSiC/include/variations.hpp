#ifndef VARIATIONS
#define VARIATIONS

// On: 28.10.2022
// https://ericniebler.github.io/range-v3
// https://github.com/ericniebler/range-v3
// #include <range/v3/all.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
// using fmt::print;

#include "Enumerate.hpp"

using namespace ranges;

// namespace RangesHelpers
// {

// // Helper function to get an integer iterator
// template <typename T = UInt_t>
// constexpr auto index_range(const int &from, const int &to)
// {
//     using namespace ranges;
//     return views::ints(from, to) | views::transform([](auto i) { return static_cast<T>(std::make_unsigned_t<int>(i)); });
// }

// template <typename T = UInt_t>
// constexpr auto index_range(const int &to)
// {
//     return index_range<T>(0, to);
// }

// } // namespace RangesHelpers
namespace Variations
{

// variations, shifts, weights and cuts
constexpr auto Cuts =
    make_enumerate("NoCuts", "GeneratorWeight", "RunLumi", "nPV", "METFilters", "TriggerCut", "TriggerMatch", "AtLeastOneClass");
constexpr auto Weights = make_enumerate("Generator", "PDF", "Alpha_S", "PileUp", "Lumi", "Trigger");
constexpr auto Variations =
    make_enumerate("Default", "JEC", "JER", "MuonScale", "MuonResolution", "ElectronScale", "ElectronResolution");
constexpr auto Shifts = make_enumerate("Nominal", "Up", "Down");

constexpr auto kTotalCuts = Outputs::Cuts.size();
constexpr auto kTotalWeights = Outputs::Weights.size();
constexpr auto kTotalVariations = Outputs::Variations.size();
constexpr auto kTotalshifts = Outputs::Shifts.size();

unsigned int variation_to_index(std::string_view variation, std::string_view shift)
{
    // default case
    if (variation == "Default")
    {
        return 0;
    }

    // general case
    return 2 * Outputs::Variations.index_of(variation) - 2 + Outputs::Shifts.index_of(shift);
}

std::pair<std::string_view, std::string_view> index_to_variation(std::size_t index)
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

inline auto VariationsAndShiftsRange = views::cartesian_product(Outputs::Variations, Outputs::Shifts) |
                                       views::remove_if([](auto variation_and_shift) {
                                           const auto [variation, shift] = variation_and_shift;
                                           return (variation == "Default" && (shift == "Up" || shift == "Down"));
                                       }) |
                                       views::remove_if([](auto variation_and_shift) {
                                           const auto [variation, shift] = variation_and_shift;
                                           return (variation != "Default" && shift == "Nominal");
                                       });

constexpr unsigned int kTotalVariationsAndShifts = (Outputs::kTotalVariations - 1) * 2 + 1;
inline const auto VariationsAndShiftsIndexRange = RangesHelpers::index_range<unsigned long>(kTotalVariationsAndShifts);

} // namespace Variations

#endif /*VARIATIONS*/
