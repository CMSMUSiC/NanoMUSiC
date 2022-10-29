//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/Tokenizer.hpp"
#include "Pxl/Pxl/interface/pxl/core/logging.hpp"

#include "Pxl/Pxl/interface/pxl/core/ConfigLoader.hpp"
#include "Pxl/Pxl/interface/pxl/core/Configuration.hpp"

#include <fstream>

#ifdef WIN32
const char HOME_ENV[] = "HOMEPATH";
#else
const char HOME_ENV[] = "HOME";
#endif

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::ConfigLoader"

namespace pxl
{

/// Add all items from a CSimpleIniA object to the pcl Config instance
void fillConfigFromSimpleIni(const CSimpleIniA &iniFile)
{
    Configuration pxlConfig = Configuration::instance();

    Tokenizer tok;
    tok.setCharType(',', Tokenizer::DELIM);
    CSimpleIniA::TNamesDepend values;
    iniFile.GetAllValues("paths", "pythonModulePaths", values);
    for (CSimpleIniA::TNamesDepend::iterator iter = values.begin(); iter != values.end(); ++iter)
    {
        tok.setText((*iter).pItem);
        while (tok.hasNext())
        {
            std::string s = tok.next();
            pxlConfig.addItem("paths", "pythonModulePath", s);
        }
    }

    iniFile.GetAllValues("paths", "analysisSearchPaths", values);
    for (CSimpleIniA::TNamesDepend::iterator iter = values.begin(); iter != values.end(); ++iter)
    {
        tok.setText((*iter).pItem);
        while (tok.hasNext())
        {
            std::string s = tok.next();
            pxlConfig.addItem("paths", "analysisSearchPaths", s);
        }
    }

    iniFile.GetAllValues("paths", "pluginDirectory", values);
    for (CSimpleIniA::TNamesDepend::iterator iter = values.begin(); iter != values.end(); ++iter)
    {
        tok.setText((*iter).pItem);
        while (tok.hasNext())
        {
            std::string s = tok.next();
            pxlConfig.addItem("paths", "pluginDirectory", s);
        }
    }
}

void fillConfigFromSimpleIni(const std::string &iniFile)
{
}

/// loads configurations from /etc/pxlrc and HOME/.pxlrc
void loadDefaultConfigurations()
{
    CSimpleIniA iniFile(false, true, false);
    SI_Error rc;
#ifdef PXL_SYSTEM_RC
    rc = iniFile.LoadFile(PXL_SYSTEM_RC);
    if (rc < 0)
    {
        PXL_LOG_INFO << "Error reading: " << PXL_SYSTEM_RC;
    }
#endif

    const char *home = getenv(HOME_ENV);
    if (!home)
    {
        PXL_LOG_WARNING << "home path not set: " << HOME_ENV;
        return;
    }

    std::string rcFile = home;
    rcFile += "/.pxlrc";
    rc = iniFile.LoadFile(rcFile.c_str());
    if (rc < 0)
    {
        PXL_LOG_INFO << "Error reading: " << rcFile;
    }

    fillConfigFromSimpleIni(iniFile);
}

} // namespace pxl
