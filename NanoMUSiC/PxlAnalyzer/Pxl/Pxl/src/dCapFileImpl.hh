//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_DCAP_FILE_HH_
#define PXL_DCAP_FILE_HH_

#include "Pxl/Pxl/interface/pxl/core/File.hh"

namespace pxl
{

class PXL_DLL_EXPORT dCapFileImpl: public FileImpl
{
	FILE *_file;
	int32_t _mode;

public:

	dCapFileImpl();

	dCapFileImpl(const std::string &filename, int32_t mode);

	~dCapFileImpl();

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

}
#endif /* PXL_SFTP_FILE_HH_ */
