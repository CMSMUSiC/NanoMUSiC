#include "Pxl/Pxl/interface/pxl/core/NamedTMPFile.hh"

#include <cstdio>
#include <cstdlib>

#include <stdexcept>

namespace pxl
{

#ifdef WIN32
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
int mkstemp(char *tmpl)
{
    int ret = -1;
    mktemp(tmpl);
    ret = open(tmpl, O_RDWR | O_BINARY | O_CREAT | O_EXCL | _O_SHORT_LIVED, _S_IREAD | _S_IWRITE);
    return ret;
}
#endif

NamedTMPFile::~NamedTMPFile()
{
    remove(_filename.c_str());
}

NamedTMPFile::NamedTMPFile()
{
#ifdef WIN32
    char template_name[] = "PXL_TMPFILE_XXXXXX";
#else
    char template_name[] = "/tmp/cmguiXXXXXX";
#endif
    int i = mkstemp(template_name);

    if (!i)
    {
        throw std::runtime_error("Cannot create tmpfile");
    }
    close(i);
    _filename = template_name;
}

// holds a tempfile and clears it up on destruction
const std::string &NamedTMPFile::getName()
{
    return _filename;
}

} // namespace pxl
