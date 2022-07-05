#ifndef PXL_STDIN_FILE_IMPL_HH_
#define PXL_STDIN_FILE_IMPL_HH_

#include "Pxl/Pxl/interface/pxl/core/macros.hh"
#include "Pxl/Pxl/interface/pxl/core/File.hh"

namespace pxl
{

class PXL_DLL_EXPORT StdFileImpl: public FileImpl
{
	int32_t _mode;
public:

	StdFileImpl();

	StdFileImpl(const std::string &filename, int32_t mode);

	~StdFileImpl();

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

#endif /* PXL_STDIN_FILE_IMPL_HH_ */
