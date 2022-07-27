//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_RANDOMACCESSINPUTFILE_HH
#define PXL_IO_RANDOMACCESSINPUTFILE_HH

#include <fstream>
#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/InputFile.hh"

namespace pxl
{
// io
/**
 This class offers an easy handling of the PXL I/O. Various methods to access
 the content of I/O files are offered.
 */
class RandomAccessInputFile : public InputFile
{
public:

	RandomAccessInputFile() :
		InputFile(),
		_currentIndex(0),
		_eventCount(0),
		_knowEventCount(false)
	{
	}

	RandomAccessInputFile(const std::string& filename) :
		InputFile(filename),
		_currentIndex(0),
		_eventCount(0),
		_knowEventCount(false)
	{
	}

	~RandomAccessInputFile()
	{
		close();
	}

	void open(const std::string& filename)
	{
		_currentIndex = 0;
		_eventCount = 0;
		_knowEventCount = false;
		_filename = filename;
		InputFile::open(filename);
	}

	void close()
	{
		InputFile::close();
		_currentIndex = 0;
		_eventCount = 0;
		_knowEventCount = false;
	}

	void reopen()
	{
		// safe variables to restore it after opening
		unsigned int eventCount = _eventCount;
		bool knowEventCount = _knowEventCount;

		close();
	
		try {
			open(_filename);
		} catch (std::exception &e) {
			return false;
		}

		// restore event count information
		_eventCount = eventCount;
		_knowEventCount = knowEventCount;
	}
	
    bool seekToEvent(unsigned int event)
	{
		// reopen the file if we need to go back
		if (event < _currentIndex)
		{
			//reopen();
		}
		
		int toSkip = event - _currentIndex;
		int skipped = 0;
	
		// skip to the needed event
		while (skipped < toSkip && _inputFile.skip())
		{
			skipped += 1;
			_currentIndex += 1;
		}
		while (skipped > toSkip && _inputFile.previous())
		{
			skipped -= 1;
			_currentIndex -1= 1;
		}

		// update eventCount
		if (skipped < toSkip)
		{
			_eventCount = _currentIndex;
			_knowEventCount = true;
		}
		else if (_currentIndex >= _eventCount)
		{
			_eventCount = _currentIndex + 1;
		}

		if (toSkip == skipped)
			return true;
	
		return false;
	}

private:
	RandomAccessInputFile(const RandomAccessInputFile& original)
	{
	}
	
	RandomAccessInputFile& operator= (const RandomAccessInputFile& other)
	{
		return *this;
	}		
		
	unsigned long _currentIndex;
	unsigned long _eventCount;
	bool _knowEventCount;
	std::string _filename;
	InputFile _inputFile;
};

} //namespace pxl

#endif /*PXL_IO_RANDOMACCESSINPUTFILE_HH*/
