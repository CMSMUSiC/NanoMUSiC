#include "ECScanner.hh"

#include "util.h"

#include <cmath>
#include <math.h>
#include <algorithm>
#include <unordered_map>

#include <TStyle.h>
#include <TH2.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TExec.h>
#include <TPave.h>

// defined in ECScanner.hh: RAPIDJSON_HAS_STDSTRING=1
#include "writer.h"
#include "prettywriter.h"
#include "filereadstream.h"
#include "filewritestream.h"
#include "error/en.h"

#include "ConvolutionComputer.hh"

namespace rs = rapidjson;
namespace ph = std::placeholders;
////////////////////////////////////////////////
//     Init functions
////////////////////////////////////////////////

//// Constructor for ECScanner
//
//
ECScanner::ECScanner(int &rounds,
                     int &startRound):
        m_scanType( ScanType::unknown ),
        m_numDicingRounds( rounds ),
        m_firstDicingRound( startRound ),
        m_scoreFunc( "p-value" ),
        m_dicingProfiler( "pseudo experiment generation" ),
        m_roiFindingProfiler( "RoI finding total" ),
        m_pValueProfiler( "p-value calculation" )
{
    // initalize function maps
    initScoreFuncMap();
    initFilterFuncMap();
}

void ECScanner::initScoreFuncMap(){
    m_scoreFuncMap.emplace( "p-value", std::bind( &ECScanner::calcPvalMUSiC, this, ph::_1, ph::_2 ) );
    m_scoreFuncMap.emplace( "significance" , std::bind( &ECScanner::calcSignificance, this, ph::_1, ph::_2 ) );
}

void ECScanner::initFilterFuncMap(){
    m_filterFuncMap.emplace( "significance", std::bind( &ECScanner::significanceFilter, this )  );
}

ECScanner::~ECScanner() {
    // might be nullptr, doesn't matter...
    delete m_regionControlPlot;
}

void ECScanner::finalize() {
    // print stuff, etc.
    std::cout << "=== PROFILERS ===" << std::endl;
    std::cout << m_dicingProfiler << std::endl;
    std::cout << m_roiFindingProfiler << std::endl;
    std::cout << m_pValueProfiler << std::endl;
    std::cout << "=== STATS ===" << std::endl;
    for( const auto pair : m_regionStatistics ) {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }
}

////////////////////////////////////////////////
//     Getters & Setters
////////////////////////////////////////////////

bool ECScanner::isDataScan() const {
    return m_scanType == ScanType::data;
}

bool ECScanner::isSignalScan() const {
    return m_scanType == ScanType::signal;
}

unsigned int ECScanner::getDicingRounds() const {
    return m_numDicingRounds;
}

unsigned int ECScanner::getFirstDicingRound() const {
    return m_firstDicingRound;
}

////////////////////////////////////////////////
//     Region of Interest finding (RoI)
////////////////////////////////////////////////

