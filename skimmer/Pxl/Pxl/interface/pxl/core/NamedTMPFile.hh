#ifndef NAMED_TMP_FILE_HH
#define NAMED_TMP_FILE_HH

#include "Pxl/Pxl/interface/pxl/core/macros.hh"
#include <string>

#ifdef WIN32
	#include <io.h>
#else
	#include <unistd.h>
#endif

namespace pxl
{
// holds a tempfile and clears it up on destruction
class PXL_DLL_EXPORT NamedTMPFile
{
public:
	
	NamedTMPFile();
	~NamedTMPFile();
	const std::string& getName();

private:
	std::string _filename;		
};

} // namespace


#endif //NAMED_TMP_FILE_HH
