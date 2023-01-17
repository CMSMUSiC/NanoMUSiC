//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_HEP_VERTEX_HH
#define PXL_HEP_VERTEX_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include <iostream>

#include "Pxl/Pxl/interface/pxl/core/Object.hpp"
#include "Pxl/Pxl/interface/pxl/core/weak_ptr.hpp"

#include "Pxl/Pxl/interface/pxl/core/Basic3Vector.hpp"
#include "Pxl/Pxl/interface/pxl/hep/CommonVertex.hpp"

namespace pxl
{

/**
 * This class represents a vertex in high energy collisions. It contains a three-vector
 * and can hold further properties of the decay vertex via the user records. In addition,
 * relations to other objects, e.g. particles, can be established.
 */
class PXL_DLL_EXPORT Vertex : public Object, public CommonVertex
{
  public:
    Vertex() : Object(), _vector()
    {
    }

    Vertex(const Vertex &original) : Object(original), _vector(original._vector)
    {
    }
    explicit Vertex(const Vertex *original) : Object(original), _vector(original->_vector)
    {
    }

    virtual const Id &getTypeId() const
    {
        return getStaticTypeId();
    }

    static const Id &getStaticTypeId()
    {
        static const Id id("80da63d3-c838-466d-9d5b-cddb6110f0e3");
        return id;
    }

    virtual void serialize(const OutputStream &out) const
    {
        Object::serialize(out);
        _vector.serialize(out);
    }

    virtual void deserialize(const InputStream &in)
    {
        Object::deserialize(in);
        _vector.deserialize(in);
    }

    /// This method grants read access to the vector.
    inline const Basic3Vector &getVector() const
    {
        return _vector;
    }

    /// Returns the x component of the vertex.
    inline double getX() const
    {
        return _vector.getX();
    }

    /// Returns the y component of the vertex.
    inline double getY() const
    {
        return _vector.getY();
    }

    /// Returns the z component of the vertex.
    inline double getZ() const
    {
        return _vector.getZ();
    }

    /// Set the x component of the vertex.
    inline void setX(double x)
    {
        _vector.setX(x);
    }

    /// Set the y component of the vertex.
    inline void setY(double y)
    {
        _vector.setY(y);
    }

    /// Set the z component of the vertex.
    inline void setZ(double z)
    {
        _vector.setZ(z);
    }

    /// Set the x, y and z components of the vertex.
    inline void setXYZ(double x, double y, double z)
    {
        _vector.setX(x);
        _vector.setY(y);
        _vector.setZ(z);
    }

    /// Directly set the contained three-vector.
    inline void setVector(const Basic3Vector &vector)
    {
        _vector = vector;
    }

    /// Add the passed x, y, z values to the vertex point.
    inline void addXYZ(double x, double y, double z)
    {
        _vector.setX(x + _vector.getX());
        _vector.setY(y + _vector.getY());
        _vector.setZ(z + _vector.getZ());
    }

    /// Adds the passed three-vector to this vertex.
    inline void addVector(const Basic3Vector &vector)
    {
        _vector += vector;
    }

    /// Adds the passed three-vector of the passed vertex to this vertex.
    inline void addVertex(const Vertex *vx)
    {
        _vector += vx->getVector();
    }

    /// Adds the vector of \p vx.
    inline const Vertex &operator+=(const Vertex &vx)
    {
        _vector += vx._vector;
        return *this;
    }
    /// Subtracts the vector of \p vx.
    inline const Vertex &operator-=(const Vertex &vx)
    {
        _vector -= vx._vector;
        return *this;
    }

    /// Returns a clone of this vertex.
    virtual Serializable *clone() const
    {
        return new Vertex(*this);
    }

    /// Prints information about this vertex to the passed stream \p os.
    virtual std::ostream &print(int level = 1, std::ostream &os = std::cout, int pan = 0) const;

  private:
    Basic3Vector _vector; /// the three-vector

    Vertex &operator=(const Vertex &original)
    {
        return *this;
    }
};

// non-member operators
PXL_DLL_EXPORT bool operator==(const Vertex &obj1, const Vertex &obj2);
PXL_DLL_EXPORT bool operator!=(const Vertex &obj1, const Vertex &obj2);

} // namespace pxl

#endif // PXL_HEP_VERTEX_HH
