//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_HEP_COLLISION_HH
#define PXL_HEP_COLLISION_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include "Pxl/Pxl/interface/pxl/core/Object.hpp"

namespace pxl
{
/**
 This class represents individual interactions in multicollision events.
 It allows the separation of different collisions as they occur
 at high-rate hadron colliders by providing the relation management
 necessary to associate pxl::Vertex or pxl::Particle objects, for instance.
 */

class PXL_DLL_EXPORT Collision : public Object
{
  public:
    Collision() : Object()
    {
    }
    /// This copy constructor provides a deep copy of the event container \p original with all data members,
    /// hep objects, and their (redirected) relations.
    Collision(const Collision &original) : Object(original)
    {
    }
    /// This copy constructor provides a deep copy of the event container \p original with all data members,
    /// hep objects, and their (redirected) relations.
    explicit Collision(const Collision *original) : Object(original)
    {
    }

    virtual WkPtrBase *createSelfWkPtr()
    {
        return new weak_ptr<Collision>(this);
    }

    virtual const Id &getTypeId() const
    {
        return getStaticTypeId();
    }

    static const Id &getStaticTypeId()
    {
        static const Id id("59b2f95c-5142-4970-844f-226ebbc57a99");
        return id;
    }

    virtual void serialize(const OutputStream &out) const
    {
        Object::serialize(out);
    }

    virtual void deserialize(const InputStream &in)
    {
        Object::deserialize(in);
    }

    /// Creates a deep copy and returns a C++ pointer to the newly-created object.
    virtual Serializable *clone() const
    {
        return new Collision(*this);
    }

    virtual std::ostream &print(int level = 1, std::ostream &os = std::cout, int pan = 0) const;

  private:
    Collision &operator=(const Collision &original)
    {
        return *this;
    }
};

} // namespace pxl

#endif // PXL_HEP_COLLISION_HH
