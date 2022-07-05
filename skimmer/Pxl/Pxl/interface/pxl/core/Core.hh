//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_CORE_CLASS_HH
#define PXL_CORE_CLASS_HH

#include "Pxl/Pxl/interface/pxl/core/macros.hh"

namespace pxl
{

class PXL_DLL_EXPORT Core
{
public:
	static void initialize();
	static void shutdown();
};

}

#endif /* INITIALIZE_HH_ */