//// Find and store the Region of Interest (RoI)
//
//
void ECScanner::findRoI( const std::string scoreType, const bool filtered ){
    m_roiFindingProfiler.start();

    const int maxBin = getMaxFilledBin();

    // Might be -1 if no data and no MC is in the distribution
    assert( maxBin >= 0 );

    // upper bin limit, EXCLUDED from search!
    auto maxMCBinIter = m_mcBins.begin() + maxBin + 1;
    //~ auto maxDataBin   = m_dataBins.begin() + maxBin ;

    // Scan might use a filter function to reduce the number of regions.
    // In this case regions are not constructed and the function
    // just determines the region with the lowest score from the remaining entries
    // in m_mcBins and m_dataBins.
    if ( filtered ) {
        m_filterFuncMap[ m_filterFunc ] ();
    }

    // cache the scoreFunc
    const auto thisScoreFunc = m_scoreFuncMap[ scoreType ];

    // clear scanner results cache
    m_scanResultsCache.clear();

    // each bin until MaxBin is used as the starting point to construct regions
    std::vector< MCBin >::iterator startMCBinIter = m_mcBins.begin();
    std::vector< double >::iterator startDataBinIter = m_dataBins.begin();

    // these variables are used to store the integral after all bins
    // have been added.
    MCBin mcbin_integral;
    double data_integral = 0;

    // Main RoI building loop
    for( ; startMCBinIter != maxMCBinIter; ++startMCBinIter , ++startDataBinIter) {
        std::vector< MCBin >::iterator endMCBinIter = startMCBinIter;
        std::vector< double >::iterator endDataBinIter = startDataBinIter;

        MCBin mcbin;
        double data = 0;

        // region goes from startMCBinIter until endMCBinIter (both incl)
        for( ; endMCBinIter != maxMCBinIter; ++endMCBinIter, ++endDataBinIter ){
            mcbin += *endMCBinIter;
            data += *endDataBinIter;

            if( m_integralScan ) {
                continue;
            }

            // preserve minimal region width
            // take +1 because start and end are both included in the interval
            if ( ( endMCBinIter - startMCBinIter + 1 ) < m_minRegionWidth ) {
                fillRegionControlPlot( mcbin, SkipReason::MIN_REGION_WIDTH );
                continue;
            }

            m_regionStatistics[ "total regions" ]++;

            // check if we should skip score function calculation for this region
            if( vetoRegion( mcbin,
                            data,
                            startMCBinIter,
                            endMCBinIter,
                            startDataBinIter,
                            endDataBinIter,
                            maxMCBinIter ) ){
                m_regionStatistics[ "vetoed" ]++;
                continue;
            }

            m_regionStatistics[ "p-values" ]++;

            // calculate new score
            const double score = thisScoreFunc( mcbin, data );
            if ( score >= 0 ) {
                // store scan result
                m_scanResultsCache.push_back(
                        ScanResult(
                            mcbin,
                            data,
                            score,
                            m_integralScan,
                            /* skipped = */false,
                            m_dataBins,
                            m_totalMcEvents,
                            m_totalMcUncert
                        )
                    );
                fillRegionControlPlot( mcbin, - std::log10(score) );
            } else {
                std::cerr << "Warning: score = " << score << std::endl;
                std::cerr << "Data: " << data << std::endl;
                std::cerr << mcbin << std::endl;
            }

            if ( filtered ) {
                break;
            }
        }
        // mcbin contains complete integral after first inner loop finished
        if( startMCBinIter == m_mcBins.begin() ){
            mcbin_integral = mcbin;
            data_integral = data;
            if( m_integralScan ) {
                bool skipped = vetoRegion( mcbin,
                                      data,
                                      startMCBinIter,
                                      endMCBinIter,
                                      startDataBinIter,
                                      endDataBinIter,
                                      maxMCBinIter,
                                      m_integralScan );
                const double score = thisScoreFunc( mcbin, data );
                m_scanResultsCache.push_back(
                        ScanResult(
                            mcbin,
                            data,
                            score,
                            /* integralScan = */ true,
                            /* skipped = */skipped,
                            m_dataBins,
                            m_totalMcEvents,
                            m_totalMcUncert
                        )
                    );
                break;
            }
        }
    }

    // We have scanned all possible regions.
    // Now determine the *final* result, the region of interest.
    ScanResult final_result;
    if ( m_scanResultsCache.size() > 0 ){
        // Sort the list by increasing compare score:
        std::sort(  m_scanResultsCache.begin(), m_scanResultsCache.end() );

        // We may have overlapping regions with the same score due to empty bins
        // choose the one with the smallest width:

        double final_score = 2.; // 2 == uninitialized. any number > 1 will suffice

        for( auto result : m_scanResultsCache ){
            if( final_score > 1 ){
                final_result = result;
                final_score  = result.getScore();
            }else{
                const double max_equal_reldiff = 1.e-4;
                const double reldiff = std::abs( final_score - result.getScore() ) / final_score;
                // If reldiff is ~0, the two results have the same significance
                // so in that case check whether the "result" candidate is smaller.
                // If so, use it.
                if( reldiff < max_equal_reldiff and result.getMcBin().width < final_result.getMcBin().width ){
                    final_result = result;
                    final_score  = result.getScore();
                    continue;
                }

                // Since the list is sorted by inc. score, there will be no more
                // equally small regions further down, so we can break.
                if( reldiff > max_equal_reldiff ) {
                    break;
                }
            }
        }
        m_scanResults.push_back( final_result );
    } else { // no scan result found. Use integral of distribution as output
        // This result has to be skipped later in the analysis!
        m_scanResults.push_back(
            ScanResult(
                mcbin_integral,
                data_integral,
                1,
                /* integralScan = */ true,
                /* skipped = */ true,
                m_dataBins,
                m_totalMcEvents,
                m_totalMcUncert
            )
        );
    }

    if ( not filtered ) {
        m_scanResultsCache.clear(); // cache may be used if we filter
    }

    m_roiFindingProfiler.stop();
}

