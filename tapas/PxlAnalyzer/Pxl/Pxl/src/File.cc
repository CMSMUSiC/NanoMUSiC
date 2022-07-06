#include "Pxl/Pxl/interface/pxl/core/File.hh"
#include "Pxl/Pxl/interface/pxl/core/logging.hh"
#include "Pxl/Pxl/interface/pxl/core/FileFactory.hh"

#include "LocalFileImpl.hh"

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::File"

namespace pxl
{

File::File() :
		impl(0)
{

}

File::File(const std::string &filename, int32_t mode) :
		impl(0)
{
	open(filename, mode);
}

File::~File()
{
	close();
}

void File::close()
{
	if (impl)
	{
		impl->close();
		impl->destroy();
		impl = 0;
	}
}

bool File::isEof()
{
	if (impl == 0)
		throw std::runtime_error("invalid File implementation!");
	return impl->isEof();
}

bool File::isBad()
{
	if (impl == 0)
		return true;
	else
		return impl->isBad();
}
bool File::isOpen()
{
	if (impl == 0)
		return false;
	else
		return impl->isOpen();
}
void File::clear()
{
	if (impl)
		impl->clear();
}
bool File::isGood()
{
	if (impl == 0)
		throw std::runtime_error("invalid File implementation!");
	return impl->isGood();
}
int64_t File::tell()
{
	if (impl == 0)
		throw std::runtime_error("invalid File implementation!");
	return impl->tell();
}
void File::seek(int64_t pos, int32_t d)
{
	if (impl == 0)
		throw std::runtime_error("invalid File implementation!");
	impl->seek(pos, d);
}
int32_t File::peek()
{
	if (impl == 0)
		throw std::runtime_error("invalid File implementation!");
	return impl->peek();
}
int64_t File::read(char *s, size_t count)
{
	if (impl == 0)
		throw std::runtime_error("invalid File implementation!");
	return impl->read(s, count);
}
int64_t File::write(const char *s, size_t count)
{
	if (impl == 0)
		throw std::runtime_error("invalid File implementation!");
	return impl->write(s, count);
}
void File::ignore(int64_t count)
{
	if (impl == 0)
		throw std::runtime_error("invalid File implementation!");
	impl->ignore(count);
}
void File::destroy()
{
	delete this;
}

bool File::open(const std::string &filename, int32_t mode)
{
	std::string expanded = expandEnvironmentVariables(filename);
	std::string schema, path;
	splitSchema(expanded, schema, path);
	if (schema.empty())
		schema = "file";

	PXL_LOG_INFO << "Open: schema=" << schema << ", path=" << path;

	impl = FileFactory::instance().create(schema);

	if (impl == 0)
	{
		PXL_LOG_ERROR << "Unknown Schema: " << schema;
		return false;
	}

	return impl->open(expanded, mode);
}

}
