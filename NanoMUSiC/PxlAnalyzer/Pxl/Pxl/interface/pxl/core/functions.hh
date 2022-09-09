//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_FUNCTIONS_HH
#define PXL_BASE_FUNCTIONS_HH

#include <string>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core/macros.hh"

namespace pxl
{

/// @internal This function returns a platform-specific CPU timestamp; internally used for performance tests.
PXL_DLL_EXPORT double getCpuTime();

/**
 * Function to remove whitespaces from the end of the string.
 * @param s String to be trimmed.
 * @param t String containing whitespaces. Default: space, tab, carriage return and newline.
 */
std::string PXL_DLL_EXPORT trim_right(const std::string &s, const std::string &t);
/**
 * Function to remove whitespaces from the beginning of the string.
 * @param s String to be trimmed.
 * @param t String containing whitespaces. Default: space, tab, carriage return and newline.
 */
std::string PXL_DLL_EXPORT trim_left(const std::string &s, const std::string &t);

/**
 * Function to remove whitespaces from the beginning and the end of the string.
 * @param s String to be trimmed.
 * @param t String containing whitespaces. Default: space, tab, carriage return and newline.
 */
std::string PXL_DLL_EXPORT trim(const std::string &s, const std::string &t);
std::string PXL_DLL_EXPORT trim(const std::string &s);

/**
 * Splits a string into pieces, and returns them in an array.
 * @param s String to be exploded.
 * @param v Vector which receives the pieces.
 * @param t String containing whitespaces. Default: space, tab, carriage return and newline.
 * @param trim_spaces Flag to decide if pieces should be trimmed. Default: false.
 */
void PXL_DLL_EXPORT explode(const std::string &s, std::vector<std::string> &v, const bool trim_spaces,
                            const std::string &t);
/**
 * Function to assemble strings from a vector into one strng.
 * @param v Vector which conatins the string pieces.
 * @param t String which is places between peaces. Default: one space " ".
 * @return Assembled string.
 */
PXL_DLL_EXPORT std::string implode(const std::vector<std::string> &v, const std::string &t);

/**
 * Function to expand enviroment variables in a string.
 * @param input String
 * @return input string with expanded variables.
 */
std::string PXL_DLL_EXPORT expandEnvironmentVariables(const std::string &input);

PXL_DLL_EXPORT std::string &replace(std::string &context, const std::string &from, const std::string &to);

std::string PXL_DLL_EXPORT getParentDirectory(const std::string &path);
bool PXL_DLL_EXPORT createDirectory(const std::string &path);
bool PXL_DLL_EXPORT isAbsolutePath(const std::string &path);
bool PXL_DLL_EXPORT isDirectory(const std::string &path);
bool PXL_DLL_EXPORT isFile(const std::string &path);
std::string PXL_DLL_EXPORT clearPathName(const std::string &path);
void PXL_DLL_EXPORT splitSchema(const std::string &url, std::string &schema, std::string &path);

template <class T> inline void safe_delete(T *&p)
{
    if (p)
    {
        delete p;
        p = 0;
    }
}

inline const char *safe_string(const char *ptr)
{
    if (ptr == 0)
        return "";
    else
        return ptr;
}

} // namespace pxl

#endif // PXL_BASE_FUNCTIONS_HH