//// Function to determine if a region should be skipped for scoreFunction calculation
//
//
bool ECScanner::vetoRegion( const MCBin& mcbin,
                            double data,
                            std::vector< MCBin >::iterator const &startMCBinIter,
                            std::vector< MCBin >::iterator const &endMCBinIter,
                            __attribute__((unused)) std::vector< double >::iterator const &startDataBinIter,
                            std::vector< double >::iterator const &endDataBinIter,
                            std::vector< MCBin >::iterator const &maxMCBinIter,
                            bool isIntegral ){

    constexpr double no_data_threshold = 1e-9; // data values less than this will be treated as 0

    // check special cases for the region

    // don't recalculate the p-value if an empty bin has been added
    if ( not m_integralScan and ( *endMCBinIter ).isEmpty() and ( *endDataBinIter ) < no_data_threshold ) {
        m_regionStatistics[ "skip: empty added" ]++;
        fillRegionControlPlot( mcbin, SkipReason::EMPTY_BIN );
        return true;
    }

    if ( data < no_data_threshold and mcbin.isEmpty() ) { // nothing (no MC, no data)
        // not a valid region
        m_regionStatistics[ "skip: empty" ]++;
        fillRegionControlPlot( mcbin, SkipReason::EMPTY_REGION );
        return true;
    }

    if ( data > no_data_threshold and mcbin.isEmpty() ) {
        std::cerr << "Warning: Region with data but without MC!" << std::endl;
        std::cerr << mcbin << std::endl;
        m_regionStatistics[ "skip: data but no MC" ]++;
        fillRegionControlPlot( mcbin, SkipReason::DATA_NO_MC );
        return true;
    }


    const double n_mc = mcbin.getTotalMcEvents();
    const double relative_uncert = std::abs( mcbin.getTotalMcUncert() / n_mc );

    if( n_mc < m_thresholdRegionYield){
        fillRegionControlPlot( mcbin, SkipReason::LOW_MC_YIELD );
        return true;
    }

    if ( data < no_data_threshold and not mcbin.isEmpty() and ( std::abs( n_mc / mcbin.getTotalMcStatUncert() ) < m_coverageThreshold ) ){
        m_regionStatistics[ "skip: (classic) coverage" ]++;
        fillRegionControlPlot( mcbin, SkipReason::MC_NO_DATA );
        return true;
    }

    const double adaptive_coverage_threshold = std::min( 1.0, std::max( 1.2 * std::pow( n_mc, -0.2 ), 0.5 ) );
    if( relative_uncert > adaptive_coverage_threshold ) { // too high uncert
        m_regionStatistics[ "skip: adaptive coverage" ]++;
        fillRegionControlPlot( mcbin, SkipReason::HIGH_REL_UNCERT );
        return true;
    }

    // too insignificant for a full p-value calculation
    if ( !isIntegral && std::abs( data - n_mc ) / mcbin.getTotalMcUncert() < m_sigmaThreshold ) {
        m_regionStatistics[ "skip: insignificant" ]++;
        fillRegionControlPlot( mcbin, SkipReason::SIGMA_THRESHOLD );
        return true;
    }

    if( n_mc <= 0. ){
        m_regionStatistics[ "skip: negative MC" ]++;
        fillRegionControlPlot( mcbin, SkipReason::MC_NEGATIVE );
        return true;
    }


    const double threshold = -0.02 * n_mc;
    for( const double yield : mcbin.mcEventsPerProcessGroup ){
        if( yield < threshold ) {
            m_regionStatistics[ "skip: too much negative bg" ]++;
            fillRegionControlPlot( mcbin, SkipReason::BG_NEGATIVE );
            return true;
        }
    }

    // Low-Statistics treatment as presented to EXO on 20. Jan 2016

    if( not m_noLowStatsTreatment ) {
        if ( mcbin.getTotalMcStatUncert() / mcbin.getTotalMcEvents() > m_thresholdLowStatsUncert) {
            m_regionStatistics[ "skip: low statistics" ]++;
            fillRegionControlPlot( mcbin, SkipReason::LOW_MC_STAT );
            return true;
        }
    }

    // Neighborhood-based low stats vetos:
    std::vector< size_t > leadingBackgroundsNeighborhood;
    std::vector< size_t > leadingBackgroundsRegion;
    if(!m_skipNeighborhoodCheck){
        // neighborhood region width determined to be +-4
        MCBin neighborhood;
        neighborhood += constructNeighborhood( startMCBinIter,
                                               -1 * m_widthLowStatsRegions,
                                               m_mcBins.begin(),
                                               maxMCBinIter );
        neighborhood += constructNeighborhood( endMCBinIter,
                                               m_widthLowStatsRegions,
                                               m_mcBins.begin(),
                                               maxMCBinIter );

        // evaluate missing leading BGs (defined on 95% interval)
        leadingBackgroundsNeighborhood = neighborhood.leadingBackgrounds( m_thresholdLowStatsDominant );
        leadingBackgroundsRegion = neighborhood.leadingBackgrounds( m_thresholdLowStatsDominant );

        auto leadingBackgroundsNeighborhoodFractions = neighborhood.leadingBackgroundsFractions(m_thresholdLowStatsDominant);
        auto leadingBackgroundsRegionFractions = mcbin.leadingBackgroundsFractions(m_thresholdLowStatsDominant);


        for( const int index : leadingBackgroundsNeighborhood ){
            // Treatment of negative backgrounds
            if( mcbin.mcEventsPerProcessGroup[ index ] < 0. ){
                // leading BG is negative -> region is unphysical
                m_regionStatistics[ "skip: leading bg is negative" ]++;
                fillRegionControlPlot( mcbin, SkipReason::LEADING_BG_NEGATIVE );
                return true;
            }

            if( mcbin.mcEventsPerProcessGroup[ index ] == 0. ){
                // leading BG is missing (no statistics)
                // corresponds to the 8TeV low-stats-treatment
                m_regionStatistics[ "skip: leading bg missing" ]++;
                fillRegionControlPlot( mcbin, SkipReason::LEADING_BG_MISSING );
                return true;
            }

            double process_fraction = mcbin.mcEventsPerProcessGroup[ index ] / mcbin.getTotalMcEvents();
            // Check if a leading background from the neighborhood fluctuates up / down in the region
            if( process_fraction < leadingBackgroundsNeighborhoodFractions[index] - m_thresholdLowStatsDominatFraction ||
                process_fraction > leadingBackgroundsNeighborhoodFractions[index] + m_thresholdLowStatsDominatFraction){
                m_regionStatistics[ "skip: leading bg fluctuates" ]++;
                fillRegionControlPlot( mcbin, SkipReason::LEADING_BG_FLUCTUATES );
                return true;
            }
        }
        // Check if a leading background from the region fluctuates up / down in the neighborhood
        // this filters e.g. single spikes
        for( const int index : leadingBackgroundsRegion ){
            double process_fraction = neighborhood.mcEventsPerProcessGroup[ index ] / neighborhood.getTotalMcEvents();
            if( process_fraction < leadingBackgroundsRegionFractions[index] - m_thresholdLowStatsDominatFraction ||
                process_fraction > leadingBackgroundsRegionFractions[index] + m_thresholdLowStatsDominatFraction){
                m_regionStatistics[ "skip: leading bg fluctuates" ]++;
                fillRegionControlPlot( mcbin, SkipReason::LEADING_BG_FLUCTUATES );
                return true;
            }
        }
    }

    // no reason to skip region
    return false;
}

