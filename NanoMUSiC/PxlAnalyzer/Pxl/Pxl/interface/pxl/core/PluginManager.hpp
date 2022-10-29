//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_CORE_PLUGIN_MANAGER_H
#define PXL_CORE_PLUGIN_MANAGER_H

#include <map>
#include <stdexcept>
#include <string>

#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

namespace pxl
{

class PXL_DLL_EXPORT PluginManager
{
    void loadSharedLibrary(const std::string &filename);

  public:
    PluginManager();
    virtual ~PluginManager();
    void shutdown();

    static PluginManager &instance();
    static std::string getUserPluginDirectory();
    static std::string getDefaultPluginDirectory();

    void loadPlugin(const std::string &filename);
    void loadPluginsFromDirectory(const std::string &name);
    void loadPlugins();
    void loadConfiguration(const std::string &filename);
};

} // namespace pxl

#ifdef _MSC_VER
#define PXL_PLUGIN_EXPORT __declspec(dllexport)
#else
#define PXL_PLUGIN_EXPORT
#endif

#define PXL_PLUGIN_INIT                                                                                                \
    extern "C" int PXL_PLUGIN_EXPORT pxl_initialize_plugin(int major, int minor)                                       \
    {                                                                                                                  \
        if (major != PXL_MAJOR_VERSION_VALUE || minor != PXL_MINOR_VERSION_VALUE)                                      \
        {                                                                                                              \
            printf("this plugin was build with PXL %d.%d", PXL_MAJOR_VERSION_VALUE, PXL_MINOR_VERSION_VALUE);          \
            return 1;                                                                                                  \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            initialize();                                                                                              \
            return 0;                                                                                                  \
        }                                                                                                              \
    }

#endif // PXL_PLUGIN_MANAGER_H
