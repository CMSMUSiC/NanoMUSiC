#ifndef EVENT_CLASS_FACTORY_HPP
#define EVENT_CLASS_FACTORY_HPP

#include "Histograms.hpp"
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"
#include "TEfficiency.h"
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <memory>
#include <optional>
#include <string_view>

#include "ObjectFactories/music_objects.hpp"
#include "Shifts.hpp"
#include "TriggerMatch.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

class EventClass
{
  public:
    TH1F h_counts;
    TH1F h_invariant_mass;
    TH1F h_sum_pt;
    TH1F h_met;

    bool has_met = false;

    static constexpr float count_bin_center = 0.5;
    static constexpr double min_bin_width = 10.;

    // std::map<std::string, int> countMap;
    // Shifts::Variations shift;

    static auto make_bin_limits(const std::map<std::string, int> &countMap)
        -> std::pair<std::vector<double>, std::vector<double>>;

    // EventClass() = default;

    EventClass(const std::string &_class_name,
               //    const std::map<std::string, int> &_countMap,
               const std::vector<double> &limits,
               const std::vector<double> &limits_met,
               const Shifts::Variations _shift,
               const std::string &_sample,
               const std::string &_year,
               const std::string &_process_group,
               const std::string &_xs_order);

    auto update_name(const std::string &_class_name,
                     const Shifts::Variations shift,
                     const std::string &_sample,
                     const std::string &_year,
                     const std::string &_process_group,
                     const std::string &_xs_order) -> void;

    auto fill(std::pair<std::size_t, const MUSiCObjects &> this_muons,
              std::pair<std::size_t, const MUSiCObjects &> this_electrons,
              std::pair<std::size_t, const MUSiCObjects &> this_taus,
              std::pair<std::size_t, const MUSiCObjects &> this_photons,
              std::pair<std::size_t, const MUSiCObjects &> this_bjets,
              std::pair<std::size_t, const MUSiCObjects &> this_jets,
              std::pair<std::size_t, const MUSiCObjects &> this_met,
              double weight) -> void;

    auto save_histo(TH1 histo) -> void;
    auto save_histo(TH2 histo) -> void;

    // auto dump_outputs(std::unique_ptr<TFile> &output_file) -> void;
    auto dump_outputs(std::unique_ptr<TFile> &output_file, Shifts::Variations shift) -> void;
};

auto make_event_class_name(
    std::pair<std::size_t, std::size_t> muon_counts,
    std::pair<std::size_t, std::size_t> electron_counts,
    std::pair<std::size_t, std::size_t> tau_counts,
    std::pair<std::size_t, std::size_t> photon_counts,
    std::pair<std::size_t, std::size_t> jet_counts,
    std::pair<std::size_t, std::size_t> bjet_counts,
    std::pair<std::size_t, std::size_t> met_counts,
    const std::optional<std::unordered_map<std::string, std::optional<TriggerMatch>>> &trigger_matches)
    -> std::tuple<std::optional<std::string>, std::optional<std::string>, std::optional<std::string>>;

inline auto get_pt(const Math::PtEtaPhiMVector &obj) -> float
{
    return obj.pt();
};

#endif // !EVENT_CLASS_FACTORY_HPP