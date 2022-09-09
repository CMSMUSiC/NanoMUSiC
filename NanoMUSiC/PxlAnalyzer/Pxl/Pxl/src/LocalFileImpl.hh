#ifndef PXL_LOCAL_FILE_IMPL_HH_
#define PXL_LOCAL_FILE_IMPL_HH_

#include "Pxl/Pxl/interface/pxl/core/File.hh"
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

namespace pxl
{

class PXL_DLL_EXPORT LocalFileImpl : public FileImpl
{
    FILE *_file;
    int32_t _mode;

  public:
    LocalFileImpl();

    LocalFileImpl(const std::string &filename, int32_t mode);

    ~LocalFileImpl();

    virtual bool open(const std::string &filename, int32_t mode);
    virtual void close();
    virtual bool isEof();
    virtual bool isOpen();
    virtual bool isBad();
    virtual void clear();
    virtual bool isGood();
    virtual int64_t tell();
    virtual void seek(int64_t pos, int32_t d);
    virtual int32_t peek();
    virtual int64_t read(char *s, size_t count);
    virtual int64_t write(const char *s, size_t count);
    virtual void ignore(int64_t count);
    virtual void destroy();
};

} // namespace pxl

#endif /* PXL_LOCAL_FILE_IMPL_HH_ */
