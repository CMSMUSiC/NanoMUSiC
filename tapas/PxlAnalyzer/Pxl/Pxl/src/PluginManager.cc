//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/macros.hh"
#include "Pxl/Pxl/interface/pxl/core/logging.hh"
#include "Pxl/Pxl/interface/pxl/core/PluginManager.hh"
#include "Pxl/Pxl/interface/pxl/core/Configuration.hh"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#else
#include <dlfcn.h>
#include <dirent.h>
#endif

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "string.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
const char PXL_PLUGIN_EXT[] = ".dll";
const char HOME_ENV[] = "HOMEPATH";
static std::string DEFAULT_PLUGIN_DIR;
const char PATH_SEPERATOR[] = ";";
#else
const char PXL_PLUGIN_EXT[] = ".so";
const char HOME_ENV[] = "HOME";
const char PATH_SEPERATOR[] = ":";
#ifdef PXL_PLUGIN_LIBEXEC_DIR
		static std::string DEFAULT_PLUGIN_DIR = STR(PXL_PLUGIN_LIBEXEC_DIR);
#else
		static std::string DEFAULT_PLUGIN_DIR;
#endif
#endif

const char PXL_PLUGIN_DIR[] = "/plugins";
const char PXL_DIR[] = "/.pxl-" PXL_LIB_VERSION;

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD reason, LPVOID reseerved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		const size_t size = 1024;
		TCHAR buffer[size];
		GetModuleFileName(hinstDll, buffer, size);
		DEFAULT_PLUGIN_DIR = buffer;
		std::string::size_type r = DEFAULT_PLUGIN_DIR.find_last_of('\\');
		if (r != std::string::npos)
		DEFAULT_PLUGIN_DIR.resize(r);
	}

	return TRUE;
}
#endif

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::PluginManager"

namespace pxl
{


PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
	shutdown();
}

PluginManager &PluginManager::instance()
{
	static PluginManager manager;
	return manager;
}

void PluginManager::loadConfiguration(const std::string &filename)
{
	std::ifstream file(filename.c_str());

	std::string pluginname;
	while (std::getline(file, pluginname))
	{
		std::string::size_type start = pluginname.find_first_not_of(" \t\r\n");
		if (pluginname.size() >= start && pluginname.at(start) != '#')
		{
			std::string::size_type end = pluginname.find_last_not_of(" \t\r\n");
			std::string filename = std::string(pluginname, start, end + 1);
			try
			{
				loadPlugin(filename);
			} catch (std::exception &e)
			{
				PXL_LOG_WARNING << "Exception in loadPlugin: " << e.what();
			}
		}
	}
}

std::string PluginManager::getUserPluginDirectory()
{
	std::string result; 
	const char* home = getenv(HOME_ENV);
	if (home)
	{
		result.append(home);
		result.append(PXL_DIR);
		result.append(PXL_PLUGIN_DIR);
	}
	return result;
}

std::string PluginManager::getDefaultPluginDirectory()
{
	return DEFAULT_PLUGIN_DIR;
}

void PluginManager::loadPlugins()
{
	PXL_LOG_INFO << "load plugins from installation path";
	if (DEFAULT_PLUGIN_DIR.size() > 0)
	{
		std::string filename_home(DEFAULT_PLUGIN_DIR);
		filename_home.append(PXL_PLUGIN_DIR);
		loadPluginsFromDirectory(filename_home);
	}

	PXL_LOG_INFO << "load plugins from home path";
	std::string userPluginDirectory = getUserPluginDirectory();
	if (userPluginDirectory.size() > 0)
	{
		loadPluginsFromDirectory(userPluginDirectory);
	}

	PXL_LOG_INFO << "load plugins from PXL_PLUGIN_PATH path";
	const char* pxlpluginpath = getenv("PXL_PLUGIN_PATH");
	if (pxlpluginpath)
	{
		std::vector<std::string> paths;
		explode(pxlpluginpath, paths, true, PATH_SEPERATOR);
		for (size_t i = 0; i<paths.size(); i++) {
			PXL_LOG_INFO << "  " << paths[i];
			loadPluginsFromDirectory(paths[i]);
		}
	}
	PXL_LOG_INFO << "load plugins from configured paths";
	
	Configuration pxlConfig = Configuration::instance();
	Configuration::multimapType m = pxlConfig.getSection("paths");
	std::pair<Configuration::multimapIterator, Configuration::multimapIterator> values = m.equal_range("pluginDirectory");
	 
	for (Configuration::multimapIterator iter = values.first; iter!=values.second; ++iter)
	{
		loadPluginsFromDirectory((*iter).second);
	}

}

