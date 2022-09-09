//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_MACROS_HH
#define PXL_BASE_MACROS_HH

#include <cstddef>

#include "Pxl/Pxl/interface/pxl/core/config.hh"

#undef PXL_LIKELY
#undef PXL_UNLIKELY
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define PXL_NORETURN __attribute__((__noreturn__))
#define PXL_LIKELY(expr) (__builtin_expect((bool)(expr), true))
#define PXL_UNLIKELY(expr) (__builtin_expect((bool)(expr), false))
#else
#define PXL_NORETURN
#define PXL_LIKELY(expr) (expr)
#define PXL_UNLIKELY(expr) (expr)
#endif

#ifdef offsetof
#define PXL_OFFSETOF(t, f) ((std::ptrdiff_t)offsetof(t, f))
#else
#define PXL_OFFSETOF(t, f) ((std::ptrdiff_t)((char *)&((t *)0)->f))
#endif

#define PXL_BASE(t, f, v) (reinterpret_cast<t *>(reinterpret_cast<char *>(v) - PXL_OFFSETOF(t, f)))

#ifdef _MSC_VER
#ifdef PXL_EXPORT
#define PXL_DLL_EXPORT __declspec(dllexport)
#else
#define PXL_DLL_EXPORT __declspec(dllimport)
#endif // PXL_EXPORT
#else
#define PXL_DLL_EXPORT
#endif // _MSC_VER
#ifndef PXL_VERSION
#define PXL_VERSION "trunk"
#endif

#ifdef WIN32
#define PXL_PATH_SEPERATOR "\\"
#else
#define PXL_PATH_SEPERATOR "/"
#endif

#define _STR(x) #x
#define STR(x) _STR(x)

#endif // PXL_BASE_MACROS_HH
