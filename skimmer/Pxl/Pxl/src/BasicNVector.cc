//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------


#include "Pxl/Pxl/interface/pxl/core/BasicNVector.hh"
#include "Pxl/Pxl/interface/pxl/core/logging.hh"

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::BasicNVector"

namespace pxl
{

void BasicNVector::serialize(const OutputStream &out) const
{
	Serializable::serialize(out);
	out.writeUnsignedInt(_size1);
	for (size_t i = 0; i < _size1; i++)
	{
		out.writeDouble(_data[i]);
	}
	out.writeString(_name);
}

void BasicNVector::deserialize(const InputStream &in)
{
	Serializable::deserialize(in);

	unsigned int size;
	in.readUnsignedInt(size);
	_size1 = size;
	if ((_data) && (!_alienarray))
		delete[] _data;
	_data = new double[_size1];
	_alienarray = false;
	for (size_t i = 0; i < _size1; i++)
	{
		in.readDouble(_data[i]);
	}
	in.readString(_name);
}

void BasicNVector::use(size_t size, double *data)
{
	if ((_data) && (!_alienarray))
	{
		delete[] _data;
		_data = NULL;
	}
	_alienarray = true;
	_data = data;
	_size1 = size;
}

void BasicNVector::unUseArray()
{
  if (!_alienarray)
  {
    PXL_LOG_WARNING << " trying to unuse alienarray without using one!";
  }
  else
  {
    _alienarray = false;
    _data = NULL;
    _size1 = 0;
  }
}

double BasicNVector::operator*(const BasicNVector& vec) const
{
	if (vec.getSize() != _size1)
	{
		throw std::runtime_error(
				"Size Mismatch! There is only a scalar product defined for vectors of same size.");
	}
	else
	{
		double result = 0;
		for (size_t i = 0; i < _size1; i++)
		{
			result += (_data)[i] * vec.getElement(i);
		}
		return result;
	}
}

double& BasicNVector::operator()(size_t i)
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

const BasicNVector BasicNVector::operator/(double scalar) const
{
	BasicNVector out = *this;
	out /= scalar;
	return out;
}

const BasicNVector& BasicNVector::operator/=(double scalar)
{
	for (size_t i = 0; i < _size1; i++)
	{
		(_data)[i] /= scalar;
	}
	return *this;
}

const BasicNVector& BasicNVector::operator-=(const BasicNVector& vec)
{
	if (vec.getSize() != _size1)
	{
		throw std::runtime_error(
				"Size Mismatch! Only vectors of same size can be substracted.");

	}
	else
	{
		for (size_t i = 0; i < _size1; i++)
		{
			(_data)[i] -= vec.getElement(i);
		}
		return *this;
	}
}

const BasicNVector& BasicNVector::operator*=(double scalar)
{
	for (size_t i = 0; i < _size1; i++)
	{
		(_data)[i] *= scalar;
	}
	return *this;
}

const BasicNVector& BasicNVector::operator+=(const BasicNVector& vec)
{
	if (vec.getSize() != _size1)
	{
		throw std::runtime_error(
				"Size Mismatch! Only vectors of same size can be added.");

	}
	else
	{
		for (size_t i = 0; i < _size1; i++)
		{
			(_data)[i] += vec.getElement(i);
		}
		return *this;
	}
}

BasicNVector BasicNVector::operator+(const BasicNVector& vec)
{
	BasicNVector out = *this;
	out += vec;
	return out;
}

BasicNVector BasicNVector::operator-(const BasicNVector& vec)
{

	BasicNVector out = *this;
	out -= vec;
	return out;
}

const BasicNVector& BasicNVector::operator=(const BasicNVector& vec)
{
	_size1 = vec.getSize();
	if ((_data) && !_alienarray)
		delete[] _data;
	if (_alienarray)
		_alienarray = false;

	_data = new double[_size1];

	memcpy(_data,vec.getConstArray(),_size1 *sizeof(double));

	return *this;
}

BasicNVector operator*(double scalar, const BasicNVector& vec)
{
	BasicNVector out = vec;
	out *= scalar;
	return out;
}

BasicNVector operator*(const BasicNVector& vec, double scalar)
{
	BasicNVector out = vec;
	out *= scalar;
	return out;
}

bool operator==(const BasicNVector& obj1, const BasicNVector& obj2)
{
	if (obj1.getSize() != obj2.getSize())
	{
		return false;
	}
	else
	{
		bool result = true;
		size_t iter = 0;
		while (result && (iter < obj1.getSize()))
		{
			result = (obj1.getElement(iter) == obj2.getElement(iter));
			iter++;
		}
		return result;
	}
}

bool operator!=(const BasicNVector& obj1, const BasicNVector& obj2)
{
	return !(obj1==obj2);
}

std::ostream& BasicNVector::print(int level, std::ostream& os, int pan) const
{
	os.precision(2);
	os << std::scientific;
	for (size_t i=0; i<_size1; i++)
	{
			os << this->getElement(i) << std::endl;
	}
	return os;
}

double BasicNVector::norm(uint32_t norm) {
	if (norm == 0xffffffff)
		return (double)_size1;
	else if (norm == 0)
		throw std::runtime_error("norm of 0 not defined");
	
	double n = 0;
	if (norm == 1)
	{
		for (size_t i = 0; i < _size1; i++)
			n += fabs(_data[i]);
	}
	else if (norm == 2)
	{
		for (size_t i = 0; i < _size1; i++)
			n += (_data[i] * _data[i]);
		n = sqrt(n);
	}
	else
	{
		for (size_t i = 0; i < _size1; i++)
			n += pow(fabs(_data[i]), (double)norm);
		n = pow (n, 1.0 / double(norm));
	}
	return n;
}

void BasicNVector::setSize(size_t size)
{
	if (_alienarray)
	{
		throw std::runtime_error("BasicNVector:: Trying to resize an unowned array!");
	}
	if (!_data)
	{
		_data = new double[size];
		std::fill_n(_data, size, 0 );
	}
	else if (size > _size1)
	{
		// allocate new arra, copy data and free memory
		double *tmp = _data;
		_data = new double[size];
		std::fill_n(_data, size, 0 );
		memcpy(_data,tmp, _size1*sizeof(double));
		//std::fill_n(_data+_size1*sizeof(double), size-_size1, 0 );
		delete[] tmp;
	}
	_size1 = size;
}




} // namespace pxl
