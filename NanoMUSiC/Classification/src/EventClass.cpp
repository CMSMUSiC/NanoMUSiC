#include "EventClass.hpp"
#include "fmt/core.h"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <optional>

auto EventClassHistogram::make_event_class_histogram(const std::string &name, bool weighted) -> EventClassHistogram
{
    auto h = EventClassHistogram{};
    h.name = name;
    h.weighted = weighted;
    h.counts.reserve(expected_max_bins);

    if (weighted)
    {
        h.squared_weights.reserve(expected_max_bins);
    }

    return h;
}

auto EventClassHistogram::push(float x, float w) -> void
{
    auto idx = bin_index(x);
    counts[idx] += w;
    if (weighted)
    {
        squared_weights[idx] += std::pow(w, 2);
    }
}

auto EventClassHistogram::count(float x) -> double
{
    auto idx = bounded_bin_index(x);
    if (idx)
    {
        return counts[*idx];
    }
    return 0.;
}

auto EventClassHistogram::error(float x) -> double
{
    auto idx = bounded_bin_index(x);
    if (idx)
    {
        return std::sqrt(squared_weights[*idx]);
    }
    return 0.;
}

auto EventClassHistogram::size() -> std::size_t
{
    return counts.size();
}

auto EventClassHistogram::bounded_bin_index(float x) -> std::optional<std::size_t>
{
    auto idx = static_cast<int>(x) / static_cast<int>(bin_size);
    if (counts.find(idx) == counts.end())
    {
        return std::nullopt;
    }
    return idx;
}

auto EventClassHistogram::bin_index(float x) -> std::size_t
{
    if (x < 0)
    {
        fmt::print(stderr, "ERROR: Could not get bin index for negative x: {}", x);
        std::exit(EXIT_FAILURE);
    }

    return static_cast<int>(x) / static_cast<int>(bin_size);
}

auto EventClass::make_event_class(const std::string &ec_name) -> EventClass
{
    auto ec = EventClass{};
    ec.ec_name = ec_name;
    for (std::size_t var = 0; var < total_variations; var++)
    {
        if (var == static_cast<std::size_t>(Shifts::Variations::Nominal))
        {
            ec.h_sum_pt[var] = EventClassHistogram::make_event_class_histogram(
                Shifts::variation_to_string(Shifts::Variations::Nominal), true);
            ec.h_invariant_mass[var] = EventClassHistogram::make_event_class_histogram(
                Shifts::variation_to_string(Shifts::Variations::Nominal), true);
            ec.h_met[var] = EventClassHistogram::make_event_class_histogram(
                Shifts::variation_to_string(Shifts::Variations::Nominal), true);
        }
        ec.h_sum_pt[var] = EventClassHistogram::make_event_class_histogram(Shifts::variation_to_string(var), false);
        ec.h_invariant_mass[var] =
            EventClassHistogram::make_event_class_histogram(Shifts::variation_to_string(var), false);
        ec.h_met[var] = EventClassHistogram::make_event_class_histogram(Shifts::variation_to_string(var), false);
    }
    return ec;
}

auto EventClass::sum_pt(std::size_t variation) -> EventClassHistogram &
{
    return h_sum_pt[variation];
}

auto EventClass::invariant_mass(std::size_t variation) -> EventClassHistogram &
{
    return h_invariant_mass[variation];
}

auto EventClass::met(std::size_t variation) -> EventClassHistogram &
{
    return h_met[variation];
}

auto EventClass::histogram(const std::string &observable, std::size_t variation) -> EventClassHistogram &
{
    if (observable == "sum_pt")
    {
        return sum_pt(variation);
    }
    if (observable == "invariant_mass")
    {
        return invariant_mass(variation);
    }
    if (observable == "met")
    {
        return met(variation);
    }

    fmt::print(stderr, "ERROR: Observable ({}) is not defined.", observable);
    std::exit(EXIT_FAILURE);
}

