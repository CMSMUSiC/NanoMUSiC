#include "BinnedMapping.hpp"

#include <cmath>
#include <sstream>

#include "Tools/MConfig.hpp"
#include "Tools/Tools.hpp"

BinnedMapping::BinnedMapping(Tools::MConfig const &config,
                             std::string const &keyName,
                             std::string const &valueName,
                             std::string const &absBinName)
    : m_bin_edges(initBinEdges(config, keyName)),
      m_bin_values(Tools::splitString<double>(config.GetItem<std::string>(valueName))),

      // Check (in the config file) if the bins are meant to be absolute numbers.
      m_abs_bins(config.GetItem<bool>(absBinName, false)),

      m_key_value_map(initKeyValueMap(config, keyName, valueName))
{
}

std::vector<double> BinnedMapping::initBinEdges(Tools::MConfig const &config, std::string const &keyName) const
{
    std::vector<double> bin_edges(Tools::splitString<double>(config.GetItem<std::string>(keyName)));

    std::sort(bin_edges.begin(), bin_edges.end());

    return bin_edges;
}

TProfile BinnedMapping::initKeyValueMap(Tools::MConfig const &config,
                                        std::string const &keyName,
                                        std::string const &valueName) const
{
    TString const name = keyName + "_" + valueName;
    TString const title = keyName + " vs. " + valueName;

    int const num_bins = m_bin_edges.size();
    int const num_values = m_bin_values.size();

    // HACK: Using TProfile as mapping from key -> value.
    // It has some advantages with respect to TH1, e.g. Set/GetBinEntries().
    TProfile keyValueMap(name, title, num_bins - 1, &m_bin_edges.at(0));

    if (num_bins - num_values == 1)
    {
        for (int bin = 1; bin <= keyValueMap.GetNbinsX(); ++bin)
        {
            keyValueMap.SetBinContent(bin, m_bin_values.at(bin - 1));
            keyValueMap.SetBinEntries(bin, 1);
        }
    }
    else if (num_bins == num_values)
    {
        if (m_abs_bins)
        {
            for (int bin = 1; bin <= keyValueMap.GetNbinsX() + 1; ++bin)
            {
                keyValueMap.SetBinContent(bin, m_bin_values.at(bin - 1));
                keyValueMap.SetBinEntries(bin, 1);
            }
        }
        else
        {
            std::stringstream err;
            err << "[ERROR] (BinnedMapping) In config file: '";
            err << config.GetConfigFilePath();
            err << "': Not supported number of values (" << valueName << ") ";
            err << "for absolute keys.";
            throw Tools::config_error(err.str());
        }
    }
    else if (num_values - num_bins == 1)
    {
        if (not m_abs_bins)
        {
            for (int bin = 0; bin <= keyValueMap.GetNbinsX() + 1; ++bin)
            {
                keyValueMap.SetBinContent(bin, m_bin_values.at(bin - 1));
                keyValueMap.SetBinEntries(bin, 1);
            }
        }
        else
        {
            std::stringstream err;
            err << "[ERROR] (BinnedMapping) In config file: '";
            err << config.GetConfigFilePath();
            err << "': Not supported number of values (" << valueName << ") ";
            err << "for non-absolute keys.";
            throw Tools::config_error(err.str());
        }
    }
    else
    {
        std::stringstream err;
        err << "[ERROR] (BinnedMapping) In config file: '";
        err << config.GetConfigFilePath();
        err << "': Unsupported number of values (" << valueName << ").";
        throw Tools::config_error(err.str());
    }

    return keyValueMap;
}

double BinnedMapping::getValue(double const key) const
{
    double const used_key = m_abs_bins ? std::fabs(key) : key;
    int const bin = m_key_value_map.FindFixBin(used_key);

    if (m_key_value_map.IsBinOverflow(bin) and not m_key_value_map.GetBinEntries(bin))
    {
        std::stringstream err;
        err << "[WARNING] (BinnedMapping): In getValue(...): ";
        err << "Unsupported key value: 'key = " << key << "'. ";
        err << "Please invesigate!" << std::endl;
        err << "Using last value in map (border " << m_bin_edges.back() << ")";
        std::cerr << err.str() << std::endl;
        return m_key_value_map.GetBinEntries(m_bin_edges.back());
    }

    if (m_key_value_map.IsBinUnderflow(bin) and not m_key_value_map.GetBinEntries(bin))
    {
        std::stringstream err;
        err << "[ERROR] (BinnedMapping): In getValue(...): ";
        err << "Unsupported key value: 'key = " << key << "'. ";
        err << "Please invesigate!";
        throw Tools::value_error(err.str());
    }

    return m_key_value_map.GetBinContent(bin);
}
