//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_GENERICINPUTHANDLER_HH
#define PXL_IO_GENERICINPUTHANDLER_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/ChunkReader.hpp"
#include "Pxl/Pxl/interface/pxl/core/InputHandler.hpp"

namespace pxl
{
// io
/**
 This class offers a generic handling of the PXL I/O. Various methods to access
 the PXL I/O content are offered.
 */
class GenericInputHandler : public InputHandler
{
  public:
    GenericInputHandler(ChunkReader &reader) : InputHandler(), _reader(&reader)
    {
    }

    virtual ~GenericInputHandler()
    {
    }

    virtual ChunkReader &getChunkReader()
    {
        if (!_reader)
            throw std::runtime_error("GenericInputHandler::getChunkReader(): ChunkReader pointer invalid.");
        return *_reader;
    }

    virtual void setChunkReader(ChunkReader *reader)
    {
        _reader = reader;
    }

  private:
    GenericInputHandler(const GenericInputHandler &original)
    {
    }

    GenericInputHandler &operator=(const GenericInputHandler &other)
    {
        return *this;
    }

    ChunkReader *_reader;
};

} // namespace pxl

#endif /*PXL_IO_GENERICINPUTHANDLER_HH*/
