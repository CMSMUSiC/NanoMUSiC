//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_SERIALIZABLE_HH
#define PXL_IO_SERIALIZABLE_HH

#include <sstream>
#include "Pxl/Pxl/interface/pxl/core/Stream.hh"
#include "Pxl/Pxl/interface/pxl/core/Id.hh"
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

namespace pxl
{

/**
 This class is the abstract base class for all objects to be stored in a PXL I/O file.
 It holds the unique ID (UUID) of each individual object. In addition, a UUID indicating the class
 type must be implemented in derived classes - each class can be identified via its unique ID.
 (De-) Serialization happens via consecutive calls to the serialize/deserialize methods of the 
 base classes.
 */
class PXL_DLL_EXPORT Serializable
{
public:

	/// Constructor.
	Serializable()
	{
	}

	/// Copy constructor. A copied object gets a new unique ID.
	Serializable(const Serializable& original) : _id()
	{
	}

	/// Assignment operator, UUID is changed in case this object is changed.
	Serializable& operator=(const Serializable& original)
	{
		if (this != &original)
			_id = Id::create();
		return *this;
	}

	/// Destructor.
	virtual ~Serializable()
	{
	}

	/// Id of the object's class (== type). This method must be reimplemented
	/// by all derived classes to return a distinct UUID identifying the
	/// class, e.g. the Particle.
	virtual const Id& getTypeId() const = 0;

	/// Returns the unique ID (UUID) of the individual object. Each individual 
	/// object can unambiguously be identified by this ID.
	const Id& getId() const
	{
		return _id;
	}

	/// Pure virtual method which should be implemented by all derived classes
	/// to return a new copy of the object.
	virtual Serializable* clone() const = 0;

	/// This method serializes this object by serializing the type ID and the 
	/// unique object ID. When extending this method, derived classes must
	/// first call the base class method.
	virtual void serialize(const OutputStream &out) const
	{
		// Serialize ID of the type
		getTypeId().serialize(out);
		// Serialize UUID.
		_id.serialize(out);
	}

	/// This method deserializes this object. When extending this method, derived 
	/// classes must first call the base class method.
	virtual void deserialize(const InputStream &in)
	{
		// Deserialize uuid;
		_id.deserialize(in);
	}

	/// Print information to the passed ostream \p os. Can be reimplemented by 
	/// derived classes to print information specific to the derived class.
	virtual std::ostream& print(int level=1, std::ostream& os=std::cout, int pan=1) const
	{
		os << "Serializable [" << getId() <<"]"<< std::endl;
		return os;
	}

	/// Returns a string with the output of the virtual print method. In general,
	/// there is no need for re-implementation in derived classes.
	virtual const std::string toString() const
	{
		std::ostringstream ss;
		this->print(0,ss);
		return ss.str();
	}
private:
	/// The unique ID (UUID) of this object. Each object can unambiguously be
	/// identified by this ID.
	Id _id;
};

}

#endif /*PXL_IO_SERIALIZABLE_HH*/
