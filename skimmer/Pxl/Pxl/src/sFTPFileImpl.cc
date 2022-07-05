#ifdef PXL_ENABLE_SFTP

#include "sFTPFileImpl.hh"

#include "Pxl/Pxl/interface/pxl/core/logging.hh"
#include "Pxl/Pxl/interface/pxl/core/FileFactory.hh"

//#include "libssh2_config.h"
#include <libssh2.h>
#include <libssh2_sftp.h>

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_WS2TCPIP_H
#include "Ws2tcpip.h"
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::sFTPFile"

namespace pxl
{


class sFTPInit
{
	bool initialized;
public:
	sFTPInit() :
			initialized(false)
	{
	}

	static sFTPInit &instance()
	{
		static sFTPInit sftp;
		return sftp;
	}

	void initialize()
	{
		if (initialized)
			return;

#ifdef WIN32
		WSADATA wsadata;

		WSAStartup(MAKEWORD(2,0), &wsadata);
#endif

		int rc = libssh2_init(0);

		if (rc != 0)
		{
			PXL_LOG_ERROR << "libssh2 initialization failed: " << rc;
		}

		initialized = true;

		PXL_LOG_DEBUG << "libssh2 initialized";
	}

	~sFTPInit()
	{
		if (initialized)
		{
			libssh2_exit();
			PXL_LOG_DEBUG << "libssh2 exited";
#ifdef WIN32
			WSACleanup();
#endif
		}
	}
};

static const char *_sftp_error_messages[] =
{ "OK", "EOF", "NO_SUCH_FILE", "PERMISSION_DENIED", "FAILURE", "BAD_MESSAGE",
		"NO_CONNECTION", "CONNECTION_LOST", "OP_UNSUPPORTED", "INVALID_HANDLE",
		"NO_SUCH_PATH", "FILE_ALREADY_EXISTS", "WRITE_PROTECT", "NO_MEDIA",
		"NO_SPACE_ON_FILESYSTEM", "QUOTA_EXCEEDED", "UNKNOWN_PRINCIPAL",
		"LOCK_CONFLICT", "DIR_NOT_EMPTY", "NOT_A_DIRECTORY", "INVALID_FILENAME",
		"LINK_LOOP" };

class ssh_agent
{
	LIBSSH2_AGENT *_agent;
public:
	ssh_agent() :
			_agent(0)
	{
	}

	bool initialize(LIBSSH2_SESSION *session)
	{
		_agent = ::libssh2_agent_init(session);

		if (!_agent)
		{
			PXL_LOG_ERROR << "Failure initializing ssh-agent support";
			return false;
		}

		if (libssh2_agent_connect(_agent))
		{

			PXL_LOG_ERROR << "Failure connecting to ssh-agent";
			return false;
		}
		if (libssh2_agent_list_identities(_agent))
		{

			PXL_LOG_ERROR <<
					"Failure requesting identities to ssh-agent";
			return false;
		}

		return true;
	}

	bool authenticate(const std::string &username)
	{
		int rc;
		struct libssh2_agent_publickey *identity, *prev_identity = NULL;
		while (1)
		{
			rc = libssh2_agent_get_identity(_agent, &identity, prev_identity);

			if (rc == 1)
			{
				PXL_LOG_ERROR << "No public keys founds.";
				return false;
			}

			if (rc < 0)
			{
				PXL_LOG_ERROR <<
						"Failure obtaining identity from ssh-agent support";
				return false;
			}

			if (libssh2_agent_userauth(_agent, username.c_str(), identity))
			{
				PXL_LOG_INFO << "Authentication with username '" <<
						username << "' and public key '" << identity->comment <<
						"' failed!";
			}
			else
			{
				PXL_LOG_INFO << "Authentication with username '" <<
						username << "' and public key '" << identity->comment <<
						"' succeeded!";
				return true;
			}
			prev_identity = identity;
		}
		if (rc)
		{
			PXL_LOG_ERROR << "Couldn't continue authentication.";
			return false;
		}

		return true;
	}

