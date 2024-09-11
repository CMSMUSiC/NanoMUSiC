#ifndef SCANRESULT_HH
#define SCANRESULT_HH

#include <ostream>

#include "json.hpp"
using json = nlohmann::json;

#include "MCBin.hpp"


class ScanResult
{
  public:
    ScanResult(const MCBin &mcbin = MCBin(),
               double data = 0,
               double score = 0,
               bool integralScan = false,
               bool skippedScan = false,
               const std::vector<double> &dicedData = std::vector<double>(),
               double totalMc = 0,
               double totalMcUncert = 0);

    // Significance comparison:
    // returns true if this result is more significant than 'other'
    bool operator<(const ScanResult &other) const
    {
        return score < other.score;
    }

    // serialize this to json 
    json jsonValue(bool verbose = true) const;

    static void writeCsvHeader(std::ostream &out);
    void writeCsvLine(std::ostream &out) const;

    const MCBin &getMcBin() const
    {
        return mcbin;
    }
    double getScore() const
    {
        return score;
    }
    double getData() const
    {
        return data;
    }
    double getTotalMcEvents() const
    {
        return totalMc;
    }
    double getTotalMcUncert() const
    {
        return totalMcUncert;
    }
    double getTotalData() const
    {
        return totalData;
    }
    bool isIntegralScan() const
    {
        return integralScan;
    }
    bool isSkippedScan() const
    {
        return skippedScan;
    }

  private:
    // MC events (and uncertainties) in the RoI
    MCBin mcbin;

    // data-events in the RoI
    double data;

    // total data events in the event class
    double totalData;
    // total mc events in the event class
    double totalMc;
    double totalMcUncert;

    // p-value
    double score;

    // Was the RoI determined through a integral scan?
    bool integralScan;

    bool skippedScan;
    // data which has been diced/used for the scan resulting in this
    // RoI (especially important for signal scans)
    std::vector<double> dicedData;
};

#endif // SCANRESULT_HH