#ifdef WIN32
void PluginManager::loadPluginsFromDirectory(const std::string &directory)
{
	std::string dir = directory + "\\*.dll";
	logger(LOG_LEVEL_INFO, "load plugins from directory ", directory);

	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	hFind = FindFirstFile(dir.c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		logger(LOG_LEVEL_DEBUG, "directory ", directory, " does not exist.");
		return;
	}

	do
	{
		if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			std::string fname = findFileData.cFileName;
			if (fname.size() <= sizeof(PXL_PLUGIN_EXT))
			continue;

			std::string ext = fname.substr(fname.size()
					- sizeof(PXL_PLUGIN_EXT) + 1);
			if (ext != PXL_PLUGIN_EXT)
			continue;

			try
			{
				std::string filename(directory);
				filename.append("/");
				filename.append(findFileData.cFileName);
				loadSharedLibrary(filename);
			}
			catch (std::exception &e)
			{
				logger(LOG_LEVEL_WARNING, e.what());
			}

		}
	}
	while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}
#else
void PluginManager::loadPluginsFromDirectory(const std::string &directory)
{
	DIR *dir = opendir(directory.c_str());

	if (dir == NULL)
	{
		PXL_LOG_DEBUG << "directory " << directory << " does not exist.";
		return;
	}
	else{
		PXL_LOG_INFO << "load plugins from directory " << directory;
	}

	struct dirent *dp = readdir(dir);
	while (dp != NULL)
	{
		std::string fname = dp->d_name;

		if (fname.size() <= sizeof(PXL_PLUGIN_EXT)) {
			dp = readdir(dir);
			continue;
		}

		std::string ext = fname.substr(fname.size() - sizeof(PXL_PLUGIN_EXT) + 1);
		if (ext != PXL_PLUGIN_EXT) {
			dp = readdir(dir);
			continue;
		}

		try
		{
			std::string filename(directory);
			filename.append("/");
			filename.append(fname);
			loadSharedLibrary(filename);
		} catch (std::exception &e)
		{
			PXL_LOG_WARNING << "Exception in loadPlugin: " << e.what();
		}

		dp = readdir(dir);
	}

	closedir(dir);
}
#endif

