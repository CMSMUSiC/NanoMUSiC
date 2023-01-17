#ifndef PXLdCache_hh
#define PXLdCache_hh

#include <iostream>
#include <string>

//#include "Tools/PXL/PXL.hpp"
#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Tools/dCache/idCacheStream.hpp"

namespace pxl
{
class dCacheInputFile : public InputHandler
{
  public:
    dCacheInputFile() : InputHandler(), stream(), reader(stream, ChunkReader::nonSeekable)
    {
    }
    dCacheInputFile(const char *filename) : InputHandler(), stream(filename), reader(stream, ChunkReader::nonSeekable)
    {
    }

    virtual void open(const std::string &filename, unsigned int timeout = 3600)
    {
        // reset and close everything that might be open
        close();
        stream.open(filename.c_str(), timeout);
        if (!stream.good())
        {
            if (stream.eof())
                std::cerr << "dCache file opened, but EOF! File empty? File: " << filename << std::endl;
            else
                throw dCache_error("Failed to open file: " + filename);
        }
    }
    virtual void close()
    {
        stream.close();
        reset();
    }

    virtual ChunkReader &getChunkReader()
    {
        return reader;
    }

  private:
    idCacheStream stream;
    ChunkReader reader;
};
} // namespace pxl

#endif // PXLdCache_hh