	~ssh_agent()
	{
		::libssh2_agent_disconnect(_agent);
		::libssh2_agent_free(_agent);
	}
};

sFTPFileImpl::sFTPFileImpl() : _session(0), _eof(false)
{
	sFTPInit::instance().initialize();
}

sFTPFileImpl::sFTPFileImpl(const std::string &filename, int32_t mode)
{
	open(filename, mode);
}

sFTPFileImpl::~sFTPFileImpl()
{
	close();
}

bool sFTPFileImpl::connect(const std::string &host, const std::string &port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0; /* Any protocol */

	int s = ::getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
	if (s != 0)
	{
		PXL_LOG_ERROR << "getaddrinfo: ", gai_strerror(s);
		return false;
	}

	/* getaddrinfo() returns a list of address structures.
	 Try each address until we successfully connect(2).
	 If socket(2) (or connect(2)) fails, we (close the socket
	 and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		_socket = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (_socket == -1)
			continue;

		if (::connect(_socket, rp->ai_addr, rp->ai_addrlen) != -1)
			break; /* Success */

#ifdef _MSC_VER
		::closesocket(_socket);
#else
		::close(_socket);
#endif
	}

	if (rp == NULL)
	{
		PXL_LOG_ERROR << "Could not connect to host '"<< _host <<
				"' at port " << _port;
		return false;
	}

	freeaddrinfo(result); /* No longer needed */

	/* Create a session instance
	 */
	_session = libssh2_session_init();

	if (!_session)
		return false;

	/* Since we have set non-blocking, tell libssh2 we are blocking */
	libssh2_session_set_blocking(_session, 1);

	/* ... start it up. This will trade welcome banners, exchange keys,
	 * and setup crypto, compression, and MAC layers
	 */
	int rc = libssh2_session_handshake(_session, _socket);
	if (rc)
	{
		PXL_LOG_ERROR << "Failure establishing SSH session: " << rc;
		return false;
	}

	PXL_LOG_INFO << "Connect to ssh server '" << _host << "' at port " <<
			_port;

	return true;
}

void sFTPFileImpl::disconnect()
{
	if (_session)
	{
		::libssh2_session_disconnect(_session, "normal exit");
		::libssh2_session_free(_session);
		_session = 0;
	}

	if (_socket)
	{
#ifdef _MSC_VER
		::closesocket(_socket);
#else
		::close(_socket);
#endif
		_socket = 0;
		PXL_LOG_INFO << "Disconnected";
	}
}

bool sFTPFileImpl::authenticate(const std::string &username)
{

	/* At this point we havn't authenticated. The first thing to do is check
	 * the hostkey's fingerprint against our known hosts Your app may have it
	 * hard coded, may go to a file, may present it to the user, that's your
	 * call
	 */
	const char *fingerprint = libssh2_hostkey_hash(_session,
			LIBSSH2_HOSTKEY_HASH_SHA1);

	char buffer[256], *b = buffer;
	for (size_t i = 0; i < 20; i++)
	{
		b += sprintf(b, "%02X ", (unsigned char) fingerprint[i]);
	}
	*b = '\0';

	PXL_LOG_INFO << "Fingerprint: " << buffer;

	/* check what authentication methods are available */
	char *userauthlist = libssh2_userauth_list(_session, username.c_str(),
			username.size());

	PXL_LOG_DEBUG << "Authentication methods: "<< userauthlist;
	if (strstr(userauthlist, "publickey") == NULL)
	{
		fprintf(stderr, "\"publickey\" authentication is not supported\n");
		return false;
	}

	ssh_agent agent;
	agent.initialize(_session);
	agent.authenticate(username);

	return true;
}

void sFTPFileImpl::parseUrl(const std::string &url)
{
	typedef std::string::size_type size_type;

	// ssh://gmueller@lx3a24:22/.bashrc
	// ^  ^
	size_type schema_start_pos = url.find_first_not_of(" \t\n\r");
	size_type schema_end_pos = url.find("://");
	if (schema_end_pos == std::string::npos)
		return;
	std::string schema = url.substr(schema_start_pos,
			schema_end_pos - schema_start_pos);

	// ssh://gmueller@lx3a24:22/.bashrc
	//       ^                 ^
	size_type host_start_pos = schema_end_pos + 3;
	size_type host_end_pos = url.find("/", host_start_pos);

	// ssh://gmueller@lx3a24:22/.bashrc
	//       ^                 ^
	size_type cred_start_pos = host_start_pos;
	size_type cred_end_pos = url.find("@", host_start_pos);
	if (cred_end_pos == std::string::npos)
	{
		cred_end_pos = cred_start_pos;
		host_start_pos = cred_end_pos;
	}
	else
	{
		_username = url.substr(cred_start_pos, cred_end_pos - cred_start_pos);
		host_start_pos = cred_end_pos + 1;
	}

	_host = url.substr(host_start_pos, host_end_pos - host_start_pos);
	_path = url.substr(host_end_pos + 1);

	PXL_LOG_DEBUG << "URL: " << url;
	PXL_LOG_DEBUG << "Username: " << _username;
	PXL_LOG_DEBUG << "Host: " << _host;
	PXL_LOG_DEBUG << "Path: " << _path;
}

