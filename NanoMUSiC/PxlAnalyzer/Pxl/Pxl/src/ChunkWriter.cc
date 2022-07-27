//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <stdexcept>
#include <zlib.h>

#include "Pxl/Pxl/interface/pxl/core/ChunkWriter.hh"

namespace pxl
{

bool ChunkWriter::newFileSection(const std::string& info, char cSectionMarker)
{
	_stream.write(&cSectionMarker, 1);
	if (_stream.isBad())
	{
		throw std::runtime_error("Cannot write to file");
	}
	_nBytes+=1;

	// info block:
	const char* cInfo = info.c_str();
	int32_t lengthInfo = info.length();
	_stream.write((char *) &lengthInfo, 4);
	_nBytes+=4;
	_stream.write(cInfo, lengthInfo);
	_nBytes+=lengthInfo;
	if (_stream.isBad())
	{
		throw std::runtime_error("Cannot write to file");
	}
	return true;
}

bool ChunkWriter::endFileSection()
{
	// end event marker:
	writeFlag('e');
	_stream.write((char* ) &_nBytes, 4);
	_nBytes=0;
	if (_stream.isBad())
	{
		throw std::runtime_error("Cannot write to file");
	}
	return true;
}

/// Write char flag.
bool ChunkWriter::writeFlag(char cEvtMarker)
{
	_stream.write(&cEvtMarker, 1);
	_nBytes+=1;
	if (_stream.isBad())
	{
		throw std::runtime_error("Cannot write to file");
	}
	return true;
}
	

bool ChunkWriter::write(std::string info) 
{
	// write out block information
	const char* cInfo = info.c_str();
	int32_t lengthInfo = info.length();
	_stream.write((char *) &lengthInfo, 4);
	_nBytes+=4;
	_stream.write(cInfo, lengthInfo);
	_nBytes+=lengthInfo;

	// write out compression mode
	char compressed = 'Z';
	if (_compressionMode == ' ') compressed = ' ';
	_stream.write((char *) &compressed, 1);
	_nBytes+=1;

	// zip block:
	const char* cBuffer = &_buffer.buffer[0];
	int32_t lengthBuffer = _buffer.buffer.size();

	const char* cZip = cBuffer;
	int32_t lengthZip = lengthBuffer;

	char* cZipSpace = 0;
	unsigned long lengthZipSpace = 0;

	if (_compressionMode == ' ')
	{
		// no compression requires no action...
	}
	else if (_compressionMode >= '0' && _compressionMode <= '9')
	{
		// data compression a la Gero, i.e. compression level = 6:
		lengthZipSpace = long(double(lengthBuffer) * 1.05 + 16);
		cZipSpace = new char[lengthZipSpace];
		
		int status = compress2((Bytef*)cZipSpace, (uLongf*)&lengthZipSpace,
				(const Bytef*)cBuffer, lengthBuffer, _compressionMode - '0');
		switch (status)
		{
		case Z_MEM_ERROR:
			throw std::runtime_error("pxl::ChunkWriter::write(): zlib: not enough memory");
			break;
		case Z_BUF_ERROR:
			throw std::runtime_error("pxl::ChunkWriter::write(): zlib: buffer too small");
			break;
		case Z_STREAM_ERROR:
			throw std::runtime_error("pxl::ChunkWriter::write(): zlib: level parameter invalid");
			break;
		default:
			break;
		}

		cZip = cZipSpace;
		lengthZip = lengthZipSpace;
	}
	else
		throw std::runtime_error("pxl::FileChunkWriter::write(): Invalid compression mode.");

	_stream.write((char *) &lengthZip, 4);
	_nBytes+=4;
	_stream.write(cZip, lengthZip);
	_nBytes+=lengthZip;

	if (cZipSpace)
		delete[] cZipSpace;

	_buffer.clear();

	if (_stream.isBad())
	{
		throw std::runtime_error("Cannot write to file");
	}
	return true;
}

} //namespace pxl
