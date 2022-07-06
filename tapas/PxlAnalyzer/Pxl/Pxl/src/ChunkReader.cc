//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <stdexcept>
#include <sstream>
#include <zlib.h>

#include "Pxl/Pxl/interface/pxl/core/ChunkReader.hh"

namespace pxl
{

bool ChunkReader::skip()
{
	if (_stream.peek()==EOF)
		return false;

	//skip event header
	if (_status == preHeader)
	{
		++_sectionCount;
		_stream.ignore(1);
		// read info size
		int32_t infoSize = 0;
		_stream.read((char *)&infoSize, 4);
		_stream.ignore(infoSize);
	}

	//skip all blocks
	while (nextBlockId()=='B' && !_stream.isEof() )
	{
		// read info size
		int32_t infoSize = 0;
		_stream.read((char *)&infoSize, 4);
		_stream.ignore(infoSize);
		_stream.ignore(1);

		// read chunk size
		int32_t chunkSize = 0;
		_stream.read((char *)&chunkSize, 4);
		_stream.ignore(chunkSize);
	}

	_stream.ignore(4);
	_status = preHeader;

	return true;
}

bool ChunkReader::previous()
{
	if (_seekMode == nonSeekable)
		return false;

	if (_status != preHeader)
	{
		endEvent();
		previous();
	}
	std::streampos pos = _stream.tell();

	int32_t eventSize;
	pos -= 4;
	if (pos<0)
		return false;
	_stream.seek(pos);
	_stream.read((char*)&eventSize, 4);

	pos -= eventSize;
	if (pos<0)
		return false;
	_stream.seek(pos);
	_status = preHeader;

	--_sectionCount;

	return true;
}

/// Reads next block from file to stream. If mode is -1, the information condition string is evaluated,
/// i.e. the block is read only if the string equals the one in the input.
bool ChunkReader::readBlock(skipMode skip, infoMode checkInfo,
		const std::string& infoCondition) 
{
	//if event header not read, return
	if (_status == preHeader)
		return false;
	
	//return false if end of file
	if (_stream.peek()==EOF)
		return false;

	//check if beginning of block
	char id = nextBlockId();

	if (id!='B')
	{
		if (id=='e') //end of event
		{
			_status = preHeader;
			_stream.ignore(4);
			return false;
		}
		else
		{
			std::stringstream ss;
			ss << "pxl::ChunkReader::readBlock(): Unknown char identifier: " << id;
			throw std::runtime_error(ss.str());
		}
	}

	int32_t infoSize = 0;
	_stream.read((char *)&infoSize, 4);

	bool readStream = true;

	if (checkInfo == evaluate)
	{
		char* infoBuffer = new char[infoSize+1];
		infoBuffer[infoSize]=0;
		_stream.read(infoBuffer, infoSize);
		std::string info;
		info.assign(infoBuffer);
		//the mode is set to -2 if the info condition is not fulfilled.
		//rest of block must be skipped and false be returned.
		if (infoCondition!=info)
			readStream = false;
		delete infoBuffer;
	}
	else
		_stream.ignore(infoSize);

	char compressionMode;
	_stream.read(&compressionMode, 1);

	// read chunk size
	uint32_t chunkSize = 0;
	_stream.read((char *)&chunkSize, 4);
	
	if (_stream.isBad() || _stream.isEof())
		return false;

	if (readStream == false)
	{
		_stream.ignore(chunkSize);
		if (skip == on)
			return readBlock(skip, checkInfo, infoCondition);
		else
			return false;
	}
	else
	{
		// read chunk into buffer
		if (compressionMode==' ')
		{
			//_buffer.destroy();
			_buffer.clear();
			_buffer.buffer.resize(chunkSize);
			_stream.read(&_buffer.buffer[0], chunkSize);
			if (_stream.isBad() || _stream.isEof() )
				return false;
		}
		else if (compressionMode=='Z')
		{
			//_buffer.destroy();
			_buffer.clear();
			unzipEventData(chunkSize);
		}
		else
		{
			throw std::runtime_error("pxl::ChunkReader::readBlock(): Invalid compression mode.");
		}
	}
	return true;
}

bool ChunkReader::readHeader(readMode mode, skipMode doSkip,
		infoMode checkInfo, const std::string& infoCondition)
{
	// if position is not before the header, end the previous event
	endEvent();
	
	++_sectionCount;
	
	if (_stream.peek()==EOF || _stream.isBad() )
		return false;

	_status = preBlock;
	// Check for nextId? char nextId = 
	nextBlockId();

	// get size of info string
	int32_t infoSize = 0;
	_stream.read((char *)&infoSize, 4);

	//if info string is to be checked
	if (checkInfo==evaluate)
	{
		char* infoBuffer = new char[infoSize+1];
		infoBuffer[infoSize]=0;
		_stream.read(infoBuffer, infoSize);
		std::string info;
		info.assign(infoBuffer);
		delete infoBuffer;
		if (infoCondition!=info)
		{
			if (doSkip == on)
				return readHeader(mode, doSkip, checkInfo, infoCondition);
			else
				return false;
		}
	}
	else
		_stream.ignore(infoSize);

	if (_stream.isEof() || _stream.isBad() )
		return false;

	return true;
}

int ChunkReader::unzipEventData(uint32_t nBytes) 
{
	size_t buffer_size_step = nBytes * 3;
	
	uint32_t ret, length = 0;
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;

	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return 0;

	// decompress until deflate stream ends or end of file
	do
	{
		int size = nBytes;
		if (size > iotl__iStreamer__lengthUnzipBuffer)
		{
			size = iotl__iStreamer__lengthUnzipBuffer;
		}

		strm.avail_in = _stream.read((char*)_inputBuffer, size);
		if (_stream.isBad())
		{
			inflateEnd(&strm);
			return 0;
		}

		nBytes -= strm.avail_in;
		
		if (_stream.isEof())
			nBytes = 0;

		if (strm.avail_in == 0)
			break;

		strm.next_in = _inputBuffer;

		// run inflate() on input until output buffer not full
		do {
			if ((_buffer.buffer.size() - length) < buffer_size_step)
				_buffer.buffer.resize(_buffer.buffer.size() + buffer_size_step);

			strm.avail_out = _buffer.buffer.size() - length;
			strm.next_out = (Bytef *)(&_buffer.buffer[length]);

			ret = inflate(&strm, Z_NO_FLUSH);
			switch (ret)
			{
			case Z_STREAM_ERROR:
				throw std::runtime_error("pxl::ChunkReader::unzipEventData(): Internal inflate stream error.");
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR; // fall through
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				inflateEnd(&strm);
				return 0;
			default:
				break;
			}

			size_t have = _buffer.buffer.size() - length  - strm.avail_out;
			length += have;
		} while (strm.avail_out == 0);
	} while (nBytes > 0); // done when inflate() says it's done

	inflateEnd(&strm);
	_buffer.buffer.resize(length);

	return length;
}


/// Returns the size of the associated file.
size_t ChunkReader::getSize() const
{
	std::streampos pos = _stream.tell();
	_stream.seek (0, SeekEnd);

	size_t length = _stream.tell();
	_stream.seek (pos);

	return length;
}

}

