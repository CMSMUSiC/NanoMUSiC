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
#include "TriggerMatch.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

class EventClass
{
  private:
  public:
    std::string output_path;

    TH1F h_counts;
    TH1F h_invariant_mass;
    TH1F h_sum_pt;
    TH1F h_met;

    bool has_met;

    static constexpr float count_bin_center = 0.5;

    double min_bin_width;
    std::map<std::string, int> countMap;
    std::string shift;

    EventClass() = default;

    EventClass(const std::string &_class_name,
               const std::string &_output_path,
               const std::map<std::string, int> &_countMap,
               const std::string _shift,
               const std::string &_sample,
               const std::string &_year,
               const std::string &_process_group,
               const std::string &_xs_order);

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

    auto dump_outputs(std::unique_ptr<TFile> &output_file) -> void;
};

auto make_event_class_name(std::pair<std::size_t, std::size_t> muon_counts,
                           std::pair<std::size_t, std::size_t> electron_counts,
                           std::pair<std::size_t, std::size_t> tau_counts,
                           std::pair<std::size_t, std::size_t> photon_counts,
                           std::pair<std::size_t, std::size_t> jet_counts,
                           std::pair<std::size_t, std::size_t> bjet_counts,
                           std::pair<std::size_t, std::size_t> met_counts,
                           const std::unordered_map<std::string, std::optional<TriggerMatch>> &trigger_matches)
    -> std::tuple<std::optional<std::string>, std::optional<std::string>, std::optional<std::string>>;

inline auto get_pt(const Math::PtEtaPhiMVector &obj) -> float
{
    return obj.pt();
};

#endif // !EVENT_CLASS_FACTORY_HPP