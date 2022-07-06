//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/InformationChunk.hh"
#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hh"

namespace pxl {

/// Write data to the OutputStream \p out.
void InformationChunk::serialize(const OutputStream &out) const
{
	Serializable::serialize(out);
	UserRecordHelper::serialize(out);
}

/// Read data from the InputStream \p in.
void InformationChunk::deserialize(const InputStream &in)
{
	Serializable::deserialize(in);
	UserRecordHelper::deserialize(in);
}

}
