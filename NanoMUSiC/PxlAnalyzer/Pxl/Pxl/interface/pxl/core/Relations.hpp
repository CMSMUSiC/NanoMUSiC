//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_RELATIONS_HH
#define PXL_BASE_RELATIONS_HH

#include <set>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core/Id.hpp"
#include "Pxl/Pxl/interface/pxl/core/Stream.hpp"

namespace pxl
{
// We only work with Relative pointers here. The class Relative
// containts three instances of the relations.
class Relative;

/**

 The class pxl::Relative owns three instances of pxl::Relations
 for managing mother, daughter and flat relations to other pxl::Relative derivatives.
 For decay tree integrity reasons, the class pxl::Relative allows to
 establish relations only to objects contained in the same object owner
 (and in the case of both objects not being contained in owners).
 The Relative only allows const access to the three Relations instances
 (mother, daughter, flat) to ensure the integrity of all relations.
 */

class PXL_DLL_EXPORT Relations
{
  public:
    typedef std::set<Relative *>::const_iterator const_iterator;
    typedef std::set<Relative *>::iterator iterator;

    /// Constructor
    Relations()
    {
    }

    /// Write the relations to the OutputStream \p out.
    /// Deserialisation of relations is done via the object
    /// owner class.
    void serialize(const OutputStream &out) const;

    /// Provide direct access to the underlying container.
    const std::set<Relative *> &getContainer() const
    {
        return _relatives;
    }

    /// Insert \p relative into the relations.
    /// Returns true if successful.
    bool set(Relative *relative)
    {
        return (_relatives.insert(relative)).second;
    }

    /// Erase relative from the relations.
    /// Returns true if successful.
    bool erase(Relative *relative)
    {
        return (_relatives.erase(relative) > 0);
    }

    /// Returns true \p relative is contained in these relations.
    bool has(Relative *relative) const
    {
        return (_relatives.count(relative) > 0);
    }

    /// Returns the first relative. In case the container is
    /// empty, 0 is returned.
    Relative *getFirst() const
    {
        if (_relatives.begin() != _relatives.end())
            return (*_relatives.begin());
        return 0;
    }

    /// Fills all relatives which match or inherit from the
    /// template type \p objecttype into the passed vector.
    /// Returns the number of added objects.
    template <class objecttype>
    size_t getObjectsOfType(std::vector<objecttype *> &relatives) const
    {
        size_t size = relatives.size();
        for (const_iterator iter = _relatives.begin(); iter != _relatives.end(); ++iter)
        {
            objecttype *obj = dynamic_cast<objecttype *>(*iter);
            if (obj)
                relatives.push_back(obj);
        }
        return relatives.size() - size;
    }

    /// Returns the size of the relations.
    size_t size() const
    {
        return _relatives.size();
    }

    /// Returns const begin-iterator.
    const_iterator begin() const
    {
        return _relatives.begin();
    }

    /// Returns begin-iterator.
    iterator begin()
    {
        return _relatives.begin();
    }

    /// Returns const end-iterator.
    const_iterator end() const
    {
        return _relatives.end();
    }

    /// Returns end-iterator.
    iterator end()
    {
        return _relatives.end();
    }

    /// Deletes all contained relations.
    void clearContainer()
    {
        _relatives.clear();
    }

  private:
    std::set<Relative *> _relatives; /// Sorted container for the related objects.
};

} // namespace pxl

#endif // PXL_BASE_RELATIONS_HH
