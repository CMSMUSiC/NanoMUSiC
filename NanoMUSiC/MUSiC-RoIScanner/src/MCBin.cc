#include "MCBin.hh"

#include <algorithm>
#include <cassert>
#include <string>
#include <stdexcept>
#include <iostream>
#include <stdlib.h>

using namespace std;

MCBin::MCBin() {}

MCBin::MCBin( const yield_vector& mcEventsPerProcessGroup,
        const yield_vector& mcStatUncertPerProcessGroup,
        const name_vector& mcProcessGroupNames,
        double lowerEdge,
        double width,
        const uncert_vector& mcSysUncerts,
        const name_vector& mcSysUncertNames ) :
    mcEventsPerProcessGroup( mcEventsPerProcessGroup ),
    mcStatUncertPerProcessGroup( mcStatUncertPerProcessGroup ),
    mcProcessGroupNames( mcProcessGroupNames ),
    lowerEdge( lowerEdge ),
    width( width ),
    mcSysUncerts( mcSysUncerts ),
    mcSysUncertNames( mcSysUncertNames ),
    initialized( true )
{
    assert( mcProcessGroupNames.size() == mcEventsPerProcessGroup.size() );
    assert( mcProcessGroupNames.size() == mcStatUncertPerProcessGroup.size() );

    assert( mcSysUncertNames.size() == mcSysUncerts.size() );
}

void MCBin::clear(){
    mcEventsPerProcessGroup.clear();
    mcStatUncertPerProcessGroup.clear();
    mcProcessGroupNames.clear();
    lowerEdge = 0;
    width = 0;
    mcSysUncerts.clear();
    mcSysUncertNames.clear();

    initialized = false;
}

MCBin& MCBin::operator+=( const MCBin &add ){
    if (!add.initialized) {
        return *this;
    }

    if (!initialized) {
        // just assign
        (*this) = add;
        initialized = true;

        assert( mcProcessGroupNames.size() == mcEventsPerProcessGroup.size() );
        assert( mcProcessGroupNames.size() == mcStatUncertPerProcessGroup.size() );
        assert( mcSysUncertNames.size() == mcSysUncerts.size() );

        return *this;
    }

    // recalculate bin borders and check if they match
    if( ( lowerEdge < add.lowerEdge and ((lowerEdge + width ) > add.lowerEdge) )
            or
            ( lowerEdge > add.lowerEdge and ((add.lowerEdge + add.width ) > lowerEdge) ) ){
        std::cerr << "Error adding MCBin. borders overlap!" << std::endl;
        std::cerr << "MCBin1 lowerEdge: " << lowerEdge;
        std::cerr << " upperEdge: "<<  (lowerEdge + width) ;
        std::cerr << " (width "<<  width << ")" <<std::endl;
        std::cerr << "MCBin2 lowerEdge: " << add.lowerEdge;
        std::cerr << " upperEdge: "<<  (add.lowerEdge + add.width) ;
        std::cerr << " (width "<<  add.width << ")" <<std::endl;
        exit(1);
    }
    lowerEdge = min( lowerEdge, add.lowerEdge );
    width += add.width;

    //total events are just summed up
    assert( mcEventsPerProcessGroup.size() == add.mcEventsPerProcessGroup.size() );
    for( size_t i=0; i < mcEventsPerProcessGroup.size(); i++ ){
        mcEventsPerProcessGroup[ i ] += add.mcEventsPerProcessGroup[ i ];
    }

    // statistical errors are uncorrelated between bins and thus added in quadrature
    assert( mcStatUncertPerProcessGroup.size() == add.mcStatUncertPerProcessGroup.size() );
    for( size_t i=0; i < mcStatUncertPerProcessGroup.size(); i++ ){
        mcStatUncertPerProcessGroup[ i ] = sqrt( pow( mcStatUncertPerProcessGroup[ i ], 2 ) + pow( add.mcStatUncertPerProcessGroup[ i ], 2 ) );
    }

    // systematic errors are 100% correlated, hence they are also just summed up
    assert( mcSysUncerts.size() == add.mcSysUncerts.size() );
    for( size_t i=0; i < mcSysUncerts.size(); i++ ){
        mcSysUncerts[ i ].first += add.mcSysUncerts[ i ].first;
        mcSysUncerts[ i ].second += add.mcSysUncerts[ i ].second;
    }

    // mark caches as dirty, they will be recalculated before next use
    totalUncertCache = DIRTY_CACHE;
    totalMcEventsCache = DIRTY_CACHE;
    totalMcStatUncertCache = DIRTY_CACHE;

    return *this;
}

bool MCBin::operator!=( const MCBin& comp) const {
    if ( comp.lowerEdge != lowerEdge ) return true;
    if ( comp.width != width ) return true;
    return false;
}

