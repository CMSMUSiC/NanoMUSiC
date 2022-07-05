//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/logging.hh"

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <algorithm>

#ifdef __unix__
#include <unistd.h>
#include <stdio.h>
#endif

namespace pxl
{

static bool wildcardCompare(const std::string &pattern, const std::string &str)
{
	size_t ps = pattern.size();
	size_t ss = str.size();

	// wildcard?
	bool wildcard = pattern[ps - 1] == '*';
	if (wildcard)
		ps -= 1;

	// too short?
	if (ss < ps)
		return false;

	// too long?
	if ((wildcard == false) && (ss > ps))
		return false;

	for (size_t i = 0; i < ps; i++)
	{
		if (pattern[i] != str[i])
			return false;
	}

	return true;
}

void ConsoleLogHandler::handle(LogLevel level, time_t timestamp,
		const std::string &module, const std::string &message)
{
	// if no enabled modules are specified, enable all
	bool enabled = enabledModules.empty();
	for (size_t i = 0; i < enabledModules.size(); i++)
	{
		if (wildcardCompare(enabledModules[i], module))
		{
			enabled = true;
			break;
		}

	}

	if (enabled == false)
		return;

	// if no enabled modules are specified, enable all
	if (disabledModules.empty() == false)
	{
		for (size_t i = 0; i < disabledModules.size(); i++)
		{
			if (wildcardCompare(disabledModules[i], module))
				return;
		}
	}

	tm *t = localtime(&timestamp);

	std::stringstream sstr;

	// print date
	sstr << t->tm_year + 1900 << "-";
	if (t->tm_mon + 1 < 10)
		sstr << "0";
	sstr << t->tm_mon + 1 << "-";
	if (t->tm_mday < 10)
		sstr << "0";
	sstr << t->tm_mday << " ";

	// print time
	if (t->tm_hour < 10)
		sstr << "0";
	sstr << t->tm_hour << ":";
	if (t->tm_min < 10)
		sstr << "0";
	sstr << t->tm_min << ":";
	if (t->tm_sec < 10)
		sstr << "0";
	sstr << t->tm_sec;

	if (level >= LOG_LEVEL_ERROR)
		std::cerr << sstr.str() << " [" << errorColor << LogLevelToString(level)
				<< endColor << "] " << module << " " << message << std::endl;
	else if (level == LOG_LEVEL_WARNING)
		std::cerr << sstr.str() << " [" << warningColor
				<< LogLevelToString(level) << endColor << "] " << module << " "
				<< message << std::endl;
	else
		std::cout << sstr.str() << " [" << LogLevelToString(level) << "] "
				<< module << " " << message << std::endl;

}

ConsoleLogHandler::ConsoleLogHandler()
{
	const char *enabledModulesString = ::getenv("PXL_LOG_ENABLE");
	if (enabledModulesString)
	{
		pxl::explode(enabledModulesString, enabledModules, true, ",");
	}

	const char *disabledModulesString = ::getenv("PXL_LOG_DISABLE");
	if (disabledModulesString)
	{
		pxl::explode(disabledModulesString, disabledModules, true, ",");
	}

#if __unix__
	if (isatty(fileno(stderr)))
	{
		warningColor = "\e[0;33m";
		errorColor = "\e[0;31m";
		endColor = "\e[m";
	}
#endif
}

LogDispatcher::LogDispatcher() :
		lowestLogLevel(LOG_LEVEL_ERROR)
{
	int level = LOG_LEVEL_WARNING;
	const char *levelEnv = ::getenv("PXL_LOG_LEVEL");
	if (levelEnv)
		level = atoi(levelEnv);
	enableConsoleLogHandler(intToLogLevel(level));
}

void LogDispatcher::disableConsoleLogHandler()
{
	removeHandler(&consoleLogHandler);
}

void LogDispatcher::enableConsoleLogHandler(LogLevel level)
{
	setHandler(&consoleLogHandler, level);
}

void LogDispatcher::updateLowestLogLevel()
{
	lowestLogLevel = LOG_LEVEL_NONE;
	for (handlers_t::iterator i = handlers.begin(); i != handlers.end(); i++)
		if (i->second < lowestLogLevel)
			lowestLogLevel = i->second;
}

void LogDispatcher::setHandler(LogHandler *handler, LogLevel loglevel)
{
	// replace existing handler
	for (handlers_t::iterator i = handlers.begin(); i != handlers.end(); i++)
	{
		if (i->first == handler)
		{
			i->second = loglevel;
			updateLowestLogLevel();
			return;
		}
	}

	// add handler
	handlers.push_back(std::make_pair(handler, loglevel));
	updateLowestLogLevel();
}

void LogDispatcher::removeHandler(LogHandler *handler)
{
	for (handlers_t::iterator i = handlers.begin(); i != handlers.end(); i++)
	{
		if (i->first == handler)
		{
			handlers.erase(i);
			updateLowestLogLevel();
			return;
		}
	}
}

void LogDispatcher::dispatch(LogLevel level, const std::string &module,
		const std::string &message)
{
	time_t timestamp;
	time(&timestamp);
	for (handlers_t::iterator i = handlers.begin(); i != handlers.end(); i++)
	{
		if (i->second <= level)
			i->first->handle(level, timestamp, module, message);
	}
}

LogLevel LogDispatcher::getLowestLogLevel()
{
	return lowestLogLevel;
}

LogDispatcher& LogDispatcher::instance()
{
	static LogDispatcher dispatcher;
	return dispatcher;
}

LogLevel intToLogLevel(int i)
{
	if (i <= LOG_LEVEL_ALL)
		return LOG_LEVEL_ALL;
	else if (i == LOG_LEVEL_DEBUG)
		return LOG_LEVEL_DEBUG;
	else if (i == LOG_LEVEL_INFO)
		return LOG_LEVEL_INFO;
	else if (i == LOG_LEVEL_WARNING)
		return LOG_LEVEL_WARNING;
	else if (i == LOG_LEVEL_ERROR)
		return LOG_LEVEL_ERROR;
	else
		return LOG_LEVEL_NONE;
}

const std::string &LogLevelToString(LogLevel level)
{
	static std::string _all("ALL    ");
	static std::string _debug("DEBUG  ");
	static std::string _info("INFO   ");
	static std::string _waning("WARNING");
	static std::string _error("ERROR  ");
	static std::string _none("NONE   ");

	if (level == LOG_LEVEL_ALL)
		return _all;
	else if (level == LOG_LEVEL_DEBUG)
		return _debug;
	else if (level == LOG_LEVEL_INFO)
		return _info;
	else if (level == LOG_LEVEL_WARNING)
		return _waning;
	else if (level == LOG_LEVEL_ERROR)
		return _error;
	else
		return _none;
}

LogBuffer::LogBuffer(LogLevel level, const char *module) :
		level(level), module(module)
{
}

LogBuffer::~LogBuffer()
{
	LogDispatcher::instance().dispatch(level, module, stream.str());
}

} // namespace pxl
