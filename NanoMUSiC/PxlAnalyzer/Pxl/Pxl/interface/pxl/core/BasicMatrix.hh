//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_BASICMATRIX_HH
#define PXL_BASE_BASICMATRIX_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <cfloat>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core/Basic3Vector.hh"
#include "Pxl/Pxl/interface/pxl/core/BasicNVector.hh"
#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hh"
#include "Pxl/Pxl/interface/pxl/core/Serializable.hh"
#include "Pxl/Pxl/interface/pxl/core/Stream.hh"
namespace pxl
{

/**
 * Storage Order
 */
typedef enum
{
    ROWMAJOR,
    COLUMNMAJOR
} StorageOrder;

/**
 This class provides a Matrix of arbitrary dimensions for data storage
 or algebra in R**NxM

 The Matrix is stored as BLAS compatible array of doubles. The default storage order is the C default of Row-Major
 order, meaning, that a Matrix is stored row by row
 double M[] = { 1,2,3,
 4,5,6
 }
 The Same matrix in Column Order would be stored as
 double M[] = {1,4,2,5,3,6};

 size1, usually defined by index i, denotes the row in row-based order,
 respectivelty the column in column-based order. In row-based order
 element i,j is hence element i*size2+j of the array, and in column-based
 order element i,j is also element i*size2+j of the array.
 **/
class PXL_DLL_EXPORT BasicMatrix : public Serializable
{
  public:
    BasicMatrix() : Serializable(), _size1(0), _size2(0), _storageType(ROWMAJOR), _alienarray(false), _data(NULL)
    {
    }
    BasicMatrix(size_t size1, size_t size2, std::string name = "BasicMatrix")
        : Serializable(), _size1(size1), _size2(size2), _storageType(ROWMAJOR), _alienarray(false), _name(name)
    {
        _data = new double[size1 * size2];
        std::fill_n(_data, size1 * size2, 0);
    }

    BasicMatrix(size_t size1, size_t size2, StorageOrder storage, const std::string name = "BasicMatrix")
        : Serializable(), _size1(size1), _size2(size2), _storageType(storage), _alienarray(false), _name(name)
    {
        _data = new double[size1 * size2];
        std::fill_n(_data, size1 * size2, 0);
    }

    BasicMatrix(const BasicMatrix &orig) : Serializable(orig)
    {
        _size1 = orig._size1;
        _size2 = orig._size2;
        _data = new double[_size1 * _size2];
        _alienarray = false;
        _storageType = orig._storageType;
        for (size_t i = 0; i < _size1 * _size2; i++)
        {
            _data[i] = orig.getConstArray()[i];
        }
    }

    explicit BasicMatrix(const BasicMatrix *orig) : Serializable(*orig)
    {
        _size1 = orig->_size1;
        _size2 = orig->_size2;
        _data = new double[_size1 * _size2];
        _alienarray = false;
        _storageType = orig->_storageType;
        for (size_t i = 0; i < _size1 * _size2; i++)
        {
            _data[i] = orig->getConstArray()[i];
        }
    }

    BasicMatrix(size_t size1, size_t size2, double *data, StorageOrder storage, const std::string name = "BasicMatrix")
        : Serializable(), _size1(size1), _size2(size2), _storageType(storage), _alienarray(true), _name(name),
          _data(data)
    {
    }

    virtual ~BasicMatrix()
    {
        if ((_data) && (!_alienarray))
            delete[] _data;
    }

    void setName(const std::string &name)
    {
        _name = name;
    }

    const std::string &getName() const
    {
        return _name;
    }
    /// Uses an external array as data array
    void use(size_t size1, size_t size2, double *data);

    /// Check whether an external array is used
    bool isAlien() const
    {
        return _alienarray;
    }

    /// Interpret matrix as rowmajor
    void setRowBasedStorage()
    {
        _storageType = ROWMAJOR;
    }

    /// Interpret matrix as columnmajor
    void setColumnBasedStorage()
    {
        _storageType = COLUMNMAJOR;
    }

    bool isRowBasedStorage() const;
    bool isColumnBasedStorage() const;

    /// Returns first dimension of matrix
    size_t getSize1() const
    {
        return _size1;
    }