MCBin ECScanner::constructNeighborhood(
        std::vector< MCBin >::iterator iter,
        const int width,
        std::vector< MCBin >::iterator minIter, // minimal possible iterator
        std::vector< MCBin >::iterator maxIter  // maximally possible iterator (maximal filled bin + 1)
) {
    assert( width != 0 );

    std::vector< MCBin >::iterator start; // inclusive
    std::vector< MCBin >::iterator end; // exclusive
    if( width < 0 ){
        start = max( iter + width, minIter ); // width is negative!
        end = iter;
    } else {
        start = iter + 1;
        end = min( start + width, maxIter );
    }
    MCBin neighborhood;
    for ( ; start != end; ++start ) {
        neighborhood += *start;
    }
    return neighborhood;
}

//// "Automated" version, use member variables.
//
//
void ECScanner::findRoI() {
    findRoI( m_scoreFunc, m_doFilter );
}

//// Calculate p-value following MUSiC convention
//
//
double ECScanner::calcPvalMUSiC( const MCBin& bin, const double data ) const {
    m_pValueProfiler.start();
    double p = -1.;

	//avoid caluclation if the difference between data and expecation is too large, just set it to a small p-value
	//because our assumptions might not be valid below there.
	if ( (std::abs(data - bin.getTotalMcEvents() ) / bin.getTotalMcUncert() ) > 7.0 ){
        // Set the value at  1e-8 (a little more than 5 sigma), because
        // our assumptions might not be valid below there.
        p = 1e-8;
	}

    if ( not m_skipLookupTable ) {
        // try to look up p-value in lookup table
        p = m_lookupTable.lookup(
                data,
                bin.getTotalMcEvents(),
                bin.getTotalMcUncert() );

        if( p > 0 ) {
            m_regionStatistics["lut: hit"]++;
        } else {
            m_regionStatistics["lut: miss"]++;
            // std::cout << "Miss at MC=" << bin.getTotalMcEvents() << ", UNCERT=" << bin.getTotalMcUncert() << ", DATA=" << data << std::endl;
            // std::cerr << bin.getTotalMcEvents() << " " << bin.getTotalMcUncert() << " " << data << " " << p << std::endl;
        }
    }

    if ( p < 0 ){
        // p negative (either lookup-table-miss or lookup table skipped)

        // call p-value calculation from ConvolutionComputer.h
        p = compute_p_convolution( data, bin.getTotalMcEvents(), bin.getTotalMcUncert(), m_p_prior );
    }

    if ( p >= 0 ) {
        // Don't allow p-values below 1e-8 (a little more than 5 sigma), because
        // our assumptions might not be valid there.
        // If multiple regions have the same score, we take the smallest.
        p = std::max(p, 1e-8);
    }

    m_pValueProfiler.stop();
    return p;
}

//// Calculate significance
//
//
double ECScanner::calcSignificance( const MCBin& bin, const double data ) const{
    return ( bin.getTotalMcEvents() - data ) / bin.getTotalMcUncert();
}

//// Filter regions based on significance
//
//
void ECScanner::significanceFilter( ){
    findRoI( "significance", false );
    // remove the found
    // we have filled the result cache and replace m_mcBins and m_dataBins
    m_mcBins.clear();
    m_dataBins.clear();
    for( size_t i=0; i < std::min<size_t>( m_nFilterRegions, m_scanResultsCache.size() ); i++ ){
        const ScanResult& res = m_scanResultsCache.at(i);
        m_mcBins.push_back( res.getMcBin() );
        m_dataBins.push_back( res.getData() );
    }
}

//// Return the position of the highest filled bin in either data or MC
//
//
int ECScanner::getMaxFilledBin() {
    int highestMC = m_mcBins.size() - 1;
    while ( highestMC >= 0 ) {
        const auto mcbin = m_mcBins[ highestMC ];
        if ( !mcbin.isEmpty() ) {
            break;
        }
        highestMC--;
    }

    int highestData = m_dataBins.size() - 1;
    while ( highestData >= 0 ) {
        const auto databin = m_dataBins[ highestData ];
        if ( databin != 0 ) {
            break;
        }
        highestData--;
    }

    return std::max( highestMC, highestData );
}


