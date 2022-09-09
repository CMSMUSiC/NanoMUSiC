//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_STREAM_HH
#define PXL_IO_STREAM_HH

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <string.h>

#include "Pxl/Pxl/interface/pxl/core/macros.hh"
#include <stdint.h>

namespace pxl
{
template <typename T> void swap_endianess(T &t)
{
#ifdef PXL_BIG_ENDIAN
    T swapped;
    unsigned char *in = (unsigned char *)&t;
    unsigned char *out = (unsigned char *)&swapped;
    for (size_t i = 0; i < sizeof(T); i++)
        out[i] = in[sizeof(T) - 1 - i];
    t = swapped;
#endif
}

// io
/**
 This abstract class serves the internal PXL I/O scheme by defining how basic C++ types
 are written to an output buffer.
 */
class PXL_DLL_EXPORT OutputStream
{

  public:
    virtual ~OutputStream()
    {
    }

    virtual void write(const void *data, size_t size) const = 0;

    // helpers
    void writeChar(char c) const
    {
        write(&c, 1);
    }

    void writeUnsignedChar(unsigned char c) const
    {
        write(&c, 1);
    }

    void writeString(const std::string &s) const
    {
        unsigned int len = static_cast<unsigned int>(s.size());
        writeUnsignedInt(len);
        write(s.data(), len);
    }

    void writeFloat(float f) const
    {
        swap_endianess(f);
        write(&f, sizeof(f));
    }

    void writeDouble(double d) const
    {
        swap_endianess(d);
        write(&d, sizeof(d));
    }
    void write(int8_t i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    void write(uint8_t i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    void write(int16_t i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    void write(uint16_t i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    void write(int32_t i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    void write(uint32_t i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    void write(int64_t i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    void write(uint64_t i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    /// ToDo: Remove legacy write / read functions
    void writeInt(int i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    void writeUnsignedInt(unsigned int i) const
    {
        swap_endianess(i);
        write(&i, sizeof(i));
    }

    void writeLong(long l) const
    {
        writeInt(static_cast<int>(l));
    }

    void writeUnsignedLong(unsigned long l) const
    {
        writeUnsignedInt(static_cast<unsigned int>(l));
    }

    void writeShort(short s) const
    {
        swap_endianess(s);
        write(&s, sizeof(s));
    }

    void writeUnsignedShort(unsigned short s) const
    {
        swap_endianess(s);
        write(&s, sizeof(s));
    }

    void writeBool(bool b) const
    {
        char c = b ? 1 : 0;
        write(&c, 1);
    }
};

/**
 This class serves the internal PXL I/O scheme by implementing how basic C++ types
 are written to an output buffer.
 */

class PXL_DLL_EXPORT BufferOutput : public OutputStream
{
  public:
    mutable std::vector<char> buffer;

    BufferOutput()
    {
        // optimal size to be determined
        buffer.reserve(1048576);
    }

    BufferOutput(size_t bufferSize)
    {
        buffer.reserve(bufferSize);
    }

    void clear()
    {
        buffer.clear();
    }

    void write(const void *data, size_t size) const
    {
        size_t pos = buffer.size();
        buffer.resize(pos + size);
        memcpy(&buffer[pos], data, size);
    }
};

/**
 This abstract class serves the internal PXL I/O scheme by defining how basic C++ types
 are read from an input buffer.
 */
class PXL_DLL_EXPORT InputStream
{

  public:
    virtual ~InputStream()
    {
    }

    virtual void read(void *data, size_t size) const = 0;
    virtual bool good() const = 0;

    void read(char &i) const
    {
        read(&i, sizeof(i));
    }

    void read(unsigned char &i) const
    {
        read(&i, sizeof(i));
    }

    void read(int16_t &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void read(uint16_t &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void read(int32_t &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void read(uint32_t &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void read(int64_t &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void read(uint64_t &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void readChar(char &c) const
    {
        read(&c, sizeof(c));
    }

    void readUnsignedChar(unsigned char &c) const
    {
        read(&c, sizeof(c));
    }

    void readInt(int &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void readUnsignedInt(unsigned int &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void readLong(long &l) const
    {
        int i;
        read(&i, sizeof(i));
        swap_endianess(i);
        l = i;
    }

    void readUnsignedLong(unsigned long &l) const
    {
        unsigned int i;
        read(&i, sizeof(i));
        swap_endianess(i);
        l = i;
    }

    void readShort(short &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void readUnsignedShort(unsigned short &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void readBool(bool &b) const
    {
        char c;
        read(&c, sizeof(c));
        b = (c != 0);
    }

    void readString(std::string &s) const
    {
        unsigned int size = 0;
        readUnsignedInt(size);
        s.clear();
        s.reserve(size);
        std::string::value_type buffer[1024];
        while (size)
        {
            size_t count = std::min(1024u, size);
            read(buffer, count);
            s.append(buffer, count);
            size -= count;
        }
    }

    void readFloat(float &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }

    void readDouble(double &i) const
    {
        read(&i, sizeof(i));
        swap_endianess(i);
    }
};

// iotl
/**
 This class serves the internal PXL I/O scheme by implementing how basic C++ types
 are read from an input buffer.
 */
class PXL_DLL_EXPORT BufferInput : public InputStream
{
    mutable size_t _readPosition;

  public:
    std::vector<char> buffer;

    BufferInput() : _readPosition(0)
    {
    }

    void clear()
    {
        buffer.clear();
        _readPosition = 0;
    }

    bool good() const
    {
        size_t av = available();
        return (av > 0);
    }

    size_t available() const
    {
        return (buffer.size() - _readPosition);
    }

    void read(void *data, size_t size) const
    {
        if (available() < size)
            throw std::runtime_error("buffer underrun!");

        memcpy(data, &buffer[_readPosition], size);
        _readPosition += size;
    }
};

} // namespace pxl

#endif /*PXL_IO_STREAM_HH*/
