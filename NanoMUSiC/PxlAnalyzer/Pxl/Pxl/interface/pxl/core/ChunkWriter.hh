//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_CHUNKWRITER_HH
#define PXL_IO_CHUNKWRITER_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <fstream>
#include <stdexcept>
#include <stdint.h>
#include <string>

#include "Pxl/Pxl/interface/pxl/core/File.hh"
#include "Pxl/Pxl/interface/pxl/core/Stream.hh"

namespace pxl
{

// io
/**
 This class implemenents methods for writing to PXL I/O files.
 PXL I/O allows the storage of complete physics events and information chunks.
 Each event or information chunk makes up a section in the output file.
 Each section consists of a header, and a number of blocks which can be compressed individually.
 The compression is incorporated via zlib.
 The entry point for the standard user is the class OutputFile.
 */
class PXL_DLL_EXPORT ChunkWriter
{
  public:
    ChunkWriter(FileImpl &stream, char compressionMode = '1')
        : _stream(stream), _nBytes(0), _compressionMode(compressionMode)
    {
    }

    ~ChunkWriter()
    {
    }

    /// Writes the current block to the output file stream.
    bool write()
    {
        return write("");
    }

    /// Writes a new file section, indicating the content by the section marker char, and the passed information string.
    bool newFileSection(const std::string &info, char cSectionMarker = 'E');

    /// Writes a new block marker.
    inline bool newBlock()
    {
        // begin block marker:
        return writeFlag('B');
    }

    /// Writes an end-of-file-section marker and the number of bytes stored in the event.
    bool endFileSection();

    /// Writes the current stream. An information string, and a compression mode char (allowed values between '0' and
    /// '9') are passed.
    bool write(std::string info);

    const BufferOutput &getOutputStream()
    {
        return _buffer;
    }

    void setCompressionMode(char compressionMode)
    {
        _compressionMode = compressionMode;
    }

    void setCompressionMode(int compressionMode)
    {
        if (0 <= compressionMode && compressionMode <= 9)
            _compressionMode = '0' + compressionMode;
        else
            throw std::runtime_error("Invalid compression mode");
    }

  protected:
    /// Write char flag.
    bool writeFlag(char cEvtMarker);

  private:
    ChunkWriter(const ChunkWriter &original) : _stream(original._stream)
    {
    }

    ChunkWriter &operator=(const ChunkWriter &other)
    {
        return *this;
    }

    FileImpl &_stream;
    BufferOutput _buffer;
    int32_t _nBytes;
    char _compressionMode;
};
} // namespace pxl
#endif /*PXL_IO_CHUNKWRITER_HH*/
