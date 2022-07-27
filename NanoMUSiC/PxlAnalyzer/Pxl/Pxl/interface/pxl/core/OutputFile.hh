//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_OUTPUTFILE_HH
#define PXL_IO_OUTPUTFILE_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <fstream>

#include "Pxl/Pxl/interface/pxl/core/OutputHandler.hh"
#include "Pxl/Pxl/interface/pxl/core/ChunkWriter.hh"

namespace pxl
{

// io
/**
 This class allows the user an easy handling of the PXL output to file. Methods to write event headers,
 the PXL event class, and information chunks are offered by inheritance from the OutputHandler.
 */

class PXL_DLL_EXPORT OutputFile : public OutputHandler
{
public:

	OutputFile(const std::string& filename, size_t maxBlockSize = 1048576, size_t maxNObjects = 1000);
	virtual ~OutputFile();
	virtual void open(const std::string& filename);
	virtual void close();
	virtual ChunkWriter& getChunkWriter();
		
	void setCompressionMode(int compressionMode);
	
private:
	
	OutputFile(const OutputFile& original);

	OutputFile& operator= (const OutputFile& other);
	
	File _stream;
	ChunkWriter _writer;
};

}//namespace pxl

#endif /*PXL_IO_OUTPUTFILE_HH*/
