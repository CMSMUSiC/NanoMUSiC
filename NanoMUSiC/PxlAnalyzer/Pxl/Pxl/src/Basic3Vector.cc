//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <string>

#include "Pxl/Pxl/interface/pxl/core/Basic3Vector.hh"
#include "Pxl/Pxl/interface/pxl/core/RotationMatrix.hh"

#ifndef EPSILON
#define EPSILON 1.0e-9
#endif

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

namespace pxl
{

bool operator==(const Basic3Vector& obj1, const Basic3Vector& obj2)
{
	return ((std::fabs(obj1.getX() - obj2.getX()) < DBL_EPSILON) && (std::fabs(obj1.getY() - obj2.getY()) < DBL_EPSILON) && (std::fabs(obj1.getZ() - obj2.getZ()) < DBL_EPSILON));
}

bool operator!=(const Basic3Vector& obj1, const Basic3Vector& obj2)
{
	return !(obj1==obj2);
		//obj1.getX() != obj2.getX() || obj1.getY() != obj2.getY() || obj1.getZ() != obj2.getZ();
}

bool Basic3Vector::isUnitVector() const
{
	if ((std::abs(sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2]) - 1.0))
			<= DBL_EPSILON)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Basic3Vector::setRhoPhi(double perp, double phi)
{
	_v[0] = std::cos(phi) * perp;
	_v[1] = std::sin(phi) * perp;
}

void Basic3Vector::setRhoPhiZ(double perp, double phi, double z)
{
	setRhoPhi(perp, phi);
	_v[2] = z;
}

void Basic3Vector::setRThetaPhi(double r, double theta, double phi)
{
	setRhoPhiZ(std::sin(theta) * r, phi, std::cos(theta) * r);
}

bool Basic3Vector::isNullPerp() const
{
	return _v[0] > -EPSILON && _v[0] < EPSILON && _v[1] > -EPSILON && _v[1]
			< EPSILON;
}

bool Basic3Vector::isNull() const
{
	return isNullPerp() && _v[2] > -EPSILON && _v[2] < EPSILON;
}

double Basic3Vector::getPerp2() const
{
	return _v[0] * _v[0] + _v[1] * _v[1];
}
double Basic3Vector::getPerp() const
{
	return std::sqrt(getPerp2());
}

double Basic3Vector::getPhi() const
{
	return isNullPerp() ? 0.0 : std::atan2(_v[1], _v[0]);
}
double Basic3Vector::getMag2() const
{
	return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
}
double Basic3Vector::getMag() const
{
	return std::sqrt(getMag2());
}

double Basic3Vector::getCosTheta() const
{
	double mag = getMag();
	return mag < EPSILON ? 1.0 : _v[2] / mag;
}

double Basic3Vector::getCos2Theta() const
{
	double mag2 = getMag2();
	return mag2 < EPSILON ? 1.0 : _v[2] * _v[2] / mag2;
}

double Basic3Vector::getTheta() const
{
	return isNull() ? 0.0 : std::atan2(getPerp(), _v[2]);
}


double Basic3Vector::deltaRho(const Basic3Vector& fv) const
{
	double dDtheta = deltaTheta(fv);
	double dDphi = deltaPhi(fv);
	return std::sqrt(dDtheta * dDtheta + dDphi * dDphi);
}

double Basic3Vector::deltaPhi(const Basic3Vector& fv) const
{
	double dDphi = getPhi() - fv.getPhi();
	while (dDphi > M_PI)
		dDphi -= 2 * M_PI;
	while (dDphi < -M_PI)
		dDphi += 2 * M_PI;
	return dDphi;
}

double Basic3Vector::deltaTheta(const Basic3Vector& fv) const
{
	double dDtheta = getTheta() - fv.getTheta();
	while (dDtheta > M_PI)
		dDtheta -= 2 * M_PI;
	while (dDtheta < -M_PI)
		dDtheta += 2 * M_PI;
	return dDtheta;
}

Basic3Vector operator*(double skalar, const Basic3Vector& vec)
{
	Basic3Vector out;
	out.setX(vec.getX() * skalar);
	out.setY(vec.getY() * skalar);
	out.setZ(vec.getZ() * skalar);
	return out;
}

