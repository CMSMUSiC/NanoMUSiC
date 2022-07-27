#include <utility>

#include "SystematicsInfo.hh"
//--------------------Constructor-----------------------------------------------------------------

SystematicsInfo::SystematicsInfo(std::string particleType,
        std::string sysType,
        std::string funcKey,
        bool isDifferential,
        double constantShift ):
    eventViewPointers( {} ),
    m_isDifferential( isDifferential ),
    m_particleType( std::move(particleType) ),
    m_sysType( std::move(sysType) ),
    m_funcKey( std::move(funcKey) ),
    m_constantShift( constantShift )
{}

//--------------------Destructor------------------------------------------------------------------

SystematicsInfo::~SystematicsInfo(){}
