#ifndef NANOEVENTCLASS_HPP
#define NANOEVENTCLASS_HPP

#include <memory>
#include <unordered_map>

#include "fmt/format.h"

#include "TFile.h"
#include "TH1.h"
#include "TROOT.h"
#include "TSystem.h"

using namespace ROOT;

class NanoEventClass
{
  private:
    static constexpr unsigned int num_histo_name_parts = 7;
    struct NanoEventHisto
    {
        const std::string process_group;
        const std::string xs_order;
        const std::string sample;
        const std::string year;
        const std::string shift;
        const std::string histo_name;
        TH1F *histogram;
        const bool is_data;
    };

    const std::string m_class_name;
    std::vector<NanoEventHisto> m_histograms;

    const std::string m_file_path;
    std::unique_ptr<TFile> m_file;
    bool m_is_data;
    bool m_debug;

    auto make_nano_event_histo(const std::string &histo_full_name) -> NanoEventHisto;

    auto split_histo_name(std::string histo_full_name, const std::string delimiter = "]_[")
        -> std::tuple<std::string, std::string, std::string, std::string, std::string, std::string, std::string>;

    // Function to check if a string matches a pattern with *
    static auto match_pattern(const std::string &str, const std::string &pattern) -> bool;

  public:
    NanoEventClass(const std::string &class_name, const std::string &file_path, bool is_data, bool debug = false);

    auto filter_histos(const std::string &process_group,
                       const std::string &xs_order,
                       const std::string &sample,
                       const std::string &year,
                       const std::string &shift,
                       const std::string &histo_name) -> std::vector<NanoEventHisto>;

    static auto make_histogram_full_name(const std::string &class_name,
                                         const std::string &process_group,
                                         const std::string &xs_order,
                                         const std::string &sample,
                                         const std::string &year,
                                         const std::string &shift,
                                         const std::string &histo_name) -> std::string;
};

#endif // NANOEVENTCLASS_HPP
