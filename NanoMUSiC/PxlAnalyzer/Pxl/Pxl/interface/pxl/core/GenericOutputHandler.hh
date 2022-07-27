//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_GENERICOUTPUTHANDLER_HH
#define PXL_IO_GENERICOUTPUTHANDLER_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/OutputHandler.hh"
#include "Pxl/Pxl/interface/pxl/core/ChunkWriter.hh"

namespace pxl
{

// io
/**
 This class allows the user an easy handling of the PXL output using any ChunkWriter. Methods to write event headers,
 the PXL event class, and information chunks are offered by inheritance from the OutputHandler.
 */

class GenericOutputHandler : public OutputHandler
{
public:
	GenericOutputHandler(ChunkWriter& writer) :
		OutputHandler(), _writer(&writer)
	{
	}

	virtual ~GenericOutputHandler()
	{
	}

	virtual ChunkWriter& getChunkWriter() 
	{
		if (!_writer)
			throw std::runtime_error("GenericOutputHandler::getChunkWriter(): ChunkWriter pointer invalid.");
		return *_writer;
	}

	virtual void setChunkWriter(ChunkWriter* writer)
	{
		_writer=writer;
	}

private:
	GenericOutputHandler(const GenericOutputHandler& original)
	{
	}
	
	GenericOutputHandler& operator= (const GenericOutputHandler& other)
	{
		return *this;
	}
		
		
	ChunkWriter* _writer;
};

}//namespace pxl

#endif /*PXL_IO_GENERICOUTPUTHANDLER_HH*/
