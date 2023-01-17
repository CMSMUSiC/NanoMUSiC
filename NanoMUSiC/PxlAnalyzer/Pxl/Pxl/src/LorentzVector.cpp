//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <string>

#include "Pxl/Pxl/interface/pxl/core/LorentzVector.hpp"

namespace pxl
{

bool operator==(const LorentzVector &obj1, const LorentzVector &obj2)
{
    return obj1.getX() == obj2.getX() && obj1.getY() == obj2.getY() && obj1.getZ() == obj2.getZ() &&
           obj1.getE() == obj2.getE();
}

bool operator!=(const LorentzVector &obj1, const LorentzVector &obj2)
{
    return obj1.getX() != obj2.getX() || obj1.getY() != obj2.getY() || obj1.getZ() != obj2.getZ() ||
           obj1.getE() != obj2.getE();
}

void LorentzVector::boost(double b_x, double b_y, double b_z)
{
    // Boost this LorentzVector
    double b2 = b_x * b_x + b_y * b_y + b_z * b_z;
    double gamma = 1.0 / sqrt(1.0 - b2);
    double bp = b_x * getX() + b_y * getY() + b_z * getZ();
    double gamma2 = b2 > 0 ? (gamma - 1.0) / b2 : 0.0;

    setX(getX() + gamma2 * bp * b_x + gamma * b_x * getT());
    setY(getY() + gamma2 * bp * b_y + gamma * b_y * getT());
    setZ(getZ() + gamma2 * bp * b_z + gamma * b_z * getT());

    setT(gamma * (getT() + bp));
}

LorentzVector LorentzVector::operator-(const LorentzVector &vec) const
{
    LorentzVector out(*this);
    out -= vec;
    return out;
}

LorentzVector LorentzVector::operator+(const LorentzVector &vec) const
{
    LorentzVector out(*this);
    out += vec;
    return out;
}

LorentzVector LorentzVector::operator-() const
{
    return LorentzVector(-1. * getX(), -1. * getY(), -1. * getZ(), -1. * getT());
}

} // namespace pxl
