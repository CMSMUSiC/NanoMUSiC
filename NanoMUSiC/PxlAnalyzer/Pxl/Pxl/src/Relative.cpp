//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <iostream>
#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/Relations.hpp"
#include "Pxl/Pxl/interface/pxl/core/Relative.hpp"
#include "Pxl/Pxl/interface/pxl/core/logging.hpp"

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::Relative"

namespace pxl
{

Relative::~Relative()
{
    // remove all relations:
    unlinkMothers();
    unlinkDaughters();
    unlinkFlat();
    if (_refWkPtrSpec)
    {
        _refWkPtrSpec->notifyDeleted();
    }

    // don't throw exception in destructor:
    if (_refObjectOwner)
    {
        PXL_LOG_ERROR << "Error in ~Relative(): Relative derivative must be deleted by ObjectOwner to guarantee safe "
                         "object handling!";
    }
}

void Relative::linkSoft(Relative *relative, const std::string &type)
{
    if (relative)
    {
        if (!_softRelations.has(relative, type))
        {
            _softRelations.set(relative, type);
            relative->linkSoft(this, type);
        }
    }
}

/// Remove a soft relation with name \p type to the Relative \p relative
void Relative::unlinkSoft(Relative *relative, const std::string &type)
{
    if (relative)
    {
        _softRelations.remove(relative, type);
        relative->_softRelations.remove(this, type);
    }
}

void Relative::linkMother(Relative *target)
{
    if (target->_refObjectOwner != this->_refObjectOwner)
    {
        throw std::runtime_error(
            "pxl::ObjectBase::linkDaughter(...): ERROR: mother and daughter have not the same object holder!");
    }

    this->_motherRelations.set(target);
    target->_daughterRelations.set(this);
}

void Relative::linkDaughter(Relative *target)
{
    if (target->_refObjectOwner != this->_refObjectOwner)
    {
        throw std::runtime_error(
            "pxl::ObjectBase::linkMother(...): ERROR: mother and daughter have not the same object holder!");
    }

    this->_daughterRelations.set(target);
    target->_motherRelations.set(this);
}

void Relative::linkFlat(Relative *target)
{
    if (target->_refObjectOwner != this->_refObjectOwner)
    {
        throw std::runtime_error(
            "pxl::ObjectBase::linkFlat(...): ERROR: potential relatives have not the same object holder!");
    }

    this->_flatRelations.set(target);
    target->_flatRelations.set(this);
}

void Relative::unlinkMother(Relative *target)
{
    this->_motherRelations.erase(target);
    target->_daughterRelations.erase(this);
}

void Relative::unlinkDaughter(Relative *target)
{
    this->_daughterRelations.erase(target);
    target->_motherRelations.erase(this);
}

void Relative::unlinkFlat(Relative *target)
{
    this->_flatRelations.erase(target);
    target->_flatRelations.erase(this);
}

void Relative::unlinkMothers()
{
    for (Relations::const_iterator iter = _motherRelations.begin(); iter != _motherRelations.end(); ++iter)
    {
        (*iter)->_daughterRelations.erase(this);
    }

    _motherRelations.clearContainer();
}

void Relative::unlinkDaughters()
{
    for (Relations::const_iterator iter = _daughterRelations.begin(); iter != _daughterRelations.end(); ++iter)
    {

        (*iter)->_motherRelations.erase(this);
    }

    _daughterRelations.clearContainer();
}

void Relative::unlinkFlat()
{
    for (Relations::const_iterator iter = _flatRelations.begin(); iter != _flatRelations.end(); ++iter)
    {

        (*iter)->_flatRelations.erase(this);
    }

    _flatRelations.clearContainer();
}

std::ostream &Relative::printDecayTree(int level, std::ostream &os, int pan) const
{
    int daughters = 0;

    print(level, os, pan);

    for (Relations::const_iterator iter = _daughterRelations.begin(); iter != _daughterRelations.end(); ++iter)
    {

        (*iter)->printDecayTree(level, os, pan + 1);
        daughters++;
    }

    if (daughters && pan > 1)
    {
        for (int p = 0; p < pan; p++)
        {
            os << "|  ";
        }
        os << "*" << std::endl;
    }

    return os;
}

std::ostream &Relative::print(int level, std::ostream &os, int pan) const
{
    return printPan1st(os, pan) << "pxl::ObjectBase with id: " << id() << std::endl;
}

std::ostream &Relative::printPan1st(std::ostream &os, int pan) const
{
    for (int p = 0; p < pan - 2; p++)
        os << "|  ";
    if (pan - 1 > 0)
        os << "+--";
    if (pan)
        os << "{ ";

    return os;
}

std::ostream &Relative::printPan(std::ostream &os, int pan) const
{
    for (int p = 0; p < pan - 1; p++)
        os << "|  ";
    if (pan)
        os << "| ";
    return os;
}

} // namespace pxl

std::ostream &operator<<(std::ostream &cxxx, const pxl::Relative &obj)
{
    return obj.print(0, cxxx, 0);
}