////////////////////////////////////////////////
//        Pseudo data dicing
////////////////////////////////////////////////

//// Dice a random number for every MCBin
//
// This function dices a random number for every
// MCBin in the class according to its systematic uncertainties and
// stores them in the m_dataBins vector
void ECScanner::diceMcPseudoData( const unsigned int round ) {
    m_dicingProfiler.start();
    m_dataBins = m_dicer.dicePseudoData( m_mcBins, round, m_p_prior );
    m_dicingProfiler.stop();
}

void ECScanner::diceSignalPseudoData( const unsigned int round ) {
    m_dicingProfiler.start();
    m_dataBins = m_dicer.dicePseudoData( m_signalBins, round, m_p_prior );
    m_dicingProfiler.stop();
}

////////////////////////////////////////////////
//        IO functions
////////////////////////////////////////////////

void ECScanner::readLookupTable( const std::string filename ) {
    if ( not m_skipLookupTable ) {
        m_lookupTable.readFile( filename );
        std::cout << "Loaded LUT from " << m_lookupTable.lastLoadedFilename() << std::endl;
    } else {
        std::cout << "LUT skipped." << std::endl;
    }
}

//// read bin infos  and uncertainties in m_mcBins
//
//
void ECScanner::readMCBinInfo() {
    assert( m_jsonDocument.HasMember( "MCBins" ) );
    const rs::Value& binVals = m_jsonDocument[ "MCBins" ];

    // construct empty bin on stack and pass as out-pointer to readMcBinArray
    MCBin integralBin;
    m_mcBins = readMCBinArray( binVals, &integralBin );

    m_totalMcEvents = integralBin.getTotalMcEvents();
    m_totalMcUncert = integralBin.getTotalMcUncert();

    // Reset everything we know about the regionControlPlot
    delete[] m_regionControlPlot;
    m_regionControlPlot = nullptr;

    // Collect bin edges for control plot
    if ( m_scanType == ScanType::data ) {
        const size_t N = m_mcBins.size();

        double* rootBinEdges = new double[ N + 1 ];
        for( size_t i = 0; i < N; i++ ){
            rootBinEdges[ i ] = m_mcBins[ i ].lowerEdge;
        }
        // add the last (highest) bin border
        const MCBin& lastBin = m_mcBins[ N - 1 ];
        rootBinEdges[ N ] = lastBin.lowerEdge + lastBin.width;
        m_regionControlPlot = new TH2F( "regions", "Region Control Plot", N, rootBinEdges, N, rootBinEdges );
        delete[] rootBinEdges;
    }

    // Reset statistics
    m_regionStatistics.clear();
}

void ECScanner::readSignalBinInfo() {
    assert( m_jsonDocument.HasMember( "SignalBins" ) );
    const rs::Value& binVals = m_jsonDocument[ "SignalBins" ];
    m_signalBins = readMCBinArray( binVals );
}

