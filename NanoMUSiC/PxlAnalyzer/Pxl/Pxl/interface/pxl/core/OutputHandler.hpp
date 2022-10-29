//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_OUTPUTHANDLER_HH
#define PXL_IO_OUTPUTHANDLER_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include <iostream>

#include "Pxl/Pxl/interface/pxl/core/BasicContainer.hpp"
#include "Pxl/Pxl/interface/pxl/core/ChunkWriter.hpp"
#include "Pxl/Pxl/interface/pxl/core/Event.hpp"
#include "Pxl/Pxl/interface/pxl/core/InformationChunk.hpp"

namespace pxl
{

// io
/**
 This abstract class allows the user an easy handling of the PXL general output. Methods to write event headers,
 the PXL event class, and information chunks are offered.
 */
class PXL_DLL_EXPORT OutputHandler
{
  public:
    /// Creates an OutputHandler with a maximum size after which a block is written.
    /// Set the maximum size to 0 (or smaller) to not let this happen automatically.
    OutputHandler(size_t maxSize = 1048576, size_t maxNObjects = 1000);

    virtual ~OutputHandler();

    virtual ChunkWriter &getChunkWriter() = 0;

    /// Queues the passed object for later writing to the output file.
    void streamObject(const Serializable *obj)
    {
        obj->serialize(getChunkWriter().getOutputStream());
        _nObjects++;
        if ((_maxSize > 0 && getOutputStream().buffer.size() > _maxSize) ||
            (_maxNObjects > 0 && _nObjects > _maxNObjects))
            writeFileSection();
    }

    /// Streams the passed pxl::Event to the output file.
    /// A file section is finished if the given maximum section size is reached.
    void writeEvent(const Event *event)
    {
        event->serialize(getChunkWriter().getOutputStream());
        _nObjects++;
        //		if ((_maxSize>0 && getOutputStream().getSize() > _maxSize) || (_maxNObjects>0 &&_nObjects >
        //_maxNObjects)) 			writeStream();
        // Restore backwards compatible behaviour for VISPA. Will be changed if necessary
        // functionality in PXL
        writeFileSection();
    }

    /// Writes the passed pxl::InformationChunk to the output file.
    void writeInformationChunk(const InformationChunk *infoChunk)
    {
        infoChunk->serialize(getChunkWriter().getOutputStream());
        _nObjects++;
        //		if ((_maxSize>0 && getOutputStream().getSize() > _maxSize) || (_maxNObjects>0 &&_nObjects >
        //_maxNObjects)) 			writeStream();
        // Restore backwards compatible behaviour for VISPA. Will be changed if necessary
        // functionality in PXL
        writeFileSection();
    }

    /// Writes the passed pxl::BasicContainer to the output file.
    void writeBasicContainer(const BasicContainer *basicContainer)
    {
        basicContainer->serialize(getChunkWriter().getOutputStream());
        _nObjects++;
        // 		if ((_maxSize>0 && getOutputStream().getSize() > _maxSize) || (_maxNObjects>0 &&_nObjects >
        // _maxNObjects)) 			writeStream(); Restore backwards compatible behaviour for VISPA. Will be changed if
        // necessary functionality in PXL
        writeFileSection();
    }

    /// Returns the associated OutputStream
    const BufferOutput &getOutputStream()
    {
        return getChunkWriter().getOutputStream();
    }

    /// Use this method to write an information string describing the new file section (same as event). Otherwise, this
    /// method need not necessarily be used.
    bool newFileSection(const std::string &info);

    /// Use this method to write out a block to file. This method is not needed if you use the writeFileSection-method.
    bool writeStream(const std::string &info = "");

    /// Use this method to write out a block to disk and finish the current file section.
    bool writeFileSection(const std::string &info = "");

    /// Finishes the section in case the stream not written to file
    void finish()
    {
        if (getOutputStream().buffer.size() > 0)
            writeFileSection();
    }

    void setMaxNObjects(size_t maxNObjects)
    {
        _maxNObjects = maxNObjects;
    }

    size_t getMaxNObjects() const
    {
        return _maxNObjects;
    }

    void setMaxSize(size_t maxSize)
    {
        _maxSize = maxSize;
    }

    size_t getMaxSize() const
    {
        return _maxSize;
    }

  private:
    OutputHandler(const OutputHandler &original)
    {
    }

    OutputHandler &operator=(const OutputHandler &other)
    {
        return *this;
    }

    size_t _maxSize;
    bool _newFileSection;
    size_t _maxNObjects;
    size_t _nObjects;
};

} // namespace pxl
// namespace pxl

#endif /*PXL_IO_OUTPUTHANDLER_HH*/