ostream& operator<<( ostream &out, const MCBin& bin ){
    out << "MCBin(" << bin.lowerEdge << ", " << (bin.lowerEdge + bin.width) << ")" << std::endl;
    out << "Events: " << bin.getTotalMcEvents() << " +/- " << bin.getTotalMcStatUncert() << std::endl;
    out << "Processes: " << std::endl;
    for ( size_t i = 0; i < bin.mcEventsPerProcessGroup.size(); i++ ) {
        out << "\t" << bin.mcProcessGroupNames[i] << ": ";
        out << bin.mcEventsPerProcessGroup[ i ] << " +/- " << bin.mcStatUncertPerProcessGroup[ i ] << std::endl;
    }
    out << "Uncertainties: " << std::endl;
    for ( size_t i = 0; i < bin.mcSysUncerts.size(); i++ ) {
        out << "\t" << bin.mcSysUncertNames[i] << ": ";

        const auto& error = bin.mcSysUncerts[i];
        if( error.first == error.second )  {
            out << " " << error.first << std::endl;
        } else {
            out << " " << error.first << " - " << error.second << std::endl;
        }
    }
    return out;
}

bool MCBin::isEmpty() const {
    if( !initialized ) return true;
    if( getTotalMcEvents() != 0. ) return false;
    if( getTotalMcUncert() != 0. ) return false;
    return true;
}

double MCBin::getTotalMcUncert() const {
    // check if cached value exists and calculate it otherwise
    if( totalUncertCache < 0 ){
        totalUncertCache = sqrt( pow( getTotalMcStatUncert(), 2 ) +
                                 pow( getTotalMcSystUncert(), 2 ) );
    }

    return totalUncertCache;
}

double MCBin::getTotalMcSystUncert() const {
    double totalUncert = 0.;
    for( auto& error : mcSysUncerts ){
        const double symmetrized = symmetrizeError( error );
            totalUncert += pow( symmetrized, 2 );
    }
    totalUncert = sqrt(totalUncert);
    return totalUncert;
}

double MCBin::getTotalMcEvents() const {
    if( totalMcEventsCache < 0 ){
        totalMcEventsCache = 0.;
        for( auto& events : mcEventsPerProcessGroup ){
            totalMcEventsCache += events;
        }
    }
    return totalMcEventsCache;
}

double MCBin::getTotalMcStatUncert() const {
    if ( totalMcStatUncertCache < 0 ){
        totalMcStatUncertCache = 0.;
        for( auto& error : mcStatUncertPerProcessGroup ){
            totalMcStatUncertCache += pow( error, 2 );
        }
        totalMcStatUncertCache = sqrt( totalMcStatUncertCache );
    }
    return totalMcStatUncertCache;
}

std::vector< size_t > MCBin::leadingBackgrounds( const double threshold ) const {
    assert( threshold >= 0 );
    assert( threshold <= 1 );

    // generate index list
    std::vector< size_t > indices( mcEventsPerProcessGroup.size() );
    for( size_t i=0; i<indices.size(); i++) {
        indices[ i ] = i;
    }

    // sort indices based on their processes event counts
    auto comparator = [=](size_t i1, size_t i2) {
        return mcEventsPerProcessGroup[ i1 ] > mcEventsPerProcessGroup[ i2 ];
    };
    std::sort( indices.begin(), indices.end(), comparator );

    const double required = getTotalMcEvents() * threshold;
    double cumulative = 0;

    // i is now a "meta index" ;)
    for( size_t i=0; i<indices.size(); i++) {
        size_t index = indices[i];
        cumulative += mcEventsPerProcessGroup[ index ];
        if( cumulative > required ) {
            // cut off the list after the current element
            indices.resize( i+1 );
            break;
        } else if( mcEventsPerProcessGroup[ index ] == 0 ){
            // This contribution is 0. Because of ordering, all following
            // contributions are also 0. Thus we should exclude them.
            indices.resize( i );
            break;
        }
    }

    return indices;
}

std::map< size_t, float > MCBin::leadingBackgroundsFractions(const double threshold) const {
    std::vector< size_t > indices = leadingBackgrounds(threshold);
    std::map< size_t, float > fractions;
    for(auto index : indices){
        fractions[index] = mcEventsPerProcessGroup[ index ] / getTotalMcEvents();
    }
    return fractions;
}

double MCBin::symmetrizeError( const std::pair<double, double> error ) {
    // double version1 = std::abs( error.first + error.second ) / 2.;
    // double version2 = (std::abs( error.first) + std::abs(error.second )) / 2.;
    //~ std::cout <<"symmetrize error" << std::endl;
    //~ if(error.first < 0.){
        //~ std::cout << "error.first " << error.first << std::endl;
    //~ }
    //~ if(error.second < 0.){
        //~ std::cout << "error.second " << error.second << std::endl;
    //~ }
    //~ if(std::abs(version1 - version2) > 0.000001){
        //~ std::cout << error.first << " " << error.second << " " << version1 << " " << version2 << std::endl;
    //~ }
    if( error.first == error.second ) return error.first;
    else return std::abs( error.first + error.second ) / 2.;
    //~ else return (std::abs( error.first) + std::abs(error.second )) / 2.;
}
