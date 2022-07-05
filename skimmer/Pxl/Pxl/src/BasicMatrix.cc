//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/BasicMatrix.hh"
#include <iostream>
namespace pxl
{

void BasicMatrix::serialize(const OutputStream &out) const
{
	Serializable::serialize(out);
	out.writeUnsignedInt(_size1);
	out.writeUnsignedInt(_size1);
	out.writeUnsignedInt(_storageType);
	for (size_t i=0; i<_size1*_size2;i++)
	{
	out.writeDouble(_data[i]);
	}
	out.writeString(_name);
}

void BasicMatrix::deserialize(const InputStream &in)
{
	Serializable::deserialize(in);
	unsigned int dummy;
	in.readUnsignedInt(dummy);
	_size1 = dummy;
	in.readUnsignedInt(dummy);
	_size2 = dummy;
	in.readUnsignedInt(dummy);
	_storageType = (StorageOrder)dummy;
	if (_data && (!_alienarray))
		delete[] _data;
	_data = new double[_size1*_size2];
	_alienarray = false;

	for (size_t i=0; i<_size1*_size2;i++)
	{
		in.readDouble(_data[i]);
	}
	in.readString(_name);
}

void BasicMatrix::use(size_t size1, size_t size2, double *data)
{
	if ((_data) && (!_alienarray))
		delete[] _data;
	_alienarray = true;
	_data = data;
	_size1 = size1;
	_size2 = size2;
}


bool BasicMatrix::isRowBasedStorage() const
{
	switch(_storageType)
	{
		case ROWMAJOR:
			return true;
			break;
		case COLUMNMAJOR:
			return false;
			break;
		default:
			throw std::runtime_error("Something went very wrong! Matrix neither in Row nor Column Storage!");
	}
}


bool BasicMatrix::isColumnBasedStorage() const
{
	switch(_storageType)
	{
		case ROWMAJOR:
			return false;
			break;
		case COLUMNMAJOR:
			return true;
			break;
		default:
			throw std::runtime_error("Something went very wrong! Matrix neither in Row nor Column Storage!");
	}
}


void BasicMatrix::resize(size_t i, size_t j)
{
	if (_data)
		delete _data;
	_data = new double[i*j];
	_size1 = i;
	_size2 = j;
}


void BasicMatrix::reshape(size_t i, size_t j)
{
	if ((i*j)!=(_size1*_size2))
	{
			throw std::runtime_error("Number of elements doens't match!");
	}
	_size1 = i;
	_size2 = j;
}


const BasicMatrix& BasicMatrix::operator=(const BasicMatrix& M)
{
		_size1 = M.getSize1();
		_size2 = M.getSize2();
		_data = new double[_size1 * _size2];
		if (M.isColumnBasedStorage())
		{
			this->setColumnBasedStorage();
		}
		else
		{
			this->setRowBasedStorage();
		}
		//const double* vecdata= vec.getConstArray();
		//memcpy(_data,vecdata,_size1);
		for (size_t i=0; i<_size1*_size2;i++)
		{
			_data[i] = M.getConstArray()[i];
		}
		return *this;
}


const BasicMatrix& BasicMatrix::operator+=(const BasicMatrix& M)
{
	if ((M.getSize1() != _size1) || (M.getSize2() != _size2))
	{
		throw std::runtime_error("Size Mismatch! Only Matrices of same size can be added.");
	}
	if (this->isColumnBasedStorage() == M.isColumnBasedStorage())
	{
		for (size_t i=0; i<_size1;i++)
		{
			for (size_t j=0; j<_size2;j++)
			{
				(_data)[i*_size2+j] += M.getElement(i,j);
			}
		}
	}
	else
	{ // different storage scheme!
	for (size_t i=0; i<_size1;i++)
		{
			for (size_t j=0; j<_size2;j++)
			{
				(_data)[i*_size2 + j] += M.getElement(j,i);
			}
		}
	}
	return *this;
}


const BasicMatrix& BasicMatrix::operator-=(const BasicMatrix& M)
{
	if ((M.getSize1() != _size1) || (M.getSize2() != _size2))
	{
		throw std::runtime_error("Size Mismatch! Only Matrices of same size can be added.");
	}
	if (this->isColumnBasedStorage() == M.isColumnBasedStorage())
	{
		for (size_t i=0; i<_size1;i++)
		{
			for (size_t j=0; j<_size2;j++)
			{
				(_data)[i*_size2+j] -= M.getElement(i,j);
			}
		}
	}
	else
	{ // different storage scheme!
	for (size_t i=0; i<_size1;i++)
		{
			for (size_t j=0; j<_size2;j++)
			{
				(_data)[i*_size2+j] -= M.getElement(j,i);
			}
		}
	}
	return *this;
}


BasicMatrix BasicMatrix::operator+(const BasicMatrix& M)
{
	BasicMatrix out = *this;
	out += M;
	return out;
}


BasicMatrix BasicMatrix::operator-(const BasicMatrix& M )
{
	BasicMatrix out = *this;
	out -= M;
	return out;
}


const BasicMatrix& BasicMatrix::operator*=(double skalar)
{
	for (size_t i=0; i<_size1*_size2;i++)
	{
		(_data)[i] *= skalar;
	}
	return *this;
}


const BasicMatrix& BasicMatrix::operator/=(double skalar)
{
	for (size_t i=0; i<_size1*_size2;i++)
	{
		(_data)[i] /= skalar;
	}
	return *this;
}


const BasicMatrix BasicMatrix::operator/(double skalar) const
{
	BasicMatrix out = *this;
	out /= skalar;
	return out;
}

double& BasicMatrix::operator()(size_t i, size_t j)
{
	if ((i >= _size1) || (j >= _size2))
		throw std::runtime_error("Index out of range");
	else
	{
		return _data[i*_size2 + j];
	}
}

std::ostream& BasicMatrix::print(int level, std::ostream& os, int pan) const
{
	os.precision(2);
	os << std::scientific;
	if (this->isRowBasedStorage())
	{
		for (size_t i=0; i<_size1; i++)
		{
			for (size_t j=0; j<_size2; j++)
			{
				os << this->getElement(i,j) << "  ";
			}
			os << std::endl;
		}
	}
	else
	{ // Column Based
		for (size_t j=0; j<_size2; j++)
		{
			for (size_t i=0; i<_size1; i++)
			{
				os << this->getElement(i,j) << "  ";
			}
			os << std::endl;
		}
	}
	return os;
}

BasicMatrix operator*(const BasicMatrix &M, double skalar)
{
	BasicMatrix R(M);
	R*=skalar;
	return R;
}

BasicMatrix operator*(double skalar, const BasicMatrix &M)
{
	BasicMatrix R(M);
	R*=skalar;
	return R;
}

bool operator==(const BasicMatrix& obj1, const BasicMatrix& obj2)
{
	if ((obj1.getSize1() != obj2.getSize1()) || (obj1.getSize2() != obj2.getSize2()))
	{
		return false;
	}
	else
	{
		bool result = true;
		size_t iter = 0;
		while (result && (iter < obj1.getSize1()*obj1.getSize2()))
		{
			result = (obj1.getConstArray()[iter] == obj2.getConstArray()[iter]);
			iter++;
		}
		return result;
	}
}

bool operator!=(const BasicMatrix& obj1, const BasicMatrix& obj2)
{
	return !(obj1==obj2);
}

BasicNVector operator*(const BasicMatrix& M, const BasicNVector& vec)
{

	if (vec.getSize() != M.getNumberOfColumns())
	{
		throw std::runtime_error("Size Mismatch in matrix-vector multiplication");
	}

	BasicNVector out(M.getNumberOfRows());
	for (size_t i=0;i<M.getNumberOfRows();i++)
	{
		for (size_t j=0;j<M.getNumberOfColumns();j++)
		{
			if (M.isRowBasedStorage())
			{
				out(i)+=vec.getElement(j)*M.getElement(i,j);
			}
			else
			{
				out(i)+=vec.getElement(j)*M.getElement(j,i);
			}
		}
	}
	return out;
}

Basic3Vector operator*(const BasicMatrix& M, const Basic3Vector& vec)
{

	if (M.getNumberOfColumns()!=3)
	{
		throw std::runtime_error("Size Mismatch in matrix-vector multiplication");
	}

	Basic3Vector out;
	for (size_t i=0;i<M.getNumberOfRows();i++)
	{
		for (size_t j=0;j<M.getNumberOfColumns();j++)
		{
			if (M.isRowBasedStorage())
			{
				out.getArray()[i]+=vec.getElement(j)*M.getElement(i,j);
			}
			else
			{
				out.getArray()[i]+=vec.getElement(j)*M.getElement(j,i);
			}
		}
	}
	return out;
}



//
//BasicNVector operator*(const BasicMatrix& M, const BasicNVector& vec);



} // namespace pxl



