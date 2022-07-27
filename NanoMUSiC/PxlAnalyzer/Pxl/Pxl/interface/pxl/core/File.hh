//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_FILE_HH_
#define PXL_FILE_HH_

#include "Pxl/Pxl/interface/pxl/core/macros.hh"
#include "Pxl/Pxl/interface/pxl/core/functions.hh"

#include <string>
#include <stdint.h>
#include <stdexcept>

namespace pxl
{

enum SeekDirectionEnum
{
	SeekBegin, SeekCurrent, SeekEnd
};

enum OpenModeEnum
{
	OpenRead = 1, OpenWrite = 2, OpenOverwrite = 4
};

class PXL_DLL_EXPORT FileImpl
{
public:

	virtual ~FileImpl()
	{
	}

	virtual bool open(const std::string &filename, int32_t mode) = 0;
	virtual void close() = 0;
	virtual bool isEof() = 0;
	virtual bool isBad() = 0;
	virtual bool isOpen() = 0;
	virtual void clear() = 0;
	virtual bool isGood() = 0;
	virtual int64_t tell() = 0;
	virtual void seek(int64_t pos, int32_t d = SeekBegin) = 0;
	virtual int32_t peek() = 0;
	virtual int64_t read(char *s, size_t count) = 0;
	virtual int64_t write(const char *s, size_t count) = 0;
	virtual void ignore(int64_t count) = 0;

	virtual void destroy() = 0;
};

class PXL_DLL_EXPORT File: public FileImpl
{
	FileImpl *impl;
public:

	File();

	File(const std::string &filename, int32_t mode = OpenRead);
	~File();

	virtual bool open(const std::string &filename, int32_t mode = OpenRead);

	virtual void close();

	virtual bool isEof();

	virtual bool isBad();
	virtual bool isOpen();
	virtual void clear();
	virtual bool isGood();
	virtual int64_t tell();
	virtual void seek(int64_t pos, int32_t d = SeekBegin);
	virtual int32_t peek();
	virtual int64_t read(char *s, size_t count);
	virtual int64_t write(const char *s, size_t count);
	virtual void ignore(int64_t count);
	virtual void destroy();
};

}

#endif /* PXL_FILE_HH_ */
