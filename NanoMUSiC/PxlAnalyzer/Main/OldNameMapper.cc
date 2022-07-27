#include <utility>

#include "OldNameMapper.hh"
//~
// Constructor
OldNameMapper::OldNameMapper ()
{
    // add general replacements in order of change in skimmer
    //addReplacement(  "general.oldname", "general.oldname");

    // add particle specific replacements in order of change in skimmer
    addReplacement( "Gamma", "sigma_iEta_iEta", "iEta_iEta" );
    addReplacement( "Gamma", "hadTowOverEm", "HoverE2012" );
}

OldNameMapper::~OldNameMapper() {
}

void OldNameMapper::addReplacement( std::string variableName,
                                    std::string replacement,
                                    std::vector< ReplacementInfo >& baseReplacementVec ){
    baseReplacementVec.push_back( ReplacementInfo( std::move(variableName), std::move(replacement), baseReplacementVec.size() + 1 ) );
}

void OldNameMapper::addReplacement( std::string variableName, std::string replacement){
    addReplacement( std::move(variableName), std::move(replacement), m_generalReplacements );
}

void OldNameMapper::addReplacement( const std::string& partName,
                                    std::string variableName,
                                    std::string replacement){
    // check if vec for particle type exists yet
    if( m_particleReplacements.find( partName ) == m_particleReplacements.end() ){
        m_particleReplacements[partName] = std::vector< ReplacementInfo > ();
    }
    addReplacement( std::move(variableName), std::move(replacement),  m_particleReplacements[partName] );
}

std::string OldNameMapper::getUserRecordName(const pxl::Object* obj, const std::string& partName, std::string variableName ){
    // throw error if no map exists fo particle
    if( m_particleReplacements.find(partName) == m_particleReplacements.end() ){
        std::stringstream errorMessage;
        errorMessage << "OldNameMapper: No replacementMap found for particle type: " << partName << std::endl;
        throw std::runtime_error( errorMessage.str() );
    }
    std::vector< ReplacementInfo > &basePartMap = m_particleReplacements[partName];
    std::map< std::string , std::string > &cachePartMap = m_ActiveParticleReplacements[partName];
    return getUserRecordName( obj, std::move(variableName), basePartMap, cachePartMap );
}

std::string OldNameMapper::getUserRecordName( const pxl::Object* obj, std::string variableName){
    return  getUserRecordName( obj, std::move(variableName), m_generalReplacements, m_ActiveGeneralReplacements );
}

// Main implementation of getUserRecordName, which returns the replacement and the most recent usable version
// for a variable with different versions.
std::string OldNameMapper::getUserRecordName( const pxl::Object* obj,
                                            std::string variableName,
                                            std::vector< ReplacementInfo > baseReplacementMap,
                                            std::map< std::string , std::string > &cacheReplacementMap,
                                             bool isToplevel){

    // Return replacement from cache map if it exists
    if( cacheReplacementMap.find( variableName ) != cacheReplacementMap.end() ){
         return cacheReplacementMap.at( variableName );
    }
    // Check if this variable exists in the pxlio and add it to the cache if yes
    if ( obj->hasUserRecord( variableName ) ){
         cacheReplacementMap.emplace( variableName,  variableName );
         return variableName;
    }
    // The variableName was not requested yet. Add it and all
    // variables in the changed version region
    auto replaceIter = std::find_if( baseReplacementMap.begin(),
                                     baseReplacementMap.end(),
                                     [&] (ReplacementInfo const& p) {
                                            return p.variableName == variableName;
                                        }
                                     );
    for( ReplacementInfo info : baseReplacementMap) std::cout << "blub "<<info.variableName << std::endl;
    if( replaceIter != baseReplacementMap.end() ){
        std::string outputReplacement;
        auto innerIter = replaceIter;
        // check if there are cascading changes for this variable name
        while( innerIter != baseReplacementMap.end() ){
            outputReplacement = innerIter->replacement;
            innerIter = std::find_if( baseReplacementMap.begin(),
                                      baseReplacementMap.end(),
                                      [&] (ReplacementInfo const& p) {
                                            return p.variableName == outputReplacement;
                                            }
                                       );
        }
        // Add all replacements with a version number smaller than the current one
        for( ; replaceIter != baseReplacementMap.end(); replaceIter++ ){
            std::string name =  replaceIter->variableName;
            std::string replacement =  replaceIter->replacement;
            if( cacheReplacementMap.find( name ) != cacheReplacementMap.end() )break;
            std::cout << "OldNameMapper: Replacing "<< name << " with " << replacement << std::endl;
            cacheReplacementMap.emplace( name, replacement );
        }
        return outputReplacement;
    }else{
        std::stringstream errorMessage;
        errorMessage << "OldNameMapper: No replacement found for variable: " << variableName << std::endl;
        throw std::runtime_error( errorMessage.str() );
    }
}
