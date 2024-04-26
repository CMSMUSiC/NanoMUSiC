
#ifndef BIN_LIMITS
#define BIN_LIMITS

#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

enum class ObjectNames
{
    Muon,
    Electron,
    Photon,
    Tau,
    bJet,
    Jet,
    MET,
};

namespace ResolutionsFuncs
{
// equation and numbers from the top left plot in figure 2 of AN-2010/059
inline double muon(double pt)
{
    if (pt < 0)
    {
        throw std::range_error("pt must be > 0!");
    }
    return 0.016 * pt + 0.00015 * pt * pt;
}

// Low energy:
// https://twiki.cern.ch/twiki/bin/view/CMSPublic/EGMElectronsMoriond2013#Electron_Resolution
// High energy
// Equation taken from Z' note (2016_404_v8) and approximating back-to-back decay (sigma_m = 2 sigma_E)
inline double electron(double const energy)
{
    if (energy < 0)
    {
        throw std::range_error("energy must be > 0!");
    }

    if (energy < 100.)
    {
        return energy * 0.025;
    }

    double const S_square = 106.09;
    double const N_square = 100.;
    double const C_square = 0.7569;

    double m_eff = 2 * energy;
    double m_rel_res = std::sqrt(S_square / m_eff + N_square / std::pow(m_eff, 2) + C_square) / 100.;
    return 2 * m_rel_res * m_eff;
}

// Private communication C.Veelken: 10% * pt
// https://hypernews.cern.ch/HyperNews/CMS/get/AUX/2012/09/09/11:07:15-23577-slides_Tau2012_v1.pdf
inline double tau(const double pt)
{
    if (pt < 0)
    {
        throw std::range_error("tau pt must be > 0!");
    }
    return 0.1 * pt;
}

// read of from resolution plots for calo only resolution
// https://twiki.cern.ch/twiki/bin/view/CMSPublic/EGMElectronsMoriond2013#Electron_Resolution
inline double photon(double energy)
{
    if (energy < 30)
    {
        return (0.085 - (0.04 / 30.) * energy) * energy;
    }
    if (energy < 100)
    {
        return (0.045 - (0.015 / 70.) * (energy - 30)) * energy;
    }
    return electron(energy);
}

// Function taken from JME-13-004 paper
// Eq 38 on page 53
// Parameter values read from fig. 37 on page 54 for PFCHS jets
// Use jet Area A = Pi * R^2 = Pi * 0.4^2 and pile-up mu = 11.5
// Thus, A * mu = 5.8 in the plot
// http://cms.cern.ch/iCMS/analysisadmin/cadilines?line=JME-13-004
inline double jet(double pt)
{
    if (pt < 0)
    {
        throw std::range_error("pt must be > 0!");
    }
    double N = 2.2;       // GeV
    double S = 8.8 / 10;  // sqrt( GeV )
    double C = 4.5 / 100; // 1
    return pt * std::sqrt(std::pow(N / pt, 2) + std::pow(S, 2) / pt + std::pow(C, 2));
}

// Parameterization of MET dependance in sumEt found in Eq. (6)
// of JME-13-003 using values approximated from Tab. 2 (worst case).
// MET_res = sigma_0 + sigma_s * sqrt( sumEt )
// Where sigma_0 is the intrinsic detector noise resolution and sigma_s is
// the MET resolution stochastic term.
inline double met(double const sumpt)
{
    if (sumpt < 0)
    {
        throw std::range_error("sumpt must be > 0!");
    }

    double const sigma_0 = 1.78;
    double const sigma_s = 0.63;

    return sigma_0 + sigma_s * std::sqrt(sumpt);
}

} // namespace ResolutionsFuncs

// #from EventClassFactory.cpp
// // BinLimits
// // Calculate bin edges, depending on resolution.
// std::vector<double> const sumptBins =
//     particleMap.getBinLimits("SumPt", countMap, 0, m_cme, m_bin_size_min, m_fudge_sumpt);

// std::vector<double> const minvBins =
//     particleMap.getBinLimits("InvMass", countMap, 0, m_cme, m_bin_size_min, m_fudge_sumpt);

// std::vector<double> const metBins = particleMap.getBinLimits("MET", countMap, 0, m_cme, m_bin_size_min,
// m_fudge_sumpt);

namespace BinLimits
{

auto getApproximateResolution(const std::unordered_map<ObjectNames, int> &countMap, double sumpt, double const fudge)
    -> const double;

auto getApproximateResolutionMET(const std::unordered_map<ObjectNames, int> &countMap, double sumpt, double const fudge)
    -> const double;

auto callResolutionFunction(const ObjectNames name, const double &value) -> const double;

auto limits(const std::unordered_map<ObjectNames, int> &countMap,
            const bool isMET,
            double min,
            double max,
            double step_size,
            const double fudge) -> const std::vector<double>;
} // namespace BinLimits

#endif // !BIN_LIMITS
