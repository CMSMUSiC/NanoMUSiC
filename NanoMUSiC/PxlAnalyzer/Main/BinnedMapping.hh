#ifndef BINNEDMAPPING
#define BINNEDMAPPING

// Class to map key -> values.
// This became necessary because there are some correction factors needed for
// different purposes. E.g. there are jet resolution corrections mapped in eta
// ranges.
//
// The idea is the following:
// In a given 'config' file, the values stored in 'keyName' are considered
// 'bin edges' (e.g. eta bins) and values stored in 'valueName' are considered
// the values to be mapped to these bins.
// 'absBinName' is the name of the variable storing if the 'bins' are meant to
// be absolute (e.g. absolute eta values).
//
// Different numbers of 'values' are considered differently:
// In both cases, if bin are absolute or not:
//    - If the number of 'values' is one less than the number of bin edges',
//      the first 'bin edge' is the lower bound and the last 'bin edge' is the
//      upper bound and no values of 'key' smaller/larger than those are
//      allowed.
// If bins are absolute:
//    - If the number of 'values' is the same as the number of 'bin edges',
//      the last entry of the 'values' is filled as the "overflow" bin and is
//      used for each 'key value' between "last bin edges" and infinity".
// If bins are not absolute
//    - If the number of 'values' is one larger than the number of 'bin edges',
//      the last entry in 'values' is filled as the "overflow" bin and is used
//      for each 'key value' between "last bin edges" and infinity",
//      AND the first entry in 'values' is filled as the "underflow" bin and is
//      used for each 'key value' between "-infinity and first bin edge".
// No other combinations are supported.

#include <string>
#include <vector>

#include "TProfile.h"

namespace Tools
{
class MConfig;
}

class BinnedMapping
{
  public:
    BinnedMapping(Tools::MConfig const &config, std::string const &keyName, std::string const &valueName,
                  std::string const &absBinName = "");
    ~BinnedMapping()
    {
    }

    // Get the value in the bin corresponding to key.
    double getValue(double const key) const;

  private:
    // Read the bin edges from config and sort them!
    std::vector<double> initBinEdges(Tools::MConfig const &config, std::string const &keyName) const;

    // Fill the TProfile bins with the values given in the config file.
    // The "names" of the keys and values is used to name the histogram in a
    // meaningful way.
    TProfile initKeyValueMap(Tools::MConfig const &config, std::string const &keyName,
                             std::string const &valueName) const;

    std::vector<double> const m_bin_edges;
    std::vector<double> const m_bin_values;

    bool const m_abs_bins;
    TProfile const m_key_value_map;
};

#endif /*BINNEDMAPPING*/
