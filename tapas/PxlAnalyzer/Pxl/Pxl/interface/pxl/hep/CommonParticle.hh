//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_HEP_COMMON_PARTICLE_HH
#define PXL_HEP_COMMON_PARTICLE_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <memory>

namespace pxl
{

class CommonParticle;

/// @cond DEVELOPER
	
/**
 The class ParticleHelper is an internal class used by the method
 getAs() in the class CommonParticle.
*/
template<class objecttype>
class PXL_DLL_EXPORT ParticleHelper
{
public:
	ParticleHelper() : object(0), particleRef(0)
	{
	}

	virtual ~ParticleHelper()
	{
		this->transferBack();
		delete object;
	}

	void connect(CommonParticle& part)
	{
		particleRef = &part;
	}


	objecttype* set()
	{
		if (!object)
			object = new objecttype;
		this->setObjectData();
		return object;
	}

private:

	void transferBack()
	{
	}

	void setObjectData()
	{
	}

	objecttype* object;
	CommonParticle* particleRef;

};
/// @endcond DEVELOPER


/**
 * This is the common, pure virtual interface class for particles.
 * The physical representation, (px, py, pz, E) or (pt, eta, phi, m/E), can
 * be chosen by the concrete implementation.
 */

class PXL_DLL_EXPORT CommonParticle
{
public:
	virtual ~CommonParticle()
	{
	}
		
	//getters for basic fourvector quantities in (px, py, pz, E)-representation
	virtual double getPx() const = 0;
	virtual double getPy() const = 0;
	virtual double getPz() const = 0;
	virtual double getE() const = 0;

	//getters for basic fourvector quantities in (pt, eta, phi, mass)-representation
	virtual double getPt() const = 0;
	virtual double getEta() const = 0;
	virtual double getPhi() const = 0;
	virtual double getMass() const = 0;
	
	virtual double getCharge() const = 0;

	//setters for basic fourvector quantities
	virtual void setP4(double px, double py, double pz, double e) = 0;
	virtual void addP4(double px, double py, double pz, double e) = 0;
	virtual void setCharge(double q) = 0;
	
	/// get auto_ptr to helper class which enables using setters
	template<class objecttype>
	// std::auto_ptr<ParticleHelper<objecttype> > getAs()
	std::unique_ptr<ParticleHelper<objecttype> > getAs()
	{
		ParticleHelper<objecttype>* temp = new ParticleHelper<objecttype>;
		temp->connect(*this);
		// return std::auto_ptr<ParticleHelper<objecttype> >(temp);
		return std::unique_ptr<ParticleHelper<objecttype> >(temp);
	}

};


}

#endif /*PXL_HEP_COMMON_PARTICLE_HH*/
