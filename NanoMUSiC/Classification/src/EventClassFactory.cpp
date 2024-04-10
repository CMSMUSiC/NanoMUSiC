#include "EventClass.hpp"
#include <cmath>
#include <cstddef>
#include <optional>

EventClassHistogram::EventClassHistogram(const std::string_view &name, bool weighted)
    : name(name),
      weighted(weighted)
{
    counts.reserve(expected_max_bins);

    if (weighted)
    {
        squared_weights.reserve(expected_max_bins);
    }
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