Basic3Vector operator*(const Basic3Vector& vec, double skalar)
{
	Basic3Vector out;
	out.setX(vec.getX() * skalar);
	out.setY(vec.getY() * skalar);
	out.setZ(vec.getZ() * skalar);
	return out;
}

Basic3Vector Basic3Vector::getETheta() const
{
	Basic3Vector out;
	out.setX(cos(this->getTheta()) * cos(this->getPhi()));
	out.setY(cos(this->getTheta()) * sin(this->getPhi()));
	out.setZ(-1 * sin(this->getTheta()));
	return out;
}

Basic3Vector Basic3Vector::getEPhi() const
{
	Basic3Vector out;
	out.setX(-1 * sin(this->getPhi()));
	out.setY(cos(this->getPhi()));
	out.setZ(0);
	return out;
}

const Basic3Vector& Basic3Vector::operator=(const Basic3Vector& vec)
{
	_v[0] = vec._v[0];
	_v[1] = vec._v[1];
	_v[2] = vec._v[2];
	return *this;
}

const Basic3Vector& Basic3Vector::operator+=(const Basic3Vector& vec)
{
	_v[0] += vec._v[0];
	_v[1] += vec._v[1];
	_v[2] += vec._v[2];
	return *this;
}

const Basic3Vector& Basic3Vector::operator-=(const Basic3Vector& vec)
{
	_v[0] -= vec._v[0];
	_v[1] -= vec._v[1];
	_v[2] -= vec._v[2];
	return *this;
}

const Basic3Vector& Basic3Vector::operator*=(double skalar)
{
	_v[0] *= skalar;
	_v[1] *= skalar;
	_v[2] *= skalar;
	return *this;
}

const Basic3Vector& Basic3Vector::operator/=(double skalar)
{
	_v[0] /= skalar;
	_v[1] /= skalar;
	_v[2] /= skalar;
	return *this;
}

Basic3Vector Basic3Vector::operator/(double skalar) const
{
	Basic3Vector out;
	out.setX(_v[0] / skalar);
	out.setY(_v[1] / skalar);
	out.setZ(_v[2] / skalar);
	return out;
}

// Scalar product
double Basic3Vector::operator*(const Basic3Vector& vec) const
{
	return vec._v[0] * _v[0] + vec._v[1] * _v[1] + vec._v[2] * _v[2];
}

Basic3Vector Basic3Vector::operator+(const Basic3Vector& vec)
{
	Basic3Vector out = *this;
	out += vec;
	return out;
}

Basic3Vector Basic3Vector::operator-(const Basic3Vector& vec)
{

	Basic3Vector out = *this;
	out -= vec;
	return out;
}

void Basic3Vector::normalize()
{
	*this/=this->getMag();
}


double Basic3Vector::norm(uint32_t norm) const
{
	if (norm == 0xffffffff)
		return 3.;
	else if (norm == 0)
		throw std::runtime_error("norm of 0 not defined");

	double n = 0;
	if (norm == 1)
	{
		for (size_t i = 0; i < 3; i++)
			n += fabs(_v[i]);
	}
	else if (norm == 2)
	{
		for (size_t i = 0; i < 3; i++)
			n += (_v[i] * _v[i]);
		n = sqrt(n);
	}
	else
	{
		for (size_t i = 0; i < 3; i++)
			n += pow(fabs(_v[i]), (double)norm);
		n = pow (n, 1.0 / double(norm));
	}

	return n;
}


double Basic3Vector::getAngleTo(const Basic3Vector& vec) const
{

		double cosdistance = (*this)*vec / this->getMag() / vec.getMag();
		// In some directions cosdistance is > 1 on some compilers
		// This ensures that the correct result is returned
		return (cosdistance >= 1.) ? 0 : ((cosdistance <= -1.) ? M_PI :acos(cosdistance)) ;
}


void Basic3Vector::rotate(const Basic3Vector& axis, double angle)
{
	RotationMatrix R(axis/axis.getMag(), angle);
	*this= R*(*this);
}

} // namespace pxl
#undef EPSILON