// static
std::vector< MCBin > ECScanner::readMCBinArray( const rs::Value& jsonArray, MCBin* integralBinOut ) {
    assert( jsonArray.IsArray() );
    std::vector< MCBin > result;

    if( integralBinOut != nullptr ){
        integralBinOut->clear();
    }

    // iterate over all MC Bin entries
    for (rs::SizeType i = 0; i < jsonArray.Size(); i++){ // Uses SizeType instead of size_t
        const rs::Value& jsonObject = jsonArray[ i ];

        const double lowerEdge = jsonObject[ "lowerEdge" ].GetDouble();
        const double width = jsonObject[ "width" ].GetDouble();

        // Handle event yield (per process group)
        assert( jsonObject[ "mcEventsPerProcessGroup" ].IsObject() );
        const rs::Value& mcEventsPerProcessObj = jsonObject[ "mcEventsPerProcessGroup" ];

        MCBin::name_vector mcProcessGroupNames;
        MCBin::yield_vector mcEventsPerProcessGroup;
        int j = 0;
        for( rs::Value::ConstMemberIterator itr = mcEventsPerProcessObj.MemberBegin();
            itr != mcEventsPerProcessObj.MemberEnd(); ++j, ++itr ){
            mcEventsPerProcessGroup.push_back( itr->value.GetDouble() );

            // Store process names
            mcProcessGroupNames.push_back( itr->name.GetString() );
            if (i > 0) {
                // check against the last bin
                assert( itr->name.GetString() == result[ i-1 ].mcProcessGroupNames[ j ] );
            }
        }
        assert( mcEventsPerProcessGroup.size() > 0 );

        // Handle statistical uncertainty (also per process group)
        assert( jsonObject[ "mcStatUncertPerProcessGroup" ].IsObject() );
        const rs::Value& mcStatUncertsObj = jsonObject[ "mcStatUncertPerProcessGroup" ];
        MCBin::yield_vector mcStatUncertPerProcessGroup;
        j = 0;
        for( rs::Value::ConstMemberIterator itr = mcStatUncertsObj.MemberBegin();
            itr != mcStatUncertsObj.MemberEnd(); ++j, ++itr ){
            const double uncert = std::abs( itr->value.GetDouble() );
            mcStatUncertPerProcessGroup.push_back( uncert );
            assert( itr->name.GetString() == mcProcessGroupNames[ j ] );
        }

        // Handle systematic uncertainties
        assert( jsonObject[ "mcSysUncerts" ].IsObject() );
        const rs::Value& mcSysUncertsObj = jsonObject[ "mcSysUncerts" ];

        // First, store raw values in a STL map, which we can query later.
        std::map<std::string, double> mcSysUncertsSingle;
        for( rs::Value::ConstMemberIterator itr = mcSysUncertsObj.MemberBegin();
            itr != mcSysUncertsObj.MemberEnd(); ++itr ){
            //~ const double uncert = std::abs( itr->value.GetDouble() );
            const double uncert = itr->value.GetDouble();
            mcSysUncertsSingle.emplace( itr->name.GetString(), uncert );
        }

        MCBin::uncert_vector mcSysUncerts;
        MCBin::name_vector mcSysUncertNames;

        // Query map for Up/Down values and group them in pairs.
        for (const auto& pair : mcSysUncertsSingle) {
            std::string name = pair.first;
            const double upValue = pair.second;
            double downValue = pair.second;

            if ( ends_with( name, "Up" ) ) {
                // TODO: repace only at the end of the string
                const std::string downName = str_replace( name, "Up", "Down" );
                auto iter = mcSysUncertsSingle.find( downName );

                if ( iter != mcSysUncertsSingle.end() ) {
                    downValue = ( *iter ).second;
                    name = str_replace( name, "Up", "" );
                }
            } else if ( ends_with( name, "Down" ) ) {
                continue;
            }

            mcSysUncerts.push_back( std::make_pair( upValue, downValue ) );
            mcSysUncertNames.push_back( name );
        }

        // Construct MCBin and append to result
        const MCBin mcBin( mcEventsPerProcessGroup,
                                    mcStatUncertPerProcessGroup,
                                    mcProcessGroupNames,
                                    lowerEdge,
                                    width,
                                    mcSysUncerts,
                                    mcSysUncertNames );

        if( integralBinOut != nullptr ){
            ( *integralBinOut ) += mcBin;
        }

        result.push_back( std::move( mcBin ) );
    }

    return result;
}

//// read in seeds and initalize map of random generators for each error
//
//
void ECScanner::readSystematicShiftsFile( const std::string filename ){
    rs::Document document = readJsonDocument( filename );

    assert( document.IsObject() );
    m_dicer.reset();

    std::map< std::string, std::vector< double > > shifts;

    for (rs::Value::ConstMemberIterator itr = document.MemberBegin();
            itr != document.MemberEnd(); ++itr){

        const rs::Value& rsShiftVals = itr->value;
        assert( rsShiftVals.IsArray() );

        std::vector< double > shiftVals( rsShiftVals.Size() );

        for( rs::SizeType i=0; i < rsShiftVals.Size(); i++ ) {
            shiftVals[ i ] = rsShiftVals[ i ].GetDouble();
        }

        const char* name = itr->name.GetString();
        shifts[ name ] = shiftVals;
    }

    m_dicer.setSystematicShifts( shifts );
}

//// read bin infos in m_dataBins
//
//
void ECScanner::readDataBinInfo(){
    assert( m_jsonDocument.HasMember( "DataBins" ) );
    const rs::Value& binVals = m_jsonDocument[ "DataBins" ];
    assert( binVals.IsArray() );
    m_dataBins.clear();
    for (rs::SizeType i = 0; i < binVals.Size(); i++){ // Uses SizeType instead of size_t
        m_dataBins.push_back( binVals[ i ].GetDouble() );
    }
}

