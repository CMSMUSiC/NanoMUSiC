//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_WEAK_PTR_HH
#define PXL_BASE_WEAK_PTR_HH

#include <iostream>
#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/WkPtrBase.hpp"
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

namespace pxl
{

// ptl

/**
 This class template represents a weak pointer to PXL objects of \p objecttype that aggregate data of \p datatype.
 */
template <class objecttype>
class weak_ptr : public WkPtrBase
{
  public:
    weak_ptr() : WkPtrBase()
    {
    }
    weak_ptr(objecttype *ptr) : WkPtrBase()
    {
        WkPtrBase::connect(ptr);
    }
    weak_ptr(objecttype &object) : WkPtrBase()
    {
        WkPtrBase::connect(&object);
    }
    weak_ptr(const weak_ptr<objecttype> &original) : WkPtrBase()
    {
        WkPtrBase::connect((objecttype *)original._objectRef);
    }
    explicit weak_ptr(const weak_ptr<objecttype> *original) : WkPtrBase()
    {
        WkPtrBase::connect((objecttype *)original->_objectRef);
    }

    virtual ~weak_ptr()
    {
        WkPtrBase::connect(0);
    }

    /// This virtual method creates a deep copy and returns a C++ pointer to the newly-created weak pointer instance.
    virtual WkPtrBase *clone() const
    {
        return new weak_ptr<objecttype>(*this);
    }

    /// This assignment operator causes the weak pointer to reference the object referenced by \p pptr.
    inline void operator=(const weak_ptr<objecttype> &pptr)
    {
        connect(pptr._objectRef);
    }
    /// This assignment operator causes the weak pointer to reference the object.
    inline void operator=(objecttype &object)
    {
        connect(&object);
    }
    /// This assignment operator causes the weak pointer to reference the object pointed to by \p objectptr.
    inline void operator=(objecttype *objectptr)
    {
        connect(objectptr);
    }

    // methods to grant object & data access
    /// This method provides direct access to the referenced object.
    inline objecttype &object() const
    {
        return *access();
    }

    /// This arrow operator de-references the weak pointer.
    inline objecttype *operator->() const
    {
        return access();
    }

    /// This arrow operator de-references the weak pointer.
    inline objecttype *ptr() const
    {
        return dynamic_cast<objecttype *>(_objectRef);
    }

    inline operator objecttype *() const
    {
        return dynamic_cast<objecttype *>(_objectRef);
    }

    /// This method attempts a dynamic cast on the referenced object
    static weak_ptr<objecttype> *cast_dynamic(WkPtrBase *orig)
    {
        objecttype *object = dynamic_cast<objecttype *>(orig->pointer());
        if (!object)
            return 0;
        // FIXME: This is crude but required:
        if (PXL_UNLIKELY(reinterpret_cast<void *>(object) != reinterpret_cast<void *>(orig->pointer())))
            throw std::runtime_error("WkPtrSpec::cast_dynamic(): Unsupported multiple inheritance configuration.");
        return reinterpret_cast<weak_ptr<objecttype> *>(orig);
    }

    // safe access to object
    inline objecttype *access() const
    {
        if (_objectRef)
            return (objecttype *)_objectRef;
        throw std::runtime_error("WkPtrSpec::access(): FATAL: The object you intend to access does not exist!");
        return 0;
    }
};

template <class objecttype>
objecttype &operator*(weak_ptr<objecttype> &wkPtr)
{
    return wkPtr.object();
}

template <class objecttype>
const objecttype &operator*(const weak_ptr<objecttype> &wkPtr)
{
    return wkPtr.object();
}

} // namespace pxl

#endif // PXL_BASE_WEAK_PTR_HH
