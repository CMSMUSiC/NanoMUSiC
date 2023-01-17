//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include "windows.h"
#ifndef S_ISREG
#define S_ISREG(x) (((x)&S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(x) (((x)&S_IFMT) == S_IFDIR)
#endif
#else
#include <sys/time.h>
#include <wordexp.h>
#endif

#include "Pxl/Pxl/interface/pxl/core/functions.hpp"

namespace pxl
{

double getCpuTime()
{
#ifdef WIN32
    return timeGetTime() / 1000.;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0);
#endif
}

const std::string &getDefaultSpaces()
{
    static std::string spaces(" \t\r\n");
    return spaces;
}

std::string trim_right(const std::string &s, const std::string &t = getDefaultSpaces())

{
    std::string::size_type i(s.find_last_not_of(t));

    if (i == std::string::npos)
        return "";
    else
        return std::string(s, 0, i);
}

std::string trim_left(const std::string &s, const std::string &t = getDefaultSpaces())
{
    return std::string(s, s.find_first_not_of(t));
}

std::string trim(const std::string &s)
{
    return trim(s, getDefaultSpaces());
}

std::string trim(const std::string &s, const std::string &t)
{
    std::string::size_type a = s.find_first_not_of(t), b = s.find_last_not_of(t);

    if (a == std::string::npos || b == std::string::npos)

        return std::string();

    return std::string(s, a, b - a + 1);
}

void explode(const std::string &s, std::vector<std::string> &v, const bool trim_spaces, const std::string &t)
{
    std::string::size_type a, b;

    a = s.find_first_not_of(t);
    b = s.find_first_of(t, a);

    while (a != std::string::npos)
    {
        if (trim_spaces)
            v.push_back(trim(s.substr(a, b - a)));
        else
            v.push_back(s.substr(a, b - a));

        a = s.find_first_not_of(t, b);
        b = s.find_first_of(t, a);
    }
}

std::string implode(const std::vector<std::string> &v, const std::string &t)
{
    size_t i;
    std::string s;

    for (i = 0; i < (v.size() - 1); i++)
    {
        s.append(v[i]);
        s.append(t);
    }

    return s + v[i];
}

std::string expandEnvironmentVariables(const std::string &input)
{
    std::string result;
#ifdef WIN32
#define BUFFER_SIZE 32767
    char buffer[BUFFER_SIZE];
    ExpandEnvironmentStrings(input.c_str(), buffer, BUFFER_SIZE);
    result = buffer;
#else
    wordexp_t p;
    wordexp(input.c_str(), &p, 0);
    // in principle wordexp can also resolve [a-c]*.foo or similar. Here
    // hiowever only the first result is returned, as it is currently
    // unclear howto achieve this on windows and also howto handle this
    // in e.g. outputfiles.
    // A future version might return multiple files as string vector?
    result = p.we_wordv[0];
    wordfree(&p);
#endif
    return result;
}

std::string &replace(std::string &context, const std::string &from, const std::string &to)
{
    size_t lookHere = 0;
    size_t foundHere;
    while ((foundHere = context.find(from, lookHere)) != std::string::npos)
    {
        context.replace(foundHere, from.size(), to);
        lookHere = foundHere + to.size();
    }
    return context;
}

std::string getParentDirectory(const std::string &path)
{
    std::string::size_type p = path.find_last_of("/\\");
    if (p == std::string::npos)
        return std::string("./");
    else
        return path.substr(0, p);
}

bool createDirectory(const std::string &path)
{
#ifdef WIN32
    BOOL result = ::CreateDirectoryA(path.c_str(), 0);
    return result == TRUE;
#else
    int result = mkdir(path.c_str(), 0x0777);
    return (result == 0);
#endif
}

bool isAbsolutePath(const std::string &_path)
{
    std::string schema, path;
    splitSchema(_path, schema, path);
    if (!schema.empty())
        return true;

#if WIN32
    if (path.size() < 3)
        return false;

    if (path[1] == ':')
    {
        char drive = tolower(path[0]);
        if (drive < 'a' || drive > 'z')
            return false;
    }
    else if (path[1] == '\\')
    {
        if (path[0] != '\\')
            return false;
    }
    return true;
#else
    if (path.size() >= 1 && path[0] == '/')
        return true;
    else
        return false;
#endif
}

bool isDirectory(const std::string &path)
{
#ifdef _MSC_VER
    struct _stat buf;
    int result = _stat(path.c_str(), &buf);
    if (result == 0 && S_ISDIR(buf.st_mode))
        return true;
    else
        return false;
#else
    struct stat buf;
    int result = stat(path.c_str(), &buf);
    if (result == 0 && S_ISDIR(buf.st_mode))
        return true;
    else
        return false;
#endif
}

bool isFile(const std::string &path)
{
#ifdef _MSC_VER
    struct _stat buf;
    int result = _stat(path.c_str(), &buf);
    if (result == 0 && S_ISREG(buf.st_mode))
        return true;
    else
        return false;
#else
    struct stat buf;
    int result = stat(path.c_str(), &buf);
    if (result == 0 && S_ISREG(buf.st_mode))
        return true;
    else
        return false;
#endif
}

std::string clearPathName(const std::string &path)
{
#ifdef WIN32
    std::string cleared = path;
    for (std::string::iterator i = cleared.begin(); i != cleared.end(); i++)
        if (*i == '/')
            *i = '\\';
    return cleared;
#else
    std::string cleared = path;
    for (std::string::iterator i = cleared.begin(); i != cleared.end(); i++)
        if (*i == '\\')
            *i = '/';
    return cleared;
#endif
}

void splitSchema(const std::string &url, std::string &schema, std::string &path)
{
    std::string::size_type schema_start_pos = url.find_first_not_of(" \t\n\r");
    std::string::size_type schema_end_pos = url.find_first_of(':');

    bool noSchmea = schema_end_pos == std::string::npos;
    bool windowsPath = (schema_end_pos - schema_start_pos) == 1;
    if (noSchmea || windowsPath)
    {
        schema.clear();
        path = url;
    }
    else
    {
        schema = url.substr(schema_start_pos, schema_end_pos - schema_start_pos);
        path = url.substr(schema_end_pos + 1);
    }
}

} // namespace pxl
