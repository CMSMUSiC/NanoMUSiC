//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_HEP_PARTICLE_HH
#define PXL_HEP_PARTICLE_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include "Pxl/Pxl/interface/pxl/core/Object.hpp"
#include "Pxl/Pxl/interface/pxl/core/weak_ptr.hpp"

#include "Pxl/Pxl/interface/pxl/core/LorentzVector.hpp"
#include "Pxl/Pxl/interface/pxl/hep/CommonParticle.hpp"

#include "TMath.h"

namespace pxl
{

/**
 * This class represents a particle as measured in a high-energy-physics collision. In addition
 * to the properties inherited from the general PXL object, the particle
 * contains a Lorentz-four-vector, charge and a PDG number. The Lorentz vector is
 * in (px, py, pz, E) representation. Further properties of particles
 * (or similar reconstructed objects) can be added to the contained user records.
 * Furthermore, relations to other object such as other particles and vertices
 * can be established.
 */
class PXL_DLL_EXPORT Particle : public Object, public CommonParticle
{
  public:
    Particle() : Object(), _charge(0), _pdgNumber(0)
    {
    }

    Particle(const Particle &original)
        : Object(original), _vector(original._vector), _charge(original._charge), _pdgNumber(original._pdgNumber)
    {
    }

    explicit Particle(const Particle *original)
        : Object(original), _vector(original->_vector), _charge(original->_charge), _pdgNumber(original->_pdgNumber)
    {
    }

    virtual const Id &getTypeId() const
    {
        return getStaticTypeId();
    }

    static const Id &getStaticTypeId()
    {
        static const Id id("c5515a0d-36bf-4076-bf33-e14343cf5a88");
        return id;
    }

    virtual void serialize(const OutputStream &out) const
    {
        Object::serialize(out);
        _vector.serialize(out);
        out.writeDouble(_charge);
        out.writeInt(_pdgNumber);
    }

    virtual void deserialize(const InputStream &in)
    {
        Object::deserialize(in);
        _vector.deserialize(in);
        in.readDouble(_charge);
        in.readInt(_pdgNumber);
    }

    /// This method grants read access to the vector.
    inline const LorentzVector &getVector() const
    {
        return _vector;
    }

    /// This method grants write access to the vector.
    inline LorentzVector &getVector()
    {
        return _vector;
    }

    /// Returns the particle charge.
    inline double getCharge() const
    {
        return _charge;
    }
    /// Sets the particle charge to \p v.
    inline void setCharge(double v)
    {
        _charge = v;
    }

    /// Returns the particle PDG number.
    inline int getPdgNumber() const
    {
        return _pdgNumber;
    }
    /// Sets the particle PDF number to \p v.
    inline void setPdgNumber(int v)
    {
        _pdgNumber = v;
    }

    /// Adds vector and charge of \p pa.
    inline const Particle &operator+=(const Particle &pa)
    {
        _vector += pa._vector;
        _charge += pa._charge;
        return *this;
    }

    /// Subtracts vector and charge of \p pa.
    inline const Particle &operator-=(const Particle &pa)
    {
        _vector -= pa._vector;
        _charge += pa._charge;
        return *this;
    }

    virtual Serializable *clone() const
    {
        return new Particle(*this);
    }

    /// Get the x component of the four-vector.
    inline double getPx() const
    {
        return _vector.getPx();
    }

    /// Get the y component of the four-vector.
    inline double getPy() const
    {
        return _vector.getPy();
    }

    /// Get the z component of the four-vector.
    inline double getPz() const
    {
        return _vector.getPz();
    }

    /// Get the absolute momentum of the three-vector.
    inline double getP() const
    {
        return _vector.getP();
    }

    /// Get the E component (energy) of the four-vector.
    inline double getE() const
    {
        return _vector.getE();
    }

    /// Get the mass of the four-vector.
    inline double getMass() const
    {
        return _vector.getMass();
    }

    /// Get the transverse momentum.
    inline double getPt() const
    {
        return _vector.getPt();
    }

    /// Get the pseudorapidity
    inline double getEta() const
    {
        return _vector.getEta();
    }

