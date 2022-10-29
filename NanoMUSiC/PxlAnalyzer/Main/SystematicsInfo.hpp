#ifndef SystematicsInfo_hh
#define SystematicsInfo_hh

#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"
#include <iostream>
#include <string>
#include <vector>
class SystematicsInfo
{

  public:
    SystematicsInfo(std::string particleType, std::string sysType, std::string funcKey, bool isDifferential = true,
                    double constantShift = 1.); // shift value for non differential systs
    ~SystematicsInfo();
    //~ std::vector< std::string > eventViewIndices;
    std::vector<pxl::EventView *> eventViewPointers;
    bool m_isDifferential;
    std::string m_particleType;
    std::string m_sysType;
    // the key of the shifting function
    // in the systematics class function map
    std::string m_funcKey;
    double m_constantShift;
};
#endif /*SystematicsInfo_hh*/