//// Open and parse json file using rapidjson
//
//
void ECScanner::readInputJson( const std::string jsonFilePath ){
    // NOTE: This function does NOT reset the scanner before reading
    // the new configuration. If readInputJson(...) is called a second time,
    // configuration from the first call might still be active, if it
    // hasn't been overriden in the second json file.
    // To fix this bug/feature, we should implement ECScanner::reset()
    // which basically repeats member initialization and has to be
    // explicitely called from outside (keeping the "override" feature).

    m_lastJsonFilePath = jsonFilePath;

    // construct document from file
    m_jsonDocument = readJsonDocument( jsonFilePath );

    // make sure we sourced a valid json with a object as root
    assert( m_jsonDocument.IsObject() );

    m_scanType = ScanType::mc;

    // decide on the type of scan
    if( m_jsonDocument.HasMember( "SignalBins" ) ){
        m_scanType = ScanType::signal;
        readSignalBinInfo();
    } if( m_jsonDocument.HasMember( "DataBins" ) ){
        m_scanType = ScanType::data;
        readDataBinInfo();
    }
    // read and parse MCBin infos
    readMCBinInfo();

    checkAndSetConfig( "minRegionWidth", m_minRegionWidth );
    assert( m_minRegionWidth >= 1 );

    checkAndSetConfig( "coverageThreshold", m_coverageThreshold );
    assert( m_coverageThreshold >= 0. );

    checkAndSetConfig( "regionYieldThreshold", m_thresholdRegionYield );
    assert( m_thresholdRegionYield >= 0. );

    checkAndSetConfig( "sigmaThreshold", m_sigmaThreshold );
    assert( m_sigmaThreshold >= 0. );

    checkAndSetConfig( "noLowStatsTreatment", m_noLowStatsTreatment );
    checkAndSetConfig( "thresholdLowStatsUncert", m_thresholdLowStatsUncert );
    checkAndSetConfig( "widthLowStatsRegions", m_widthLowStatsRegions );
    checkAndSetConfig( "thresholdLowStatsDominant", m_thresholdLowStatsDominant );

    checkAndSetConfig( "integralScan", m_integralScan );
    // skip neighborhood criterion if integralScan is used
    if(m_integralScan) m_skipNeighborhoodCheck = true;
    checkAndSetConfig( "skipLookupTable", m_skipLookupTable );

    checkAndSetConfig( "name", m_ECName );
    checkAndSetConfig( "distribution", m_distribution );
    checkAndSetConfig( "key", m_submissionKey );
    checkAndSetConfig( "hash", m_submissionHash );

    // temporarily use int for prior (PriorMode can only be read from json as int)
    int tmp_prior = static_cast<int>(m_p_prior);
    checkAndSetConfig( "prior", tmp_prior );
    m_p_prior = static_cast<PriorMode>(tmp_prior);

    // check if we should use a alternative core function
    if( m_jsonDocument.HasMember("ScoreFunction") ){
        m_scoreFunc = m_jsonDocument["ScoreFunction"].GetString();
        if( m_scoreFuncMap.find( m_scoreFunc ) == m_scoreFuncMap.end() ) {
            std::cerr << "Error: No score function " << m_scoreFunc << " available" << std::endl;
            exit(1);
        }
    }

    // check if we should use filtering of regions
    if( m_jsonDocument.HasMember("Filter") ){
        m_doFilter = true;
        m_filterFunc = m_jsonDocument["Filter"].GetString();
        if( m_filterFuncMap.find( m_filterFunc ) == m_filterFuncMap.end() ) {
            std::cerr << "Error: No filter function " << m_filterFunc << " available" << std::endl;
            exit(1);
        }
    }

    if( m_jsonDocument.HasMember("poissonSeed") ){
        const int external_seed = m_jsonDocument["poissonSeed"].GetInt();
        const int class_seed = std::hash<std::string>()( m_ECName );

        // combining the two seeds a la http://stackoverflow.com/a/2595226/489345
        const int combined_seed = external_seed ^ ( class_seed + 0x9e3779b9 +
            ( external_seed << 6 ) + ( external_seed >> 2 ) );

        std::cout << "Fixing poisson random seed for this class to " << combined_seed << "." << std::endl;

        m_dicer.setPoissonSeed( combined_seed );
    }
}

rs::Document ECScanner::readJsonDocument( const std::string filename ){
    FILE* fp = fopen( filename.c_str() , "r"); // non-Windows use "r"

    char buffer[ 8192 ]; // 8 kB, Stack size on lxcluster currently ~10MB.
    if( fp == nullptr ){
        const char* cwd = getcwd( buffer, sizeof( buffer ) );
        std::cerr << "Error: JSON input file '" << cwd << "/" << filename << "' cannot be opened for reading." << std::endl;
        exit(1);
    }

    rs::Document document;

    rs::FileReadStream stream( fp, buffer, sizeof( buffer ) );

    document.ParseStream( stream );

    if (document.HasParseError()) {
        std::string error_message = rs::GetParseError_En( document.GetParseError() );
        std::cerr << "Parse error while parsing JSON input file: " << error_message << std::endl;
        exit(1);
    }

    fclose( fp );

    return document;
}

void ECScanner::writeJsonDocument( const std::string filename, const rs::Document& document ){
    FILE* fp = fopen( filename.c_str(), "wb" );

    char buffer[ 8192 ]; // 8kB
    if( fp == nullptr ){
        const char* cwd = getcwd( buffer, sizeof( buffer ) );
        std::cerr << "Error: JSON output file '" << cwd << "/" << filename << "' cannot be opened for writing." << std::endl;
        return;
    }

    rs::FileWriteStream stream( fp, buffer, sizeof( buffer ) );

    rs::PrettyWriter<rs::FileWriteStream> writer( stream );

    document.Accept( writer );

    fclose(fp);
}


template< typename T >
void ECScanner::checkAndSetConfig( const std::string name, T& config ) {
    if( m_jsonDocument.HasMember( name.c_str() ) ) {
        auto& member = m_jsonDocument[ name.c_str() ];
        config = member.Get< T >();
    }
    std::cout << name << " = " << std::boolalpha << config << std::endl;
}

