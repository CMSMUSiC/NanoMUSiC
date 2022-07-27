//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_SFTP_FILE_HH_
#define PXL_SFTP_FILE_HH_

#include "Pxl/Pxl/interface/pxl/core/File.hh"

typedef struct _LIBSSH2_SESSION LIBSSH2_SESSION;
typedef struct _LIBSSH2_SFTP LIBSSH2_SFTP;
typedef struct _LIBSSH2_SFTP_HANDLE LIBSSH2_SFTP_HANDLE;

namespace pxl
{

class PXL_DLL_EXPORT sFTPFileImpl: public FileImpl
{
	LIBSSH2_SESSION *_session;
	LIBSSH2_SFTP *_sftp_session;
	LIBSSH2_SFTP_HANDLE *_sftp_handle;

	std::string _username, _password, _host, _port, _path;

	int32_t _mode;
	int _socket;
	bool _eof;

	bool connect(const std::string &host, const std::string &port);
	void disconnect();
	bool authenticate(const std::string &username);
	void parseUrl(const std::string &url);

public:

	sFTPFileImpl();

	sFTPFileImpl(const std::string &filename, int32_t mode);

	~sFTPFileImpl();

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
