//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_INPUTHANDLER_HH
#define PXL_IO_INPUTHANDLER_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <iostream>
#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/BasicContainer.hh"
#include "Pxl/Pxl/interface/pxl/core/ChunkReader.hh"
#include "Pxl/Pxl/interface/pxl/core/Event.hh"
#include "Pxl/Pxl/interface/pxl/core/Id.hh"
#include "Pxl/Pxl/interface/pxl/core/InformationChunk.hh"

namespace pxl
{
// io
/**
 This abstract class offers the basic functionality for reading the PXL physics event structure.
 Derived classes can handle concrete I/O operations.
 */
using namespace skipSpace;

class PXL_DLL_EXPORT InputHandler
{
  public:
    InputHandler() : _objectCount(0)
    {
    }

    virtual ~InputHandler()
    {
    }

    virtual ChunkReader &getChunkReader() = 0;

    /// Returns the number of file sections from the current ChunkReader.
    /// File sections are counted regardless of their actual content (InformationChunk, Event, etc).
    unsigned long getSectionCount()
    {
        return getChunkReader().getSectionCount();
    }

    /// Reads in the header of the next file section (e.g. event or information chunk)
    bool nextFileSection()
    {
        if (getChunkReader().next())
            return true;
        return false;
    }

    /// Reads in the next file section if the information condition is fulfilled. Else, false is returned.
    bool nextFileSectionIf(const std::string &info, skipMode doSkip = on)
    {
        if (getChunkReader().next(doSkip, ChunkReader::evaluate, info))
            return true;
        return false;
    }

    /// Use this method in case the file contains pxl::Events after a next-statement.
    /// A pxl::Event is passed to this method and filled with the current event.
    /// If the file section header has not been read or other objects occur,
    /// false is returned.
    bool readEvent(Event *event);

    /// Use this method in case the file contains pxl::Events after a next-statement.
    /// A pxl::Event is passed to this method and filled with the current event.
    /// If the file section header has not been read or other objects occur,
    /// false is returned.
    bool readEventIf(Event *event, const std::string &blockInfo, skipMode doSkip = on);

    /// Use this method in case the file contains pxl::BasicContainers after a next-statement.
    /// A pxl::BasicContainer is passed to this method and filled with the current container.
    /// If the file section header has not been read or other objects occur,
    /// false is returned.
    bool readBasicContainer(BasicContainer *basicContainer);

    /// Use this method in case the file contains pxl::BasicContainers after a next-statement.
    /// A pxl::BasicContainer is passed to this method and filled with the current container.
    /// If the file section header has not been read, the info condition is not fulfilled,
    /// or other objects occur, false is returned.
    bool readBasicContainerIf(BasicContainer *basicContainer, const std::string &blockInfo, skipMode doSkip = on);

    /// A pxl::InformationChunk is passed to this method and filled if the next block contains an information chunk.
    /// Else, false is returned.
    bool readInformationChunk(InformationChunk *chunk);

    /// A pxl::InformationChunk is passed to this method and filled if the info condition is fulfilled and there is an
    /// information chunk. Else, false is returned.
    bool readInformationChunkIf(InformationChunk *event, const std::string &blockInfo, skipMode doSkip = on);

    /// Skips one file section.
    bool skip()
    {
        return getChunkReader().skip();
    }

    /// Goes to the previous file section.
    bool previous()
    {
        return getChunkReader().previous();
    }

    /// With this method, n file sections can be skipped in forward or backward direction.
    /// The number of actually skipped file sections is returned (positive or negative).
    int skipFileSections(int n);

    /// seek to the desired file section (with 0 being the first file section)
    bool seekToFileSection(int index);

    /// Reads in the next block.
    bool readBlock()
    {
        return getChunkReader().nextBlock();
    }

    /// Reads in the next block if the info condition is fulfilled.
    bool readBlockIf(const std::string &blockInfo, skipMode doSkip = on);

    /// Explicitly reads an object of type objecttype.
    /// Caution: This method should only be used if the type of the following object is known by hard.
    /// Else use the readNextObject method. This method may be deprecated in the future.
    template <class objecttype> bool readObject(objecttype *obj);

    /// Seek to the desired object (with 0 being the first object).
    /// Current implementation inefficient due to missing file index.
    Serializable *seekToObject(size_t index);

    /// This method reads in the next object from the file, regardless of file section boundaries.
    /// In case there are no more objects to be read, a zero pointer is returned.
    /// Attention: This method returns an object which was created with new. The user takes
    /// deletion responsibility.
    Serializable *readNextObject();

    /// This method reads in the previous object from the file, regardless of file section boundaries.
    /// Attention: This method returns an object which was created with new. The user takes
    /// deletion responsibility.
    Serializable *readPreviousObject();

    /// This method fills the objects from the read-in block into the passed vector. The number of added objects is
    /// returned. Attention: The objects in the vector are created with new, the user takes deletion responsibility.
    int readObjects(std::vector<Serializable *> &objects);

    /// This method fills the objects from the read-in block into the passed pxl::BasicContainer. The number of added
    /// objects is returned. Deletion responsibility is taken by the BasicContainer.
    int readObjects(BasicContainer *container);

    /// This method fills the objects from the read-in block into the passed pxl::Event. The number of added objects is
    /// returned. Only derivatives of pxl::Relative are filled, other items are skipped. Deletion responsibility is
    /// taken by the Event.
    int readObjects(Event *event);

    /// Returns the number of read objects
    size_t objectCount() const
    {
        return _objectCount;
    }

    /// Resets InputHandler in case a new file/stream is opened.
    void reset()
    {
        _objectCount = 0;
        getChunkReader().reset();
    }

    /// Returns the size of the associated file.
    size_t getSize()
    {
        return getChunkReader().getSize();
    }

    /// Returns the current position in the associated file.
    size_t getPosition()
    {
        return getChunkReader().getPosition();
    }

  private:
    InputHandler(const InputHandler &original)
    {
    }

    InputHandler &operator=(const InputHandler &other)
    {
        return *this;
    }

    size_t _objectCount;
};

} // namespace pxl
// namespace pxl

#endif // PXL_IO_INPUTHANDLER_HH
