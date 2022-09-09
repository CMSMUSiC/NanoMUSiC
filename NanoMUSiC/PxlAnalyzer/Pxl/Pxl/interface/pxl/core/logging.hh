//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_LOGGING_HH
#define PXL_BASE_LOGGING_HH

#include "Pxl/Pxl/interface/pxl/core/functions.hh"
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

namespace pxl
{

enum LogLevel
{
    LOG_LEVEL_ALL,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE
};

PXL_DLL_EXPORT LogLevel intToLogLevel(int i);
PXL_DLL_EXPORT const std::string &LogLevelToString(LogLevel level);

class PXL_DLL_EXPORT LogHandler
{
  public:
    virtual ~LogHandler()
    {
    }

    virtual void handle(LogLevel level, time_t timestamp, const std::string &module, const std::string &message) = 0;
};

class PXL_DLL_EXPORT ConsoleLogHandler : public LogHandler
{
    std::vector<std::string> enabledModules;
    std::vector<std::string> disabledModules;
    std::string warningColor, errorColor, endColor;

  public:
    ConsoleLogHandler();

    virtual ~ConsoleLogHandler()
    {
    }

    void handle(LogLevel level, time_t timestamp, const std::string &module, const std::string &message);
};

class PXL_DLL_EXPORT LogDispatcher
{
  public:
    typedef std::vector<std::pair<LogHandler *, LogLevel>> handlers_t;

  private:
    handlers_t handlers;
    LogLevel lowestLogLevel;
    ConsoleLogHandler consoleLogHandler;
    std::string indent;

    void updateLowestLogLevel();

  public:
    LogDispatcher();

    void pushIndent(char c)
    {
        indent.append(1, c);
    }

    void popIndent()
    {
        indent.resize(indent.length() - 1);
    }

    const std::string &getIndent()
    {
        return indent;
    }

    void setHandler(LogHandler *handler, LogLevel loglevel);

    void removeHandler(LogHandler *handler);

    void dispatch(LogLevel level, const std::string &module, const std::string &message);

    LogLevel getLowestLogLevel();

    void disableConsoleLogHandler();
    void enableConsoleLogHandler(LogLevel level);

    static LogDispatcher &instance();
};

class PXL_DLL_EXPORT Logger
{
  private:
    std::string module;

  public:
    Logger(const char *module_name) : module(module_name)
    {
    }

    void log(LogLevel level, const std::string &msg)
    {
        if (LogDispatcher::instance().getLowestLogLevel() <= level)
        {
            LogDispatcher::instance().dispatch(level, module, LogDispatcher::instance().getIndent() + msg);
        }
    }

    template <typename T0> void operator()(LogLevel level, const T0 &t0)
    {
        if (LogDispatcher::instance().getLowestLogLevel() <= level)
        {
            std::stringstream stream;
            stream << LogDispatcher::instance().getIndent();
            stream << t0;
            LogDispatcher::instance().dispatch(level, module, stream.str());
        }
    }

    template <typename T0, typename T1> void operator()(LogLevel level, const T0 &t0, const T1 &t1)
    {
        if (LogDispatcher::instance().getLowestLogLevel() <= level)
        {
            std::stringstream stream;
            stream << LogDispatcher::instance().getIndent();
            stream << t0;
            stream << " ";
            stream << t1;
            LogDispatcher::instance().dispatch(level, module, stream.str());
        }
    }

    template <typename T0, typename T1, typename T2>
    void operator()(LogLevel level, const T0 &t0, const T1 &t1, const T2 &t2)
    {
        if (LogDispatcher::instance().getLowestLogLevel() <= level)
        {
            std::stringstream stream;
            stream << LogDispatcher::instance().getIndent();
            stream << t0 << " " << t1 << " " << t2;
            LogDispatcher::instance().dispatch(level, module, stream.str());
        }
    }

