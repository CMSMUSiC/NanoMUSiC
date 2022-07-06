#include "ScanResult.hh"
#include <numeric>
#include <algorithm>

#define RAPIDJSON_HAS_STDSTRING 1
#include "document.h"

namespace rs = rapidjson;

ScanResult::ScanResult( const MCBin& mcbin,
            double data,
            double score,
            bool integralScan,
            bool skippedScan,
            const std::vector< double >& dicedData,
            double totalMc,
            double totalMcUncert
          )
    : mcbin( mcbin )
    , data( data )
    , totalMc( totalMc )
    , totalMcUncert( totalMcUncert )
    , score( score )
    , integralScan( integralScan )
    , skippedScan( skippedScan )
    , dicedData( dicedData )
{
    totalData = std::accumulate( dicedData.begin(), dicedData.end(), 0.0 );
}

rs::Value ScanResult::rapidjsonValue( rs::Document::AllocatorType& allocator, bool verbose ) const {
    rs::Value resultObject( rs::kObjectType );

    resultObject.AddMember( "mcEvents", mcbin.getTotalMcEvents(), allocator );
    resultObject.AddMember( "mcStatUncert", mcbin.getTotalMcStatUncert(), allocator );
    resultObject.AddMember( "mcSystUncert", mcbin.getTotalMcSystUncert(), allocator );
    resultObject.AddMember( "mcTotalUncert", mcbin.getTotalMcUncert(), allocator );
    resultObject.AddMember( "dataEvents", data, allocator );
    resultObject.AddMember( "totalMcEventsEC", totalMc, allocator );
    resultObject.AddMember( "totalMcUncertEC", totalMcUncert, allocator );
    resultObject.AddMember( "totalDataEventsEC", totalData, allocator );
    resultObject.AddMember( "lowerEdge", mcbin.lowerEdge, allocator );
    resultObject.AddMember( "width", mcbin.width, allocator );
    resultObject.AddMember( "CompareScore", score, allocator );
    resultObject.AddMember( "integralScan", integralScan, allocator );
    resultObject.AddMember( "skippedScan", skippedScan, allocator );

    if( verbose ){
        // Add systematic errors
        rs::Value mcSysUncertsObj( rs::kObjectType );

        for( size_t i=0; i < mcbin.mcSysUncerts.size(); i++ ) {
            const double upValue = mcbin.mcSysUncerts[ i ].first;
            const double downValue = mcbin.mcSysUncerts[ i ].second;
            const std::string name = mcbin.mcSysUncertNames[ i ];

            if ( upValue != downValue ) {
                // Non-symmetric errors, append -Down and -Up to the systematic name
                const std::string downName = name + "Down";
                mcSysUncertsObj.AddMember( rs::Value( downName, allocator ).Move(), downValue, allocator );

                const std::string upName = name + "Up";
                mcSysUncertsObj.AddMember( rs::Value( upName, allocator ).Move(), upValue, allocator );
            } else {
                // Symmetric error, just add to document
                mcSysUncertsObj.AddMember( rs::Value( name, allocator ).Move(), upValue, allocator );
            }
        }
        resultObject.AddMember( "mcSysUncerts" , mcSysUncertsObj, allocator );

        rs::Value mcEventsPerProcessObj( rs::kObjectType );
        rs::Value mcStatUncertsObj( rs::kObjectType );

        for ( size_t i=0; i < mcbin.mcEventsPerProcessGroup.size(); i++ ) {
            const std::string name = mcbin.mcProcessGroupNames[ i ];
            const double events = mcbin.mcEventsPerProcessGroup[ i ];
            const double statUncert = mcbin.mcStatUncertPerProcessGroup[ i ];
            mcEventsPerProcessObj.AddMember( rs::Value( name, allocator ).Move(), events, allocator );
            mcStatUncertsObj.AddMember( rs::Value( name, allocator ).Move(), statUncert, allocator );
        }
        resultObject.AddMember( "mcEventsPerProcessGroup", mcEventsPerProcessObj, allocator );
        resultObject.AddMember( "mcStatUncertPerProcessGroup", mcStatUncertsObj, allocator );

        rs::Value dicedDataArray( rs::kArrayType );
        for( double n : dicedData ){
            dicedDataArray.PushBack( n, allocator );
        }
        resultObject.AddMember( "dicedData", dicedDataArray, allocator );
    }

    return resultObject;
}

void ScanResult::writeCsvHeader( std::ostream& out ) {
    out
        << "roi_lower_edge,"
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

void ScanResult::writeCsvLine( std::ostream& out ) const {
    out
        << mcbin.lowerEdge << ","
        << mcbin.width << ","
        << mcbin.getTotalMcEvents() << ","
        << mcbin.getTotalMcUncert() << ","
        << data << ","
        << totalMc << ","
        << totalMcUncert << ","
        << totalData << ","
        << score << ","
        << integralScan << ","
        << skippedScan << ",";
}
