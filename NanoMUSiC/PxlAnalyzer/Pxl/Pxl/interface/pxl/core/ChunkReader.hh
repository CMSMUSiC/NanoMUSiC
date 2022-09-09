//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_CHUNK_READER_HH
#define PXL_IO_CHUNK_READER_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/File.hh"
#include "Pxl/Pxl/interface/pxl/core/Stream.hh"

#define iotl__iStreamer__lengthUnzipBuffer 65536

namespace pxl
{

namespace skipSpace
{

enum skipMode
{
    off = 0,
    on
};

} // namespace skipSpace

/// This class implemenents various methods for reading from PXL I/O and
/// bases on the event structure defined in the ChunkWriter class.
/// The entry point for the user is the class InputFile.

using namespace skipSpace;

class PXL_DLL_EXPORT ChunkReader
{
  public:
    /// The status flag can take one out four values, either before an
    /// event (pre-header), or before a block, then either within an information chunk,
    /// or an event, or something unknown
    enum statusFlag
    {
        preHeader = 0,
        evPreBlock,
        infoPreBlock,
        preBlock
    };

    /// The readMode indicates if anything is read, just events, or only information chunks
    enum readMode
    {
        all = 0,
        event,
        infoChunk
    };

    /// The infoMode can be passed as a flag to indicate that a string condition
    /// must be fulfilled to read a block, an event, or an information chunk
    enum infoMode
    {
        ignore = 0,
        evaluate
    };

    /// The fileMode flag says if the read-in istream is seekable, or not. It needs to be modified by
    /// implementing classes.
    enum fileMode
    {
        nonSeekable = 0,
        seekable
    };

    ChunkReader(FileImpl &stream, fileMode seekMode = seekable)
        : _stream(stream), _status(preHeader), _sectionCount(0), _seekMode(seekMode)
    {
        _inputBuffer = new unsigned char[iotl__iStreamer__lengthUnzipBuffer];
        _outputBuffer = new unsigned char[iotl__iStreamer__lengthUnzipBuffer];
    }

    ~ChunkReader()
    {
        delete[] _inputBuffer;
        delete[] _outputBuffer;
    }

    void reset()
    {
        _sectionCount = 0;
        _status = preHeader;
        _buffer.clear();
    }

    unsigned long getSectionCount()
    {
        return _sectionCount;
    }

    /// Reads in the next event header.
    bool readHeader(readMode mode, skipMode skip, infoMode checkInfo, const std::string &infoCondition);

    /// Reads in the next block.
    bool readBlock(skipMode skip, infoMode checkInfo, const std::string &infoCondition);

    /// Skips an event/information chunk.
    bool skip();

    /// Goes back one event or information chunk.
    bool previous();

    /// Reads in the header of the next event. False is returned if not successful.
    bool next(skipMode skip = on, infoMode checkInfo = ignore, const std::string &infoCondition = "")
    {
        return readHeader(all, skip, checkInfo, infoCondition);
    }

    /// Reads the next block and puts data into the input stream. False is returned if not successful.
    bool nextBlock(skipMode skip = on, infoMode checkInfo = ignore, const std::string &infoCondition = "")
    {
        return readBlock(skip, checkInfo, infoCondition);
    }

    /// Access to the data read in the individual blocks.
    inline const InputStream &getInputStream()
    {
        return _buffer;
    }

    bool isBlock()
    {
        return (_stream.peek() == 'B');
    }

    bool isEnd()
    {
        return (_stream.peek() == 'e');
    }

    /// Method used internally to get the status, indicating the position in the I/O file.
    inline statusFlag getStatus() const
    {
        return _status;
    }

    inline void setStatus(statusFlag flag)
    {
        _status = flag;
    }

    void endEvent()
    {
        if (_status != preHeader)
            while (nextBlock())
                ;
    }

    /// Returns the size of the associated file.
    size_t getSize() const;

    /// Returns the current position in the associated file.
    size_t getPosition() const
    {
        return _stream.tell();
    }

    bool eof() const
    {
        return _stream.peek() == EOF;
    }

  protected:
    /// Helper method to perform the unzipping.
    int unzipEventData(uint32_t nBytes);

    /// Reads in a char from file and returns this.
    inline char nextBlockId()
    {
        char identifier;
        _stream.read(&identifier, 1);
        return identifier;
    }

  private:
    ChunkReader(const ChunkReader &original) : _stream(original._stream)
    {
    }

    ChunkReader &operator=(const ChunkReader &other)
    {
        return *this;
    }

    FileImpl &_stream;
    BufferInput _buffer;
    /// Status flag. 0 at end of event, 1 at end of block.
    statusFlag _status;
    unsigned long _sectionCount;
    fileMode _seekMode;

    unsigned char *_inputBuffer;
    unsigned char *_outputBuffer;
};

} // namespace pxl

#endif // PXL_IO_CHUNK_READER_HH
