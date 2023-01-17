//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/WkPtrBase.hpp"
#include "Pxl/Pxl/interface/pxl/core/Relative.hpp"

namespace pxl
{

void WkPtrBase::notifyDeleted()
{
    _objectRef = 0;
    if (_notifyChainOut)
        _notifyChainOut->notifyDeleted();
    _notifyChainIn = 0;
    _notifyChainOut = 0;
}

void WkPtrBase::connect(Relative *pointer)
{
    // disconnect:
    if (_objectRef)
    {
        if (_objectRef->_refWkPtrSpec == this)
            _objectRef->_refWkPtrSpec = _notifyChainOut;
        if (_notifyChainIn && _notifyChainOut)
        {
            _notifyChainIn->_notifyChainOut = _notifyChainOut;
            _notifyChainOut->_notifyChainIn = _notifyChainIn;
        }
        else
        {
            if (_notifyChainIn)
                _notifyChainIn->_notifyChainOut = 0;
            if (_notifyChainOut)
                _notifyChainOut->_notifyChainIn = 0;
        }
    }
    _notifyChainOut = 0;
    _notifyChainIn = 0;

    // connect
    if (pointer)
    {
        _notifyChainIn = 0;
        _notifyChainOut = pointer->_refWkPtrSpec;
        if (_notifyChainOut)
            _notifyChainOut->_notifyChainIn = this;
        pointer->_refWkPtrSpec = this;
    }

    _objectRef = pointer;
}

} // namespace pxl
