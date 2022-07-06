//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/OutputFile.hh"

#include <stdexcept>

namespace pxl
{
OutputFile::OutputFile(const std::string& filename, size_t maxBlockSize,
		size_t maxNObjects) :
		OutputHandler(maxBlockSize, maxNObjects), _stream(filename, OpenWrite), _writer(
				_stream)
{
	if (_stream.isGood() == false)
		throw std::runtime_error(
				"OutputFile: " + filename + " could not be opened.");
}

OutputFile::~OutputFile()
{
	close();
}

void OutputFile::open(const std::string& filename)
{
	_stream.open(filename.c_str(), OpenWrite);
	if (_stream.isGood() == false)
		throw std::runtime_error(
				"OutputFile: " + filename + " could not be opened.");
}

void OutputFile::close()
{
	finish();
	_stream.close();
}

ChunkWriter& OutputFile::getChunkWriter()
{
	return _writer;
}

void OutputFile::setCompressionMode(int compressionMode)
{
	_writer.setCompressionMode(compressionMode);
}

OutputFile::OutputFile(const OutputFile& original) :
		_stream(), _writer(_stream)
{
}

OutputFile& OutputFile::operator=(const OutputFile& other)
{
	return *this;
}

}
