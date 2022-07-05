//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_INFORMATIONCHUNK_HH
#define PXL_BASE_INFORMATIONCHUNK_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/Serializable.hh"
#include "Pxl/Pxl/interface/pxl/core/UserRecord.hh"

namespace pxl
{
/**
 This class holds generic information and is intended to subdivide
 PXL I/O files into different sections by offering a place to store
 information about the following items in the file.
 The InformationChunk has a name, and all information is stored and
 can be accessed via the contained user records.
 */
class PXL_DLL_EXPORT InformationChunk: public Serializable,
		public UserRecordHelper
{
public:

	/// Get the unique class ID  (UUID) of the InformationChunk.
	virtual const Id& getTypeId() const
	{
		return getStaticTypeId();
	}

	/// The unique class ID  (UUID) of the InformationChunk.
	static const Id& getStaticTypeId()
	{
		static const Id id("7be73236-5038-4988-ba8e-9f65a26c4e72");
		return id;
	}

	/// Write data to the OutputStream \p out.
	virtual void serialize(const OutputStream &out) const;

	/// Read data from the InputStream \p in.
	virtual void deserialize(const InputStream &in);

	/// Creates a deep copy and returns a C++ pointer to the newly-created object.
	virtual Serializable* clone() const
	{
		return new InformationChunk(*this);
	}

	/// Set the name of the InformationChunk
	void setName(const std::string& name)
	{
		_name = name;
	}

	/// Get the name of the InformationChunk
	inline const std::string& getName() const
	{
		return _name;
	}

private:
	std::string _name; /// Name of the information chunk.

};

} //namespace pxl

#endif /*PXL_BASE_INFORMATIONCHUNK_HH*/
