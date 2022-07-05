//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_WKPTRBASE_HH
#define PXL_BASE_WKPTRBASE_HH

#include <stdexcept>
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

namespace pxl {

// ptl

class Relative;

/** 
 This base class provides common functionalities for all derived PXL weak pointers. 
 */
class PXL_DLL_EXPORT WkPtrBase
{
public:
	virtual ~WkPtrBase()
	{
		connect(0);
	}

	/// This virtual method creates a deep copy and returns a C++ pointer to the newly-created weak pointer instance.  
	virtual WkPtrBase* clone() const
	{
		return new WkPtrBase(*this);
	}

	/// This method returns a C++ pointer of type Relative to the referenced object  
	inline Relative* pointer() const
	{
		return _objectRef;
	}
	/// This method returns true, if the referenced object exists.  
	inline bool valid() const
	{
		return _objectRef != 0;
	}

	/// This allows pointer-like tests if the weak pointer is valid.
	inline operator bool()
	{
		return valid();
	}
	/// This arrow operator de-references the weak pointer.   
	inline Relative* operator->() const
	{
		return access();
	}
	/// compare the referenced object pointers
	inline bool operator==(WkPtrBase &other) const
	{
		return (_objectRef == other.pointer());
	}
	/// compare the referenced object pointers   
	inline bool operator!=(WkPtrBase &other) const
	{
		return (_objectRef != other.pointer());
	}

	/// This method attempts a dynamic cast on the referenced object
	static inline WkPtrBase* cast_dynamic(WkPtrBase* orig)
	{
		return orig;
	}

	// safe access to object
	inline Relative* access() const 
	{
		if (_objectRef)
			return _objectRef;
		throw std::runtime_error("WkPtrBase::access(): FATAL: The object you intend to access does not exist!");
		return 0;
	}

protected:
	WkPtrBase() :
		_notifyChainIn(0), _notifyChainOut(0), _objectRef(0)
	{
	}

	void notifyDeleted();

	void connect(Relative* pointer);

	WkPtrBase* _notifyChainIn;
	WkPtrBase* _notifyChainOut;

	Relative* _objectRef;

	friend class Relative;
};

} // namespace pxl

#endif // PXL_BASE_WK_PTR_BASE_HH
