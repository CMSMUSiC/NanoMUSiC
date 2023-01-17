//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef INITIALIZE_HEP_HH_
#define INITIALIZE_HEP_HH_

#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

namespace pxl
{
class PXL_DLL_EXPORT Hep
{
  public:
    static void initialize();
    static void shutdown();
};
} // namespace pxl

#endif /* INITIALIZE_HH_ */
