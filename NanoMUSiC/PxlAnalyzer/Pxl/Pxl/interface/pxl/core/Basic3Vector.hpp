//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_BASIC3VECTOR_HH
#define PXL_BASE_BASIC3VECTOR_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include "Pxl/Pxl/interface/pxl/core/Stream.hpp"
#include <cfloat>
#include <cmath>
#include <stdexcept>

class runtime_error;

namespace pxl
{

/**
 This class provides a simple threevector with basic algebra. The methods provided are self-explanatory.
 Note that theta is defined as 0 in z-direction, as usual in collider physics.
 */
class PXL_DLL_EXPORT Basic3Vector
{
  public:
    Basic3Vector()
    {
        _v[0] = 0;
        _v[1] = 0;
        _v[2] = 0;
    }
    Basic3Vector(const Basic3Vector &orig)
    {
        _v[0] = orig._v[0];
        _v[1] = orig._v[1];
        _v[2] = orig._v[2];
    }
    explicit Basic3Vector(const Basic3Vector *orig)
    {
        _v[0] = orig->_v[0];
        _v[1] = orig->_v[1];
        _v[2] = orig->_v[2];
    }
    Basic3Vector(double x, double y, double z)
    {
        _v[0] = x;
        _v[1] = y;
        _v[2] = z;
    }

    Basic3Vector(const double x[3])
    {
        _v[0] = x[0];
        _v[1] = x[1];
        _v[2] = x[2];
    }

    virtual ~Basic3Vector()
    {
    }

    virtual void serialize(const OutputStream &out) const
    {
        out.writeDouble(_v[0]);
        out.writeDouble(_v[1]);
        out.writeDouble(_v[2]);
    }

    virtual void deserialize(const InputStream &in)
    {
        in.readDouble(_v[0]);
        in.readDouble(_v[1]);
        in.readDouble(_v[2]);
    }

    inline void setX(double x)
    {
        _v[0] = x;
    }
    inline void setY(double y)
    {
        _v[1] = y;
    }
    inline void setZ(double z)
    {
        _v[2] = z;
    }

    inline void setXYZ(double x, double y, double z)
    {
        _v[0] = x;
        _v[1] = y;
        _v[2] = z;
    }

    inline void setElement(int i, double val)
    {
        if (i < 0 || 1 > 2)
            throw std::runtime_error("Basic3Vector index out of range");
        _v[i] = val;
    }

    inline double getElement(int i) const
    {
        if (i < 0 || 1 > 2)
            throw std::runtime_error("Basic3Vector index out of range");
        return _v[i];
    }

    inline double *getArray()
    /// Returns a pointer to the C Array double[3] for the three
    /// components of the vector
    {
        return _v;
    }

    const double *getConstArray() const
    /// Returns a pointer to the C Array double[3] for the three
    /// components of the vector
    {
        return _v;
    }

    void setCArray(double val[3])
    {
        _v[0] = val[0];
        _v[1] = val[1];
        _v[2] = val[2];
    }

    void setRhoPhi(double perp, double phi);

    void setRhoPhiZ(double perp, double phi, double z);

    void setRThetaPhi(double r, double theta, double phi);

    inline double getX() const
    {
        return _v[0];
    }
    inline double getY() const
    {
        return _v[1];
    }
    inline double getZ() const
    {
        return _v[2];
    }

    bool isNullPerp() const;

    bool isNull() const;

    double getPerp2() const;

    double getPerp() const;

    double getPhi() const;

    double getMag2() const;

    double getMag() const;

    double getCosTheta() const;

    double getCos2Theta() const;

    double getTheta() const;

    /// Returns unit vector in spherical coordinates
    Basic3Vector getETheta() const;

    /// Returns unit vector in spherical coordinates
    Basic3Vector getEPhi() const;

    double deltaRho(const Basic3Vector *fv) const
    {
        return deltaRho(*fv);
    }

    double deltaPhi(const Basic3Vector *fv) const
    {
        return deltaPhi(*fv);
    }

    double deltaTheta(const Basic3Vector *fv) const
    {
        return deltaPhi(*fv);
    }

    double deltaRho(const Basic3Vector &fv) const;

    double deltaPhi(const Basic3Vector &fv) const;

    double deltaTheta(const Basic3Vector &fv) const;

    const Basic3Vector &operator=(const Basic3Vector &vec);

    const Basic3Vector &operator+=(const Basic3Vector &vec);

    const Basic3Vector &operator-=(const Basic3Vector &vec);

    const Basic3Vector &operator*=(double skalar);

    const Basic3Vector &operator/=(double skalar);

    Basic3Vector operator+(const Basic3Vector &vec);

    /// Returns a copy of (this minus vec).
    Basic3Vector operator-(const Basic3Vector &vec);

    /// Returns a copy of (this divided by passed scalar).
    Basic3Vector operator/(double skalar) const;

    /// scalar product
    double operator*(const Basic3Vector &vec) const;

    /// cross product
    inline Basic3Vector cross(const Basic3Vector &p) const
    {
        return Basic3Vector(_v[1] * p._v[2] - p._v[1] * _v[2], _v[2] * p._v[0] - p._v[2] * _v[0],
                            _v[0] * p._v[1] - p._v[0] * _v[1]);
    }

    /// Return a copy of this Basic3Vector where all components are multiplied with -1.
    inline Basic3Vector operator-() const
    {
        return Basic3Vector(-1. * _v[0], -1. * _v[1], -1. * _v[2]);
    }
    inline double &operator()(size_t i)
    {
        if (i > 2)
            throw std::runtime_error("Basic3Vector index out of range");
        return _v[i];
    }

    /// returns the given norm, -1 for infinity norm
    double norm(uint32_t norm) const;

    bool isUnitVector() const;

    /// Returns angle between the vector and vec
    double getAngleTo(const Basic3Vector &vec) const;

    /// Rotates the vector around axis by the angle rad
    void rotate(const Basic3Vector &axis, double angle);

    /// normalizes the vector to 1
    void normalize();

  private:
    double _v[3];
};

// non-member operators
PXL_DLL_EXPORT bool operator==(const Basic3Vector &obj1, const Basic3Vector &obj2);
PXL_DLL_EXPORT bool operator!=(const Basic3Vector &obj1, const Basic3Vector &obj2);

PXL_DLL_EXPORT Basic3Vector operator*(const double skalar, const Basic3Vector &vec);
PXL_DLL_EXPORT Basic3Vector operator*(const Basic3Vector &vec, const double skalar);

} // namespace pxl

#endif // PXL_BASE_BASIC3VECTOR_HH
