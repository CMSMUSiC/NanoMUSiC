#ifndef NANOEVENTCLASS_HPP
#define NANOEVENTCLASS_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>

#include "fmt/format.h"

#include "TFile.h"
#include "TH1.h"
#include "TROOT.h"
#include "TSystem.h"

using namespace ROOT;

class NanoEventHisto
{
  public:
    const std::string class_name = "";
    const std::string process_group = "";
    const std::string xs_order = "";
    const std::string sample = "";
    const std::string year = "";
    const std::string shift = "";
    const std::string histo_name = "";
    std::shared_ptr<TH1F> histogram = std::make_shared<TH1F>();
    const bool is_data = false;

    static constexpr unsigned int num_histo_name_parts = 7;

    static auto split_histo_name(std::string histo_full_name, const std::string delimiter = "]_[")
        -> std::tuple<std::string, std::string, std::string, std::string, std::string, std::string, std::string>;

    static auto make_nano_event_histo(const std::string &histo_full_name, TH1F *histo_ptr) -> NanoEventHisto;

    static auto make_histogram_full_name(const std::string &class_name,
                                         const std::string &process_group,
                                         const std::string &xs_order,
                                         const std::string &sample,
                                         const std::string &year,
                                         const std::string &shift,
                                         const std::string &histo_name) -> std::string;

    auto to_string() const -> std::string;

    // ~NanoEventHisto();
};

class NanoEventClass
{
  public:
    const std::string m_class_name = "";
    std::vector<NanoEventHisto> m_counts = {};
    std::vector<NanoEventHisto> m_invariant_mass = {};
    std::vector<NanoEventHisto> m_sum_pt = {};
    std::vector<NanoEventHisto> m_met = {};
    double m_data_count = 0.;
    double m_mc_count = 0.;
    bool m_is_valid = false;

    static constexpr double min_mc_count = 0.1;
    static constexpr double min_data_count = 1.;

    // Function to check if a string matches a pattern with *
    // static auto match_pattern(const std::string &str, const std::string &pattern) -> bool;

    NanoEventClass();

    NanoEventClass(const std::string &class_name,
                   const std::vector<NanoEventHisto> &counts,
                   const std::vector<NanoEventHisto> &invariant_mass,
                   const std::vector<NanoEventHisto> &sum_pt,
                   const std::vector<NanoEventHisto> &met);

    auto to_string() const -> std::string;

    static auto get_class_name(std::string histo_full_name, const std::string delimiter = "]_[") -> std::string;

    static auto make_event_class_name(std::pair<std::size_t, std::size_t> muon_counts,
                                      std::pair<std::size_t, std::size_t> electron_counts,
                                      std::pair<std::size_t, std::size_t> tau_counts,
                                      std::pair<std::size_t, std::size_t> photon_counts,
                                      std::pair<std::size_t, std::size_t> jet_counts,
                                      std::pair<std::size_t, std::size_t> bjet_counts,
                                      std::pair<std::size_t, std::size_t> met_counts)
        -> std::tuple<std::optional<std::string>, std::optional<std::string>, std::optional<std::string>>;
};

class NanoEventClassCollection
{
  public:
    std::unordered_map<std::string, NanoEventClass> m_classes = {};

    auto ClassesWithData(const std::vector<std::string> &root_file_paths) -> std::set<std::string>;

    NanoEventClassCollection(const std::vector<std::string> &root_file_paths,
                             const std::vector<std::string> &class_patterns);

    auto get_classes() const -> std::vector<std::string>;
    auto get_class(const std::string &class_name) -> NanoEventClass &;
};

#endif // NANOEVENTCLASS_HPP
