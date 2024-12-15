#include "ScanResult.hpp"
#include <algorithm>
#include <numeric>

#include "json.hpp"
using json = nlohmann::json;

ScanResult::ScanResult(const MCBin &mcbin,
                       double data,
                       double score,
                       bool integralScan,
                       bool skippedScan,
                       const std::vector<double> &dicedData,
                       double totalMc,
                       double totalMcUncert)
    : mcbin(mcbin),
      data(data),
      totalMc(totalMc),
      totalMcUncert(totalMcUncert),
      score(score),
      integralScan(integralScan),
      skippedScan(skippedScan),
      dicedData(dicedData)
{
    totalData = std::accumulate(dicedData.begin(), dicedData.end(), 0.0);
}

json ScanResult::jsonValue(bool verbose) const
{
    json resultObject;

    resultObject["mcEvents"] = mcbin.getTotalMcEvents();
    resultObject["mcStatUncert"] = mcbin.getTotalMcStatUncert();
    resultObject["mcSystUncert"] = mcbin.getTotalMcSystUncert();
    resultObject["mcTotalUncert"] = mcbin.getTotalMcUncert();
    resultObject["dataEvents"] = data;
    // resultObject["totalMcEventsEC"] = totalMc;
    // resultObject["totalMcUncertEC"] = totalMcUncert;
    // resultObject["totalDataEventsEC"] = totalData;
    resultObject["lowerEdge"] = mcbin.lowerEdge;
    resultObject["width"] = mcbin.width;
    resultObject["CompareScore"] = score;
    // resultObject["integralScan"] = integralScan;
    resultObject["skippedScan"] = skippedScan;

    if (verbose)
    {
        // Add systematic errors
        json mcSysUncertsObj;

        for (size_t i = 0; i < mcbin.mcSysUncerts.size(); i++)
        {
            const double upValue = mcbin.mcSysUncerts[i].first;
            const double downValue = mcbin.mcSysUncerts[i].second;
            const std::string name = mcbin.mcSysUncertNames[i];

            if (upValue != downValue)
            {
                // Non-symmetric errors, append -Down and -Up to the systematic name
                const std::string downName = name + "Down";
                mcSysUncertsObj[downName] = downValue;

                const std::string upName = name + "Up";
                mcSysUncertsObj[upName] = upValue;
            }
            else
            {
                // Symmetric error, just add to document
                mcSysUncertsObj[name] = upValue;
            }
        }
        // resultObject["mcSysUncerts"] = mcSysUncertsObj;

        json mcEventsPerProcessObj;
        json mcStatUncertsObj;

        for (size_t i = 0; i < mcbin.mcEventsPerProcessGroup.size(); i++)
        {
            const std::string name = mcbin.mcProcessGroupNames[i];
            const double events = mcbin.mcEventsPerProcessGroup[i];
            const double statUncert = mcbin.mcStatUncertPerProcessGroup[i];
            mcEventsPerProcessObj[name] = events;
            mcStatUncertsObj[name] = statUncert;
        }
        // resultObject["mcEventsPerProcessGroup"] = mcEventsPerProcessObj;
        // resultObject["mcStatUncertPerProcessGroup"] = mcStatUncertsObj;

        json dicedDataArray;
        for (double n : dicedData)
        {
            dicedDataArray.push_back(n);
        }
        // resultObject["dicedData"] = dicedDataArray;
    }

    return resultObject;
}

void ScanResult::writeCsvHeader(std::ostream &out)
{
    out << "roi_lower_edge,"
        << "roi_width,"
        << "mc_events,"
        << "mc_uncert,"
        << "data_events,"
        << "ec_total_mc_events,"
        << "ec_total_mc_uncert,"
        << "ec_total_data_events,"
        << "score,"
        << "integral,"
        << "skipped,";
}

void ScanResult::writeCsvLine(std::ostream &out) const
{
    out << mcbin.lowerEdge << "," << mcbin.width << "," << mcbin.getTotalMcEvents() << "," << mcbin.getTotalMcUncert()
        << "," << data << "," << totalMc << "," << totalMcUncert << "," << totalData << "," << score << ","
        << integralScan << "," << skippedScan << ",";
}
