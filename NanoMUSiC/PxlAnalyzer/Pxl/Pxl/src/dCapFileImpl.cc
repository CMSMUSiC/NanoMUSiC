#ifdef PXL_ENABLE_DCAP

#include "dCapFileImpl.hh"

#include <dcap.h>

#include <stdio.h>

namespace pxl
{

	dCapFileImpl::dCapFileImpl() : _file(0), _mode(0)
	{
	}

	dCapFileImpl::dCapFileImpl(const std::string &filename, int32_t mode)
	{
		open(filename, mode);
	}

	dCapFileImpl::~dCapFileImpl()
	{
		close();
	}

	bool dCapFileImpl::open(const std::string &filename, int32_t mode)
	{
		close();
		if (mode & OpenRead)
			_file = dc_fopen64(filename.c_str(), "rb");
		else if (mode & OpenWrite)
			_file = dc_fopen64(filename.c_str(), "wb");

		_mode = mode;

		return isOpen();
	}

	void dCapFileImpl::close()
	{
		if (_file == 0)
			return;
		dc_fclose(_file);
		_file = 0;
	}

	bool dCapFileImpl::isEof()
	{
		if (_file == 0)
			return true;

		return dc_feof(_file);
	}

	bool dCapFileImpl::isOpen()
	{
		return (_file);
	}

	bool dCapFileImpl::isBad()
	{
		if (_file == 0)
			return true;

		if (dc_feof(_file))
			return true;

		return false;
	}

	void dCapFileImpl::clear()
	{
	}

	bool dCapFileImpl::isGood()
	{
		if (_file == 0)
			return false;

		if (dc_feof(_file))
			return false;

		return true;
	}

	int64_t dCapFileImpl::tell()
	{
		if (_file == 0)
			return 0;

		return dc_ftell(_file);
	}

	void dCapFileImpl::seek(int64_t pos, int32_t d)
	{
		int whence = -99;
		if (d == SeekBegin)
			whence = SEEK_SET;
		else if (d == SeekCurrent)
			whence = SEEK_CUR;
		else if (d == SeekEnd)
			whence = SEEK_END;

		dc_fseeko64(_file, pos, whence);
	}

	int32_t dCapFileImpl::peek()
	{
		int32_t c;

		c = dc_fgetc(_file);
		dc_fseeko64(_file, -1, SEEK_CUR);

		return c;
	}

	int64_t dCapFileImpl::read(char *s, size_t count)
	{
		return dc_fread(s, sizeof(char), count, _file);
	}

	int64_t dCapFileImpl::write(const char *s, size_t count)
	{
		return dc_fwrite(s, sizeof(char), count, _file);
	}

	void dCapFileImpl::ignore(int64_t count)
	{
		dc_fseeko64(_file, count, SEEK_CUR);
	}

	void dCapFileImpl::destroy()
	{
		delete this;
	}

}

#endif