    template <typename T0, typename T1, typename T2, typename T3>
    void operator()(LogLevel level, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3)
    {
        if (LogDispatcher::instance().getLowestLogLevel() <= level)
        {
            std::stringstream stream;
            stream << LogDispatcher::instance().getIndent();
            stream << t0 << " " << t1 << " " << t2 << " " << t3;
            LogDispatcher::instance().dispatch(level, module, stream.str());
        }
    }

    template <typename T0, typename T1, typename T2, typename T3, typename T4>
    void operator()(LogLevel level, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
    {
        if (LogDispatcher::instance().getLowestLogLevel() <= level)
        {
            std::stringstream stream;
            stream << LogDispatcher::instance().getIndent();
            stream << t0 << " " << t1 << " " << t2 << " " << t3 << " " << t4;
            LogDispatcher::instance().dispatch(level, module, stream.str());
        }
    }

    template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    void operator()(LogLevel level, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
    {
        if (LogDispatcher::instance().getLowestLogLevel() <= level)
        {
            std::stringstream stream;
            stream << LogDispatcher::instance().getIndent();
            stream << t0 << " " << t1 << " " << t2 << " " << t3 << " " << t4 << " " << t5;
            LogDispatcher::instance().dispatch(level, module, stream.str());
        }
    }

    template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void operator()(LogLevel level, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5,
                    const T6 &t6)
    {
        if (LogDispatcher::instance().getLowestLogLevel() <= level)
        {
            std::stringstream stream;
            stream << LogDispatcher::instance().getIndent();
            stream << t0 << " " << t1 << " " << t2 << " " << t3 << " " << t4 << " " << t5 << " " << t6;
            LogDispatcher::instance().dispatch(level, module, stream.str());
        }
    }
};

class LogBuffer
{
    std::stringstream stream;
    LogLevel level;
    const char *module;

  public:
    LogBuffer(LogLevel level, const char *module);
    ~LogBuffer();

    inline operator std::ostream &()
    {
        return stream;
    }

    template <typename T> inline LogBuffer &operator<<(T &data)
    {
        stream << data;
        return *this;
    }

    inline LogBuffer &operator<<(std::ostream &(*func)(std::ostream &))
    {
        stream << func;
        return *this;
    }
};

} // namespace pxl

#define PXL_LOG_ERROR                                                                                                  \
    if (pxl::LogDispatcher::instance().getLowestLogLevel() > pxl::LOG_LEVEL_ERROR)                                     \
    {                                                                                                                  \
    }                                                                                                                  \
    else                                                                                                               \
        pxl::LogBuffer(pxl::LOG_LEVEL_ERROR, PXL_LOG_MODULE_NAME)
#define PXL_LOG_WARNING                                                                                                \
    if (pxl::LogDispatcher::instance().getLowestLogLevel() > pxl::LOG_LEVEL_WARNING)                                   \
    {                                                                                                                  \
    }                                                                                                                  \
    else                                                                                                               \
        pxl::LogBuffer(pxl::LOG_LEVEL_WARNING, PXL_LOG_MODULE_NAME)
#define PXL_LOG_INFO                                                                                                   \
    if (pxl::LogDispatcher::instance().getLowestLogLevel() > pxl::LOG_LEVEL_INFO)                                      \
    {                                                                                                                  \
    }                                                                                                                  \
    else                                                                                                               \
        pxl::LogBuffer(pxl::LOG_LEVEL_INFO, PXL_LOG_MODULE_NAME)
#define PXL_LOG_DEBUG                                                                                                  \
    if (pxl::LogDispatcher::instance().getLowestLogLevel() > pxl::LOG_LEVEL_DEBUG)                                     \
    {                                                                                                                  \
    }                                                                                                                  \
    else                                                                                                               \
        pxl::LogBuffer(pxl::LOG_LEVEL_DEBUG, PXL_LOG_MODULE_NAME)

#endif // PXL_BASE_LOGGING_HH
