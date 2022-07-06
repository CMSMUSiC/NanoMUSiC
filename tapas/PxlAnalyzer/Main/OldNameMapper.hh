#ifndef OldNameMapper_hh
#define OldNameMapper_hh

#include <string>
#include <map>
#include <sstream>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"


class OldNameMapper {
    struct ReplacementInfo
    {
        ReplacementInfo( std::string varName, std::string varReplacement, int replacementVersion ):
            variableName( std::move(varName) ),
            replacement( std::move(varReplacement)),
            version( replacementVersion )
        { };
        std::string variableName;
        std::string replacement;
        int version;
    };

    public:
       // Constructor
       OldNameMapper( );
       // Destructor
        ~OldNameMapper();
       std::string getUserRecordName( const pxl::Object* obj, std::string variableName);
       std::string getUserRecordName( const pxl::Object* obj, const std::string& partName, std::string variableName);
       std::string getUserRecordName( const pxl::Object* obj, std::string variableName,
                                      std::vector< ReplacementInfo > baseReplacementMap,
                                      std::map< std::string , std::string > &cacheReplacementMap,
                                      bool isToplevel=true
                                    );
        void addReplacement( std::string variableName,
                             std::string replacement,
                             std::vector< ReplacementInfo >& baseReplacementVec );

        void addReplacement( std::string variableName, std::string replacement);

        void addReplacement( const std::string& partName, std::string variableName, std::string replacement);

    private:
        std::vector< ReplacementInfo > m_generalReplacements;
        std::map< std::string, std::vector< ReplacementInfo > > m_particleReplacements;
        std::map< std::string, std::map< std::string, std::string > > m_ActiveParticleReplacements;
        std::map<  std::string , std::string  > m_ActiveGeneralReplacements;

};
#endif