    /// Get the transverse energy.
    inline double getEt() const
    {
        return _vector.getEt();
    }

    /// Get the azimuth angle.
    inline double getPhi() const
    {
        return _vector.getPhi();
    }

    /// Get the polar angle.
    inline double getTheta() const
    {
        return _vector.getTheta();
    }

    /// Set all four-vector components in the (px, py, pz, E) representation.
    /// Note: Due to consistency reasons, no single setters are present.
    inline void setP4(double px, double py, double pz, double e)
    {
        _vector.setPx(px);
        _vector.setPy(py);
        _vector.setPz(pz);
        _vector.setE(e);
    }

    inline void setXYZM(double x, double y, double z, double m)
    {
        if (m >= 0)
            setP4(x, y, z, TMath::Sqrt(x * x + y * y + z * z + m * m));
        else
            setP4(x, y, z, TMath::Sqrt(TMath::Max((x * x + y * y + z * z - m * m), 0.)));
    }

    inline void setPtEtaPhiM(double pt, double eta, double phi, double m)
    {
        pt = TMath::Abs(pt);
        setXYZM(pt * TMath::Cos(phi), pt * TMath::Sin(phi), pt * sinh(eta), m);
    }

    inline void setPtEtaPhiE(double pt, double eta, double phi, double e)
    {
        pt = TMath::Abs(pt);
        setP4(pt * TMath::Cos(phi), pt * TMath::Sin(phi), pt * sinh(eta), e);
    }

    /// Set the four-vector components to those of \p vector.
    inline void setP4(const LorentzVector &vector)
    {
        _vector = vector;
    }

    /// Set the four-vector components to those of \p vector.
    inline void setVector(const LorentzVector &vector)
    {
        _vector = vector;
    }

    /// Adds the passed four-vector components.
    inline void addP4(double px, double py, double pz, double e)
    {
        _vector.setPx(px + _vector.getPx());
        _vector.setPy(py + _vector.getPy());
        _vector.setPz(pz + _vector.getPz());
        _vector.setE(e + _vector.getE());
    }

    /// Sets the four-vector to the sum of the four-vectors of
    /// all direct daughters.
    void setP4FromDaughters();

    /// Recursively sets the four-vector to the sum of the four-vectors of
    /// all daughters. Starts at daughters without further daughters.
    void setP4FromDaughtersRecursive();

    /// Adds the four-vector \p vector.
    inline void addP4(const LorentzVector &vector)
    {
        _vector += vector;
    }

    /// Adds the four-vector \p vector.
    inline void addVector(const LorentzVector &vector)
    {
        _vector += vector;
    }

    /// Adds the four-vector \p particle.
    inline void addP4(const Particle *particle)
    {
        _vector += particle->getVector();
    }

    inline void addParticle(const Particle *pa)
    {
        _vector += pa->getVector();
        _charge += pa->getCharge();
    }

    /// Returns the boost vector for this particle.
    inline Basic3Vector getBoostVector() const
    {
        return _vector.getBoostVector();
    }

    /// Boost this particle by a given Basic3Vector
    inline void boost(const Basic3Vector &boostvector)
    {
        _vector.boost(boostvector);
    }

    /// Boost this particle by the given boost vector components x,y, and z.
    inline void boost(double b_x, double b_y, double b_z)
    {
        _vector.boost(b_x, b_y, b_z);
    }

    /// Prints information about this particle to the stream \p os.
    virtual std::ostream &print(int level = 1, std::ostream &os = std::cout, int pan = 0) const;

    virtual WkPtrBase *createSelfWkPtr()
    {
        return new weak_ptr<Particle>(this);
    }

  private:
    LorentzVector _vector; /// four-vector
    double _charge;        /// float variable representing the charge
    int _pdgNumber;        /// integer number, representing the Particle Data Group ID

    Particle &operator=(const Particle &original)
    {
        return *this;
    }
};

// non-member operators
PXL_DLL_EXPORT bool operator==(const Particle &obj1, const Particle &obj2);
PXL_DLL_EXPORT bool operator!=(const Particle &obj1, const Particle &obj2);

} // namespace pxl

#endif // PXL_HEP_PARTICLE_HH
