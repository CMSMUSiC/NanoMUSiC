//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/OutputHandler.hh"
#include "Pxl/Pxl/interface/pxl/core/Id.hh"
#include "Pxl/Pxl/interface/pxl/core/logging.hh"

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::OutputHandler"

namespace pxl
{

OutputHandler::OutputHandler(size_t maxSize, size_t maxNObjects)
    : _maxSize(maxSize), _newFileSection(true), _maxNObjects(maxNObjects), _nObjects(0)
{
}

OutputHandler::~OutputHandler()
{
}

/// Use this method to write an information string describing the new event. Otherwise, this method need not necessarily
/// be used.
bool OutputHandler::newFileSection(const std::string &info)
{
    if (!_newFileSection)
    {
        PXL_LOG_ERROR << "Finish the current event first.";
        return false;
    }
    getChunkWriter().newFileSection(info);
    _newFileSection = false;
    return true;
}

/// Use this method to write out a block to file. This method is not needed if you use the writeEvent-method.
bool OutputHandler::writeStream(const std::string &info)
{
    if (_newFileSection)
    {
        getChunkWriter().newFileSection("");
        _newFileSection = false;
    }
    getChunkWriter().newBlock();
    return getChunkWriter().write(info);
}

/// Use this method to write out a block to disk and finish the current event.
bool OutputHandler::writeFileSection(const std::string &info)
{
    if (_newFileSection)
    {
        getChunkWriter().newFileSection("");
        _newFileSection = false;
    }
    getChunkWriter().newBlock();
    getChunkWriter().write(info);
    _newFileSection = true;
    _nObjects = 0; // reset number of objects
    return getChunkWriter().endFileSection();
}

} // namespace pxl
