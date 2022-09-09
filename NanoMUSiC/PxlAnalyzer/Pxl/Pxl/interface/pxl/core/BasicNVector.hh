//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_BASICNVECTOR_HH
#define PXL_BASE_BASICNVECTOR_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <cfloat>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core/Basic3Vector.hh"
#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hh"
#include "Pxl/Pxl/interface/pxl/core/Serializable.hh"
#include "Pxl/Pxl/interface/pxl/core/Stream.hh"

namespace pxl
{

/**
 This class provides a vector of arbitrary length for data storage or algebra in R**N
 */
class PXL_DLL_EXPORT BasicNVector : public Serializable
{
  public:
    BasicNVector() : Serializable(), _data(NULL), _size1(0), _alienarray(false), _name("BasicNVector")
    {
    }

    BasicNVector(size_t size) : Serializable(), _data(NULL), _size1(size), _alienarray(false), _name("BasicNVector")
    {
        _data = new double[size];
        std::fill_n(_data, size, 0);
    }

    BasicNVector(const BasicNVector &orig) : Serializable(orig)
    {
        _size1 = orig.getSize();
        _data = new double[_size1];
        _alienarray = false;
        for (size_t i = 0; i < _size1; i++)
        {
            _data[i] = orig.getElement(i);
        }
        _name = "BasicNVector";
    }

    BasicNVector(const Basic3Vector &vec) : Serializable()
    {
        _size1 = 3;
        _data = new double[_size1];
        _alienarray = false;
        _data[0] = vec.getX();
        _data[1] = vec.getY();
        _data[2] = vec.getZ();
        _name = "BasicNVector";
    }

    explicit BasicNVector(const BasicNVector *orig) : Serializable(*orig)
    {
        _size1 = orig->getSize();
        _data = new double[_size1];
        _alienarray = false;
        for (size_t i = 0; i < _size1; i++)
        {
            _data[i] = orig->getElement(i);
        }
        _name = "BasicNVector";
    }

    BasicNVector(size_t size, double *data, const std::string name = "BasicNVector") : Serializable()

    {
        _size1 = size;
        _data = data;
        _alienarray = true;
        _name = name;
    }

    virtual ~BasicNVector()
    {
        if (_data && (!_alienarray))
        {
            delete[] _data;
        }
    }

    /// Uses the given array as data array without copying it
    /// If the array is deleted, the BasicNVector will segfault
    void use(size_t size, double *data);

    /// Stop using an externally allocated array as internal data storage
    void unUseArray();

    /// resets the size (and allocates memory if not yet done)
    void setSize(size_t size);

    size_t getSize() const
    {
        return _size1;
    }

    void setName(const std::string &name)
    {
        _name = name;
    }

    const std::string &getName() const
    {
        return _name;
    }

    double getElement(size_t i) const
    {
        if (i < _size1)
        {
            return _data[i];
        }
        else
        {
            throw std::runtime_error("Index out of range");
        }
    }

    void setElement(size_t i, double value)
    {
        if (i < _size1)
            _data[i] = value;
        else
            throw std::runtime_error("Index out of range");
    }

    virtual std::ostream &print(int level = 1, std::ostream &os = std::cout, int pan = 1) const;

    double *getArray()
    {
        return _data;
    }

    const double *getConstArray() const
    {
        return _data;
    }

    virtual Serializable *clone() const
    {
        return new BasicNVector(*this);
    }

    static const Id &getStaticTypeId()
    {
        static const Id id("b95fb401-e0a7-87b6-1737-9ce54aae6f29");
        return id;
    }

    virtual const Id &getTypeId() const
    {
        return getStaticTypeId();
    }

    virtual void serialize(const OutputStream &out) const;

    virtual void deserialize(const InputStream &in);

    const BasicNVector &operator=(const BasicNVector &vec);
    BasicNVector operator+(const BasicNVector &vec);
    BasicNVector operator-(const BasicNVector &vec);
    const BasicNVector &operator+=(const BasicNVector &vec);
    const BasicNVector &operator-=(const BasicNVector &vec);
    const BasicNVector &operator*=(double skalar);
    const BasicNVector &operator/=(double skalar);
    const BasicNVector operator/(double skalar) const;

    double &operator()(size_t i);
    //// Scalar product
    double operator*(const BasicNVector &vec) const;
    double norm(uint32_t norm);

  private:
    double *_data;
    size_t _size1;
    bool _alienarray;
    std::string _name;
};
// non-member operators
PXL_DLL_EXPORT bool operator==(const BasicNVector &obj1, const BasicNVector &obj2);
PXL_DLL_EXPORT bool operator!=(const BasicNVector &obj1, const BasicNVector &obj2);

PXL_DLL_EXPORT BasicNVector operator*(double skalar, const BasicNVector &vec);
PXL_DLL_EXPORT BasicNVector operator*(const BasicNVector &vec, double skalar);

} // namespace pxl

#endif // PXL_BASE_BASICNVECTOR_HH
