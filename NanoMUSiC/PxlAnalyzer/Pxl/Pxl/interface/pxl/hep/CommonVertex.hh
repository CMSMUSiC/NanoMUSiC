//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_HEP_COMMON_VERTEX_HH
#define PXL_HEP_COMMON_VERTEX_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

namespace pxl
{
/**
 * This is the common, pure virtual interface class for vertices.
 */

class PXL_DLL_EXPORT CommonVertex
{
public:
	virtual ~CommonVertex()
	{
	}
		
	//getters for basic vector quantities.
	virtual double getX() const = 0;
	virtual double getY() const = 0;
	virtual double getZ() const = 0;

	//setters for basic fourvector quantities
	virtual void setX(double x) = 0;
	virtual void setY(double y) = 0;
	virtual void setZ(double z) = 0;

	//setters for basic fourvector quantities
	virtual void setXYZ(double x, double y, double z) = 0;
	virtual void addXYZ(double x, double y, double z) = 0;

};

}

#endif /*PXL_HEP_COMMON_VERTEX_HH*/