    /// Returns second dimension of matrix
    size_t getSize2() const
    {
        return _size2;
    }

    /// Returns numbers of rows of matrix
    size_t getNumberOfRows() const
    {
        if (_storageType == ROWMAJOR)
            return _size1;
        else
            return _size2;
    }

    /// Returns numbers of columns of matrix
    size_t getNumberOfColumns() const
    {
        if (_storageType == ROWMAJOR)
            return _size2;
        else
            return _size1;
    }

    // Resizes the array. The data stored gets lost
    void resize(size_t i, size_t j);

    /// Only reshapes the matrix but preserves the data in the array
    void reshape(size_t i, size_t j);

    /// Returns element m_i,j of the matrix
    double getElement(size_t i, size_t j) const
    {
        if (i >= _size1)
        {
            if (this->isRowBasedStorage())
                throw std::runtime_error("Index exceeds number of rows");
            else
                throw std::runtime_error("Index exceeds number of columns");
        }
        if (j >= _size2)
        {
            if (this->isColumnBasedStorage())
                throw std::runtime_error("Index exceeds number of rows");
            else
                throw std::runtime_error("Index exceeds number of columns");
        }

        return _data[i * _size2 + j];
    }

    /// Sets element m_i,j
    void setElement(size_t i, size_t j, double val)
    {
        if (i >= _size1)
        {
            if (this->isRowBasedStorage())
                throw std::runtime_error("Index exceeds number of rows");
            else
                throw std::runtime_error("Index exceeds number of columns");
        }
        if (j >= _size2)
        {
            if (this->isColumnBasedStorage())
                throw std::runtime_error("Index exceeds number of rows");
            else
                throw std::runtime_error("Index exceeds number of columns");
        }

        _data[i * _size2 + j] = val;
    }

    /// Returns a pointer to the internal array storing the matrix, e.g.
    /// for access using blas or lapack implementations
    double *getArray()
    {
        return _data;
    }

    /// Returns a const pointer to the internal array storing the matrix, e.g.
    /// for access using blas or lapack implementations
    const double *getConstArray() const
    {
        return _data;
    }

    const BasicMatrix &operator=(const BasicMatrix &M);
    BasicMatrix operator+(const BasicMatrix &M);
    BasicMatrix operator-(const BasicMatrix &M);
    const BasicMatrix &operator+=(const BasicMatrix &M);
    const BasicMatrix &operator-=(const BasicMatrix &M);
    const BasicMatrix &operator*=(double skalar);
    const BasicMatrix &operator/=(double skalar);
    const BasicMatrix operator/(double skalar) const;

    /// Returns element m_i,j of M
    double &operator()(size_t i, size_t j);

    virtual std::ostream &print(int level = 1, std::ostream &os = std::cout, int pan = 1) const;

    virtual Serializable *clone() const
    {
        return new BasicMatrix(*this);
    }

    virtual void serialize(const OutputStream &out) const;

    virtual void deserialize(const InputStream &in);

    static const Id &getStaticTypeId()
    {
        static const Id id("85ebd9cb-08ae-6c7b-2257-45604aae6f55");
        return id;
    }

    virtual const Id &getTypeId() const
    {
        return getStaticTypeId();
    }

  private:
    size_t _size1;
    size_t _size2;
    StorageOrder _storageType;
    bool _alienarray; // true if working with an not self allocated array - if so, memory is not freed!
    std::string _name;

  protected:
    double *_data;
};
PXL_DLL_EXPORT bool operator==(const BasicMatrix &obj1, const BasicMatrix &obj2);
PXL_DLL_EXPORT bool operator!=(const BasicMatrix &obj1, const BasicMatrix &obj2);

PXL_DLL_EXPORT BasicMatrix operator*(double skalar, const BasicMatrix &vec);
PXL_DLL_EXPORT BasicMatrix operator*(const BasicMatrix &vec, double skalar);

// Fundamental Matrix Vector Products

PXL_DLL_EXPORT BasicNVector operator*(const BasicMatrix &M, const BasicNVector &vec);
PXL_DLL_EXPORT Basic3Vector operator*(const BasicMatrix &M, const Basic3Vector &vec);

} // namespace pxl

#endif // PXL_BASE_BASICMATRIX_HH
