//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/Object.hh"
#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hh"

namespace pxl
{

void Object::serialize(const OutputStream &out) const
{
    Relative::serialize(out);
    out.writeBool(_locked);
    out.writeInt(_workflag);
    UserRecordHelper::serialize(out);
}

void Object::deserialize(const InputStream &in)
{
    Relative::deserialize(in);
    in.readBool(_locked);
    in.readInt(_workflag);
    UserRecordHelper::deserialize(in);
}

} // namespace pxl
std::ostream &pxl::Object::print(int level, std::ostream &os, int pan) const
{
    printPan1st(os, pan);
    os << "pxl::Object with name: " << getName() << "\n";
    if (level > 0)
        printContent(level, os, pan);
    return os;
}

std::ostream &pxl::Object::printContent(int level, std::ostream &os, int pan) const
{
    os << "WorkFlag: " << _workflag << ", locked: " << _locked;
    os << std::endl;
    getUserRecords().print(level, os, pan);
    return os;
}
