#ifndef NANO_EC
#define NANO_EC

#include <cstddef>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <array>
#include <boost/format.hpp>    // only needed for printing
#include <boost/histogram.hpp> // make_histogram, integer, indexed
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std::string_literals;

struct NanoEC
{
    using axes_t = std::tuple<boost::histogram::axis::regular<>>;
    using hist_t = boost::histogram::histogram<axes_t>;

    hist_t sum_pt;
    hist_t mass;
    hist_t met;

    NanoEC()
        : sum_pt(boost::histogram::make_weighted_histogram(boost::histogram::axis::regular<>(13000, 0, 13000))),
          mass(boost::histogram::make_weighted_histogram(boost::histogram::axis::regular<>(13000, 0, 13000))),
          met(boost::histogram::make_weighted_histogram(boost::histogram::axis::regular<>(13000, 0, 13000)))
    {
    }

    auto fill(const float &_sum_pt, const float &_mass, const float &_met, const float &weight = 1.0) -> void
    {
        sum_pt(_sum_pt, boost::histogram::weight(weight));
        mass(_mass, boost::histogram::weight(weight));
        met(_met, boost::histogram::weight(weight));
    }

    auto add(const NanoEC &other) -> void
    {
        sum_pt += other.sum_pt;
        mass += other.mass;
        met += other.met;
    }

    auto scale(const float &a) -> void
    {
        sum_pt *= a;
        mass *= a;
        met *= a;
    }

    friend std::ostream &operator<<(std::ostream &os, const NanoEC &ec)
    {
        os << "NanoEC: [" << boost::histogram::algorithm::sum(ec.sum_pt) << "]";
        return os;
    }
};

struct NanoECCollection
{
    static constexpr unsigned int n_processes = 1000;
    using NanoECArray_t = std::array<NanoEC, n_processes>;

    std::unordered_map<unsigned long, NanoECArray_t> event_classes;

    auto fill(const unsigned long &ec_hash, const std::size_t &process_index, const float &_sum_pt, const float &_mass,
              const float &_met, const float &weight = 1.0) -> void
    {
        if (process_index >= n_processes)
        {
            throw std::runtime_error(fmt::format("The request process index({}) was not found.\n", process_index));
        }

        if (event_classes.count(ec_hash) == 0)
        {
            event_classes.emplace(ec_hash, NanoECArray_t());
        }
        event_classes[ec_hash][process_index].fill(_sum_pt, _mass, _met, weight);
    }

    auto merge(const NanoECCollection &other) -> void
    {
        // for (auto &&[this_classes, this_array] : this->event_classes)
        // {
        //     fmt::print("this_classes: {}\n", this_classes);
        // }
        // for (auto &&[other_classes, other_array] : other.event_classes)
        // {
        //     fmt::print("other_classes: {}\n", other_classes);
        // }
        // fmt::print("has zero? {}\n", event_classes.count(20));
        for (const auto &[other_class, other_nanoec_array] : other.event_classes)
        {
            fmt::print("Merging: {}\n", other_class);
            if (event_classes.count(other_class) == 0)
            {
                // fmt::print("new: {}\n", other_class);
                event_classes.emplace(other_class, NanoECArray_t());
            }
            for (std::size_t process_index = 0; process_index < other_nanoec_array.size(); process_index++)
            {
                event_classes[other_class][process_index].add(other_nanoec_array[process_index]);
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