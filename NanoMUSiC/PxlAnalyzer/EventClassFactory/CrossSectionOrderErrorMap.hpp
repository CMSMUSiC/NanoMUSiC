#ifndef CROSSSECTIONORDERERRORMAP
#define CROSSSECTIONORDERERRORMAP

#include "Tools/MConfig.hpp"
#include "Tools/Tools.hpp"

class CrossSectionOrderErrorMap
{
  public:
    std::vector<std::string> const availableOrders;
    std::map<std::string, double> weightMap;
    CrossSectionOrderErrorMap(Tools::MConfig const &cfg)
        : availableOrders(Tools::splitString<std::string>(cfg.GetItem<std::string>("General.Syst.Orders"), true))
    {
        for (auto &order : availableOrders)
        {
            weightMap.emplace(order, cfg.GetItem<double>("General.Syst." + order));
        }
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
    std::map<std::string, double>::iterator end()
    {
        return weightMap.end();
    }
    std::map<std::string, double>::iterator find(std::string key)
    {
        return weightMap.find(key);
    }
};

#endif /*CROSSSECTIONORDERERRORMAP*/
