//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_OBJECT_HH
#define PXL_BASE_OBJECT_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <string>
#include <iostream>
#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/Relative.hh"
#include "Pxl/Pxl/interface/pxl/core/weak_ptr.hh"
#include "Pxl/Pxl/interface/pxl/core/UserRecord.hh"
#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hh"

namespace pxl
{

/**
 This class provides common functionalities of PXL physics objects like
 data members for storing an object name and flags for status, Monte-Carlo mode and
 object locking; more specific information, such as b-tags, jet cone sizes or energy
 corrections, for instance, can be stored in the so-called user records (see UserRecord).
 An integer workflag facilitates tagging of individual objects.
 */
class PXL_DLL_EXPORT Object: public Relative, public UserRecordHelper
{
public:
	Object() :
		Relative(), _locked(0), _workflag(0)
	{
	}

	Object(const Object& original) :
		Relative(original), UserRecordHelper(original), _locked(original._locked), _workflag(
				original._workflag)
	{
	}

	explicit Object(const Object* original) :
		Relative(*original), UserRecordHelper(*original), _locked(original->_locked), _workflag(
				original->_workflag)
	{
	}

	virtual const Id& getTypeId() const
	{
		return getStaticTypeId();
	}

	static const Id& getStaticTypeId()
	{
		static const Id id("3b3a2442-04f6-400e-8e30-1de2dbc8d628");
		return id;
	}

	virtual void serialize(const OutputStream &out) const;

	virtual void deserialize(const InputStream &in);

	/// Creates a deep copy and returns a C++ pointer to the newly-created object.
	virtual Serializable* clone() const
	{
		return new Object(*this);
	}

	/// Returns the value of the lock flag.
	inline bool isLocked() const
	{
		return _locked;
	}

	/// Returns the value of the workflag.
	inline int getWorkFlag() const
	{
		return _workflag;
	}

	/// Sets the value of the lock flag to \p v.
	inline void setLocked(bool v)
	{
		_locked = v;
	}

	/// Sets the value of the workflag to \p v.
	inline void setWorkFlag(int v)
	{
		_workflag = v;
	}

	/// Prints out object state information on various verbosity levels.
	/// @param level verbosity level
	/// @param os output _stream, default is std::cout
	/// @param pan print indention
	/// @return output _stream
	virtual std::ostream& print(int level = 1, std::ostream& os = std::cout,
			int pan = 0) const;

	virtual std::ostream& printContent(int level = 1, std::ostream& os =
			std::cout, int pan = 0) const;

	virtual WkPtrBase* createSelfWkPtr()
	{
		return new weak_ptr<Object> (this);
	}

private:
	bool _locked;
	int _workflag;

	Object& operator=(const Object& original)
	{
		return *this;
	}
};

}
// namespace pxl

#endif // PXL_BASE_OBJECT_HH
