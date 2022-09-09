//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <iostream>
#include <string>

#include "Pxl/Pxl/interface/pxl/hep/EventView.hh"

namespace pxl
{

std::ostream &EventView::print(int level, std::ostream &os, int pan) const
{
    printPan1st(os, pan) << "EventView: " << getName() << std::endl;

    if (level > 0)
        printContent(level, os, pan);

    for (ObjectOwner::const_iterator iter = getObjectOwner().begin(); iter != getObjectOwner().end(); iter++)
    {

        if ((*iter)->getMotherRelations().size() == 0)
            (*iter)->printDecayTree(level, os, pan);
    }
    return os;
}

const Id &EventView::getStaticTypeId()
{
    static const Id id("c8db3cce-dc4b-421e-882a-83e213c9451f");
    return id;
}

} // namespace pxl
