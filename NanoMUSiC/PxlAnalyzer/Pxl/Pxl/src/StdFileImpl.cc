#include "StdFileImpl.hh"
#include "Pxl/Pxl/interface/pxl/core/FileFactory.hh"

#include <stdio.h>

namespace pxl
{

StdFileImpl::StdFileImpl() : _mode(0)
{
}

StdFileImpl::StdFileImpl(const std::string &filename, int32_t mode) : _mode(0)
{
    open(filename, mode);
}

StdFileImpl::~StdFileImpl()
{
    close();
}

bool StdFileImpl::open(const std::string &filename, int32_t mode)
{
    _mode = mode;

    return isOpen();
}

void StdFileImpl::close()
{
}

bool StdFileImpl::isEof()
{
    if (_mode == OpenRead)
        return std::cin.eof();
    else
        return std::cout.eof();
}

bool StdFileImpl::isOpen()
{
    return true;
}

bool StdFileImpl::isBad()
{
    if (_mode == OpenRead)
        return std::cin.bad();
    else
        return std::cout.bad();
}

void StdFileImpl::clear()
{
    if (_mode == OpenRead)
        std::cin.clear();
    else
        std::cout.clear();
}

bool StdFileImpl::isGood()
{
    if (_mode == OpenRead)
        return std::cin.good();
    else
        return std::cout.good();
}

int64_t StdFileImpl::tell()
{
    if (_mode == OpenRead)
        return std::cin.tellg();
    else
        return std::cout.tellp();
}

void StdFileImpl::seek(int64_t pos, int32_t d)
{
    std::ios::seekdir sd;
    if (d == SeekBegin)
        sd = std::ios::beg;
    else if (d == SeekCurrent)
        sd = std::ios::cur;
    else if (d == SeekEnd)
        sd = std::ios::end;
    else
        throw std::runtime_error("Uninitialized value in LocalFileImpl::seek.");

    if (_mode == OpenRead)
    {
        std::cin.seekg(pos, sd);
        std::cin.clear();
    }
    else
    {
        std::cout.seekp(pos, sd);
        std::cout.clear();
    }
}

int32_t StdFileImpl::peek()
{
    int32_t c;

    c = std::cin.peek();

    return c;
}

int64_t StdFileImpl::read(char *s, size_t count)
{
    std::cin.read(s, count);
    return std::cin.gcount();
}

int64_t StdFileImpl::write(const char *s, size_t count)
{
    std::cout.write(s, count);
    return std::cout.good() ? count : 0;
}

void StdFileImpl::ignore(int64_t count)
{
    if (_mode == OpenRead)
        std::cin.ignore(count);
    else
        throw std::runtime_error("stdout cannot ignore.");
}

void StdFileImpl::destroy()
{
    delete this;
}

} // namespace pxl
