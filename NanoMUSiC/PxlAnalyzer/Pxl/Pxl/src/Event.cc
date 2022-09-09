//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/Event.hh"

namespace pxl
{

void Event::serialize(const OutputStream &out) const
{
    Serializable::serialize(out);
    _objects.serialize(out);
    UserRecordHelper::serialize(out);
}

void Event::deserialize(const InputStream &in)
{
    Serializable::deserialize(in);
    _objects.deserialize(in);
    UserRecordHelper::deserialize(in);
}

std::ostream &Event::print(int level, std::ostream &os, int pan) const
{
    os << "Event." << std::endl;

    if (level > 0)
        getUserRecords().print(level, os, pan);

    for (ObjectOwner::const_iterator iter = _objects.begin(); iter != _objects.end(); ++iter)
    {
        if ((*iter)->getMotherRelations().size() == 0)
            (*iter)->printDecayTree(0, os, pan);
    }
    return os;
}

const Id &Event::getStaticTypeId()
{
    static const Id id("c95f7434-2481-45e2-91dc-9baff0669bb3");
    return id;
}

} // namespace pxl
