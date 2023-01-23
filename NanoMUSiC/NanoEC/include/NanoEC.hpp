#ifndef NANO_EC
#define NANO_EC

#include <cmath>
#include <cstddef>
// #include <fmt/core.h>
// #include <fmt/ostream.h>
#include <numeric>

#include <array>
// #include <boost/format.hpp>    // only needed for printing
// #include <boost/histogram.hpp> // make_histogram, integer, indexed
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std::string_literals;

template <std::size_t N>
struct xHisto
{
  private:
    float start;
    float stop;
    float width;

  public:
    std::array<float, N> counts{};
    std::array<float, N> variances{};
    std::array<unsigned int, N> indexes{};

    xHisto(const float &_start = 0., const float &_stop = 1.)
        : start(_start), stop(_stop), width((stop - start) / (static_cast<float>(N)))
    {
        if (stop < start)
        {
            throw std::runtime_error("Start should be greater than Stop.");
        }

        // fill indexes
        for (std::size_t i = 0; i < N; i++)
        {
            indexes[i] = i;
        }
    }

    auto get_bin(const float &value) -> std::size_t
    {
        if (value < start)
        {
            throw std::runtime_error("Value should be greater than Start.");
        }

        if (value > stop)
        {
            throw std::runtime_error("Value should be smaller than Stop.");
        }

        return static_cast<unsigned int>(std::floor((value - start) / width));
    }

    auto fill(const float &value, const float &weight = 1.0) -> void
    {
        counts[get_bin(value)] += weight;
        variances[get_bin(value)] += std::pow(weight, 2);
    }

    auto add(const xHisto<N> &other)
    {
        for (std::size_t i = 0; i < N; i++)
        {
            counts[i] += other.counts[i];
            variances[i] += other.variances[i];
        }
    }

    auto scale(const float &x)
    {
        for (std::size_t i = 0; i < N; i++)
        {
            counts[i] *= x;
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            variances[i] *= std::pow(x, 2);
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
            // ///// REVIEW!!!!
        }
    }

    auto integral() const -> float
    {
        return std::reduce(std::cbegin(counts), std::cend(counts));
    }
};

struct NanoEC
{
    static constexpr unsigned int n_bins = 13000;
    using hist_t = xHisto<n_bins>;

    hist_t sum_pt;
    hist_t mass;
    hist_t met;

    NanoEC() : sum_pt(hist_t(0, n_bins)), mass(hist_t(0, n_bins)), met(hist_t(0, n_bins))
    {
    }

    auto fill(const float &_sum_pt, const float &_mass, const float &_met, const float &weight = 1.0) -> void
    {
        sum_pt.fill(_sum_pt, weight);
        mass.fill(_mass, weight);
        met.fill(_met, weight);
    }

    auto add(const NanoEC &other) -> void
    {
        sum_pt.add(other.sum_pt);
        mass.add(other.mass);
        met.add(other.met);
    }

    auto scale(const float &a) -> void
    {
        sum_pt.scale(a);
        mass.scale(a);
        met.scale(a);
    }

    friend std::ostream &operator<<(std::ostream &os, const NanoEC &ec)
    {
        os << "NanoEC: [" << ec.sum_pt.integral() << "]";
        return os;
    }
};

struct NanoECCollection
{
    std::unordered_map<unsigned long, std::unordered_map<std::string, NanoEC>> event_classes{};

    std::unordered_map<unsigned long, std::unordered_set<unsigned long>> run_event_map{};

    const bool is_data;
    NanoECCollection(const bool &_is_data = false) : is_data(_is_data)
    {
    }

    auto is_already_processed(const unsigned long &run_number, const unsigned long &event_number) -> bool
    {
        if (run_event_map.count(run_number) > 0)
        {
            if (run_event_map[run_number].count(event_number) > 0)
            {
                return true;
            }
        }
        return false;
    }

    auto fill(const unsigned long &ec_hash, const std::string &process_index, const float &_sum_pt, const float &_mass,
              const float &_met, const float &weight = 1.0, const bool &is_data = false,
              const unsigned long &run_number = 0, const unsigned long &event_number = 0) -> void
    {
        if (is_data)
        {
            if (is_already_processed(run_number, event_number))
            {
                return;
            }
            if (run_event_map.count(run_number) == 0)
            {
                run_event_map.emplace(run_number, std::unordered_set<unsigned long>());
            }
            run_event_map[run_number].insert(event_number);
        }

        if (event_classes.count(ec_hash) == 0)
        {
            event_classes.emplace(ec_hash, std::unordered_map<std::string, NanoEC>());
        }
        if (event_classes[ec_hash].count(process_index) == 0)
        {
            event_classes[ec_hash].emplace(process_index, NanoEC());
        }
        event_classes[ec_hash][process_index].fill(_sum_pt, _mass, _met, weight);
    }

    auto merge(const NanoECCollection &other) -> void
    {
        for (const auto &[other_class, other_nanoec_map] : other.event_classes)
        {
            if (event_classes.count(other_class) == 0)
            {
                event_classes.emplace(other_class, std::unordered_map<std::string, NanoEC>());
            }
            for (const auto &[other_process_idx, other_nanoec] : other_nanoec_map)
            {
                if (event_classes[other_class].count(other_process_idx) == 0)
                {
                    event_classes[other_class].emplace(other_process_idx, NanoEC());
                }
                event_classes[other_class][other_process_idx].add(other_nanoec_map.at(other_process_idx));
            }
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const NanoECCollection &ec)
    {
        os << "NanoECCollection: [ Size: " << ec.event_classes.size() << " ]";
        return os;
    }
};

#endif // !NANO_EC