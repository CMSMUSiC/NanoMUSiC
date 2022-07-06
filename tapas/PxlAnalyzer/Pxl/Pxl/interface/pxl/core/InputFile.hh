//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_INPUTFILE_HH
#define PXL_IO_INPUTFILE_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <fstream>
#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/InputHandler.hh"
#include "Pxl/Pxl/interface/pxl/core/ChunkReader.hh"
#include "Pxl/Pxl/interface/pxl/core/File.hh"
#include "Pxl/Pxl/interface/pxl/core/logging.hh"

#define PXL_LOG_MODULE_NAME "pxl::InputFile"

namespace pxl
{

// io
/**
 This class offers an easy handling of the PXL I/O. Various methods to access
 the content of I/O files are offered.
 */
class InputFile: public InputHandler
{
public:

	InputFile() :
			InputHandler(), _stream(), _reader(_stream)
	{
	}

	InputFile(const std::string& filename) :
			InputHandler(), _stream(), _reader(_stream,
					ChunkReader::seekable)
	{
		this->open(filename);
	}

	virtual void open(const std::string& filename)
	{
		if (_stream.isOpen())
			_stream.close();
		_stream.clear();
		reset();
		if (!_stream.open(filename.c_str(), OpenRead))
		{
			PXL_LOG_ERROR << "Error opening file " << filename;
			throw std::runtime_error("Error opening file");
		}
		if (_stream.isBad())
		{
			PXL_LOG_ERROR << "Bad file " << filename;
			throw std::runtime_error("Bad file");
		}
		_stream.seek(0, SeekBegin);
		_reader.setStatus(ChunkReader::preHeader);
	}

	virtual void close()
	{
		if (_stream.isOpen())
			_stream.close();
		reset();
		_reader.setStatus(ChunkReader::preHeader);
	}

	virtual ~InputFile()
	{
		if (_stream.isOpen())
			_stream.close();
	}

	virtual ChunkReader& getChunkReader()
	{
		return _reader;
	}

	virtual bool good()
	{
		return _stream.isGood();
	}

	virtual bool eof()
	{
		return _stream.isEof();
	}

	virtual bool bad()
	{
		return _stream.isBad();
	}

private:
	InputFile(const InputFile& original) :
			_stream(), _reader(_stream, ChunkReader::seekable)
	{
	}

	InputFile& operator=(const InputFile& other)
	{
		return *this;
	}

	File _stream;
	ChunkReader _reader;

};

} //namespace pxl

#endif /*PXL_IO_INPUTFILE_HH*/
