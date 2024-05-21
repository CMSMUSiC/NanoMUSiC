#ifndef EVENT_CLASS_HPP
#define EVENT_CLASS_HPP

#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "Shifts.hpp"
#include "TFile.h"

#include "BinLimits.hpp"

namespace Histograms
{
constexpr double fudge = 1.;
constexpr double min_bin_size = 10.;

constexpr int n_energy_bins = 1300;
constexpr float min_energy = 0;
constexpr float max_energy = 13000;

constexpr int n_eta_bins = 20;
constexpr float min_eta = -3.;
constexpr float max_eta = 3.;

constexpr int n_phi_bins = 20;
constexpr float min_phi = -M_PI;
constexpr float max_phi = M_PI;

constexpr float min_dR = 0;
constexpr float max_dR = 10;
constexpr int n_dR_bins = static_cast<int>((max_dR - min_dR) / 0.4);

constexpr int n_multiplicity_bins = 11;
constexpr float min_multiplicity = -0.5;
constexpr float max_multiplicity = static_cast<float>(n_multiplicity_bins - 1) + 0.5;
} // namespace Histograms

class EventClassHistogram
{
  public:
    constexpr static float bin_size = Histograms::min_bin_size;
    constexpr static std::size_t expected_max_bins = 1300;

    std::string name;
    bool weighted;
    std::unordered_map<unsigned int, double> counts;
    std::unordered_map<unsigned int, double> squared_weights;

    auto static make_event_class_histogram(const std::string &name = "", bool weighted = false) -> EventClassHistogram;
    auto push(float x, float w = 1.f) -> void;
    auto count(float x) -> double;
    auto error(float x) -> double;
    auto size() -> std::size_t;

    auto bounded_bin_index(float x) -> std::optional<std::size_t>;
    auto bin_index(float x) -> std::size_t;
    auto merge_inplace(EventClassHistogram &other) -> void;

    auto serialize_to_root(const std::unique_ptr<TFile> &output_root_file,
                           const std::unordered_map<ObjectNames, int> &count_map,
                           std::vector<double> &bins_limits,
                           const std::string &event_class_name,
                           const std::string &process_name,
                           const std::string &process_group,
                           const std::string &xsec_order,
                           const std::string &year,
                           bool is_data,
                           const std::string &histogram_name,
                           const std::string &variation_name) -> void;
};

class EventClass
{
    static constexpr std::size_t total_variations = static_cast<std::size_t>(Shifts::Variations::kTotalVariations);

  public:
    std::string ec_name;
    std::array<EventClassHistogram, total_variations> h_sum_pt;
    std::array<EventClassHistogram, total_variations> h_invariant_mass;
    std::array<EventClassHistogram, total_variations> h_met;

    static auto make_event_class(const std::string &ec_name) -> EventClass;

    auto histogram(const std::string &observable, std::size_t variation) -> EventClassHistogram &;
    auto sum_pt(std::size_t variation) -> EventClassHistogram &;
    auto invariant_mass(std::size_t variation) -> EventClassHistogram &;
    auto met(std::size_t variation) -> EventClassHistogram &;

    template <typename T>
    auto push(float sum_pt_value,
              float invariant_mass_value,
              const std::optional<float> &met_value,
              float weight,
              T _variation) -> void
    {
        auto variation = static_cast<std::size_t>(_variation);
        sum_pt(variation).push(sum_pt_value, weight);
        invariant_mass(variation).push(invariant_mass_value, weight);
        if (met_value)
        {
            met(variation).push(*met_value, weight);
        }
    }

    template <typename T>
    auto push(float sum_pt_value, float invariant_mass_value, float weight, T variation) -> void
    {
        push(sum_pt_value, invariant_mass_value, std::nullopt, weight, variation);
    }

    inline auto size() -> std::size_t
    {
        return h_sum_pt.size();
    };

    auto merge_inplace(EventClass &other) -> void;

    auto serialize_to_root(const std::unique_ptr<TFile> &output_root_file,
                           const std::string &event_class_name,
                           const std::string &process_name,
                           const std::string &process_group,
                           const std::string &xsec_order,
                           const std::string &year,
                           bool is_data) -> void;
};

class EventClassContainer
{
  public:
    std::unordered_map<std::string, EventClass> classes;

    auto unsafe_ec(const std::string &ec_name) -> EventClass &
    {
        return classes[ec_name];
    }

    auto ec(const std::string &ec_name) -> EventClass &
    {
        return classes.at(ec_name);
    }

    auto has_ec(const std::string &ec_name) -> bool
    {
        return not(classes.find(ec_name) == classes.end());
    }

    auto push(const std::string &ec_name) -> void
    {
        classes[ec_name] = EventClass::make_event_class(ec_name);
    }

    auto merge_inplace(std::unique_ptr<EventClassContainer> &&other) -> void;

    static auto serialize_to_root(EventClassContainer &cont,
                                  const std::string &ouput_file_path,
                                  const std::string &process_name,
                                  const std::string &process_group,
                                  const std::string &xsec_order,
                                  const std::string &year,
                                  bool is_data) -> void;
};

// Utils
inline auto set_of_bins(const EventClassHistogram &hist1, const EventClassHistogram &hist2) -> std::set<std::size_t>
{
    std::set<std::size_t> result;

    for (const auto &item : hist1.counts)
    {
        result.insert(item.first);
    }

    for (const auto &item : hist2.counts)
    {
        result.insert(item.first);
    }

    return result;
}

inline auto set_of_classes(const EventClassContainer &cont1, const EventClassContainer &cont2) -> std::set<std::string>
{
    std::set<std::string> result;

    for (const auto &item : cont1.classes)
    {
        result.insert(item.first);
    }

    for (const auto &item : cont2.classes)
    {
        result.insert(item.first);
    }

    return result;
}
#endif // !EVENT_CLASS_HPP
