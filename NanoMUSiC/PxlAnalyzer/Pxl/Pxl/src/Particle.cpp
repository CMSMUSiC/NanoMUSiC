//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <iostream>
#include <string>

#include "Pxl/Pxl/interface/pxl/hep/Particle.hpp"

namespace pxl
{

bool operator==(const Particle &obj1, const Particle &obj2)
{
    return obj1.getVector() == obj2.getVector() && obj1.getCharge() == obj2.getCharge();
}

bool operator!=(const Particle &obj1, const Particle &obj2)
{
    return obj1.getVector() != obj2.getVector() || obj1.getCharge() == obj2.getCharge();
}

std::ostream &Particle::print(int level, std::ostream &os, int pan) const
{
    printPan1st(os, pan);
    os << "Particle: '" << getName() << "', p = (" << getPt() << ", " << getPz() << ") m = " << getMass() << std::endl;
    if (level > 0)
        Object::printContent(level, os, pan);
    return os;
}

void Particle::setP4FromDaughters()
{
    setP4(0., 0., 0., 0.);
    for (Relations::const_iterator iter = getDaughterRelations().begin(); iter != getDaughterRelations().end(); ++iter)
    {
        CommonParticle *daughter = dynamic_cast<CommonParticle *>(*iter);
        if (daughter)
            addP4(daughter->getPx(), daughter->getPy(), daughter->getPz(), daughter->getE());
    }
}

void Particle::setP4FromDaughtersRecursive()
{
    if (getDaughterRelations().size() > 0)
        setP4(0., 0., 0., 0.);

    for (Relations::const_iterator iter = getDaughterRelations().begin(); iter != getDaughterRelations().end(); ++iter)
    {
        // update daughter particles
        Particle *particle = dynamic_cast<Particle *>(*iter);
        if (particle)
            particle->setP4FromDaughtersRecursive();
    }

    for (Relations::const_iterator iter = getDaughterRelations().begin(); iter != getDaughterRelations().end(); ++iter)
    {
        // add all common particles
        CommonParticle *daughter = dynamic_cast<CommonParticle *>(*iter);
        if (daughter)
            addP4(daughter->getPx(), daughter->getPy(), daughter->getPz(), daughter->getE());
    }
}

} // namespace pxl
