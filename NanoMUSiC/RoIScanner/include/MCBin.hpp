#ifndef MCBIN_HH
#define MCBIN_HH

#include <cmath>
#include <iostream>
#include <map>
#include <ostream>
#include <unordered_map>
#include <vector>

class MCBin
{
    /* EventErrorCollection holds various information about the events and errors of one bin.
     * All information are stored in a way which allows to easily add a number of EventErrorCollections, resulting in
     * the events and errors for the region of interest formed by all added bins.
     */
  public:
    typedef std::vector<std::string> name_vector;
    typedef std::vector<std::pair<double, double>> uncert_vector;
    typedef std::vector<double> yield_vector;

    MCBin();

    // overloaded constructor
    MCBin(const yield_vector &mcEventsPerProcessGroup,
          const yield_vector &mcStatUncertPerProcessGroup,
          const name_vector &mcProcessGroupNames,
          double lowerEdge,
          double width,
          const uncert_vector &mcSysUncerts,
          const name_vector &mcSysUncertNames);

    // Add another MCBin to this MCBin, adding all events and errors in the right way.
    MCBin &operator+=(const MCBin &add);

    bool operator!=(const MCBin &comp) const;
    // get the number of events, either with or without CL

    // output all kinds of stuff
    friend std::ostream &operator<<(std::ostream &out, const MCBin &bin);

    // function to determine if no entries exists ( also in systematics)
    bool isEmpty() const;

    void clear();

    double getTotalMcUncert() const;
    double getTotalMcEvents() const;
    double getTotalMcStatUncert() const;
    double getTotalMcSystUncert() const;

    std::vector<size_t> leadingBackgrounds(const double threshold) const;
    std::map<size_t, float> leadingBackgroundsFractions(const double threshold) const;

    // Number of MC events (weighted) in one bin (or region)
    // does not include CL
    yield_vector mcEventsPerProcessGroup;
    // statistical error by contribution in this bin
    yield_vector mcStatUncertPerProcessGroup;
    // vector containing process group names,
    // must stay constant after initialization
    name_vector mcProcessGroupNames;

    double lowerEdge = 0;
    double width = 0;
    // Number of MC events (weighted) per error unit in one bin (or region)
    // does not include CL
    //~ std::vector< double > ErrorUnitEvents;

    // Absolute errors
    // uncorrelated, so need to be added in quadrature
    // We need a symmetric uncertainty to dice and to compute the p-value.
    // The JES errors are stored signed. They may point into the same "direction"
    // (i.e. have the same sign). But this is the best way we have to treat them
    // by now.
    uncert_vector mcSysUncerts;
    // vector containing systematic names
    name_vector mcSysUncertNames;

    static double symmetrizeError(const std::pair<double, double> error);

  private:
    bool initialized = false;

    // Use this to mark a cache as "dirty" such that it will be recomputed
    // next time.
    constexpr static double DIRTY_CACHE = -1;

    // using C++11 initialization here
    mutable double totalUncertCache = DIRTY_CACHE;
    mutable double totalMcEventsCache = DIRTY_CACHE;
    mutable double totalMcStatUncertCache = DIRTY_CACHE;
};

#endif // MCBIN_HH
