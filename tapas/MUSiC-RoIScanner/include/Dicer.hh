#ifndef DICER_HH
#define DICER_HH

#include <map>
#include <random>

#include "MCBin.hh"

// include convolution computer, only for the prior mode
#include "ConvolutionComputer.hh"

class Dicer
{
public:
    typedef std::mt19937_64 RNG;

    Dicer();

    void setSystematicShifts( const std::map< std::string, std::vector< double > >& shifts );
    void setPoissonSeed( RNG::result_type seed );
    std::vector< double > dicePseudoData( const std::vector< MCBin >& bins, unsigned int round, PriorMode prior );

    void reset();

private:
    std::vector< double > loadSystematicShifts( unsigned int round, const MCBin& referenceBin );
    std::vector< double > dicePoissonData( const std::vector< MCBin >& bins, const std::vector< double >& systematicShifts, PriorMode prior );

    std::map< std::string, std::vector< double > > m_systematicShifts;

    RNG m_uncorrelatedGenerator;
    RNG::result_type m_poissonSeed = 0;
};

#endif // DICER_HH