//// Write all scan results to member m_jsonDocument
//
//
void ECScanner::writeOutputFiles( const std::string outputDirectory ){
    const std::string nameBase =
        outputDirectory + "/" + m_ECName + "_" + m_distribution;

    // CSV file for immediate results
    std::ofstream resultCsvFile( nameBase + "_output.csv" );

    // Write CSV header
    resultCsvFile
        << "hash,"
        << "event_class,"
        << "distribution,"
        << "round,";
    ScanResult::writeCsvHeader( resultCsvFile );
    resultCsvFile << std::endl;

    // Use new document object
    rs::Document infoJsonDocument;
    infoJsonDocument.SetObject();

    // get a reference to the object allocator
    auto& allocator = infoJsonDocument.GetAllocator();

    // Add scan results
    rs::Value scanResultArray( rs::kArrayType );
    for( size_t i = 0; i < m_scanResults.size(); i++ ){
        const ScanResult& result = m_scanResults[ i ];

        // initialize rapidjson object for each result
        rs::Value resultObject = result.rapidjsonValue( allocator );

        // Add object to list of scan results
        scanResultArray.PushBack( resultObject, allocator );

        // Append line to CSV file: first 4 general fields, then info about the result
        resultCsvFile
            << m_submissionHash << ","
            << m_ECName << ","
            << m_distribution << ","
            << ( m_firstDicingRound + i ) << ",";
        result.writeCsvLine( resultCsvFile );
        resultCsvFile << std::endl;
    }
    resultCsvFile.close();

    // Add list of results to rapidjson member document
    infoJsonDocument.AddMember( "ScanResults", scanResultArray, allocator );

    // Add timing results
    rs::Value timingObj( rs::kObjectType );
    timingObj.AddMember( "dicing", m_dicingProfiler.rapidjsonValue( allocator ), allocator );
    timingObj.AddMember( "roiFinding", m_roiFindingProfiler.rapidjsonValue( allocator ), allocator );
    timingObj.AddMember( "pValue", m_pValueProfiler.rapidjsonValue( allocator ), allocator );
    infoJsonDocument.AddMember( "timing", timingObj, allocator );

    // Add stats results
    rs::Value statsObj( rs::kObjectType );
    for( std::pair< std::string, int > pair : m_regionStatistics ) {
        statsObj.AddMember( rs::Value( pair.first, allocator ).Move(), pair.second, allocator );
    }
    infoJsonDocument.AddMember( "stats", statsObj, allocator );

    // Add EC name and scanned distribution
    infoJsonDocument.AddMember( "name", rs::Value( m_ECName, allocator ).Move(), allocator );
    infoJsonDocument.AddMember( "distribution", rs::Value( m_distribution, allocator ).Move(), allocator );
    infoJsonDocument.AddMember( "hash", rs::Value( m_submissionHash, allocator ).Move(), allocator );
    infoJsonDocument.AddMember( "key", rs::Value( m_submissionKey, allocator ).Move(), allocator );
    //Add starting round within global list of rounds
    infoJsonDocument.AddMember( "firstRound", m_firstDicingRound, allocator );
    // Add reference to input json file
    infoJsonDocument.AddMember( "JsonFile", rs::Value( m_lastJsonFilePath, allocator ).Move(), allocator );
    infoJsonDocument.AddMember( "prior", m_p_prior, allocator );

    // Add flag to identify data/signal scans
    std::string typeIdentifier;
    switch( m_scanType ) {
        case ScanType::data:
            typeIdentifier = "dataScan"; break;
        case ScanType::signal:
            typeIdentifier = "signalScan"; break;
        case ScanType::mc:
            typeIdentifier = "pseudoScan"; break;
        default:
            typeIdentifier = "unknown"; break;
    }
    infoJsonDocument.AddMember( "ScanType", rs::Value( typeIdentifier, allocator ).Move(), allocator );

    const std::string infoJsonFilePath = nameBase + "_info.json";
    writeJsonDocument( infoJsonFilePath, infoJsonDocument );


    // write control plot
    if( m_regionControlPlot != nullptr ) {
        const std::string rootname = nameBase + "_regions.root";
        TFile file( rootname.c_str(), "RECREATE");
        m_regionControlPlot->Write();
        file.Close();
    }
}

std::string ECScanner::replaceExtension( const std::string filename, const std::string newExtension ){
    const int index = filename.find_last_of(".");
    std::string newName = filename.substr(0, index);
    newName += newExtension;
    return newName;
}


void ECScanner::fillRegionControlPlot( const MCBin& mcbin, const double value ) {
    if ( m_regionControlPlot != nullptr ) {
        const double regionX = mcbin.lowerEdge;
        const double regionY = mcbin.lowerEdge + mcbin.width;
        const int binX = m_regionControlPlot->GetXaxis()->FindBin( regionX );
        const int binY = m_regionControlPlot->GetYaxis()->FindBin( regionY );
        m_regionControlPlot->SetBinContent( binX, binY, value );
    }
}

// overloaded method for skip reasons, they're treated like negative p-values
void ECScanner::fillRegionControlPlot( const MCBin& mcbin, const SkipReason value ) {
    fillRegionControlPlot( mcbin, static_cast<double>(value) );
}
