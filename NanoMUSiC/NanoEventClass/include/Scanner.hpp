#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <unordered_map>

#include "fmt/format.h"

#include "Distribution.hpp"
#include "NanoEventClass.hpp"
#include "indicators.hpp"
#include "roothelpers.hpp"

#include "ROOT/RVec.hxx"

#include "ConvolutionComputer.hpp"
#include "ConvolutionLookup.hpp"
#include "roothelpers.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;

enum ScanStatus
{
    SUCCESS,
    FAILED
};

enum DataSource
{
    DATA,
    TOY,
    SIGNAL
};

class ScanResult
{
  public:
    double p_value;
    double lower_edge;
    double upper_edge;
    ScanStatus scan_status;
    DataSource data_source;

    static auto header() -> std::string
    {
        return "p_value,lower_edge,upper_edge,scan_status,data_source";
    }

    auto to_string() const -> std::string
    {
        auto scan_status_str = "SUCCESS";
        if (scan_status == ScanStatus::FAILED)
        {
            scan_status_str = "FAILED";
        }
  
        auto data_source_str = "DATA";
        if (data_source == DataSource::TOY)
        {
            data_source_str = "TOY";
        }
        if (data_source == DataSource::SIGNAL)
        {
            data_source_str = "SIGNAL";
        }

        return fmt::format("{},{},{},{},{}", p_value, lower_edge, upper_edge, scan_status_str, data_source_str);
    }
};

class Region
{
  public:
    double data;
    double mc;
    double sigma;
};

class Scanner
{
  public:
    LookupTable m_lookup_table;

    Scanner();
    static auto make_region(const std::string &distribution_name,
                            const RVec<double> data_bins,
                            const RVec<double> mc_bins,
                            const RVec<double> sigma_bins,
                            std::size_t lower_index,
                            std::size_t upper_index) -> std::optional<Region>;
    auto compute_p_value(const double data,
                         const double mc,
                         const double sigma,
                         const PriorMode prior,
                         const int debug_level) const -> double;
    auto roi_scan(const std::string &distribution_name, const RVec<double> &data, DataSource data_source) const
        -> ScanResult;
    auto sample(const std::unordered_map<std::string, std::vector<double>> &shifts, std::size_t shift_index) const
        -> RVec<double>;
    auto scan(const std::shared_ptr<Distribution> &distribution,
              const std::unordered_map<std::string, std::vector<double>> &shifts,
              unsigned int n_shifts) -> std::vector<ScanResult>;
};

#endif // SCANNER_HPP
