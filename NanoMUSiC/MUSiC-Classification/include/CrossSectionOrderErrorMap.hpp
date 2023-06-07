#ifndef CROSSSECTIONORDERERRORMAP_HPP
#define CROSSSECTIONORDERERRORMAP_HPP

// #include "Tools/Tools.hpp"

#include <string>
#include <unordered_map>
#include <vector>

class CrossSectionOrderErrorMap
{
  public:
    std::vector<std::string> const availableOrders = {"LO", "NLO", "NNLO", "NNNLO", "NLO_W", "N3LO"};
    std::unordered_map<std::string, double> weightMap =
        {{"LO", 0.5}, {"NLO", 0.0}, {"NNLO", 0.0}, {"NNNLO", 0.0}, {"NLO_W", 0.1}, {"N3LO", 0.0}};
    CrossSectionOrderErrorMap()
    {
    }
    // implement some map like functions
    double at(std::string key)
    {
        return weightMap.at(key);
    };
    double &operator[](std::string key)
    {
        return weightMap[key];
    };
    //~ const double& operator[]( std::string key ) const { return weightMap[key]; };
    void emplace(std::string key, double value)
    {
        weightMap.emplace(key, value);
    };
    std::unordered_map<std::string, double>::iterator end()
    {
        return weightMap.end();
    }
    std::unordered_map<std::string, double>::iterator find(std::string key)
    {
        return weightMap.find(key);
    }
};

#endif /*CROSSSECTIONORDERERRORMAP_HPP*/