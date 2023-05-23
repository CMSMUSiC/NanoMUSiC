#ifndef ParticleVector_hh
#define ParticleVector_hh

#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"

#include <string>

class ParticleVector
{
    const std::string name;
    const std::vector<pxl::Particle *> particles;

  public:
    ParticleVector()
    {
    }
    ParticleVector(const std::string &name, const std::vector<pxl::Particle *> &vec)
        : name(name),
          particles(vec)
    {
    }
    std::string getParticleName() const
    {
        return name;
    }
    const std::vector<pxl::Particle *> &getParticles() const
    {
        return particles;
    }
    int getCount() const
    {
        return particles.size();
    }
    const pxl::Particle *at(const size_t index) const
    {
        return particles.at(index);
    }
};

#endif
