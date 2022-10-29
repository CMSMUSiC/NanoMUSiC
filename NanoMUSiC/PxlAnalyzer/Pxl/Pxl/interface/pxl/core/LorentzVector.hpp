//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_LORENTZVECTOR_HH
#define PXL_BASE_LORENTZVECTOR_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include <cmath>

#include "Pxl/Pxl/interface/pxl/core/Basic3Vector.hpp"
#include "Pxl/Pxl/interface/pxl/core/Stream.hpp"

namespace pxl
{
// pol

/**
 This class provides a simple Lorentz-fourvector with basic algebra. The methods provided are self-explanatory.
 */
class PXL_DLL_EXPORT LorentzVector : public Basic3Vector
{
  public:
    LorentzVector() : Basic3Vector(0, 0, 0), _t(0)
    {
    }

    LorentzVector(const Basic3Vector &orig, double t = 0) : Basic3Vector(orig), _t(t)
    {
    }
    LorentzVector(const LorentzVector &orig) : Basic3Vector(orig), _t(orig._t)
    {
    }
    explicit LorentzVector(const LorentzVector *orig) : Basic3Vector(orig), _t(orig->_t)
    {
    }

    LorentzVector(double x, double y, double z, double t = 0) : Basic3Vector(x, y, z), _t(t)
    {
    }

    virtual ~LorentzVector()
    {
    }

    virtual void serialize(const OutputStream &out) const
    {
        Basic3Vector::serialize(out);
        out.writeDouble(_t);
    }

    virtual void deserialize(const InputStream &in)
    {
        Basic3Vector::deserialize(in);
        in.readDouble(_t);
    }

    // setX inherited
    // setY inherited
    // setZ inherited
    inline void setT(double t)
    {
        _t = t;
    }

    inline void setPx(double px)
    {
        setX(px);
    }
    inline void setPy(double py)
    {
        setY(py);
    }
    inline void setPz(double pz)
    {
        setZ(pz);
    }

    inline void setE(double e)
    {
        _t = e;
    }

    // getX inherited
    // getY inherited
    // getZ inherited
    inline double getT() const
    {
        return _t;
    }

    inline double getPx() const
    {
        return getX();
    }

    inline double getPy() const
    {
        return getY();
    }

    inline double getPz() const
    {
        return getZ();
    }

    inline double getE() const
    {
        return _t;
    }

    /// returns squared mass
    inline double getMass2() const
    {
        return _t * _t - getMag2();
    }

    inline double getMass() const
    {
        double m2 = getMass2();
        return m2 < 0.0 ? 0.0 : std::sqrt(m2);
    }

    // getPerp inherited

    /// returns transverse momentum
    inline double getPt() const
    {
        return getPerp();
    }

    double getP() const
    {
        return getMag();
    }

    // getPhi inherited
    // getTheta inherited
    // deltaRho inherited
    // deltaPhi inherited
    // deltaTheta inherited

    /// returns the pseudorapidity
    inline double getEta() const
    {
        return -std::log(std::tan(getTheta() * 0.5));
    }

    /// returns the rapidity
    inline double getRapidity() const
    {
        return 0.5 * std::log((getE() + getP()) / (getE() - getP()));
    }

    /// returns the rapidity relative to the beam axis
    inline double getBeamlineRapidity() const
    {
        return 0.5 * std::log((getE() + getPz()) / (getE() - getPz()));
    }

    /// returns transverse energy squared
    inline double getEt2() const
    {
        double pt2 = getPerp2();
        return pt2 == 0.0 ? 0.0 : _t * _t * pt2 / getMag2();
    }

    /// returns transverse energy
    inline double getEt() const
    {
        return std::sqrt(getEt2());
    }

    /// returns sqrt(dEta**2 + dPhi**2) between this LorentzVector and parameter value
    /// Note: This version with a passed pointer is deprecated
    inline double deltaR(const LorentzVector *fv) const
    {
        return deltaR(*fv);
    }

    /// returns sqrt(dEta**2 + dPhi**2) between this LorentzVector and parameter value
    double deltaR(const LorentzVector &fv) const
    {
        double dDeta = deltaEta(fv);
        double dDphi = deltaPhi(fv);
        return std::sqrt(dDeta * dDeta + dDphi * dDphi);
    }

    /// returns the difference in pseudorapidity between this and parameter value
    /// Note: This version with a passed pointer is deprecated
    inline double deltaEta(const LorentzVector *fv) const
    {
        return deltaEta(*fv);
    }

    /// returns the difference in pseudorapidity between this and parameter value
    double deltaEta(const LorentzVector &fv) const
    {
        return getEta() - fv.getEta();
    }

    /// returns the difference in rapidity between this and parameter value
    inline double deltaRapidity(const LorentzVector &fv) const
    {
        return getRapidity() - fv.getRapidity();
    }

    /// returns the difference in beamline rapidity between this and parameter value
    inline double deltaBeamlineRapidity(const LorentzVector &fv) const
    {
        return getBeamlineRapidity() - fv.getBeamlineRapidity();
    }

    /// returns spatial components divided by time component
    inline Basic3Vector getBoostVector() const
    {
        return Basic3Vector(getX() / getT(), getY() / getT(), getZ() / getT());
    }

    /// LorentzBoost: Boost this vector with given vector boostvector
    inline void boost(const Basic3Vector &boostvector)
    {
        boost(boostvector.getX(), boostvector.getY(), boostvector.getZ());
    }

    /// LorentzBoost: Boost this vector with given vector boostvector components
    void boost(double b_x, double b_y, double b_z);

    /// set x,y and z components of this LorentzVector to vec
    inline const LorentzVector &operator=(const Basic3Vector &vec)
    {
        Basic3Vector::operator=(vec);
        return *this;
    }

    /// set this LorentzVector to vec
    inline const LorentzVector &operator=(const LorentzVector &vec)
    {
        Basic3Vector::operator=(vec);
        _t = vec._t;
        return *this;
    }

    /// this = this plus vec
    inline const LorentzVector &operator+=(const LorentzVector &vec)
    {
        Basic3Vector::operator+=(vec);
        _t += vec._t;
        return *this;
    }

    /// this = this minus vec
    inline const LorentzVector &operator-=(const LorentzVector &vec)
    {
        Basic3Vector::operator-=(vec);
        _t -= vec._t;
        return *this;
    }

    /// return a copy of this where all components are multiplied with -1.
    LorentzVector operator-() const;

    /// return a copy of (this minus vec)
    LorentzVector operator-(const LorentzVector &) const;

    /// return a copy of (this plus vec)
    LorentzVector operator+(const LorentzVector &) const;

  private:
    double _t;
};

// non-member operators
PXL_DLL_EXPORT bool operator==(const LorentzVector &obj1, const LorentzVector &obj2);
PXL_DLL_EXPORT bool operator!=(const LorentzVector &obj1, const LorentzVector &obj2);

} // namespace pxl

#endif // PXL_BASE_LORENTZVECTOR_HH
