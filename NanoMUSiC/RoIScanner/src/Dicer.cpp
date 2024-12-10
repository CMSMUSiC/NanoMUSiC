#include "Dicer.hpp"

#include <cassert>
#include <limits>
#include <vector>

Dicer::Dicer()
{
    std::random_device rd;
    m_uncorrelatedGenerator.seed(rd());
    reset();
}

void Dicer::setSystematicShifts(const std::unordered_map<std::string, std::vector<double>> &shifts)
{
    m_systematicShifts = shifts;
}

std::vector<double> Dicer::dicePseudoData(const std::vector<MCBin> &bins, const unsigned int round, PriorMode prior)
{

    if (bins.size() > 0)
    {
        const std::vector<double> systematicShifts = loadSystematicShifts(round, bins[0]);
        if (m_poissonSeed)
        {
            // combine fixed seed (fixed per event class and external) and round
            const unsigned int seed = (m_poissonSeed << 16) | round;
            m_uncorrelatedGenerator.seed(seed);
        }
        return dicePoissonData(bins, systematicShifts, prior);
    }
    else
    {
        return std::vector<double>();
    }
}

std::vector<double> Dicer::loadSystematicShifts(const unsigned int round, const MCBin &referenceBin)
{
    std::vector<double> result;
    result.reserve(referenceBin.mcSysUncertNames.size());

    for (size_t i = 0; i < referenceBin.mcSysUncertNames.size(); i++)
    {
        const MCBin::name_vector::value_type name = referenceBin.mcSysUncertNames[i];

        const std::vector<double> &shifts = m_systematicShifts[name];
        const double shift = shifts[round];

        result.push_back(shift);
    }

    return result;
}

void Dicer::setPoissonSeed(const RNG::result_type seed)
{
    m_poissonSeed = seed;
}

std::vector<double> Dicer::dicePoissonData(const std::vector<MCBin> &bins,
                                           const std::vector<double> &systematicShifts,
                                           PriorMode prior)
{

    assert(bins.size() > 0);

    std::vector<double> result(bins.size());

    for (size_t i = 0; i < bins.size(); i++)
    {
        const MCBin &bin = bins[i];

        // No need to dice pseudo data for empty bin
        if (!bin.isEmpty())
        {
            // New mean is constructed as the sum of systematic uncertainties and the current mean
            double stackedMean = bin.getTotalMcEvents();
            for (size_t j = 0; j < bin.mcSysUncerts.size(); j++)
            {
                // Using symmetrized errors in shifting! (approximation)
                const double uncert = MCBin::symmetrizeError(bin.mcSysUncerts[j]);
                if (prior == NORMAL_PRIOR)
                {
                    // normal prior: use "old" stacking
                    stackedMean += systematicShifts[j] * uncert;
                }
                else if (prior == LOGNORMAL_PRIOR)
                {
                    const double relative_uncert = uncert / bin.getTotalMcEvents();
                    stackedMean *= std::pow(1. + relative_uncert, systematicShifts[j]);
                }
                else
                {
                    throw std::invalid_argument("Unknown prior mode (not implemented in " __FILE__ ").");
                }
            }

            // Fallback for the case of no stat uncertainty
            double newMean = stackedMean;

            // Smear mean with gaussian using statistical uncertainty
            if (bin.getTotalMcStatUncert() != 0)
            {
                std::normal_distribution<double> gaus(stackedMean, bin.getTotalMcStatUncert());

                // Take the global generator which is uncorrelated between the bins
                newMean = gaus(m_uncorrelatedGenerator);
            }

            // Take care that value after dicing is still positive
            newMean = std::max<double>(newMean, 0);

            // Safety-check: check whether the distance is less than 10 sigma away.
            // This corresponds to a probability of 1 : 6.6e22 and thus should never happen!
            assert((stackedMean < 20) || ((std::abs(newMean - stackedMean) / bin.getTotalMcUncert()) < 10));

            // Smear with Poisson: exp(-xgaus) * xgaus^(X) / X! (simulate measurement)

            // Check whether the new mean can be represented as long int, leave 2 orders
            // of magnitude as buffer (thus the factor 100).
            assert(static_cast<unsigned long long>(100 * newMean) < std::numeric_limits<unsigned long long>::max());

            // Construct poisson distribution. Using "long long" as int datatype guarantees a
            // range of at least 64bits (up to 1.84e19 ).
            std::poisson_distribution<unsigned long long> pois(newMean);

            // The unsigned long long number is here casted back to double.
            result[i] = pois(m_uncorrelatedGenerator);
        }
    }

    return result;
}

void Dicer::reset()
{
    m_systematicShifts.clear();
}
