#include "LocalFileImpl.hpp"
#include "Pxl/Pxl/interface/pxl/core/FileFactory.hpp"
#include "Pxl/Pxl/interface/pxl/core/functions.hpp"

#include <stdio.h>

namespace pxl
{

LocalFileImpl::LocalFileImpl() : _file(0)
{
}

LocalFileImpl::LocalFileImpl(const std::string &filename, int32_t mode) : _file(0)
{
    open(filename, mode);
}

LocalFileImpl::~LocalFileImpl()
{
    close();
}

bool LocalFileImpl::open(const std::string &filename, int32_t mode)
{
    close();
    std::string open_mode;
    if (mode & OpenRead)
        open_mode.append("r");
    else if (mode & OpenWrite)
        open_mode.append("w");
    open_mode.append("b");

    std::string schema, path;
    splitSchema(filename, schema, path);
#if defined(_MSC_VER)
    errno_t err = fopen_s(&_file, filename.c_str(), open_mode.c_str());
    if (err != 0)
        _file = 0;
#elif defined(__APPLE__)
    _file = fopen(path.c_str(), open_mode.c_str());
#else
    _file = fopen64(path.c_str(), open_mode.c_str());
#endif

    _mode = mode;

    return isOpen();
}

void LocalFileImpl::close()
{
    if (_file == 0)
        return;
    fclose(_file);
    _file = 0;
}

bool LocalFileImpl::isEof()
{
    if (_file == 0)
        return true;

    return (feof(_file) != 0);
}

bool LocalFileImpl::isOpen()
{
    return (_file != 0);
}

bool LocalFileImpl::isBad()
{
    if (_file == 0)
        return true;

    return false;
}

void LocalFileImpl::clear()
{
}

bool LocalFileImpl::isGood()
{
    if (_file == 0)
        return false;

    if (feof(_file))
        return false;

    return true;
}

int64_t LocalFileImpl::tell()
{
    if (_file == 0)
        return 0;

#if defined(_MSC_VER)
    return _ftelli64(_file);
#elif defined(__APPLE__)
    return ftell(_file);
#else
    return ftello64(_file);
#endif
}

void LocalFileImpl::seek(int64_t pos, int32_t d)
{
    int whence;
    if (d == SeekBegin)
        whence = SEEK_SET;
    else if (d == SeekCurrent)
        whence = SEEK_CUR;
    else if (d == SeekEnd)
        whence = SEEK_END;
    else
        throw std::runtime_error("Uninitialized value in LocalFileImpl::seek. This never should happen!.");

#if defined(_MSC_VER)
    _fseeki64(_file, pos, SEEK_CUR);
#elif defined(__APPLE__)
    fseek(_file, pos, whence);
#else
    fseeko64(_file, pos, whence);
#endif
}

int32_t LocalFileImpl::peek()
{
    int32_t c;

    c = fgetc(_file);
    ungetc(c, _file);

    return c;
}

int64_t LocalFileImpl::read(char *s, size_t count)
{
    return fread(s, sizeof(char), count, _file);
}

int64_t LocalFileImpl::write(const char *s, size_t count)
{
    return fwrite(s, sizeof(char), count, _file);
}

void LocalFileImpl::ignore(int64_t count)
{
    seek(count, SeekCurrent);
}

void LocalFileImpl::destroy()
{
    delete this;
}

} // namespace pxl