void PluginManager::loadPlugin(const std::string &name)
{
	try
	{
		// try as is, full path
		loadSharedLibrary(name);
		return;
	} catch (std::exception &e)
	{
		PXL_LOG_WARNING << "Exception in loadPlugin: " << e.what();
	}

	try
	{
		// make platform module name
		//
		std::string filename(name);
		filename.append(PXL_PLUGIN_EXT);
		loadSharedLibrary(filename);
		return;
	} catch (std::exception &e)
	{
		PXL_LOG_WARNING << "Exception in loadPlugin: " << e.what();
	}

	try
	{
		const char* pxlpluginpath = getenv("PXL_PLUGIN_PATH");
		if (pxlpluginpath) {
			std::vector<std::string> paths;
			explode(pxlpluginpath, paths, true, PATH_SEPERATOR);
			for (size_t i = 0; i < paths.size(); i++)
			{
				PXL_LOG_INFO << "  " << paths[i];
				loadSharedLibrary(paths[i] + "/" + name + PXL_PLUGIN_EXT);
			}
		}
	} catch (std::exception &e)
	{
		PXL_LOG_WARNING << "Exception in loadPlugin: " << e.what();
	}

	try
	{
		// load from home directory
		std::string filename_home(getenv(HOME_ENV));
		filename_home.append(PXL_DIR);
		filename_home.append(PXL_PLUGIN_DIR);
		filename_home.append("/");
		filename_home.append(name);
		filename_home.append(PXL_PLUGIN_EXT);
		loadSharedLibrary(filename_home);
		return;
	} catch (std::exception &e)
	{
		PXL_LOG_WARNING << "Exception in loadPlugin: " << e.what();
	}

	try
	{
		// load from installation diretory
		std::string filename_home(DEFAULT_PLUGIN_DIR);
		filename_home.append(PXL_PLUGIN_DIR);
		filename_home.append("/");
		filename_home.append(name);
		filename_home.append(PXL_PLUGIN_EXT);
		loadSharedLibrary(filename_home);
		return;
	} catch (std::exception &e)
	{
		PXL_LOG_WARNING << "Exception in loadPlugin: " << e.what();
	}

	throw std::runtime_error("plugin '" + name + "' could not be loaded.");
}

void PluginManager::shutdown()
{
}

typedef int (*INITIALIZE_FUNC)(int major, int minor);

#ifdef WIN32
std::string ErrorString(DWORD err)
{
	std::string Error;
	LPTSTR s;
	if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
					| FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, (LPTSTR) &s, 0, NULL)
			== 0)
	{
		// failed
		Error = "unknown error";
	}
	else
	{
		// success
		Error = s;
		::LocalFree(s);
	}

	return Error;
}

void PluginManager::loadSharedLibrary(const std::string &filename)
{

	HMODULE handle = LoadLibrary(filename.c_str());
	if (handle == 0)
	{
		if (::GetLastError() == ERROR_MOD_NOT_FOUND)
		return;
		else
		throw std::runtime_error(
				"PluginManager: error loading module'" + filename
				+ "', " + ErrorString(::GetLastError()));
	}

	INITIALIZE_FUNC init = (INITIALIZE_FUNC) GetProcAddress(handle,
			"pxl_initialize_plugin");
	if (init == 0)
	{
		logger(LOG_LEVEL_WARNING, "plugin ", filename,
				" has no 'pxl_initialize_plugin' function!");
		FreeLibrary(handle);
		return;
	}

	if (init(PXL_MAJOR_VERSION_VALUE, PXL_MINOR_VERSION_VALUE) != 0)
	{
		logger(LOG_LEVEL_WARNING, "plugin ", filename,
				" was build with another version of pxl!");
		FreeLibrary(handle);
		return;
	}

	logger(LOG_LEVEL_DEBUG, "plugin ", filename, " loaded");

}
#else
void PluginManager::loadSharedLibrary(const std::string &filename)
{
	//	void (*register_plugins)(std::map<std::string, ModuleProducerInterface*> &);

	void *handle = dlopen(filename.c_str(), RTLD_LOCAL | RTLD_LAZY);
	if (handle == 0)
		throw std::runtime_error(
				"PluginManager: error while loading module'"
						+ filename + "':" + dlerror());

	INITIALIZE_FUNC init = (INITIALIZE_FUNC) dlsym(handle,
			"pxl_initialize_plugin");
	if (init == 0)
	{
		PXL_LOG_WARNING << "plugin " << filename << " has no 'pxl_initialize_plugin' function!";
		dlclose(handle);
		return;
	}

	if (init(PXL_MAJOR_VERSION_VALUE, PXL_MINOR_VERSION_VALUE) != 0)
	{
		PXL_LOG_WARNING << "plugin " << filename << " was build with another version of pxl!";
		dlclose(handle);
		return;
	}

	PXL_LOG_INFO << "plugin " << filename << " loaded";
}
#endif

} // namespace pxl
