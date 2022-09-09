#ifndef ECSCANNER_HH
#define ECSCANNER_HH

#include <vector>

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <streambuf>
#include <vector>

#include <TH2.h>
#include <TH3.h>
#include <TStyle.h>

#define RAPIDJSON_HAS_STDSTRING 1
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#include "document.h"
#pragma GCC diagnostic pop

#include "ConvolutionLookup.hh"
#include "Dicer.hh"
#include "MCBin.hh"
#include "Profiler.hh"
#include "ScanResult.hh"

namespace rs = rapidjson;

class ECScanner
{
  public:
    ECScanner(int &rounds, int &startRound);
    ~ECScanner();
    void finalize();
    // public member functions
    void readInputJson(const std::string jsonFilePath);
    void readLookupTable(const std::string lookupTablePath = "");
    void writeOutputFiles(const std::string outputDirectory);

    void readMCBinInfo();
    void readSignalBinInfo();
    void readDataBinInfo();

    void readSystematicShiftsFile(const std::string filename);

    int getMaxFilledBin();

    void diceMcPseudoData(const unsigned int round);
    void diceSignalPseudoData(const unsigned int round);

    void findRoI(const std::string scoreType, const bool filtered);
    void findRoI();

    double calcPvalMUSiC(const MCBin &bin, const double data) const;
    double calcSignificance(const MCBin &bin, const double data) const;

    bool isDataScan() const;
    bool isSignalScan() const;
    unsigned int getDicingRounds() const;
    unsigned int getFirstDicingRound() const;

  private:
    // has to come before SkipReason is used as parameter type
    enum class SkipReason : int
    {
        SkipReasonStart = -100,

        MIN_REGION_WIDTH,
        DATA_NO_MC,
        MC_NO_DATA,
        EMPTY_REGION,
        SIGMA_THRESHOLD,
        EMPTY_BIN,
        LEADING_BG_MISSING,
        LEADING_BG_FLUCTUATES,
        LOW_MC_STAT,         // = -91
        MC_NEGATIVE,         // = -90
        HIGH_REL_UNCERT,     // = -89
        LEADING_BG_NEGATIVE, // = -88
        BG_NEGATIVE,         // = -87
        LOW_MC_YIELD,

        SkipReasonEnd // = -86
    };

    // private member functions
    void initScoreFuncMap();
    static std::vector<MCBin> readMCBinArray(const rs::Value &jsonArray, MCBin *integralBinOut = nullptr);
    void initFilterFuncMap();
    void significanceFilter();
    bool vetoRegion(const MCBin &mcbin, double data, std::vector<MCBin>::iterator const &startMCBinIter,
                    std::vector<MCBin>::iterator const &endMCBinIter,
                    std::vector<double>::iterator const &startDataBinIter,
                    std::vector<double>::iterator const &endDataBinIter,
                    std::vector<MCBin>::iterator const &maxMCBinIter, bool isIntegral = false);

    template <typename T> void checkAndSetConfig(const std::string name, T &config);

    void fillRegionControlPlot(const MCBin &mcbin, const double value);
    void fillRegionControlPlot(const MCBin &mcbin, const SkipReason reason);

    static bool scoreSort(const ScanResult &r1, const ScanResult &r2)
    {
        return r1.getScore() > r2.getScore();
    };
    static MCBin constructNeighborhood(std::vector<MCBin>::iterator iter, const int width,
                                       std::vector<MCBin>::iterator minIter, std::vector<MCBin>::iterator maxIter);

    static rs::Document readJsonDocument(const std::string filename);
    static void writeJsonDocument(const std::string filename, const rs::Document &document);
    static std::string replaceExtension(const std::string filename, const std::string newExtension);

    // private variables
    std::string m_ECName;
    std::string m_distribution;
    std::string m_submissionHash;
    std::string m_submissionKey;

    rs::Document m_jsonDocument;
    std::string m_lastJsonFilePath;

    enum class ScanType
    {
        unknown = 0,
        data,
        mc,
        signal,
    };
    ScanType m_scanType;

    unsigned int m_numDicingRounds = 0;
    unsigned int m_firstDicingRound = 0;

    bool m_doFilter = false;

    //  Data (MC) Bins contains the (diced) measured number of (pseudo-) data points
    std::vector<MCBin> m_mcBins;
    std::vector<MCBin> m_signalBins;
    std::vector<double> m_dataBins;

    double m_totalMcEvents;
    double m_totalMcUncert;

    // chosen score function
    std::string m_scoreFunc;

    // function map for function calls by string for compare score
    // which is used to score candidates for the region of interest
    std::map<std::string, std::function<double(MCBin &, double &)>> m_scoreFuncMap;

    // chosen filtering function
    std::string m_filterFunc;
    // Number of scan results that should be considered for full scan when filtering
    int m_nFilterRegions = 1e9;
    // function map for filter functions. A filter function is expected
    // to construct regions and replace the existing vectors m_mcBins and m_dataBins
    // with versions, which contain only the considered candidate regions
    std::map<std::string, std::function<void()>> m_filterFuncMap;

    // boolean to determine if only integral scan should be performed
    bool m_integralScan = false;

    // boolean to determine whether the lookup-table should be skipped
    bool m_skipLookupTable = false;

    // minimalNumber of bins per region
    int m_minRegionWidth = 1;

    // minimal MC event/error ratio
    float m_coverageThreshold = 0.f;

    // exclude insignificant regions
    float m_sigmaThreshold = 0.f;

    // skip low stats treatment
    bool m_noLowStatsTreatment = false;

    // Threshold for the realtive statistical uncert in region for low stats treatment
    float m_thresholdLowStatsUncert = 0.6;

    // width for neighborhood regins in low stat treatment
    int m_widthLowStatsRegions = 4;

    bool m_skipNeighborhoodCheck = false;
    // Quantile for processes to be considered as dominant in neighborhood
    float m_thresholdLowStatsDominant = 0.95;

    // minimum region yield for a region to be considered
    float m_thresholdRegionYield = 1e-6;

    // relative fraction a dominant process might fluctuate compared to the
    // fraction it contiributes in the regions neighborhood
    float m_thresholdLowStatsDominatFraction = 0.15;

    LookupTable m_lookupTable;

    Dicer m_dicer;

    PriorMode m_p_prior = NORMAL_PRIOR;

    // vector for scan results
    std::vector<ScanResult> m_scanResults;
    std::vector<ScanResult> m_scanResultsCache;

    // debugging / control plots

    // these are mutable so we can access then from const methods!
    mutable Profiler m_dicingProfiler;
    mutable Profiler m_roiFindingProfiler;
    mutable Profiler m_pValueProfiler;

    mutable std::map<const char *, unsigned long long> m_regionStatistics;
    TH2F *m_regionControlPlot = nullptr;
};

#endif // ECSCANNER_HH
