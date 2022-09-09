//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <iostream>
#include <string>

#include "Pxl/Pxl/interface/pxl/hep/Vertex.hh"

namespace pxl
{

bool operator==(const Vertex &obj1, const Vertex &obj2)
{
    return obj1.getVector() == obj2.getVector();
}

bool operator!=(const Vertex &obj1, const Vertex &obj2)
{
    return obj1.getVector() != obj2.getVector();
}

std::ostream &Vertex::print(int level, std::ostream &os, int pan) const
{
    printPan1st(os, pan) << "Vertex: '" << getName() << "', x = (" << getX() << ", " << getY() << ", " << getZ() << ")"
                         << std::endl;
    if (level > 0)
        printContent(level, os, pan);
    return os;
}

} // namespace pxl