bool sFTPFileImpl::open(const std::string &filename, int32_t mode)
{
	parseUrl(filename);
	if (_port.empty())
	{
		_port = "22";
	}

	if (_username.empty() && ::getenv("USER"))
	{
		_username = ::getenv("USER");
	}

	connect(_host, _port);
	authenticate(_username);

	_sftp_session = libssh2_sftp_init(_session);

	if (!_sftp_session)
	{
		PXL_LOG_ERROR << "Unable to init SFTP session";
		return false;
	}

	int flags = 0;
	if (mode & OpenRead)
		flags = LIBSSH2_FXF_READ;
	else if (mode & OpenWrite)
	{
		flags = LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT;
		if (mode & OpenOverwrite)
			flags |= LIBSSH2_FXF_TRUNC;
	}

	/* Request a file via SFTP */
	_sftp_handle = ::libssh2_sftp_open(_sftp_session, _path.c_str(), flags,
			LIBSSH2_SFTP_S_IRWXU);

	if (!_sftp_handle)
	{
		PXL_LOG_ERROR << "Unable to open file with SFTP: " <<
				_sftp_error_messages[libssh2_sftp_last_error(_sftp_session)];
		return false;
	}

	_eof = false;

	return true;
}

void sFTPFileImpl::close()
{
	if (_sftp_handle)
	{
		::libssh2_sftp_close_handle(_sftp_handle);
		_sftp_handle = 0;
	}

	if (_sftp_session)
	{
		::libssh2_sftp_shutdown(_sftp_session);
		_sftp_session = 0;
	}

	disconnect();
}

bool sFTPFileImpl::isEof()
{
	return _eof;
}

bool sFTPFileImpl::isOpen()
{
	return true;
}

bool sFTPFileImpl::isBad()
{
	return false;
}

void sFTPFileImpl::clear()
{

}

bool sFTPFileImpl::isGood()
{
	return true;
}

int64_t sFTPFileImpl::tell()
{
	return ::libssh2_sftp_tell64(_sftp_handle);
}

void sFTPFileImpl::seek(int64_t pos, int32_t d)
{
	::libssh2_sftp_seek64(_sftp_handle, pos);
}

int32_t sFTPFileImpl::peek()
{
	if (_eof)
		return EOF;

	char c;
	if (::libssh2_sftp_read(_sftp_handle, (char *) &c, 1))
	{
		::libssh2_sftp_seek64(_sftp_handle, tell() - 1);
		return c;
	}
	else
	{
		_eof = true;
		return EOF;
	}
}

int64_t sFTPFileImpl::read(char *s, size_t count)
{
	size_t read = 0;

	while (read < count)
	{
		int64_t rc = ::libssh2_sftp_read(_sftp_handle, s + read, count - read);
		if (rc < 0)
		{
			PXL_LOG_ERROR <<
					"Unable to read file with SFTP:" <<
					_sftp_error_messages[libssh2_sftp_last_error(_sftp_session)];
			_eof = true;
			return 0;
		}
		else if (rc > 0)
		{
			read += rc;
		}
		else
		{
			break;
		}
	}
	return read;
}

int64_t sFTPFileImpl::write(const char *s, size_t count)
{
	return ::libssh2_sftp_write(_sftp_handle, s, count);
}

void sFTPFileImpl::ignore(int64_t count)
{
	::libssh2_sftp_seek64(_sftp_handle, tell() + count);
}

void sFTPFileImpl::destroy()
{
	delete this;
}

}

#endif
