//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/Relations.hpp"
#include "Pxl/Pxl/interface/pxl/core/Relative.hpp"

namespace pxl
{

void Relations::serialize(const OutputStream &out) const
{
    out.writeInt(size());
    for (const_iterator iter = begin(); iter != end(); ++iter)
    {
        (*iter)->getId().serialize(out);
    }
}

} // namespace pxl
